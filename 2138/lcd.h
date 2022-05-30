/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    lcd.h
 *
 * Description:
 *    Expose public functions related to LCD functionality.
 *
 *****************************************************************************/
#ifndef _LCD_H_
#define _LCD_H_

static void delay37us(void);
static void initLCD(void);
static void delay(void);
void secondRowLCD(void);
void clearLCD(void);
static void writeLCD(tU8 reg, tU8 data);
static void lcdBacklight(tU8 onOff);
void printStringLCD(tU8 *string);

#endif
