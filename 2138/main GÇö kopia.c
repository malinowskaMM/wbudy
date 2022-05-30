/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Sample application that demonstrates how to create processes.
 *
 *****************************************************************************/


#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>

#define MAIN_STACK_SIZE 2048
#define INIT_STACK_SIZE 400

static tU8 mainStack[MAIN_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;

static void mainFunction(void* arg);
static void game(void);
static void initProc(void* arg);
int rand(void);
static void reflexesGame(void);

void testLedMatrix(void); //
void testArrow(tU8 direction);
void testLcd(void); //
void writeStr(tU8 *s);
void newLineLCD();
void testMotor(void); //
void testRGB(tU8 red, tU8 green, tU8 blue); //
void testI2C(void);
void testAdc(void);
void initialize();



tU8 mainMenu(void);
static void startTimer1(tU16 delayInMs);
void pause(void);

enum MENU_STATE							//Stan menu
{
    MAIN_MENU,
    NEW_GAME,
    ROUND_TIME,
    ROUNDS_AMOUNT,
    LAST_SCORE
};

#define J_UP 		(1 << 17)			/** Doojstik - pozycja do gory */
#define J_DOWN		(1 << 20)			/** Doojstik - pozycja na dol */
#define J_LEFT		(1 << 19)			/** Doojstik - pozycja w lewo */
#define J_RIGHT		(1 << 18)			/** Doojstik - pozycja w prawo */
#define J_CENTER	(1 << 16)			/** Doojstik - nacisniety */

static tU8 menu_state = MAIN_MENU;
extern volatile long int time_ticks;
volatile long int choose;
static tU32 round_time = 5000;

/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;

  //immediately turn off buzzer (if connected)
  IODIR0 |= 0x00000080;
  IOSET0  = 0x00000080;
  
  osInit();     //This function must be called before any other call to the operating system.
	printf("Ala ma Asa..........\n");
  while(1);
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
initProc(void* arg)
{
  tU8 error;

                //Initializes the evaluation board environment
  eaInit();     //If using the console functionality (printf()- and scanf()-like functions) observe that the function eaInit() must be called before printf() and the console can be used.
  testLcd();
  osCreateProcess(mainFunction, mainStack, MAIN_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);
  osDeleteProcess();    // deletes the currently running process. The process control block used by the process will be freed and is therefore available for new processes.
}

static void mainFunction(void* arg)
{
    //GĹ‚ĂłwnaPÄ™tlaGry
    //W zamysle to bedzie glowne menu wybierane joystickiem. Narazie robie ze joystick wybiera funkcjonalnosci, ktore udalo sie zrobic xD
	initialize();

	printf("Ala ma Asa\n");
	while(1)
    {
        menu_state = mainMenu();
        testMotor();
        printf("jestem zadowol");
        if(menu_state == NEW_GAME)
        {
            testRGB(0, 0, 255);
            /*reflexesGame();*/
            /*reflexesGame();*/
            pause();
            game();
            /*game();*/
        }
        else if(menu_state == LAST_SCORE)
        {
            testRGB(0, 255, 0);
            pause();
        }
        else if(menu_state == ROUNDS_AMOUNT)
        {
            testRGB(255, 0, 0);
            pause();
        }
        else if(menu_state == ROUND_TIME)
        {
            testRGB(125,125,125);
            pause();
        }
        else if(menu_state == MAIN_MENU)
        {
            testLedMatrix();
            pause();
        }
    }
}

static void messageLED(int correct) {
    if(correct == 0) {
        writeStr("Gratulacje !!!  Dobry ruch!");
    } else if (correct == -1) {
        writeStr("Niestety :(((   Zły ruch!");
    }
}

tU8 mainMenu(void)
{
    tU32 start_time = 0;
    start_time = time_ticks;
    IODIR &= ~0x001f0000;   //Odczytuje wejĹ›cie joysticka
    while(1)
    {
        if (~IOPIN & J_UP)
            return NEW_GAME;
        else if(~IOPIN & J_DOWN)
            return LAST_SCORE;
        else if(~IOPIN & J_LEFT)
            return ROUND_TIME;
        else if(~IOPIN & J_RIGHT)
            return ROUNDS_AMOUNT;
        else if(~IOPIN & J_CENTER)
            return MAIN_MENU;
        if((time_ticks - start_time)>round_time)
        {
            return LAST_SCORE;
        }
    }
    return MAIN_MENU;
}

static void game(void)
{
	while(1)
	{
		pause();
		testRGB(0, 0, 0);
		pause();
		tU8 chosenChar = time_ticks%4;
		tU8 playerChoice = 4;

		if(chosenChar==0 )
			testArrow('g');
		if(chosenChar==1 )
			testArrow('d');
		if(chosenChar==2 )
			testArrow('l');
		if(chosenChar==3 )
			testArrow('p');


/*		IOPIN &= ~0x00000000;*/
		//Odczytuje wejĹ›cie joysticka
		IODIR &= ~0x001f0000;
		while(1)
		{
			if (~IOPIN & J_UP)
		   {
			   playerChoice =0;
			   break;
		   }
		   else if(~IOPIN & J_DOWN)
		   {
			   playerChoice =1;
			   break;
		   }
		   else if(~IOPIN & J_LEFT)
		   {
			   playerChoice =2;
			   break;
		   }
		   else if(~IOPIN & J_RIGHT)
		   {
			   playerChoice =3;
			   break;
		   }
		}

		   if (playerChoice == chosenChar) {
               messageLED(0);
               testRGB(0, 255, 0);
           }
		   else {
               messageLED(-1);
			   testRGB(255, 0, 0); }
	}

}



void pause(void)
{
    IODIR &= ~0x00004000;			//odczytuje P0.14
    while(1)
        if (!(IOPIN & 0x00004000))
            break;
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void
appTick(tU32 elapsedTime)
{
}

