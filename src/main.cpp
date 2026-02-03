#include <Arduino.h>
#include "Actuator.h"
#include "NetComm.h"

ServoValve servoOxValve(16, 900, 1970); //ox is servo 1
ServoValve servoIPAValve(18, 1100, 2090);//IPA is servo 2
ServoValve servoMainValve(20, 1115, 2140);//main is the big double servo
SolenoidQD OxQD(8);
SolenoidQD IPAQD(9);
LightTree lightTree(10,11,12,13);
Ignitor ignitor(6,7);

void setup(){
    delay(5000);
    initialiseEthernet();
    servoOxValve.init();
    servoIPAValve.init();
    servoMainValve.init();
}

void loop(){
    String message = readPacket();

    if(message == ""){
        Serial.println("no response recieved");
    }
    //quick disconnects
    else if(message == "OXDISCONNECT"){
      OxQD.disconnectQD();
    }
    else if(message == "IPADISCONNECT"){
      IPAQD.disconnectQD();
    }
    //oxygen controls
    else if(message == "OXOPEN"){
      servoOxValve.openValve();
    }
    else if(message == "OXCLOSE"){
      servoOxValve.closeValve();
    }
    //IPA controls
    else if(message == "IPAOPEN"){
      servoIPAValve.openValve();
    }
    else if(message == "IPACLOSE"){
      servoIPAValve.closeValve();
    }
    //main valve controls
    else if(message == "MAINOPEN"){
      servoMainValve.openValve();
    }
    else if(message == "MAINCLOSE"){
      servoMainValve.closeValve();
    }
    //LightTree
    else if(message == "HONK"){
        lightTree.shortHorn();
    }
    //other commands
    else if(message == "PING"){
      sendPacket("PONG");
    }
    else if(message == "FAULT"){
      lightTree.setLightRedFlash(true);
      lightTree.shortHorn();
    }
    //Igno
    else if(message == "IGNITE"){
      ignitor.ignite();
    }

    // if(message.length() > 0){
    //   sendPacket("Recieved packet with content: " + message);
    // }
    delay(1000);
    //Serial.println("loop");
}
