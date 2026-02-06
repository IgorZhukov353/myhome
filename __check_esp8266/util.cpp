#include "Arduino.h"
#include <RTClib.h>
#include <avr/wdt.h>

#define __util_h__ 
#include "util.h"

#define TRACE 1
#define BAUD 115200  //9600

bool traceInit = false;             // признак инициализации трассировки
short ramMemory = 10000;
RTC_DS1307 RTC;  // часы реального времени

//------------------------------------------------------------------------
// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;

//------------------------------------------------------------------------
// Функция, возвращающая количество свободного ОЗУ (RAM)
int checkMemoryFree() {
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__bss_end);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  if(ramMemory > freeValue)
    ramMemory = freeValue;
  return freeValue;
}

void trace_l(const long i) {
 #ifdef TRACE
  Serial.print(i);
  #endif
}

void trace_i(const int i) {
 #ifdef TRACE
  Serial.print(i);
  #endif
}
void trace_c(const char *c) {
 #ifdef TRACE
  Serial.print(c);
  #endif
}
void trace_s(const String &s) {
 #ifdef TRACE
  Serial.print(s);
  #endif
}
void trace_end(){
 #ifdef TRACE
  Serial.println();
  #endif
}
void trace(const String& msg, byte mode = 0 ) {
 #ifdef TRACE
  if (!traceInit) {
    traceInit = true;
    Serial.begin(BAUD);  // инициализируем порт
  }
  if (mode <= 1) {
    Serial.print(getCurrentDate(1));
    Serial.print("=>");

  }
  if (mode <= 2)
    Serial.print(msg);

  if (mode == 0 || mode == 3)
    Serial.println();
  #endif
}

//------------------------------------------------------------------------
String getCurrentDate(byte noYear = 1){
  DateTime now = RTC.now();
  uint16_t d[6] = { now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second() };
  if(noYear == 1){ // для режима трассировки - сначала день, потом месяц
    short tmp = d[1];
    d[1] = d[2];
    d[2] = tmp;
  }
  String dd;
  dd.reserve(20);
  for (byte i = noYear; i < 6; i++) {
    if (d[i] < 10)
      dd += "0";
    dd += String(d[i]);
    if (i <= 1)
      dd += "-";
    else if (i == 2)
      dd += " ";
    else if (i < 5)
      dd += ":";
  }
  return dd;
}

//------------------------------------------------------------------------
void wdt_delay(int t)
{
  delay(t);
  return;
const int p = 100;  
int i = t / p;
int o = t % p;
for(int ii = 0; ii < i; ii++){
  delay(p);
  wdt_reset();  
  }
if(o)
  delay(o);
}  
