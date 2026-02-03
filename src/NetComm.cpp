#include "NetComm.h"
#include <Arduino.h>


// ---------------------------
// Network configuration
// ---------------------------
byte teensyMAC[6] = { 0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E };   // Use a valid MAC
IPAddress teensyIP(10, 10, 10, 69);                         // Change for your LAN
IPAddress blankIP(0,0,0,0);
IPAddress controlIP;
IPAddress lastIP;
unsigned int localPort = 8888;                        // Port to listen on

EthernetUDP Udp;
String currentStatus = "";



/**
 * Initialise network communication
 */
bool initialiseEthernet(){
    Serial.println("Initializing Ethernet...");

    // if (Ethernet.begin(mac) == 0) {
    //     Serial.println("DHCP failed, using static IP.");
    //     Ethernet.begin(mac, ip);
    // }
    Ethernet.begin(teensyMAC, teensyIP);
    controlIP = IPAddress(0, 0, 0, 0);//default control IP

    Serial.println("IP Address: ");
    Serial.println(Ethernet.localIP());

    Udp.begin(localPort);
    Serial.printf("UDP listening on port %d\n", localPort);
    return true;
}

CMD getCMD(){
    String packet = readPacket();
    if(packet == ""){
        return CMD::NONE;
    }
    else if(packet == "MARGARITAVILLE"){
        if(controlIP == blankIP){
            controlIP = lastIP;
            sendPacket("LOGON SUCCESS");
        }
        else{
            sendPacket("LOGON FAILED, CONNECTION ALREADY ACTIVE");
        }
        return CMD::SPECIAL;
    }
    else if(packet == "LOGOFF"){
        if(controlIP == blankIP){
            sendPacket("NO LOGIN ACTIVE");
            return CMD::SPECIAL;
        }
        else{
            controlIP = IPAddress(0,0,0,0);
            sendPacket("LOGOFF SUCCESS");
            return CMD::SPECIAL;
        }
    }
    else if(packet == "PING"){
        sendPacket("PONG");
        return CMD::SPECIAL;
    }
    else if(packet == "VERSION"){
        sendPacket("NITRON GROUNDSTATION 0.9.0");
        return CMD::SPECIAL;
    }
    else if(packet == "STATUS"){
        //response packet sent in main
        return CMD::STATUS;
    }
    else if(packet == "FAULT"){
        sendPacket("FAULT ACKNOWLEDGED");
        return CMD::FAULT;
    }
    else{
        if(lastIP == controlIP){
            if(packet == "OXDISCONNECT"){
                sendPacket("OXDISCONNECT ACKNOWLEDGED");
                return CMD::OXDISCONNECT;
            }
            else if(packet == "IPADISCONNECT"){
                sendPacket("IPADISCONNECT ACKNOWLEDGED");
                return CMD::IPADISCONNECT;
            }
            else if(packet == "OXOPEN"){
                sendPacket("OXOPEN ACKNOWLEDGED");
                return CMD::OXOPEN;
            }
            else if(packet == "OXCLOSE"){
                sendPacket("OXCLOSE ACKNOWLEDGED");
                return CMD::OXCLOSE;
            }
            else if(packet == "IPAOPEN"){
                sendPacket("IPAOPEN ACKNOWLEDGED");
                return CMD::IPAOPEN;
            }
            else if(packet == "IPACLOSE"){
                sendPacket("IPACLOSE ACKNOWLEDGED");
                return CMD::IPACLOSE;
            }
            else if(packet == "MAINOPEN"){
                sendPacket("MAINOPEN ACKNOWLEDGED");
                return CMD::MAINOPEN;
            }
            else if(packet == "MAINCLOSE"){
                sendPacket("MAINCLOSE ACKNOWLEDGED");
                return CMD::MAINCLOSE;
            }
            else if(packet == "IGNITE"){
                sendPacket("IGNITE ACKNOWLEDGED");
                return CMD::IGNITE;
            }
            else if(packet == "HONK"){
                sendPacket("HONK ACKNOWLEDGED");
                return CMD::HONK;
            }
        }
        else{
            if(controlIP == IPAddress(0,0,0,0)){
                sendPacket("ACCESS DENIED, NO REMOTE LOGON");
            }
            else{
                sendPacket("ACCESS DENIED, INVALID IP FOR REMOTE CONTROL");
            }
        }
    }
    return CMD::NONE;
}
/**
 * Read any incoming packet data, process, and return the command
 */
String readPacket(){
    char packetBuffer[256];  // Incoming packet storage
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        lastIP = Udp.remoteIP();
        // Serial.printf("Received %d bytes from %d.%d.%d.%d:%d\n",
        //     packetSize,
        //     lastIP[0], lastIP[1], lastIP[2], lastIP[3],
        //     Udp.remotePort()
        // );
    }

    // Read packet
    int len = Udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) packetBuffer[len] = '\0';

    if(len >0){
        return String(packetBuffer);
    }
    return "";
}
/**
 * Send the provided data via Ethernet
 */
void sendPacket(String response){
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(response.c_str());
    Udp.endPacket();
}

void setStatus(String newStatus){
    currentStatus = newStatus;
}

String getLoggedInIP(){
    char ipStr[20];
    snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", controlIP[0], controlIP[1], controlIP[2], controlIP[3]);
    return ipStr;
}