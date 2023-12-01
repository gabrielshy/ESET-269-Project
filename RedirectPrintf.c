/* Do not modify any code. This is to redirect
printf to the console window with Launchpad*/

#include "msp.h"
#include <stdio.h>

void OutputInit(void) {
    EUSCI_A0->CTLW0 |= 1;       
    EUSCI_A0->MCTLW = 0;        
    EUSCI_A0->CTLW0 = 0x0081;   
    EUSCI_A0->BRW = 26;         
    P1->SEL0 |= 0x0C;           
    P1->SEL1 &= ~0x0C;
    EUSCI_A0->CTLW0 &= ~1;      
}

/* Receive from PC */
unsigned char ReadInput(void) {
    char c;

    while(!(EUSCI_A0->IFG & 0x01)) ;
    c = EUSCI_A0->RXBUF;
    return c;
}

/* Send to PC */
int SendOutput(unsigned char c) {
    while(!(EUSCI_A0->IFG&0x02)) ;
    EUSCI_A0->TXBUF = c;
    return c;
}

/* The code below is the interface to the C standard I/O library.*/
//struct __FILE { int handle; };
FILE __stdin  = {0};
FILE __stdout = {1};
FILE __stderr = {2};

int fgetc(FILE *f) {
    int c;

    c = ReadInput();      

    if (c == '\r') {    
        SendOutput(c);    
        c = '\n';
    }
    SendOutput(c);         
    return c;
}

int fputc(int c, FILE *f) {
    return SendOutput(c);  
}

