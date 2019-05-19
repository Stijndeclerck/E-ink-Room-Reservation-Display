#include "Sender.h"
#include "mbed.h"

#define MAX_TOKEN_COUNT 128
//#define IP_ADDRESS "172.16.102.148"
#define LINK "/roominfo.php?roomName="
#define size_buffer 1500

const char* Sender::sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int Sender::scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;

    printf("Scan:\n");

    int count = wifi->scan(NULL,0);

    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;

    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);

    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);

    delete[] ap;
    return count;
}

int Sender::init(){
    printf("WiFi example\n");

#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    /*int count = scan_demo(wifi);
    if (count == 0) {
        printf("No WIFI APs found - can't continue further.\n");
        return -1;
    }
    */
    wifi->disconnect();
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    printf("SSID: %s\r\n",MBED_CONF_APP_WIFI_SSID);
    printf("PASSWORD: %s\r\n",MBED_CONF_APP_WIFI_PASSWORD); 
 
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }
    

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());

    printf("\nDone\n");
    
    return 0;
}

char* Sender::get(char *IP_ADDRESS, char *ROOM){
    int remaining;
    int rcount;
    char *pp;
    char *data = new char[size_buffer];
    nsapi_size_or_error_t result;
    
   
    char message[128];
    sprintf(message, "GET http://%s%s%s HTTP/1.1\r\nHost: http://%s\r\nConnection: close\r\n\r\n",IP_ADDRESS,LINK,ROOM,IP_ADDRESS); // 
    nsapi_size_t size = strlen(message);
   
    sock.open(wifi);
    result = sock.connect(IP_ADDRESS, 80);
    if(result < 0) {
        printf("\nerror: Not able to connect...\r\n");    
    }
    else {
        printf("\nconnected\r\n");    
    }
    remaining = size_buffer;
    rcount = 0;
    pp = data;
    
    result = sock.send(message, size);
    while (remaining > 0 && 0 < (result = sock.recv(pp, remaining))) {
        pp += result;
        rcount += result;
        remaining -= result;
    }
    data[size_buffer] = '\0';
    printf("recv %d [%.*s]\r\n\n", rcount, strstr(data, "\r\n")-data, data);
    sock.close();
    if (result < 0) {
        printf("Error! socket.recv() returned: %d\n", result);
    }
    else {
        printf("%s\r\n\r\n", data);
    }
    
    int length = 1024;
    
    
    /*for(int i = 0; i < size_buffer; i++){
        if(data[i] == 'L' && data[i+1] == 'e' && data[i+2] == 'n' && data[i+3] == 'g' && data[i+4] == 't' && data[i+5] == 'h' ) {
            length = (data[i+8]-48)*100 + (data[i+9]-48)*10 + data[i+10]-48;
            break;    
        }   
    }*/
    
    int k = 0, flag = 0;
    
    for(int i = 0; i < size_buffer; i++){
        if(data[i] == '{') {
            flag = 1; 
        }
        if(flag == 1){
            JSON_STRING[k] = data[i];
            k++;     
        }
        if(data[i] == ']') {
            flag = 0;
            printf("flag = 0  r\n");
            JSON_STRING[k] = '}';
            JSON_STRING[k+1] = '\0';
            break;  
        }
        
    }
    printf("JSONSTRING:%s\r\n\r\n", JSON_STRING);
    return JSON_STRING;    
}
