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

static volatile uint32_t g_ui32ADC0Reading = 0;

//*****************************************************************************
//
// Counters used to count how many times certain ISR event occur.
//
//*****************************************************************************
static uint32_t g_ui32uDMAErrCount = 0;

//*****************************************************************************
//
// The control table used by the uDMA controller.	This table must be aligned
// to a 1024 byte boundary.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
tDMAControlTable sControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(sControlTable, 1024)
tDMAControlTable sControlTable[1024];
#else
tDMAControlTable sControlTable[1024] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// Declaration of preload structure, referred to by the task list.
// It will be defined later.
//
//*****************************************************************************
tDMAControlTable g_sADC0TaskListPreload;

//*****************************************************************************
//
// This is the task list that defines the DMA scatter-gather operation.
// Each "task" in the task list gets copied one at a time into the alternate
// control structure for the channel, where it is then executed.	Each time
// the DMA channel is triggered, it executes these tasks.
//
//*****************************************************************************
tDMAControlTable g_ADC0TaskTable[] =
{
		//
		// Task 1: transfer ADC data
		// Copy available data from ADC FIFO to buffer
		//
		{
			(void*) (ADC0_BASE + ADC_O_SSFIFO0),							// src addr is ADC0SS0 FIFO
			(void*) &g_ui32ADC0Reading,											 // dst addr ADC read var
			UDMA_CHCTL_SRCSIZE_32 | UDMA_CHCTL_SRCINC_NONE |	// src is a 32 bit word and there is no increment
			UDMA_CHCTL_DSTSIZE_32 | UDMA_CHCTL_DSTINC_NONE |	// dst is a 32 bit word and there is no increment
			UDMA_CHCTL_ARBSIZE_1 |														// arb size is 1
			((1 - 1) << UDMA_CHCTL_XFERSIZE_S) |							// transfer size is 1
			UDMA_CHCTL_XFERMODE_MEM_SGA											 // memory scatter-gather
		},

		//
		// Task 2: reset task list
		// Reprogram this DMA channel by reloading the primary structure with
		// the pointers needed to copy the task list for SG mode.
		// Only the control word actually needs to be reloaded.
		// This will allow another DMA transfer to start on this channel the
		// next time a peripheral request occurs.
		//
		{
			&(g_sADC0TaskListPreload.ui32Control),					 // src addr is the task reload control word
			&(sControlTable[UDMA_CHANNEL_ADC0].ui32Control), // dst addr is the ADC0 primary control word in the master uDMA control table
			UDMA_CHCTL_SRCSIZE_32 | UDMA_CHCTL_SRCINC_NONE | // src is a 32 bit word and there is no increment
			UDMA_CHCTL_DSTSIZE_32 | UDMA_CHCTL_DSTINC_NONE | // dst is a 32 bit word and there is no increment
			UDMA_CHCTL_ARBSIZE_1 |													 // arb size is 1
			((1 - 1) << UDMA_CHCTL_XFERSIZE_S) |						 // transfer size is 1
			UDMA_CHCTL_XFERMODE_PER_SGA											// peripheral scatter-gather
		}
};

//*****************************************************************************
//
// This is the preloaded channel control structure for the ADC channel.	The
// values in this structure are configured to perform a memory scatter-gather
// operation whenever a request is received.	This structure is copied over
// into the DMA control table by software at the beginning of this app.	After
// that, it is reloaded into the control table by a scatter-gather operation.
//
// Transfer is 2 uDMA tasks, comprising 4 32-bit words each, for a total of 8 transfers
// Source of the scatter gather transfer is the start of the task list.
// Note that it must point to the last location to copy.
// Destination is the alternate structure for the ADC channel
// Setup transfer parameters for peripheral scatter-gather,
// The arb size is set to 8 to allow the full transfer to happen in one
// transaction.
//
//****************************************************************************
tDMAControlTable g_sADC0TaskListPreload =
{
		&(g_ADC0TaskTable[1].ui32Spare),																	// src addr is the end of the ADC0 read task list
		&(sControlTable[UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT].ui32Spare),	// dst addr is the end of the ADC0 channel in the master uDMA control table
		UDMA_CHCTL_SRCSIZE_32 | UDMA_CHCTL_SRCINC_32 |										// src is 32 bit words
		UDMA_CHCTL_DSTSIZE_32 | UDMA_CHCTL_DSTINC_32 |										// dest is 32 bit words
		UDMA_CHCTL_ARBSIZE_4 |																						// arb size is 4
		((8 - 1) << UDMA_CHCTL_XFERSIZE_S) |															// transfer size is 8
		UDMA_CHCTL_XFERMODE_MEM_SG																				// memory scatter-gather
};

