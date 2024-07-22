/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  22-07-2024
*/

#include "Arduino.h"
#include "esp_wifi.h"

//--------------------------------------------------
// внешние функции
void trace(const String& msg);
void responseProcessing(const String& resp);
void esp_power_switch(bool p);
int checkMemoryFree();
String getCurrentDate(byte noYear = 1);
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

//const char *OK = OK;

#define ESP_Serial Serial1 // для МЕГИ

//------------------------------------------------------------------------
ESP_WIFI::ESP_WIFI()
{
wifi_initialized = false;
sendErrorCounter = 0;
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

  String cmd1; 
  cmd1 = F("AT+CIPSTART=\"TCP\",\"");
  cmd1 += String(HOST_IP_STR);
  cmd1 += F("\",80");
  String request;
  short maxlen = reqStr.length() + postBuf.length()+ 200;
  if(maxlen > 1024)
    maxlen = 1024;
  request.reserve(maxlen);
  
  if(postBuf == ""){
    request = F("GET /");
    request += reqStr; 
    request += F(" HTTP/1.1\r\nHost: ");
    request += String(HOST_STR);
    request += F("\r\nConnection: close\r\n");
    }
  else {
    request = F("POST /");
    request += reqStr; 
    request += F(" HTTP/1.1\r\nHost: ");
    request += String(HOST_STR);
    request += F( "\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\nCache-Control: no-cache\r\nContent-Length: ");
    request += String(postBuf.length());
    request += F("\r\n\r\n");
    request += postBuf;
    request += F( "\r\n");    
    }

  short requestLength = request.length() + 2; // add 2 because \r\n will be appended by Serial.println().
  String cmd2 = F("AT+CIPSEND=");
  cmd2 += String(requestLength);

  bool r;
  for(short i=0; i<3; i++){
      r = espSendCommand( cmd1, STATE::OK , 15000 );  // установить соединение с хостом (3 попытки)
      if(!r){
        delay(1000);
        continue;
      }
      r = espSendCommand( cmd2, STATE::OK , 5000 );             // подготовить отсылку запроса - длина запроса
      bytesSended += requestLength;
      r = espSendCommand( request , STATE::CLOSED , 15000 );  // отослать запрос и получить ответ
      /*if(!r){
        delay(1000);
        continue;
      }*/
      
      //espSendCommand("AT+CIPCLOSE"STATE::OK , 5000 );
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
r = espSendCommand(F("ATE0"), STATE::OK , 5000 );

delay(100); // Without this delay, sometimes, the program will not start until Serial Monitor is connected
r = espSendCommand( F("AT+CIFSR"), STATE::OK, 5000 );
r = espSendCommand( F("AT+CWMODE=1"), STATE::OK, 5000 );
{
  String str = F("AT+CWJAP=\""); 
  str += WSSID;
  str += F("\",\"");
  str += WPASS;
  str += F("\"");
  r = espSendCommand(str, STATE::OK, 20000);  
  //r = espSendCommand( String(F("AT+CWJAP=\"")) + WSSID + String(F("\",\"")) + WPASS + String(F("\"")), STATE::OK , 20000 );
}
if(!r){
  routerConnectErrorCounter++; 
}
else
  routerConnectErrorCounter = 0;
return r;
}

//------------------------------------------------------------------------
#define BUF_SIZE 10
#define STATE_STR_MAX 5
static const char *state_str[STATE_STR_MAX] = {"OK", "ERROR", "HTTP/1.1", "200 OK", "CLOSED"};
static const byte state_str_len[STATE_STR_MAX] = {2, 5, 8, 6, 6};

bool ESP_WIFI::espSendCommand(const String& cmd, const STATE goodResponse, const unsigned long timeout)
{
  {
    String str = F("espSendCommand(\"");
    str += cmd;
    str += F("\",");
    str += (byte)goodResponse;
    str += F(",");
    str += timeout;
    str += F(")");
    trace(str);
  }
  ESP_Serial.println(cmd);
  if(maxSendedMSG < cmd.length())
    maxSendedMSG = cmd.length();
    
  unsigned long tnow, tstart;
  bool result;
  tnow = tstart = millis();
  String response;
  if(goodResponse == STATE::CLOSED)
    response.reserve(512);
  else
    response.reserve(100);
  char c;
  char cbuffer[BUF_SIZE]; //= {'*','*','*','*','*','*','*','*','*','*'};
  bool state_str_on[STATE_STR_MAX] = {0, 0, 0, 0, 0};
  byte recived = 0;
  while ( tnow <= tstart + timeout ) {
    c = ESP_Serial.read();
    if(c > 0) {
      response += c; //String(c);
      //Serial.println(String(cbuffer));
      if (!state_str_on[(byte)STATE::ERR] && !state_str_on[(byte)STATE::CLOSED]) {
        memmove(cbuffer, cbuffer + 1, sizeof(cbuffer) - 1);
        cbuffer[sizeof(cbuffer) - 1] = c;
        for (byte i = 0; i < STATE_STR_MAX; i++) {
          if ((i == (byte)STATE::HTTP_OK || i == (byte)STATE::CLOSED) && !state_str_on[(byte)STATE::HTTP])
            continue;
          if (!memcmp(cbuffer + sizeof(cbuffer) - 1 - state_str_len[i], state_str[i], state_str_len[i])) {
            state_str_on[i] = true;
            if(i == (byte)goodResponse || i == (byte)STATE::ERR)
              recived = true;
          }
        }
      }
    }
    if(recived)
      break;
    tnow = millis();
  }
  //Serial.println(String(state_str_on[0]) + String(state_str_on[1]) + String(state_str_on[2]) + String(state_str_on[3]) + String(state_str_on[4]));
  
  while (ESP_Serial.available()) {
    c = ESP_Serial.read();
    response += c; //String(c);
  }
  {
	  String msg = F("espSendCommand:");
	  if ( recived) {
		bool httpFail;
		if(state_str_on[(byte)STATE::HTTP] && !state_str_on[(byte)STATE::HTTP_OK]){
		  httpFail = true;
		  httpFailCounter++;
		}
		else 
		  httpFail = false;
		  
		result = (state_str_on[(byte)STATE::ERR] || httpFail) ? false : true;
		msg += (result) ? F("SUCCESS") : F("ERROR");
	  }
	  else {
		result = false;
		msg += F("ERROR - Timeout");
	  }
	  msg += F(" - Response time: " );
	  msg += String(millis() - tstart);
	  msg += F("ms.");

	  msg += F("\n\rRESPONSE:");
	  msg += response;
	  msg += F("\n\r---END RESPONSE---");
	  trace(msg);
  }
  if(result)
    responseProcessing(response);
    
  checkMemoryFree();
  return result;
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
}

//------------------------------------------------------------------------
void ESP_WIFI::addEvent2Buffer(short id, const String& msgText)
{
  String str;
  bool is_json = (msgText[0] == '{')?1:0;
  str += F("{\"type\":\"E\",\"id\":");
  str += id;
  str += F(",\"text\":");
  if(!is_json)
    str += "\"";
  str += msgText;
  if(!is_json)
    str += "\"";
  str += F(",\"date\":\"");
  str += getCurrentDate(0);
  str += F("\"}");
  addInfo2Buffer(str);  
}

//------------------------------------------------------------------------
void ESP_WIFI::addTempHum2Buffer(short id, short temp, short hum)
{
  String str;
  str  = F("{\"type\":\"T\",\"id\":");
  str += id;
  str += F(",\"temp\":"); 
  str += temp;
  str += F(",\"hum\":");
  str += hum;
  str += F(",\"date\":\"");
  str += getCurrentDate(0);
  str += F("\"}");
  addInfo2Buffer(str);  
}

//------------------------------------------------------------------------
void ESP_WIFI::addSens2Buffer(short id, short val)
{
  String str;
  str  = F("{\"type\":\"S\",\"id\":");
  str += id;
  str += F(",\"v\":"); 
  str += val;
  str += F(",\"date\":\"");
  str += getCurrentDate(0);
  str += F("\"}");
  addInfo2Buffer(str);  
}

//------------------------------------------------------------------------
void ESP_WIFI::sendBuffer2Site()
{
  if(buffer.length() == 0)
    return;
  if(_send2site(String(F("upd/send_info.php")), String(F("str=[")) + buffer + String(F("]"))))
    buffer = "";
    
  //buffer = "";  
}

//------------------------------------------------------------------------
bool ESP_WIFI::check_Wait_Internet()
{
   if(!checkInitialized())
    return 0;
   bool result = false;
   trace(F("check_Wait_Internet ...")); 
   unsigned long tstart, tnow, timeout = 1000L * 60 * 2; // izh 28-10-2018 таймаут 2 мин или до появления пинга
   tnow = tstart = millis();
   while(tnow < tstart + timeout ){
    String str = F("AT+PING=\"");
    str += HOST_IP_STR;
    str += F("\"");
    result = espSendCommand(str, STATE::OK , 15000); // попытка пингануть свой сервер
    if(result){
      break;
    }
    tnow = millis();
    delay(10000);
    }
    return result;
}

//------------------------------------------------------------------------
void ESP_WIFI::closeConnect()
{
  if(wifi_initialized){
    espSendCommand(F("AT+CWQAP"), STATE::OK, 5000 );
    wifi_initialized = false;
    esp_power_switch(false);
  }  
}

//------------------------------------------------------------------------
bool ESP_WIFI::sendError_check()
{
  {
    String str = F(" SErr=");
    str += sendErrorCounter; //String(sendErrorCounter);
    str += F(" RCErr=");
    str += routerConnectErrorCounter;
    str += F(" httpErr=");
    str += httpFailCounter;
    str += F(" MsgLen=");
    str += maxSendedMSG;
    str += F(" mem=");
    str += checkMemoryFree();
    trace(str);
  }
  bool res = true;
  if(sendErrorCounter > 3)
    res = false;
  else  
    res = espSendCommand( F("AT+PING=\"192.168.8.1\""), STATE::OK , 5000); // попытка пингануть модем    
    
  if(!res){
    {
    String str = F("AT+PING=\"");
    str += HOST_IP_STR;
    str += F("\"");
    res = espSendCommand(str, STATE::OK , 15000); // попытка пингануть свой сервер
    }
    if(res){ // все наладилось
        sendErrorCounter = 0;
        return 0;
        }
    res = espSendCommand(F("AT+PING=\"192.168.0.1\""), STATE::OK , 5000); // попытка пингануть роутер
    if(res || millis() - lastRouterReboot > (60000 * 60) ){ // если роутер жив, то проблема с доступом в Инет, если нет - пропал WIFI (но не чаще чем в 1 час) - перегрузить роутер
      closeConnect(); // izh 22-05-2020 отключить от WIFI
      lastRouterReboot = millis();
      routerRebootCount++;
      return 1;
      }
    }
  return 0;
}
