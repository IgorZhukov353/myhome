/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  21-06-2024
*/

#include "Arduino.h"
#include "esp_wifi.h"

//--------------------------------------------------
// внешние функции
void trace(const String& msg);
void responseProcessing(const String& resp);
void esp_power_switch(bool p);
//--------------------------------------------------
/* пример POST запроса
POST /upd/send_info.php HTTP/1.1
Host: f0195241.xsph.ru
Content-Type: application/x-www-form-urlencoded
Authorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==

str=[{"type":"S","id":1,"v":1},{"type":"T","id":1,"temp":12,"hum":80},{"type":"E","text":"hello, world!"},{"type":"E","id":6,"text":"test 6"}]

*/     
  
//const String WSSID = "TP-LINK_B3D2";    // Нахим
//const String WSSID = "TP-LINK_FA82EC";    // Дом
const String WSSID = "Keenetic-7832";
//const String WSSID_PROG = "WIFI353";    // программный WIFI на mini-pc
const String WPASS  = "tgbvgy789";
//const String HOST_STR = "igorzhukov353.000webhostapp.com";
//const String HOST_STR = "24683.databor.pw";
//const String HOST_STR = "z916629e.beget.tech"; // 16-03-2018 очередной бесплатный хостинг
//const String HOST_STR = "santalov.ru";
//const String HOST_STR = "f0195241.xsph.ru";
const char *HOST_STR = "igorzhukov353.h1n.ru"; 
const char *HOST_IP_STR = "81.90.182.128"; 
//const char *HOST_IP_STR = "igorzhukov353.h1n.ru";

const char *ok_str = (char*)"OK";

#define ESP_Serial Serial1 // для МЕГИ
//String curWSSID;

