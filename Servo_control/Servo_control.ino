#include <PWMServo.h>
#include <SPI.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

//---------------------------------------------------------------
//Declare any function methods
//---------------------------------------------------------------
//Servo-related methods
void ActuatorManip(uint8_t pinNum);//Function to dymanically choose which actuator to write to
uint8_t ChesterManip(uint8_t pos); //Function to change servo position
void PinManip(uint8_t pos); //Function to change servo position


//State machine functions
void ChangeState(uint8_t nextState);
void UpdateLightStatus(uint8_t lightMode);

//Heartbeat methods
void HeartBeat(void);
void FailedCheck();
//---------------------------------------------------------------
//Declare any globally important values
//---------------------------------------------------------------
//Servo-related Values
PWMServo Chester; 
uint8_t cmmnd; //8 bits to hold recieved command keyword
uint8_t actuatorNum;//8 bits to hold which actuator is being referenced
uint8_t isOpen; //8 bits to keep track of which actuator is open

//Ethernet stuff
byte mac[] = {
  0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E      // MAC address of the Teensy
};
byte ip[] = {10, 2, 1, 75}; //Set the Teensy's IP address
byte dns[] = {10, 2, 1, 1};
byte gateway[] = {10, 2, 1, 1}; //This will be the ip the ground station sends from
byte subnet[] = {255, 255, 255, 0};
unsigned int localPort = 8080; //Port the Teensy listens on 
EthernetUDP data; //Initialise the UDP object
uint8_t packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //This is the packet buffer to take incoming packets

//Packet-related values
uint16_t heartbeatCount = 0; //Amount of heartbeats
uint16_t beatTimer = 0; //Timer to send out heartbeat packet
uint8_t heartData = 0x01; //Arbitrary string to use for a heartbeat packet
uint8_t heartCheck; //like da command string, but for the heartbeat
int dataLength;

// STATES and related vars
uint8_t CURRENT_STATE = 0x00;
uint8_t CURRENT_PERMISSION_BITS = 0x00;
//permission bits as follows for controlled components, and should be validated as active or locked before actuating:
/*
 * 0x01 propellant valve
 * 0x02 fuel bottle valve
 * 0x03 oxidizer bottle valve
 * 0x04 fuel bottle quick disconnect
 * 0x05 oxidizer bottle quick disconnect
 * 0x06 e-match
 */

//--------------------------------------------------------------
// Arduino setup for servo comms and ethernet data reading

void setup() {
  //Begin Ethernet 
  Ethernet.begin(mac, ip, dns, gateway, subnet);

//Start the serial communications
  Serial.begin(38400);
  while(!Serial){
    ; //Wait for serial to connect
  }
  
//Set all servos, named Chester, to be closed
  for(int i = 3; i < 6; i++){
    Chester.attach(i, 500, 2500);
    Chester.write(90);
  }
  Chester.attach(10, 0, 0);

//Begin UDP connection
//TODO: replace delay with link check?
  data.begin(localPort);
  delay(27000); //May or may not be needed,
                //was used to wait for an actual connection to test computer
}
//--------------------------------------------------------------
// Main loop
void loop() {
    //Check for packet------------------
    dataLength = data.parsePacket(); //Get length of incoming packet
    if (dataLength){ //Check for incoming packet
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE); //Put incoming packet into the buffer
      Serial.print("Received packet of size "); //.
      Serial.println(dataLength);               //. This is all information
      Serial.print("From ");                    //. pertaining to the ip 
      IPAddress remote = data.remoteIP();       //. address of the sending 
      for (int i =0; i < 4; i++){               //. device
        Serial.print(remote[i], DEC);           //. 
        if (i < 3){                             //. This is printed to the 
          Serial.print(".");                    //. serial for debugging
        }                                       //. purposes
      }                                         //.
      Serial.print(", port ");                  //.
      Serial.println(data.remotePort());        //.

      // read the packet into packetBuffer
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      Serial.println("Contents:"); //Put the contents of the buffer into
      //TODO: check warning here. may be fine if just for debug
      Serial.println((char)packetBuffer);//serial monitor. This is for debug
       
      cmmnd = packetBuffer[1]; //Command byte is put into var
      actuatorNum = packetBuffer[0]; //Servo number byte as well
      ActuatorManip(actuatorNum); //Manipulate the corresponding actuator
      for(int k = 0; k < 24; k++){ //Clear the packet buffer
        packetBuffer[k] = '\u0000';
      }
    }

    //end packet check-----------------
    HeartBeat(); //Call heartbeat function
  delay(10);
}


