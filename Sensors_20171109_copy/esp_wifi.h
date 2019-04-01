/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  01-04-2019
*/
class ESP_WIFI {
//protected:  
  String buffer;
  
 public:
  bool wifi_initialized = false;
  unsigned long lastWIFISended;
  short sendErrorCounter;
  short routerConnectErrorCounter;

  int sendErrorCounter_ForAll;
  unsigned long sendCounter_ForAll;
  
  unsigned long bytesSended;

  ESP_WIFI();
  bool espSerialSetup();
  bool espSendCommand(String cmd, char* goodResponse, unsigned long timeout);
  bool send2site(String reqStr);
  bool _send2site(String reqStr, String postBuf);

  void addEvent2Buffer(short id, String msgText);
  void addTempHum2Buffer(short id, short temp, short hum);
  void addSens2Buffer(short id, short val);
  void addInfo2Buffer(String str);
  void sendBuffer2Site(); 

  void checkIdle(); // отключение в случае простоя
  bool checkInitialized();
  void check_Wait_Internet();
};
