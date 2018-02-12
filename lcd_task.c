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
#include "throttle_sensor.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define LCDTASKSTACKSIZE				128				 // Stack size in words

//*****************************************************************************
//
// The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define LCD_ITEM_SIZE					 sizeof(uint8_t)*16
#define LCD_QUEUE_SIZE					1


extern xQueueHandle g_pADCQueue;


extern xSemaphoreHandle g_pLCDSemaphore;

void itoascii(uint32_t val, char * str)
{
		uint32_t temp_val;
		temp_val = val;
		str[0] = (temp_val / 10000) + 48;
		temp_val = (temp_val % 10000);
		str[1] = (temp_val / 1000) + 48;
		temp_val = (temp_val % 1000);
		str[2] = (temp_val / 100) + 48;
		temp_val = (temp_val % 100);
		str[3] =  (temp_val / 10) + 48;
		temp_val = (temp_val % 10);
		str[4] = temp_val + 48;
		str[5] = '\0';
}

//*****************************************************************************
//
// This task toggles the user selected LED at a user selected frequency. User
// can make the selections by pressing the left and right buttons.
//
//*****************************************************************************
static void LCDTask(void *pvParameters)
{
	
	lcdI2cInit(0x3f,16,2,0);
	int32_t adcReading[4];
	char buffer[5];
	
	while(1)
		{
			
			 xQueueReceive( g_pADCQueue, &( adcReading ), ( TickType_t ) 10 );
			 itoascii(adcReading[0],buffer);
			 xSemaphoreTake(g_pLCDSemaphore, portMAX_DELAY);
			 lcdI2cPrint(buffer);
			 xSemaphoreGive(g_pLCDSemaphore);
			 lcdI2cHome();
			 vTaskDelay(100);
			
		}
}

//*****************************************************************************
//
// Initializes the LCD task.
//
//*****************************************************************************
uint32_t LCDTaskInit(void)
{
		//
		// Print the current loggling LED and frequency.
		//
		UARTprintf("\nLCD task running!!");

		//
		// Create the LED task.
		//
		if(xTaskCreate(LCDTask, (const portCHAR *)"LCD", LCDTASKSTACKSIZE, NULL,
									 tskIDLE_PRIORITY + PRIORITY_LCD_TASK, NULL) != pdTRUE)
		{
				return(1);
		}

		//
		// Success.
		//
		return(0);
		
}
