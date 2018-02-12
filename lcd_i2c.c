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
#include <stddef.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "lcd_i2c.h"
#include "delay.h"
#include "i2cDriver.h"

static volatile uint8_t _addr;
static volatile uint8_t _displayfunction;
static volatile uint8_t _displaycontrol;
static volatile uint8_t _displaymode;
static volatile uint8_t _cols;
static volatile uint8_t _rows;
static volatile uint8_t _charsize;
static volatile uint8_t _backlightval;

static void expanderWrite(uint8_t data);
static void pulseEnable(uint8_t _data);
static void write4bits(uint8_t value);
static void send(uint8_t value, uint8_t mode);
static void noBacklight(void);
static void backlight(void);
static void setBacklight(uint8_t new_val);
static void write(uint8_t value);
static void command(uint8_t value);

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 
//


/********** high level commands, for the user! */

/**************************************************************************
* @brief  This function initializes the i2c LCD 
*	@param	lcd_addr is the address of the lcd
* @param  lcd_cols is the number of columns in the lcd
* @param  lcd_rows is the number of rows in the lcd
* @param  charsize set to zero or one. Zero is for 5x8 default size chars
*					one is for 5x10 larger size characters
* @return none
***************************************************************************/
void lcdI2cInit(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, bool charsize)
{
		Timer1_Init();
	
		_addr = lcd_addr;
		_cols = lcd_cols;
		_rows = lcd_rows;
		_charsize = charsize;
		_backlightval = LCD_BACKLIGHT;

		i2cDriverInit();

		_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

		if (_rows > 1) {
			_displayfunction |= LCD_2LINE;
		}

		
		// for some 1 line displays you can select a 10 pixel high font
		if ((_charsize != 0) && (_rows == 1)) {
			_displayfunction |= LCD_5x10DOTS;
		}
		
		delay_ms(50);
					
		// Now we pull both RS and R/W low to begin commands
		expanderWrite(_backlightval);	// reset expander and turn backlight off (Bit 8 =1)
		//delay_ms(50);

			
		//put the LCD into 4 bit mode
		// this is according to the hitachi HD44780 datasheet
		// figure 24, pg 46

		// we start in 8bit mode, try to set 4 bit mode
		write4bits(0x03 << 4);
		delay_us(4500); // wait min 4.1ms

		// second try
		write4bits(0x03 << 4);
		delay_us(4500); // wait min 4.1ms

		// third go!
		write4bits(0x03 << 4); 
		delay_us(150);

		// finally, set to 4-bit interface
		write4bits(0x02 << 4); 

		// set # lines, font size, etc.
		command(LCD_FUNCTIONSET | _displayfunction);  

		// turn the display on with no cursor or blinking default
		_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
		lcdI2cDisplay();

		// clear it off
		lcdI2cClear();

		// Initialize to default text direction (for roman languages)
		_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

		// set the entry mode
		command(LCD_ENTRYMODESET | _displaymode);

		lcdI2cHome();

}

/**************************************************************************
* @brief  This function clears the i2c LCD
* @return none
***************************************************************************/
void lcdI2cClear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delay_us(2000);  // this command takes a long time!
}

/**************************************************************************
* @brief  This function moves cursor to home position
* @return none
***************************************************************************/
void lcdI2cHome(){
	command(LCD_RETURNHOME);  // set cursor position to zero
	delay_us(2000);  // this command takes a long time!
}

/**************************************************************************
* @brief  This function moves cursor to specific position
* @param  col is the column number
* @param  row is the row number
* @return none
***************************************************************************/
void lcdI2cSetCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _rows) {
		row = _rows-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

/**************************************************************************
* @brief  This function prints characters on the screen
* @param  str is a pointer to the string
* @return none
***************************************************************************/
void lcdI2cPrint(char * str)
{
	char * str_tmp = str;
	while(*str_tmp != NULL)
	{
		write(*str_tmp);
		str_tmp++;
	}
}

/**************************************************************************
* @brief  This function turns the display on
* @return none
***************************************************************************/ 
void lcdI2cNoDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/**************************************************************************
* @brief  This function Turn the display off
* @return none
***************************************************************************/
void lcdI2cDisplay() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}


/**************************************************************************
* @brief  Turns the underline cursor off
* @return none
***************************************************************************/ 
void lcdI2cNoCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}


/**************************************************************************
* @brief  Turns the underline cursor on
* @return none
***************************************************************************/
void lcdI2cCursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor

/**************************************************************************
* @brief  Turns the blinking cursor off
* @return none
***************************************************************************/
void lcdI2cNoBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}


/**************************************************************************
* @brief  Turns the blinking cursor on
* @return none
***************************************************************************/
void lcdI2cBlink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}


/**************************************************************************
* @brief  Turns backlight on/off
* @param	new_val set to 1 to turn backlight and set to zero to turn off
* @return none
***************************************************************************/
void setBacklight(uint8_t new_val){
	if (new_val) {
		backlight();		// turn backlight on
	} else {
		noBacklight();		// turn backlight off
	}
}


/************ low level data pushing commands **********/

void expanderWrite(uint8_t data)
{
		i2cDriverWrite(_addr,(int)(data) | _backlightval);
}


static void pulseEnable(uint8_t _data)
{

	expanderWrite(_data | En);	// En high
	delay_us(1);		// enable pulse must be >450ns
	
	expanderWrite(_data & ~En);	// En low
	delay_us(50);		// commands need > 37us to settle

}


static void write4bits(uint8_t value) 
{
	expanderWrite(value);
	pulseEnable(value);
}


static void send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
	write4bits((highnib)|mode);
	write4bits((lownib)|mode); 
}


// Turn the (optional) backlight off/on
static void noBacklight(void) {
	_backlightval=LCD_NOBACKLIGHT;
	expanderWrite(0);
}


static void backlight(void) {
	_backlightval=LCD_BACKLIGHT;
	expanderWrite(0);
}

/*********** mid level commands, for sending data/cmds */

static void write(uint8_t value) {
	send(value, Rs);
}


static void command(uint8_t value) {
	send(value, 0);
}

