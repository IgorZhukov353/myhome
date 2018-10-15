/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  12-10-2018
*/
#define VERSION "Ver 1.2 of 17-06-2018 Igor Zhukov (C)"

#include <avr/wdt.h>
#include <math.h> 
#include "MsTimer2.h"
#include "DHT.h"
#include "esp_wifi.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "activity.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#define MEGA  1

#define TRACE 1
//#undef TRACE 

#define PIN0	0		// Serial0 RX
#define PIN1	1		// Serial0 TX

#define PIN2	2		// Датчик температуры и влажности в комнате
#define PIN3	3		// Датчик температуры и влажности в подполе
#define PIN4	4  	// Датчик температуры и влажности на улице
#define PIN5	5		// PIR 2 (Движение Гараж)
#define PIN6	6		// PIR 1 (Движение перед Дверь № 1)
#define PIN7	7		// Дверь № 1
#define PIN8	8		// Дверь № 2
#define PIN9	9  	// PIR 3 (Движение Кухня)
#define PIN10	10	// 
#define PIN11	11	// 	
#define PIN12	12  // Датчик температуры DS18B20 в дренажном колодце
#define PIN13	13	// Светодиод

#define PIN14	14
#define PIN15	15
#define PIN16	16
#define PIN17	17
#define PIN18	18    // TX1 COM1 - передача в ESP
#define PIN19	19    // RX1 COM1 - прием из ESP
#define PIN20	20		// SDA --> 4 для часов реального времени
#define PIN21	21		// SCL --> 5 для часов реального времени

#define PIN22	22		// Реле преключение шлейфа термостат котла линия № 1
#define PIN23	23		// Реле преключение шлейфа термостат котла линия № 2
#define PIN24	24		// Реле перезагрузки INT1
#define PIN25	25		// Реле вентиляторов вытяжки в подполе INT2
#define PIN26	26		// Питание насоса в дренажном колодце
#define PIN27	27    // Питание греющего кабеля в дренажном колодце
#define PIN28	28    // Датчик уровня в дренажном колодце
#define PIN29	29    // Включение питания ESP8266 реле INT3

#define PIN30	30    // Свободный вход реле INT4
#define PIN31	31    // Реле питания греющего кабеля водяных труб в подполе INT1 (Реле №2 в ванной)
#define PIN32	32    // Датчик температуры DS18B20 водяных труб в подполе (через макетную плату Белый)
#define PIN33	33
#define PIN34	34
#define PIN35	35
#define PIN36	36
#define PIN37	37
#define PIN38	38
#define PIN39	39

#define PIN50	50	// MISO --> 12	для SD-карты
#define PIN51	51	// MOSI	--> 11	для SD-карты
#define PIN52	52	// SLK	--> 13	для SD-карты
#define PIN53	53	// CS	--> 10	для SD-карты

#define BAUD 115200 //9600

//-------------------------------------------------
RTC_DS1307  RTC; // часы реального времени
ESP_WIFI    esp; // wi-fi ESP266

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)

OneWire oneWire(PIN30); // этот будет 12
OneWire oneWire2(PIN12); // этот будет 33

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature dallasTemp[2] = {DallasTemperature(&oneWire),DallasTemperature(&oneWire2)};

DHT dht[3] = {DHT(PIN2, DHT22), DHT(PIN3, DHT22), DHT(PIN4, DHT22)};
short prevTemp[5] = {-100,-100,-100,-100,-100}, prevHum[5];      // последние показания датчика температуры и влажности

#define state_led_pin 		  PIN13
#define SENS_CHECK_TIMEOUT 	100
#define SENS_TIMEOUT  		  500
#define TEMP_TIMEOUT 		    (60000 * 5) // проверка температуры и влажности раз в 5 минут
#define WATCHDOG_TIMEOUT    (60000 * 60)
#define COMMAND_TIMEOUT     (60000 * 10)
#define BOILER_TIMEOUT      (60000 * 1)

#define MAX_ALARMS 	7
#define ALARM_ON 	  1
#define ALARM_OFF 	0

typedef struct { 
byte id;
byte value; 
byte pre_value; 
byte norm_state; 
byte on; 
byte pin; 
unsigned long change_time; 
bool analog; 
bool check_for_any_status;
} alarm_info;

