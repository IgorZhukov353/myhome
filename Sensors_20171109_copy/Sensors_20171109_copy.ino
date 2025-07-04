/*
  Igor Zhukov (c)
  Created:       01-11-2017
  Last changed:  30-06-2025	-++
*/
#define VERSION "Ver 1.185 of 30-06-2025 Igor Zhukov (C)"

#include <avr/wdt.h>
#include <math.h>
#include "MsTimer2.h"
#include "DHT.h"
#include "esp_wifi.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include "activity.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#define MEGA 1


//#undef TRACE

#define PIN0 0  // Serial0 RX
#define PIN1 1  // Serial0 TX

#define PIN2 2    // Датчик температуры и влажности в комнате
#define PIN3 3    // Датчик температуры и влажности в подполе
#define PIN4 4    // Датчик температуры и влажности на улице
#define PIN5 5    // PIR 2 (Движение Гараж)
#define PIN6 6    // PIR 1 (Движение перед Дверь № 1)
#define PIN7 7    // Дверь № 1
#define PIN8 8    // Дверь № 2
#define PIN9 9    // PIR 3 (Движение Кухня)
#define PIN10 10  //
#define PIN11 11  //
#define PIN12 12  // Датчик температуры DS18B20 в дренажном колодце
#define PIN13 13  // Светодиод

#define PIN14 14
#define PIN15 15
#define PIN16 16
#define PIN17 17
#define PIN18 18  // TX1 COM1 - передача в ESP
#define PIN19 19  // RX1 COM1 - прием из ESP
#define PIN20 20  // SDA --> 4 для часов реального времени
#define PIN21 21  // SCL --> 5 для часов реального времени

#define PIN22 22  // Реле преключение шлейфа термостат котла линия № 1
#define PIN23 23  // Реле преключение шлейфа термостат котла линия № 2
#define PIN24 24  // Реле INT1 - перезагрузка роутера
#define PIN25 25  // Реле INT2 - Включение 12V на полив (раньше было - Выключение MINI-PC и CAM22 (раньше было -вентиляторы вытяжки в подполе))
#define PIN26 26  // Питание насоса в дренажном колодце (INT1 реле в ванной)
#define PIN27 27  // Питание греющего кабеля в дренажном колодце & септике (INT2 реле в ванной)
#define PIN28 28  // Датчик уровня в дренажном колодце
#define PIN29 29  // Реле INT3 - Включение питания ESP8266

#define PIN30 30  // Реле INT4 - перезагрузка камер, регистратора
#define PIN31 31  // Реле питания греющего кабеля водяных труб в подполе (INT3 реле в ванной)
#define PIN32 32  // Датчик температуры DS18B20 в септике (через макетную плату Белый)
#define PIN33 33  // Наличие питания ~220 V (16-11-2023 через устройство на оптроне 0 - есть, 1 - нет)
#define PIN34 34  // Датчик температуры DS18B20 (площадка 2 этаж)
#define PIN35 35  // PIR 4 (Движение площадка 2 этаж)
#define PIN36 36  // Датчик температуры DS18B20 (гараж овощной ящик)
#define PIN37 37  // Датчик температуры DS18B20 (теплица)
#define PIN38 38  // Реле 1 INT1 - клапан подачи воды в бочку полива (вкл 12v)
#define PIN39 39  // Реле одноканальное - клапан подачи воды 2 для полива

#define PIN40 40  // Реле 1 INT2 - переключатель между клапаном подачи воды в бочку полива и актуатором крана воды из бочки на капельный полив
#define PIN41 41  // Реле 2 INT1,INT2 - вкл/выкл (смена полярности питания 12v) актуатором крана воды из бочки на капельный полив
#define PIN42 42  // Датчик уровня воды в бочке полива 1-неполная, 0-полная
#define PIN43 43
#define PIN44 44
#define PIN45 45
#define PIN46 46
#define PIN47 47
#define PIN48 48
#define PIN49 49

#define PIN50 50  // MISO --> 12	для SD-карты
#define PIN51 51  // MOSI	--> 11	для SD-карты
#define PIN52 52  // SLK	--> 13	для SD-карты
#define PIN53 53  // CS	--> 10	для SD-карты

#define BAUD 115200  //9600

//-------------------------------------------------
//RTC_DS1307 RTC;  // часы реального времени
ESP_WIFI esp;  // wi-fi ESP266

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
#define MAX_TEMP_SENS 8
#define MAX_DALLAS_SENS 5

OneWire oneWire[MAX_DALLAS_SENS] = { OneWire(PIN12), OneWire(PIN32), OneWire(PIN34), OneWire(PIN36), OneWire(PIN37) };  //

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature dallasTemp[MAX_DALLAS_SENS] = { DallasTemperature(&oneWire[0]), DallasTemperature(&oneWire[1]), DallasTemperature(&oneWire[2]), DallasTemperature(&oneWire[3]), DallasTemperature(&oneWire[4]) };

