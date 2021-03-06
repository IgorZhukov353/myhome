/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  16-03-2018
*/

#include "Arduino.h"
//#include <SoftwareSerial.h>
#include "esp_wifi.h"

const String WSSID = "TP-LINK_B3D2";    // Нахим
//const String WSSID = "TP-LINK_FA82EC";    // Дом
const String WPASS  = "tgbvgy789";
//const String HOST_STR = "igorzhukov353.000webhostapp.com";
//const String HOST_STR = "24683.databor.pw";
//const String HOST_STR = "z916629e.beget.tech"; // 16-03-2018 очередной бесплатный хостинг // не работает!
//const String HOST_STR = "santalov.ru";

const String HOST_STR = "f0195241.xsph.ru";
//const String HOST_STR = "igorzhukov353.h1n.ru"; 
char *ok_str = (char*)"OK";

//SoftwareSerial ESP_Serial(10, 11); // RX, TX
#define ESP_Serial Serial2 // для МЕГИ
//#define SIM800_Serial Serial2 // для МЕГИ

HardwareSerial *com;
/* пример запроса
POST /upd/send_info.php HTTP/1.1
Host: f0195241.xsph.ru
Content-Type: application/x-www-form-urlencoded
Authorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==

str=[{"type":"S","id":1,"v":1},{"type":"T","id":1,"temp":12,"hum":80},{"type":"E","text":"hello, world!"},{"type":"E","id":6,"text":"test 6"}]

*/     
//------------------------------------------------------------------------
ESP_WIFI::ESP_WIFI()
{
wifi_initialized = false;
sendErrorCounter = 0;
com = &ESP_Serial;
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::send2site(String reqStr, String postBuf) 
{
	if(!checkInitialized()){
			return false;
		}

  String cmd1 = "AT+CIPSTART=\"TCP\",\"" + HOST_STR +"\",80";
  String getRequest = "POST /" + reqStr + " HTTP/1.1\r\nHost: " + HOST_STR + "\r\nContent-Type: application/x-www-form-urlencoded\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\nCache-Control: no-cache\r\nConnection: close\r\nContent-Length: " + postBuf.length() + "\r\n\r\n" + postBuf + "\r\n";
  
  int getRequestLength = getRequest.length() + 2; // add 2 because \r\n will be appended by Serial.println().
  String cmd2 = "AT+CIPSEND=" + String(getRequestLength);

  int i;
  bool r;
  for(i=0; i<3; i++){
      //r = espSendCommand( cmd1 , ok_str , 15000 );  // установить соединение с хостом (3 попытки)
      r = espSendCommand( cmd1 , (char*)"CONNECT", 15000 );  // установить соединение с хостом (3 попытки)
      if(!r){
        delay(1000);
        continue;
      }
      r = espSendCommand( cmd2 , (char*)">" , 15000 );             // подготовить отсылку запроса - длина запроса
      r = espSendCommand( getRequest , (char*)"CLOSED" , 15000 );  // отослать запрос и получить ответ
      break;
  }
  if(!r)
    sendErrorCounter++;   // счетчик ошибочных отправок (для определения проблемы доступа к интернету - возможно нужно перезагрузить роутер)
  else
    sendErrorCounter = 0;
    
  lastWIFISended = millis();
  return r;
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::send2site(String reqStr) 
{
  /*
  sendErrorCounter = 0;
  lastWIFISended = millis();
  return 1; // временно, пока не наладится хостинг
  */
  
  if(!checkInitialized()){
      return false;
    }

  String cmd1 = "AT+CIPSTART=\"TCP\",\"" + HOST_STR +"\",80";
  String getRequest = "GET /" + reqStr + " HTTP/1.0\r\nHost: " + HOST_STR + "\r\n";
  int getRequestLength = getRequest.length() + 2; // add 2 because \r\n will be appended by Serial.println().
  String cmd2 = "AT+CIPSEND=" + String(getRequestLength);

  int i;
  bool r;
  for(i=0; i<3; i++){
      //r = espSendCommand( cmd1 , ok_str , 15000 );  // установить соединение с хостом (3 попытки)
      r = espSendCommand( cmd1 , (char*)"CONNECT", 15000 );  // установить соединение с хостом (3 попытки)

      if(!r){
        delay(1000);
        continue;
      }
      r = espSendCommand( cmd2 , ok_str , 5000 );             // подготовить отсылку запроса - длина запроса
      r = espSendCommand( getRequest , (char*)"CLOSED" , 15000 );  // отослать запрос и получить ответ
      break;
  }
  if(!r)
    sendErrorCounter++;   // счетчик ошибочных отправок (для определения проблемы доступа к интернету - возможно нужно перезагрузить роутер)
  else
    sendErrorCounter = 0;
    
  lastWIFISended = millis();
  return r;
}

//------------------------------------------------------------------------
bool ESP_WIFI::espSerialSetup() 
{
  bool r;
/*
//ESP_Serial.begin(115200); // default baud rate for ESP8266
ESP_Serial.begin(9600);
delay(100);
ESP_Serial.println("AT+CIOBAUD=115200");
//ESP_Serial.println("AT+CIOBAUD=19200");
delay(100);
//ESP_Serial.begin(19200);
ESP_Serial.begin(115200); 

delay(100); // Without this delay, sometimes, the program will not start until Serial Monitor is connected
r = espSendCommand( "AT+CIFSR" , ok_str , 5000 );
r = espSendCommand( "AT+CWMODE=1" , ok_str , 5000 );
r = espSendCommand( "AT+CWJAP=\""+WSSID+"\",\""+WPASS+"\"" , ok_str , 7000 );
*/

r = espSendCommand( "AT" , ok_str , 1000);
r = espSendCommand( "AT+CIPSHUT", ok_str, 1000);
r = espSendCommand( "ATE0", ok_str, 1000);
r = espSendCommand( "AT+CIPSTATUS", (char*)"IP INITIAL", 5000);
r = espSendCommand( "AT+CIPMUX=0", ok_str , 5000);
r = espSendCommand( "AT+CSTT=\"internet\",\"\",\"\"", ok_str , 5000);
r = espSendCommand( "AT+CIICR", ok_str , 20000);
espSendCommand( "AT+CIFSR", (char*)"100." , 100);


// AT+CIPSHUT

if(!r)
  routerConnectErrorCounter++; 
else
  routerConnectErrorCounter = 0;
return r;
}

//---------------------------------------------------------------------
bool ESP_WIFI::espSendCommand(String cmd, char* goodResponse, unsigned long timeout) 
{
trace("espSendCommand( " + cmd + " , " + goodResponse + " , " + String(timeout) + " )" );
ESP_Serial.println(cmd);
unsigned long tnow, tstart;
tnow = tstart = millis();
String response = "";
char c, cbuffer[11];
short len = strlen(goodResponse);
if( len > sizeof(cbuffer) - 1)
  len = sizeof(cbuffer) - 1;
cbuffer[sizeof(cbuffer) - 1] = 0;

while( true ) {
  if( tnow > tstart + timeout ) {
    trace("espSendCommand: FAILED - Timeout exceeded " + String(timeout) + " seconds" );
    if( response.length() > 0 ) {
      trace("espSendCommand: RESPONSE:" + response);
    } 
    else {
      trace("espSendCommand: NO RESPONSE");
    }
    return false;
  } 
  c = ESP_Serial.read();
  if( c >= 0 ) {
    response += String(c);
    for(short i=0; i<sizeof(cbuffer)-1; i++)
      cbuffer[i] = cbuffer[i+1];
    cbuffer[sizeof(cbuffer) - 2] = c;

    if(!memcmp(cbuffer + ((sizeof(cbuffer) - 1) - len),goodResponse,len)){
      trace("espSendCommand: SUCCESS - Response time: " + String(millis() - tstart) + "ms.");
      while(ESP_Serial.available()){
          c = ESP_Serial.read();
          response += String(c);
        }
      trace("RESPONSE: " + response + "\n\r---END RESPONSE---");
      responseProcessing(response);
      return true;
      }
    else
      if(!memcmp(cbuffer + 5,"ERROR",5)){
        trace("espSendCommand: ERROR - Response time: " + String(millis() - tstart) + "ms.");
        while(ESP_Serial.available()){
            c = ESP_Serial.read();
            response += String(c);
          }
        trace("RESPONSE: " + response + "\n\r---END RESPONSE---");
        return false;
      }  
    }
    tnow = millis();
  }
}

//------------------------------------------------------------------------
void ESP_WIFI::checkIdle() // отключение в случае простоя
{
  if(wifi_initialized && ((millis() - lastWIFISended) > (60000*2))){ // если нет передачи >= 2 мин то отключить
    espSendCommand( "AT+CWQAP" , ok_str, 5000 );
    wifi_initialized = false;
  }
}

//------------------------------------------------------------------------
bool ESP_WIFI::checkInitialized()
{
  if(!wifi_initialized){
    wifi_initialized = espSerialSetup();
  }
  return wifi_initialized;
}

//------------------------------------------------------------------------
void ESP_WIFI::addInfo2Buffer(String str)
{
  if(buffer.length() == 0) 
    buffer = str; 
  else 
    buffer += "," + str;
  trace( "Len=" + String(buffer.length()) + " " + buffer); 
}

//------------------------------------------------------------------------
void ESP_WIFI::addEvent2Buffer(short id, String msgText)
{
  addInfo2Buffer("{\"type\":\"E\",\"id\":" + String(id) + ",\"text\":\"" + msgText + "\"}");  
}

//------------------------------------------------------------------------
void ESP_WIFI::addTempHum2Buffer(short id, short temp, short hum)
{
  addInfo2Buffer("{\"type\":\"T\",\"id\":" + String(id) + ",\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}");  
}

//------------------------------------------------------------------------
void ESP_WIFI::addSens2Buffer(short id, short val)
{
  addInfo2Buffer("{\"type\":\"S\",\"id\":" + String(id) + ",\"v\":" + String(val) + "}");  
}

//------------------------------------------------------------------------
void ESP_WIFI::sendBuffer2Site()
{
  if(buffer.length() == 0)
    return;
  if(send2site("upd/send_info.php", "str=[" + buffer + "]"))
    buffer = "";
    
  buffer = "";  
}

