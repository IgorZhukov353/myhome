/* 
 Igor Zhukov (c)
 Created:       01-03-2024
 Last changed:  10-03-2024
*/

void trace(String str);
int memoryFree();

//---------------------------------------------------------------------------
class APP {
char WSSID[20], WPASS[10], HOST_STR[25], HOST_IP_STR[15]; 

public:
  void configRead();
  void addEvent2Buffer(int p1, String str) { if (p1 == 4) trace(str);}
  
};

enum _amount {TEMPER_MAX=7,LED_MAX=1,PIN_MAX=10};
#define ALL_MAX (TEMPER_MAX+LED_MAX+PIN_MAX) 

#ifdef MAIN
TempSensor temper[TEMPER_MAX];
LED led[LED_MAX];
PIN p[PIN_MAX];
Sensor * s[ALL_MAX];
SensorArray sa;
APP app;
LED *sysledptr;
#else
extern TempSensor temper[TEMPER_MAX];
extern LED led[LED_MAX];
extern PIN p[PIN_MAX];
extern Sensor * s[ALL_MAX];
extern SensorArray sa;
extern APP app;
extern LED *sysledptr;

#endif