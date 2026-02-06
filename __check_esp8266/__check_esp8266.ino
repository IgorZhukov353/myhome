
//#define COM_BAUD 115200
//#define espSerial Serial1

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

#include "esp_wifi.h"
#include "util.h"
ESP_WIFI esp;  // wi-fi ESP266

#define VERSION "Ver 1.187 of 06-02-2026 Igor Zhukov (C)"
/*
void setup() {
  espSerial.begin(COM_BAUD);
  Serial.begin(COM_BAUD);
  Serial.println("Setup");
  espSerial.write("OK");
}
void loop() {
  
  if ( espSerial.available() )
    Serial.write( espSerial.read() );
  if ( Serial.available() )
    espSerial.write( Serial.read() );

}
*/

void setup() 
{
    trace(F(VERSION));

    Wire.begin();
    RTC.begin();

	esp.check_Wait_Internet();
  esp._send2ya();
}
void loop() 
{
}
