/*
 ============================================================================
 Name        : DigitalOutputs.c
 Author      : Catalin Triculescu
 Version     :
 Copyright   : Hella University Workshop
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <p18f8722.h>

#include "DigitalOutputs.h"


// configuration bits
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config WDT = OFF        // Watchdog Timer (WDT disabled (control is placed on the SWDTEN bit))

byte nrLED;
byte leftButtonEv = 0;
byte rightButtonEv = 0;


/*******************************************************************************
 * Init LEDs Function
 */
void initLEDs(void)
{
	/* set all PortD bits as Output */
	TRISD = 0x0; 
	/* set state of the pins as STD_OFF */
	LATD = 0x0; 
} /* void initLEDs(void) */



/*******************************************************************************
 * Init Function
 */
void init(void)
{

    initLEDs();
    

} /* void init(void) */


/* -- write LED -- */
void setLED(byte nrLED, unsigned state)
{
    /* check what state is desired for nrLED */
    if (ON == state){
        /* if the bit should be set to ON we use the OR operation */
        LATD = LATD |   (ON << nrLED);
    } else {
        /* we set the bit with on and negates the operation */
        LATD = LATD & ~ (ON << nrLED);
    }
} /* void setLED(byte nrLED, unsigned state) */

void delay50(void) {
    uint8 i;
    for (i=0; i<50; i++) {
            _delay(50000);
        }
}


void delays(uint8 x) {
    uint8 i;
    for (i=0; i<x; i++) {
            _delay(50000);
        }
}

void sequence1(void)
{
    /* sequence start */
        delay50();
        LATD = 0x00 | (1<<7) | (1<<6) | (1<<1) | 1;
        delay50();
        LATD = ~LATD;

    /* sequence end */
}

void sequence2(void)
{
    /* sequence start */
    uint8 i = 0;
    
    LATD = 0x00;

    for (i = 0; i< 8; i++) {
        LATD = 0x01 << i;
        delays(20);
    }   
    

    //delay50();
    for (i = 0; i< 8; i++)  {
        LATD >>= 1;
        delays(20);
    }
    
    LATD = 0x00;
    delays(15);
    LATD = 0x01;
    //put your code here

    /* sequence end */
}

void sequence3(void)
{
    /* sequence start */

     uint8 i,j;
     
     j = 7;
    
    LATD = 0x00;
    uint8 VAR = 0x00;

    for (j = 7; j>=0; j--) {
        for (i = 0; i< j+1; i++) {
          LATD = VAR | 1<<i;
            delays(15);

        }
        
        LATD |= 1<<j;
        VAR = LATD;
        
        if (j <= 0)
            break;
    }
    
    
    delays(20);
    LATD = 0x00;
    
    delays(20);
    LATD = 0xff;
    
    delays(20);
    LATD = 0x00;
    
    delays(20);
    LATD = 0xff;
    
    delay50();
    

    /* sequence end */
}

/*******************************************************************************
 * Main Function
 */



void main(void)
{
    init();

    while(1) {
        sequence3();
    }
} /* void main(void) */
