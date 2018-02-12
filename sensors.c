// Author: Ahmed Sobhy
// Date: 02/08/2018
// Project: UNB SAE EV 
// Sensors API

#include <stdbool.h>
#include <stdint.h>
#include "adc_api.h"


/******************************************************************************
Description: contains APIs that interface with several sensors and returns
their readings. Readings are stored into a 32-bits array with their location
from 0 to 3 as follows:

Sensor 1: Analog 0-5V Throttle Sensor 				connected to PE3 on AIN0
Sensor 2: Analog 0-5V Break Pressure Sensor 	connected to PE1 on AIN2
Sensor 3: Analog 0-5V Steering Sensor 				connected to PD1 on AIN6
******************************************************************************/


void ThrottleSensorInit(void)
{
	ADCTimerTriggeredInit();
}

uint32_t ThrottleSensorGetValue(void)
{
	return ADCGetSensor1();
}


