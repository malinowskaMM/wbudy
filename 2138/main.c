/******************************************************************************
 *
 * Copyright:
 *    Grupa G09
 *      Magdalena Malinowska
 *      Michal Andrzejczak (lider)
 *      Michal Banasiak
 *
 * Description:
 *    Glowny plik zawierajacy logike dzialania gry Refleks.
 *    Pozostale pliki aktywnie uz�ywane w projekcie to testRGB, testMotor, testLedMatrix, testLcd.
 *
 *****************************************************************************/

//===========================
//SEKCJA DYREKYW PREPROCESORA

//Sekcja skladajca sie z includowanych plikow
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

//Funkcje poczatkowe - inicjalizuje
static void mainFunction(void);
static void initProc(void);

//Obluga macierz LED 8x8 i SPI
void testLedMatrix(void);
void testArrow(tU8 direction);

//Obsluga LCD
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

//Wybor menu
enum MENU_STATE
{
    MAIN_MENU,
    NEW_GAME,
    ROUND_TIME,
    ROUNDS_AMOUNT,
    LIVES_AMOUNT
};

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
static tU8 round_amount = 4;			/**< Ilosc rund */

//Inne zmienne rozgrywki



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
    tU8 initStack[INIT_STACK_SIZE];

    //Wylacza brzeczyk (jezeli jest podlaczony)
    IODIR0 |= 0x00000080;
    IOSET0  = 0x00000080;

    osInit();     //Ta funkcja musi byc uruchomiona przed jakimkolwiek innym odwolniem sie do systemu operacyjnego
    //Tworzenie i start procesu.
    osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
    osStartProcess(pid, &error);
    osStart();
    return 0;
}

/*!
*  @brief    Pierwsza funkcja ktore uruchamiamy przez system operacyjny
*  @param arg
*             Ten parametr nie jest uzywany w aplikacji
*  @returns  brak
*  @side effects:
*            brak
*/
static void initProc(void)
{
    tU8 mainStack[MAIN_STACK_SIZE];
    tU8 error;
    tU8 pid1;

    eaInit();     //Inicjalizacja terminala
    testLcd();    //Inicjalizacja wyswietlacza ???
    testLedMatrix();

    //Tworznie i start procesu, a nastepnie jego usuniecie.
    osCreateProcess(mainFunction, mainStack, MAIN_STACK_SIZE, &pid1, 3, NULL, &error);
    osStartProcess(pid1, &error);
    osDeleteProcess();
}

/*!
*  @brief    Glowna funkcja gry. Zawiera takze petle odpowiadajaca za menu.
*  @param arg
*             Ten parametr nie jest uzywany w aplikacji
*  @returns  brak
*  @side effects:
*            brak
*/
static void mainFunction(void) {

    testRGB(0,0,0);     //Wyczyszczenie diody RGB.
    const char* p1 = "Zespol Michal Banasiak, Magdalena Malinowska, Michal Andrzejczak zaprasza do zagrania w gre REFLEKS.\n";
    const char* p2 = "Wyproboj swoj";
    const char* p3 ="REFLEKS";
    writeInfo(p1, p2, p3);
    (void)printf("LEGENDA ruchu joystickiem:\n1-ruch joystickiem w gore, 2-w lewo,3-w prawo, 4-w dol, 5-srodek\n");
    pause();

    static tU8 menu_state = MAIN_MENU;
    //PÄ™tla menu
    while (1) {
        menu_state = mainMenu();    //Wybor przycisku z menu za pomoca joysticka

        if (menu_state == (tU8) NEW_GAME) {
            newGame();      //Rozpocznij gre
        } else if (menu_state == (tU8) ROUNDS_AMOUNT) {
            roundsAmount();     //Ustawienie ilosci rund
        } else if (menu_state == (tU8) ROUND_TIME) {
            roundsTime();       //Ustawienie czasu rundy
        } else if (menu_state == (tU8) LIVES_AMOUNT) {
            livesAmount(); //Ustawienie liczby zyc
        } else
        {}
    }
}

