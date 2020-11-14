/* 
 Igor Zhukov (c)
 Created:       21-06-2019
 Last changed:  21-06-2019
*/

class w5500_sender : inet_sender {
//protected:  
public:
 virtual bool init();
 virtual bool connect(const char *host, uint16_t port);	
 virtual int available();
 virtual char* read();
 virtual void close();
 virtual bool ping(const char *host);
};