struct DATA {
    struct {
      byte sysState;
      byte ledState;
      byte tmp_value;
      alarm_info a[MAX_ALARMS] = {
        {1,0,0, LOW, ALARM_ON, 6,0,false,false},  //pir1
        {2,0,0, LOW, ALARM_ON, 5,0,false,false},  //pir2
        {3,0,0, HIGH,ALARM_ON, 7,0,false,false},  //дверь № 1 
        {4,0,0, HIGH,ALARM_ON, 8,0,false,false},  //дверь № 2 
        {5,0,0, HIGH,ALARM_ON, PIN33,0,false,true},   // наличие питания (пока отключен)
        {6,0,0, LOW ,ALARM_ON, 9,0,false,false},  //pir3
        {7,0,0, LOW ,ALARM_ON, PIN28,0,false,true}   //уровень в дрен колодце
        };
    };
  } d;

bool traceInit = false;						      // признак инициализации трассировки
bool watchDogOK_Sended2BD = 0;          // признак отправки дежурного пакета в БД
//--------------------------------------------------------------------------------
class DeviceControl {
public:
bool  ControlOn;                        // признак управления 
unsigned long ControlUntilTime;         // управлять до этого времени

DeviceControl(){ControlOn=false;}
} fan,pump;

class Boiler : public DeviceControl {
public:  
short TargetTemp;                 // целевая температура
bool  CurrentMode;                // текущий режим ардуино-термостата
} boiler;  

/*--------------------------------------------------------------------------------
bool  boilerControlOn = false;          // признак управления газовым котлом (переключаем на ардуино релейную линию с штатного термостата) // 17-12-2017
short boilerTargetTemp;                 // целевая температура
bool  boilerCurrentMode;                // текущий режим ардуино-термостата
unsigned long boilerControlUntilTime;   // управлять котлом до этого времени, потом переключить на штатный термостат
*/

unsigned long lastRouterReboot;         // время последней перезагрузки роутера

void blinky_check();
void sens_check();
void temp_check();
void command_check();
void remoteTermostat_check();
void sendError_check();
void sendBuffer2Site_check();

Activity state_led_blink(1000,blinky_check);
Activity sens(SENS_CHECK_TIMEOUT,sens_check);
Activity tempHum(TEMP_TIMEOUT,temp_check);
//Activity tempHum(10000,temp_check);
Activity readCommand(COMMAND_TIMEOUT,command_check);
Activity remoteTermostat(BOILER_TIMEOUT,remoteTermostat_check);
Activity sendError(COMMAND_TIMEOUT,sendError_check); 
Activity sendBuffer2Site(1000,sendBuffer2Site_check); // передача буфера информации раз в секунду (если есть что)
//------------------------------------------------------------------------
// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;
 
// Функция, возвращающая количество свободного ОЗУ (RAM)
int memoryFree()
{
   int freeValue;
   if((int)__brkval == 0)
      freeValue = ((int)&freeValue) - ((int)&__bss_end);
   else
      freeValue = ((int)&freeValue) - ((int)__brkval);
   return freeValue;
}
//------------------------------------------------------------------------
void setup() 
{
   Wire.begin();
   RTC.begin();
    
   d.sysState = 0;
   d.ledState = LOW;                   // этой переменной устанавливаем состояние светодиода
   pinMode(state_led_pin, OUTPUT);
   digitalWrite(state_led_pin, d.ledState);
    
   sens_setup();
   dallasTemp[0].begin();
   dallasTemp[1].begin();
   
   trace(VERSION);
   
   esp.addEvent2Buffer(1,String(millis()));
   esp.sendBuffer2Site();
}

//------------------------------------------------------------------------
void sens_setup() 
{
   for(byte i=0; i<MAX_ALARMS; i++){
    if(d.a[i].on == ALARM_OFF)
      continue;
    if(d.a[i].analog != true){
      pinMode(d.a[i].pin, INPUT); 
	    digitalWrite(d.a[i].pin, d.a[i].norm_state); // подключение внутр резистора
      }
    d.a[i].pre_value = d.a[i].norm_state;
    d.a[i].value = d.a[i].norm_state;
    trace("Sens init! id=" + String(d.a[i].id) + " v=" + String(d.tmp_value)+ " v2=" + String(d.a[i].pre_value)); 
    }
}
	
