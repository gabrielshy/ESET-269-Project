#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "msp432p401r.h"

void timerConfig(uint32_t interval){
	TIMER32_1->LOAD = interval - 1;
	TIMER32_1->CONTROL = TIMER32_CONTROL_ENABLE | TIMER32_CONTROL_IE | TIMER32_CONTROL_ONESHOT;
}

void msDelay(uint32_t ms) {
	TIMER32_1->LOAD = ms * 3000000 - 1;
  TIMER32_1->CONTROL = 0x42;
	P2->OUT |= 0x00;
	TIMER32_1->CONTROL |= 0x80;
	while((TIMER32_1->RIS & 1)!=1){
		
	}
	TIMER32_1->INTCLR &=~0x01;
	P2->OUT &= ~0x00;
	TIMER32_1->CONTROL &=~0x80;
}
	
void controlRGB (int combination, int time, int blink) {
	int i;
	P2->SEL1 &=	~combination;
	P2->SEL0 &= ~combination;
	P2->DIR |= combination;
	P2->OUT &= ~combination;

	for (i = 0; i < blink * 2; ++i) {
		P2->OUT ^= combination;
		msDelay(time);
	}
	return;
}	
void ADCInit(void){
	//Ref_A settings
	REF_A ->CTL0 &= ~0x8; //enable temp sensor
	REF_A ->CTL0 |= 0x30; //set ref voltage
	REF_A ->CTL0 &= ~0x01; //enable ref voltage
	//do ADC stuff
	ADC14 ->CTL0 |= 0x10; //turn on the ADC
	ADC14 ->CTL0 &= ~0x02; //disable ADC
	ADC14 ->CTL0 |=0x4180700; //no prescale, mclk, 192 SHT
	ADC14 ->CTL1 &= ~0x1F0000; //configure memory register 0
	ADC14 ->CTL1 |= 0x800000; //route temp sense
	ADC14 ->MCTL[0] |= 0x100; //vref pos int buffer
	ADC14 ->MCTL[0] |= 22; //channel 22
	ADC14 ->CTL0 |=0x02; //enable adc
	return;
}
float tempRead(void){
	float temp; //temperature variable
	uint32_t cal30 = TLV ->ADC14_REF2P5V_TS30C; //calibration constant
	uint32_t cal85 = TLV ->ADC14_REF2P5V_TS85C; //calibration constant
	float calDiff = cal85 - cal30; //calibration difference
	ADC14 ->CTL0 |= 0x01; //start conversion
	while((ADC14 ->IFGR0) ==0)
	{
	//wait for conversion
	}
	temp = ADC14 ->MEM[0]; //assign ADC value
	temp = (temp - cal30) * 55; //math for temperature per manual
	temp = (temp/calDiff) + 30; //math for temperature per manual
	return temp; //return temperature in degrees C
}

void UARTInit(void){
	EUSCI_A0 ->CTLW0 |= 1;
	EUSCI_A0 ->MCTLW = 0;
	EUSCI_A0 ->CTLW0 |= 0x80;
	EUSCI_A0 ->BRW = 0x34;
	EUSCI_A0 ->CTLW0 &= ~0x01;
	P1->SEL0 |= 0x0C;
	P1->SEL1 &= ~0x0C;
	return;
}

void TX(char text[]){
	int i =0;
	while(text[i] != '\0')
	{
		EUSCI_A0 ->TXBUF = text[i];
		while((EUSCI_A0 ->IFG & 0x02) == 0)
	{
		//wait until character sent
	}
	i++;
	}
	return;
}

int RX(void){	
	int i = 0;
	char command[2];
	char x;
	while(1)
	{
		if((EUSCI_A0 ->IFG & 0x01) !=0){
			command[i] = EUSCI_A0->RXBUF;
			EUSCI_A0 ->TXBUF = command[i]; //echo
			while((EUSCI_A0 ->IFG & 0x02)==0); //wait
			if(command[i] == '\r') {
				command[i] = '\0';
				break;
			} else {
			i++;
			}
		}
	}
	x = atoi(command);
	TX("\n\r");
	return x;
}

void sysTick(void){
	SysTick->LOAD = 3000000-1;
	SysTick->CTRL |=0x4;
	SysTick->CTRL |=0x1;
	while((SysTick->CTRL & 0x10000)==0){
	}
	SysTick->CTRL &= ~0x01;
}
int main(void){
	int option, rgb, time, blinks, temp, i;
	char menu[80], comboString[32], timeString[20], blinkString[24], loadString[20], tempString[43];
	UARTInit();
	ADCInit();
	
	do {
		sprintf(menu,"\n\rMSP432 Menu\n\n\r1. RGB Control\n\r2. Digital Input\n\r3. Temperature Reading\n\r");
		TX(menu);
		option = RX();
		switch(option){
			case 1:
				sprintf(comboString,"Enter Combination of RGB (1-7):");
				TX(comboString);
				rgb = RX();
				sprintf(timeString,"Enter Toggle Time:");
				TX(timeString);
				time = RX();
				sprintf(blinkString,"Enter Number of Blinks:");
				TX(blinkString);
				blinks = RX();
				sprintf(loadString,"Blinking LED...\n");
				TX(loadString);
				controlRGB(rgb, time, blinks);
				TX("Done\n\r");
				break;
			
			case 2:
				P1->DIR &= ~0x12;
				P1->REN |= 0x12;
				P1->OUT |= 0x12;
				int x = 0;
				while(x == 0){
					if((P1->IN & 0x02)==0 && (P1->IN & 0x10)!=0){
						TX("Button 1 Pressed\n\r");
						x = 1;
					} else if((P1->IN & 0x10)==0 && (P1->IN & 0x02)!=0){
						TX("Button 2 Pressed\n\r");
						x = 1;
					} else if((P1->IN & 0x10)==0 && (P1->IN & 0x02)==0){
						TX("Button 1 and Button 2 Pressed\n\r");
						x = 1;
					} else {
						TX("No Button Pressed\n\r");
						x = 1;
					}
				}
				break;
				
			case 3:
				sprintf(tempString,"Enter Number of Temperature Reading (1-5):");
				TX(tempString);
				temp = RX();
				float F, C;
				char tempText[100];
				if (temp > 5){
					temp = 5;
				} else if (temp < 1){
					temp = 1;
				}
				for(int i = 0; i < temp; ++i){
					C = tempRead();
					F = C * (9.0/5.0) + 32.0;
					sprintf(tempText, "Reading %d: %.2f C & %.2f F\r\n", i + 1, C, F);
					TX(tempText);
					sysTick();
				}
				break;
			
			default:
				TX("Invalid input\n\n\r");
				break;
		}
	} while (1);
}