//------------------------------------------------------------------------
ESP_WIFI::ESP_WIFI()
{
wifi_initialized = false;
sendErrorCounter = 0;
//curWSSID = WSSID_PROG;
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::send2site(const String& reqStr) 
{
  return _send2site(reqStr, "");
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::_send2site(const String& reqStr, const String& postBuf) 
{
	if(!checkInitialized()){
			return false;
		}

  String cmd1 = "AT+CIPSTART=\"TCP\",\"" + String(HOST_IP_STR) +"\",80";
  String request = (postBuf == "")? 
    "GET /"  + reqStr + " HTTP/1.1\r\nHost: " + String(HOST_STR) + "\r\nConnection: close\r\n" :
    "POST /" + reqStr + " HTTP/1.1\r\nHost: " + String(HOST_STR) + "\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\nCache-Control: no-cache\r\nContent-Length: "  + postBuf.length() + "\r\n\r\n" + postBuf + "\r\n";    
//    "GET /" + reqStr + "?" + postBuf + " HTTP/1.1\r\nHost: " + HOST_STR + "\r\nConnection: close\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\n";

  short requestLength = request.length() + 2; // add 2 because \r\n will be appended by Serial.println().
  String cmd2 = "AT+CIPSEND=" + String(requestLength);

  bool r;
  for(short i=0; i<3; i++){
      r = espSendCommand( cmd1 , ok_str , 15000 );  // установить соединение с хостом (3 попытки)
      if(!r){
        delay(1000);
        continue;
      }
      r = espSendCommand( cmd2 , ok_str , 5000 );             // подготовить отсылку запроса - длина запроса
      bytesSended += requestLength;
      r = espSendCommand( request , (char*)"CLOSED" , 15000 );  // отослать запрос и получить ответ
      if(!r){
        delay(1000);
        continue;
      }
      
      //espSendCommand("AT+CIPCLOSE", ok_str , 5000 );
      break;
  }
  if(!r){
    sendErrorCounter++;   // счетчик ошибочных отправок (для определения проблемы доступа к интернету - возможно нужно перезагрузить роутер)
    sendErrorCounter_ForAll++;
  }
  else
    sendErrorCounter = 0;

  sendCounter_ForAll++;  
  lastWIFISended = millis();
  return r;
}

//------------------------------------------------------------------------
bool ESP_WIFI::espSerialSetup() 
{
  bool r;
esp_power_switch(true);
delay(200);

ESP_Serial.begin(115200); // default baud rate for ESP8266
delay(100);
r = espSendCommand(F("ATE0") , ok_str , 5000 );

delay(100); // Without this delay, sometimes, the program will not start until Serial Monitor is connected
r = espSendCommand( F("AT+CIFSR") , ok_str , 5000 );
r = espSendCommand( F("AT+CWMODE=1") , ok_str , 5000 );
r = espSendCommand( String(F("AT+CWJAP=\"")) + WSSID + String(F("\",\"")) + WPASS + String(F("\"")) , ok_str , 20000 );
if(!r){
  routerConnectErrorCounter++; 
}
else
  routerConnectErrorCounter = 0;
return r;
}

//---------------------------------------------------------------------
bool ESP_WIFI::espSendCommand(const String& cmd, char* goodResponse, unsigned long timeout) 
{
trace("espSendCommand( " + cmd + " , " + goodResponse + " , " + String(timeout) + " )" );
ESP_Serial.println(cmd);
unsigned long tnow, tstart;
tnow = tstart = millis();
String response;
response.reserve(255);
#define BUF_SIZE 11
char c, cbuffer[BUF_SIZE];  //для отладки = {'*','*','*','*','*','*','*','*','*','*'};
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
    //for(short i=0; i<sizeof(cbuffer)-1; i++)
      //cbuffer[i] = cbuffer[i+1];
      
    memmove(cbuffer,cbuffer + 1, BUF_SIZE - 2);  
    cbuffer[sizeof(cbuffer) - 2] = c;

    if(!memcmp(cbuffer + ((sizeof(cbuffer) - 1) - len),goodResponse,len)){
      trace("espSendCommand: SUCCESS - Response time: " + String(millis() - tstart) + "ms.");
      short res = true;
      short http;
      while(ESP_Serial.available()){
          c = ESP_Serial.read();
          response += String(c);
/*          
          memmove(cbuffer,cbuffer + 1, BUF_SIZE - 2);
          cbuffer[BUF_SIZE - 2] = c;
          //trace("cbuffer={" + String(cbuffer) + "}");
          if(!memcmp(cbuffer + ((sizeof(cbuffer) - 1) - 7),":HTTP/1",7)){
            http = true;
            res = false;
          }
          if(http && !memcmp(cbuffer + ((sizeof(cbuffer) - 1) - 7)," 200 OK",7)){
            res = true;
          }
*/          
        }
      http = response.indexOf(F(":HTTP/1"));
      if(http>0){
        res = response.indexOf(F("200 OK"),http);
        res = (res>0)?1:0;
      }
      trace("http=" + String(http) + " res=" + String(res) + F(" RESPONSE: ") + response + F("\n\r---END RESPONSE---"));
      if( res)
        responseProcessing(response);
      return res;
      }
    else
      if(!memcmp(cbuffer + 5,"ERROR",5)){
        trace(String(F("espSendCommand: ERROR - Response time: ")) + String(millis() - tstart) + "ms.");
        while(ESP_Serial.available()){
            c = ESP_Serial.read();
            response += String(c);
          }
        trace(String(F("RESPONSE: ")) + response + String(F("\n\r---END RESPONSE---")));
        return false;
      }  
    }
    tnow = millis();
  }
}

