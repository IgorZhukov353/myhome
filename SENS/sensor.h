/* 
 Igor Zhukov (c)
 Created:       01-03-2024
 Last changed:  10-03-2024
*/

//#include <Wire.h>
#include <SD.h>
#include <dht.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

//---------------------------------------------------------------------------
enum _Type {_dht=1, _dallas=2, _pin=4, _analogPin=8, _pir=0x10, _ping=0x20, _led=0x40, _group=0x80};

class Sensor{
public:
byte id,pin;
short value,preValue;
_Type type;
int virtual check()=0;
void virtual init(byte _id,_Type _type,byte _ppin,JsonObject root){id =_id;type = _type; pin = _ppin;preValue=-100;}
};
//---------------------------------------------------------------------------
class Activity {
public:
unsigned long timeout;
unsigned long lastActivated;

int virtual actionRunFunc()=0;
int run(){
  long currentMillis = millis();
  if(currentMillis - lastActivated > timeout) {lastActivated = currentMillis; return actionRunFunc();}
  return 0;
  };
};
//---------------------------------------------------------------------------
class TempSensor : public Sensor{
  OneWire oneWire;
  DallasTemperature t1;
  dht t2;
  short humValue;

public:
TempSensor(){};
void virtual init(byte _id,_Type _type,byte _ppin,JsonObject root);
int virtual check();
};
//---------------------------------------------------------------------------
class LED : public Sensor,public Activity{
public:
void virtual init(byte _id,_Type _type,byte _ppin,JsonObject root);
int virtual check(){run();}
int virtual actionRunFunc();
};
//---------------------------------------------------------------------------
class PIN : public Sensor{
public:
short timeout;
unsigned long lastActivated;
short normval;
byte sysledoff;
void virtual init(byte _id,_Type _type,byte _ppin,JsonObject root);
int virtual check();
};
//---------------------------------------------------------------------------
class SensorArray : public Activity{
public:
short len;  
short temper_ptr, led_ptr, pin_ptr;
short groupCurPtr;
//----------------------------
void init(JsonDocument root);
int virtual actionRunFunc();
check(byte sensorMask=0, byte actMask=0);
load(String json);
};
