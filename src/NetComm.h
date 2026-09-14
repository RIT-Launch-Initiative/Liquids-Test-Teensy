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
    BACKSTAGE,
    LOCKOUT,
    UNLOCK,
    //temp states
    REDLIGHT,
    YELLOWLIGHT,
    GREENLIGHT,
    NOLIGHT,
    SENDIT,
    DELAYIG
};

bool initialiseEthernet();
void setStatus(String newStatus);
int extractIntFromString(const std::string& str);
int getIgDelayMillisRequested();

CMD getCMD(String packet);
String readPacket();
void sendPacket(String response);
void setStatus(String newStatus);
String getLoggedInIP();