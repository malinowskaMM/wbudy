

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

/*****************************************************************************
 *
 * Description:
 *    A process entry function.
 *
 ****************************************************************************/
void testRGB(tU8 red, tU8 green, tU8 blue)
{
    PINSEL0 &= 0xfff03fffu;  //Enable PWM2 on P0.7, PWM4 on P0.8, and PWM6 on P0.9
    PINSEL0 |= 0x000a8000;  //Enable PWM2 on P0.7, PWM4 on P0.8, and PWM6 on P0.9

    //PULSE WIDTH MODULATION INIT*********************************************
    PWM_PR  = 0x00;    // Prescale Register
    PWM_MCR = 0x02;    // Match Control Register
    PWM_MR0 = 0x1000;    // TOTAL PERIODTID   T
    PWM_MR2 = red * 3;    // H�G SIGNAL        t
    PWM_MR4 = blue * 3;    // H�G SIGNAL        t
    PWM_MR6 = green * 3;    // H�G SIGNAL        t
    PWM_LER = 0x55;    // Latch Enable Register
    PWM_PCR = 0x5400;  // Prescale Counter Register PWMENA2, PWMENA4, PWMENA6
    PWM_TCR = 0x09;    // Counter Enable och PWM Enable
    //************************************************************************
}
