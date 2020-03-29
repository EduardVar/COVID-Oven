#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <SPI.h>
#include "Adafruit_MAX31855.h"

// HEAT
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

double lastTemp = -274;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

#define outputA 7
#define outputB 8
#define outputSW 6

// RELAY
int relayPin = 9;



// ENCODER
int target = 0;
int current = 0;
int aState;
int aLastState;

bool confirmed = false;

void setup() {
  // put your setup code here, to run once:
  //ENCODER
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
  //lcd.print("Hello, World!");

  lcd.setCursor(0, 0);
  lcd.print("Current: ");

  lcd.setCursor(0, 1);
  lcd.print("Target: ");
}

void loop() {
  // put your main code here, to run repeatedly:
  // TEMPERATURE
  /*
  double c = thermocouple.readCelsius();
  if (isnan(c)) {
   Serial.println("Something wrong with thermocouple!");
  } else {
   Serial.print("C = ");
   Serial.println(c);
  }
  */

  /*
  // RELAY
  digitalWrite(relayPin, HIGH);
  delay(1000);
  digitalWrite(relayPin, HIGH);
  delay(1000);
*/
  
  // SWITCH
  if (!digitalRead(outputSW)) {
    confirmed = !confirmed;

    if (confirmed) {
      lcd.setCursor(15, 1);
      lcd.print("!");
    } else {
      lcd.setCursor(15, 1);
      lcd.print("X");
    }

    delay(2000);
    lcd.setCursor(13, 1);
    lcd.print("   ");
  }
  
  if (confirmed) {
    /*if (current != target) {
      if (current < target)
        current += 1;
      else if (current > target)
        current -= 1;
      */

    double c = thermocouple.readCelsius();

    if (c != lastTemp) {
      lcd.setCursor(9, 0);
      lcd.print("     ");
      lcd.setCursor(9, 0);
      lcd.print(c, 1);
      lcd.print((char)223);
      lcd.print('C');
    }

    lastTemp = c;
    delay(1000);
  }

  aState = digitalRead(outputA);

  if (!confirmed && aState != aLastState) {
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
