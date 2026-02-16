/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  13-02-2026
*/

#include "Arduino.h"
#include <avr/wdt.h>
#include "util.h"
#include "esp_wifi.h"

//--------------------------------------------------
// внешние функции
void responseProcessing(const String& resp){};
void esp_power_switch(bool p){
#define PIN29 29
    if (p == true) {
    pinMode(PIN29, OUTPUT);
    digitalWrite(PIN29, LOW);
  } else {
    digitalWrite(PIN29, HIGH);
    pinMode(PIN29, INPUT);
  }
};
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


#define ESP_Serial Serial1 // для МЕГИ

#define MYHOME_HOST F("igorzhukov353.h1n.ru")
#define MYHOME_HOST_IP F("81.90.182.128")
#define OTHER_HOST F("cloud-api.yandex.net")
#define OTHER_HOST_TOKEN F("y0__xCUvKUJGJSePSCz26qaFjDmy9b-B4i8Bo9O74ZhdYVkUc5SMnESwVgv")

//------------------------------------------------------------------------
ESP_WIFI::ESP_WIFI() {
wifi_initialized = false;
sendErrorCounter = 0;
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::send2site(const String &reqStr) {
  return _send2site(reqStr, NULL);
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::_send() {
  enum _SendPar:byte {_SSL=1,_MYHOME_HOST=2,_HTTP_PUT=4};
  byte param = _SSL; // + _MYHOME_HOST;
  const char *postBuf = nullptr; //"[{\"type\":\"S\",\"id\":1,\"v\":1},{\"type\":\"T\",\"id\":1,\"temp\":12,\"hum\":80}]";
  
  const String reqStr = F("/v1/disk/resources?path=fortest%2Fdir1&fields=custom_properties");

  short postBufLen = (postBuf) ? strlen(postBuf) : 0;  //

  bool r;
  {
  String cmd1;
  cmd1 = F("AT+CIPSTART=\"");
  cmd1 += (param & _SSL)? F("SSL"):F("TCP");
  cmd1 += F("\",\"");
  cmd1 += (param & _MYHOME_HOST)? MYHOME_HOST_IP : OTHER_HOST;
  cmd1 += F("\"");
  if(param & _SSL){
    r = espSendCommand( F("AT+CIPSSLSIZE=4096"), STATE::OK, 5000 );

    cmd1 += F(",443,1200");
    }
  r = espSendCommand( cmd1, STATE::OK, 15000 );   // установить соединение с хостом
  if(lastErrorTypeId == ErrorType::ALREADY_CONNECT){
    r = true;
    }
  }

  if(r){
    String request, request2;
    short requestLength;
    short maxlen = reqStr.length() + postBufLen + 200;
    if (maxlen > 1024)
      maxlen = 1024;
    request.reserve(maxlen);

    if (!postBuf) {
      request = F("GET ");
    } else {
      if(param & _HTTP_PUT)
        request = F("PUT ");
      else
        request = F("POST ");
    }
    request += reqStr;
    request += F(" HTTP/1.1\r\nHost: ");
    request += (param & _MYHOME_HOST)? MYHOME_HOST : OTHER_HOST;
    request += F("\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36"); // чтобы не блокировали за частые запросы 7-04-2025
    request += F("\r\nConnection: close\r\n");

    if (!postBuf) {
      requestLength = request.length() + 2; // add 2 because \r\n will be appended by Serial.println().
      if(!(param & _MYHOME_HOST)){
        request += F("Accept: application/json\r\nAuthorization: OAuth y0__xCUvKUJGJSePSCz26qaFjDmy9b-B4i8Bo9O74ZhdYVkUc5SMnESwVgv\r\n");
      }
    } else {
      if(param & _MYHOME_HOST){
        request += F("Cache-Control: no-cache\r\n");
        request += F("Authorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\n");
        request += F("Content-Length: "); 
        request += postBufLen;
        request += F("\r\n");
        request += F("Content-Type: application/json\r\n");
      }
      else {
        request += F("Accept: application/json\r\nAuthorization: OAuth y0__xCUvKUJGJSePSCz26qaFjDmy9b-B4i8Bo9O74ZhdYVkUc5SMnESwVgv\r\n");
      }

      request += F("\r\n"); // перед postBuf
      request2 = F("\r\n"); // после postBuf
      requestLength = request.length() + postBufLen  + request2.length() + 2; // add 2 because \r\n will be appended by Serial.println().
    }
    {
      String cmd2 = F("AT+CIPSEND=");
      cmd2 += requestLength;
      r = espSendCommand( cmd2, STATE::OK, 5000 );              // подготовить отсылку запроса - длина запроса
      bytesSended += requestLength;
    }

    r = espSendCommand( request, STATE::CLOSED, 15000, postBuf, request2);    // отослать запрос и получить ответ
  }
}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::_send2ya() {
  	if (!checkInitialized()) {
    		return false;
  	}
 	//String token = F("y0__xCUvKUJGJSePSCz26qaFjDmy9b-B4i8Bo9O74ZhdYVkUc5SMnESwVgv");
	String HOST_IP_STR = F("www.google.com");//cloud-api.yandex.net");
 	//String url   = F("https://cloud-api.yandex.net/v1/disk/resources?path=fortest%2Fdir1&fields=custom_properties");
   	String cmd1;
    	cmd1 = F("AT+CIPSTART=\"SSL\",\"");
    	cmd1 += HOST_IP_STR;
    	cmd1 += F("\",443,1200");
    	bool r = espSendCommand( cmd1, STATE::OK, 15000 );   // установить соединение с хостом

}

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool ESP_WIFI::_send2site(const String &reqStr, const char *postBuf) {
  if (!checkInitialized()) {
    return false;
  }
  if(yandexOnly){
    if(ping(HOST_IP_STR, 15000)) // попытка пингануть свой сервер
      yandexOnly = false;
    else
      return false;
  }
  short postBufLen = (postBuf) ? strlen(postBuf) + 6 : 0;  // + 6 ->str =[...]
  bool r;
  {
    String cmd1;
    cmd1 = F("AT+CIPSTART=\"TCP\",\"");
    cmd1 += HOST_IP_STR;
    cmd1 += F("\",80");
    r = espSendCommand( cmd1, STATE::OK, 15000 );   // установить соединение с хостом
    if(lastErrorTypeId == ErrorType::ALREADY_CONNECT){
      r = 1;
    }
  }
  if (r) {  // соединение установлено
    String request, request2;
    short requestLength;
    short maxlen = reqStr.length() + postBufLen + 200;
    if (maxlen > 1024)
      maxlen = 1024;
    request.reserve(maxlen);

    if (!postBuf) {
      request = F("GET /");
    } else {
      request = F("POST /");
    }
    request += reqStr;
    request += F(" HTTP/1.1\r\nHost: ");
    request += HOST_STR;
    request += F("\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36"); // чтобы не блокировали за частые запросы 7-04-2025
    request += F("\r\nConnection: close\r\n");
    if (!postBuf) {
      requestLength = request.length() + 2; // add 2 because \r\n will be appended by Serial.println().
    } else {
      request += F( "Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\nCache-Control: no-cache\r\nContent-Length: ");
      request += postBufLen;
      request += F("\r\n\r\n");
      request += F("str=[");
      request2 = F("]"); 
      request2 += F("\r\n");
      requestLength = request.length() + (postBufLen - 6) + request2.length() + 2; // add 2 because \r\n will be appended by Serial.println().
    }
    
    {
      String cmd2 = F("AT+CIPSEND=");
      cmd2 += requestLength;
      r = espSendCommand( cmd2, STATE::OK, 5000 );              // подготовить отсылку запроса - длина запроса
      bytesSended += requestLength;
    }
    r = espSendCommand( request, STATE::CLOSED, 15000, postBuf, request2);    // отослать запрос и получить ответ
  }
  else{
    lastErrorTypeId = ErrorType::CONNECT;
    connectFailCounter++;
  }
  
  if (!r && lastErrorTypeId >= ErrorType::TIMEOUT) {
    sendErrorCounter++;   // счетчик ошибочных отправок (для определения проблемы доступа к интернету - возможно нужно перезагрузить роутер)
    sendErrorCounter_ForAll++;
  } 
  else {
    sendErrorCounter = 0;
    lastWIFISended = millis();  // время последней успешной отправки на сервер
  }

  sendCounter_ForAll++;

  return r;
}

//------------------------------------------------------------------------
bool ESP_WIFI::espSerialSetup() {
bool r;
esp_power_switch(true);
//delay(200);
wdt_delay(200); // 6-02-2025 !watchdog!

ESP_Serial.begin(115200); // default baud rate for ESP8266
//delay(100);
wdt_delay(100); // 6-02-2025 !watchdog!

r = espSendCommand(F("AT"), STATE::OK , 5000 );
//AT+RST
r = espSendCommand(F("ATE0"), STATE::OK , 5000 );
r = espSendCommand( F("AT+GMR"), STATE::OK, 10000 );
r = espSendCommand( F("AT+CWMODE=1"), STATE::OK, 5000 );
{
  String str = F("AT+CWJAP=\""); 
  str += WSSID;
  str += F("\",\"");
  str += WPASS;
  str += F("\"");
  r = espSendCommand(str, STATE::OK, 20000);  
}
if(!r){
  routerConnectErrorCounter++; 
  } else {
  routerConnectErrorCounter = 0;
  r = espSendCommand( F("AT+CIFSR"), STATE::OK, 5000 );
}
return r;
}

//------------------------------------------------------------------------
#define BUF_SIZE 10
#define STATE_STR_MAX 6
#define RESPONSE_LEN_MAX 512
static const char *state_str[STATE_STR_MAX] = {"OK", "ERROR", "HTTP/1.1", "200 OK", "CLOSED", "ALREADY"};
static const byte state_str_len[STATE_STR_MAX] = {2, 5, 8, 6, 6, 7};

bool ESP_WIFI::espSendCommand(const String &cmd, const STATE goodResponse, const unsigned long timeout, const char *postBuf, const String &cmd2) {
{
    trace_begin(F("espSendCommand(\""));
    trace_s(cmd);
    if(postBuf){
      trace_c(postBuf);
      trace_s(cmd2);
    }
    trace_s(F("\","));
    trace_i((byte)goodResponse);
    trace_s(F(","));
    trace_l(timeout);
    trace_s(F(")"));
    trace_end();  
  }
  return true;

  short msglen = cmd.length();
  if(postBuf){
    msglen += strlen(postBuf);  
    msglen += cmd2.length();
    ESP_Serial.print(cmd);
    ESP_Serial.print(postBuf);
    ESP_Serial.println(cmd2);
  } else
    ESP_Serial.println(cmd);
    
  if(maxSendedMSG < msglen)
    maxSendedMSG = msglen;
    
  unsigned long tnow, tstart;
  bool result;
  String response;
  short responseLenMax = (goodResponse == STATE::CLOSED)? RESPONSE_LEN_MAX: 200;
  char c;
  char cbuffer[BUF_SIZE]; //= {'*','*','*','*','*','*','*','*','*','*'};
  bool state_str_on[STATE_STR_MAX] = {0, 0, 0, 0, 0, 0};
  bool recived = false;
  bool bufOverFlag = false;
  short currResponseLen = 0;
  tnow = tstart = millis();
  response.reserve(responseLenMax);
  
  while ( tnow <= tstart + timeout ) {
    c = ESP_Serial.read();
    if(c > 0) {
      currResponseLen++;
      if(currResponseLen < responseLenMax){
        response += c; //String(c);
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
      else{
        bufOverFlag = true;
      }
    }
    if(recived)
      break;
    tnow = millis();

    wdt_reset(); // 6-02-2025 !watchdog!
  }
  
  while (ESP_Serial.available()) {
    c = ESP_Serial.read();
    currResponseLen++;
    if(currResponseLen < responseLenMax)
      response += c;
    else{
      bufOverFlag = true;
     }
  }
  {
    if(bufOverFlag)
      buffOverCounter++;  
      
    trace_begin(F("espSendCommand:"));
	  if ( recived) {
      if(state_str_on[(byte)STATE::ALREADY]){
        lastErrorTypeId = ErrorType::ALREADY_CONNECT;
        result = false;
        } 
      else{
		  if(state_str_on[(byte)STATE::HTTP] && !state_str_on[(byte)STATE::HTTP_OK]){
		    httpFailCounter++;
        lastErrorTypeId = ErrorType::HTTP_FAIL;
        result = false;
        } 
      else{
        result = (state_str_on[(byte)STATE::ERR] || bufOverFlag) ? false : true;
        lastErrorTypeId = (result) ? ErrorType::NONE : (bufOverFlag)? ErrorType::BUFFOVER : ErrorType::OTHER;
        }
      }
      trace_s((result) ? F("SUCCESS") : F("ERROR"));
      
      } 
    else {
		  result = false;
      trace_s(F("ERROR - Timeout"));
      timeoutCounter++;
      lastErrorTypeId = ErrorType::TIMEOUT;
	  }
    trace_s(F(" - Response time: " ));
    trace_l(millis() - tstart);
    trace_s(F("ms. Len: "));
    trace_i(currResponseLen);
	  trace_s(F("\n\rRESPONSE:"));
	  trace_s(response);
	  trace_s(F("\n\r---END RESPONSE---"));
	  trace_end();
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
bool ESP_WIFI::checkInitialized() {
  if(!wifi_initialized){
    wifi_initialized = espSerialSetup();
  }
  return wifi_initialized;
}

//------------------------------------------------------------------------
void ESP_WIFI::addInfo2Buffer(const char *str) {
  short currBufLen = strlen(buffer);
  short strLen = strlen(str);

  if (strLen > sizeof(buffer)) {
    buffOverCounter++;
    return;
  }
  if (!currBufLen) {
    strcpy(buffer, str);
    return;
  }
  if (currBufLen + strLen + 1 > sizeof(buffer)) {
    buffOverCounter++;
    strcpy(buffer, str);
    return;
  }
  //trace("1 currBufLen="+String(strlen(buffer)) +" buffer=" + String(buffer));
  strcat(buffer, ",");
  strcat(buffer, str);
  //trace("2 currBufLen="+String(strlen(buffer)) +" buffer=" + String(buffer));
}

//------------------------------------------------------------------------
void ESP_WIFI::addEvent2Buffer(short id, const String &msgText) {
  char *pfmt = (msgText[0] == '{') ? PSTR("{\"type\":\"E\",\"id\":%d,\"text\":%s,\"date\":\"%s\"}") : PSTR("{\"type\":\"E\",\"id\":%d,\"text\":\"%s\",\"date\":\"%s\"}");
  short len = strlen_P(pfmt) + 1;
  char fmt[len];
  strcpy_P(fmt, pfmt);
  len += 2 + msgText.length() + 20;
  char loc_buf[len];
  sprintf(loc_buf, fmt, id, msgText.c_str(), getCurrentDate(0).c_str());
  addInfo2Buffer(loc_buf);
}
//------------------------------------------------------------------------
void ESP_WIFI::addTempHum2Buffer(short id, short temp, short hum) {
  char *pfmt = PSTR("{\"type\":\"T\",\"id\":%d,\"temp\":%d,\"hum\":%d,\"date\":\"%s\"}");
  short len = strlen_P(pfmt) + 1;
  char fmt[len];
  strcpy_P(fmt, pfmt);
  len += 2 + 4 + 4 + 20;
  char loc_buf[len];
  sprintf(loc_buf, fmt, id, temp, hum, getCurrentDate(0).c_str());
  addInfo2Buffer(loc_buf);
}

//------------------------------------------------------------------------
void ESP_WIFI::addSens2Buffer(short id, short val) {
  char *pfmt = PSTR("{\"type\":\"S\",\"id\":%d,\"v\":%d,\"date\":\"%s\"}");
  short len = strlen_P(pfmt) + 1;
  char fmt[len];
  strcpy_P(fmt, pfmt);
  len += 2 + 1 + 20;
  char loc_buf[len];
  sprintf(loc_buf, fmt, id, val, getCurrentDate(0).c_str());
  addInfo2Buffer(loc_buf);
}

//------------------------------------------------------------------------
bool ESP_WIFI::sendBuffer2Site() {
  if (strlen(buffer) == 0)
    return false;
  if (_send2site(F("upd/send_info.php"), buffer))
    buffer[0] = 0;
  return true;
}

//------------------------------------------------------------------------
bool ESP_WIFI::check_Wait_Internet() {
   if(!checkInitialized())
    return 0;
   bool res = false;
   trace(F("check_Wait_Internet ...")); 
   unsigned long tstart, tnow, timeout = 1000L * 60 * 2; // izh 28-10-2018 таймаут 2 мин или до появления пинга
   tnow = tstart = millis();
   while(tnow < tstart + timeout ){
    //delay(10000);
    wdt_delay(10000); // 6-02-2025 !watchdog!
    res = ping(HOST_IP_STR, 15000);  // попытка пингануть свой сервер
    if (res) {
      break;
    }
    res = ping(F("ya.ru"), 5000); // попытка пингануть яндекс
    if(res){ // 6-01-2026 отключен мобильный интернет по БПЛА-опасности, перегружать нет смысла, нужно ждать когда будет доступен HOST_IP_STR
      yandexOnly = true;  
      return 0;
    }
    tnow = millis();
    
    }
  return res;
}

//------------------------------------------------------------------------
void ESP_WIFI::closeConnect() {
  if(wifi_initialized){
    espSendCommand(F("AT+CWQAP"), STATE::OK, 15000 );
    //delay(1000);
    wdt_delay(1000); // 6-02-2025 !watchdog!
    espSendCommand(F("AT+RST"), STATE::OK , 20000 );
    //delay(1000); 
    wdt_delay(1000); // 6-02-2025 !watchdog!
    wifi_initialized = false;
    esp_power_switch(false);
  }  
}

//------------------------------------------------------------------------
bool ESP_WIFI::sendError_check() {
  {
    trace_begin(F("MEM="));
    trace_i(checkMemoryFree());
    trace_s(F(" MaxMsgLen="));
    trace_i(maxSendedMSG);
    trace_s(F(" LastSendErr="));
    trace_i((byte)lastErrorTypeId);
    trace_s(F(" SErr="));
    trace_i(sendErrorCounter);
    trace_s(F(" RCErr="));
    trace_i(routerConnectErrorCounter);
    trace_s(F(" HttpErr="));
    trace_i(httpFailCounter);
    trace_s(F(" BufOvrErr="));
    trace_i(buffOverCounter);
    trace_s(F(" TOutErr="));
    trace_i(timeoutCounter);
    trace_s(F(" ConnErr="));
    trace_i(connectFailCounter);
    trace_s(F(" YandexOnly="));
    trace_i(yandexOnly);
    trace_end();
  }
  //ping(F("ya.ru"), 5000); // попытка пингануть яндекс
  
  bool res;
  if(sendErrorCounter > 3)
    res = false;
  else  
    res = 1; //ping(F("192.168.8.1"), 5000); // попытка пингануть модем    
    
  if(!res){
    res = ping(HOST_IP_STR, 15000); // попытка пингануть свой сервер
    if(res){ // все наладилось
        res = ping(HOST_IP_STR, 15000, 1); // попытка соединиться со своим сервером через TCP
        if(res){ // все действительно наладилось
          ping(HOST_IP_STR, 15000, 2);  // отсоединиться
          sendErrorCounter = 0;
          yandexOnly = false;
          return 0;
        }
    }
  else {
    res = ping(F("ya.ru"), 5000); // попытка пингануть яндекс
    if(res){ // 6-01-2026 возможно отключен мобильный интернет по БПЛА-опасности (доступен только яндекс), перегружать нет смысла, нужно ждать когда будет доступен HOST_IP_STR
      yandexOnly = true;  
      return 0;
    }
  }

    res = ping(F("192.168.0.1"), 5000); // попытка пингануть роутер
    if (!res && lastErrorTypeId == ErrorType::TIMEOUT && lastRouterReboot > lastWIFISended) {  // роутер не отвечает на ping, последняя ошибка была по таймауту и последнее успешное отправление было до перезагрузки роутера -> перегрузить МЕГУ
      wdt_enable(WDTO_8S);                                                                     // Для тестов не рекомендуется устанавливать значение менее 8 сек
      delay(10000); // перезагрузка
      return 0; // сюда уже не попадем
    }
    if(millis() - lastRouterReboot > (60000 * 60) ){ // если роутер отвечает на ping, то проблема с доступом в Инет (модем, кончились деньги и т.п.), инче проблема в роутере -> перегрузить роутер (но не чаще чем в 1 час)
      closeConnect(); // izh 22-05-2020 отключить от WIFI
      lastRouterReboot = millis();
      routerRebootCount++;
      sendErrorCounter_ForAll = 0;
      httpFailCounter = 0;
      timeoutCounter = 0;
      buffOverCounter = 0;
      connectFailCounter = 0;
      yandexOnly = false;
      return 1;
      //return 0;
      }
    }
  return 0;
}

bool ESP_WIFI::ping(const String &host, short timeout = 5000, byte _act=0) {
  String str;
  switch(_act){
  case 0:  
    str = F("AT+PING=\"");
    str += host;
    str += F("\"");
    break;
  case 1:
    str = F("AT+CIPSTART=\"TCP\",\"");
    str += host;
    str += F("\",80");
    break;
  case 2:
    str = F("AT+CIPCLOSE");
    break;
  }
  return espSendCommand(str, STATE::OK , timeout);
}
