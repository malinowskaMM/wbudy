/******************************************************************************
 *
 * Copyright:
 *    Grupa G09
 *      Magdalena Malinowska
 *      MichaĹ‚ Andrzejczak
 *      MichaĹ‚ Banasiak
 *
 * Description:
 *    GĹ‚Ăłwny plik zawierajÄ…cy zaĹ‚Ä… logikÄ™ dziaĹ‚ania gry Refleks.
 *    PozostaĹ‚e pliki aktywnie uĹĽywane w projekcie to testRGB, testMotor, testLedMatrix, testLcd.
 *
 *****************************************************************************/

//===========================
//SEKCJA DYREKYW PREPROCESORA

//Sekcja skĹ‚adajÄ…ca siÄ™ z includowanych plikĂłw
#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>

//Rozmiary stosu
#define MAIN_STACK_SIZE 2048
#define INIT_STACK_SIZE 400

//===========================
//SEKCJA FUNKCJI

//Tworzenie stosu
static tU8 mainStack[MAIN_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;

//Funkcje poczÄ…tkowe - inicjalizujÄ…ce
static void mainFunction(void);
static void initProc(void);

//ObsĹ‚uga macierz LED 8x8 i SPI
void testLedMatrix(void);
void testArrow(tU8 direction);

//ObsĹ‚uga LCD
void testLcd(void);
void clearLCD(void);
void writeStr(tU8 *s);
void newLineLCD();
void writeInfo(char *console_message, char* lcd_1line, char *lcd_2line);
void clearLCD();
//ObsĹ‚uga Motoru i PWM(dioda LED RGB)
void testMotor(void);
void testRGB(tU8 red, tU8 green, tU8 blue);

//Przycisk P0.14
void pause(void);

//Logika rozgrywki
static tU8 mainMenu(void);
static void newGame(void);
static void roundsAmount(void);
static void roundsTime(void);
static void livesAmount(void);

//Funkcje pomocnicze
char* insertdNumber(char* message, int position, int number);

//===========================
//SEKCJA ZMIENNYCH GLOBALNYCH I TYPĂ“W UĹ»YTKOWNIKA

//WybĂłr menu
enum MENU_STATE
{
    MAIN_MENU,
    NEW_GAME,
    ROUND_TIME,
    ROUNDS_AMOUNT,
    LIVES_AMOUNT
};
static tU8 menu_state = MAIN_MENU;

//Stany joysticka
#define J_UP 		(1 << 17)			/** Doojstik - pozycja do gory */
#define J_DOWN		(1 << 20)			/** Doojstik - pozycja na dol */
#define J_LEFT		(1 << 19)			/** Doojstik - pozycja w lewo */
#define J_RIGHT		(1 << 18)			/** Doojstik - pozycja w prawo */
#define J_CENTER	(1 << 16)			/** Doojstik - nacisniety */


//Zmienna odpowiedzialna za upĹ‚yw czasu.
extern volatile long int time_ticks;

//Parametry rozgrywki
static tU8 lives = 2;
static tU32 round_time = 5000;
static tU8 round_amount = 4;			/**< IloÄŹĹĽËťÄŹĹĽËť rund */

//Inne zmienne rozgrywki
static tU32 max_answer_time = 0;		/**< NajdÄŹĹĽËťuÄŹĹĽËťszy czas rekacji gracza */
static tU32 min_answer_time = 32767;	/**< NajkrÄŹĹĽËťtszy czas rekacji gracza */
static long int entire_answer_time = 0;	/**< ÄŹĹĽËťredni czas rekacji gracza */

static tU32 j_position = 0;             /**< Losowanie pozycji joysticka */
/*!
 *  @brief    Pierwsza uruchamiana funkcja w programie. Uruchamia system operacyjny
 *  @param   brak
 *  @returns  brak
 *  @side effects:
 *            brak
 */
int main(void)
{
    tU8 error;
    tU8 pid;

    //WyĹ‚Ä…cza brzÄ™czyk (jeĹ›li jest podĹ‚Ä…czony)
    IODIR0 |= 0x00000080;
    IOSET0  = 0x00000080;

    osInit();     //Ta funkcja musi byĂ„â€ˇ uruchomiona przed jakimkolwiek innym odwoÄąâ€šaniem siĂ„â„˘ do systemu operacyjnego
    //Tworzenie i start procesu.
    osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
    osStartProcess(pid, &error);
    osStart();
    return 0;
}

/*!
*  @brief    Pierwsza funkcja ktĂłrÄ… uruchamiamy przez system operacyjny
*  @param arg
*             Ten parametr nie jest uĹĽywany w aplikacji
*  @returns  brak
*  @side effects:
*            brak
*/
static void initProc(void)
{
    tU8 error;

    eaInit();     //Inicjalizacja terminala
    testLcd();    //Inicjalizacja wyĹ›wietlacza ???
    testLedMatrix();

    //Tworznie i start procesu, a nastÄ™pnie jego usuniÄ™cie.
    osCreateProcess(mainFunction, mainStack, MAIN_STACK_SIZE, &pid1, 3, NULL, &error);
    osStartProcess(pid1, &error);
    osDeleteProcess();
}

/*!
*  @brief    GĹ‚Ăłwna funkcja gry. Zawiera teĹĽ pÄ™tlÄ™ odpowiadajÄ…cÄ… za menu.
*  @param arg
*             Ten parametr nie jest uĹĽywany w aplikacji
*  @returns  brak
*  @side effects:
*            brak
*/
static void mainFunction(void) {

    testRGB(0,0,0);     //Wyczyszczenie diody RGB.

    writeInfo("Zespol Michal Banasiak, Magdalena Malinowska, Michal Andrzejczak zaprasza do zagrania w gre REFLEKS.\n", "Wyproboj swoj", "REFLEKS");
    printf("LEGENDA ruchu joystickiem:\n1-ruch joystickiem w gore, 2-w lewo,3-w prawo, 4-w dol, 5-srodek\n");
    pause();

    //PĂ„â„˘tla menu
    while (1) {
        menu_state = mainMenu();    //WybÄ‚Ĺ‚r przycisku z menu za pomocĂ„â€¦ joysticka

        if (menu_state == (tU8) NEW_GAME) {
            newGame();      //Rozpocznij grĂ„â„˘
        } else if (menu_state == (tU8) ROUNDS_AMOUNT) {
            roundsAmount();     //Ustawienie iloÄąâ€şci rund
        } else if (menu_state == (tU8) ROUND_TIME) {
            roundsTime();       //Ustawienie czasu rundy
        } else if (menu_state == (tU8) LIVES_AMOUNT) {
            livesAmount(); //Ustawienie liczby ÄąÄ˝yĂ„â€ˇ
        } else
        {}
    }
}

/*!
*  @brief    WybĂłr opcji z menu. Odczyt z joystick'a.
*  @param brak
*  @returns  Odczyt z joystick'a oznaczajÄ…cy wybĂłr opcji z menu.
*  @side effects:
*            brak
*/
static tU8 mainMenu(void)
{
    //Wypisanie infromacji
    writeInfo("Glowne menu\n", "Glowne menu", "Wybor-joystick");
    pause();
    writeInfo("1-Gra 2-Czas 3-Rundy 4-Zycia\n", "1-Graj 2-Czas", "3-Rundy 4-Zycia");
    testMotor();
    //WybĂłr opcji za pomocÄ… joystick'a
    IODIR &= ~0x001f0000;   //Odczyt z joysticka
    while(1)
    {
        if (~IOPIN & J_UP)
        {
            return NEW_GAME;
        }
        else if(~IOPIN & J_LEFT)
        {
            return ROUND_TIME;
        }
        else if(~IOPIN & J_RIGHT)
        {
            return ROUNDS_AMOUNT;
        }
        else if(~IOPIN & J_DOWN)
        {
            return LIVES_AMOUNT;
        } else
        {}

    }
}

/*!
*  @brief    GĹ‚Ăłwna rozgrywka
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void newGame(void)
{
    //RozpoczÄ™cie gry
    writeInfo("Wybrano nowa gre.\n","Wybrano","Nowa Gra");
    osSleep(200);
    testMotor();
    //Parametry rozgrywki
    printf("Ilosc rund: %d.\nCzas rundy: %d ms\nIlosc zyc: %d\n", round_amount, round_time, lives);
    char messagev1[] = " Rundy:    ";
    writeInfo("", insertdNumber(messagev1, 9, round_amount), "");
    osSleep(200);
    char messagev2[] = " Czas:      ";
    writeInfo("", insertdNumber(messagev2, 10, round_time), "");
    osSleep(200);
    char messagev3[] = " Zycia:    ";
    writeInfo("", insertdNumber(messagev3, 9, lives), "");
    osSleep(200);

    //Wyczyszczenie zmiennych rundy
    max_answer_time = 0;		/**< Najdďż˝uďż˝szy czas rekacji gracza */
    min_answer_time = 30000;	/**< Najkrďż˝tszy czas rekacji gracza */
    entire_answer_time = 0;		/**< ďż˝redni czas rekacji gracza */
    tU8 tmpLives=lives;

    tU32 start_time;    //Zmienna odpowiadajÄ…ca za czas rozpoczÄ™cia rundy

    IODIR &= ~0x001f0000;   //Odczyt z joysticka

    //Zgadywanie strzaĹ‚ki przez wybranÄ… iloĹ›Ä‡ rund.
    int i;
    for(i=0; i<round_amount; i++)
    {
        j_position = time_ticks%4;    //Losowanie strzaĹ‚ki
        if(j_position==(tU32)0)
        {
            testArrow('g');
        }
        if(j_position==(tU32)1)
        {
            testArrow('d');
        }
        if(j_position==(tU32)2)
        {
            testArrow('l');
        }
        if(j_position==(tU32)3)
        {
            testArrow('p');
        }


        //Infomacje o rundzie
        char message[] = "Runda:    ";
        writeInfo("Uzyj joystick w kierunku wskazanym przez strzalke.\n", "Wybor-joystick.", insertdNumber(message, 9, i+1));

        //Czas rozpoczÄ™cia rundy - czas od ktĂłrego liczony jest czas na reakcjÄ™
        start_time = time_ticks;

        tU32 result = 0;
        tU32 correct = 0;
        tU32 prevIOPIN;
        tU32 answer;

        //PÄ™tla reakcji na strzaĹ‚kÄ™
        while(1)
        {
            prevIOPIN = IOPIN;
            osSleep(5);

            answer = (prevIOPIN & ~IOPIN) & 0x001f0000; //PorÄ‚Ĺ‚wnanie, czy joystick siĂ„â„˘ zmieniÄąâ€š

            //Sprawdznie, czy wybrano dobry kierunek
            if(answer != (tU32) 0x00000000)
            {
                if ((~IOPIN & J_UP) && (j_position == 0))
                {
                    correct = 1;
                }
                else if ((~IOPIN & J_DOWN) && (j_position == 1))
                {
                    correct = 1;
                }
                else if ((~IOPIN & J_LEFT) && (j_position == 2))
                {
                    correct = 1;
                }
                else if ((~IOPIN & J_RIGHT) && (j_position == 3))
                {
                    correct = 1;
                } else
                {}

                result = (tU32)time_ticks - (tU32)start_time;   //Obliczenie czasu reakcji
                break;
            }

            //Ograniczenie reakcji do zadeklarowanego w menu czasu na odpowiedÄąĹź w danej rundzie
            if(((tU32)time_ticks - (tU32)start_time)>(tU32)round_time)
            {
                correct = -1;
                break;
            }
        }

        //Zmiana zmiennych
        if(correct == (tU32) 1)
        {
            //Zmiana maksymalnego i minimalnego czasu na ruch.
            if(max_answer_time < result)
            {
                max_answer_time = result;
            }
            else if(min_answer_time > result)
            {
                min_answer_time = result;
            } else
            {}

            entire_answer_time += (long) result;   //Suma czasu odpowiedzi

            testRGB(0, 255, 0); //Poprawna odpowiedÄąĹź
            osSleep(50);
            testRGB(0, 0, 0);
            //testMotor();    //Famfary za poprawnĂ„â€¦ odpowiedÄąĹź


            //Informacje dla gracza
            printf("Odpowiedz byla poprawna, a szybkosc wystarczajaca\nPozostala liczba rund: %d.\nTwoj czas zareagowania: %d ms.\n", round_amount, result);
            char message[] = "Czas:     ";
            writeInfo("", "Poprawnie", insertdNumber(message, 9, result));
            osSleep(300);
        }
        else if(correct == (tU32) -1)
        {
            //Dla za pĂłĹşnego ruchu
            testRGB(255, 255, 0);
            osSleep(50);
            testRGB(0, 0, 0);
            //testMotor();
            writeInfo("Nie zdazyles w ustalonym czasie. Tracisz zycie.\n", "Brak czasu.", "Straciles zycie.");
            tmpLives--;
            osSleep(300);
        }
        else
        {
            //Dla niepoprawnego ruchu
            testRGB(255, 0, 0);
            osSleep(50);
            testRGB(0, 0, 0);
            //testMotor();
            writeInfo("Wybrales zly kierunek.Tracisz zycie\n", "Zly kierunek", "Strata zycia");
            tmpLives--;
            osSleep(300);
        }

        if(tmpLives< (tU8) 1)
        {
            //Gdy skoĹ„czyĹ‚a siÄ™ liczba ĹĽyÄ‡.
            testRGB(255, 255, 255);
            writeInfo("Straciles wszystkie zycia. Przegrales.\n", "Koniec zyc.", "Koniec gry.");
            osSleep(300);

            //Wyczyszczenie zmiennych rozgrywki
            max_answer_time = 0;		/**< NajdÄŹĹĽËťuÄŹĹĽËťszy czas rekacji gracza */
            min_answer_time = 32767;	/**< NajkrÄŹĹĽËťtszy czas rekacji gracza */
            entire_answer_time = 0;		/**< ÄŹĹĽËťredni czas rekacji gracza */

            clearLCD();
            testArrow('c');
            testRGB(0,0,0);     //Wyczyszczenie diody RGB.

            break;
        }
        testRGB(0,0,0);     //Wyczyszczenie diody RGB.
    }

    //Podsumowanie rozgrywki
    if(entire_answer_time == 0) //Nie ukoÄąâ€žczono gry z powodu straty wszystkich ÄąÄ˝yĂ„â€ˇ
    {
        return;
    }

    //Informacja o interakcji
    writeInfo("Gra sie zakonczyla.\n", "Koniec Gry", "");
    pause();

    //Podsumowanie

    writeInfo("","Gratulacje","");
    pause();
    printf("Najpozniejsza reakcja, najszybsza reakcja: %d, %d\n", max_answer_time, min_answer_time);
    char message3[] = "Najw.:     ", message4[] = "Najm.:     ";
    writeInfo("", insertdNumber(message3, 10, max_answer_time), insertdNumber(message4, 10, min_answer_time));
    osSleep(500);

    clearLCD();
    testArrow('c');
    testRGB(0,0,0);     //Wyczyszczenie diody RGB.


}

