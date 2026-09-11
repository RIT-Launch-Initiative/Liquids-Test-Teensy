#include <Arduino.h>
#include "Actuator.h"
// #include <TeensyThreads.h>

/**
 * Constructor for servo valve object
 */
ServoValve::ServoValve(int pinNum, int fullyOpen, int fullyClosed){
    Serial.print("servo created for pin number: ");
    Serial.println(pinNum);
    this->pinNum = pinNum;
    this->fullyOpen = fullyOpen;
    this->fullyClosed = fullyClosed;
    Serial.print("values: ");
    Serial.println(pinNum);
    Serial.println(this->fullyOpen);
    Serial.println(this->fullyClosed);
    this->setServoPosition(this->fullyClosed);
    Serial.print("position in constructor: ");
    Serial.println(this->servo.readMicroseconds());
}
/**
 * Initialise servo valve assembly. Sets to fully closed position then attaches to pin.
 */
void ServoValve::init(){
    // this->setServoPosition(this->fullyClosed);


    Serial.print("values: ");
    Serial.println(pinNum);
    Serial.println(this->fullyOpen);
    Serial.println(this->fullyClosed);

    
    Serial.print("position before attach: ");
    Serial.println(this->servo.readMicroseconds());
    this->servo.attach(this->pinNum);
    Serial.print("position after attach: ");
    Serial.println(this->servo.readMicroseconds());
    // this->setServoPosition(this->fullyClosed);//TODO: VERIFY WHICH IS PROPER ORDER.
}

/**
 * Deconstructor for servo valve object
 */
ServoValve::~ServoValve(){
    Serial.print("servo for pin ");
    Serial.print(this->pinNum);
    Serial.println(" detached and deconstucted");
    this->servo.detach();
}

/**
 * Set position of servo valve to specified position in microseconds
 */
void ServoValve::setServoPosition(int position){
    Serial.print("writing position value: ");
    Serial.println(position);
    Serial.println(this->servo.readMicroseconds());
    this->servo.writeMicroseconds(position);
    Serial.println("Position reads as: ");
    Serial.println(this->servo.readMicroseconds());
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
    pinMode(pinNum, OUTPUT);
}

/**
 * Activate the pin for the associate SolenoidQD
 */
void SolenoidQD::disconnectQD(){
    digitalWrite(this->pinNum, HIGH);
    delay(500);//TODO: determine if delay is needed or better to handle in main loop
    digitalWrite(this->pinNum, LOW);
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
    Serial.println("HONK HONK BITCH");
    digitalWrite(this->hornNum, HIGH);
    hornOn = true;
    hornOffTime = millis()+500;

}
/**
 * Activate horn for 5 seconds
 */
void LightTree::longHorn(){
    Serial.println("HOOOOOONNNKKKKKKK BITCH");
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
    pinMode(this->ignitePin, OUTPUT);
    pinMode(this->sensePinHigh, INPUT);
    pinMode(this->sensePinLow, INPUT);
}
/**
 * Send output high for one second to ignite the ignitor
 */
void Ignitor::ignite(){
    Serial.println("light that bitch up");
    digitalWrite(this->ignitePin, HIGH);
    //TODO: read sense pin for signal to go low, trigger any faults as needed
    delay(this->igDelayMilliseconds);//TODO: determine best case to handle ignite function with continuity check. likely unavoidable delay but ideally would be very short.
    digitalWrite(this->ignitePin, LOW);
    this->status=100;
    Serial.println("snuff that bitch out");
}
bool Ignitor::setIgDelayTime(int delayMillisReq){
    if(delayMillisReq > 500){
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