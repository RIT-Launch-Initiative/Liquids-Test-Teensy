#include <Arduino.h>
#include "Actuator.h"
#include "NetComm.h"

ServoValve* servoOxValve; //ox is servo 1
ServoValve* servoIPAValve;//IPA is servo 2
ServoValve* servoMainValve;//main is the big double servo
SolenoidQD OxQD(7);
SolenoidQD IPAQD(8);
LightTree lightTree(9, 10,11,12);
Ignitor ignitor(6, 23, 22);

bool IGNITION_SEQUENCE_ACTIVE = false;
uint32_t MAIN_VALVE_OPEN_TIME;

void setup(){
    delay(5000); //TODO: CHECK IF THIS STILL NEEDED WITH QNETHERNET
    initialiseEthernet();
    servoOxValve = new ServoValve(16, 900, 1970);
    servoIPAValve = new ServoValve(18, 1100, 2090);
    servoMainValve = new ServoValve(20, 1050, 2100);//replacing main, old positions 1115 and 2140
    servoOxValve->init();
    servoIPAValve->init();
    servoMainValve->init();
    Serial.println("Nitron controls software v0.9.7 hotfire 2");
}

//int loops = 0;
void loop(){
    //loops++;
    String message = readPacket();
    if(message.length() > 0){
        //sendPacket("Recieved packet with content: " + message);

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
            servoOxValve->openValve();
        }
        else if(command == CMD::OXCLOSE){
            servoOxValve->closeValve();
        }
        //IPA controls
        else if(command == CMD::IPAOPEN){
            servoIPAValve->openValve();
        }
        else if(command == CMD::IPACLOSE){
            servoIPAValve->closeValve();
        }
        //main valve controls
        else if(command == CMD::MAINOPEN){
            servoMainValve->openValve();
        }
        else if(command == CMD::MAINCLOSE){
            servoMainValve->closeValve();
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
            IGNITION_SEQUENCE_ACTIVE = true;//set sequence active to allow tick function to continue ignition process
            ignitor.ignite();//set e-match ignition output high. output will be set low in tick function by config item for ignitor hold duration
            MAIN_VALVE_OPEN_TIME = millis() + getIgDelayMillisRequested();//set time for valve to open in tick function, no correlation with ignitor hold duration
        }
        else if(command == CMD::DELAYIG){
            if(ignitor.setIgDelayTime(getIgDelayMillisRequested())){
                sendPacket("DELAYIG UPDATED");
            }
            else{
                sendPacket("DELAYIG REJECTED");
            }
        }
    }
    //Serial.println("loop active");
    lightTree.tickLights();//Update logic every loop for light and horn actuation
    ignitor.tickIgnitor();
    OxQD.tickSolenoid();
    IPAQD.tickSolenoid();

    //tick logic for non-blocking ignitor hold and main valve delay control. after testing this should be refactored to a function
    if(IGNITION_SEQUENCE_ACTIVE){//only proceed with tick logic if ignition is active
        if(MAIN_VALVE_OPEN_TIME < millis()){//main valve scheduled open time has passed
            servoMainValve->openValve();//open valve
            IGNITION_SEQUENCE_ACTIVE = false;//de-activate automated ignition sequence
        }
    }
}