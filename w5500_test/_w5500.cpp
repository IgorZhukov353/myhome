#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet2.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp2.h>
#include <Twitter.h>
#include <util.h>

#include <SPI.h>

#include "inet.h"
#include "_w5500.h"

// МАК адрес
byte _mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress _ip(192, 168, 0, 20);
EthernetClient _client;
unsigned char _buffer[512]; // буфер для чтения

//--------------------------------------------------
bool w5500_sender::init() 
{
  Ethernet.begin(_mac, _ip);
  delay(1000); // ждем инициалацию модуля
}

//--------------------------------------------------
bool w5500_sender::connect(const char *host, uint16_t port)
{
  return _client.connect(host, port);
}
//--------------------------------------------------
int w5500_sender::available()
{
  return _client.available();
}
//--------------------------------------------------
char * w5500_sender::read()
{
  short count_read = _client.read(_buffer, 512);
  _buffer[ count_read ] = '\0'; // запишем в конец символ конца строки
  return (char *) _buffer;
}
//--------------------------------------------------
void w5500_sender::close()
{
    _client.flush(); // сброс
    _client.stop();  // отключение
}
//--------------------------------------------------
bool w5500_sender::ping(const char* host)
{
    return false;
}
