#include "Board_Buttons.h"              // ::Board Support:Buttons
#include "LPC17xx.h"                    // Device header
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "Open1768_LCD.h"
#include "LCD_ILI9325.h"
#include "asciiLib.h"
#include "TP_Open1768.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Driver_USART.h"
#include "notes.h"
#include <stdint.h>
#include <stddef.h>
#include "at45db041x.h"
#include "spi_master.h"
#include "Driver_SPI.h"
#define maxLen 4656
#define maxPageNotes 48


extern ARM_DRIVER_SPI Driver_SPI2; // deklaracja sterownika do interfejsu SPI
// liczniki systemowe SysTick
volatile int msTicks = 0;
volatile int msTicks2 = 0;

short isPlayed = 0;


// wypisywanie lancucha znakow na UART
void print(const char* a);
// przerwanie pochodzace od zegara systemowego
void SysTick_Handler(void);
// odczekanie czasu t, liczonego w milisekundach
void delay(int t);
// przerwanie odpowiadajace czestotliwosci freq.
void f(int freq);
// funkcja przekazuje strukture nuta (opisana w notes.h) oraz tempo danego utworu (liczba cwiercnut na minute)
void playnote1(Note note, int tempo);
void rysuj(char literka, uint16_t x, uint16_t y);
void rysujprostokat( uint16_t x, uint16_t y,uint16_t xx, uint16_t yy,uint16_t color);
//zapisywanie utworu o okreslonym id, id <= 20
int save_utwor(int id);

//funkcja odgrywa utwor o okreslonym id z pamieci flash
void play_utwor(int id);

void rysuj_tekst(const char*, int, int);

int check_to_save_utwor();
int check_to_play_utwor();
int is_utwor_present(int id);
void rysuj_tytul(int);

