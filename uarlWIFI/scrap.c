#if 0
    if (  0 && exp2==NULL) { // we can use stream.find -- easy
        if (SERIAL_WIFI.find(exp1)) {
            eatspace;
            debug_println("< yes");
            return true;
        } else {
            eatspace;
            debug_println("< no");
            return false;
        }
    }
    
    String data = SERIAL_WIFI.readString();
    
    /*debug_print("data:<<<");
    debug_print(data);
    debug_println(">>>");
    */
    /*
    for (int i=0; i<data.length(); i++) {
        debug_print(data.charAt(i));
        debug_print("  ");
        debug_printhex(data.charAt(i));
        debug_println("  ");
    }
    */
    if (data.indexOf(exp1)!=-1 || data.indexOf(exp2)!=-1) {
        debug_println("< yes");
        return true;
    } else {
        debug_println("< no");
        debug_print("data:<<<");
        debug_print(data);
        debug_println(">>>");
        return false;
    }

    
#define eatspace { \
    while (1) { \
        char c = SERIAL_WIFI.peek(); \
        if (c=='\r' || c=='\n') SERIAL_WIFI.read(); \
        else break; \
    } \
}

#endif