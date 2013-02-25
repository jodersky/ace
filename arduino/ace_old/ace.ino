#include "link.h"
#include "physical.h"
#include "transport.h"


// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 11);

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#define LED_PIN 13
uint8_t msg_buffer[] = {97,98,99};
  message msg = {msg_buffer, 3};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  lcd.begin(16, 2);
  init_physical_layer(115200);
  init_transport_layer(200);
 
}


void step_circle() {
  static volatile uint8_t position = 0;
  static const char chars[] = {'|', '/', '-', '\\'};
  position = position + 1;
  if (position >= 4) position = 0;
 
  lcd.setCursor(15, 0);
  lcd.print(chars[position]);
}


void loop() {

  //lcd.print(freeRam());
  
  int x = analogRead (0);
  //lcd.setCursor(10,1);
  if (x < 100) {
    //lcd.print ("Right ");
  }
  else if (x < 200) {
    //lcd.print ("Up    ");
  }
  else if (x < 400){
    //lcd.print ("Down  ");
  }
  else if (x < 600){
//    lcd.print ("Raw    ");
    to_physical_layer(97);
  }
  else if (x < 800){
    lcd.setCursor(0,0);
    lcd.print ("SENDING...");
    to_transport_layer(&msg);
    lcd.print ("DONE");
  }
  else {
    
  }
  
  delay(100);

  //step_circle();
}

/*void from_physical_layer(uint8_t byte) {
 if (byte == 'A') digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}*/
/*void from_link_layer(packet* s) {
 if (s->data[0] == 'A') digitalWrite(LED_PIN, HIGH);
 else digitalWrite(LED_PIN, LOW);

};*/


static volatile uint32_t errs = 0;
void from_transport_layer(transport_code code, message* s) {
 lcd.clear();
 lcd.setCursor(0,0);
 if (code == RECEIVED) {
   lcd.print("RECEIVED");
   lcd.setCursor(0,1);
   for (int i = 0; i < s->length; ++i) lcd.write(s->data[i]);
 }
 
 if (code == SEND_SUCCESS) {
   lcd.print("SENT");
   for (int i = 0; i < s->length; ++i) lcd.write(s->data[i]);
 }
 
 if (code >= SEND_ERROR) {
   lcd.print("ERROR");
   lcd.setCursor(0,1);
   switch(code) {
     case NO_ACK:
       lcd.print("NO ACK");
       break;
     case BAD_ACK:
       lcd.print("BAD ACK");
       break;
     case BUSY:
       lcd.print("BUSY");
       break;
   }
 }
} //implemented in transport
