#include <Servo.h>

class ServoValve{
    private:
        int pinNum, fullyClosed, fullyOpen;
        Servo servo;
        void setServoPosition(int position);

    public:
        ServoValve(int pinNum, int fullyOpen, int fullyClosed);
        void init();
        ~ServoValve();
        void openValve();
        void closeValve();
        int getStatus();
};

class SolenoidQD{
    private:
        int pinNum;

    public:
        SolenoidQD(int pinNum);
        void disconnectQD();
};

class LightTree{
    private:
        int greenNum, yellowNum, redNum, hornNum;
        bool redLightFlashing;
        void redLightFlashLoop();
        void setLightStatus(int greenStatus, int yellowStatus, int redStatus);

    public:
        LightTree(int greenNum, int yellowNum, int redNum, int hornNum);
        void setLightGreen();
        void setLightYellow();
        void setLightRed();
        void setNoLights();
        void setLightRedFlash(bool on);
        void shortHorn();
        void longHorn();
};

class Ignitor{
    private:
        int ignitePin, sensePin;
        int status;

    public:
        Ignitor(int ignitePin, int sensePin);
        void ignite();
        int getStatus();
};
//TODO: get status for all acutators and sensors, to be called in main and sent as a status message