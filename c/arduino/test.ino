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
      lcd.print(message[i]);
    } 
  }
  else {
    digitalWrite(ERR_PIN, HIGH);
    lcd.clear();
    lcd.print("txerr: ");
    lcd.print(e);
  }
}

void setup() {
  init_ace(9600, 200);
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("ready");
  pinMode(ERR_PIN, OUTPUT);
}

uint8_t i = 0;
void loop() {
  delay(1000);
  i += 1;
  ace_send0(1, &i);
}
