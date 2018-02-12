#ifndef __delay_h
#define __delay_h

void SysTickInt(void);
void delay_us(uint32_t time);
void Timer1_Init(void);
void delay_ms(uint32_t time);

#endif
