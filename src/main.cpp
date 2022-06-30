#include <Arduino.h>
#include <OneWire.h>
#include <DS2438.h>

// For a connection via I2C using the Arduino Wire include:
#include "WiFi.h"
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"


#include "images.h"
#include <Bounce2.h>
// define the Arduino digital I/O pin to be used for the 1-Wire network here
const uint8_t ONE_WIRE_PIN = 18;

// define the 1-Wire address of the DS2438 battery monitor here (lsb first)
uint8_t DS2438_address[] = { 0x26, 0xc6, 0x03, 0x41, 0x01, 0x00, 0x00, 0xc6 };

OneWire ow(ONE_WIRE_PIN);
DS2438 ds2438(&ow, DS2438_address);

SSD1306Wire display(0x3c, SDA, SCL);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
OneWire net(18);  // on pin 18


#define BUTTON_PIN 23

#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 1;
int led_status = 0;
int LED = 2;
int card = 0;

int CHARGE = 0;
float Temp,Volt,Curr=0;
char Ts[20],Vs[20],Cs[20];
Bounce2::Button button = Bounce2::Button();//实例化一个抖动对象

/**********************************
 *********** Function**************
***********************************
*/
void buttonInterrupt(){
  CHARGE = ~CHARGE;
  Serial.println("Button!");

}
void drawOS(){
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  sprintf(Ts,"BAT = %.0f \%",100*(Volt-3.0)/1.1);
  Ts[10]='%';
  display.drawString(64, 26, Ts);
  sprintf(Cs,"I = %.1f mA",Curr);
  display.drawString(64, 37, Cs);
  sprintf(Vs,"V = %.2f V",Volt);
  display.drawString(64, 48, Vs);

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
  button.attach(BUTTON_PIN, INPUT);
  button.interval(20);//间隔是5ms
  //Charge Control
  pinMode(19,OUTPUT);
  
  
  //UART
  Serial.begin(115200);
  Serial.println();
  Serial.println("Hey Jenny");

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // attachInterrupt(digitalPinToInterrupt(button), buttonInterrupt, FALLING);
  

  //DS2438
  ds2438.begin();
}


/**********************************
 ************* Loop ***************
***********************************
*/

long timeSinceLastModeSwitch = 0;

void loop() {
  // Button 
  button.update();//更新
  if ( button.pressed() ) {
    CHARGE = ~CHARGE;
    digitalWrite(19,CHARGE);
    Serial.print(CHARGE);
    Serial.println("Button!");
  }

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
    // Serial.println("Case 0");
    /* code */
    break;
  case 1:
    // Serial.println("Case 1");
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
    ds2438.update();
    if (ds2438.isError()) {
        Serial.println("Error reading from DS2438 device");
    } else {
        Serial.print("Temperature = ");
        Serial.print(ds2438.getTemperature(), 1);
        Temp = ds2438.getTemperature();
        Serial.print("°C, Voltage= ");
        Serial.print(ds2438.getVoltage(DS2438_CHA), 1);
        Volt = ds2438.getVoltage(DS2438_CHA);
        Serial.print("V, Current = ");
        Serial.print(ds2438.getCurrent(),1);
        Curr = ds2438.getCurrent();
        Serial.println("mA");
    }
    // Serial.println("Case 3");
  break;

  case 4:
    // Serial.println("Case 4");
  break;
  default:
    break;
  }
  
  // delay(100);
}