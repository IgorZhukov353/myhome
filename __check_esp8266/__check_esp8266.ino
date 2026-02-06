
//#define COM_BAUD 115200
//#define espSerial Serial1

#include "esp_wifi.h"
ESP_WIFI esp;  // wi-fi ESP266

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
	esp.check_Wait_Internet();
  esp._send2ya();
}
void loop() 
{
}
