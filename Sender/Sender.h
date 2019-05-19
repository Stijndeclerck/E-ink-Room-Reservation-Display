#ifndef MBED_Sender_H
#define MBED_Sender_H

#include "EthernetInterface.h"
#include "mbed.h"

class Sender
{
    public:
    // Variables
    WiFiInterface *wifi;
    TCPSocket sock;
    char JSON_STRING[1536];
    
    Sender(){ 
    }
    
    // Functions
    int init();
    char* get(char *IP_ADDRESS, char *ROOM);
    
    private:
    // functions
    const char *sec2str(nsapi_security_t sec);
    int scan_demo(WiFiInterface *wifi);
};

#endif