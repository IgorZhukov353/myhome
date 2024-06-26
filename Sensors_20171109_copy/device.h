/* 
 Igor Zhukov (c)
 Created:       21-11-2023
 Last changed:  25-06-2024
*/
// id команд в таблице COMMAND
#define BOILER_CMD      1
#define PUMP_CMD        2
#define HEAT_CABLE_CMD  3
#define HVS_CMD         4
#define FILL_TANK_CMD   5
#define OPEN_TAB_CMD    6
#define REBOOT_ROUTER_CMD   7
#define REBOOT_CAM_CMD  8

//---------------------------------------------------------------------------
class DeviceControl {
  public:
    byte id;
    short pin;
    String name;
    bool ControlOn;                         // признак управления
    unsigned long ControlUntilTime;         // управлять до этого времени
    unsigned long totalWorkTime;            // общее время работы
    unsigned long activeWorkTime;           // активное время работы
    long activeWorkCount;                   // кол-во включений за время работы
    unsigned long tmpWorkTime;              // для вычисления активного время работы

    DeviceControl(short ppin) {
      pin = ppin;
      ControlOn = false;
      activeWorkTime = 0;
      activeWorkCount = 0;
    }
};

class Boiler : public DeviceControl {
  public:
    short pin2;
    short tempSensorId;               // номер датчика температуры
    short TargetTemp;                 // целевая температура
    float currTemp;                   // текущая температура
    short delta;                      // выключать при температуре (TargetTemp + delta)
    bool  CurrentMode;                // текущий режим ардуино-термостата
    unsigned long putInfoLastTime;    // время отправки инфо (1 раз в мин)
    
    Boiler(byte _id,short ppin, String pname, short ptempSensorId = 0, short ppin2 = 0, short pdelta = 0): DeviceControl(ppin)  {
      //  pin = ppin;
      id = _id;
      name = pname;
      pin2 = ppin2;
      tempSensorId = ptempSensorId;
      delta = pdelta;
      TargetTemp = 0;
    };
    void init(const String& response, short ind2, short parInHours=0)
    {
      trace(name + String(F(": init.")));
      putInfoLastTime = 0;
      //trace("2 response=" + response.substring(ind2, response.length()));
      
      short inHours;  // коэфициент время работы для насоса в минутах, для остальных в часах
      short ind;
      if ( name != "pump" && parInHours == 0) {  // у насоса нет целевой температуры
        ind2++;
        ind = response.indexOf(";", ind2);
        TargetTemp = response.substring(ind2, ind).toInt();
        inHours = 60;       // коэфициент время работы для насоса в часах
      }
      else {
        ind = ind2;
        inHours = 1;      // коэфициент время работы для насоса в минутах
      }
      ind++;
      ind2 = response.indexOf(";", ind);
      ControlUntilTime = response.substring(ind, ind2).toInt();
      if (!ControlUntilTime) {
        String err = name + String(F(": Error period reading!"));
        trace(err);
        esp.addEvent2Buffer(4, err);
        return;
      }
      ControlUntilTime = millis() + ControlUntilTime * 60000 * inHours;
      ControlOn = true;
      CurrentMode = false;
      if (pin > 0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH); // выключено (HIGH)
      }
      if (pin2 > 0) { // для работы с котлом
        pinMode(pin2, OUTPUT);
        digitalWrite(pin2, LOW); // включено (отключаем штатный термостат)
      }
      totalWorkTime = millis();
    };
    void processing()
    {
      if (millis() > ControlUntilTime) { // закончен период работы ардуино-термостата
        if (CurrentMode) {
          CurrentMode = false;
          activeWorkTime += millis() - tmpWorkTime;
        }
        ControlOn = false;
        putInfoLastTime = 0;
        if (pin > 0) {
          digitalWrite(pin, HIGH);
          pinMode(pin, INPUT);
        }
        if (pin2 > 0) {
          digitalWrite(pin2, HIGH);
          pinMode(pin2, INPUT);
        }
      }
      else {
        currTemp = ( tempSensorId > 0) ? getTemp(tempSensorId) : -100;

        if ( currTemp < TargetTemp     // если насос (не задан датчик температуры то здесь всегда ТРУЕ)
             && !CurrentMode) {  // если текущая температура ниже заданной и нагрев выключен - включить
          CurrentMode = true;
          if (pin > 0) {
            digitalWrite(pin, LOW);
          }
          activeWorkCount++;
          tmpWorkTime = millis();
        }
        else {
          if ((currTemp - delta) > TargetTemp && CurrentMode) { // если текущая температура выше заданной и нагрев включен - выключить
            CurrentMode = false;
            if (pin > 0) {
              digitalWrite(pin, HIGH);
            }
            activeWorkTime += millis() - tmpWorkTime;
          }
        }
      }
      if(millis() - putInfoLastTime > 60000){
        putInfo();
        putInfoLastTime = millis();
      }
    };
    void putInfo()
    {
      String str;
      str.reserve(100);
      //unsigned long ms = activeWorkTime + (CurrentMode)?millis() - tmpWorkTime:0;
      str = F("{\"id\":\"");
      str += String(id);
      str +=  F("\",\"l\":");
      str +=  String((ControlOn) ? ControlUntilTime - millis() : 0);
      str +=  F(",\"cnt\":");
      str +=  String(activeWorkCount);
      str +=  F(",\"w\":");
      str +=  String(ControlOn);
      str +=  F(",\"a\":");
      str +=  String(CurrentMode);
      str +=  F(",\"actt\":");
      str +=  String(activeWorkTime + ((CurrentMode) ? millis() - tmpWorkTime : 0));
      str +=  F(",\"ont\":");
      str +=  String(millis() - totalWorkTime);
      if(tempSensorId > 0)
        str += String(F(",\"t\":")) + String(TargetTemp) + String(F(",\"curt\" : \"")) + String(currTemp) + String(F("\""));
      str +=  F("}");
      trace( str);
      esp.addEvent2Buffer(8, str);
    }
};
