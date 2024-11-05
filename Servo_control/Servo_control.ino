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
bool isOpen = false; //Boolean to safeguard against repeat commands

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
          //Chester.write(-45);
          //delay(100);
          //currentAng = Chester.read();
          data.beginPacket(data.remoteIP(), data.remotePort());
          //data.write("%d", currentAng);
          data.write("Will do one");
          data.endPacket();
          isOpen = false;
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("Already Closed");
          data.endPacket();
        }
      } 

      if(cmmnd == "Open"){
        if(!isOpen){
        //Chester.write(45);
        //delay(100);
        //currentAng = Chester.read();
          data.beginPacket(data.remoteIP(), data.remotePort());
        //data.write("%d", currentAng);
          data.write("Will do two");
          data.endPacket();
          isOpen = true;
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort);
          data.write("Already Open");
          data.endPacket();
        }
      }
      for(int k = 0; k < 24; k++){
        packetBuffer[k] = '\u0000';
      }
    }
      //HEARTBEAT, BABY
    beatTimer++; //increment timer for heartbeat check
    if(beatTimer >= 100){
      //Clear Packet buffer
      for(int k = 0; k < 24; k++){
      packetBuffer[k] = '\u0000';
      }
      //Write heartbeat packet and send it
      data.beginPacket(data.remoteIP(), data.remotePort());
      data.write("Joemama");
      data.endPacket();
      delay(100);
      //Check the data sent back and compare to what was sent
      dataLength = data.parsePacket();
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      heartCheck = packetBuffer;
      if(heartCheck == heartData){
        heartbeatCount++;
      }else{ 
        if(currentAng != 0){ //Check if open or closed
          //Chester.write(-45); //If it's not closed, it'll be at 45 degrees
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("close pls");
          data.endPacket();
          //Wait indefinitely, will only do something
          //once turned off, then on again.
          for(;;){}
        }else{
          data.beginPacket(data.remoteIP(), data.remotePort());
          data.write("Oops");
          data.endPacket();
          for(;;){}
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
