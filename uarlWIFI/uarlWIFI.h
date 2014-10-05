/**
 * ESP8266 library
 * 
 * Original by user "534659123" on github -- https://github.com/534659123/OCROBOT-WIFI-ESP8266-arduino-library
 * Adapted by Tyler Bletsch (Tyler.Bletsch@gmail.com) -- https://github.com/tkbletsc/OCROBOT-WIFI-ESP8266-arduino-library
 */
#ifndef __UARLWIFI_H__
#define __UARLWIFI_H__
#include <Arduino.h>

// timeout to wait for 'success' message when using expect() -- timeout implies failure of a command
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
    WIFI();
    
    void begin(HardwareSerial& serial);
    
    void set_debug_stream(Stream& s);
	
	bool Initialize(wifi_mode_t mode, char* ssid, char* pwd, byte channel = 1, wifi_enc_t enc = WAP_PSK);
	//void ipConfig(byte type, String addr, int port, boolean a = 0, byte id = 0);
	
    bool command_cr(char* cmd, char* exp1, char* exp2=NULL, int timeout=DEFAULT_EXPECT_TIMEOUT);
    bool command(char* cmd, char* exp1, char* exp2=NULL, int timeout=DEFAULT_EXPECT_TIMEOUT);
    bool expect(char* exp1, char* exp2=NULL, int timeout=DEFAULT_EXPECT_TIMEOUT);
    
    bool tcp_connect(char* ip_address, unsigned short port);
    
    bool send(char* data);
    char* wget(char* url);
    
  private:
  
    HardwareSerial* SerialWifi;
    Stream* dbg;

};

#endif