DHT dht[3] = { DHT(PIN2, DHT22), DHT(PIN3, DHT22), DHT(PIN4, DHT22) };
short prevTemp[MAX_TEMP_SENS] = { -100, -100, -100, -100, -100, -100, -100, -100 }, prevHum[MAX_TEMP_SENS];  // последние показания датчика температуры и влажности

#define state_led_pin PIN13
#define SENS_CHECK_TIMEOUT 100
#define SENS_TIMEOUT 500
#define TEMP_TIMEOUT (60000 * 5)       // проверка температуры и влажности раз в 5 минут
#define WATCHDOG_TIMEOUT (60000 * 60)  // дежурные пакеты
#define COMMAND_TIMEOUT (60000 * 1)    // проверка команд для выполнения
#define BOILER_TIMEOUT (60000 * 1)

#define MAX_ALARMS 9
#define ALARM_ON 1
#define ALARM_OFF 0

typedef struct {
  byte id;
  byte value;
  byte pre_value;
  byte norm_state;
  byte on;
  byte pin;
  unsigned long change_time;
  bool analog;
  bool check_for_any_status;  // проверять при любом статусе системы
  bool status_led_no_change;  // при изменении статуса датчика не менять состояние светодиода
} alarm_info;

struct DATA {
  struct {
    byte sysState;
    byte ledState;
    byte tmp_value;
    alarm_info a[MAX_ALARMS] = {
      { 1, 0, 0, LOW, ALARM_OFF, PIN6, 0, false, false, false },  //pir1
      { 2, 0, 0, LOW, ALARM_ON, PIN5, 0, false, false, false },   //pir2 гараж
      { 3, 0, 0, HIGH, ALARM_ON, PIN7, 0, false, false, false },  //дверь № 1
      { 4, 0, 0, HIGH, ALARM_ON, PIN8, 0, false, false, false },  //дверь № 2
      { 5, 0, 0, LOW, ALARM_ON, PIN33, 0, false, true, true },    // наличие питания // new
      { 6, 0, 0, LOW, ALARM_OFF, PIN9, 0, false, false, false },  //pir3 кухня
      { 7, 0, 0, LOW, ALARM_ON, PIN28, 0, false, true, true },    //уровень в дрен колодце
      { 8, 0, 0, LOW, ALARM_ON, PIN35, 0, false, false, false },  //pir4 площадка 2 этаж
      { 9, 1, 1, HIGH, ALARM_ON, PIN42, 0, false, true, true },   //уровень в бочке полива
    };
  };
} d;


bool powerAC_off = false;           // признак отсутствия внешнего напряжения 220В
float accum_DC_V;                   // напряжение на аккумуляторе БП
unsigned long powerAC_ON_OFF_Time;  // время отключения внешнего напряжения 220В

/*--------------------------------------------------------------------------------*/
bool watchDogOK_Sended2BD = 0;           // признак отправки дежурного пакета в БД
unsigned long lastWatchDogOK_Sended2BD;  // время отправки дежурного пакета в БД
short timerResetCounter;                 // количество сбросов таймера с начала работы (отслеживание перехода через 50 дней)
short timerResetDays;                    // количество дней до последнего сброса таймера (переходящее количество дней для вычисление общего количества дней)
unsigned long timerResetOstatok;         // переходящее количество тиков таймера


byte get_param_load_flag = 0;
byte checked_ip = 6;
byte tcp_last_byte[10] = { 9, 22, 18, 26, 28, 29 };  // список пингуемых ip
byte pump_force;                                     // =1 включить дренажный насос в установленное время независимо от значения датчика уровня
byte open_tap_time = 25,                             // в это время открыть кран для полива на 120 мин, если >= 24, то не открывать
  fill_tank_time = 25,                                // в это время открыть клапан для заполнение бочки на 30 мин, если >= 24 или уровень == 0, то не открывать
  ip_ping_reboot = 0;                                // если пинги контролируемых устройств не прошли (1 раз/час), то перегрузить их

void blinky_check();
void sens_check();
void temp_check();
void command_check();
void remoteTermostat_check();
void sendError_check();
void sendBuffer2Site_check();
void checkPump_check();

float readDallasTemp(DallasTemperature *d);
void remoteRebootExecute(int act);
float getTemp(short tempSensorId);

void fill_tank_check();
void open_tap_check();

Activity state_led_blink(1000, blinky_check);
Activity sens(SENS_CHECK_TIMEOUT, sens_check);
Activity tempHum(TEMP_TIMEOUT, temp_check);
Activity readCommand(COMMAND_TIMEOUT, command_check);
Activity remoteTermostat(BOILER_TIMEOUT, remoteTermostat_check);
Activity sendError(COMMAND_TIMEOUT, sendError_check);
Activity sendBuffer2Site(2000, sendBuffer2Site_check);  // передача буфера информации раз в 2 секунду (если есть что)
Activity checkPump(((60000 * 60)), checkPump_check);
Activity check_fill_tank((60000), fill_tank_check);
Activity check_open_tap((60000), open_tap_check);

