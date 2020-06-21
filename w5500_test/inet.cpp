/* 
 Igor Zhukov (c)
 Created:       21-06-2020
 Last changed:  21-06-2020
*/

#include "Arduino.h"
#include "inet.h"
#include "w5500.h"

//--------------------------------------------------
// внешние функции
void trace(String msg);
void responseProcessing(String resp);
void esp_power_switch(bool p);
//--------------------------------------------------

const char *HOST_STR = "igorzhukov353.h1n.ru"; 
const char *HOST_IP_STR = "145.239.233.78"; 
const char *ok_str = (char*)"OK";

w5500_sender w55;

//------------------------------------------------------------------------
inet::inet()
{
initialized = false;
sendErrorCounter = 0;
sender = (inet_sender *)&w55;
}

//------------------------------------------------------------------------
bool inet::setup()
{
	return sender->init();
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере 
bool inet::send2site(String reqStr) 
{
  return _send2site(reqStr, "");
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере 
bool inet::_send2site(String reqStr, String postBuf) 
{
	if(!checkInitialized()){
		return false;
		}
  if(!seneder->connect(HOST_IP_STR, 80)){
    return false;
    }
   
  String request = (postBuf == "")? 
  "GET /"  + reqStr + " HTTP/1.1\r\nHost: " + String(HOST_STR) + "\r\nConnection: close\r\n" :
  "POST /" + reqStr + " HTTP/1.1\r\nHost: " + String(HOST_STR) + "\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\nCache-Control: no-cache\r\nContent-Length: "  + postBuf.length() + "\r\n\r\n" + postBuf + "\r\n";    
 
	return true;
}


//---------------------------------------------------------------------
bool inet::sendCommand(String cmd, char* goodResponse, unsigned long timeout) 
{
}

//------------------------------------------------------------------------
void inet::checkIdle() // отключение в случае простоя
{
}

//------------------------------------------------------------------------
bool inet::checkInitialized()
{
  if(!initialized){
    initialized = setup();
  }
  return initialized;
}

//------------------------------------------------------------------------
void inet::addInfo2Buffer(String str)
{
  if(buffer.length() == 0) 
    buffer = str; 
  else 
    buffer += "," + str;
}

//------------------------------------------------------------------------
void inet::addEvent2Buffer(short id, String msgText)
{
  addInfo2Buffer("{\"type\":\"E\",\"id\":" + String(id) + ",\"text\":\"" + msgText + "\"}");  
}

//------------------------------------------------------------------------
void inet::addTempHum2Buffer(short id, short temp, short hum)
{
  addInfo2Buffer("{\"type\":\"T\",\"id\":" + String(id) + ",\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}");  
}

//------------------------------------------------------------------------
void inet::addSens2Buffer(short id, short val)
{
  addInfo2Buffer("{\"type\":\"S\",\"id\":" + String(id) + ",\"v\":" + String(val) + "}");  
}

//------------------------------------------------------------------------
void inet::sendBuffer2Site()
{
  if(buffer.length() == 0)
    return;
  if(_send2site("upd/send_info.php", "str=[" + buffer + "]"))
    buffer = "";
    
  buffer = "";  
}

//------------------------------------------------------------------------
void inet::check_Wait_Internet()
{
   if(!checkInitialized())
    return;
   trace("check_Wait_Internet ..."); 
   unsigned long tstart, tnow, timeout = 1000 * 60 * 2; // izh 28-10-2018 таймаут 2 мин или до появления пинга
   tnow, tstart = millis();
   while(tnow < tstart + timeout ){
    if(sender->ping(HOST_IP_STR))
      break;
    tnow = millis();
    delay(10000);
    }
}
