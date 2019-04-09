/* ========================================================================== */
/*                                                                            */
/*   DigitalInputs.c                                                               */
/*   (c) 2015 Micu&Georgiana                                                          */
/*                                                                            */
/*  Verificare Intrari digitale                                                             */
/*                                                                            */
/* ========================================================================== */


#include <stdio.h>
#include <stdlib.h>

#include <p18f8722.h>
#include "spi.h"
#include "delays.h"
#include "LCD.h"
#include "DigitalInputs.h"
#include <string.h>

// configuration bits
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config WDT = OFF        // Watchdog Timer (WDT disabled (control is placed on the SWDTEN bit))
#pragma config LVP = OFF

/* TODO: Write your port here */
#define LEFTBUTTON PORTBbits.RB0
#define RIGHTBUTTON PORTAbits.RA4 
/* END OF TODO: Write your port here */



/*******************************************************************************
 * Init Buttons Function
 */
void initButtons(void)
{
    /* RB0 butonul stang */
    TRISB0 = 1; /* configurare RB0 ca input */
    
    /* RA4 butonul drept */
    TRISA4 = 1; /* configurare RA4 ca input */

} /* void initButtons(void) */





/*******************************************************************************
   
 * sequence1 -  Functie pentru afisarea starii unui buton (Pressed sau Not pressed)
 */
void sequence1(void)
{
    char leftButton = LEFTBUTTON;

    /*Verificare daca butonul stang a fost apasat*/

    if (0 == leftButton)
    {
        LcdGoTo(0);

        /*Afisare pe ecran starea butonului - Pressed*/

        LcdWriteString("Pressed        ");

    }
    else
    {
        LcdGoTo(0);

        /*Afisare pe ecran starea butonului - Not pressed*/

        LcdWriteString("Not pressed    ");
        
        
    }

} /* void sequence1(void) */



void sequence2(void)
{
    /* TODO: 
  *Afisarea numarului de apasari pentru butonul drept RA4
     * 
     * 
  * END OF TODO */
    
    unsigned char text[16];
    static int count;
    char leftButton = LEFTBUTTON;
    static char ok;
    

    LcdGoTo(0);
    
   
    //sprintf(text, "Value: %d", count);
   // LcdWriteString(text);
         
    sprintf(text, " Value: %d", count);
     LcdWriteString(text);

    if (0 == leftButton && !ok)
    {
        count++; 
        ok = 1;
        
       if (count <= 0xff)
            LATD = 0x00 | count;

    }
    else if(1 == leftButton)
        ok = 0;  

}/* void sequence2(void) */

void sequence3(void)

{
  /* TODO: 
   *
  *Implementarea SW Debounce pentru butonul stang RB0
  * END OF TODO */
    unsigned char text[16];
    static int count;
    char leftButton = LEFTBUTTON, rightButton = RIGHTBUTTON;
    static char ok;
    
    static char somethingImportant;
    

    LcdGoTo(0);
    
   
    //sprintf(text, "Value: %d", count);
   // LcdWriteString(text);
         
    sprintf(text, " Value: %d", count);
     LcdWriteString(text);

    if (0 == leftButton && !ok || !somethingImportant)
    {
        count++; 
        ok = 1;
        
       if (count <= 0xff)
            LATD = 0x00 | count;
       else count = 0;

    }
    else if(1 == leftButton)
        ok = 0;  
    
 
    
}

/* void sequence3(void) */



/*
 * "sequence"  - Variabila masinii de stari care va fi initializata cu numarul exercitiului pe care doriti sa il rulati
 *  Ex: Daca doriti sa rulati seqeunce1(), initializati variabila "sequence" cu 1. s.a.m.d
 */

int sequence = 3;
 
void main()
{
 /* Functie pentru setarea butoanelor ca pini de intrare  */

  initButtons();

  /*initializarea LCD-ului - functia este apelata de 3 ori pentru stabilizare*/

  LcdInit();
  LcdInit();
  LcdInit();
  
  	/* set all PortD bits as Output */
	TRISD = 0x0; 
	/* set state of the pins as STD_OFF */
	LATD = 0x0; 

  while(1)
  {
      switch(sequence)
      {
          case 1:
          {
              /*Executarea primei secvente de citire a starii butonului RB0- pressed/not pressed*/
              sequence1();
              break;
          }
          case 2:
          {
              /*Executarea secventei a 2-a care contorizeaza numarul de apasari ale butonului RA4*/
              sequence2();
              break;
          }

          case 3:
          {
              /*Executarea secventei a 3-a SW Debounce pe butonul RB0*/
              sequence3();
              break;
          }

          default:
          {
              break;
          }
      }
    
  }
  
  
}