#define DC_12V_ON_PIN 25
#define VALVE_ON_PIN 38  //(TEMP_PIN+1)
#define SPRINKLING_ON_PIN 39
#define LEVEL_PIN 42              //(TEMP_PIN+2)
#define VALVE_OR_WATERTAP_PIN 40  //PIN (TEMP_PIN+3)
#define WATERTAP_ON_PIN 41        //(TEMP_PIN+4)

//--------------------------------------------------------------------------------
#include "util.h"
#include "device.h"

class Boiler
  pump(PUMP_CMD, 26),
  boiler(BOILER_CMD, 23, 1, 22),
  heating_cable(HEAT_CABLE_CMD, 27, 4),
  vegetableStorage(HVS_CMD, 31, 7, 0, 5),
  fill_tank(FILL_TANK_CMD, VALVE_ON_PIN),
  open_tap(OPEN_TAB_CMD, 0, 0, 0, 0),
  sprinkling(SPRINKLING_CMD, SPRINKLING_ON_PIN);

//------------------------------------------------------------------------
void get_param() {
  esp.send2site(F("get_param.php"));  // прочитать параметры
  String str = F("Checked_IP=");
  str += checked_ip;
  str += F("(");
  for (short i = 0; i < checked_ip; i++) {
    str += tcp_last_byte[i];
    str += ((i == checked_ip - 1) ? ")" : ",");
  }
  str += F(" poliv=");
  str += open_tap_time;
  trace(str);
  get_param_load_flag = 1;
}

//------------------------------------------------------------------------
void sens_setup() {
  for (byte i = 0; i < MAX_ALARMS; i++) {
    if (d.a[i].on == ALARM_OFF)
      continue;
    if (d.a[i].analog != true) {
      pinMode(d.a[i].pin, INPUT);
      digitalWrite(d.a[i].pin, HIGH);  // подключение внутр резистора
    }
    d.a[i].pre_value = d.a[i].norm_state;
    d.a[i].value = d.a[i].norm_state;

    trace_begin(F("Sens init! id="));
    trace_i(d.a[i].id);
    trace_s(F(" v="));
    trace_i(d.tmp_value);
    trace_s(F(" v2="));
    trace_i(d.a[i].pre_value);
    trace_end();
  }

  //pinMode(5, INPUT);
}

//------------------------------------------------------------------------
void sens_check() {
  unsigned long currentMillis = millis();

  for (byte i = 0; i < MAX_ALARMS; i++) {
    if (d.a[i].on == ALARM_OFF || (!d.sysState && !d.a[i].check_for_any_status))  // не активен или проверка датчиков отключена и датчик можно не проверять
      continue;
    if (d.a[i].analog == false)
      d.tmp_value = digitalRead(d.a[i].pin);
    else {
      d.tmp_value = analogRead(d.a[i].pin);
      //trace(String(F("Analog Sens check! id=")) + String(d.a[i].id) + " v=" + String(d.tmp_value));
      if (d.tmp_value > 10)
        d.tmp_value = 1;
      else
        d.tmp_value = 0;
    }
    if (d.tmp_value != d.a[i].pre_value) {
      d.a[i].pre_value = d.tmp_value;
      d.a[i].change_time = currentMillis;
      //trace("Sens check! id=" + String(d.a[i].id) + " v=" + String(d.a[i].pre_value));
    }

    if (d.a[i].change_time > 0 && (currentMillis - d.a[i].change_time) > SENS_TIMEOUT) {
      d.a[i].value = d.a[i].pre_value;
      d.a[i].change_time = 0;
      esp.addSens2Buffer(d.a[i].id, d.a[i].value);

      trace_begin(F("Sens changed! id="));
      trace_i(d.a[i].id);
      trace_s(F(" v="));
      trace_i(d.a[i].value);
      trace_end();

      if (d.a[i].id == 5) { /* izh 28-10-2018 */
        powerAC_off = !d.a[i].value;
        powerAC_ON_OFF_Time = millis();
      }
    }
  }

  short new_ledInterval = 1000;

  for (byte i = 0; i < MAX_ALARMS; i++) {
    if (d.a[i].on == ALARM_OFF                            // не активен
        || (!d.sysState && !d.a[i].check_for_any_status)  // проверка датчиков отключена и датчик можно не проверять
        || d.a[i].status_led_no_change == true            // при изменении статуса датчика не менять состояние светодиода
    )
      continue;

    if (d.a[i].change_time != 0) {
      new_ledInterval = state_led_blink.timeout;
      break;
    }
    if (d.a[i].value != d.a[i].norm_state) {
      new_ledInterval = 100;
      break;
    }
  }
  if (state_led_blink.timeout != new_ledInterval) {
    state_led_blink.timeout = new_ledInterval;
  }
}

