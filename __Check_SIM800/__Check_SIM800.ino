#include <SoftwareSerial.h>
#include "esp_wifi.h"

class esp : public ESP_WIFI{
public:  
virtual void trace(String msg);
virtual void responseProcessing(String resp);
} esp;

//SoftwareSerial espSerial(10, 11);

#define COM_BAUD 115200
#define espSerial Serial1
byte f;

// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;
 
// Функция, возвращающая количество свободного ОЗУ (RAM)
int memoryFree()
{
   int freeValue;
   if((int)__brkval == 0)
      freeValue = ((int)&freeValue) - ((int)&__bss_end);
   else
      freeValue = ((int)&freeValue) - ((int)__brkval);
   return freeValue;
}

void setup() {
  espSerial.begin(COM_BAUD);
  Serial.begin(COM_BAUD);
  esp.trace("Setup started");
return;
  //esp.send2site("upd/send_info.php","str=[{\"type\":\"T\",\"id\":1,\"temp\":99,\"hum\":10},{\"type\":\"E\",\"id\":3,\"text\":\"test 6 sdfgsdgfkmskdfg sdfgsdfgkmsdfkmg fgsndnfgjnsdfjgnjsdf jnfgjsdnfgjnsdfjgn jgnfjsdnfgjsnfdjgn jngsjnfdjgnsdfjgn jfsgndjdfngjsndfjg sgdnjdfng sndfgjfdngjndf sngjdfng sdfg jfsdgnjsdnfjg\"}]");
  
  
  esp.addEvent2Buffer(1,String(millis()));

  esp.addTempHum2Buffer(1, 10, 20);
  esp.addTempHum2Buffer(2, 10, 20);
  esp.addTempHum2Buffer(3, 10, 20);
  esp.addSens2Buffer(1, 1);
  esp.addSens2Buffer(1, 0);
  esp.addSens2Buffer(2, 1);
  esp.addSens2Buffer(2, 0);
  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");
  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");
//  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");
//  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");
//  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");
//  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");
  
  esp.sendBuffer2Site();
  esp.trace("send 1");
  
  esp.addTempHum2Buffer(1, 10, 20);
  esp.addTempHum2Buffer(2, 10, 20);
  esp.addTempHum2Buffer(3, 10, 20);
  esp.addSens2Buffer(1, 1);
  esp.addSens2Buffer(1, 0);
  esp.addSens2Buffer(2, 1);
  esp.addSens2Buffer(2, 0);
  esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");

  esp.sendBuffer2Site();
  esp.trace("send 2");

delay(10000);

  esp.addTempHum2Buffer(1, 10, 20);
  esp.addTempHum2Buffer(2, 10, 20);
  esp.addTempHum2Buffer(3, 10, 20);
  esp.addSens2Buffer(1, 1);
  esp.addSens2Buffer(1, 0);
  esp.addSens2Buffer(2, 1);
  esp.addSens2Buffer(2, 0);
  //esp.addEvent2Buffer(3,"left join Sensor_Activity as sa2 on sa2.Sensor_Activity_ID = (select Sensor_Activity_ID from Sensor_Activity WHERE Sensor_ID=sa1.sensor_id and Date > sa1.date order by Date limit 1) and sa2.Value ");

  esp.sendBuffer2Site();
  esp.trace("send 3");
  

  esp.trace("Setup done");
}

void loop() {
  
  if ( espSerial.available() )
    Serial.write( espSerial.read() );
  if ( Serial.available() )
    espSerial.write( Serial.read() );

}

//------------------------------------------------------------------------
void esp::trace(String msg)
{
  Serial.println( String(memoryFree()) + "=>" + msg );
}

//------------------------------------------------------------------------
void esp::responseProcessing(String response)
{
}

