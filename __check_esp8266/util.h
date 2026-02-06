#define trace_begin(str) (trace(str,1));
int checkMemoryFree();
void trace(const String& str, byte mode = 0);
void trace_l(const long i);
void trace_i(const int i);
void trace_c(const char *c);
void trace_s(const String &s);
void trace_end();
String getCurrentDate(byte noYear = 1);
void wdt_delay(int t);
#ifndef __util_h__
extern bool traceInit;   // признак инициализации трассировки
extern short ramMemory;
#ifdef _RTCLIB_H_
extern RTC_DS1307 RTC;  // часы реального времени
#endif
#endif
