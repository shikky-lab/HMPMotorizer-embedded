#include "VL53L0XWrapper.hpp"
#include "VL53L0X.h"

void VL53L0XWrapper::bootOn(){
    digitalWrite(shutdown_pin,HIGH);
}

void VL53L0XWrapper::shutDown(){
    digitalWrite(shutdown_pin,LOW);
}

VL53L0XWrapper::VL53L0XWrapper(uint8_t shutdown_pin){
    this->shutdown_pin = shutdown_pin;
    pinMode(shutdown_pin,OUTPUT);
    shutDown();
}

int VL53L0XWrapper::init(uint8_t address){
    if(address > 127){
        return -1;
    }
    this->address = address;
    bootOn();
    if(!sensor.init()){
        Serial.println("Failed to detect and initialize sensor!");
        return -1;
    }
    sensor.setTimeout(500);
    sensor.setAddress(address);
    return 0;
}
uint16_t VL53L0XWrapper::readRangeSingleMillimeters(){
    return sensor.readRangeSingleMillimeters();
}
bool VL53L0XWrapper::isInnnerRange(int range){
    uint16_t val = readRangeSingleMillimeters();  
    if(timeoutOccurred()){
        setDist(10000);
        return false;
    }
    setDist(val);
    if(val<range && val>30){
        return true;
    }
    return false;

}
bool VL53L0XWrapper::timeoutOccurred(){
    return sensor.timeoutOccurred();
}