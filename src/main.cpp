// For a connection via I2C using the Arduino Wire include:
#include "WiFi.h"
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

#include "images.h"

SSD1306Wire display(0x3c, SDA, SCL);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h



#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 1;
int led_status = 0;
int LED = 2;
int card = 0;
int button = 23;
bool CHARGE = true;
/**********************************
 *********** Function**************
***********************************
*/
void buttonInterrupt(){
  CHARGE = ~CHARGE;

}
void drawOS(){
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 26, "BAT = 70%");
  display.drawString(64, 37, "I = 10mA");
  display.drawString(64, 48, "V = 3.7V");

  if(CHARGE){
    display.drawString(64, 15, "Charging");
  }
  else{
    display.drawString(64,15,"NOT Charging");
  }
  

}


/**********************************
 ************ Setup ***************
***********************************
*/
void setup() {

  //LED
  pinMode(LED,OUTPUT);
  pinMode(button , INPUT);

  //UART
  Serial.begin(115200);
  Serial.println();
  Serial.println("Hey Jenny");

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  attachInterrupt(digitalPinToInterrupt(button), buttonInterrupt, FALLING);

}


/**********************************
 ************* Loop ***************
***********************************
*/

long timeSinceLastModeSwitch = 0;

void loop() {
  if(card<5){
    card++;
  }
  else{
    card = 0;
  }

  switch (card)
  {
  case 0:
    led_status = ~led_status;
    digitalWrite(LED,led_status);
    /* code */
    break;
  case 1:
    // clear the display
    display.clear();
    // draw the current demo method
    drawOS();

    display.setFont(ArialMT_Plain_10);
    // display.setTextAlignment(TEXT_ALIGN_RIGHT);
    // display.drawString(128, 54, String(millis()));
    // write the buffer to the display
    display.display();

  //   if (millis() - timeSinceLastModeSwitch > DEMO_DURATION) {
  //     demoMode = (demoMode + 1)  % demoLength;
  //     timeSinceLastModeSwitch = millis();
  // }
  counter++;
  break;

  case 3:

  break;

  case 4:

  break;
  default:
    break;
  }
  
  delay(100);
}