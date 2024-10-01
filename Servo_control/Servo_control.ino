//#include <PWMServo.h>
#include <SPI.h>
#include <NativeEthernet.h>

#include <NativeEthernetUdp.h>



// Enter a MAC address and IP address for your controller below.

// The IP address will be dependent on your local network:

byte mac[] = {  

  0x04, 0xE9, 0xE5, 0x11, 0xEF, 0x6E };

byte ip[] = {129, 69, 69, 69};
  IPAddress balls(10, 10, 10, 70);


unsigned int localPort = 8888;      // local port to listen on



// An EthernetUDP instance to let us send and receive packets over UDP

EthernetUDP Udp;



void setup() {

  // start the Ethernet and UDP:
  Serial.begin(38400);

  Ethernet.begin(mac,ip);

  Udp.begin(localPort);



}


void loop() {
  do{
    Serial.println(Ethernet.hardwareStatus());
    if (Udp.beginPacket(balls, 8888)){
      Udp.beginPacket(balls, 8888);
      Udp.write("lol");
      Udp.endPacket();
    }else{
      Serial.printf("lol");
    }
  }while(1);
}