/*
 * VL53L0XWrapper.hpp
 *
 *  Created on: Jul 18, 2019
 *      Author: mfukunaga
 */

#ifndef VL53L0XWRAPPER_HPP_
#define VL53L0XWRAPPER_HPP_

#include <cstdint>
#include <Arduino.h>
#include "VL53L0X.h"

class VL53L0XWrapper{
private:
  uint8_t shutdown_pin;
  uint8_t address;
  VL53L0X sensor;
	void bootOn();
	void shutDown();
public:
  VL53L0XWrapper(uint8_t shutdown_pin);
  int init(uint8_t address);
  uint16_t readRangeSingleMillimeters();
  bool timeoutOccurred();
  bool isInnnerRange(int range);
};

#endif /* VL53L0XWRAPPER_HPP_ */