//------------------------------------------------------------------------
float readDallasTemp(DallasTemperature *d) {
  float ft;
  d->requestTemperaturesByIndex(0);  // Send the command to get temperatures
  for (short ii = 0; ii < 10; ii++) {
    ft = d->getTempCByIndex(0);
    if (ft > -100) {
      break;
    }

    //izh 25-07-2024 вызывает странную проблему dallasTemp[ii].begin();  // повторная инициализация, часто помогает
    delay(50);
  }
  return ft;
}
//------------------------------------------------------------------------
float getTemp(short tempSensorId) {
  if (tempSensorId < 1 || tempSensorId > MAX_TEMP_SENS)
    return -127;
  if (tempSensorId < 4) {
    return dht[tempSensorId - 1].readTemperature();
  } else {
    return readDallasTemp(&dallasTemp[tempSensorId - 4]);
  }
}
//------------------------------------------------------------------------
void temp_check() {
  // считывание температуры или влажности занимает примерно 250 мс!
  // считанные показания могут отличаться от актуальных примерно на 2 секунды (это очень медленный датчик)

  short h, t;
  for (short i = 0; i < MAX_TEMP_SENS; i++) {
    if (i < 3) {
      h = round(dht[i].readHumidity());
      if (dht[i].state == false) {
        trace(String(F("Ошибка чтения температуры для id=")) + String(i + 1) + "!");
        continue;
      }
      t = round(dht[i].readTemperature());
    } else {
      h = 0;
      int ind = i - 3;
      //trace("0.1 currBufLen="+String(strlen(esp.buffer)) +" buffer=" + String(esp.buffer));
      t = round(readDallasTemp(&dallasTemp[ind]));
      //trace("0.2 currBufLen="+String(strlen(esp.buffer)) +" buffer=" + String(esp.buffer));
    }

    trace_begin(F("Темп и влажн. id="));
    trace_i(i + 1);
    trace_s(F(" t="));
    trace_i(t);
    trace_s(F(" h="));
    trace_i(h);
    trace_end();

    if (h != prevHum[i] || t != prevTemp[i]) {
      esp.addTempHum2Buffer(i + 1, t, h);
      prevHum[i] = h;
      prevTemp[i] = t;
    }
  }
}

//------------------------------------------------------------------------
void command_check() {
  esp.send2site(F("get_command.php"));  // проверка наличия команд
}

//------------------------------------------------------------------------
void blinky_check() {
  if (!d.sysState)
    return;
  d.ledState = !d.ledState;                 // если светодиод не горит, то зажигаем, и наоборот
  digitalWrite(state_led_pin, d.ledState);  // устанавливаем состояния выхода, чтобы включить или выключить светодиод
}

// ---------------------------------------------------обработчик прерывания, 2 мс
void timerInterrupt() {
  //  wdt_reset();  // сброс сторожевого таймера
}

