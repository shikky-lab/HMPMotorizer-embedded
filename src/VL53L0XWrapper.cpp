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

int VL53L0XWrapper::init(uint8_t address,bool isContinuous){
    if(address > 127){
        return -1;
    }
    this->isContinuous=isContinuous;
    this->address = address;
    bootOn();
    if(!sensor.init()){
        Serial.println("Failed to detect and initialize sensor!");
        return -1;
    }
    sensor.setTimeout(500);
    sensor.setAddress(address);
    if(this->isContinuous){
        sensor.startContinuous();
    }
    return 0;
}

int VL53L0XWrapper::init(uint8_t address){
    return init(address,false);
}

uint16_t VL53L0XWrapper::readRangeSingleMillimeters(){
    return sensor.readRangeSingleMillimeters();
}

bool VL53L0XWrapper::isInnnerRange(int maxRange , int minRange){
    uint16_t val;
    if(this->isContinuous){
        val=readLastRange();
    }else{
        val = readRangeSingleMillimeters();  
    }
    setDist(val);
    if(minRange<val && val<maxRange){
        return true;
    }
    return false;
}


uint16_t VL53L0XWrapper::readLastRange(){
    return sensor.readRangeContinuousMillimeters();
}

bool VL53L0XWrapper::timeoutOccurred(){
    return sensor.timeoutOccurred();
}