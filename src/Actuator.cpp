#include <Arduino.h>
#include "Actuator.h"
// #include <TeensyThreads.h>

static const int IGNITOR_HOLD_TIME = 500;
static const int QD_SOLENOID_HOLD_TIME = 15000;
//TODO: check types for recently added busy-loop timing blocks

/**
 * Constructor for servo valve object
 */
ServoValve::ServoValve(int pinNum, int fullyOpen, int fullyClosed){
    this->pinNum = pinNum;
    this->fullyOpen = fullyOpen;
    this->fullyClosed = fullyClosed;
    this->setServoPosition(this->fullyClosed);//this is reduntant logic between constructor and init() function. working around Teensyduino startup issue, need further testing to determine proper setup sequence
}
/**
 * Initialise servo valve assembly. Sets to fully closed position then attaches to pin.
 */
void ServoValve::init(){
    this->servo.attach(this->pinNum);
    this->setServoPosition(this->fullyClosed);
    //Teensyduino servo library initialises servo position to 1500 microseconds (partially open) upon attaching, and seemingly cannot be changed through config
    //empirical testing shows that attaching and then immediately setting to fully closed position does not cause any unexpected behavior
}

/**
 * Deconstructor for servo valve object
 */
ServoValve::~ServoValve(){
    // Serial.print("servo for pin ");
    // Serial.print(this->pinNum);
    // Serial.println(" detached and deconstucted");
    this->servo.detach();
}

/**
 * Set position of servo valve to specified position in microseconds
 */
void ServoValve::setServoPosition(int position){
    this->servo.writeMicroseconds(position);
}

/**
 * Set the servo valve to the fully open position
 */
void ServoValve::openValve(){
    this->setServoPosition(this->fullyOpen);
}

 /**
  * Set the servo valve to the fully closed position
  */
 void ServoValve::closeValve(){
    this->setServoPosition(this->fullyClosed);
 }

 /**
  * Constructor for SolenoidQD object
  */
SolenoidQD::SolenoidQD(int pinNum){
    this->pinNum = pinNum;
    this->isActive = false;
    pinMode(pinNum, OUTPUT);
}

/**
 * Activate the pin for the associate SolenoidQD
 * this is a non-blocking sequence, so output will remain high after function is exited and will be turned off by tickSolenoid() function
 */
void SolenoidQD::disconnectQD(){
    digitalWrite(this->pinNum, HIGH);
    this->isActive = true;
    solenoidOffTime = millis() + QD_SOLENOID_HOLD_TIME;//set timer for current time + length of hold time
}

/**
 * called periodically in main loop to check active status and off time, and reset status to low as needed
 */
void SolenoidQD::tickSolenoid(){
    if(this->isActive){
        if(this->solenoidOffTime < millis()){
            digitalWrite(this->pinNum, LOW);
            this->isActive = false;
        }
    }
}

/**
 * Constructor for light tree
 * Only one of these should ever exist
 */
LightTree::LightTree(int hornNum, int greenNum, int yellowNum, int redNum){
    this->greenNum = greenNum;
    this->yellowNum = yellowNum;
    this->redNum = redNum;
    this->hornNum = hornNum;
    pinMode(this->greenNum, OUTPUT);
    pinMode(this->yellowNum, OUTPUT);
    pinMode(this->redNum, OUTPUT);
    pinMode(this->hornNum, OUTPUT);
}
/**
 * Set each of the light outputs to the provided values
 */
void LightTree::setLightStatus(int greenStatus, int yellowStatus, int redStatus){
    digitalWrite(this->greenNum, greenStatus);
    digitalWrite(this->yellowNum, yellowStatus);
    digitalWrite(this->redNum, redStatus);
}
/**
 * Turn on the green light, and all others off
 */
void LightTree::setLightGreen(){
    this->setLightStatus(HIGH, LOW, LOW);
}
/**
 * Turn on the yellow light, and all others off
 */
void LightTree::setLightYellow(){
    this->setLightStatus(LOW, HIGH, LOW);
}
/**
 * Turn on the red light, and all others off
 */
