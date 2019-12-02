#include <SPI.h>

#include <Ethernet.h>

// МАК адрес
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// адрес сервера
char server[] = "www.cbr-xml-daily.ru";

const String HOST_STR = "f0195241.xsph.ru";
//const char *HOST_STR = "igorzhukov353.h1n.ru"; 

IPAddress ip(192, 168, 0, 177);

EthernetClient client;

unsigned char buffer[512]; // буфер для чтения
int i = 0, count_read = 0;

  #if defined(__AVR_ATmega2560__)
  #define _mega "Setup start: MEGA"
  #else
  #define _mega "Setup start: not MEGA"
  #endif

//------------------------------------------------------------------------
// выполнить команду на удаленном сервере (через wi-fi)
bool _send2site(String reqStr, String postBuf) 
{

//  String cmd1 = "AT+CIPSTART=\"TCP\",\"" + String(HOST_STR) +"\",80";
  String request = (postBuf == "")? 
    "GET /"  + reqStr + " HTTP/1.1\r\nHost: " + String(HOST_STR) + "\r\nConnection: close\r\n" :
    "POST /" + reqStr + " HTTP/1.1\r\nHost: " + String(HOST_STR) + "\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nAuthorization: Basic aWdvcmp1a292MzUzOlJEQ3RndjE5Ng==\r\nCache-Control: no-cache\r\nContent-Length: "  + postBuf.length() + "\r\n\r\n" + postBuf + "\r\n";

  short requestLength = request.length() + 2; // add 2 because \r\n will be appended by Serial.println().
  String cmd2 = "AT+CIPSEND=" + String(requestLength);

  bool r;
  for(short i=0; i<3; i++){
      r = client.open(HOST_STR,80);  // установить соединение с хостом (3 попытки)
      if(!r){
        delay(1000);
        continue;
      }
      r = espSendCommand( cmd2 , ok_str , 5000 );             // подготовить отсылку запроса - длина запроса
      bytesSended += requestLength;
      r = espSendCommand( request , (char*)"CLOSED" , 15000 );  // отослать запрос и получить ответ
//      r = espSendCommand("AT+CIPCLOSE", ok_str , 5000 );
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
bool espSerialSetup() 
{
  bool r = true;
  return r;
}

//---------------------------------------------------------------------
bool espSendCommand(String cmd, char* goodResponse, unsigned long timeout) 
{
trace("espSendCommand( " + cmd + " , " + goodResponse + " , " + String(timeout) + " )" );
ESP_Serial.println(cmd);
unsigned long tnow, tstart;
tnow = tstart = millis();
String response;
response.reserve(255);
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

void setup() 
{
  Serial.begin(115200);
  Serial.println(_mega);

  if (Ethernet.begin(mac) == 0) 
  {
    Serial.println("Ошибка инициалации, включение режима DHCP");
    Ethernet.begin(mac, ip);
  }
  
  delay(1000); // ждем инициалацию модуля

  Serial.println("Setup end");
}

void loop() 
{
  //return;
  if (client.connect(server, 80))
  {
    Serial.println("Есть подключение к серверу");

    // формирование HTTP запроса
    client.println("GET /daily_json.js HTTP/1.1");
    client.println("Host: www.cbr-xml-daily.ru");
    client.println("Connection: close");
    client.println();

    i = 0, count_read = 0;

    while(true)
    {
        while( client.available() ) 
        {
          count_read = client.read(buffer, 512);

          if( count_read == 0 )
          {
            break;
          }

          buffer[ count_read ] = '\0'; // запишем в конец символ конца строки
          
          Serial.print( (char *) buffer);
        }

        // ждем данные
        if( ! client.available() )
        {
            Serial.print(".");
            i += 1;
            delay(22);
        }

        if(i > 20)
        {
          break;
        }
    }
  
    Serial.println();
    Serial.println("Отключение.");
    client.flush(); // сброс
    client.stop();  // отключение
  } 
  else 
  {
    Serial.println("Не удалось подключиться");
  }

  delay(5000);
}