//------------------------------------------------------------------------
// Обработка принятой строки
//------------------------------------------------------------------------
void responseProcessing(const String &response) {

  short ind = response.indexOf(F("command="));  // признак команды
  short ind2;

  if (ind >= 0) {
    ind += 8;                           // длина признака команды
    ind2 = response.indexOf(";", ind);  // поиск первой точки-запятой
    if (ind2 >= 0) {
      String cmd = response.substring(ind, ind2);
      {
        String str = F("cmd=");
        str += cmd;
        trace(str);
        esp.addEvent2Buffer(7, str);
      }

      if (cmd == F("reboot_router"))
        remoteRebootExecute(1);
      else if (cmd == F("reboot"))
        remoteRebootExecute(2);
      else if (cmd == F("sensor_ignore")) {
        d.sysState = 0;
        d.ledState = 0;
        digitalWrite(state_led_pin, d.ledState);
      } else if (cmd == F("sensor_check")) {
        d.sysState = 1;
      } else if (cmd == F("boiler_stop")) {
        if (boiler.ControlOn)
          boiler.ControlUntilTime = 0;
      } else if (cmd == F("heating_cable_stop")) {
        if (heating_cable.ControlOn)
          heating_cable.ControlUntilTime = 0;
      } else if (cmd == F("heating_vegetable_storage_stop")) {
        if (vegetableStorage.ControlOn)
          vegetableStorage.ControlUntilTime = 0;
      } else if (cmd == F("pump_stop")) {
        if (pump.ControlOn)
          pump.ControlUntilTime = 0;

      } else if (cmd == F("pump")) {
        pump.init(response, ind2);
      } else if (cmd == F("boiler")) {
        boiler.init(response, ind2);
      } else if (cmd == F("heating_cable")) {
        heating_cable.init(response, ind2);
      } else if (cmd == F("heating_vegetable_storage")) {
        vegetableStorage.init(response, ind2);
      } else
        if (cmd == F("fill_tank")){
          if(!fill_tank.ControlOn && !open_tap.ControlOn) {  // izh 17-06-2024 это реле работает от внешнего питания, которое подается при включении  12В DC_12V_ON_PIN
            pinMode(DC_12V_ON_PIN, OUTPUT);                                              //
            digitalWrite(DC_12V_ON_PIN, LOW);                                            // включаем внешнее питание реле и питание для клапана (1) или крана (0)
            pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
            digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // выбираем питание для клапана (1)
            pinMode(VALVE_ON_PIN, OUTPUT);
            digitalWrite(VALVE_ON_PIN, HIGH);  // пока выключаем питание клапана
            fill_tank.init(response, ind2, 1);
            check_fill_tank.timeout = 2000;  // сделать вызов процедуры обработки раз в 2 сек
          }
      } else
        if (cmd == F("open_tap")){
          if(!open_tap.ControlOn && !fill_tank.ControlOn) {  // izh 17-06-2024 это реле работает от внешнего питания, которое подается при включении  12В DC_12V_ON_PIN
            pinMode(VALVE_OR_WATERTAP_PIN, INPUT_PULLUP );
            pinMode(WATERTAP_ON_PIN, INPUT_PULLUP );
            pinMode(DC_12V_ON_PIN, INPUT_PULLUP );
/*            
            pinMode(DC_12V_ON_PIN, OUTPUT);
            digitalWrite(DC_12V_ON_PIN, LOW);  // включаем внешнее питание реле и питание для клапана (1) или крана (0)
            pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
            digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // пока выбираем питание для клапана (1) (выключаем питание крана)
            pinMode(WATERTAP_ON_PIN, OUTPUT);
            digitalWrite(WATERTAP_ON_PIN, HIGH);  // пока ставим в положение закрыто (+12В-12В)
*/            
            open_tap.init(response, ind2, 1);
            check_open_tap.timeout = 10000;  // сделать вызов процедуры обработки раз в 10 сек
          }
      } else

        if (cmd == F("sprinkling") && !sprinkling.ControlOn) {  // izh 17-06-2024 это реле работает от внешнего питания, которое подается при включении  12В DC_12V_ON_PIN
        pinMode(DC_12V_ON_PIN, OUTPUT);                         //
        digitalWrite(DC_12V_ON_PIN, LOW);                       // включаем внешнее питание реле и питание для клапана (1) или крана (0)
        pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
        digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // выбираем питание для клапана (1)
        pinMode(SPRINKLING_ON_PIN, OUTPUT);
        digitalWrite(SPRINKLING_ON_PIN, HIGH);  // пока выключаем питание клапана
        sprinkling.init(response, ind2, 1);

      } else if (cmd == F("fill_tank_stop")) {
        if (fill_tank.ControlOn)
          fill_tank.ControlUntilTime = 0;
      } else if (cmd == F("open_tap_stop")) {
        if (open_tap.ControlOn)
          open_tap.ControlUntilTime = 0;
      } else if (cmd == F("sprinkling_stop")) {
        if (sprinkling.ControlOn)
          sprinkling.ControlUntilTime = 0;
      }

      else if (cmd == F("setdatetime")) {  // izh 19-02-2020 проверка местного времени, если надо корректировка
        ind2 += 1;
        ind = response.indexOf(";", ind2);
        String date_str = response.substring(ind2, ind);
        ind += 1;
        ind2 = response.indexOf(";", ind);
        String time_str = response.substring(ind, ind2);

        DateTime dt1(RTC.now());
        DateTime dt2(date_str.c_str(), time_str.c_str());
        {
          String str = F("TimeCheckCorr:");
          str += date_str + " " + time_str;
          trace(str);
          esp.addEvent2Buffer(12, str);
        }
        if (dt1.year() != dt2.year() || dt1.month() != dt2.month() || dt1.day() != dt2.day() || dt1.hour() != dt2.hour() || dt1.minute() != dt2.minute() || dt1.second() != dt2.second()) {
          RTC.adjust(dt2);
          esp.addEvent2Buffer(12, F("Time update."));
        }
      }
    }
  } else {                             // izh 29-03-2022 считать параметры
    ind = response.indexOf("param_");  // признак команды
    if (ind >= 0) {
      trace(F("Считать параметры."));
      while (1) {
        ind2 = response.indexOf("=", ind);
        if (ind2 == -1)
          break;
        String ParamName = response.substring(ind, ind2);
        ParamName.trim();
        trace("name=" + ParamName);
        ind = response.indexOf(";", ind2);
        if (ind == -1)
          break;
        String ParamValue = response.substring(++ind2, ind);
        ind++;
        trace("value=" + ParamValue);
        if (ParamName == "param_ping") {  // izh 29-03-2022 считать список пингуемых ip-шников из БД
          checked_ip = 0;
          int ind01 = 0;
          int ind02;

          for (short i = 0; i < 10; i++) {
            ind02 = ParamValue.indexOf(",", ind01);
            if (ind02 == -1)
              ind02 = ParamValue.length();
            tcp_last_byte[i] = ParamValue.substring(ind01, ind02).toInt();
            ind01 = ind02 + 1;
            checked_ip = i + 1;
            if (ind02 == ParamValue.length())
              break;
          }
        } else if (ParamName == F("param_pump_force")) {
          pump_force = ParamValue.substring(0, 1).toInt();
          //Serial.println(pump_force);
        } else if (ParamName == F("fill_tank_time")) {
          fill_tank_time = ParamValue.toInt();
        } else if (ParamName == F("open_tap_time")) {
          open_tap_time = ParamValue.toInt();
        } else if (ParamName == F("ip_ping_reboot")) {
          ip_ping_reboot = ParamValue.toInt();
        }
      }
    }
  }
  checkMemoryFree();
}