/*!
*  @brief    Wybor opcji z menu. Odczyt z joystick'a.
*  @param brak
*  @returns  Odczyt z joystick'a oznaczajacy wybor opcji z menu.
*  @side effects:
*            brak
*/
static tU8 mainMenu(void)
{
    //Wypisanie infromacji
    const char* p1 = "Glowne menu\n";
    const char* p2 = "Glowne menu";
    const char* p3 = "Wybor-joystick";
    writeInfo(p1, p2, p3);
    pause();
    const char* p4 = "1-Gra 2-Czas 3-Rundy 4-Zycia\n";
    const char* p5 = "1-Graj 2-Czas";
    const char* p6 = "3-Rundy 4-Zycia";
    writeInfo(p4, p5, p6);
    testMotor();
    //Wybor opcji za pomoca joystick'a
    IODIR &= ~0x001f0000;   //Odczyt z joysticka
    while(1)
    {
        enum MENU_STATE res;
        if (IOPIN & J_UP)
        {
            res = NEW_GAME;
        }
        else if(~IOPIN & J_LEFT)
        {
            res = ROUND_TIME;
        }
        else if(~IOPIN & J_RIGHT)
        {
            res = ROUNDS_AMOUNT;
        }
        else if(~IOPIN & J_DOWN)
        {
            res = LIVES_AMOUNT;
        } else
        {}
        return res;
    }
}

