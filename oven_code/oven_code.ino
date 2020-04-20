#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>
#include <PID_v1.h>
#include "Adafruit_MAX31855.h"

// HEAT
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

double lastTemp = -274;
double c;               // 'c' will be the current temperature
int targetTemp = 0;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// TIMER
unsigned long targetTime = 15 * 60000;
unsigned long timerStart;
int currMin = 0;
int lastMin = -1;
boolean done = false;

double variance = 0.8;

// LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// ENCODER
#define outputA 7
#define outputB 8
#define outputSW 6

int aState;
int aLastState;

bool confirmed = false;

// PID CONTROLLER
double Setpoint, Input, Output;   // Define Variables we'll be connecting to

// Specify the links and initial tuning parameters
double Kp = 48, Ki = 0.213, Kd = 2700.0;

// Created PID object
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT );

int WindowSize = 5000;
unsigned long windowStartTime;
unsigned long previousMillis = 0;        // will store last time Thermocouple
const long interval = 500;           // interval at which to sample (milliseconds)

// RELAY
int RelayPin = 9;

// TIMER
unsigned long currTime = 0;
unsigned long tempTime = 0;
unsigned long switchTime = 0;
unsigned long ovenTime = 0;

boolean pressed = false;
boolean rightTemp = false;

void setup() {
  Serial.begin(9600);

  // ENCODER
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(outputSW, INPUT_PULLUP);

  aLastState = digitalRead(outputA);

  // RELAY
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, HIGH);

  // PID CONTROLLER
  windowStartTime = millis();

  myPID.SetOutputLimits(0, WindowSize); // tell the PID to range between 0 and the full window size

  myPID.SetMode(AUTOMATIC); // turn the PID on

  // LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.setCursor(0, 1);
  lcd.print("Target: ");
}

void loop() {
  currTime = millis(); // Updates current time

  updateSwitch();

  if (confirmed) updateTimer();

  updateTemperature();

  updateEncoder();

  if (confirmed && !done && currTime > switchTime) updateRelay();
  else digitalWrite(RelayPin, HIGH);
}

void updateSwitch () {
  if (currTime > switchTime && !digitalRead(outputSW)) {
    switchTime = currTime + 2000; // 2000 ms delay between next update

    confirmed = !confirmed;
    pressed = true;
  }
  else if (switchTime > currTime) {
    if (confirmed) {
      lcd.setCursor(15, 1);
      lcd.print("!");
      Setpoint = targetTemp;

      timerStart = currTime;
      rightTemp = false;
      done = false;
    } else {
      lcd.setCursor(15, 1);
      lcd.print("X");
    }
  }
  else if (currTime > switchTime && pressed) {
    lcd.setCursor(15, 1);
    lcd.print(" ");
    pressed = false;
  }
}

void updateTemperature() {
  if (currTime > tempTime) {
    tempTime = currTime + 1000; // 1000 ms delay between next update

    c = thermocouple.readCelsius();

    if (c != lastTemp) {
      lcd.setCursor(9, 0);
      lcd.print("      ");
      lcd.setCursor(9, 0);
      lcd.print(c, 1);
      lcd.print((char)223);
      lcd.print('C');
    }

    lastTemp = c;
  }
}

void updateTimer() {
  currMin = (int)floor((currTime - timerStart) / 60000);
  if (lastMin != currMin && !pressed && rightTemp) {
    lcd.setCursor(14, 1);
    lcd.print("  ");

    Serial.println("HERE");

    if (currMin < 10) {
      lcd.setCursor(15, 1);
      lcd.print(currMin);
      Serial.println("Printing Single");
    } else {
      lcd.setCursor(14, 1);
      lcd.print(currMin);
      Serial.println("Printing Double");
    }

    Serial.println(currMin);

    lastMin = currMin;
  } else if (!rightTemp) {
    lcd.setCursor(14, 1);
    lcd.print("  ");
  }
}

void updateEncoder() {
  aState = digitalRead(outputA);

  if ((!confirmed && currTime > switchTime) && aState != aLastState) {
    if (digitalRead(outputB) != aState) {
      if (targetTemp <= 345)
        targetTemp += 5;
    } else {
      if (targetTemp >= 5)
        targetTemp -= 5;
    }

    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.setCursor(8, 1);
    lcd.print(targetTemp);
    lcd.print((char)223);
    lcd.print('C');
  }

  aLastState = aState;
}

void updateRelay() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
  }

  Input = c;
  myPID.Compute();

  /************************************************
     turn the output pin on/off based on pid output
   ************************************************/
  unsigned long now = millis();
  if (now - windowStartTime > WindowSize) {
    //time to shift the Relay Window
    windowStartTime += WindowSize;
  }

  if (c < targetTemp && Output > now - windowStartTime)
    digitalWrite(RelayPin, LOW);  // HEAT ON
  else
    digitalWrite(RelayPin, HIGH); // HEAT OFF

  // Checks if current temp is within target temp range
  if (abs(c - targetTemp) <= variance) {
    rightTemp = true;

    if (currTime - timerStart >= targetTime) {
      Serial.println("\nDone");
      done = true;
    }
  } else {
    // Resets the start of the time
    timerStart = currTime;
    rightTemp = false;
  }
}
