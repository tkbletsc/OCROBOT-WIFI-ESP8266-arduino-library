/**
 * ESP8266 library
 * 
 * Original by user "534659123" on github -- https://github.com/534659123/OCROBOT-WIFI-ESP8266-arduino-library
 * Adapted by Tyler Bletsch (Tyler.Bletsch@gmail.com) -- https://github.com/tkbletsc/OCROBOT-WIFI-ESP8266-arduino-library
 */
#include <uarlWIFI.h>

WIFI::WIFI() {
    //SerialWifi->println("constr");
    dbg = NULL;
}

void WIFI::begin(HardwareSerial& serial) {
    SerialWifi = &serial;
    SerialWifi->begin(115200);
    SerialWifi->setTimeout(SERIAL_WIFI_TIMOUT);
}

// allows debug_println, etc. after set_debug_stream has been called to provide a stream (e.g. a SoftwareSerial object)
#define debug_print(s)   { if (dbg) dbg->print(s); }
#define debug_printhex(s)   { if (dbg) dbg->print(s,HEX); }
#define debug_println(s) { if (dbg) dbg->println(s); }

#define DEBUG_EXPECT 0 // enable to dump the data read by an expect operation

void WIFI::set_debug_stream(Stream& s) {
    dbg = &s;
}

bool WIFI::command(char* cmd, char* exp1, char* exp2, int timeout) {
    debug_print("> ");
    debug_println(cmd);
    SerialWifi->println(cmd); // implies CRLF, which is basically okay, except it screws up CIPSEND, since the device considers CR to be the break, so the NL is counted as part of data
    SerialWifi->flush();
    return expect(exp1,exp2,timeout);
}

bool WIFI::command_cr(char* cmd, char* exp1, char* exp2, int timeout) {
    debug_print("*> ");
    debug_println(cmd);
    SerialWifi->print(cmd); // CR only, needed for CIPSEND command (but otherwise considered annoying)
    SerialWifi->print("\r");
    SerialWifi->flush();
    return expect(exp1,exp2,timeout);
}

bool WIFI::expect(char* exp1, char* exp2, int timeout) {
    char* e1 = exp1;
    char* e2 = exp2;
    String data;
    int tStart = millis();
    while (millis() < tStart+timeout) {
        if (SerialWifi->available()) {
            char c = SerialWifi->read();
            data += c;
            if (c==*e1) {
                e1++;
                if (*e1 == '\0') {
                    debug_println("< yes1")
#if DEBUG_EXPECT
                    debug_print("<<")
                    debug_print(data);
                    debug_println(">>")
#endif
                    return true;
                }
            } else {
                e1 = exp1;
            }
            if (exp2 && c==*e2) {
                e2++;
                if (*e2 == '\0') {
                    debug_println("< yes2")
#if DEBUG_EXPECT
                    debug_print("yes2:<<")
                    debug_print(data);
                    debug_println(">>")
#endif
                    return true;
                }
            } else {
                e2 = exp2;
            }
        }
    }
    debug_println("< no")
#if 1 || DEBUG_EXPECT
    debug_print("no:<<")
    debug_print(data);
    debug_println(">>")
#endif
    return false;

}


bool WIFI::Initialize(wifi_mode_t mode, char* ssid, char* pwd, byte channel, wifi_enc_t enc) {
    debug_println("init");
    
    if (!command("AT+RST","ready\r\n")) {
        debug_println("error: unable to reset module");
        return false;
    }
        
    
	if (mode == STA) {	
		if (!command("AT+CWMODE=1","done","no change")) {
            debug_println("error: unable to set mode to STA (station)");
			return false;
		}
        
        char cmd[64];
        if (pwd) {
            sprintf(cmd,"AT+CWJAP=\"%s\",\"%s\"",ssid,pwd);
        } else {
            sprintf(cmd,"AT+CWJAP=\"%s\"",ssid);
        }
        if (!command(cmd, "OK")) {
            debug_println("error: unable to join AP");
            return false;
        }
	} else {
        debug_println("error: modes other than STA not yet supported");
        return false;
    }
    
    // mux not supported yet
    if (!command("AT+CIPMUX=0","OK")) {
        debug_println("error: unable to disable mux");
        return false;
    }
    
    delay(POST_INIT_DELAY); // wait for DHCP (if DHCP is to happen)
    
	return true;
}

bool WIFI::tcp_connect(char* ip_address, unsigned short port) {
    char cmd[64];
    sprintf(cmd,"AT+CIPSTART=\"TCP\",\"%s\",%d",ip_address,port);
    return command(cmd, "Linked");
}
#define eatspace { \
    while (1) { \
        char c = SerialWifi->peek(); \
        if (c=='\r' || c=='\n') SerialWifi->read(); \
        else break; \
    } \
}


bool WIFI::send(char* data) {
    char cmd[32];
    sprintf(cmd,"AT+CIPSEND=%d",strlen(data));
    if (!command_cr(cmd, "> ")) {
        debug_println("error: unable to begin send");
        return false;
    }
    SerialWifi->write(data);
    SerialWifi->flush();
    if (!expect("SEND OK")) {
        debug_println("error: got bad response from actual send");
        return false;
    }
    return true;
}

char* WIFI::wget(char* url) {
    static char buf[256];
    //TODO: (!url.startsWith("http://")
    char* host = url+7;
    // TODO: port numbers
    char* first_slash = strchr(host, '/');
    *first_slash = '\0';
    char* path = first_slash + 1;
    
    debug_println(host);
    debug_println(path);
    tcp_connect(host,80);
    sprintf(buf,"GET /%s HTTP/1.0\nHost: %s\n\n", path, host);
    //sprintf(buf,"GET /%s HTTP/1.0\r\n\r\n", path);
    send(buf);
    expect("FUN");
}