void
ConfigADC0(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_ALWAYS, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 0);
}

//*****************************************************************************
//
// This function loads the channel control structure with the starting values,
// and then enables the DMA channel.
//
//*****************************************************************************
void
ConfigADC0SGTaskList(void)
{
		//
		// Copy the value of the preload task list into the primary structure
		// to initialize it for the first time.	After the DMA starts it will be
		// re-initialized each time by the third s-g task.
		//
		sControlTable[UDMA_CHANNEL_ADC0].pvDstEndAddr = g_sADC0TaskListPreload.pvDstEndAddr;
		sControlTable[UDMA_CHANNEL_ADC0].pvSrcEndAddr = g_sADC0TaskListPreload.pvSrcEndAddr;
		sControlTable[UDMA_CHANNEL_ADC0].ui32Control = g_sADC0TaskListPreload.ui32Control;

		//
		// Enable the ADC DMA channel.	This will allow it to start running when
		// it receives a request from the peripheral
		//
		uDMAChannelEnable(UDMA_CHANNEL_ADC0);
}

//*****************************************************************************
//
// The interrupt handler for uDMA errors.	This interrupt will occur if the
// uDMA encounters a bus error while trying to perform a transfer.	This
// handler just increments a counter if an error occurs.
//
//*****************************************************************************
void
uDMAErrorHandler(void)
{
		//
		// Check for uDMA error bit
		//
		uint32_t ui32Status = uDMAErrorStatusGet();

		//
		// If there is a uDMA error, then clear the error and increment
		// the error counter.
		//
		if(ui32Status) {
				uDMAErrorStatusClear();
				g_ui32uDMAErrCount++;
		}
}

void AdcSgInit(void)
{
			SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
		uDMAEnable();
		uDMAControlBaseSet(sControlTable);
		uDMAIntRegister(INT_UDMAERR, uDMAErrorHandler);
		IntEnable(INT_UDMAERR);

		ConfigADC0();
		ConfigADC0SGTaskList();
}

uint32_t AdcSgRead(void)
{
	return g_ui32ADC0Reading;
}

//*****************************************************************************
//
// single_ended.c - Example demonstrating how to configure the ADC for
//									single ended operation.
//
// Copyright (c) 2010-2016 Texas Instruments Incorporated.	All rights reserved.
// Software License Agreement
// 
//	 Redistribution and use in source and binary forms, with or without
//	 modification, are permitted provided that the following conditions
//	 are met:
// 
//	 Redistributions of source code must retain the above copyright
//	 notice, this list of conditions and the following disclaimer.
// 
//	 Redistributions in binary form must reproduce the above copyright
//	 notice, this list of conditions and the following disclaimer in the
//	 documentation and/or other materials provided with the	
//	 distribution.
// 
//	 Neither the name of Texas Instruments Incorporated nor the names of
//	 its contributors may be used to endorse or promote products derived
//	 from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This is part of revision 2.1.3.156 of the Tiva Firmware Development Package.
//
//*****************************************************************************
/*
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
*/
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
		SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

		//
		// For this example ADC0 is used with AIN0 on port E7.
		// The actual port and pins used may be different on your part, consult
		// the data sheet for more information.	GPIO port E needs to be enabled
		// so these pins can be used.
		// TODO: change this to whichever GPIO port you are using.
		//
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	
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
		ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

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
		ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0 );
		ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH2 );		
		ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH6 );
		ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH6 | ADC_CTL_IE | ADC_CTL_END);
		
		//
		// Since sample sequence 3 is now configured, it must be enabled.
		//
		ADCSequenceEnable(ADC0_BASE, 1);

		//
		// Clear the interrupt status flag.	This is done to make sure the
		// interrupt flag is cleared before we sample.
		//
		ADCIntClear(ADC0_BASE, 1);
		
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
		ADCProcessorTrigger(ADC0_BASE, 1);

		//
		// Wait for conversion to be completed.
		//
		while(!ADCIntStatus(ADC0_BASE, 1, false))
		{
		}

		//
		// Clear the ADC interrupt flag.
		//
		ADCIntClear(ADC0_BASE, 1);

		//
		// Read ADC Value.
		//
		ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
		
}