//------------------------------------------------------------------------
void fill_tank_check()  // запускается один раз в 2 сек, когда работает команда
{
  if (fill_tank.ControlOn) {
    if (millis() - fill_tank.putInfoLastTime > 60000) {
      trace_begin(F("fill_tank_check level="));
      trace_i(d.a[8].value);
      trace_end();
    }
    fill_tank.processing();
    if (fill_tank.ControlOn == 0) {  // закончили работу
      check_fill_tank.timeout = 60000;
      digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // все в исходное состояние
      digitalWrite(DC_12V_ON_PIN, HIGH);
      pinMode(VALVE_OR_WATERTAP_PIN, INPUT);
      pinMode(DC_12V_ON_PIN, INPUT);
      fill_tank.pin2 == 0;
    } else {  // в работе

      if (d.a[8].value == 0) {     // бак полон
        if (fill_tank.pin2 == -1)  // это не первый всплеск
          responseProcessing(F("command=fill_tank_stop;"));
        else
          fill_tank.pin2 = -1;  // компенсировать первый всплеск
      }
    }
  }
}

//------------------------------------------------------------------------
void open_tap_check()  // запускается один раз в 20 сек, когда работает команда
{
  if (open_tap.ControlOn) {
    bool saved = open_tap.CurrentMode;  // текущий режим, если == false, значит начало работы
    if (millis() - open_tap.putInfoLastTime > 60000) {
      trace_begin(F("open_tap=3 mode="));
      trace_i(open_tap.pin2);
      trace_end();
    }
    open_tap.processing();
    
    if (open_tap.ControlOn == 0) {                       // конец полива
      if (open_tap.pin2 == -1 || open_tap.pin2 == -2) {  // кран еще не закрыт, нужно закрыть
        trace_begin(F("open_tap=4 mode="));
        trace_i(open_tap.pin2);
        trace_end();
        
        digitalWrite(DC_12V_ON_PIN, LOW);          // подаем питание 12В на реле и кран/клапан
        digitalWrite(WATERTAP_ON_PIN, HIGH);       // меняем полярность на закрытие крана
        digitalWrite(VALVE_OR_WATERTAP_PIN, LOW);  // переключаем питание с клапана на кран

        pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
        pinMode(WATERTAP_ON_PIN, OUTPUT);
        pinMode(DC_12V_ON_PIN, OUTPUT);

        open_tap.ControlOn = true;                     // включаем команду
        open_tap.CurrentMode = true;                   // возвращаем команду в активный режем
        open_tap.tmpWorkTime = millis();               // начало активного периода 
        open_tap.ControlUntilTime = millis() + 20000;  // продляем еще работу на 20 сек чтобы кран успел закрыться
        open_tap.pin2 = 0;                             // теперь точно конец работы команды
      } else {                                         // теперь точно все
        trace(F("open_tap=5"));

        digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // выключаем реле переключаем питание с крана на клапан
        digitalWrite(WATERTAP_ON_PIN, HIGH);        // выключаем реле смены полярности
        digitalWrite(DC_12V_ON_PIN, HIGH);          // выключаем реле подачи питание 12В на реле и кран/клапан

        pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
        pinMode(WATERTAP_ON_PIN, OUTPUT);
        pinMode(DC_12V_ON_PIN, OUTPUT);
        
        pinMode(VALVE_OR_WATERTAP_PIN, INPUT_PULLUP );
        pinMode(WATERTAP_ON_PIN, INPUT_PULLUP );
        pinMode(DC_12V_ON_PIN, INPUT_PULLUP );
        check_open_tap.timeout = 60000;
      }
    } else {
      if (!saved && open_tap.CurrentMode) {        // начало работы полива, первый проход
        trace(F("open_tap=1"));
        digitalWrite(WATERTAP_ON_PIN, LOW);        // переключаем полярность на открывание крана (-12В+12В)
        digitalWrite(VALVE_OR_WATERTAP_PIN, LOW);  // переключаем питание с клапана на кран
        digitalWrite(DC_12V_ON_PIN, LOW);          // включаем внешнее питание реле и питание для клапана (1) или крана (0)

        pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
        pinMode(WATERTAP_ON_PIN, OUTPUT);
        pinMode(DC_12V_ON_PIN, OUTPUT);
        
        open_tap.pin2 = -1;                        // кран открывается ~5-10 секунд, после этого питание крана и реле можно отключить до момента закрытия крана после полива, выставляем признак на снятие питания
      } else if (open_tap.pin2 == -1) {
        trace(F("open_tap=2"));
        digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // кран наверное уже открылся, переключаем питание с крана на клапан
        digitalWrite(WATERTAP_ON_PIN, HIGH);        // выключаем реле смены полярности
        digitalWrite(DC_12V_ON_PIN, HIGH);          // отключаем питание 12В
 
        pinMode(VALVE_OR_WATERTAP_PIN, OUTPUT);
        pinMode(WATERTAP_ON_PIN, OUTPUT);
        pinMode(DC_12V_ON_PIN, OUTPUT);
        
        pinMode(VALVE_OR_WATERTAP_PIN, INPUT_PULLUP );
        pinMode(WATERTAP_ON_PIN, INPUT_PULLUP );
        pinMode(DC_12V_ON_PIN, INPUT_PULLUP );

        open_tap.pin2 = -2;                         // признак закрытия крана по окончанию полива
        open_tap.CurrentMode = false;               // переводим команду в холостой режем 
        open_tap.activeWorkTime += millis() - open_tap.tmpWorkTime;
        open_tap.TargetTemp = -200;                 // что бы не вошли в активный режим
      }
    }
  }
}

