#include <stdint.h>
#include <stdbool.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"


volatile uint32_t usec = 0;

//Interrupt handler for the timer


void SysTickInt(void)
{
  MAP_TimerIntClear( TIMER1_BASE, TIMER_TIMA_TIMEOUT );
  usec++;
}

void Timer1_Init(void)
{
	uint32_t period = (MAP_SysCtlClockGet()/1000000)-1 ;		// 1 us
	// Enable timer pin from system
	MAP_SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER1 );
	// Timer mode set to periodic timer 16 bit timer
	MAP_TimerDisable( TIMER1_BASE, TIMER_A );
  MAP_TimerConfigure( TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC_UP );
	// Load the period of systick
	MAP_TimerLoadSet( TIMER1_BASE, TIMER_A, period);
	// Clear interrupt flag
	MAP_TimerClockSourceSet( TIMER1_BASE, TIMER_CLOCK_SYSTEM );
	
	TimerIntRegister( TIMER1_BASE, TIMER_A, SysTickInt);
	MAP_TimerIntClear( TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	
	// Enable timer
	MAP_TimerEnable( TIMER1_BASE, TIMER_A);	
}

void delay_us(uint32_t utime)
{
	volatile uint32_t temp1 = usec;
	while((usec-temp1) < utime);
}

void delay_ms(uint32_t mtime)
{
	mtime *= 1000;
	volatile uint32_t temp2 = usec;
	while((usec-temp2) < mtime);
}

