#ifndef ADC_SG_H
#define ADC_SG_H




void AdcSgInit(void);
uint32_t AdcSgRead(void);

void ADCInit(void);
void ADCGetValue(uint32_t * );

void ADCTimerTriggeredInit(void);
uint32_t ADCGetSensor1(void);
uint32_t ADCGetSensor2(void);
uint32_t ADCGetSensor3(void);



#endif
