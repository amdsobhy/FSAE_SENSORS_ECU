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

#ifndef LCD_I2C_H_
#define LCD_I2C_H_


// commands
#define LCD_CLEARDISPLAY 					0x01
#define LCD_RETURNHOME 						0x02
#define LCD_ENTRYMODESET 					0x04
#define LCD_DISPLAYCONTROL 				0x08
#define LCD_CURSORSHIFT 					0x10
#define LCD_FUNCTIONSET 					0x20
#define LCD_SETCGRAMADDR 					0x40
#define LCD_SETDDRAMADDR 					0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 						0x00
#define LCD_ENTRYLEFT 						0x02
#define LCD_ENTRYSHIFTINCREMENT 	0x01
#define LCD_ENTRYSHIFTDECREMENT 	0x00

// flags for display on/off control
#define LCD_DISPLAYON 						0x04
#define LCD_DISPLAYOFF 						0x00
#define LCD_CURSORON 							0x02
#define LCD_CURSOROFF 						0x00
#define LCD_BLINKON 							0x01
#define LCD_BLINKOFF 							0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 					0x08
#define LCD_CURSORMOVE 						0x00
#define LCD_MOVERIGHT 						0x04
#define LCD_MOVELEFT 							0x00

// flags for function set
#define LCD_8BITMODE 							0x10
#define LCD_4BITMODE 							0x00
#define LCD_2LINE 								0x08
#define LCD_1LINE 								0x00
#define LCD_5x10DOTS 							0x04
#define LCD_5x8DOTS 							0x00

// flags for backlight control
#define LCD_BACKLIGHT 						0x08
#define LCD_NOBACKLIGHT 					0x00

#define En 												(1<<2)  // Enable bit
#define Rw 												(1<<1)  // Read/Write bit
#define Rs 												(1<<0)  // Register select bit

/********** high level commands, for the user! */
void lcdI2cInit(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, bool charsize);
void lcdI2cClear(void);
void lcdI2cHome(void);
void lcdI2cSetCursor(uint8_t col, uint8_t row);
// Turn the display on/off (quickly)
void lcdI2cNoDisplay(void);
void lcdI2cDisplay(void); 
// Turns the underline cursor on/off
void lcdI2cNoCursor(void);
void lcdI2cCursor(void); 
// Turn on and off the blinking cursor
void lcdI2cNoBlink(void);
void lcdI2cBlink(void);
void lcdI2cPrint(char * str);

#endif