//------------------------------------------------------------------------
// удаленная перезагрузка всех устройств
void remoteRebootExecute(int act) {
  int pin = (act == 1) ? PIN24 : PIN30;  // 24 - роутер; 30 - камеры, регистратор
  trace(F("Rebooting..."));
  pinMode(pin, OUTPUT);

  digitalWrite(pin, LOW);

  delay(1000 * 10);  // 10 сек
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);

  trace(F("Rebooted."));
}

//------------------------------------------------------------------------
// проверка напряжения на аккумуляторе
void checkAccumDC() {

  int analogInput = 3;
  float vout = 0.0;
  float R1 = 96600.0;  // resistance of R1 (100K) -see text!
  float R2 = 11600.0;  // resistance of R2 (10K) — see text!
  int value = 0;
  int pinVal = 1;

  pinMode(analogInput, INPUT);
  value = analogRead(analogInput);
  vout = (value * 4.9) / 1024.0;  // see text
  accum_DC_V = vout / (R2 / (R1 + R2));
  if (accum_DC_V < 0.09) {
    accum_DC_V = 0.0;  //statement to quash undesired reading !
  }
  //trace("VIN=" + String(accum_DC_V));
}

//------------------------------------------------------------------------
// функция термостата газового котла и не только
void remoteTermostat_check() {
  checkAccumDC();
  if (pump.ControlOn)
    pump.processing();
  if (boiler.ControlOn)
    boiler.processing();
  if (heating_cable.ControlOn)
    heating_cable.processing();
  if (vegetableStorage.ControlOn)
    vegetableStorage.processing();
  if (sprinkling.ControlOn) {
    sprinkling.processing();
    if (sprinkling.ControlOn == 0) {              // закончили работу
      digitalWrite(VALVE_OR_WATERTAP_PIN, HIGH);  // все в исходное состояние
      digitalWrite(DC_12V_ON_PIN, HIGH);
      pinMode(VALVE_OR_WATERTAP_PIN, INPUT);
      pinMode(DC_12V_ON_PIN, INPUT);
    }
  }
}

//------------------------------------------------------------------------
// проверка наличия пакета ошибок передач по WIFI если надо - попытка перезагрузки, запуск раз в 10 мин
void sendError_check() {
  if (esp.sendError_check()) {
    remoteRebootExecute(1);
  }
}

//------------------------------------------------------------------------
void sendBuffer2Site_check() {
  bool result = esp.sendBuffer2Site();
  if( result){ // если была передача данных на сайт
    if(millis() - readCommand.lastActivated > (readCommand.timeout - 1000 )){ // и сразу после этого ожидается запрос к сайту по активным командам
        readCommand.lastActivated += 5000; // то отложим этот запрос на 5 секунд чтобы небыло блокировки от провайдера (503 Service Unavailable)
      }
    }
}

//------------------------------------------------------------------------
void checkPump_check()  // запускается один раз в час
{
  DateTime now = RTC.now();
  if (now.hour() == 5 && (d.a[6].value == 1 || pump_force == 1)) {  // в 5 утра если установлен датчик уровня или принудительное включение -> включить насос
    responseProcessing(F("command=pump;15;"));
  }
  if (fill_tank_time < 24 && now.hour() == fill_tank_time && d.a[8].value == 1) {  // наполнить бочку если не полная
    responseProcessing(F("command=fill_tank;30;"));
  }
  //trace("open_tap_time="+String(open_tap_time) + " now.hour=" + String(now.hour()));
  if (open_tap_time < 24 && now.hour() == open_tap_time) {  // полить в теплице
    responseProcessing(F("command=open_tap;10;"));
  }
  //esp.addEvent2Buffer(12, "hour=" + String(now.hour()));
  if (now.hour() == 0) {
    esp.send2site(F("get_date.php"));  // в 00 часа взять дату-время с сервера и если локальные часы не совпадают, то установить их по серверу
  }
  if (get_param_load_flag = 0 || now.hour() == 1) {
    get_param();  // прочитать параметры
  }
}

