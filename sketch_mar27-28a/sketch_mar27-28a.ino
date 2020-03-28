#include <LiquidCrystal_I2C.h>


#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

#define outputA 6
#define outputB 7
#define outputSW 5

int relayPin = 9;

bool confirmed = false;

int target = 0;
int current = 0;
int aState;
int aLastState;

int time = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(outputSW, INPUT_PULLUP);

  // RELAY
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  Serial.begin(9600);

  aLastState = digitalRead(outputA);

  // LCD
  lcd.begin(16, 2);
  lcd.clear();
  //lcd.print("Hello, World!");

  lcd.setCursor(0, 0);
  lcd.print("Curr Temp: ");

  lcd.setCursor(0, 1);
  lcd.print("Target: ");
}

void loop() {
  // put your main code here, to run repeatedly:
  // RELAY
  digitalWrite(relayPin, LOW);
  delay(3000);
  digitalWrite(relayPin, HIGH);
  delay(3000);

  
  if (!digitalRead(outputSW)) {
    confirmed = !confirmed;

    if (confirmed) {
      lcd.setCursor(14, 1);
      lcd.print("ON");
    } else {
      lcd.setCursor(13, 1);
      lcd.print("OFF");
    }

    delay(2000);
    lcd.setCursor(13, 1);
    lcd.print("   ");
  }

  
  if (confirmed && time >= 10000) {
    if (current != target) {
      if (current < target)
        current += 1;
      else if (current > target)
        current -= 1;
    }


    lcd.setCursor(11, 0);
    lcd.print("    ");
    lcd.setCursor(11, 0);
    lcd.print(current);
    lcd.print((char)223);

    time = 0;
  } else {
    time++;
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
  }

  

  aLastState = aState;
}
