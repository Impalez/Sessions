/* 
 * File:   clima.c
 * Author: Dragos
 *
 * Created on February 2, 2015, 11:39 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <p18f8722.h>

#include "clima.h"
#include "lcd.h"
#include "uart.h"

// configuration bits
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config WDT = OFF        // Watchdog Timer (WDT disabled (control is placed on the SWDTEN bit))




#define DBG_MSG 1
#if (DBG_MSG == 1)
#define DBG(x)  UART_puts((char *)x)    // debug messages on UART
#else
#define DBG(x)                  // debug messages are lost
#endif

//                           0123456789012345
#define LCD_STATE_OFF_1     "  Clima is OFF  "
#define LCD_STATE_OFF_2     "----------------"
#define LCD_STATE_ON_COOL_1 "Te:+  C Ti:+  C "
#define LCD_STATE_ON_COOL_2 "Rece       <  C>"
#define LCD_STATE_ON_HEAT_1 "Te:+  C Ti:+  C "
#define LCD_STATE_ON_HEAT_2 "Cald       <  C>"
#define LCD_STATE_ON_VENT_1 "Te:+  C Ti:+  C "
#define LCD_STATE_ON_VENT_2 "Vent          C "



const char LcdLines[STATE_MAX][2][18] =
{
    {LCD_STATE_OFF_1, LCD_STATE_OFF_2},
    {LCD_STATE_ON_COOL_1,LCD_STATE_ON_COOL_2},
    {LCD_STATE_ON_HEAT_1,LCD_STATE_ON_HEAT_2},
    {LCD_STATE_ON_VENT_1,LCD_STATE_ON_VENT_2}
};

typedef unsigned char byte;
#define ON          1
#define OFF         0
#define TEMP_STEP   1
#define FAN_STEPS   6
#define TEMP_MIN    21

#define TIMER_CYCLE         (100)   // timer cyclic events  (ms)
#define INPUT_DEBOUNCE_TIME (3000)  // time (ms)
#define INPUT_DEBOUNCE_CNT  (INPUT_DEBOUNCE_TIME/TIMER_CYCLE) //   time(ms)/ cycle100(ms)

#define TEMP_SENS_MPC_OFFSET    (400)   // output voltage @ 0*C
#define TEMP_SENS_LM_OFFSET     (0)     // output voltage @ 0*C
#define TEMP_SENS_MPC_RES       (19)    // output voltage / *C
#define TEMP_SENS_LM_RES        (10)    // output voltage / *C


#define TRIS_OUT                0
#define TRIS_HEAT_ELEMENT       (TRISDbits.TRISD3)
#define TRIS_HEAT_VENT_FAN      (TRISDbits.TRISD4)
#define TRIS_COOL_FAN           (TRISDbits.TRISD5)

#define PIN_OFF                 0
#define PIN_ON                  1
#define PIN_HEAT_ELEMENT        (PORTDbits.RD3)
#define PIN_FAN_COOL            (PORTDbits.RD4)
#define PIN_FAN_HEAT_VENT       (PORTDbits.RD5)



void setSpeedFanCool(byte speed);
void setSpeedFanHeatVent(byte speed);
void setLevelHeat(byte level);

unsigned int ADCRead(unsigned char ch);
void checkInputs(void);
byte getOnOffButton(void);
void test(void);
void setLcd(void);
void updateLcd(void);
void checkInputs(void);
void stateMachine(void);
void initButtons(void);
void initAdc(void);
void initPwm(void);
void init(void);
void main(void);

/*******************************************************************************
 * Clima state
 */
state_e climaState;         /* state of the clima: OFF, VENT, COOL, HEAT */

byte fanSpeedCool;          /* speed for cool fan */
byte fanSpeedHeatVent;      /* speed for heat/ventilation fan */
byte levelHeat;             /* heating level */

byte heatElement;           /* LED */
byte coolElement;           /* LED */
byte standbyLed;            /* LED */
byte lcdBacklightLed;       /* LED */

byte leftButtonEv = 0;      /* event generated when the transition from NOT_PRESSED -> PRESSED is detected on left button */
//byte rightButtonEv = 0;
byte setTemp = 0;           /* desired temperature */
unsigned int inTemp = 0;            /* interior temperature */
unsigned int outTemp = 0;   /* outside temperature */

