#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet2.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp2.h>
#include <Twitter.h>
#include <util.h>

#include <SPI.h>

//#include <Ethernet.h>

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