int main(void){
	ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;
	uint32_t retC = SysTick_Config(SystemCoreClock/1000000); //Config ustawiony na milisekundy
    //inicjalizacja LCD
    Joystick_Initialize();
	lcdConfiguration();
	init_ILI9325();
    //konfigurowanie PINow do UART0
	PIN_Configure(0, 2, 1, 0, 0);
	PIN_Configure(0, 3, 1, 0, 0);
	PIN_Configure(0, 26, 2, 2, 0);


	//ustawienie odpowiednich dzielników
	LPC_UART0->LCR = 3 |(1<<7);
	LPC_UART0->DLM = 0;
	LPC_UART0->DLL = 27;
	LPC_UART0->LCR = 3;


    //Inicjalizacja sterownika do SPI
    SPIdrv->Initialize(NULL);
    //Dostarczenie zasilania do peryferium (SPI)
    SPIdrv->PowerControl(ARM_POWER_FULL);
    //Konfiguracja SPI: Master, (1,1), Most-Significant-Bit-First, 8-bit, 1MBit/s
    SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_SW | ARM_SPI_DATA_BITS(8), 1000000);
    //SS line = INACTIVE = HIGH
    SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
    //Sprawdzenie, czy flash istnieje
	print("Sprawdzanie pamieci flash...\r\n");
	if(flash_check_present()) {
		print("Flash OK\r\n");
	};
		int registerStatus=lcdReadReg(OSCIL_ON);
	lcdWriteReg(ENTRYM, 0x1030);
	lcdWriteReg(ADRX_RAM,  0);
	lcdWriteReg(ADRY_RAM,  0);

	while(1){
		int flag_save = 0;

	rysujprostokat(0,0,320,240,LCDBlack);

	const char* UI_txt1 = "Nutowy odtwarzacz dzwieku";
	rysuj_tekst(UI_txt1,144,64);
	const char* UI_txt4 = "Uzyj joysticka do wybierania opcji:";
		rysuj_tekst(UI_txt4,160,24);
	const char* UI_txt2 = "<- Tryb zwykly";
	rysuj_tekst(UI_txt2,250,8);
	const char* UI_txt3 = "Tryb serwis ->";
	rysuj_tekst(UI_txt3,250,198);
		rysujprostokat(0,0,8,240,LCDBlack);

	while(1){
        switch(Joystick_GetState()){
            case(JOYSTICK_RIGHT): {
                        ///MENU NR 2B: SERWIS
                const char * UI_serwis_1 = "Tryb serwisowy";
                rysujprostokat(0,90,320,40,LCDBlack);
                rysuj_tekst(UI_serwis_1,144,108);
                const char* UI_serwis_2 = "<- Wyczysc pamiec";
								rysujprostokat(0,8,320,20,LCDBlack);
                rysuj_tekst(UI_serwis_2, 238, 8);
                const char* UI_serwis_3 = "Zapisz utwor ->";
                rysuj_tekst(UI_serwis_3, 226, 180);
							  const char* UI_serwis_4 = "(wymagany UART)";
                rysuj_tekst(UI_serwis_4, 250, 184);
								char krak [70];
                while(1){

                    switch(Joystick_GetState()){

                        case(JOYSTICK_RIGHT): {

                            int id = check_to_save_utwor();
														if (id == -1 || !flash_check_present()){
															rysuj_tekst("BLAD PAMIECI FLASH LUB PAMIEC PELNA", 144, 16);
															rysuj_tekst("       ZRESTARTUJ URZADZENIE       ", 160, 16);
															while(1);
														}

                            sprintf(krak, "Zapisywanie utworu, id = %d",id);
                            rysujprostokat(0,0,320,100,LCDBlack);
                            rysuj_tekst(krak, 250, 8);
                            print(krak);
                            save_utwor(id);
														rysujprostokat(0,0,320,100,LCDBlack);
                            memset(krak, NULL, 40);
                            sprintf(krak, "Utwor zapisano.");
														flag_save = 1;
                            rysuj_tekst(krak, 250, 8);
														memset(krak, NULL, 40);
                            sprintf(krak, "Joystick w prawo by zapisac kolejny ->");
                            rysuj_tekst(krak, 234, 8);
														memset(krak, NULL, 40);
                            sprintf(krak, "RESET by wrocic do normalnego trybu");
                            rysuj_tekst(krak, 218, 8);
                            //play_utwor(id);
                            break;
													}
                        case(JOYSTICK_LEFT): {
														if(!flag_save) {
                            rysujprostokat(0,0,320,240,LCDBlack);
                            rysuj_tekst("Czyszczenie pamieci flash...", 250, 8);
                            flash_erase_all();
														int k = -1;
														char buf_k[9];
														sprintf(buf_k,"%d",k);
														for(int id = 0; id < 20; id++){
															flash_write_page(id*100+3,(uint8_t *)(buf_k), sizeof(int),1);

														}
                            rysuj_tekst("Czyszczenie OK. Zrestartuj urzadzenie.", 226, 8);
													}
                            break;
                    }
										break;
                }

							}
            case (JOYSTICK_LEFT): {
							rysujprostokat(0,90,320,40,LCDBlack);
							rysujprostokat(0,8,320,20,LCDBlack);
                            ///MENU NR 2A: ZWYKLY
                int k = -1;
                k = check_to_play_utwor();

								if(k == -1) {
										rysuj_tekst("Brak utworow w pamieci.", 144, 70);
										rysuj_tekst("Przejdz do trybu serwisowego ->", 160, 40);
										break;
								}
                if (k != -1){
                    rysuj_tytul(k);
                }
                if (k != 0){
                    rysuj_tekst("<- Poprzedni utwor", 240, 8);

                }
                rysuj_tekst("Nastepny utwor ->", 240, 180);
                rysuj_tekst("^ Zagraj", 180, 128);
                    while(1){
                        switch(Joystick_GetState()){
                        case(JOYSTICK_UP): {
													rysujprostokat(0,15,320,30,LCDBlack);
														rysuj_tytul(k);
                            rysuj_tekst("Odtwarzanie...", 180, 104);
													if(flash_check_present())
															play_utwor(k);
                            print("UTWOR ZAGRANO.\r\n");
														                if (k != -1){
																rysuj_tytul(k);
														}
														rysujprostokat(100,70,140,30,LCDBlack);
														if (k != 0){
																rysuj_tekst("<- Poprzedni utwor", 240, 8);

														}
														rysuj_tekst("Nastepny utwor ->", 240, 180);
														rysuj_tekst("^ Zagraj", 180, 128);
                            break;
												}
                        case(JOYSTICK_RIGHT): {
														rysujprostokat(0,8,320,32,LCDBlack);
                            if(k != 19 && is_utwor_present(k+1)){
                                k++;
                                rysuj_tytul(k);
                                rysuj_tekst("<- Poprzedni utwor", 240, 8);
                                rysuj_tekst("Nastepny utwor ->", 240, 180);
                                rysuj_tekst("^ Zagraj", 180, 128);
                                break;// powrot wyzej, tylko z innym id xd
                            }
                            else{
                                k = 0;
                                rysuj_tytul(k);
                                rysuj_tekst("Nastepny utwor ->", 240, 180);
                                rysuj_tekst("^ Zagraj", 180, 128);
                                continue;
                            } //powrot wyzej, z id = 0
													}
                        case(JOYSTICK_LEFT): {
                            if(k != 0 && is_utwor_present(k-1)){
															rysujprostokat(0,8,320,32,LCDBlack);
                                k--;
                                rysuj_tytul(k);
                                if(k != 0)
                                rysuj_tekst("<- Poprzedni utwor", 240, 8);
                                rysuj_tekst("Nastepny utwor ->", 240, 180);
                                rysuj_tekst("^ Zagraj", 180, 128);
                            }
                            break;
													}
    }
}
}
}



    //Jesli chcemy wyczyscic pamiec flash:
	//print("Erasing...\n");
	//flash_erase_all();
	//print("\rErasing OK\r\n");
}
}
}
	return 0;
}

