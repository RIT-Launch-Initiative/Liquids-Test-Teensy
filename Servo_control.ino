#include <PWMServo.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
//--------------------------------------------------------------
//Declare any globally important values
PWMServo Chester;
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED      // Set the Mac Address
};
IPAddress ip(10, 10, 10, 75);
unsigned int localPort = 8080;
EthernetUDP data; 
uint16_t heartbeatCount = 0; //Amount of heartbeats
uint16_t beatTimer; //Timer to send out heartbeat packet
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
//--------------------------------------------------------------
// Arduino setup for servo comms and ethernet data reading
void setup() {
  Chester.attach(23);
  Ethernet.begin(mac, ip);
  data.begin(localPort);

  Serial.begin(/*Determine bit rate of the thing*/)
}
//--------------------------------------------------------------
// Main loop
void loop() {
  int currentAng;
  int dataLength;
  char heartData = /*Decide what to send as heartbeat*/;
  for(;;){
    currentAng = Chester.read();
    //Check for packet------------------
    dataLength = data.parsepacket();
    if (dataLength){
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      Serial.print("Received packet of size ");
      Serial.println(dataLength);
      Serial.print("From ");
      for (int i =0; i < 4; i++){
        Serial.print(remote[i], DEC);
        if (i < 3){
          Serial.print(".");
        }
      }
      Serial.print(", port ");
      Serial.println(Udp.remotePort());

      // read the packet into packetBuffer
      data.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      Serial.println("Contents:");
      Serial.println(packetBuffer);
      //beatTimer++;
      //if(beatTimer == /*Some amount of seconds*/){
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
  }
}
