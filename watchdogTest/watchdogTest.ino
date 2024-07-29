#include <avr/wdt.h>
//------------------------------------------------------------------------
// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;

// Функция, возвращающая количество свободного ОЗУ (RAM)
int checkMemoryFree() {
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__bss_end);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
 // if(ramMemory > freeValue)
   // ramMemory = freeValue;
  return freeValue;
}

void setup() {
  wdt_disable(); // бесполезная строка до которой не доходит выполнение при bootloop
  Serial.begin(9600);
  Serial.println("Setup..");
  
  Serial.println("Wait 5 sec..");
  delay(5000); // Задержка, чтобы было время перепрошить устройство в случае bootloop
  wdt_enable (WDTO_8S); // Для тестов не рекомендуется устанавливать значение менее 8 сек.
  Serial.println("Watchdog enabled.");
}

int timer = 0;
int size = 1000;

void loop(){
  // Каждую секунду мигаем светодиодом и значение счетчика пишем в Serial
  if(!(millis()%1000)){
    timer++;
    Serial.println(timer);
    digitalWrite(13, digitalRead(13)==1?0:1); delay(1);
    Serial.println("Mem=" + String(checkMemoryFree()));
    
    void *ptr = malloc(size);
    if(!ptr){
      Serial.println("malloc" +String(size)+ " failed!");
      size /= 2;
    }
  }
  
  wdt_reset();
}
