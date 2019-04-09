#include "Pwm.h"
#include "Pwm_Private.h"
#include <xc.h>
#include "Adc_1.h"


void PwmInit()
{
	TRISCbits.RC2=1;
        TRISCbits.RC1=1;
        TRISGbits.RG0=1;

	CCP1CON=0b00001100;
        CCP2CON=0b00001100;
        CCP3CON=0b00001100;

	CCPR1L=0x00;
        CCPR2L=0x00;
        CCPR3L=0x00;

	T2CON=0b00000101;
	while (!PIR1bits.TMR2IF);

	TRISCbits.RC2=0;
        TRISCbits.RC1=0;
        TRISGbits.RG0=0;
}

void InteruptInit()
{
    INTCONbits.GIE=1;
    INTCONbits.INT0IE=1;
    INTCONbits.INT0IF=0;
    INTCON2bits.INTEDG0=1;
}

void setDuty(uint8 chanId,uint8 duty)
{
    switch(chanId)
    {
        case RC2_0:
        {
            CCPR1L = duty;
            break;
        }
        case RC1_0:
        {
            CCPR2L = duty;
            break;
        }
        case RG0_0:
        {
            CCPR3L = duty;
            break;
        }
    }
}

void setPeriod(uint8 period)
{
	PR2 = period;
}

void LedAction()
{
}

void LedControl()
{
    //Add your code here
}