/**
 * ESP8266 library
 * 
 * Original by user "534659123" on github -- https://github.com/534659123/OCROBOT-WIFI-ESP8266-arduino-library
 * Adapted by Tyler Bletsch (Tyler.Bletsch@gmail.com) -- https://github.com/tkbletsc/OCROBOT-WIFI-ESP8266-arduino-library
 */
#include <uarlWIFI.h>

WIFI::WIFI(void) {
    //SERIAL_WIFI.println("constr");
    dbg = NULL;
}

// allows debug_println, etc. after set_debug_stream has been called to provide a stream (e.g. a SoftwareSerial object)
#define debug_print(s)   { if (dbg) dbg->print(s); }
#define debug_printhex(s)   { if (dbg) dbg->print(s,HEX); }
#define debug_println(s) { if (dbg) dbg->println(s); }

#define DEBUG_EXPECT 0

void WIFI::set_debug_stream(Stream& s) {
    dbg = &s;
}

bool WIFI::command(char* cmd, char* exp1, char* exp2, int timeout) {
    debug_print("> ");
    debug_println(cmd);
    SERIAL_WIFI.println(cmd);
    SERIAL_WIFI.flush();
    return expect(exp1,exp2,timeout);
}

bool WIFI::expect(char* exp1, char* exp2, int timeout) {
    char* e1 = exp1;
    char* e2 = exp2;
    String data;
    int tStart = millis();
    while (millis() < tStart+timeout) {
        if (SERIAL_WIFI.available()) {
            char c = SERIAL_WIFI.read();
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
#if DEBUG_EXPECT
    debug_print("no:<<")
    debug_print(data);
    debug_println(">>")
#endif
    return false;

}


bool WIFI::Initialize(wifi_mode_t mode, char* ssid, char* pwd, byte channel, wifi_enc_t enc) {
    debug_println("init");
    SERIAL_WIFI.begin(115200);
    SERIAL_WIFI.setTimeout(SERIAL_WIFI_TIMOUT);
    
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

/*void WIFI::ipConfig(byte type, String addr, int port, boolean a, byte id) {
	if (a == 0 ) {
		confMux(a);
		long timeStart = millis();
		while (1) {
			long time0 = millis();
			if (time0 - timeStart > 5000) {
				break;
			}
		}
		newMux(type, addr, port);
	}
	else if (a == 1) {
		confMux(a);
		long timeStart = millis();
		while (1) {
			long time0 = millis();
			if (time0 - timeStart > 5000) {
				break;
			}
		}
		newMux(id, type, addr, port);
	}
}
*/

int WIFI::ReceiveMessage(char *buf, int MsgLen) {
	//+IPD:11,wifi read11
	//done
	String data = "";
	if (SERIAL_WIFI.available()>0) {
		//Serial.println(SERIAL_WIFI.available());
		char c0 = SERIAL_WIFI.read();
		if (c0 == '+') {
			while (1) {
				if (SERIAL_WIFI.available()>0) {
					char c = SERIAL_WIFI.read();
					data += c;
				}
				if (data.indexOf("done")!=-1) {
					break;
				}
			}
			//Serial.println(data);
			int sLen = strlen(data.c_str());
			int i;
			for (i = 0; i <= sLen; i++) {
				if (data[i] == ':') {
					break;
				}
				
			}
			String _size = data.substring(4, i);
			int iSize = _size.toInt();
			//Serial.println(_size);
			String str = data.substring(i+1, i+1+iSize);
			strcpy(buf, str.c_str());
			//Serial.println(str);
			return iSize;
		}
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////


/*********************************************
 *********************************************
 *********************************************
                   TPC/IP功能指令
 *********************************************
 *********************************************
 *********************************************
 */


/*==============================================*/
/*
 *
 * 获取目前的链接状态
 * <ID>  0-4
 * <type>  tcp or udp
 * <addr>  ip
 * <port>  port number
 * /
 /*============================================*/
String WIFI::showStatus(void) {
    SERIAL_WIFI.println("AT+ShowSTA");  //发送AT指令
      String data;
      while (1) {
       if(SERIAL_WIFI.available()>0) {
       char a =SERIAL_WIFI.read();
       data=data+a;
       }
       if (data.indexOf("done")!=-1) {
           break;
       }
    }

          char head[4] = {0x0D,0x0A};   //头部多余字符串
          char tail[7] = {0x0D,0x0A,0x0D,0x0A};        //尾部多余字符串
          data.replace("AT+ShowSTA","");
          data.replace("done","");
          data.replace(head,"");
          data.replace(tail,"");
          return data;
}

/*============================================*/
/*
 * 查询目前的链接模式（单链接or多链接）
 * */
/*============================================*/
String WIFI::showMux(void) {
    String data;
    SERIAL_WIFI.println("AT+MUX?");  //发送AT指令

      while (1) {
       if(SERIAL_WIFI.available()>0) {
       char a =SERIAL_WIFI.read();
       data=data+a;
       }
       if (data.indexOf("done")!=-1) {
           break;
       }
    }
          char head[3]={0x0D,0x0A};   //将回显数据处理成需要的格式
          data.replace("AT+MUX?","");
          data.replace("+MUX","showMux");
          data.replace("done","");
          data.replace(head,"");
          return data;
}

/*============================================*/
/*
 * 设置链接模式（单链接or多链接）
 * */
/*============================================*/
void WIFI::confMux(boolean a) {
 SERIAL_WIFI.print("AT+MUX=");
 SERIAL_WIFI.println(a);           //发送AT指令
 while (1) {                            //当串口有完成数据返回时，结束语句
     if(SERIAL_WIFI.find("done")==true || SERIAL_WIFI.find("Link is builded")==true) {
        break;
     }
  }
}


/*============================================*/
/*
 * 建立tcp链接或者注册UDP端口号（单路模式）
 *
 * */
/*===========================================*/
void WIFI::newMux(byte type, String addr, int port) {
    String data;
    SERIAL_WIFI.print("AT+NewSTA=");
    if(type>0) {
        SERIAL_WIFI.print("\"TCP\"");
    }else
    {
        SERIAL_WIFI.print("\"UDP\"");
    }
    SERIAL_WIFI.print(",");
    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.print(addr);
    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.print(",");
//    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.println(String(port));
//    SERIAL_WIFI.println("\"");
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("done")!=-1 || data.indexOf("ALREAY CONNECT")!=-1 || data.indexOf("ERROR")!=-1) {
         break;
     }
  }
}
/*============================================*/
/*
 * 建立tcp链接或者注册UDP端口号（多路模式）（0-4共计5路）
 *
 * */
/*===========================================*/
void WIFI::newMux( byte id, byte type, String addr, int port) {

    SERIAL_WIFI.print("AT+NewSTA=");
    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.print(String(id));
    SERIAL_WIFI.print("\"");
    if(type>0) {
        SERIAL_WIFI.print("\"tcp\"");
    }else
    {
        SERIAL_WIFI.print("\"UDP\"");
    }
    SERIAL_WIFI.print(",");
    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.print(addr);
    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.print(",");
//    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.println(String(port));
//    SERIAL_WIFI.println("\"");
    String data;
    while (1) {

     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("done")!=-1 || data.indexOf("ALREAY CONNECT")!=-1 || data.indexOf("ERROR")!=-1) {
         break;
     }
  }

}
/*==============================================*/
/*
 * 发送数据(单路模式)
 *
 * */
/*==============================================*/
void WIFI::Send(String str) {
    SERIAL_WIFI.print("AT+UpDate=");
//    SERIAL_WIFI.print("\"");
    SERIAL_WIFI.println(str.length());
//    SERIAL_WIFI.println("\"");
    while (1) {                            //当串口有完成数据返回时，结束语句
        if(SERIAL_WIFI.find(">")==true ) {
           break;
        }
     }
    SERIAL_WIFI.println(str);


    String data;
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("SEND OK")!=-1) {
         break;
     }
  }
}