unsigned char tick = 0;
unsigned char ev = 0;
unsigned char cnt = 0;


char msg[20] = {0};         /* used to format print messages */

unsigned char inDeb = 0;    /* debounce counter for temperature sensor */




/*******************************************************************************
 * Set Standby LED Function
 */
void setStandbyLed(unsigned state)
{
    standbyLed = state; /* store new state */
} /* void setStandbyLed(unsigned state) */



/*******************************************************************************
 * Set LCD backlight LED Function
 */
void setLcdBacklightLed(unsigned state)
{
    lcdBacklightLed = state; /* store new state */
} /* void setLcdBacklightLed(unsigned state) */



/*******************************************************************************
 * Set Heating Element Function
 */
void setHeatElement(unsigned state)
{
    heatElement = state; /* store new state */
} /* void setHeatElement(unsigned state) */



/*******************************************************************************
 * Set Cooling Element Function
 */
void setCoolElement(unsigned state)
{
    coolElement = state; /* store new state */
} /* void setCoolElement(unsigned state) */



/*******************************************************************************
 * Set FAN Speed Function for cool fan
 */
void setSpeedFanCool(byte speed)
{
    if (speed)
        speed += 3;
    fanSpeedCool = speed; /* store new speed */
} /* void setSpeedFanCool() */


/*******************************************************************************
 * Set FAN Speed Function for heat/vent fan
 */
void setSpeedFanHeatVent(byte speed)
{
    if (speed)
        speed += 3;
    fanSpeedHeatVent = speed; /* store new speed */
} /* void setSpeedFanHeatVent() */


/*******************************************************************************
 * Set Heat level Function
 */
void setLevelHeat(byte level)
{
    if (level)
        level += 3;
    levelHeat = level; /* store new level */
} /* void setLevelHeat() */



/*******************************************************************************
 * Check Inputs Function
 */
byte getOnOffButton(void)
{
    return leftButtonEv;
} /* unsigned getOnOffButton(void) */


//byte getRightButton(void)
//{
//    return rightButtonEv;
//} /* unsigned getOnOffButton(void) */



/*******************************************************************************
 * Check Inputs Function
 */
void setLcd(void)
{
    LcdClear();
    LcdGoTo(0); /* first Line */
    LcdWriteString(LcdLines[climaState][0]);
    LcdGoTo(0x40); /* second Line */
    LcdWriteString(LcdLines[climaState][1]);
} /* void setLcd() */



/*******************************************************************************
 * Check Inputs Function
 */
void updateLcd(void)
{
    //TODO state_e climaState_old;
    byte fanSpeed = 0;

 /* display set temperature */
    if (climaState != STATE_OFF)
    {
        /* out temp */
        sprintf(msg, "%02d", outTemp);

        LcdGoTo(0x00+4);
        LcdWriteString(msg);

        /* in temp */
        sprintf(msg, "%02d", inTemp);
        LcdGoTo(0x00+12);
        LcdWriteString(msg);

        /* mode */
        LcdGoTo(0x40);
        if (climaState == STATE_ON_COOL)
            LcdWriteString("Rece");
        else if (climaState == STATE_ON_HEAT)
            LcdWriteString("Cald");
        else if (climaState == STATE_ON_VENT)
            LcdWriteString("Vent");

        /* fan speed */
        LcdGoTo(0x40+5);
        if (climaState == STATE_ON_COOL)
            fanSpeed = fanSpeedCool;
        else
            fanSpeed = fanSpeedHeatVent;

        if (fanSpeed == 4)
            LcdWriteString("|....");
        else if (fanSpeed == 5)
            LcdWriteString("||...");
        else if (fanSpeed == 6)
            LcdWriteString("|||..");
        else if (fanSpeed == 7)
            LcdWriteString("||||.");
        else
            LcdWriteString("|||||");

        /* set temp */
        sprintf(msg, "%d", setTemp);
        LcdGoTo(0x40+12);
        LcdWriteString(msg);

        if (setTemp == TEMP_MIN)
        {
            LcdGoTo(0x40+11);
            LcdWriteString(" ");
            LcdGoTo(0x40+15);
            LcdWriteString(">");
        }
        else if (setTemp == TEMP_MIN+15)
        {
            LcdGoTo(0x40+11); /* second Line */
            LcdWriteString("<");
            LcdGoTo(0x40+15); /* second Line */
            LcdWriteString(" ");
        }
        else
        {
            LcdGoTo(0x40+11); /* second Line */
            LcdWriteString("<");
            LcdGoTo(0x40+15); /* second Line */
            LcdWriteString(">");
        }
    }
} /* void updateLcd(state_e state) */



