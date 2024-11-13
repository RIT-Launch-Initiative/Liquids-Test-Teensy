#include <PWMServo.h>
#include <SPI.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
//--------------------------------------------------------------
//Declare any globally important values
//Servo-related Values
PWMServo Chester;
int currentAng;
String cmmnd; //command string
uint8_t isOpen = 0; //"Boolean" to safeguard against repeat commands

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
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //This is the packet buffer to take incoming packets

//Packet-related values
void heartBeat(void); //Heartbeat function method
uint16_t heartbeatCount = 0; //Amount of heartbeats
uint16_t beatTimer = 0; //Timer to send out heartbeat packet
String heartData = "thump"; //Arbitrary string to use for a heartbeat packet
String heartCheck; //like da command string, but for the heartbeat
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
  
//attach the servo, named Chester, and set to 90 degrees
  Chester.attach(3);
  Chester.writeMicroseconds(1500);

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
        if (i < 3){                             //.
          Serial.print(".");                    //.
        }                                       //.
      }                                         //.
      Serial.print(", port ");                  //.
      Serial.println(data.remotePort());        //.

      // read the packet into packetBuffer
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      Serial.println("Contents:"); //Put the contents of the buffer into
      Serial.println(packetBuffer);//serial monitor. This is for debug
      
      cmmnd = packetBuffer; //Put contents from the buffer into a string 
      if(cmmnd == "Close"){ //Check if the contents of the packet match control keywords
        if(isOpen){ //Check "boolean" isOpen to determine servo state
          Chester.writeMicroseconds(1500); //Set servo to neutral position, hopefully closed
          delay(200); //Wait for servo to reach position
          currentAng = Chester.read(); //Get current servo angle
          data.beginPacket(data.remoteIP(), data.remotePort()); //.
          data.write("Closing");                                //. Send packet to sender acknowledging
          data.endPacket();                                     //. command
          isOpen = 0; //Set servo state to closed
        }else{ //If the servo is already closed, send packet saying as such
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("Already Closed");
          data.endPacket();
        }
      } 

      if(cmmnd == "Open"){ //Same process as above, only for opening the servo
        if(!isOpen){
        Chester.writeMicroseconds(2250);
        delay(200);
        currentAng = Chester.read();
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("Opening");
          data.endPacket();
          isOpen = 1;
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("Already Open");
          data.endPacket();
        }
      }
      for(int k = 0; k < 24; k++){ //Clear the packet buffer
        packetBuffer[k] = '\u0000';
      }
    }

    //end packet check-----------------
    heartBeat();
  delay(10);
}

\\Begin Functions
void  heartBeat(void){
  //HEARTBEAT, BABY
  beatTimer++; //increment timer for heartbeat check
  if(beatTimer >= 10000){
    //Clear Packet buffer
    for(int k = 0; k < 24; k++){
    packetBuffer[k] = '\u0000';
    }
    //writeMicroseconds heartbeat packet and send it
    data.beginPacket(data.remoteIP(), data.remotePort());
    data.write("thump");
    data.endPacket();
    delay(100);
    //Check the data sent back and compare to what was sent
    dataLength = data.parsePacket();
    data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    heartCheck = packetBuffer;
    if(heartCheck == heartData){
      heartbeatCount++;
    }else{ 
      if(isOpen){ //Check if open or closed
        //Chester.writeMicroseconds(1500); //If it's not closed, it'll be at 45 degrees
        data.beginPacket(data.remoteIP(), data.remotePort());
        data.write("Err:hrtbt0"); //Error message to indicate no heartbeat
        data.endPacket();         //(Pretty neat, huh?)
        //Wait indefinitely, will only do something
        //once turned off, then on again.
       // for(;;){}
      }else{
        data.beginPacket(data.remoteIP(), data.remotePort());
        data.write("Err:hrtbt0");
        data.endPacket();
       // for(;;){}
      }
    }
    beatTimer = 0;
    for(int k = 0; k < 24; k++){ //Clear packet buffer
    packetBuffer[k] = '\u0000'; 
    }
  }
}