/*==============================================*/
/*
 * 发送数据(多路路模式)
 *
 * */
/*==============================================*/
void WIFI::Send(byte id, String str) {
    SERIAL_WIFI.print("AT+UpDate=");

    SERIAL_WIFI.print(String(id));
    SERIAL_WIFI.println(",");
    SERIAL_WIFI.println(str.length());
    while (1) {                            //当串口有完成数据返回时，结束语句
        if(SERIAL_WIFI.find(">")==true ) {
           break;
        }
     }
    SERIAL_WIFI.println(str);


    String data;
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("SEND OK")!=-1) {
         break;
     }
  }
}

/*========================================*/
/*
 *
 * 关闭TCP或UDP链接（单路模式）
 *
 * *-*/
/*=======================================*/
void WIFI::closeMux(void) {
    SERIAL_WIFI.println("AT+CLOSE");

    String data;
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("Linked")!=-1 || data.indexOf("ERROR")!=-1 || data.indexOf("we must restart")!=-1) {
         break;
     }
  }
}


/*========================================*/
/*
 *
 * 关闭TCP或UDP链接（多路模式）
 *
 * *-*/
/*=======================================*/
void WIFI::closeMux(byte id) {
    SERIAL_WIFI.print("AT+CLOSE=");
    SERIAL_WIFI.println(String(id));
    String data;
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("done")!=-1 || data.indexOf("Link is not")!=-1 || data.indexOf("Cant close")!=-1) {
         break;
     }
  }

}

/*=========================================*/
/*
 * 获取目前的本机IP地址
 * */
/*=========================================*/
String WIFI::showIP(void) {
    SERIAL_WIFI.println("AT+ShowIP");  //发送AT指令
    String data;
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("done")!=-1) {
         break;
     }
  }
    return data;
}

/*=======================================*/
/*
 *
 * 配置为服务器
 *
*/
/*=======================================*/

void WIFI::confServer(byte mode, int port) {
    SERIAL_WIFI.print("AT+Server=");  //发送AT指令
    SERIAL_WIFI.print(String(mode));
    SERIAL_WIFI.print(",");
    SERIAL_WIFI.println(String(port));

    String data;
    while (1) {
     if(SERIAL_WIFI.available()>0) {
     char a =SERIAL_WIFI.read();
     data=data+a;
     }
     if (data.indexOf("done")!=-1 || data.indexOf("Link is builded")!=-1) {
         break;
     }
  }
}
