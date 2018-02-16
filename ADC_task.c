#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "utils/uartstdio.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "lcd_i2c.h"
#include "sensors.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define ADCTASKSTACKSIZE				128				 // Stack size in words

//*****************************************************************************
//
// The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define ADC_ITEM_SIZE					 sizeof(uint32_t)*1				// change to four sensors reading in the future
#define ADC_QUEUE_SIZE				 1


xQueueHandle g_pADCQueue;
extern xSemaphoreHandle g_pADCSemaphore;


//*****************************************************************************
//
// This task toggles the user selected LED at a user selected frequency. User
// can make the selections by pressing the left and right buttons.
//
//*****************************************************************************
static void ADCTask(void *pvParameters)
{

	uint32_t ThrottleValue;
	
	while(1)
		{
			
			xSemaphoreTake(g_pADCSemaphore, portMAX_DELAY);
			ThrottleValue = ThrottleSensorGetValue();
			xSemaphoreGive(g_pADCSemaphore);
			
			xQueueSend( g_pADCQueue, &ThrottleValue, 0);
			
			// this task runs every 5ms
			vTaskDelay(5);
			
		}
}

//*****************************************************************************
//
// Initializes the LCD task.
//
//*****************************************************************************
uint32_t ADCTaskInit(void)
{
	
		ThrottleSensorInit();
		//
		// Print the current loggling LED and frequency.
		//
		UARTprintf("\nADC task running!!");
		
		//
		// Create a queue for sending messages to the LCD task.
		//
		g_pADCQueue = xQueueCreate(ADC_QUEUE_SIZE, ADC_ITEM_SIZE);

		//
		// Create the LED task.
		//
		if(xTaskCreate(ADCTask, (const portCHAR *)"ADC", ADCTASKSTACKSIZE, NULL,
									 tskIDLE_PRIORITY + PRIORITY_ADC_TASK, NULL) != pdTRUE)
		{
				return(1);
		}

		//
		// Success.
		//
		return(0);

}