void rysuj_tytul(int k){
    if (k >= 0 && k < 20){
    char krak[16];
    sprintf(krak,"Utwor numer %d",k);
    rysujprostokat(0,100,320,40,LCDBlack);
		rysujprostokat(195,135,20,20,LCDBlack);
    rysuj_tekst(krak, 120, 108);
    char buffer_name[64];
		char buffer2_name[64];
    memset(buffer_name, 0, sizeof(buffer_name)); //zerujemy bufor
		memset(buffer2_name, 0, sizeof(buffer2_name));
    flash_read_page(k*100 + 1, (uint8_t*)buffer_name, 64); //odczytujemy do bufora strone z tytulem
    //sscanf(buffer2_name, "%[^\n]", buffer_name); //parsowanie znakow w buffer_name
		strcpy(buffer2_name, buffer_name);
			int d = strlen(buffer2_name);
    rysuj_tekst(buffer2_name, 144, 8);
    }
}

int is_utwor_present(int id){
    char buffer_lenU [9]; //chcemy odczytac dlugosc utworu
	int lenU = 0;
	flash_read_page(id*100 + 3, (uint8_t*)buffer_lenU, sizeof(int));
	sscanf(buffer_lenU, "%d", &lenU); //zapisujemy do zmiennej lenU
	if (lenU >= 1 && lenU <= maxLen)
        return 1;
    else return 0;
}

int check_to_play_utwor(){
    char buffer_lenU [9]; //chcemy odczytac dlugosc utworu
	int lenU = 0;
	int id;
	for (id = 0; id < 20; id++){
	flash_read_page(id*100 + 3, (uint8_t*)buffer_lenU, sizeof(int));
	sscanf(buffer_lenU, "%d", &lenU); //zapisujemy do zmiennej lenU
	if (lenU >= 1 && lenU <= maxLen)
        return id;
}
    return -1;
}



int check_to_save_utwor(){
    char buffer_lenU [9]; //chcemy odczytac dlugosc utworu
	memset(buffer_lenU,NULL,9);
	int lenU = 0;
	int id;
	for (id = 0; id <= 20; id++){
	flash_read_page(id*100 + 3, (uint8_t*)buffer_lenU, sizeof(int));
	sscanf(buffer_lenU, "%d", &lenU); //zapisujemy do zmiennej lenU
	if (lenU >= 1 && lenU <= maxLen)
        continue;
	if (id == 20)
		return -1;
return id;//zwracamy pierwsze "wolne" id
	}
	return -1;
}

void rysuj_tekst(const char* tekst, int x, int y){
	do{
		rysuj(*tekst, x, y);
		y += 8;
	}
	while((*++tekst) != '\0');
}

void rysujprostokat( uint16_t x, uint16_t y,uint16_t xx, uint16_t yy,uint16_t color){
	for(uint16_t j=x;j<(x+xx);j++)
	{
		for(uint16_t i=(y);i<(y+yy);i++){

				lcdWriteReg(ADRX_RAM,  i);
				lcdWriteReg(ADRY_RAM,  j);
				lcdWriteReg(DATA_RAM,color);
		}


	}
}