/*!
*  @brief    Ustawienie iloĹ›ci rund.
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void roundsAmount(void)
{
    //Informacjie o interakcji
    writeInfo("Menu wyboru liczby rund.\nSterowanie odbywa sie za pomoca joysticka", "Liczba rund", "Wybor-joystick");
    pause();
    writeInfo("1-Zwieksz o jedna runde, 4 zmniejsz o jedna runde, 2,3,5-wyjdz\n", "1-Zwieksz", "4-Zmniejsz");
    pause();
    tU8 end = 0;
    tU8 change = 0;

    IODIR &= ~0x001f0000;   //Odczyt z joystick'a
    //WybĂłr iloĹ›ci rund
    while(!end)
    {
        if (~IOPIN & J_UP)
        {
            round_amount++;
            change = 1;
        }
        else if (~IOPIN & J_DOWN)
        {
            if(round_amount<=(tU8)1)
            {
                round_amount=1;
            }
            else
            {
                round_amount--;
            }
            change = 1;
        }
        else if (~IOPIN & J_CENTER)
        {
            end = 1;
        }
        else if (~IOPIN & J_RIGHT)
        {
            end = 1;
        }
        else if (~IOPIN & J_LEFT)
        {
            end = 1;
        } else
        {}

        //WyÄąâ€şwietlenie zmiany
        if(change == (tU8) 1)
        {
            printf("Rundy: %d.\n", round_time);
            char message[] = "   ";
            writeInfo( "", "Rundy:", insertdNumber(message, 2, round_amount));
            change = 0;
            osSleep(100);
        }
    }

    //Informacja o ostatecznej zmianie
    printf("Rundy: %d.\nNacisnij przycisk.\n", round_amount);
    char message[] = " Rundy:    ";
    writeInfo("", insertdNumber(message, 9, round_amount), "");
    pause();

}

/*!
*  @brief    Zmiana czasu na reakcjÄ™.
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void roundsTime(void)
{
    //Informacje o interakcji
    writeInfo("Menu wyboru ilosci  czasu.\nSterowanie odbywa sie za pomoca joysticka", "Ilosc czasu", "Wybor-joystic");
    pause();
    writeInfo("1-Zwieksz o sekunde, 4 zmniejsz o sekunde, 2,3,5-wyjdz\n", "1-Zwieksz", "4-Zmniejsz");
    pause();
    tU8 end = 0;
    tU8 change = 0;

    IODIR &= ~0x001f0000;   //Odczyt z joystick'a
    //PÄ™tla zmiany czasu na reakcjÄ™
    while(!end)
    {
        if(~IOPIN & J_UP)
        {
            round_time+=(tU32)1000;
            change = 1;
        }
        else if(~IOPIN & J_DOWN)
        {
            if(round_time<=(tU32)1000)
            {
                round_time=(tU32)1000;
            }
            else
            {
                round_time-=(tU32)1000;
            }
            change = 1;
        }
        else if(~IOPIN & J_LEFT)
        {
            end = 1;
        }
        else if(~IOPIN & J_RIGHT)
        {
            end = 1;
        }
        else if(~IOPIN & J_CENTER)
        {
            end = 1;
        } else
        {}

        //WyÄąâ€şwietlenie zmiany
        if(change == (tU8) 1)
        {
            printf("Czas: %d.\n", round_time);
            char message[] = "     ";
            writeInfo( "", " Czas:", insertdNumber(message, 4, round_time));
            change = 0;
            osSleep(10);
        }
    }

    //Ostateczny stan.
    printf("Czas: %d.\nNacisnij przycisk.\n", round_time);
    char message[] = " Czas:      ";
    writeInfo("", insertdNumber(message, 10, round_time), "");
    pause();
}

/*!
*  @brief    Zmiana liczby ĹĽyÄ‡.
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void livesAmount(void)
{
    //Informacjie o interakcji
    writeInfo("Menu wyboru ilosci zyc.\nSterowanie odbywa sie za pomoca joysticka", "Ilosc zyc", "Wybor-joystic");
    pause();
    writeInfo("1-Zwieksz o sekunde, 4 zmniejsz o sekunde, 2,3,5-wyjdz\n", "1-Zwieksz", "4-Zmniejsz");
    pause();
    tU8 end = 0;
    tU8 change = 0;

    IODIR &= ~0x001f0000;   //Odczyt z joystick'a
    //WybĂłr iloĹ›ci rund
    while(!end)
    {
        if (~IOPIN & J_UP)
        {
            lives++;
            change = 1;
        }
        else if (~IOPIN & J_DOWN)
        {
            if(lives<=(tU8)1)
            {
                lives=1;
            }
            else
            {
                lives-=(tU8)1;
            }
            change = 1;
        }
        else if (~IOPIN & J_CENTER)
        {
            end = 1;
        }
        else if (~IOPIN & J_RIGHT)
        {
            end = 1;
        }
        else if (~IOPIN & J_LEFT)
        {
            end = 1;
        } else
        {}

        //WyÄąâ€şwietlenie zmiany
        if(change == (tU8) 1)
        {
            printf("Zycia: %d.\n", lives);
            char message[] = "   ";
            writeInfo( "", "Zycia:", insertdNumber(message, 2, lives));
            change = 0;
            osSleep(100);
        }
    }

    //Informacja o ostatecznej zmienie
    printf("Zycia: %d.\nNacisnij przycisk\n", lives);
    char message[] = " Zycia:    ";
    writeInfo("", insertdNumber(message, 9, lives), "");
    pause();

}

/*!
*  @brief    Wypisuje informacje na konsolÄ™ i na LCD.
*  @param console_message
*             WiadomoĹ›Ä‡ na konsolÄ™
*  @param lcd_1line
*             WiadomoĹ›Ä‡ do pierwszego wiersza LCD.
*  @param lcd_2line
*             WiadomoĹ›Ä‡ do drugiego wiersza LCD.
*  @returns  brak
*  @side effects:
*            brak
*/
void writeInfo(char *console_message, char* lcd_1line, char *lcd_2line)
{
    clearLCD();
    writeStr(lcd_1line);
    newLineLCD();
    writeStr(lcd_2line);
    printf(console_message);
}

/*!
*  @brief    Wstawiam do konkretnej wiadomoĹ›Ä‡ podany numer na podanÄ… pozycjÄ™.
*  @param message
*             WiadomoĹ›Ä‡ do ktĂłrej wstawiamy numer
*  @param position
*             Pozycja na ktĂłrej wstawiamy numer.
*  @param number
*             Wstawiany numer
*  @returns  Zmieniona wiadomoĹ›Ä‡
*  @side effects:
*            brak
*/
char* insertdNumber(char* message, int position, int number)
{
    //Zabezpiecz przypadek 0
    position = position - (int) 1;
    message[position] = '0' + (number % 10);
    number /= 10;

    while(number)
    {
        position = position - (int) 1;
        message[position] = '0' + (number % 10);
        number /= 10;
    }
    return message;
}

/*!
*  @brief    Pauzuje grÄ™ do czasu naciĹ›niÄ™ci P0.14
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
void pause(void)
{
    IODIR &= ~0x00004000;			//odczytuje P0.14
    while(1)
        if (!(IOPIN & 0x00004000))
        {
            break;
        }

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
void appTick(tU32 elapsedTime) {}