//------------------------------------------------------------------------
void sens_check()
{
     unsigned long currentMillis = millis();
	   
	   for(byte i=0; i<MAX_ALARMS; i++){
		  if(d.a[i].on == ALARM_OFF || (!d.sysState && !d.a[i].check_for_any_status)) // не активен или проверка датчиков отключена и датчик можно не проверять
			 continue;
      if(d.a[i].analog == false)
		   d.tmp_value = digitalRead(d.a[i].pin);
      else{
       d.tmp_value = analogRead(d.a[i].pin);
       if( d.tmp_value > 100)
        d.tmp_value = 1;
       else
        d.tmp_value = 0; 
      }

		 if( d.tmp_value != d.a[i].pre_value){
			  d.a[i].pre_value = d.tmp_value;
			  d.a[i].change_time = currentMillis;
        //trace("Sens check! id=" + String(d.a[i].id) + " v=" + String(d.a[i].pre_value));
			}
		 
		 if( d.a[i].change_time > 0 && (currentMillis - d.a[i].change_time) > SENS_TIMEOUT){
			d.a[i].value = d.a[i].pre_value;
			d.a[i].change_time = 0;
      esp.addSens2Buffer(d.a[i].id, d.a[i].value);
      trace("Sens changed! id=" + String(d.a[i].id) + " v=" + String(d.a[i].value));
			}
		}
	  
	  short new_ledInterval = 1000;
	 
	  for(byte i=0; i<MAX_ALARMS; i++){
		   if(d.a[i].on == ALARM_OFF || (!d.sysState && !d.a[i].check_for_any_status)) // не активен или проверка датчиков отключена и датчик можно не проверять
        continue;
		   if(d.a[i].change_time != 0){
			  new_ledInterval = state_led_blink.timeout;
			  break;  
		    }
		   if( d.a[i].value != d.a[i].norm_state){
			  new_ledInterval = 100;
			  break;
		   }
		}
	  if(state_led_blink.timeout != new_ledInterval){
			state_led_blink.timeout = new_ledInterval;
		}
 }

//------------------------------------------------------------------------
short Thermister(byte analogPin) {
  double Temp;
  int RawADC = analogRead(analogPin);
  trace("Thermister.AnalogRead=" + String(RawADC));
  
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;   // Kelvin to Celcius
  short t = round(Temp);
  trace("Thermister.T=" + String(t) + " " +String(Temp));
  return t;
}

//------------------------------------------------------------------------
void temp_check() 
{
    // считывание температуры или влажности занимает примерно 250 мс!
    // считанные показания могут отличаться от актуальных примерно на 2 секунды (это очень медленный датчик)
    
    short h,t;
    for(short i=0; i<5; i++){
      if(i < 3){
        h = round(dht[i].readHumidity());
        if(dht[i].state == false){
          trace("Ошибка чтения температуры для id=" + String(i+1) + "!");
          continue;
        }
        t = round(dht[i].readTemperature());
      }
      else {
        h = 0;
        int ind = i - 3;
        dallasTemp[ind].requestTemperaturesByIndex(0); // Send the command to get temperatures
        for(short ii=0; ii < 10; ii++){
          float ft = dallasTemp[ind].getTempCByIndex(0);
          
          if( ft != -127){
            t = round(ft);
            break;
          }
          delay(50);  
        }
      }
      trace("Темп и влажн. id=" + String(i+1) + " t=" + String(t) + " h=" + String(h));
      
      if( h != prevHum[i] || t != prevTemp[i]){ 
        esp.addTempHum2Buffer(i+1, t, h);
        prevHum[i] = h;
        prevTemp[i] = t;
      }
    }
 }

//------------------------------------------------------------------------
void command_check() 
{
    esp.send2site("get_command.php"); // проверка наличия команд
}

//------------------------------------------------------------------------
void blinky_check() 
{
    if(!d.sysState)
        return;
    d.ledState = !d.ledState; // если светодиод не горит, то зажигаем, и наоборот
    digitalWrite(state_led_pin, d.ledState); // устанавливаем состояния выхода, чтобы включить или выключить светодиод
}

// ---------------------------------------------------обработчик прерывания, 2 мс
void  timerInterrupt() {
//  wdt_reset();  // сброс сторожевого таймера 
}

//------------------------------------------------------------------------
void trace(String msg)
{
#ifdef TRACE
  if(!traceInit){
    traceInit = true;
    Serial.begin(BAUD); // инициализируем порт
  }
    DateTime now = RTC.now();  
/*    
    Serial.print(now.year(), DEC);
    Serial.print('-');
    Serial.print(now.month(), DEC);
    Serial.print('-');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
*/    
    //Serial.println( String(millis()) + ":" + msg );
    short d[5] = {now.day(),now.month(),now.hour(),now.minute(),now.second()};
    String dd;
    for(int i=0; i<5; i++){
      if( d[i] < 10)
        dd += "0";
      dd += String(d[i]); 
      if(i == 0)
        dd += ".";
      else  
        if(i == 1)
          dd += " ";  
      else  
        if(i < 4)
          dd += ":";
    }
    Serial.println( dd +  "=>" + msg );
    //Serial.println( dd + " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) +  "=>" + msg );
#endif     
}

