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
uint8_t isOpen = 0; //Boolean to safeguard against repeat commands

//Ethernet stuff
byte mac[] = {
  0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E      // Set the Mac Address
};
byte ip[] = {10, 10, 10, 75};
byte dns[] = {10, 10, 10, 1};
byte gateway[] = {169, 254, 147, 58};
byte subnet[] = {255, 255, 0, 0};
unsigned int localPort = 8080;
EthernetUDP data;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

//Packet-related values
uint16_t heartbeatCount = 0; //Amount of heartbeats
uint16_t beatTimer = 0; //Timer to send out heartbeat packet
String heartData = "Joemama";
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
  
//attach the servo, named Chester
  Chester.attach(3);

//Begin UDP connection
  data.begin(localPort);
  delay(27000);
}
//--------------------------------------------------------------
// Main loop
void loop() {
    //Check for packet------------------
    dataLength = data.parsePacket();
    if (dataLength){
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      Serial.print("Received packet of size ");
      Serial.println(dataLength);
      Serial.print("From ");
      IPAddress remote = data.remoteIP();
      for (int i =0; i < 4; i++){
        Serial.print(remote[i], DEC);
        if (i < 3){
          Serial.print(".");
        }
      }
      Serial.print(", port ");
      Serial.println(data.remotePort());
      // read the packet into packetBuffer
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      Serial.println("Contents:");
      Serial.println(packetBuffer);
      cmmnd = packetBuffer;
      if(cmmnd == "Close"){
        if(isOpen){
          Chester.writeMicroseconds(1500);
          delay(1000);
          Chester.writeMicroseconds(1500);
          currentAng = Chester.read();
          data.beginPacket(data.remoteIP(), data.remotePort());
          //data.writeMicroseconds("%d", currentAng);
          data.writeMicroseconds("Closing");
          data.endPacket();
          isOpen = 0;
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.writeMicroseconds("Already Closed");
          data.endPacket();
        }
      } 

      if(cmmnd == "Open"){
        if(!isOpen){
        Chester.writeMicroseconds(1500);
        delay(1000);
        Chester.writeMicroseconds(1500);
        currentAng = Chester.read();
          data.beginPacket(data.remoteIP(), data.remotePort());
        //data.writeMicroseconds("%d", currentAng);
          data.writeMicroseconds("Opening");
          data.endPacket();
          isOpen = 1;
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.writeMicroseconds("Already Open");
          data.endPacket();
        }
      }
      for(int k = 0; k < 24; k++){
        packetBuffer[k] = '\u0000';
      }
    }
      //HEARTBEAT, BABY
    beatTimer++; //increment timer for heartbeat check
    if(beatTimer >= 10000){
      //Clear Packet buffer
      for(int k = 0; k < 24; k++){
      packetBuffer[k] = '\u0000';
      }
      //writeMicroseconds heartbeat packet and send it
      data.beginPacket(data.remoteIP(), data.remotePort());
      data.writeMicroseconds("Joemama");
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
          //Chester.writeMicroseconds(-45); //If it's not closed, it'll be at 45 degrees
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.writeMicroseconds("close pls");
          data.endPacket();
          //Wait indefinitely, will only do something
          //once turned off, then on again.
         // for(;;){}
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.writeMicroseconds("Oops");
          data.endPacket();
         // for(;;){}
        }
      }
      beatTimer = 0;
      for(int k = 0; k < 24; k++){
      packetBuffer[k] = '\u0000';
      }
    }
    
    
    //end packet check-----------------
  delay(10);
}
