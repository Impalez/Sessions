/* 
 * File:   StateMachine.c
 * Author: Anca
 *
 * Created on May 19, 2014, 7:15 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <p18f8722.h>
#include <spi.h>
#include <delays.h>

#include "StateMachine.h"
#include "LCD.h"

// configuration bits
#pragma config OSC = HS       // Oscillator Selection bits (HS oscillator)
#pragma config WDT = OFF      // Watchdog Timer (WDT disabled (control is placed on the SWDTEN bit))

//                           0123456789012345
#define LCD_STATE_ONE_1     "     STATE 1    "
#define LCD_STATE_ONE_2     "PRESS RB0 BUTTON"
#define LCD_STATE_TWO_1     "     STATE 2    "
#define LCD_STATE_TWO_2     "     WAIT 10s   "
#define LCD_STATE_THREE_1   "     STATE 3    "
#define LCD_STATE_THREE_2   "PRESS RB0 BUTTON"

const char LcdLines[STATE_MAX][2][18] =
{
    {LCD_STATE_ONE_1, LCD_STATE_ONE_2},
    {LCD_STATE_TWO_1,LCD_STATE_TWO_2},
    {LCD_STATE_THREE_1,LCD_STATE_THREE_2}
};

typedef unsigned char byte;

#define ON          1
#define OFF         0
/*******************************************************************************
 * State machine example
 */
state_e state;
byte RD5Led;
byte RD8Led;
byte RD7Led;

            char s[16];

byte SetTemperature;
byte InputTemperature;

char State_Text[16];

byte leftButtonEv = 0;

state_clima State;

void UpdateLCDText(void) {
         
       // sprintf(s, "Input t: %d   ", InputTemperature);
        LcdClear();
 
        LcdGoTo(0); /* first Line */
        LcdWriteString(State_Text);
}


void Adc_GetMess(byte *Value)
{
    ADCON0 = 0b00000001;
    ADCON1 = 0b00001110;
    ADCON2 = 0b00111010;
    ADCON0bits.GO = 1;

    while(ADCON0bits.GO)
    {/*Do nothing wait for conversion*/
    }

    *Value = ADRESH;
}

/*******************************************************************************
 * Set RD5 LED Function
 */
void setRD5Led(unsigned state)
{
    RD5Led = state; /* store new state */

} /* void setRD5Led(unsigned state) */

 
void setRD7Led(unsigned state)
{
    RD7Led = state; /* store new state */

} 

/*******************************************************************************
 * Set RD8 LED Function
 */
void setRD8Led(unsigned state)
{
    RD8Led = state; /* store new state */

} /* void setRD8Led(unsigned state) */

/*******************************************************************************
 * Check On/Off Left Button Function
 */
byte getOnOffLeftButton(void)
{
    return leftButtonEv;
} /* unsigned getOnOffLeftButton(void) */

/*******************************************************************************
 * Set LCD Function
 */
void setLcd(void)
{
    LcdClear();
    LcdGoTo(0); /* first Line */
    LcdWriteString(LcdLines[state][0]);
    LcdGoTo(0x40); /* second Line */
    LcdWriteString(LcdLines[state][1]);
} /* void setLcd(void) */

/*******************************************************************************
 * Check Inputs Function
 */
void checkInput(void)
{
    byte leftButton = 0;
    static byte leftButton_old = 0;

/* RB0 - check left push button event */
    leftButton = PORTBbits.RB0;
    if (   (leftButton == 0) /* push button pressed */
        && (leftButton != leftButton_old) /* not pressed before */
       )
    {
        leftButtonEv = 1;
    }
    leftButton_old = leftButton;

} /* void checkInput(void) */

/*******************************************************************************
 * Delay of 1s Function
 */
void Delay1s (int x)
{
    int i;
    for (i=0; i<x; i++)
    {
        /* Fosc = 10MHz
         * 10000 x 250 x 4 x (1/10e6) = 1s
         */
        Delay10KTCYx(250);
    }
}/* void Delay1s(int x) */


