#include <PWMServo.h>
#include <SPI.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
//---------------------------------------------------------------
//Declare any function methods
//---------------------------------------------------------------
//Servo-related methods
void ServoManip(uint8_t pinNum);//Function to dymanically choose which servo to
                                //write to
uint8_t ChesterManip(uint8_t pos); //Function to change servo position

//Heartbeat methods
void HeartBeat(void);
void FailedCheck();
//---------------------------------------------------------------
//Declare any globally important values
//---------------------------------------------------------------
//Servo-related Values
PWMServo Chester; 
uint8_t cmmnd; //8 bits to hold recieved command keyword
uint8_t servoNum;//8 bits to hold which servo is being referenced
uint8_t isOpen; //8 bits to keep track of which servo is open

//Ethernet stuff
byte mac[] = {
  0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E      // MAC address of the Teensy
};
byte ip[] = {10, 10, 10, 75}; //Set the Teensy's IP address
byte dns[] = {10, 10, 10, 1};
byte gateway[] = {169, 254, 147, 58}; //This will be the ip the ground station sends from
byte subnet[] = {255, 255, 0, 0};
unsigned int localPort = 8080; //Port the Teensy listens on 
EthernetUDP data; //Initialise the UDP object
uint8_t packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //This is the packet buffer to take incoming packets

//Packet-related values
uint16_t heartbeatCount = 0; //Amount of heartbeats
uint16_t beatTimer = 0; //Timer to send out heartbeat packet
uint8_t heartData = 0x01; //Arbitrary string to use for a heartbeat packet
uint8_t heartCheck; //like da command string, but for the heartbeat
int dataLength;

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
      Serial.println((char)packetBuffer);//serial monitor. This is for debug
       
      cmmnd = packetBuffer[1]; //Command byte is put into var
      servoNum = packetBuffer[0]; //Servo number byte as well
      ServoManip(servoNum); //Manipulate the corresponding servo
      for(int k = 0; k < 24; k++){ //Clear the packet buffer
        packetBuffer[k] = '\u0000';
      }
    }

    //end packet check-----------------
    HeartBeat(); //Call heartbeat function
  delay(10);
}

//Begin Functions--------------------------------------------------------------
void ServoManip(uint8_t pinNum){
  Chester.write(90); //this write makes sure that attaching the servo sets it to neutral
  switch(pinNum){ //Switch to determine which servo to attach
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
    default:
      Chester.attach(3, 500, 2500);
      ChesterManip(isOpen & 0x01);
  }
  //Chester.detach();
  return;
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
  //HEARTBEAT, BABY
  beatTimer++; //increment timer for heartbeat check
  if(beatTimer >= 10000){
    //Clear Packet buffer
    for(int k = 0; k < 24; k++){
    packetBuffer[k] = '\u0000';
    }
    //write heartbeat packet and send it
    data.beginPacket(data.remoteIP(), data.remotePort());
    data.write("%d", heartData);
    data.endPacket();
    delay(100);
    //Check the data sent back and compare to what was sent
    dataLength = data.parsePacket();
    data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    heartCheck = packetBuffer;
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