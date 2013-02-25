#include "Arduino.h"

#include "LiquidCrystal.h"
#include "ace.h"

LiquidCrystal lcd(8, 9, 4, 5, 6, 11);

#define ERR_PIN 3

void ace_event(message_event e, int16_t size, const uint8_t* const message) {

  if (e == 0 || e == 1) {
    digitalWrite(ERR_PIN, LOW);
    lcd.clear();
    for(int i = 0; i < size; ++i) {
      lcd.write(message[i]);
    } 
  }
  else {
    digitalWrite(ERR_PIN, HIGH);
    lcd.clear();
    lcd.print(e);
  }
}

void setup() {
  init_ace(115200, 20);
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("ready");
  pinMode(ERR_PIN, OUTPUT);
}

uint8_t i = 0;
void loop() {
  
  lcd.clear();
  lcd.print("|");

  
  delay(1000);
  ace_send0(1, &i);
  delay(1000);
}