//------------------------------------------------------------------------
void esp_power_switch(bool p) {
  //return;
  if (p == true) {
    pinMode(PIN29, OUTPUT);
    digitalWrite(PIN29, LOW);
  } else {
    digitalWrite(PIN29, HIGH);
    pinMode(PIN29, INPUT);
  }
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void setup() {
  wdt_disable();  // бесполезная строка до которой не доходит выполнение при bootloop
  trace(F(VERSION));

  Wire.begin();
  RTC.begin();

  d.sysState = 1;
  d.ledState = LOW;  // этой переменной устанавливаем состояние светодиода
  pinMode(state_led_pin, OUTPUT);
  digitalWrite(state_led_pin, d.ledState);

  sens_setup();
  for (short i = 0; i < MAX_DALLAS_SENS; i++) {
    dallasTemp[i].begin();
  }

  if (esp.check_Wait_Internet()) {

    esp.addEvent2Buffer(1, "");
    esp.sendBuffer2Site();

    get_param();
  }

  //temp_check();
  //esp.closeConnect();
}

//------------------------------------------------------------------------
void loop() {
  //esp.checkIdle(); // not used
  
  sendBuffer2Site.checkActivated();
  readCommand.checkActivated();
  
  sendError.checkActivated();
  state_led_blink.checkActivated();
  sens.checkActivated();
  tempHum.checkActivated();
  
  remoteTermostat.checkActivated();
  checkPump.checkActivated();
  check_fill_tank.checkActivated();
  check_open_tap.checkActivated();
  checkMemoryFree();

  unsigned long t = millis();
  if (lastWatchDogOK_Sended2BD == 0         // первая проверка
      || (t % WATCHDOG_TIMEOUT) < 10000) {  // регулярная отправка дежурного сообщения ( раз в час )
    if (!watchDogOK_Sended2BD) {
      watchDogOK_Sended2BD = true;

      esp.checkInitialized();
      short ind;
      String dopInfo;
      dopInfo.reserve(255);
      dopInfo = "";
      for (ind = 0; ind < checked_ip; ind++) {  // пинги видеорегистратора и камер
        if (tcp_last_byte[ind]) {
          String cmd = F("AT+PING=\"192.168.0.");
          cmd += tcp_last_byte[ind];
          cmd += F("\"");
          if (!esp.espSendCommand(cmd, STATE::OK, 5000)) {
            if (dopInfo != "")
              dopInfo += ",";
            dopInfo += tcp_last_byte[ind];
          }
        }
      }
      if (dopInfo != "") {
        dopInfo = String(F("PingErr:")) + dopInfo + " ";
        if (ip_ping_reboot == 1)
          responseProcessing(F("command=reboot;"));
      }
      dopInfo += F("M=");
      dopInfo += (floor((((float)ramMemory / 1024)) * 100) / 100);
      dopInfo += F(" Snd=");
      dopInfo += esp.sendCounter_ForAll;
      dopInfo += F(" SndKB=");
      dopInfo += esp.bytesSended / 1024;
      if (esp.sendErrorCounter_ForAll) {
        dopInfo += F(" SErr=");
        dopInfo += esp.sendErrorCounter_ForAll;
      }
      if (esp.httpFailCounter) {
        dopInfo += F(" HttpErr=");
        dopInfo += esp.httpFailCounter;
      }
      if (esp.timeoutCounter) {
        dopInfo += F(" ToErr=");
        dopInfo += esp.timeoutCounter;
      }
      if (esp.buffOverCounter) {
        dopInfo += F(" BOvrErr=");
        dopInfo += esp.buffOverCounter;
      }
      if (esp.connectFailCounter) {
        dopInfo += F(" ConnErr=");
        dopInfo += esp.connectFailCounter;
      }

      if (esp.routerRebootCount) {
        dopInfo += F(" RR=");
        dopInfo += esp.routerRebootCount;
        dopInfo += F("(");
        dopInfo += ((t - esp.lastRouterReboot) / (60 * 60000));
        dopInfo += F("h.)");
      }

      const unsigned long ticksPerDay = 86400000;  // 1000 * 60 * 60 * 24;
      const unsigned long ticksPerHour = 3600000;  //1000 * 60 * 60;

      if (lastWatchDogOK_Sended2BD > t) {  // сброс таймера (> ~50 дней)
        timerResetCounter++;
        timerResetDays += 49;                           // количество дней до последнего сброса таймера (переходящее количество дней для вычисление общего количества дней)
        timerResetOstatok += 0xffffffff % ticksPerDay;  // переходящее количество тиков таймера
        timerResetDays += timerResetOstatok / ticksPerDay;
        timerResetOstatok = timerResetOstatok % ticksPerDay;
      }
      unsigned int d = t / ticksPerDay + (t % ticksPerDay + timerResetOstatok) / ticksPerDay + timerResetDays;
      unsigned int h = ((t % ticksPerDay + timerResetOstatok) % ticksPerDay) / ticksPerHour;

      lastWatchDogOK_Sended2BD = (t == 0) ? 1 : t;
      String str = F("T=");
      if (d > 0) {
        str += d;
        str += F("d.");
      }
      str += h;
      str += F("h. (");
      str += dopInfo;
      str += F(")");

      esp.addEvent2Buffer(3, str);
      traceInit = false;
      ramMemory = 10000;
    }
  } else
    watchDogOK_Sended2BD = false;
}
