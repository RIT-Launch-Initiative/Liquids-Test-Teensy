#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

enum class CMD{
    NONE,
    SPECIAL,
    FAULT,
    STATUS,
    OXDISCONNECT,
    IPADISCONNECT,
    OXOPEN,
    OXCLOSE,
    IPAOPEN,
    IPACLOSE,
    MAINOPEN,
    MAINCLOSE,
    IGNITE,
    HONK
};

bool initialiseEthernet();
void setStatus(String newStatus);

CMD getCMD();
String readPacket();
void sendPacket(String response);
void setStatus(String newStatus);
String getLoggedInIP();