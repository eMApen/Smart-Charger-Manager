#include <Arduino.h>
// OneWire and DS2438 Library include:
#include <OneWire.h>
#include <DS2438.h>

// For a connection via I2C using the Arduino Wire include:
#include <WiFiMulti.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

#include "images.h"
#include <Bounce2.h>

// For InfluxDB Data Update 
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include "password.h"


// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("wifi_status");

//------------- define the Arduino digital I/O pin to be used for the 1-Wire network here -----------
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
uint32_t updateflag = 0;
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
  button.interval(20);//weibanzi5ms
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

  // WiFi and Database Initialize Setting
  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Add tags
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }


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

    break;
  case 1:

    display.clear();
    drawOS();
    display.setFont(ArialMT_Plain_10);
    display.display();

  counter++;
  break;

  case 3:
    ds2438.update();
    if (ds2438.isError()) {
        Serial.println("Error reading from DS2438 device");
    } else {
        // Serial.print("Temperature = ");
        // Serial.print(ds2438.getTemperature(), 1);
        Temp = ds2438.getTemperature();
        // Serial.print("°C, Voltage= ");
        // Serial.print(ds2438.getVoltage(DS2438_CHA), 1);
        Volt = ds2438.getVoltage(DS2438_CHA);
        // Serial.print("V, Current = ");
        // Serial.print(ds2438.getCurrent(),1);
        Curr = ds2438.getCurrent();
        // Serial.println("mA");
    }
    // Serial.println("Case 3");
  break;

  case 4:
    // Serial.println("Case 4");
      // Clear fields for reusing the point. Tags will remain untouched
      if(updateflag == 1000)
      {
        updateflag == 0;
        sensor.clearFields();
        // Store measured value into point
        // Report RSSI of currently connected network
        sensor.addField("rssi", WiFi.RSSI());

        sensor.addField("Temprature",Temp);
        sensor.addField("Voltage",Volt);
        sensor.addField("Current",Curr);
        // Print what are we exactly writing
        Serial.print("Writing: ");
        Serial.println(sensor.toLineProtocol());

        // Check WiFi connection and reconnect if needed
        if (wifiMulti.run() != WL_CONNECTED) {
          Serial.println("Wifi connection lost");
        }

        // Write point
        if (!client.writePoint(sensor)) {
          Serial.print("InfluxDB write failed: ");
          Serial.println(client.getLastErrorMessage());
        }

        Serial.println("Wait 10s");

      }
      else{
        updateflag++;
      }
    
  break;
  default:
    break;
  }
  
  delay(100);
}