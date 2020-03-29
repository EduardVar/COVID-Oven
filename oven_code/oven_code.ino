#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>
#include <PID_v1.h>
#include "Adafruit_MAX31855.h"

// defines variables
long duration;
int distance;

// HEAT
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

double lastTemp = -274;
double c;               // 'c' will be the current temperature
int targetTemp = 0;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

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
//double Kp=1, Ki=3.1, Kd=0.2;
double Kp = 48, Ki = 0.213, Kd = 2700.0;

// PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
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

void setup() {
  // put your setup code here, to run once:
  // ENCODER
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(outputSW, INPUT_PULLUP);

  Serial.begin(9600);

  aLastState = digitalRead(outputA);

  // RELAY
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, HIGH);
  
  //digitalWrite(relayPin, HIGH);
  windowStartTime = millis();

  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  // turn the PID on
  myPID.SetMode(AUTOMATIC);

  // LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.setCursor(0, 1);
  lcd.print("Target: ");

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  currTime = millis(); // Updates current time
  
  updateSwitch();

  updateTemperature();

  updateEncoder();

  if (confirmed && currTime > switchTime) updateRelay();
  else digitalWrite(RelayPin, HIGH);
}

void updateSwitch () {
  if (currTime > switchTime && !digitalRead(outputSW)) {
    switchTime = currTime + 2000; // 2000 ms delay between next update
    
    confirmed = !confirmed;
  } 
  else if (switchTime > currTime) {
    if (confirmed) {
      lcd.setCursor(15, 1);
      lcd.print("!");
      Setpoint = targetTemp;
    } else {
      lcd.setCursor(15, 1);
      lcd.print("X");
    }
  } 
  else if (currTime > switchTime) {
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
}

void updateTemperature() {
  if (currTime > tempTime) { 
    tempTime = currTime + 1000; // 1000 ms delay between next update
    
    c = thermocouple.readCelsius();
  
    if (c != lastTemp) {
      lcd.setCursor(9, 0);
      lcd.print("     ");
      lcd.setCursor(9, 0);
      lcd.print(c, 1);
      lcd.print((char)223);
      lcd.print('C');
    }
  
    lastTemp = c;
  }

  if (confirmed) {
    // Do something here
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
    lcd.print("    ");
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

    // Serial.print(currentMillis);
    Serial.print(" Time: ");
    Serial.print(currentMillis);
    Serial.print(" Temp: ");
    Serial.print(c);
    Serial.print(" Set Point: ");
    Serial.print(Setpoint);
    Serial.println();
  }

  Input = c;
  myPID.Compute();


  /************************************************
     turn the output pin on/off based on pid output
   ************************************************/
  unsigned long now = millis();
  if (now - windowStartTime > WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (Output > now - windowStartTime) {
    
    digitalWrite(RelayPin, LOW);
   // Serial.print("HEAT ON    ");
  }  else {
    digitalWrite(RelayPin, HIGH);
 //   Serial.print("HEAT OFF   ");
  }
}
