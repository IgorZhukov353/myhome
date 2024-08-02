/* 
 Igor Zhukov (c)
 Created:       01-03-2024
 Last changed:  10-03-2024
*/

#include "sensor.h"

#define MAIN 1
#include "main.h"

#define CS_PIN 53


//------------------------------------------------------------------------
// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;

// Функция, возвращающая количество свободного ОЗУ (RAM)
int memoryFree() {
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__bss_end);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  return freeValue;
}

//---------------------------------------------------------------------------
void trace(String str) {  Serial.println(str);}

//---------------------------------------------------------------------------
void APP::configRead() {
  int result;
  for(int i=0;i<3;i++){
    result = SD.begin(CS_PIN);
    if(result)
      break;
    delay(10);
  }
  trace((result)?"SD init-OK!":"SD init -failed!");
  if (!result) {
    return;
  }

  File textFile = SD.open("wokwi.txt");
  if (textFile) {
    String strbuf;
    char c;
    while (textFile.available()) {
      c = textFile.read();
      strbuf += String(c);
    }
    textFile.close();
   // trace("config:" + str);
   sa.load(strbuf);
  } else {
    trace("Error opening config.ini!");
  }
}
//---------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  trace("mem1=" + String(memoryFree()));
  app.configRead();
  trace("mem2=" + String(memoryFree()));
  
}

short mem, nn;
void loop() {
  nn++;
  sa.check(_group);
  short m = memoryFree();
  if( mem != m){
    mem = m;
    trace("mem=" + String(mem));
  }
//  delay(1000);
}

