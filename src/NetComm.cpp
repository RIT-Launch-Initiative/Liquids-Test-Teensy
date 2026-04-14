#include "NetComm.h"
#include <Arduino.h>
using namespace qindesign::network;


// ---------------------------
// Network configuration
// ---------------------------
byte teensyMAC[6] = { 0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E };
IPAddress teensyIP(10, 10, 10, 69); //10.10.10.69 expected for launch setup
IPAddress blankIP(0,0,0,0);
IPAddress controlIP;
IPAddress lastIP;
unsigned int localPort = 8888;

EthernetUDP udp;
String currentStatus = "";



/**
 * Initialise network communication
 */
bool initialiseEthernet(){
    Serial.println("Initialising Ethernet...");

    // if (Ethernet.begin(mac) == 0) {
    //     Serial.println("DHCP failed, using static IP.");
    //     Ethernet.begin(mac, ip);
    // }
    Ethernet.begin();
    Ethernet.setLocalIP(teensyIP);
    Ethernet.setSubnetMask(IPAddress(255,255,255,0));
    Ethernet.setGatewayIP(IPAddress(10,10,10,1));//unknown what test setup gateway will be
    Ethernet.setDHCPEnabled(false);
    udp.begin(localPort);
    controlIP = IPAddress(0, 0, 0, 0);//default control IP

    Serial.println("IP Address: ");
    Serial.println(Ethernet.localIP());
    //Serial.println("MAC Address: ");
    // Serial.println(Ethernet.macAddress());
    Serial.printf("UDP listening on port %d\n", localPort);
    return true;
}

CMD getCMD(String packet){
    // String packet = readPacket();
    Serial.println("got a packet...");
    if(packet == ""){
        return CMD::NONE;
    }
    //BEGIN COMMANDS ACCEPTED BY ANY REMOTE USER
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
        sendPacket("NITRON GROUNDSTATION 0.9.2 COLDFLOW");
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
    //BEGIN COMMANDS ONLY ACCEPTED FROM LOGGED IN USER
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
            else if(packet == "NEXTSTAGE"){
                sendPacket("NEXTSTAGE ACKNOWLEDGED, NOT IMPLEMENTED");
                return CMD::NEXTSTAGE;
            }
            else if(packet == "BACKSTAGE"){
                sendPacket("BACKSTAGE ACKNOWLEDGED, NOT IMPLEMENTED");
                return CMD::BACKSTAGE;
            }
            else if(packet == "LOCKOUT"){
                sendPacket("LOCKOUT ACKNOWLEDGED, NOT IMPLEMENTED");
                return CMD::LOCKOUT;
            }
            else if(packet == "UNLOCK"){
                sendPacket("UNLOCK ACKNOWLEDGED, NOT IMPLEMENTED");
                return CMD::UNLOCK;
            }
            else if(packet == "SENDIT"){
                sendPacket("SENDIT ACKNOWLEDGED, NOT IMPLEMENTED");
                return CMD::SENDIT;
            }
            //temp states for light actuation
            else if(packet == "REDLIGHT"){
                sendPacket("REDLIGHT ACKNOWLEDGED");
                return CMD::REDLIGHT;
            }
            else if(packet == "YELLOWLIGHT"){
                sendPacket("YELLOWLIGHT ACKNOWLEDGED");
                return CMD::YELLOWLIGHT;
            }
            else if(packet == "GREENLIGHT"){
                sendPacket("GREENLIGHT ACKNOWLEDGED");
                return CMD::GREENLIGHT;
            }
            else if(packet == "NOLIGHT"){
                sendPacket("NOLIGHT ACKNOWLEDGED");
                return CMD::NOLIGHT;
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
    Ethernet.loop();//Have to tick ethernet constantly or it gets mad.
    char packetBuffer[256];  // Incoming packet storage
    int packetSize = udp.parsePacket();
    if (packetSize) {
        lastIP = udp.remoteIP();
        // Serial.printf("Received %d bytes from %d.%d.%d.%d:%d\n",
        //     packetSize,
        //     lastIP[0], lastIP[1], lastIP[2], lastIP[3],
        //     udp.remotePort()
        // );
    }

    // Read packet
    int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
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
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(response.c_str());
    udp.endPacket();
}

void setStatus(String newStatus){
    currentStatus = newStatus;
}

String getLoggedInIP(){
    char ipStr[20];
    snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", controlIP[0], controlIP[1], controlIP[2], controlIP[3]);
    return ipStr;
}