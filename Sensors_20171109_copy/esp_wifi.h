/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  25-07-2024
*/
enum class STATE {OK = 0, ERR = 1, HTTP = 2, HTTP_OK = 3, CLOSED = 4};
enum class ErrorType {NONE=0,HTTP_FAIL=1,TIMEOUT=2,OTHER=3};
class ESP_WIFI {
//protected:  
  //String buffer;
  
  
 public:
 char buffer[1800];
 
  bool wifi_initialized = false;
  unsigned long lastWIFISended;
  short sendErrorCounter;
  short httpFailCounter;
  short buffOverCounter;
  short timeoutCounter;
  ErrorType lastErrorTypeId;
  
  short routerConnectErrorCounter;
  int routerRebootCount = 0;              // счетчик перезагрузок роутера
  unsigned long lastRouterReboot;         // время последней перезагрузки роутера

  int sendErrorCounter_ForAll;
  unsigned long sendCounter_ForAll;
  unsigned long bytesSended;
  short maxSendedMSG;

  ESP_WIFI();
  bool espSerialSetup();
  //bool espSendCommand(const String& cmd, const STATE goodResponse, const unsigned long timeout);
  bool espSendCommand(const String& cmd, const STATE goodResponse, const unsigned long timeout, const char *postBuf=NULL, const String &cmd2="");
  bool send2site(const String& reqStr);
  //bool _send2site(const String& reqStr, const String& postBuf);
  bool _send2site(const String& reqStr, const char * postBuf);
  void addEvent2Buffer(short id, const String& msgText);
  void addTempHum2Buffer(short id, short temp, short hum);
  void addSens2Buffer(short id, short val);
  //void addInfo2Buffer(const String& str);
  void addInfo2Buffer(const char *str);
  
  void sendBuffer2Site(); 

  void checkIdle(); // отключение в случае простоя
  bool checkInitialized();
  bool check_Wait_Internet();
  void closeConnect();
  bool sendError_check();
  bool ping(const String &host, short timeout=5000);
};
