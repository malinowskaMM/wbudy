/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 *****************************************************************************/


#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>
#include <config.h>

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

#define  SPI_CS   0x00008000  //<= new board, old board = 0x00800000


const tU8 directionText[] = {
	0x18,0x24,0x42,0x81,0xE7,0x24,0x24,0x3C,
	0x18,0x14,0xF2,0x81,0x81,0xF2,0x14,0x18,
	0x3C,0x24,0x24,0xE7,0x81,0x42,0x24,0x18,
	0x18,0x28,0x4F,0x81,0x81,0x4F,0x28,0x18,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00};

extern tU8 pattern[8];
pattern = {0,0,0,0,0,0,0,0};


/*!
*  @brief   Funkcja dostarczona przez tworce przykladu.
*/
static void
startTimer1(tU16 delayInMs)
{
  //initialize VIC for Timer1 interrupts
  VICIntSelect &= ~0x20;           //Timer1 interrupt is assigned to IRQ (not FIQ)
  VICVectAddr5  = (tU32)ledMatrix; //register ISR address
  VICVectCntl5  = 0x25;            //enable vector interrupt for timer1
  VICIntEnable  = 0x20;            //enable timer1 interrupt
  
  //initialize and start Timer #0
  T1TCR = 0x00000002;                           //disable and reset Timer1
  T1PC  = 0x00000000;                           //no prescale of clock
  T1MR0 = delayInMs *                           //calculate no of timer ticks
         ((CRYSTAL_FREQUENCY * PLL_FACTOR) / (1000 * VPBDIV_FACTOR));
  T1IR  = 0x000000ff;                           //reset all flags before enable IRQs
  T1MCR = 0x00000003;                           //reset counter and generate IRQ on MR0 match
  T1TCR = 0x00000001;                           //start Timer1
}


/*!
*  @brief   Funkcja dostarczona przez tworce przykladu.
*/
void
testLedMatrix(void)
{
  PINSEL0 |= 0x00001500 ;  //Initiering av SPI
  SPI_SPCCR = 0x08;    
  SPI_SPCR  = 0x60;
  IODIR0 |= SPI_CS;
  startTimer1(2);
}

/*!
*  @brief    Wypisuje strzalke na macierzy LED
*  @param direction Kierunek strzalki
*  @returns  brak
*  @side effects:
*            brak
*/
void
testArrow(tU8 direction)
{
	PINSEL0 |= 0x00001500 ;  //Initiering av SPI
    SPI_SPCCR = 0x08;
    SPI_SPCR  = 0x60;
    IODIR0 |= SPI_CS;

	tU8 cntA = 0;
	if (direction == 'g'){
		  cntA = 8;
	  }else if (direction == 'p'){
		  cntA = 16;
	  }else if (direction == 'd'){
		  cntA = 24;
	  }else if (direction == 'l'){
		  cntA = 0;
	  }else if (direction == 'c'){ //clear
		  cntA = 32;
	  }else {
        cntA = 0;
    }

		pattern[0] = directionText[cntA];
		pattern[1] = directionText[cntA+1];
		pattern[2] = directionText[cntA+2];
		pattern[3] = directionText[cntA+3];
		pattern[4] = directionText[cntA+4];
		pattern[5] = directionText[cntA+5];
		pattern[6] = directionText[cntA+6];
		pattern[7] = directionText[cntA+7];
}

