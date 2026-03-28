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
}
/**
 * Initialise servo valve assembly. Sets to fully closed position then attaches to pin.
 */
void ServoValve::init(){
    this->setServoPosition(this->fullyClosed);
    this->servo.attach(this->pinNum);
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
    //delay(1500);
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

 int ServoValve::getStatus(){
    return this->servo.readMicroseconds();
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
    delay(500);
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
 * Continually flash red light at a duty cycle of 1 second on 1 second off, until 
 */
//TODO: set flags and use main loop to change logic for all light and horn continuos functions.
void LightTree::redLightFlashLoop(){
    while(true){
        if(redLightFlashing){
            setLightRed();
            Serial.println("red light on");
            delay(1000);
            setNoLights();
            Serial.println("red light off");
            delay(1000);
        }
        else{
            Serial.println("exiting red light flash sequence");
            return;
        }
    }
}
/**
 * Flash the red light at 1 second intervals, all other lights off
 */
void LightTree::setLightRedFlash(bool on){
    this->redLightFlashing = on;
    if(on){
        // threads.addThread(this->redLightFlashLoop);
        Serial.println("red light flash enabled");
    }
}
/**
 * Activate horn for 2 seconds
 */
void LightTree::shortHorn(){
    Serial.println("HONK HONK BITCH");
    digitalWrite(this->hornNum, HIGH);
    delay(500);
    digitalWrite(this->hornNum, LOW);

}
/**
 * Activate horn for 10 seconds
 */
void LightTree::longHorn(){
    Serial.println("HOOOOOONNNKKKKKKK BITCH");
    digitalWrite(this->hornNum, HIGH);
    delay(2000);
    digitalWrite(this->hornNum, LOW);
}

/**
 * Constructor for Ignitor object
 */
Ignitor::Ignitor(int ignitePin, int sensePin){
    this->ignitePin = ignitePin;
    this->sensePin = sensePin;
    this->status = 1;
    pinMode(this->ignitePin, OUTPUT);
    pinMode(this->sensePin, INPUT);
}
/**
 * Send output high for one second to ignite the ignitor
 */
void Ignitor::ignite(){
    Serial.println("light that bitch up");
    digitalWrite(this->ignitePin, HIGH);
    //TODO: read sense pin for signal to go low, trigger any faults as needed
    delay(1000);
    digitalWrite(this->ignitePin, LOW);
    this->status=100;
    Serial.println("snuff that bitch out");
}

int Ignitor::getStatus(){
    return this->status;
}