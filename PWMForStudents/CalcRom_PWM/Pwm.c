#include "Pwm.h"
#include "Pwm_Private.h"
#include <xc.h>
#include "Adc_1.h"

typedef unsigned char byte;

enum {
    albastru = RC2_0,
    verde = RC1_0,
    rosu = RG0_0
};


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
        
    TRISA = TRISA | (1<<0); /* only RA0 as input */

    ADCON0 = 0b00000001;
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
        case RC2_0:  // albastru
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

void delay(int j) {
    uint8 i;
    for (i=0; i<j; i++) {
            _delay(500);
        }
}

void LedControl()
{ int i;
    //Add your code here
    for(i=0; i<=255; i++ )
    {setDuty(albastru, i);
    delay(20);
    }

    delay(20);

    for(i=255; i>=0; i-- )
    { setDuty(albastru, i);  
      delay(20);
    }
    
     delay(20);

     for(i=0; i<=255; i++ )
    {setDuty(verde, i);
    delay(20);
    }

    delay(20);

    for(i=255; i>=0; i-- )
    { setDuty(verde, i);  
      delay(20);
    }
    
     delay(20);
     
     for(i=0; i<=255; i++ )
    {setDuty(rosu, i);
    delay(20);
    }

    delay(20);

    for(i=255; i>=0; i-- )
    { setDuty(rosu, i);  
      delay(20);
    }
    
     //delay(20);
}

void LedControl2()
{
    uint8 potiValue = 0;

    /* start conversion*/
  /*  byte potiValue = 0;
    ADCON0bits.GO_DONE = 0x01;
    
   while( ADCON0bits.GO_DONE){}
    
    potiValue = ADRESH; */
    
    
    Adc_GetMess(&potiValue);
    setDuty(verde, potiValue); 
}

void LedControl3()
{
     uint8 potiValue = 0;
     
     
     Adc_GetMess(&potiValue);
     
     if( potiValue <= 80 )
     {setDuty(verde, 0); 
      setDuty(rosu, 255);
      setDuty(albastru, 0);
     }
     else if ( potiValue <= 160)
     {setDuty(verde, 255); 
      setDuty(rosu, 0);
      setDuty(albastru, 0);
     }
     else
     {
      setDuty(verde, 0); 
      setDuty(rosu, 0);
      setDuty(albastru, 255);
     }
 
     
     
}

void LedControl4()
{
     uint8 potiValue = 0;
     
     
     Adc_GetMess(&potiValue);
     
      if( potiValue <= 80 )
     {setDuty(verde, 255); 
      setDuty(rosu, 255);
      setDuty(albastru, 0);
     }
     else if ( potiValue <= 160)
     {setDuty(verde, 255); 
      setDuty(rosu, 0);
      setDuty(albastru, 255);
     }
     else
     {
      setDuty(verde, 0); 
      setDuty(rosu, 255);
      setDuty(albastru, 255);
     }
}

void LedControl5()
{   int i;
     uint8 potiValue = 0;
    
     
     Adc_GetMess(&potiValue);
     
     if( potiValue <= 85){ 
             setDuty(verde, 0);
           for(i=255; i>=0; i-- )
            {
               setDuty(rosu, i);
               delay(20);
            }
           
          setDuty(albastru, 0);
          delay(50);
          for(i=0; i<=255; i++ )
            {
              setDuty(verde, i);
              delay(20);
            }
         
      delay(50);
          
        
     }
     
     else if ( potiValue <= 170)
     {
         setDuty(rosu, 0);
         for(i=255; i>=0; i-- )
            {setDuty(verde, i);
              delay(20);
            }
          
         setDuty(verde, 0);
          for(i=0; i<=255; i++ )
            {setDuty(albastru, i);
              delay(20);
            }
          
         setDuty(albastru, 0);
     }
     else
     {
      
         setDuty(verde, 0);
         
          for(i=0; i<=255; i++ )
            {setDuty(rosu, i);
              delay(20);
            }
         
          setDuty(rosu, 0);
          
          for(i=255; i>=0; i-- )
            {setDuty(albastru, i);
              delay(20);
            }
         
     }
}