#include <SoftwareSerial.h>
//SoftwareSerial espSerial(10, 11);

#define COM_BAUD 115200
#define espSerial Serial1


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