//Begin Functions--------------------------------------------------------------
void ActuatorManip(uint8_t actuatorNum){
  if(actuatorNum <4){//actuators 1, 2, and 3 represent servos
    Chester.write(90); //this write makes sure that attaching the servo sets it to neutral
  }
  switch(actuatorNum){ //Switch to determine which actuator to attach
    case 1:
      Chester.attach(3, 500, 2500);
      ChesterManip(isOpen & 0x01);
      break;
    case 2:
      Chester.attach(4, 500, 2500);
      ChesterManip(isOpen & 0x02);
      break;
    case 3:
      Chester.attach(5, 500, 2500);
      ChesterManip(isOpen & 0x04);
      break;
    case 4:
      PinManip(isOpen & 0x08, actuatorNum);
    case 5:
      PinManip(isOpen & 0x10, actuatorNum);
    case 6:
      PinManip(isOpen & 0x20, actuatorNum);
    default:
      //default same as case 1. consider combining/refactoring to prevent code duplication and aid in readability.
      Chester.attach(3, 500, 2500);
      ChesterManip(isOpen & 0x01);
  }
  //Chester.detach();
  return;
}

void PinManip(uint8_t pos, uint8_t actuatorNum){
  if(cmmnd == 0x10){//check that command is to "open" / activate
    if(!pos){//pos should be 00000000 if the corresponding actuator is not yet activated
      switch(actuatorNum){
        //TODO: determine how and when these outputs will be reset/toggled back off. this could be done with a timer, off command, or as part of the state system such that pin 6 turns off when pin 7 goes on.
        case 4:
          //write pin high and set the corresponding bit high
          digitalWrite(6, HIGH);
          isOpen &= ~(0x08);
        case 5:
          digitalWrite(7, HIGH);
          isOpen &= ~(0x10);
        case 6:
          digitalWrite(6, HIGH);
          isOpen &= ~(0x20);
        default:
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("invalid case encountered in PinManip");
          data.endPacket();
      }
    }
    else{
      data.beginPacket(data.remoteIP(), data.remotePort());
      data.write("commanded Pin already active");
      data.endPacket();
    }
  }
  else{
    data.beginPacket(data.remoteIP(), data.remotePort());
    data.write("invalid command in PinManip");
    data.endPacket();
  }
}

uint8_t ChesterManip(uint8_t pos){ 
  if(cmmnd == 0x01){ //Check if the contents of the packet match control keywords
    if(pos){ //Check "boolean" isOpen to determine servo state
      Chester.write(90); //Set servo to neutral position, hopefully closed
      delay(200); //Wait for servo to reach position
      data.beginPacket(data.remoteIP(), data.remotePort()); //.
      data.write("Closing");                                //. Send packet to sender acknowledging
      data.endPacket();                                     //. command
    }else{ //If the servo is already closed, send packet saying as such
      data.beginPacket(data.remoteIP(), data.remotePort());
      data.write("Already Closed");
      data.endPacket();
    }
    switch(pos){ //Edit the corresponding isOpen bit
      case 1:
        isOpen &= ~(0x01);
        break;
      case 2:
        isOpen &= ~(0x02);
        break;
      case 4:
        isOpen &= ~(0x04);
        break;
      default:
      //Maybe include some error state here
      break;
    }
    return 0;
  } 

  if(cmmnd == 0x10){ //Same process as above, only for opening the servo
      if(!pos){
      Chester.write(45);
      delay(200);
      data.beginPacket(data.remoteIP(), data.remotePort());
      data.write("Opening");
      data.endPacket();
    }else{
      data.beginPacket(data.remoteIP(), data.remotePort());
      data.write("Already Open");
      data.endPacket();
    }
    switch(pos){ //Edit the corresponding isOpen bit
      case 1:
        isOpen |= 0x01;
        break;
      case 2:
        isOpen |= 0x02;
        break;
      case 4:
        isOpen |= 0x04;
        break;
      default:
      //Maybe include some error state here
      break;
    }
    return 0;
  }
  return 0;
}

