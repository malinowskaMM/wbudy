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
void testMotor(void) {
    tU8 continueTest;
    tU32 counter = 0;
    tU8 direction = 0;
    tU8 forward = 0;

    PINSEL0 |= 0x000a0000; //Enable PWM4 on P0.8 and PWM6 on P0.9
    IODIR0 |= 0x00000400; //P0.10 = output
    IOCLR0 = 0x00000400; //set P0.10 low = disable motor

    //PULSE WIDTH MODULATION INIT*********************************************
    PWM_PR = 0x00; // Prescale Register
    PWM_MCR = 0x02; // Match Control Register
    PWM_MR0 = 0x1000; // TOTAL PERIODTID   T
    PWM_MR4 = 0x0000; // H�G SIGNAL        t
    PWM_MR6 = 0x0000; // H�G SIGNAL        t
    PWM_LER = 0x51; // Latch Enable Register
    PWM_PCR = 0x5000; // Prescale Counter Register PWMENA4, PWMENA6
    PWM_TCR = 0x09; // Counter Enable och PWM Enable
    //************************************************************************
    IOSET0 = 0x00000400; //ustaw P0.10 wysoko = wlacz silniczek

    continueTest = TRUE;
    while (continueTest == TRUE) {
        if (direction == 0) {
            counter += 0x0060;
            if (counter > 0x1100) {
                direction = 1;
                IOCLR0 = 0x00000400;
            }
        } else {
            if (counter == 0x0000) {
                direction = 0;
                PWM_MR4 = counter; // H�G SIGNAL
                PWM_MR6 = counter; // H�G SIGNAL
                PWM_LER = 0x50; // Latch Enable Register
                continueTest = FALSE;
                //}
            } else
                counter -= 0x0010;
        }

        if (forward == 0) {
            PWM_MR4 = counter; // H�G SIGNAL
            PWM_LER = 0x10; // Latch Enable Register
        } else {
            PWM_MR6 = counter; // H�G SIGNAL
            PWM_LER = 0x40; // Latch Enable Register
        }
        osSleep(1);
    }
    IOCLR0 = 0x00000400;
}

