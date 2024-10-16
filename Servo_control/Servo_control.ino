#include <PWMServo.h>
#include <SPI.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
//--------------------------------------------------------------
//Declare any globally important values
//Servo-related Values
PWMServo Chester;
int currentAng;

//Ethernet stuff
byte mac[] = {
  0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E      // Set the Mac Address
};
byte ip[] = {10, 10, 10, 75};
byte dns[] = {10, 10, 10, 1};
byte gateway[] = {169, 254, 147, 58};
byte subnet[] = {255, 255, 255, 0};
unsigned int localPort = 8080;
EthernetUDP data;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

//Packet-related values
uint16_t heartbeatCount = 0; //Amount of heartbeats
uint16_t beatTimer; //Timer to send out heartbeat packet
//String heartData = /*Decide what to send as heartbeat*/;
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
  //Chester.attach(3);

//Begin UDP connection
  data.begin(localPort);
}
//--------------------------------------------------------------
// Main loop
void loop() {
    //currentAng = Chester.read();
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

      data.beginPacket(data.remoteIP(), data.remotePort());
      data.write("acknowldeged");
      data.endPacket();

      //beatTimer++;
      //if(beatTimer >= /*Some amount of seconds*/){
        //Write heartbeat packet and send it
      //  delay(/*Scary time for heartbeat retrieveal*/)
        //Check the data sent back and compare to what was sent
      //  data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      //  if(packetBuffer == heartData){
      //    heartbeatCount++;
      //  }else{ 
      //    if(currentAng != 0){ //Check if open or closed
      //      Chester.write(0);
            //Wait indefinitely, will only do something
            //once turned off, then on again.
      //      for(;;){}
      //    }else{
      //      for(;;){}
      //    }
      //  }
      //}
    }
    //end packet check-----------------
  delay(10);
}
