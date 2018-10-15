/* 
 Igor Zhukov (c)
 Created:       01-11-2017
 Last changed:  21-01-2018
*/
class ESP_WIFI {
  String buffer;
    
 public:
  bool wifi_initialized = false;
  unsigned long lastWIFISended;
  short sendErrorCounter;
  short routerConnectErrorCounter;

  ESP_WIFI();
  bool espSerialSetup();
  bool espSendCommand(String cmd, char* goodResponse, unsigned long timeout);
  bool send2site(String reqStr);
  bool send2site(String reqStr,String postBuf);
  virtual void trace(String msg) = 0;
  void checkIdle(); // отключение в случае простоя
  bool checkInitialized();
  virtual void responseProcessing(String resp) = 0;

  void addEvent2Buffer(short id, String msgText);
  void addTempHum2Buffer(short id, short temp, short hum);
  void addSens2Buffer(short id, short val);
  void addInfo2Buffer(String str);
  void sendBuffer2Site(); 

};