void LightTree::setLightRed(){
    this->setLightStatus(LOW, LOW, HIGH);
}
/**
 * Turn off all lights
 */
void LightTree::setNoLights(){
    this->setLightStatus(LOW, LOW, LOW);
}

/**
 * Flash the red light at 1 second intervals, all other lights off
 */
void LightTree::setLightRedFlash(bool on){
    this->redLightFlashing = on;
    if(this->redLightFlashing){
        this->setLightRed();
        this->redLightToggleTime = millis()+2000;
        Serial.println("red light flash enabled");
    }
    //TODO: do we need an else case or will lightTtree tick handle this?
}
/**
 * Activate horn for 0.5 seconds
 */
void LightTree::shortHorn(){
    digitalWrite(this->hornNum, HIGH);
    hornOn = true;
    hornOffTime = millis()+500;

}
/**
 * Activate horn for 5 seconds
 */
void LightTree::longHorn(){
    digitalWrite(this->hornNum, HIGH);
    hornOn=true;
    hornOffTime = millis()+5000;
}
/**
 * Activate horn for 10 seconds
 */
void LightTree::turnOffHorn(){
    digitalWrite(this->hornNum, LOW);
    hornOn=false;
}
/**
 * tick the status of lights and horn
 */
void LightTree::tickLights(){
    int curTime = millis();
    if(hornOn){
        if(hornOffTime < curTime){
            this->turnOffHorn();
        }
    }
    if(redLightFlashing){
        if(redLightToggleTime < curTime){

            //toggle red light
            Serial.println("toggle red light high/low");
        }
    }
}

/**
 * Constructor for Ignitor object
 */
Ignitor::Ignitor(int ignitePin, int sensePinHigh, int sensePinLow){
    this->ignitePin = ignitePin;
    this->sensePinHigh = sensePinHigh;
    this->sensePinLow = sensePinLow;
    this->status = 1;
    this->isActive = false;
    pinMode(this->ignitePin, OUTPUT);
    pinMode(this->sensePinHigh, INPUT);
    pinMode(this->sensePinLow, INPUT);
}
/**
 * Send output high for one second to ignite the ignitor
 * this is a non-blocking function, output will remain high after function is exited and will be set low by tickIgnitor() function
 */
void Ignitor::ignite(){
    digitalWrite(this->ignitePin, HIGH);
    this->isActive = true;
    this->ignitorOffTime = millis() + IGNITOR_HOLD_TIME;//set off time to current + length of hold time
}

/**
 * called periodically in main loop to check active status and off time, and reset status to low as needed
 */
void Ignitor::tickIgnitor(){
    if(this->isActive){
        if(this->ignitorOffTime < millis()){
            digitalWrite(this->ignitePin, LOW);
            this->isActive = false;
        }
    }
}

/**
 * function to update the value of the delay time for ignitor. this function is un-used, and will be repurposed later to adjust ignitor hold time
 */
bool Ignitor::setIgDelayTime(int delayMillisReq){
    if(delayMillisReq > -1){
        this->igDelayMilliseconds = delayMillisReq;
        Serial.println("updated ignitor timing delay to " + String(this->igDelayMilliseconds) + " milliseconds");
        return true;
    }
    else{
        Serial.println("Rejected a timing change request of "+ String(this->igDelayMilliseconds) +" milliseconds");
        return false;
    }
}

/**
 * Check continuity between pinsSenseHigh and pinSenseLow to check continuity of ignitor
 * return true if there is continuity
 * this function is unused and will be updated when procedure for checking continuity is finalized
 */
bool Ignitor::checkContinuity(){
    //TODO: check continuity between pinSenseHigh and pinSenseLow , return true if there is continuity
    return false;
}

/**
 * return a json formatted string of all actuator statuses
 */
String getActuatorStatus(){
    return "{\"MainServo\": \"status\", \"OxServo\": \"status\", \"IPAServo\": \"status\"}";
}