/*
MIT License

Copyright (c) [2017] [Ahmed Sobhy]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"

/**************************************************************************
* @brief  This function Initializes the I2C1 driver in TM4C123GXL using
*				  pins A6 and A7 as SCL and SDL. I2C1 is setup as Master
* @return none
***************************************************************************/
void i2cDriverInit(void)
{	

		//
    // The I2C1 peripheral must be enabled before use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    //
    // For this example I2C0 is used with PortB[3:2].  The actual port and
    // pins used may be different on your part, consult the data sheet for
    // more information.  GPIO port A needs to be enabled so these pins can
    // be used.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for I2C1 functions on port A6 and A7.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
		
		//
    // Select the I2C function for these pins.  This function will also
    // configure the GPIO pins pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.  Consult the data sheet
    // to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
		//
    // Enable and initialize the I2C1 master module.  Use the system clock for
    // the I2C1 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.  For this example we will use a data rate of 100kbps.
    //
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);
		
}

/**************************************************************************
* @brief  This function sends data to a device using I2C1 as Master 
*	@param	\address is the address of the device to send data to
* @param  \data is the data to be sent
* @return none
***************************************************************************/
void i2cDriverWrite(uint8_t address, uint8_t data)
{

		I2CMasterSlaveAddrSet(I2C1_BASE, address, false);
		I2CMasterDataPut(I2C1_BASE, data);
		I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
		while(I2CMasterBusy(I2C1_BASE));	

}
