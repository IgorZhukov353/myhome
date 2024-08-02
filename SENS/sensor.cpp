/* 
 Igor Zhukov (c)
 Created:       01-03-2024
 Last changed:  10-03-2024
*/

#include "sensor.h"
#include "main.h"

//---------------------------------------------------------------------------
void TempSensor::init(byte _id,_Type _type,byte _ppin,JsonObject root){
  Sensor::init(_id,_type,_ppin,root);
  if(_type & _dallas){
    oneWire.begin(_ppin);
    t1.setOneWire(&oneWire);
    t1.begin();
  }
} 
int TempSensor::check(){
  if(type & _dallas){
    t1.requestTemperaturesByIndex(0);  // Send the command to get temperatures
    value = round(t1.getTempCByIndex(0));
  }
  else {
    t2.read22(pin);
    value = round(t2.temperature);
    humValue = round(t2.humidity);
  }
  if(preValue != value){
    preValue = value;
    trace("temp id="+String(id)+" temp="+ String(value));
    return 1;
  }
  return 0;
}

//---------------------------------------------------------------------------
void LED::init(byte _id,_Type _type,byte _ppin,JsonObject root){
  Sensor::init(_id,_led,_ppin,root);
  pinMode(pin, OUTPUT);
  timeout = root["timeout"];
  if(!timeout) timeout = 1000;
  preValue = timeout;
  short v = root["sysled"];
  if(v)    sysledptr = this;
} 
int LED::actionRunFunc(){
  value = !value;         // если светодиод не горит, то зажигаем, и наоборот
  digitalWrite(pin, value);  // устанавливаем состояния выхода, чтобы включить или выключить светодиод
}

//---------------------------------------------------------------------------
void PIN::init(byte _id,_Type _type,byte _ppin,JsonObject root){
  Sensor::init(_id,_pin,_ppin,root);
  timeout = 500;  // уровень сигнала должен держаться 0,5 сек
  sysledoff = root["sysledoff"];
  normval = root["normval"];
  short v = root["pullup"]; // если подтяжка по питанию
  preValue = value = normval; 
  if(v) pinMode(pin, INPUT_PULLUP);
} 
int PIN::check(){
  long currentMillis = millis();
  short v = digitalRead(pin);
  if (v != preValue) {
      preValue = v;
      lastActivated = currentMillis;
      trace("v="+ String(v));
      }
  if (lastActivated > 0 && (currentMillis - lastActivated) > timeout) {
      value = preValue;
      lastActivated = 0;
      trace("pin id="+String(id)+" v="+ String(value));
      if(!sysledoff && sysledptr){
        sysledptr->timeout = (value == normval)? sysledptr->preValue: 200;
        }
      return 1;
    }
  return 0;
}

//---------------------------------------------------------------------------
void SensorArray::init(JsonDocument root){
  timeout = root["timeout"];
  if(!timeout) timeout = 1000L; // * 60 * 2; // 2 мин
  //trace("timeout="+ String(timeout));
}
//----------------------------
int SensorArray::actionRunFunc(){  // проверка датчиков по кругу через timeout у которых в типе есть _group
  //trace("groupCurPtr="+ String(groupCurPtr));
  for (;groupCurPtr < len; groupCurPtr++) {
    if(s[groupCurPtr]->type & _group){
      s[groupCurPtr]->check();
      break;
    }
  }
  if(++groupCurPtr >= len)
    groupCurPtr = 0;
}
//----------------------------
SensorArray::check(byte sensorMask=0, byte actMask=0){
  //trace(String(sensorMask));
  run();
  for (int i = 0; i < len; i++) {
    if(!(s[i]->type & sensorMask))
      s[i]->check();
  }
}
//----------------------------
SensorArray::load(String json){
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, json);
	if (error) {
		trace("Json() failed! Err=" + String(error.c_str()));
		return -1;
	  }

	len = temper_ptr = led_ptr = pin_ptr = 0;
	JsonArray arr = doc["sensor"];
	init(doc);

	//trace("mem15=" + String(memoryFree()));
	for (int i = 0; i < arr.size(); i++) {
		JsonObject root = arr[i];
		short notUsed = root["notused"];
		if(notUsed == 1)
		  continue;  
		short id = root["id"];
		short type = root["type"];
		short pin = root["pin"];
		short group = root["group"];
		if (!id || !type || !pin) { //
		  trace("Не определен \"id\" или \"type\" датчика!");
		  continue;
		}
		switch(type){
		case _dht:
		case _dallas:
		  s[len] = &temper[temper_ptr++];
		  break;
		case _led:
		  s[len] = &led[led_ptr++];
		  break;
		case _pin:
		case _pir:
		  s[len] = &p[pin_ptr++];
		  break;
		default: continue;
		}
		type += group * _group;
		s[len++]->init(id,type,pin,root);
	  }
	trace("sens="+ String(len));
}