void stateMachine_clima(void) {
    static byte DisplayState;
    
    switch (State) {
        
        case STATE_OFF:
            sprintf(State_Text, "STATE_OFF  ");
            if (!DisplayState)
                UpdateLCDText(), DisplayState = 1;
                
            setRD5Led(OFF);
            setRD7Led(OFF);
            setRD8Led(OFF);
            
 
                
            if (getOnOffLeftButton())
                DisplayState = 0, State = STATE_ON_VENT;
            
            break;
            
        case STATE_ON_VENT:
            
            setRD5Led(OFF);
            setRD7Led(OFF);
            setRD8Led(ON);
            
            sprintf(State_Text, "STATE_VENT  ");
            if (!DisplayState)
                UpdateLCDText(), DisplayState = 1;
            
            
             if (getOnOffLeftButton()) 
                DisplayState=0, State = STATE_OFF;
             else 
                 if (InputTemperature > SetTemperature)
                    DisplayState = 0, State = STATE_ON_COOL;
                 else if (InputTemperature < SetTemperature)
                   DisplayState= 0, State = STATE_ON_HEAT;
            
           
            break;
        
        case STATE_ON_COOL:
            sprintf(State_Text, "STATE_COOL  ");
            
             if (!DisplayState)
                UpdateLCDText(), DisplayState = 1;
            
            setRD7Led(ON);
            setRD5Led(OFF);
            setRD8Led(ON);
            
            
            if (getOnOffLeftButton()) 
                 DisplayState = 0, State = STATE_OFF;
            else {
               
                if (InputTemperature < SetTemperature)
                  DisplayState = 0, State = STATE_ON_HEAT;
                else if (InputTemperature == SetTemperature)
                  DisplayState = 0,  State = STATE_ON_VENT;
             //   else InputTemperature--;
            }

            break;
            
            
        case STATE_ON_HEAT:
            sprintf(State_Text, "STATE_HEAT  ");
            if (!DisplayState)
                UpdateLCDText(), DisplayState = 1;
            
            setRD5Led(ON);
            setRD7Led(OFF);
            setRD8Led(ON);
            
            if (getOnOffLeftButton()) 
                 State = STATE_OFF;
            else {

                if (InputTemperature > SetTemperature)
                    DisplayState = 0, State = STATE_ON_COOL;
                else if (InputTemperature == SetTemperature)
                    DisplayState = 0, State = STATE_ON_VENT;
               // else InputTemperature++;
              
            }
            
            break;
    }

}



/*******************************************************************************
 * State Machine Function
 */
void stateMachine(void)
{
    byte speed;

    switch (state)
    {
        case STATE_ONE:
        {
            /* Left ON/OFF button pressed */
            if (getOnOffLeftButton()) /* trigger transition to STATE2 */
            {
                state = STATE_TWO; /* enter STATE2 */
                setLcd(); /* set LCD according to STATE2 */
                setRD5Led(ON); /* set RD5 LED ON */
                //setRD8Led(OFF); /* set RD8 LED OFF */
            }
            else
            {
                /* remain in STATE1 */
            }
            break;
        }
        case STATE_TWO:
        {
            /* enter STATE3 after 10s delay */
            Delay1s(10);
            state = STATE_THREE; /* enter STATE3 */
            setLcd(); /* set LCD according to STATE3 state */
            setRD8Led(ON); /* set RD8 LED ON */
            break;
        }
        case STATE_THREE:
        {
            /* Left ON/OFF button pressed */
            if (getOnOffLeftButton()) /* trigger transition back to STATE1 */
            {
                state = STATE_ONE; /* return to STATE1 */
                setLcd(); /* set LCD according to STATE1 */
                setRD5Led(OFF); /* set RD5 LED OFF */
                setRD8Led(OFF); /* set RD8 LED OFF */
            }
            else
            {
                /* remain in state 3 */
            }
            break;
        }
        default:
        {
            break;
        }
    }

 } /* void stateMachine(void) */

/*******************************************************************************
 * Update outputs Function
 */
void updateOutputs(void)
{
    /* put RD5 led on the output */
    PORTDbits.RD4 = RD5Led;
    PORTDbits.RD6 = RD7Led;

    /* put RD8 LED on the output */
    PORTDbits.RD7= RD8Led;
   
} /* void updateOutputs(void) */


/*******************************************************************************
 * Init Buttons Function
 */
void initButtons(void)
{
    /* RB0 left push button */
    TRISB = TRISB | (1<<0); /* only RB0 as input */
    
} /* void initButtons(void) */




/*******************************************************************************
 * Init Function
 */
void init(void)
{
    /* init buttons */
    initButtons();
 
    TRISD=0;
    MEMCONbits.EBDIS=1;
    PORTD=0;
    
  /* init LCD */
    LcdInit();
    
    SetTemperature = 50;
    
    

  /* transition from "Power OFF" to "STATE1" */

//   state = STATE_ONE;
  //  setLcd(); /* set LCD according to STATE1 */

 } /* void init(void) */

/*******************************************************************************
 * Main Function
 */
void main(void)
{
    init();

    while(1)
    {
        Adc_GetMess(&InputTemperature);
        checkInput();
        stateMachine_clima();
        updateOutputs();
        
        sprintf(s, "Input t: %d   ", InputTemperature);
        LcdGoTo(0x40); /* second Line */
        LcdWriteString(s);

        /* clear event from left button */
        leftButtonEv = 0;
    }
} /* void main(void) */
