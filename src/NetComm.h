// #include <NativeEthernet.h>
// #include <NativeEthernetUdp.h>
#include <QNEthernet.h>

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
    HONK,
    NEXTSTAGE,
    BACKSTAGE
};

bool initialiseEthernet();
void setStatus(String newStatus);

CMD getCMD(String packet);
String readPacket();
void sendPacket(String response);
void setStatus(String newStatus);
String getLoggedInIP();