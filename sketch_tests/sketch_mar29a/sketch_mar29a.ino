#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_MAX31855.h"

// HEAT
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

double lastTemp = -274;
double c;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// ENCODER
#define outputA 7
#define outputB 8
#define outputSW 6

int target = 0;
int current = 0;
int aState;
int aLastState;

bool confirmed = false;

// RELAY
int relayPin = 9;

// TIMER
unsigned long currTime = 0;
unsigned long tempTime = 0;
unsigned long switchTime = 0;

void setup() {
  // put your setup code here, to run once:
  // ENCODER
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(outputSW, INPUT_PULLUP);

  Serial.begin(9600);

  aLastState = digitalRead(outputA);

  // RELAY
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  // LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.setCursor(0, 1);
  lcd.print("Target: ");
}

void loop() {
  // put your main code here, to run repeatedly:
  currTime = millis(); // Updates current time
  Serial.println(currTime);
  Serial.println((bool)currTime % 1000);
  
  /*
  // RELAY
  digitalWrite(relayPin, HIGH);
  delay(1000);
  digitalWrite(relayPin, HIGH);
  delay(1000);
  */
  
  updateSwitch();

  updateTemperature();

  updateEncoder();
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
      if (target <= 345)
        target += 5;
    } else {
      if (target >= 5)
        target -= 5;
    }
    
    lcd.setCursor(8, 1);
    lcd.print("    ");
    lcd.setCursor(8, 1);
    lcd.print(target);
    lcd.print((char)223);
    lcd.print('C');
  }

  aLastState = aState;
}