//------------------------------------------------------------------------
void ESP_WIFI::checkIdle() // отключение в случае простоя
{
  return; // izh 23-05-2020 отключено
  
  if(wifi_initialized && ((millis() - lastWIFISended) > (60000*2))){ // если нет передачи >= 2 мин то отключить
    closeConnect();
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
void ESP_WIFI::addInfo2Buffer(const String& str)
{
  if(buffer.length() == 0) 
    buffer = str; 
  else 
    buffer += "," + str;
  //sendBuffer2Site();   
}

//------------------------------------------------------------------------
void ESP_WIFI::addEvent2Buffer(short id, const String& msgText)
{
  if(msgText.startsWith("{"))
    addInfo2Buffer(String(F("{\"type\":\"E\",\"id\":")) + String(id) + String(F(",\"text\":")) + msgText + String(F("}")));  
  else
    addInfo2Buffer(String(F("{\"type\":\"E\",\"id\":")) + String(id) + String(F(",\"text\":\"")) + msgText + String(F("\"}")));  
}

//------------------------------------------------------------------------
void ESP_WIFI::addTempHum2Buffer(short id, short temp, short hum)
{
  addInfo2Buffer(String(F("{\"type\":\"T\",\"id\":")) + String(id) + String(F(",\"temp\":")) + String(temp) + String(F(",\"hum\":")) + String(hum) + String(F("}")));  
}

//------------------------------------------------------------------------
void ESP_WIFI::addSens2Buffer(short id, short val)
{
  addInfo2Buffer(String(F("{\"type\":\"S\",\"id\":")) + String(id) + String(F(",\"v\":")) + String(val) + String(F("}")));  
}

//------------------------------------------------------------------------
void ESP_WIFI::sendBuffer2Site()
{
  if(buffer.length() == 0)
    return;
  if(_send2site(String(F("upd/send_info.php")), String(F("str=[")) + buffer + String(F("]"))))
    buffer = "";
    
  buffer = "";  
}

//------------------------------------------------------------------------
void ESP_WIFI::check_Wait_Internet()
{
   if(!checkInitialized())
    return;
   trace(F("check_Wait_Internet ...")); 
   unsigned long tstart, tnow, timeout = 1000 * 60 * 2; // izh 28-10-2018 таймаут 2 мин или до появления пинга
   tnow, tstart = millis();
   while(tnow < tstart + timeout ){
    if(espSendCommand(String(F("AT+PING=\""))+ String(HOST_IP_STR) +"\"" , (char*)"OK" , 5000 ))
      break;
    tnow = millis();
    delay(10000);
    }
}

//------------------------------------------------------------------------
void ESP_WIFI::closeConnect()
{
  if(wifi_initialized){
    espSendCommand(F("AT+CWQAP") , ok_str, 5000 );
    wifi_initialized = false;
    esp_power_switch(false);
  }  
}

//------------------------------------------------------------------------
bool ESP_WIFI::sendError_check()
{
  trace( "WSSID=" + WSSID + " SErr=" + String(sendErrorCounter) + " RCErr=" + String(routerConnectErrorCounter) + " DNSErr=" + String(dnsFailCounter));
  bool res = true;
  if(sendErrorCounter > 3)
    res = false;
  else  
    res = espSendCommand( F("AT+PING=\"192.168.8.1\"") , (char*)"OK" , 5000); // попытка пингануть модем    
    
  if(!res){
    res = espSendCommand(String(F("AT+PING=\"")) + String(HOST_IP_STR) +"\"" , (char*)"OK" , 15000); // попытка пингануть свой сервер
    if(res){ // все наладилось
        sendErrorCounter = 0;
        dnsFail = 0;
        return 0;
        }
    res = espSendCommand(F("AT+PING=\"192.168.0.1\"") , (char*)"OK" , 5000); // попытка пингануть роутер
    if(res || millis() - lastRouterReboot > (60000 * 60) ){ // если роутер жив, то проблема с доступом в Инет, если нет - пропал WIFI (но не чаще чем в 1 час) - перегрузить роутер
      closeConnect(); // izh 22-05-2020 отключить от WIFI
      lastRouterReboot = millis();
      routerRebootCount++;
      return 1;
      }
    }
  return 0;
}
