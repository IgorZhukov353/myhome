/* 
 Igor Zhukov (c)
 Created:       21-01-2018
 Last changed:  21-01-2018
*/
class Activity { // класс для периодического выполнения заданной процедуры
 public:
  unsigned long lastActivated;  // время последнего выполнения процедуры
  unsigned long timeout;        // через какой промежуток времени выполнять (ms)
  void (*actionRunFunc)();      // указатель на процедуру, которую выполнять

  Activity(unsigned long pTimeout, void (*pFunc)()) {timeout = pTimeout; actionRunFunc = pFunc; lastActivated = 0;};
  void checkActivated() {if(millis() - lastActivated > timeout) {lastActivated = millis(); actionRunFunc();}};
 };

