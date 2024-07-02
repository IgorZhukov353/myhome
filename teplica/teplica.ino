#include <OneWire.h>
#include <DallasTemperature.h>

#define DC_12V_ON_PIN 25
#define TEMP_PIN 37
#define VALVE_ON_PIN 38 //(TEMP_PIN+1)
#define LEVEL_PIN 42 //39 //(TEMP_PIN+2)
#define VALVE_WATER_PIN 40 //PIN (TEMP_PIN+3)
#define WATER_ON_PIN 41 //(TEMP_PIN+4)
#define VALVE_ON_PIN2 39

OneWire oneWire(TEMP_PIN);  //
byte level=1, savelevel =1;
byte mode=0;

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature dallasTemp(&oneWire);

void setup() {
  Serial.begin(115200);
  pinMode(LEVEL_PIN, INPUT_PULLUP);
 dallasTemp.begin();
  
  Serial.println("1. Температура");
  Serial.println("2. Открыть клапан");
  Serial.println("3. Закрыть клапан");
  Serial.println("4. Открыть кран");
  Serial.println("5. Закрыть кран");
  Serial.println("6. Датчик уровня");
  Serial.println("7. Включить 12 В");
  Serial.println("8. Выключить 12 В");
  Serial.println("9. Открыть клапан 2");
  Serial.println("10. Закрыть клапан 2");
}
void loop() {
  int menu=0;
  Serial.println("Какой датчик вы хотели бы прочитать? ");
  while (Serial.available() == 0) {
    level = digitalRead(LEVEL_PIN);
    if(level != savelevel){
      savelevel = level;
      level = digitalRead(LEVEL_PIN);
      Serial.println("level="+String(level));
    }
    if(mode==2 && level == 0){
      Serial.println("Емкость заполнена.");
      menu = 3;
      break;
      }
  }
  
  if(menu == 0)
    menu = Serial.parseInt();
    
  switch (menu) {
    case 1:
      // код для датчика температуры
        float ft;
    dallasTemp.requestTemperaturesByIndex(0);  // Send the command to get temperatures
      ft = dallasTemp.getTempCByIndex(0);

      Serial.print("Температура: ");
      Serial.println(ft);
      break;
    case 2:
      //
      Serial.println("Клапан открыт.");
      digitalWrite(VALVE_WATER_PIN, HIGH);
      pinMode(VALVE_WATER_PIN, INPUT);
      
      pinMode(VALVE_ON_PIN, OUTPUT);
      digitalWrite(VALVE_ON_PIN, LOW);
      mode = 2;
  
      //Serial.println(Rh);
      break;
    case 3:
      //
      Serial.println("Клапан закрыт.");
      digitalWrite(VALVE_WATER_PIN, HIGH);
      pinMode(VALVE_WATER_PIN, INPUT);
      digitalWrite(VALVE_ON_PIN, HIGH);
      pinMode(VALVE_ON_PIN, INPUT);
      mode = 0;
      break;
    case 4:
      //
      Serial.println("Кран открыт.");
      pinMode(VALVE_WATER_PIN, OUTPUT);
      digitalWrite(VALVE_WATER_PIN, HIGH);
      pinMode(VALVE_WATER_PIN, INPUT);
      
      pinMode(WATER_ON_PIN, OUTPUT);
      digitalWrite(WATER_ON_PIN, LOW);
    //  pinMode(WATER_ON_PIN1, OUTPUT);
    //  digitalWrite(WATER_ON_PIN1, LOW);
      
      pinMode(VALVE_WATER_PIN, OUTPUT);
      digitalWrite(VALVE_WATER_PIN, LOW);
  
      //Serial.println(Rh);
      break;
    case 5:
      //
      Serial.println("Кран закрыт.");
       pinMode(VALVE_WATER_PIN, OUTPUT);
      digitalWrite(VALVE_WATER_PIN, HIGH);
      pinMode(VALVE_WATER_PIN, INPUT);

      pinMode(WATER_ON_PIN, OUTPUT);
      digitalWrite(WATER_ON_PIN, HIGH);
      pinMode(WATER_ON_PIN, INPUT);
    //   pinMode(WATER_ON_PIN1, OUTPUT);
    //  digitalWrite(WATER_ON_PIN1, HIGH);
   //   pinMode(WATER_ON_PIN1, INPUT);
      pinMode(VALVE_WATER_PIN, OUTPUT);
      digitalWrite(VALVE_WATER_PIN, LOW);
  
      //Serial.println(Rh);
      break;
    case 6:
      //
      byte i;
      i = digitalRead(LEVEL_PIN);
      Serial.print("Датчик уровня:");
      Serial.println(i);
      break;
    case 7:
      //
      Serial.println("12В включено.");
      pinMode(DC_12V_ON_PIN, OUTPUT);
      digitalWrite(DC_12V_ON_PIN, LOW);
      
      break;
    case 8:
      //
      Serial.println("12В выключено.");
      digitalWrite(DC_12V_ON_PIN, HIGH);
      pinMode(DC_12V_ON_PIN, INPUT);
      break;
    case 9:
      //
      Serial.println("Клапан 2 открыт.");
       pinMode(VALVE_WATER_PIN, OUTPUT);
      digitalWrite(VALVE_WATER_PIN, HIGH);
      pinMode(VALVE_WATER_PIN, INPUT);
      
      pinMode(VALVE_ON_PIN, OUTPUT);
      digitalWrite(VALVE_ON_PIN, HIGH);
      
      pinMode(VALVE_ON_PIN2, OUTPUT);
      digitalWrite(VALVE_ON_PIN2, LOW);
      
  
      //Serial.println(Rh);
      break;
      case 10:
      //
      Serial.println("Клапан 2 закрыт.");
      digitalWrite(VALVE_WATER_PIN, HIGH);
      pinMode(VALVE_WATER_PIN, INPUT);
      digitalWrite(VALVE_ON_PIN2, HIGH);
      pinMode(VALVE_ON_PIN2, INPUT);
      mode = 0;
      break;
    default:
      Serial.println("Пожалуйста, сделайте свой выбор 1,2,3,4,5,6");
  }
  
}
