#include <Servo.h>

String getActuatorStatus();

class ServoValve{
    private:
        int pinNum, fullyClosed, fullyOpen;
        Servo servo;

    public:
        ServoValve(int pinNum, int fullyOpen, int fullyClosed);
        void init();
        ~ServoValve();
        void openValve();
        void closeValve();
        void setServoPosition(int position);
};

class SolenoidQD{
    private:
        int pinNum;
        bool isActive;
        int solenoidOffTime;

    public:
        SolenoidQD(int pinNum);
        void disconnectQD();
        void tickSolenoid();
};

class LightTree{
    private:
        int greenNum, yellowNum, redNum, hornNum;
        int hornOffTime, redLightToggleTime;
        bool hornOn;

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
        void turnOffHorn();
        void tickLights();
};

class Ignitor{
    private:
        int ignitePin, sensePinHigh, sensePinLow;
        int status;
        int igDelayMilliseconds = 1000;
        bool isActive;
        int ignitorOffTime;

    public:
        Ignitor(int ignitePin, int sensePinHigh, int sensePinLow);
        void ignite();
        bool checkContinuity();
        bool setIgDelayTime(int delayMillis);
        void tickIgnitor();
};