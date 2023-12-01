#ifndef MAIN_H_
#define MAIN_H_
#include "stdint.h"

void controlRGB(int combi, int time, int blink);
void timerConfig(uint32_t interval);
void msDelay(uint32_t ms);
void ADCInit(void);
void UARTInit(void);
void TX(char text[]);

#endif
