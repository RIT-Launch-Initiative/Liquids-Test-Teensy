#include <Arduino.h>
#include "Actuator.h"
#include "NetComm.h"

ServoValve servoOxValve(16, 900, 1970);    // ox is servo 1
ServoValve servoIPAValve(18, 1100, 2090);  // IPA is servo 2
ServoValve servoMainValve(20, 1115, 2140); // main is the big double servo
SolenoidQD OxQD(8);
SolenoidQD IPAQD(9);
LightTree lightTree(10, 11, 12, 13);
Ignitor ignitor(6, 7);

void sendStatus();

void setup()
{
    delay(5000);
    initialiseEthernet();
    servoOxValve.init();
    servoIPAValve.init();
    servoMainValve.init();
}

void loop()
{
    CMD current_cmd = getCMD();
    // String message = readPacket();

    switch (current_cmd)
    {
    case CMD::NONE:
        break;
    case CMD::SPECIAL:
        break;
    case CMD::OXDISCONNECT:
        Serial.println("Ox disconnect");
        OxQD.disconnectQD();
        break;
    case CMD::IPADISCONNECT:
        Serial.println("IPA disconnect");
        IPAQD.disconnectQD();
        break;
    case CMD::OXOPEN:
        Serial.println("Ox open");
        servoOxValve.openValve();
        break;
    case CMD::OXCLOSE:
        Serial.println("Ox close");
        servoOxValve.closeValve();
        break;
    case CMD::IPAOPEN:
        Serial.println("IPA open");
        servoIPAValve.openValve();
        break;
    case CMD::IPACLOSE:
        Serial.println("IPA close");
        servoIPAValve.closeValve();
        break;
    case CMD::MAINOPEN:
        Serial.println("Main open");
        servoMainValve.openValve();
        break;
    case CMD::MAINCLOSE:
        Serial.println("Main close");
        servoMainValve.closeValve();
        break;
    case CMD::HONK:
        Serial.println("HONK");
        lightTree.shortHorn();
        break;
    case CMD::IGNITE:
        Serial.println("IGNITE");
        ignitor.ignite();
        break;
    case CMD::FAULT:
        lightTree.setLightRedFlash(true);
        lightTree.shortHorn();
        Serial.println("FAULT");
        setStatus("FAULT");
        break;
    case CMD::STATUS:
        Serial.println("status CMD");
        sendStatus();
        break;
    default:
        Serial.println("default switch");
        break;
    }

    delay(100);
}

void sendStatus(){
    char buffer[200];
    snprintf(buffer, sizeof(buffer),
            "{\"OxStatus\": %d, \"IPAStatus\": %d, \"MainStatus\": %d, \"ControlIP\": \"%s\"}",
            servoOxValve.getStatus(),
            servoIPAValve.getStatus(),
            servoMainValve.getStatus(),
            getLoggedInIP().c_str());
    sendPacket(buffer);
}