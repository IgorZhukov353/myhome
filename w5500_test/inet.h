/* 
 Igor Zhukov (c)
 Created:       04-06-2019
 Last changed:  04-06-2019
*/

class inet_sender {
 virtual bool init()=0;
 virtual bool connect(const char *host, uint16_t port)=0;	
 virtual int available()=0;
 virtual int read()=0;
};

class inet {
//protected:  
  String buffer;
  
 public:
  inet_sender *sender;
  bool initialized = false;
  unsigned long lastSended;
  short sendErrorCounter;
  short routerConnectErrorCounter;

  int sendErrorCounter_ForAll;
  unsigned long sendCounter_ForAll;
  
  unsigned long bytesSended;

  inet();
  virtual bool setup();
  virtual bool sendCommand(String cmd, char* goodResponse, unsigned long timeout);
  virtual bool send2site(String reqStr);
  virtual bool _send2site(String reqStr, String postBuf);

  void addEvent2Buffer(short id, String msgText);
  void addTempHum2Buffer(short id, short temp, short hum);
  void addSens2Buffer(short id, short val);
  void addInfo2Buffer(String str);
  void sendBuffer2Site(); 

  void checkIdle(); // отключение в случае простоя
  bool checkInitialized();
  void check_Wait_Internet();
};
