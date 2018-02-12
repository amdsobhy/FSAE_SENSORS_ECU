// Author: Ahmed Sobhy
// Date: 02/08/2018
// Project: UNB SAE EV 
// Pedal Sensor Interface

#include <stdbool.h>
#include <stdint.h>
#include "adc_api.h"

#define SIZE	10



void ThrottleSensorInit(void)
{
	ADCInit();
}

void ThrottleSensorGetValue(uint32_t * readings)
{
	ADCGetValue(readings);
}