/*!
*  @brief    Glowna rozgrywka
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void newGame(void)
{
    tU32 j_position = 0;             /**< Losowanie pozycji joysticka */
    //Rozpoczenie gry
    const char* p1 ="Wybrano nowa gre.\n";
    const char* p2 ="Wybrano";
    const char* p3 = "Nowa Gra";
    writeInfo(p1, p2, p3);
    osSleep(200);
    testMotor();
    //Parametry rozgrywki
    (void)printf("Ilosc rund: %d.\nCzas rundy: %d ms\nIlosc zyc: %d\n", round_amount, round_time, lives);
    const char* messagev1 = " Rundy:    ";
    const char* emptyStr = "";
    writeInfo(emptyStr, insertdNumber(messagev1, 9, round_amount), emptyStr);
    osSleep(200);
    const char* messagev2 = " Czas:      ";
    writeInfo(emptyStr, insertdNumber(messagev2, 10, round_time), emptyStr);
    osSleep(200);
    const char* messagev3 = " Zycia:    ";
    writeInfo(emptyStr, insertdNumber(messagev3, 9, lives), emptyStr);
    osSleep(200);

    //Wyczyszczenie zmiennych rundy
    static tU32 max_answer_time = 0;		/**< Najdluzszyszy czas rekacji gracza */
    tU32 min_answer_time = 30000;	        /**< Najkrotszy czas rekacji gracza */
    long int entire_answer_time = 0;		/**< Sredni czas rekacji gracza */
    tU8 tmpLives=lives;

    tU32 start_time;    //Zmienna odpowiadajaca za czas rozpoczEcia rundy

    IODIR &= ~0x001f0000;   //Odczyt z joysticka

    //Zgadywanie strzalki przez wybrana ilosc rund.
    tU8 i;
    for(i=0; i<round_amount; i++)
    {
        j_position = time_ticks%4;    //Losowanie strzalki
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
        const char* message = "Runda:    ";
        const char* p4 = "Uzyj joystick w kierunku wskazanym przez strzalke.\n";
        const char* p5 = "Wybor-joystick.";
        writeInfo(p4, p5, insertdNumber(message, 9, i+1));

        //Czas rozpoczecia rundy - czas od ktoego liczony jest czas na reakcje
        start_time = time_ticks;

        tU32 result = 0;
        tU32 correct = 0;
        tU32 prevIOPIN;
        tU32 answer;

        //Petla reakcji na strzalke
        while(1)
        {
            prevIOPIN = IOPIN;
            osSleep(5);

            answer = (prevIOPIN & ~IOPIN) & 0x001f0000; //Porownanie, czy joystick sie zmienil

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

            //Ograniczenie reakcji do zadeklarowanego w menu czasu na odpowiedz w danej rundzie
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

            testRGB(0, 255, 0); //Poprawna odpowiedz
            osSleep(50);
            testRGB(0, 0, 0);
            //testMotor();    //Famfary za poprawna odpowiedz


            //Informacje dla gracza
            (void)printf("Odpowiedz byla poprawna, a szybkosc wystarczajaca\nPozostala liczba rund: %d.\nTwoj czas zareagowania: %d ms.\n", round_amount, result);
            const char* message = "Czas:     ";
            const char* p6 = "";
            const char* p7 = "Poprawnie";
            writeInfo(p6, p7, insertdNumber(message, 9, result));
            osSleep(300);
        }
        else if(correct == (tU32) -1)
        {
            //Dla za poznego ruchu
            testRGB(255, 255, 0);
            osSleep(50);
            testRGB(0, 0, 0);
            //testMotor();
            const char* p6 = "Nie zdazyles w ustalonym czasie. Tracisz zycie.\n";
            const char* p7 = "Brak czasu.";
            const char* p8 = "Straciles zycie.";
            writeInfo(p6, p7, p8);
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
            const char* p9 = "Wybrales zly kierunek.Tracisz zycie\n";
            const char* p10 = "Zly kierunek";
            const char* p11 ="Strata zycia";
            writeInfo(p9, p10, p11);
            tmpLives--;
            osSleep(300);
        }

        if(tmpLives< (tU8) 1)
        {
            //Gdy skonczyla sie liczba zyc
            testRGB(255, 255, 255);
            const char* p12 = "Straciles wszystkie zycia. Przegrales.\n";
            const char* p13 = "Koniec zyc.";
            const char* p14 = "Koniec gry.";
            writeInfo(p12, p13, p14);
            osSleep(300);

            //Wyczyszczenie zmiennych rozgrywki
            max_answer_time = 0;		/**< Najdluzszy czas rekacji gracza */
            min_answer_time = 32767;	/**< Najkrotszy czas rekacji gracza */
            entire_answer_time = 0;		/**< Sredni czas rekacji gracza */

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
    const char* p15 ="Gra sie zakonczyla.\n";
    const char* p16 = "Koniec Gry";
    const char* p17 = "";
    writeInfo(p15, p16, p17);
    pause();

    //Podsumowanie
    const char* p18 = "";
    const char* p19 = "Gratulacje";
    writeInfo(p18, p19, p18);
    pause();
    (void)printf("Najpozniejsza reakcja, najszybsza reakcja: %d, %d\n", max_answer_time, min_answer_time);
    const char* message3 = "Najw.:     ", message4[] = "Najm.:     ";
    const char* p20 = "";
    writeInfo(p20, insertdNumber(message3, 10, max_answer_time), insertdNumber(message4, 10, min_answer_time));
    osSleep(500);

    clearLCD();
    testArrow('c');
    testRGB(0,0,0);     //Wyczyszczenie diody RGB.


}

/*!
*  @brief    Ustawienie ilosci rund.
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void roundsAmount(void)
{
    //Informacjie o interakcji
	const char* p1 = "Menu wyboru liczby rund.\nSterowanie odbywa sie za pomoca joysticka";
	const char* p2 = "Liczba rund";
	const char* p3 = "Wybor-joystick";
    writeInfo(p1, p2, p3);
    pause();
    const char* p4 = "1-Zwieksz o jedna runde, 4 zmniejsz o jedna runde, 2,3,5-wyjdz\n";
    const char* p5 = "1-Zwieksz";
    const char* p6 = "4-Zmniejsz";
    writeInfo(p4, p5, p6);
    pause();
    tU8 end = 0;
    tU8 change = 0;

    IODIR &= ~0x001f0000;   //Odczyt z joystick'a
    //Wybor ilosci rund
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
        //Wyswietlenie zmiany
        if(change == (tU8) 1)
        {
            (void)printf("Rundy: %d.\n", round_time);
            const char* message = "   ";
            const char* p7 = "Rundy:";
            const char* p8 = "";
            writeInfo(p8, p7, insertdNumber(message, 2, round_amount));
            change = 0;
            osSleep(100);
        }
    }

    //Informacja o ostatecznej zmianie
    (void)printf("Rundy: %d.\nNacisnij przycisk.\n", round_amount);
    const char* message = " Rundy:    ";
    const char* p9 = "";
    writeInfo(p9, insertdNumber(message, 9, round_amount), p9);
    pause();

}

/*!
*  @brief    Zmiana czasu na reakcje.
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void roundsTime(void)
{
    //Informacje o interakcji
	const char* p1 = "Menu wyboru ilosci  czasu.\nSterowanie odbywa sie za pomoca joysticka";
	const char* p2 = "Ilosc czasu";
	const char* p3 = "Wybor-joystic";
    writeInfo(p1, p2, p3);
    pause();
    const char* p4 = "1-Zwieksz o sekunde, 4 zmniejsz o sekunde, 2,3,5-wyjdz\n";
    const char* p5 = "1-Zwieksz";
    const char* p6 = "4-Zmniejsz";
    writeInfo(p4, p5, p6);
    pause();
    tU8 end = 0;
    tU8 change = 0;

    IODIR &= ~0x001f0000;   //Odczyt z joystick'a
    //Petla zmiany czasu na reakcje
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

        //Wyswietlenie zmiany
        if(change == (tU8) 1)
        {
            (void)printf("Czas: %d.\n", round_time);
            const char* message = "     ";
            const char* p7 = "Czas:";
            const char* p8 = "";
            writeInfo(p8, p7, insertdNumber(message, 4, round_time));
            change = 0;
            osSleep(10);
        }
    }

    //Ostateczny stan.
    (void)printf("Czas: %d.\nNacisnij przycisk.\n", round_time);
    const char* message = " Czas:      ";
    const char* p9 = "";
    writeInfo(p9, insertdNumber(message, 10, round_time), p9);
    pause();
}

/*!
*  @brief    Zmiana liczby zyc
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
static void livesAmount(void)
{
    //Informacjie o interakcji
	const char* p1 = "Menu wyboru ilosci zyc.\nSterowanie odbywa sie za pomoca joysticka";
	const char* p2 = "Ilosc zyc";
	const char* p3 = "Wybor-joystic";
    writeInfo(p1, p2, p3);
    pause();
    const char* p4 = "1-Zwieksz o sekunde, 4 zmniejsz o sekunde, 2,3,5-wyjdz\n";
    const char* p5 = "1-Zwieksz";
    const char* p6 = "4-Zmniejsz";
    writeInfo(p4, p5, p6);
    pause();
    tU8 end = 0;
    tU8 change = 0;

    IODIR &= ~0x001f0000;   //Odczyt z joystick'a
    //Wybor ilosci rund
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

        //Wyswietlenie zmiany
        if(change == (tU8) 1)
        {
            (void)printf("Zycia: %d.\n", lives);
            const char* message = "   ";
            const char* p7 = "";
            const char* p8 = "Zycia:";
            writeInfo(p7, p8, insertdNumber(message, 2, lives));
            change = 0;
            osSleep(100);
        }
    }

    //Informacja o ostatecznej zmienie
    (void)printf("Zycia: %d.\nNacisnij przycisk\n", lives);
    const char* message = " Zycia:    ";
    const char* emptyStr = "";
    writeInfo(emptyStr, insertdNumber(message, 9, lives), emptyStr);
    pause();

}

/*!
*  @brief    Wypisuje informacje na konsole� i na LCD.
*  @param console_message
*             Wiadomosc na konsole�
*  @param lcd_1line
*             Wiadomosc do pierwszego wiersza LCD.
*  @param lcd_2line
*             Wiadomosc do drugiego wiersza LCD.
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
    (void)printf(console_message);
}

/*!
*  @brief    Wstawiam do konkretnej wiadomosci podany numer na podana pozycje.
*  @param message
*             Wiadomosci do ktorej wstawiamy numer
*  @param position
*             Pozycja na ktorej wstawiamy numer.
*  @param number
*             Wstawiany numer
*  @returns  Zmieniona wiadomosc
*  @side effects:
*            brak
*/
char* insertdNumber(char* message, int position, int number)
{
    //Zabezpiecz przypadek 0
    int positionInFunction = position;
    int numberInFunction = number;
    positionInFunction = positionInFunction - (int) 1;
    message[positionInFunction] = '0' + (numberInFunction % 10);
    numberInFunction /= 10;

    while(numberInFunction==1)
    {
        positionInFunction = positionInFunction - (int) 1;
        message[positionInFunction] = '0' + (numberInFunction % 10);
        numberInFunction /= 10;
    }
    return message;
}

/*!
*  @brief    Pauzuje gre do czasu nacisniecia P0.14
*  @param brak
*  @returns  brak
*  @side effects:
*            brak
*/
void pause(void)
{
    IODIR &= ~0x00004000;			//odczytuje P0.14
    while(1) {
        if (!(IOPIN & 0x00004000))
        {
            break;
        }
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