void rysuj(char literka, uint16_t x, uint16_t y){
	for(uint16_t j=x;j<(x+16);j++){
		for(uint16_t i=(y);i<(y+8);i++){
			unsigned char buffer[16];
			GetASCIICode(1,buffer,literka);
			if(*(buffer+j-x) & (1<<abs(8-(i-y)))){
				lcdWriteReg(ADRX_RAM,  16-j);
				lcdWriteReg(ADRY_RAM,  i);
				lcdWriteReg(DATA_RAM,LCDWhite);
			}
			else{
				lcdWriteReg(ADRX_RAM,  16-j);
				lcdWriteReg(ADRY_RAM,  i);
				lcdWriteReg(DATA_RAM,LCDBlack);
			}
		}
	}
}


void print(const char* a)  {
	do {
	while(!(LPC_UART0->LSR & (1 << 5)));
	LPC_UART0->THR = *a;
	} while((*++a)!='\0');
}
void SysTick_Handler(void) {
	msTicks++;
	msTicks2++;
}

void delay(int t) {
	msTicks = 0;
	while(t>msTicks);
}

void f(int freq) {
	float t = 1./freq;
	delay(1000000*t);
}


void playnote1(Note note, int tempo) {
    //zmienne pomocnicze
    float freq = note.freq;
    float duration = (float)note.duration;
    float bps = tempo/60.; //z nut na minute przechodzimy na nuty na sekunde
    float dv = 1000000*4.0*(1/bps)/duration; //zmienna pomocnicza, konwertujaca dlugosc trwania nuty na milisekundy, z uwzglednieniem tempa
    msTicks2 = 0;   // konieczna jest osobna zmienna, by moc uwzglednic dlugosc trwania nuty
    do{
        LPC_DAC->DACR = (500 << 6); //natezenie sygnalu ustawione na sztywno
        f(freq); // ta funkcja wykorzystuje msTicks
        LPC_DAC->DACR = (0 << 6);
        f(freq);
		}
        while (dv > msTicks2);
    delay(dv * .065); //bardzo krótka przerwa, by dalo sie uslyszec koniec trwania nuty
    return;
}

int save_utwor(int id){
	char* wsk; // wskaznik pomocniczy
	char name[64]; //bufor nazwy, max 63 znaki
	memset(name, NULL, 64);
    //chcialbym tu data memset(name, NULL, 64);
	wsk = name;

	print("Zapisywanie utworu...\r\nNazwa utworu:...\r\n");

	//zapisywanie znak po znaku do bufora
	do{
        while (!(LPC_UART0-> LSR & (1<<0)));
        *wsk = LPC_UART0->RBR;
        print(wsk); // odczytuj¹c z klawiatury, wypisujemy na ekran to, co wczytaliœmy
	} while((*wsk++)!= 13); //13 jest kodem ascii dla ENTER-a; po porównaniu wskaznik przesuwa siê dalej
	*--wsk = '\0'; //cofamy wskaznik za ostatni¹ literê i dopisujemy \0, wtedy w tablicy name bêdzie pe³noprawny ³añcuch znaków


	//zapisywanie do pamieci flash tytulu na strone numer id*100 + 1
	flash_write_page(id*100 + 1, (uint8_t*)(name), strlen(name)+1, 1);

    //zapisywanie tempa
	print("\r\nPodaj tempo:\r\n");
	char buffer[5];
	memset(buffer,NULL,5);
	char *wsk_buff = buffer; //wskaznik pomocniczy
	do{
        while (!(LPC_UART0-> LSR & (1<<0)));
        *wsk_buff = LPC_UART0->RBR;
		print(wsk_buff);
	} while((*wsk_buff++)!= 13); //warunek taki sam jak wczesniej
	*--wsk_buff = '\0';

	//tempo jest zapisywane na strone id*100 + 2
	flash_write_page(id*100 + 2, (uint8_t*)(buffer), strlen(buffer), 1);

    //zapisywanie utworu
	char musicData[5*48]; //zakladamy 32 nuty na stronê
	char *mData = musicData; //wskaznik pomocniczy
	int counter = 0;        //ten licznik pomoze nam przesledzic, ile nut wpisalismy (kazda nuta obejmuje 5 znaków)
	int noteNumber = 0;     //ten licznik wskaze nam liczbe nut w calym utworze
	int pageIndex = 4;      //zapis rozpoczynamy od strony id*100 + 4
	print("\r\nWpisuj nuty...\r\n");
	//glowna petla zapisu

	do{
        //jesli osiagniemy 48 nut...
        if(counter == (maxPageNotes*5)) {
            flash_write_page(id*100 + pageIndex, (uint8_t*)(musicData), maxPageNotes*5, 1); //...to tyle ich zapisujemy na stronie
            pageIndex++;
            counter = 0;
            mData = musicData; // ustawiamy wskaznik na poczatek bufora
            memset(musicData,NULL,maxPageNotes*5); //zerujemy bufor
        }
        //wpisywanie nut
        while (!(LPC_UART0-> LSR & (1<<0)));
        *mData = LPC_UART0->RBR;
        counter++;
        noteNumber++;
        print(mData); //wypisywanie tego, co wpisalismy
	} while((*mData++)!= 13); //az do znaku ENTER
	*--mData = '\0'; //\0 jest znakiem konczacym utwór

	noteNumber /= 5; //tak otrzymamy liczbe nut
	char buf_noteNumber[9];
	sprintf(buf_noteNumber,"%d",noteNumber);
	flash_write_page(id*100 + pageIndex, (uint8_t*)(musicData), maxPageNotes*5, 1); //na koniec zapisujemy na stronie reszte bufora.
	flash_write_page(id*100 + 3, (uint8_t*)(buf_noteNumber), sizeof(int), 1); // wpisujemy liczbe nut w utworze na stronie id*100 + 3
	print("\r\nNuty zapisane.\r\n");
	return 1; //oznaka pomyslnosci zapisu
}