//------------------------------------------------------------------------
void responseProcessing(String response)
{
  short ind = response.indexOf("command="); // признак команды
  if(ind >= 0){
    ind += 8; // длина признака команды
    short ind2 = response.indexOf(";", ind); // поиск первой точки-запятой
    if(ind2 >= 0){
      String cmd = response.substring(ind, ind2);
      trace( "Command processing=" + cmd);  
      
      if(cmd == "reboot")
        remoteRebootExecute();
      else      
      if(cmd == "sensor_ignore"){
        d.sysState = 0;  
        d.ledState = 0;
        digitalWrite(state_led_pin, d.ledState);
      }
      else
      if(cmd == "sensor_check"){
        d.sysState = 1;  
      }
      else
      if(cmd == "boiler_stop"){
       if(boiler.ControlOn)
          boiler.ControlUntilTime = 0;
      }
      if(cmd == "fan_stop"){
       if(fan.ControlOn)
          fan.ControlUntilTime = 0;
      }
      if(cmd == "pump_stop"){
       if(pump.ControlOn)
          pump.ControlUntilTime = 0;
      }
      else
      if(cmd == "fan"){
          trace( "fan init." );
          ind2 += 1;
          ind = response.indexOf(";", ind2);
          fan.ControlUntilTime = atoi(response.substring(ind2, ind).c_str());  
          if( !fan.ControlUntilTime){
            trace( "Error fan period reading!");  
            return;
            }
          fan.ControlUntilTime = millis() + fan.ControlUntilTime * 60000;  // в минутах
          fan.ControlOn = true;
          pinMode(PIN25, OUTPUT);
          digitalWrite(PIN25, LOW);
      }
      else
      if(cmd == "pump"){
          trace( "pump init." );
          ind2 += 1;
          ind = response.indexOf(";", ind2);
          pump.ControlUntilTime = atoi(response.substring(ind2, ind).c_str());  
          if( !pump.ControlUntilTime){
            trace( "Error pump period reading!");  
            return;
            }
          pump.ControlUntilTime = millis() + pump.ControlUntilTime * 60000;  // в минутах
          pump.ControlOn = true;
          pinMode(PIN26, OUTPUT);
          digitalWrite(PIN26, LOW);
      }
      else  
      if(cmd == "boiler"){
          trace( "boiler termostat init." );
          ind2 += 1;
          ind = response.indexOf(";", ind2);
          boiler.TargetTemp = atoi(response.substring(ind2, ind).c_str());  
          if( !boiler.TargetTemp){
            trace( "Error target temp reading!");  
            return;
            }
          ind += 1;
          ind2 = response.indexOf(";", ind);
          boiler.ControlUntilTime = atoi(response.substring(ind, ind2).c_str());
          if( !boiler.ControlUntilTime){
            trace( "Error period reading!");  
            return;
            }
          boiler.ControlUntilTime = millis() + boiler.ControlUntilTime * 60000 * 60;
          boiler.ControlOn = true;
          boiler.CurrentMode = false;
          pinMode(PIN22, OUTPUT);
          pinMode(PIN23, OUTPUT);
          digitalWrite(PIN22, LOW);
          digitalWrite(PIN23, HIGH);
        }
    }
  }
}

//------------------------------------------------------------------------
// удаленная перезагрузка всех устройств
void remoteRebootExecute() 
{
  trace( "Rebooting...");
  pinMode(PIN24, OUTPUT);
  
  digitalWrite(PIN24, LOW);
  delay(2000);
  digitalWrite(PIN24, HIGH);
  pinMode(PIN24, INPUT);
  
  trace( "Rebooted.");
}