/*******************************************************************************
 * Check Inputs Function
 */
void checkInputs(void)
{
    unsigned int adcVal = 0;
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


    /* read ADC AN0 - on board potentiometer */
    /* ADC 10 bit resolution
     * AD values     : 0..1023  (1024 values)
     * Set temp steps: 16..31 (16 steps)
     * k = 1024/16 = 64
     * setTemp = adcVal/64 + offset
    */
    adcVal = ADCRead(0);
    setTemp = adcVal/64 + TEMP_MIN;

    /* debounce temperature measurement: counter elapsed */
    if (inDeb == 0)
    {
        /* read ADC AN1 - on board temperature sensor */
        /* MCP9701:
         * Resolution: 19mV/*C
         * OFFSET: U @ 0*C = 400mV
         * T = (U-OFFSET)/resolution
         *
         * U = ADC*5000/1023 ~= ADC*5;
         * T = (U-OFFSET)/RESOLUTION ~= (ADC*5 - OFFSET)/RESOLUTION
         */
        adcVal = ADCRead(1);
        outTemp = (adcVal*5 - TEMP_SENS_MPC_OFFSET)/TEMP_SENS_MPC_RES;
        DBG("-> Temperature out:");
        sprintf(msg, "%d", outTemp);
        DBG(msg);
        DBG("\n\r");

        /* read ADC AN3 - inside temperature sensor */
        /* LM35: 
         * Resolution: 10mV/*C
         * Offset: U @ 0*C = 0mV
         * T = (U-OFFSET)/resolution
         *
         * U = ADC*5000/1023
         * T = (U-OFFSET)/RESOLUTION ~= (ADC*5 - OFFSET)/RESOLUTION
         */
        /*
        adcVal = ADCRead(3);
        outTemp = (adcVal*5 - TEMP_SENS_MPC_OFFSET)/TEMP_SENS_MPC_RES;
        DBG("-> Temperature out:");
        sprintf(msg, "%d", outTemp);
        DBG(msg);
        DBG("\n\r");

		 /* TO DO*/
        
        inDeb = INPUT_DEBOUNCE_CNT;
    }
    else
    {
        inDeb--;
    }

} /* void checkInputs(void) */


/*******************************************************************************
 * Update outputs Function
 */
void updateOutputs(void)
{
    /* put standby LED on the output */
    PORTDbits.RD7 = standbyLed;

    /* put LCD backlight on the output */
    PORTDbits.RD6 = lcdBacklightLed;

    /* put cooling element state on the output */
    PORTDbits.RD1 = coolElement;

    /* put heating element on the output */
    PORTDbits.RD0 = heatElement;
} /* void updateOutputs(void) */

/*******************************************************************************
 * State Machine Function
 */
void stateMachine(void)
{
    byte err;

    switch (climaState)
    {
        case STATE_OFF: {      
            setLcd();
            //updateLcd();
            
            setStandbyLed(ON);
            setLcdBacklightLed(ON);
            setHeatElement(OFF);
            setCoolElement(OFF);
            updateOutputs();
            
            setSpeedFanCool(0);
            setSpeedFanHeatVent(0);

            if ( !PORTBbits.RB0)//getOnOffButton())
                climaState = STATE_ON_VENT; 
            
            break;
        }
        case STATE_ON_COOL:
        {
            /* TODO*/
            setHeatElement(OFF);
            setCoolElement(ON);
            updateOutputs();
            
            float value = (outTemp - setTemp) / setTemp * 100; // 0 - 100 percent value
            
            if (value < 0)              climaState = STATE_ON_HEAT;
            else if (value == 0)        climaState = STATE_ON_VENT;
            if (value <= 25)            setSpeedFanCool(25);
            else if (value <= 50)       setSpeedFanCool(50); 
            else if (value <= 75)       setSpeedFanCool(75);
            else if (value <= 100);     setSpeedFanCool(100);
            
            break;
        }
        case STATE_ON_HEAT:
        {
            /* TODO*/
            break;
        }
        case STATE_ON_VENT:
        {
            /* TODO*/
            setLcdBacklightLed(OFF);
            setStandbyLed(ON);
            setHeatElement(OFF);
            setCoolElement(OFF);
            updateOutputs();
            
            if (outTemp > setTemp)                  climaState = STATE_ON_COOL;
            else if (outTemp < setTemp)             climaState = STATE_ON_HEAT;
                     
            
            break;

        }
        default:
        {
            break;
        }
    }
} /* void stateMachine(void) */






