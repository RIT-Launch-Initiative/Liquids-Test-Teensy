#include <Arduino.h>
#include "Actuator.h"
#include "NetComm.h"

ServoValve servoOxValve(16, 900, 1970); //ox is servo 1
ServoValve servoIPAValve(18, 1100, 2090);//IPA is servo 2
ServoValve servoMainValve(20, 1115, 2140);//main is the big double servo
SolenoidQD OxQD(8);
SolenoidQD IPAQD(9);
LightTree lightTree(10,11,12,13);
Ignitor ignitor(6,7, 20);

void setup(){
    delay(5000); //TODO: CHECK IF THIS STILL NEEDED WITH QNETHERNET
    initialiseEthernet();
    servoOxValve.init();
    servoIPAValve.init();
    servoMainValve.init();
}

int loops = 0;
void loop(){
    loops++;
    String message = readPacket();
    if(message.length() > 0){
        sendPacket("Recieved packet with content: " + message);

        CMD command = getCMD(message);
        if(command == CMD::SPECIAL){
            Serial.println("special command recieved and processed by netcomm");
        }
        else if(command == CMD::STATUS){
            Serial.println("status requested");
            //TODO: get status from actuators, send packet
        }
        //quick disconnects
        else if(command == CMD::OXDISCONNECT){
            OxQD.disconnectQD();
        }
        else if(command == CMD::IPADISCONNECT){
            IPAQD.disconnectQD();
        }
        //oxygen controls
        else if(command == CMD::OXOPEN){
            servoOxValve.openValve();
        }
        else if(command == CMD::OXCLOSE){
            servoOxValve.closeValve();
        }
        //IPA controls
        else if(command == CMD::IPAOPEN){
            servoIPAValve.openValve();
        }
        else if(command == CMD::IPACLOSE){
            servoIPAValve.closeValve();
        }
        //main valve controls
        else if(command == CMD::MAINOPEN){
            servoMainValve.openValve();
        }
        else if(command == CMD::MAINCLOSE){
            servoMainValve.closeValve();
        }
        //LightTree
        else if(command == CMD::HONK){
            lightTree.longHorn();
        }
        //stage commands
        else if(command == CMD::FAULT){
            // lightTree.setLightRedFlash(true);
            lightTree.shortHorn();
        }
        else if(command == CMD::NEXTSTAGE){
            Serial.println("newstage requested, not implemented");
        }
        else if(command == CMD::BACKSTAGE){
            Serial.println("previous stage requested, not implemented");
        }
        else if(command == CMD::LOCKOUT){
            Serial.println("command lockout requested, not implemented");
        }
        else if(command == CMD::UNLOCK){
            Serial.println("command unlock requested, not implemented");
        }
        //TEMP LIGHT TREE COMMANDS
        else if(command == CMD::REDLIGHT){
            lightTree.setLightRed();
        }
        else if(command == CMD::YELLOWLIGHT){
            lightTree.setLightYellow();
        }
        else if(command == CMD::GREENLIGHT){
            lightTree.setLightGreen();
        }
        else if(command == CMD::NOLIGHT){
            lightTree.setNoLights();
        }
        else if(command == CMD::HONK){
            lightTree.longHorn();
        }
        //Ignitor
        else if(command == CMD::IGNITE){
            ignitor.ignite();
        }
        // else if(command == CMD::SENDIT){
        //     servoMainValve.openValve();
        //     //TODO: timing, should probably offload this logic into Actuator
        //     ignitor.ignite();
        // }
      lightTree.tickLights();//Update logic every loop for light and horn actuation
    }
}
//TODO: use main loop to check flags for actuator loop and toggle off without any delays or while true