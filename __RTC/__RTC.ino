
#include <Wire.h>
#include "RTClib.h"
 
RTC_DS1307 RTC;
 
void setup () 
{
    //digitalWrite(10, HIGH);
    
    Serial.begin(115200);
    Wire.begin();
    RTC.begin();
 //RTC.adjust(DateTime(__DATE__, __TIME__));
  if (! RTC.isrunning()) 
  {
    Serial.println("RTC is NOT running!");
    //sets the RTC to the date & time this sketch was compiled
   // RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  
 
}
 
void loop () 
{
  if( millis() > 10000)
  {
    pinMode(10, OUTPUT);
       //pinMode(state_led_pin, OUTPUT);
   //digitalWrite(state_led_pin, d.ledState);

  }
    Serial.println(millis());
    DateTime now = RTC.now();  
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    delay(1000);
}