/*******************************************************************************
 * Init Buttons Function
 */
void initButtons(void)
{
    /* RB0 left push button */
    TRISB0 = 1; /* only RB0 as input */
} /* void initButtons(void) */



/*******************************************************************************
 * Init Buttons Function
 */
void initAdc(void)
{
    /* RA0 potentiometer */
    TRISA = TRISA | (1<<0); /* RA0 as input */

    /* RA1 onboard temperature sensor */
    TRISA = TRISA | (1<<1); /* RA1 as input */

    /* RA4 outside temperature sensor */
    TRISA = TRISA | (1<<4); /* RA4 as input */

// ADCON0
    ADCON0bits.CHS = 1;         // channel AN0
    ADCON0bits.GO_nDONE = 0;    // ADC Idle
    ADCON0bits.ADON = 0;        // turn OFF ADC module
// ADCON1
    ADCON1bits.VCFG = 0b00;     // Voltage ref AVdd AVss
    ADCON1bits.PCFG = 0b0000;   // A/D Port config AN0-AN4 analog, AN5-AN15 digital
// ADCON2
    ADCON2bits.ADFM = 1;        // 0=left, 1=right justified
    ADCON2bits.ACQT = 0b111;    // A/D Acquisition time 20 Tad
    ADCON2bits.ADCS = 0b010;    // A/D Conversion clock Fosc/32
} /* void initButtons(void) */



/*******************************************************************************
 * ADC Read Function
 */
unsigned int ADCRead(unsigned char ch)
{
   if(ch>13) return 0;  //Invalid Channel

   ADCON0bits.ADON = 1;     // disable AD module
   ADCON0bits.CHS = ch;      // select channel
   ADCON0bits.ADON = 1;     // switch on the adc module
   ADCON0bits.GO_nDONE = 1; //Start conversion
   while(ADCON0bits.GO_nDONE); //wait for the conversion to finish
   ADCON0bits.ADON = 0;  //switch off adc

   return ADRES;
} /* unsigned int ADCRead(unsigned char ch) */



/*******************************************************************************
 * Init PWM Function
 */
void initPwm(void)
{
    /* set pin direction */

    /* set pin out for heat element */
    TRIS_HEAT_ELEMENT = TRIS_OUT;
    /* set pin out for heat/vent fan */
    TRIS_HEAT_VENT_FAN = TRIS_OUT;
    /* set pin out for cool fan */
    TRIS_COOL_FAN = TRIS_OUT;

} /* void initPwm(void) */


/*******************************************************************************
 * Init Timer Function
 */