//-------------------------------------------------------
void HeartBeat(void){
  // Serial.println("heartbeat function");
  //HEARTBEAT, BABY
  beatTimer++; //increment timer for heartbeat check
  if(beatTimer >= 10000){
    Serial.println("beat timer over 10000");
    //Clear Packet buffer
    for(int k = 0; k < 24; k++){
    packetBuffer[k] = '\u0000';
    }
    //write heartbeat packet and send it
    data.beginPacket(data.remoteIP(), data.remotePort());
    data.write("%d", heartData);
    data.endPacket();
    delay(100);
    Serial.println("wrote packet containing data:");
    Serial.println(heartData);
    //Check the data sent back and compare to what was sent
    dataLength = data.parsePacket();
    data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    heartCheck = packetBuffer;
    Serial.println("read packet containing data:");
    Serial.println(heartCheck);
    if(heartCheck == heartData){
      heartbeatCount++;
    }else{ 
      for(int failCnt = 3; failCnt < 6; failCnt++){
        Chester.attach(failCnt, 500, 2500);
        FailedCheck();
      }
      //for(;;){}
    }
    beatTimer = 0;
    for(int k = 0; k < 24; k++){ //Clear packet buffer
    packetBuffer[k] = '\u0000'; 
    }
  }
  return;
}

void FailedCheck(){
    Chester.write(90); //If it's not closed, it'll be at 45 degrees
    data.beginPacket(data.remoteIP(), data.remotePort());
    data.write("Err:hrtbt0"); //Error message to indicate no heartbeat
    data.endPacket();         //(Pretty neat, huh?)
    //Wait indefinitely, will only do something
    //once turned off, then on again.
    return;
}

/**
 * function to change state based on recieved command
 * control permission bits and light status will be updated accordingly
 * determination of type of command packed recieved (state related or other) to be done by calling function
 */
void ChangeState(uint8_t nextState){
  if(nextState-1 != CURRENT_STATE){//reject command and do no operation if state is not the next one in sequence
    return;
  }

  //TODO: determine any other cases for rejection of command to move to next stage
  //TODO: verify these bits against the chart, if this state system actually ends up being used.

  switch(nextState){
    case 1:
      CURRENT_PERMISSION_BITS = 2;//propellent valve active 00000010
      UpdateLightStatus(1);
      break;
    case 2:
      CURRENT_PERMISSION_BITS = 6;//fuel bottle valve active 00000110
      UpdateLightStatus(3);
      break;
    case 3:
      CURRENT_PERMISSION_BITS = 16;//fuel bottle quick disconnect active
      UpdateLightStatus(3);
      break;
    case 4:
      CURRENT_PERMISSION_BITS = 10;//oxidizer bottle valve active 00001010
      UpdateLightStatus(3);
      break;
    case 5:
      CURRENT_PERMISSION_BITS = 32;//oxidizer bottle quick disconnect active
      UpdateLightStatus(3);
      break;
    case 6:
      CURRENT_PERMISSION_BITS = 65;//e-match and propellant valve active
      UpdateLightStatus(3);
      break;
    case 7:
      CURRENT_PERMISSION_BITS = 0;//no controls active
      UpdateLightStatus(3);
      break;
    default:
      CURRENT_PERMISSION_BITS = 0;
      UpdateLightStatus(0);
      break;
  }
}

/**
 * function to set correct outputs for light control
 * 1 indicates green, 2 indicates yellow, 3 indicates red. 0 or other values indicate a fault or non-state such as at startup
 */
void UpdateLightStatus(uint8_t lightMode){
  switch(lightMode){
    case 1://green light only
      digitalWrite(41, LOW);
      digitalWrite(40, LOW);
      digitalWrite(39, HIGH);
      break;
    case 2://yellow light only
      digitalWrite(41, LOW);
      digitalWrite(40, HIGH);
      digitalWrite(39, LOW);
    case 3://red light only
      digitalWrite(41, HIGH);
      digitalWrite(40, LOW);
      digitalWrite(39, LOW);
      break;
    default://fault or non-state, all lights on
      digitalWrite(41, HIGH);
      digitalWrite(40, HIGH);
      digitalWrite(39, HIGH);
      break;
  }
}