void play_utwor(int id) {

	char buffer_name[64];
	char buffer2_name[64];
	memset(buffer_name, 0, sizeof(buffer_name)); //zerujemy bufor
	memset(buffer2_name,0,sizeof(buffer2_name));
	flash_read_page(id*100 + 1, (uint8_t*)buffer_name, 64); //odczytujemy do bufora strone z tytulem

	sscanf(buffer2_name, "%[^\n]", buffer_name); //parsowanie znaków w buffer_name
	print("Utwor: ");
	print(buffer2_name); //wypisuje nazwe utworu
	print("\r\n");

	char buffer_tempo [9]; //odczytujac z pamieci flash, potrzebujemy wskaznika na char
	int tempo;
	flash_read_page(id*100 + 2, (uint8_t*)buffer_tempo, sizeof(int)); //sczytuje 4 bajty
	sscanf(buffer_tempo, "%d", &tempo); //parsuje odczytane bajty do odpowiedniego typu calkowitego (tempo)
	char bukf[9];
	sprintf(bukf, "%d",tempo); //chcemy wypisac tempo na UART
	print("Tempo: ");
	print(bukf); //wypisuje tempo utworu
	print("\r\n");

	char buffer_lenU [9]; //chcemy odczytac dlugosc utworu
	int lenU;
	flash_read_page(id*100 + 3, (uint8_t*)buffer_lenU, sizeof(int));
	sscanf(buffer_lenU, "%d", &lenU); //zapisujemy do zmiennej lenU
	//jesli chcemy wypisac dlugosc utworu:
	char buff2[9];
	sprintf(buff2, "%d",lenU);
	print(buff2);

    //glowna petla odpowiadajaca za granie utworu
	for(int i = 0; i < ((lenU - 1) / maxPageNotes) + 1; i++) { //dzielenie calkowite -> z i-tej strony zostanie odczytane do 32 nut.
		char buf[20];
		sprintf(buf, "Strona numer %d", i);
		print(buf);
		char musicData[5*maxPageNotes];
		// chcialbym tu dac memset(musicData, NULL, 240)
		char *msc_ptr = musicData;
        //nuty odczytywane z i-tej strony, i = 0,1,...,n
		flash_read_page(id*100 + 4 + i, (uint8_t*)musicData, 5*maxPageNotes);
        //odgrywanie 48 nut
		for(int j = 0; j < maxPageNotes; j++) {
			char noteData[5];
			memcpy(noteData, msc_ptr, 5);
			Note nuta = getNote(msc_ptr); //getNote sczytuje 5 znaków
			playnote1(nuta, tempo); // wlasciwa czesc odegrania
			msc_ptr += 5;
			if (*(msc_ptr-1) == '\0'){ //oznacza koniec utworu, ew. koniec strony
				sprintf(buf, "%d",((lenU-1)/maxPageNotes)+1);
				print(buf);
				return;

			}
		}
	}

}