void initTmr(void)
{
    PORTJbits.RJ6 = 0;
    PORTJbits.RJ7 = 0;
    TRISJbits.TRISJ7 = 0;   // This sets pin RB7 to output
    TRISJbits.TRISJ6 = 0;   // This sets pin RB6 to output

    PORTDbits.RD3 = 0;
    PORTDbits.RD4 = 0;
    PORTDbits.RD5 = 0;
    TRISDbits.TRISD3 = 0;   // This sets pin RB3 to output
    TRISDbits.TRISD4 = 0;   // This sets pin RB4 to output
    TRISDbits.TRISD5 = 0;   // This sets pin RB5 to output


    // START - TMR0 setup
    TMR0 = 0;
    T0CON = 0;
    T0CONbits.TMR0ON = 0;   // timer OFF
    T0CONbits.T08BIT = 0;   // 16bit timer
    T0CONbits.T0CS = 0;     // internal source clock
    T0CONbits.T0SE = 0;     // source edge Low-2-High
    T0CONbits.PSA = 0;      // prescaler active
    T0CONbits.T0PS = 0;     // prescaler 3 bits (Fosc/4)/presc 1:2 => timer clock = (10MHz/4) / 2 = 1.25Mhz
    /* 1000Hz */
    /* 1khz => 1ms period
     * 1.25Mhz / 1khz => 1250
     * 65535 - 1250 = 64285 = FB1Dx;
     * TODO: Here can be improved
     */
    TMR1H = 0xFB;           //
    TMR1L = 0x1D;           //
    T0IE = 1; //enable TMR0 overflow interrupts
    GIE = 1; //enable Global interrupts
    T0CONbits.TMR0ON = 1;   // timer ON
// END - TMR0 setup
} /* void initTmr(void) */

unsigned char a = 0;


/*******************************************************************************
 * Interrupt Service Routine (keyword "interrupt" tells the compiler it's an ISR)
 */
void interrupt ISR(void)
{
// TMR0 interrupt
    if (T0IE && T0IF)
    {
        T0IF  = 0;              // clear interrupt flag
        T0CONbits.TMR0ON = 0;   // Turn timer off to reset count register
        TMR0H = 0xFB;           // Reset timer count - 1khz = 1ms
        TMR0L = 0x1D-2;         //

        T0CONbits.TMR0ON = 1;   // Turn timer back on
        tick++; // each 1ms

        if ((tick & 0b11) == 0b11) // each 4ms
        {
            cnt++;
            
            if (cnt == 25) // each 25*4ms = 100ms
            //if (cnt == 250) // each 250*4ms = 1000ms
            {
                ev = 1;
                cnt = 0;
            }
        }
        /* generate SW PWM for cool FAN */
        if (fanSpeedCool > (tick & 0x07))
            PIN_FAN_COOL = PIN_ON;
        else
            PIN_FAN_COOL = PIN_OFF;

        /* generate SW PWM for heat/vent FAN */
        if (fanSpeedHeatVent > (tick & 0x07))
            PIN_FAN_HEAT_VENT = PIN_ON;
        else
            PIN_FAN_HEAT_VENT = PIN_OFF;

        /* generated SW PWM for heat element */
        if (levelHeat  > (tick & 0x07))
            PIN_HEAT_ELEMENT = PIN_ON;
        else
            PIN_HEAT_ELEMENT = PIN_OFF;
    }

    // process other interrupt sources here, if required
}



/*******************************************************************************
 * Init Function
 */
void init(void)
{
    TRISD=0;
    PORTD=0;
    MEMCONbits.EBDIS=1;

    /* init UART */
   // UART_Init();
   //UART_puts((char *)"\n\rInitializing...\n\r");

    /* init buttons */
    initButtons();

    /* init ADC */
    initAdc();

    /* init PWM */
    initPwm();

    /* init TMR */
    initTmr();

    /* init LCD */
    LcdInit();

    /* START - transition from "Power OFF" to "OFF"*/
    climaState = STATE_OFF;
    setLcd(); /* LCD according to OFF state */

    /* Standby LED ON */
    setStandbyLed(ON);
    /* set Heating element OFF */
    setHeatElement(OFF);

    /* cool FAN speed 0 = OFF */
    setSpeedFanCool(0);
    /* heat/vent FAN speed 0 = OFF */
    setSpeedFanHeatVent(0);
    /* Heat level 0 = OFF */
    setHeatElement(0);

/* END - transition from "Power OFF" to "OFF"*/
} /* void init(void) */



/*******************************************************************************
 * Main Function
 */
void main(void)
{
    init();
    TRISD = 0;
    
            
        LcdClear();
        LcdGoTo(0);
        LcdWriteString("t");
    
    while(1)
    {
        

        
        while (ev == 0);
        ev = 0;

        checkInputs();
        stateMachine();
        updateOutputs();
        
        leftButtonEv = 0; 

    }
/* END - endless loop */

} /* void main(void) */