//------------------------------------------------------------------------
// функция термостата газового котла и не только
void remoteTermostat_check() 
{
  if(fan.ControlOn){
      if(millis() > fan.ControlUntilTime){ // закончен период работы вентилятора
        trace( "fan stop." );
        fan.ControlOn = false;
        digitalWrite(PIN25, HIGH);
        pinMode(PIN25, INPUT);
        return;      
      }
  }

  if(pump.ControlOn){
      if(millis() > pump.ControlUntilTime){ // закончен период работы насоса
        trace( "pump stop." );
        pump.ControlOn = false;
        digitalWrite(PIN26, HIGH);
        pinMode(PIN26, INPUT);
        return;      
      }
  }
       
  if(boiler.ControlOn){
      if(millis() > boiler.ControlUntilTime){ // закончен период работы ардуино-термостата, переходим на штатный
        trace( "boiler termostat stop." );
        boiler.ControlOn = false;
        digitalWrite(PIN22, HIGH);
        digitalWrite(PIN23, HIGH);
        pinMode(PIN22, INPUT);
        pinMode(PIN23, INPUT);
        return;      
      }

      double t = dht[0].readTemperature();
      unsigned long ms = boiler.ControlUntilTime - millis();
      unsigned int h = (ms / (60*60000));
      unsigned int m = (ms % (60*60000)) / 60000;
      trace( "Target=" + String(boiler.TargetTemp) + " Current=" + String(t) + " Left=" + String(h) + "h " + String(m) + "m"); 
      if( t < boiler.TargetTemp){
        if(!boiler.CurrentMode){
          trace( "Heat on."); 
          boiler.CurrentMode = true;  
          digitalWrite(PIN23, LOW);        
        }
      }
      else{
        if(boiler.CurrentMode){
          trace( "Heat off."); 
          boiler.CurrentMode = false;  
          digitalWrite(PIN23, HIGH);        
        }
      }
    }
}

//------------------------------------------------------------------------
// проверка наличия пакета ошибок передач по WIFI если надо - попытка перезагрузки, запуск раз в 10 мин
void sendError_check() 
{
  trace( "SendErrorCounter=" + String(esp.sendErrorCounter) + " RouterConnectErrorCounter=" + String(esp.routerConnectErrorCounter));
  if(esp.sendErrorCounter > 10){
    bool res = esp.espSendCommand( "AT+PING=\"192.168.0.1\"" , (char*)"OK" , 5000); // попытка пингануть роутер
    if(res || millis() - lastRouterReboot > WATCHDOG_TIMEOUT ){ // если он жив, то проблема с доступом в Инет, перегрузить роутер или пропал WIFI (но не чаще чем в 1 час)
      lastRouterReboot = millis();
      remoteRebootExecute();
      }
    }
}

//------------------------------------------------------------------------
void sendBuffer2Site_check()
{
  esp.sendBuffer2Site();
}

//------------------------------------------------------------------------
void loop() 
{
// return;
  
 esp.checkIdle();
 sendError.checkActivated();

 state_led_blink.checkActivated();
 sens.checkActivated();
 tempHum.checkActivated();
 readCommand.checkActivated();
 remoteTermostat.checkActivated();

 if( (millis() % WATCHDOG_TIMEOUT) < 10000){ // регулярная отправка дежурного сообщения
    if(!watchDogOK_Sended2BD){
      watchDogOK_Sended2BD = true;

      esp.checkInitialized();
      byte ind, a[6] = {9,10,11,12,14,16};
      String dopInfo = "";
      for(ind = 0; ind < 6; ind++){ // пинги видеорегистратора и камер
        String str = (esp.espSendCommand( "AT+PING=\"192.168.0." + String(a[ind]) + "\"" , (char*)"OK" , 5000 ))? "ok":"failed";
        dopInfo += String(a[ind]) + ":" + str + ";"; // json экранирование ":" -> \\u003A
      }

      unsigned long t = millis();
      unsigned int d = t/(24*60*60000);
      unsigned int h = (t%(24*60*60000)) / (60*60000);
      trace( "Watchdog=" + String(t) + dopInfo);

      //esp.send2site("send_mail.php"); /*izh 17-03-2018 раз в час проверить срабатывание датчиков и температуры */

      esp.addEvent2Buffer(3, "days=" + String(d) + "hours="  + String(h) + "(" + dopInfo + String(t) + ")");
      //esp.sendBuffer2Site();
      //esp.send2site("send_event.php?id=3&text=days=" + String(d) + "hours=" + String(h)+ "(" + dopInfo + String(t) + ")"); 
      traceInit = false;
    }
 }
 else
    watchDogOK_Sended2BD = false;

 sendBuffer2Site.checkActivated();     
}

//------------------------------------------------------------------------
void esp_power_switch(bool p)
{
  return;
  if(p== true){
    pinMode(PIN29, OUTPUT);
    digitalWrite(PIN29, LOW);
    }
  else {
    digitalWrite(PIN29, HIGH);
    pinMode(PIN29, INPUT);
  }

}

