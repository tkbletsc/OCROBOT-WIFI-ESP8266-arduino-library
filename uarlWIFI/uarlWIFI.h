/**
 * ESP8266 library
 * 
 * Original by user "534659123" on github -- https://github.com/534659123/OCROBOT-WIFI-ESP8266-arduino-library
 * Adapted by Tyler Bletsch (Tyler.Bletsch@gmail.com) -- https://github.com/tkbletsc/OCROBOT-WIFI-ESP8266-arduino-library
 */
#ifndef __UARLWIFI_H__
#define __UARLWIFI_H__
#include <Arduino.h>

#ifndef SERIAL_WIFI
#define SERIAL_WIFI Serial
#endif

// for commands with multiple success states, i just wait until the thing has said all it wants to say, then check
// (a more correct implementation would be a state machine based on multiple expected strings, but meh)
// anyway, this is the timeout for that wait
#define DEFAULT_EXPECT_TIMEOUT 2000

// port-level timeout (.setTimeout())
#define SERIAL_WIFI_TIMOUT 2000

// time to wait after init, mainly for DHCP
#define POST_INIT_DELAY 3000

// Access point encryption types (given back from the AT+CWLAP command that lists APs)
typedef enum {
    OPEN = 0,
    WEP = 1,
    WAP_PSK = 2,
    WAP2_PSK = 3,
    WAP_WAP2_PSK = 4,
} wifi_enc_t;

// protocols
typedef enum {
    TCP = 1,
    UDP = 0,
} conn_type_t;

// Used with +CWMODE to select WIFI application mode
// STA = Station (client), AP = Access Point, AP_STA = Both
typedef enum {
    STA = 1,
    AP = 2,
    AP_STA = 3,
} wifi_mode_t;

class WIFI {
  public:
    Stream* dbg;

    WIFI(void);
    
    void set_debug_stream(Stream& s);
	
	bool Initialize(wifi_mode_t mode, char* ssid, char* pwd, byte channel = 1, wifi_enc_t enc = WAP_PSK);
	//void ipConfig(byte type, String addr, int port, boolean a = 0, byte id = 0);
	
	void Send(String str);  // Send
	void Send(byte id, String str);  //多路模式发送数据
		
	int ReceiveMessage(char *buf, int MsgLen = 0);
	
    bool command(char* cmd, char* exp1, char* exp2=NULL, int timeout=DEFAULT_EXPECT_TIMEOUT);
    bool expect(char* exp1, char* exp2=NULL, int timeout=DEFAULT_EXPECT_TIMEOUT);
    
    /*================TCP/IP指令================*/
    String showStatus(void);    //查询链接状态
    String showMux(void);       //查询目前的链接模式（单链接or多链接）
    void confMux(boolean a);    //设置链接模式（单链接【0】or多链接【1】）
    void newMux(byte type, String addr, int port);   //创建 tcp或者udp链接（单路模式)
    void newMux(byte id, byte type, String addr, int port);   //创建 tcp或者udp链接（多路模式)（id:0-4） 0号可以连接客户端和服务器，其他只能连接服务器
    void closeMux(void);   //关闭tcp或udp （单路模式）
    void closeMux(byte id); //关闭tcp货udp （多路模式）
    String showIP(void);    //显示本机目前获取的ip地址
    void confServer(byte mode, int port);  //配置服务器
	
	String m_rev;
    
};

#endif
