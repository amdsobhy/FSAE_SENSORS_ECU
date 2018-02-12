// AIN0 AIN2 AIN6 will be used as single ended
// if sensors turn out to be differential then the succesive AIN can be used

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_adc.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_udma.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

void ADC0IntHandler(void);

static volatile uint32_t g_ui32ADC0Reading = 0;

static uint32_t ADCData[4];

//*****************************************************************************
//
//! \addtogroup adc_examples_list
//! <h1>Single Ended ADC (single_ended)</h1>
//!
//! This example shows how to setup ADC0 as a single ended input and take a
//! single sample on AIN0/PE3.
//!
//! This example uses the following peripherals and I/O signals.	You must
//! review these and change as needed for your own board:
//! - ADC0 peripheral
//! - GPIO Port E peripheral (for AIN0 pin)
//! - AIN0 - PE3

// AIN0 AIN2 AIN6 will be used as single ended -> pins PE3, PE1, PD1 respectively
// if sensors turn out to be differential then the succesive AIN can be used

void ADCInit(void)
{
		// The ADC0 peripheral must be enabled for use.
		//
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

		//
		// For this example ADC0 is used with AIN0 on port E7.
		// The actual port and pins used may be different on your part, consult
		// the data sheet for more information.	GPIO port E needs to be enabled
		// so these pins can be used.
		// TODO: change this to whichever GPIO port you are using.
		//
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	
		//
		// Select the analog ADC function for these pins.
		// Consult the data sheet to see which functions are allocated per pin.
		// TODO: change this to select the port/pin you are using.
		//
		MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_1 );
		MAP_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);
		//
		// Enable sample sequence 3 with a processor signal trigger.	Sequence 3
		// will do a single sample when the processor sends a signal to start the
		// conversion.	Each ADC module has 4 programmable sequences, sequence 0
		// to sequence 3.	This example is arbitrarily using sequence 3.
		//
		MAP_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

		//
		// Configure step 0 on sequence 3.	Sample channel 0 (ADC_CTL_CH0) in
		// single-ended mode (default) and configure the interrupt flag
		// (ADC_CTL_IE) to be set when the sample is done.	Tell the ADC logic
		// that this is the last conversion on sequence 3 (ADC_CTL_END).	Sequence
		// 3 has only one programmable step.	Sequence 1 and 2 have 4 steps, and
		// sequence 0 has 8 programmable steps.	Since we are only doing a single
		// conversion using sequence 3 we will only configure step 0.	For more
		// information on the ADC sequences and steps, reference the datasheet.
		//
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0 );
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH2 );		
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH6 );
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH6 | ADC_CTL_IE | ADC_CTL_END);
		
		//
		// Since sample sequence 3 is now configured, it must be enabled.
		//
		MAP_ADCSequenceEnable(ADC0_BASE, 1);

		//
		// Clear the interrupt status flag.	This is done to make sure the
		// interrupt flag is cleared before we sample.
		//
		MAP_ADCIntClear(ADC0_BASE, 1);
		
}

	
void ADCGetValue(uint32_t* ADCValues)
{
	
		//
		// This array is used for storing the data read from the ADC FIFO. It
		// must be as large as the FIFO for the sequencer in use.	This example
		// uses sequence 3 which has a FIFO depth of 1.	If another sequence
		// was used with a deeper FIFO, then the array size must be changed.
		//
		uint32_t* pui32ADC0Value = ADCValues;
	
		//
		// Trigger the ADC conversion.
		//
		MAP_ADCProcessorTrigger(ADC0_BASE, 1);

		//
		// Wait for conversion to be completed.
		//
		while(!MAP_ADCIntStatus(ADC0_BASE, 1, false))
		{
		}

		//
		// Clear the ADC interrupt flag.
		//
		MAP_ADCIntClear(ADC0_BASE, 1);

		//
		// Read ADC Value.
		//
		MAP_ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
		
}


void ADCTimerTriggeredInit(void)
{
	
		// The ADC0 peripheral must be enabled for use.
		//
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

		//
		// For this example ADC0 is used with AIN0 on port E7.
		// The actual port and pins used may be different on your part, consult
		// the data sheet for more information.	GPIO port E needs to be enabled
		// so these pins can be used.
		// TODO: change this to whichever GPIO port you are using.
		//
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	
		//
		// Select the analog ADC function for these pins.
		// Consult the data sheet to see which functions are allocated per pin.
		// TODO: change this to select the port/pin you are using.
		//
		GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_1 );
		GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);
		//
		// Enable sample sequence 3 with a processor signal trigger.	Sequence 3
		// will do a single sample when the processor sends a signal to start the
		// conversion.	Each ADC module has 4 programmable sequences, sequence 0
		// to sequence 3.	This example is arbitrarily using sequence 3.
		//
		MAP_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_TIMER, 0);

		//
		// Configure step 0 on sequence 3.	Sample channel 0 (ADC_CTL_CH0) in
		// single-ended mode (default) and configure the interrupt flag
		// (ADC_CTL_IE) to be set when the sample is done.	Tell the ADC logic
		// that this is the last conversion on sequence 3 (ADC_CTL_END).	Sequence
		// 3 has only one programmable step.	Sequence 1 and 2 have 4 steps, and
		// sequence 0 has 8 programmable steps.	Since we are only doing a single
		// conversion using sequence 3 we will only configure step 0.	For more
		// information on the ADC sequences and steps, reference the datasheet.
		//
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0 );
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH2 );		
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH6 );
		MAP_ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH6 | ADC_CTL_IE | ADC_CTL_END);
		
		//
		// Since sample sequence 3 is now configured, it must be enabled.
		//
		MAP_ADCSequenceEnable(ADC0_BASE, 1);

		//
		// Clear the interrupt status flag.	This is done to make sure the
		// interrupt flag is cleared before we sample.
		//
		MAP_ADCIntClear(ADC0_BASE, 1);
	
	  // Enable the Timer peripheral
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Timer should run periodically
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Set the value that is loaded into the timer everytime it finishes
    // it's the number of clock cycles it takes till the timer triggers the ADC
		// frequency set to 1000Hz sampling rate set by the timer
    #define F_SAMPLE    1000
	
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/F_SAMPLE);

    // Enable triggering
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    // Enable processor interrupts.
    MAP_IntMasterEnable();
		ADCIntRegister(ADC0_BASE, 1, ADC0IntHandler);
    MAP_ADCIntEnable(ADC0_BASE, 1);

    // Enable the timer
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

}

void ADC0IntHandler(void) {
	
    // Clear the interrupt status flag.
    MAP_ADCIntClear(ADC0_BASE, 1);
	    // Read ADC Data
    MAP_ADCSequenceDataGet(ADC0_BASE, 1, ADCData);
	
}

uint32_t ADCGetSensor1() {
	return ADCData[0];
}

uint32_t ADCGetSensor2() {
	return ADCData[1];
}

uint32_t ADCGetSensor3() {
	return ADCData[2];
}
