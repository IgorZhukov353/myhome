/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  23-06-2020
*/
enum class STATE {OK = 0, ERR = 1, HTTP = 2, HTTP_OK = 3, CLOSED = 4};

class ESP_WIFI {
//protected:  
  String buffer;
  
 public:
  bool wifi_initialized = false;
  unsigned long lastWIFISended;
  short sendErrorCounter;
  short dnsFailCounter;
  bool dnsFail;
  short routerConnectErrorCounter;
  int routerRebootCount = 0;              // счетчик перезагрузок роутера
  unsigned long lastRouterReboot;         // время последней перезагрузки роутера

  int sendErrorCounter_ForAll;
  unsigned long sendCounter_ForAll;
  
  unsigned long bytesSended;

  ESP_WIFI();
  bool espSerialSetup();
  bool espSendCommand2(const String& cmd, char* goodResponse, unsigned long timeout);
  bool espSendCommand(const String& cmd, const STATE goodResponse, const unsigned long timeout);
  bool send2site(const String& reqStr);
  bool _send2site(const String& reqStr, const String& postBuf);

  void addEvent2Buffer(short id, const String& msgText);
  void addTempHum2Buffer(short id, short temp, short hum);
  void addSens2Buffer(short id, short val);
  void addInfo2Buffer(const String& str);
  void sendBuffer2Site(); 

  void checkIdle(); // отключение в случае простоя
  bool checkInitialized();
  void check_Wait_Internet();
  void closeConnect();
  bool sendError_check();
};
