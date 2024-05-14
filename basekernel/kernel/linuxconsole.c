*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "keyboard_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <ncurses.h>
#include <unistd.h>  
#include <time.h>  
#include <ctype.h> 
 




/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define RAND_MAXX 32767
#define ENTER_KEY_CODE 0x1C

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* current cursor location */
unsigned int current_loc = 0;
unsigned long next = 1;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000; //25x 80



struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

typedef enum{
	YON_SOL =1,
	YON_SAG =2,
	YON_YUKARI =3,
	YON_ASAGI=4
}YON;
typedef struct{
	int x;
	int y;
	YON yon;
	char karakter;
}Yilan;


typedef struct {
	int x;
	int y;
	char karakter;
}Yem;
typedef struct {
	time_t tv_sec;
	long   tv_nsec;
}timespec;

int genislik = 80, yukseklik = 20;
const int maxYilan = 500;
int kuyrukUzunlugu =0 ;
int yemboyutu = 0;
int yems = 1;
int yemX[80];
int yemY[20];
char sahne[80][20];
char letter[5][7];
char tuslar[256];
char yilanKarakteri = 219;
char yemKarakteri = 219;
Yilan yilanKuyruk[500];
Yem yemler;
int score = 0;
bool boole= {true};
int art = 25;
int cart = 50;

int str_len(char* string) 
{ 
    // variable to store the 
    // length of the string 
    int len = 0; 
    while (*string != '\0') { 
        len++; 
        string++; 
    } 
  
    return len; 
}

int rand(){
next = next * 1103515245 + 12345;
return((((unsigned int)(next/65536) % RAND_MAXX + 1) / 10) / 10) /10 + 21  ;
}
void srand(unsigned int seed){
	next = seed;
}
void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{	
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}
void kprintint(int sayi){
char yuz[2],on[3],bir[2];
int cx = 0;
int i;
on[0] = 32;
on[1] = 32;
on[2] = 32;

	if(sayi > 0){
	while(sayi > 0){
	int says = sayi % 10;
	int bb = '0' + says;
	on[cx] = bb;
	yuz[0] = bb;
	yuz[1] = 0;
	//kprint(yuz);
	sayi = sayi / 10;
	cx = cx + 1;
	i = cx;
	
	}
	for(i=cx ; i>=0; i--){
	char alan[2];
		alan[0] = on[i];
		alan[1] = 0;
		
		kprint(alan);
		
	
	}
	}
	else if(sayi == 0){
	int bb = '0' + sayi;
	yuz[0] = bb;
	yuz[1] = 0;
	kprint(yuz);
	}
	
	
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x05;
	}
}
void gotoxy(int x, int y){
current_loc = 2 * ( x * 80 + y);

}
void ekranaCiz() {
	int y = 0,x =0;
	
	for (y = 0; y < yukseklik;y++) {

		for (x = 0; x < genislik; x++) {
			char alan[2];
		alan[0] = sahne[x][y];
		alan[1] = 0;
		gotoxy(y,x);
		kprint(alan);
		}
		gotoxy(y,0);
	}
}
void temizle() {
	int y,x;
	for (y = 0; y < yukseklik;y++) {

		for (x = 0; x < genislik; x++) {
			sahne[x][y] = ' ';
			
		}
		
	}
}
void sinirlar() {
	//printf("sinirlar1\n");
	int x,y;
	//printf("%d \n",x);
	for (x = 0; x < 80; x++) { 
		sahne[x][0] = 219; //üst
		sahne[x][yukseklik - 1] = 219; //alt
		//printf("basla3\n");
	}
	for (y = 0; y < 20; y++) {
		sahne[0][y] = 219; //sol
		sahne[genislik - 1][y] = 219; //sag
		//printf("basla4 \n");
	}
	//printf("%d \n",y);
}
void duvarkontrol(){
	if(yilanKuyruk[0].x == 0 || yilanKuyruk[0].y == 0 || yilanKuyruk[0].x == 80 || yilanKuyruk[0].y == 25){
		
		
		gotoxy(0,0);
		kprint("Game Over " );
			kprint("Your Score is Here : ");
			kprintint(score-1);
			
			boole = false;
	}
}
void yilanuzerindemi(){
	int i;
	for(i = maxYilan-1 ; i>0;i--){
		if(yilanKuyruk[0].x == yilanKuyruk[i].x && yilanKuyruk[0].y == yilanKuyruk[i].y){
			
			
			gotoxy(0,0);
			kprint("Game Over " );
			kprint("Your Score is Here : ");
			kprintint(score-1);
			
			boole= false;
		}
	}
}
void yilanhareketettir(){
	int i;
	for(i = 0; i<kuyrukUzunlugu;i++){
		switch(yilanKuyruk[i].yon){
			case YON_SAG:
				yilanKuyruk[i].x++;
				break;
			case YON_SOL:
				yilanKuyruk[i].x--;
				break;
			case YON_ASAGI:
				yilanKuyruk[i].y++;
				break;
			case YON_YUKARI:
				yilanKuyruk[i].y--;
				break;
		}
	
	}	for(i = kuyrukUzunlugu - 1; i> 0 ;i--){
		yilanKuyruk[i].yon = yilanKuyruk[i-1].yon;
	}
}
void yilanKuyrugunaEkle(){
	if(kuyrukUzunlugu == maxYilan)
		return;
	
	int x  =yilanKuyruk[kuyrukUzunlugu-1].x;
	int y  =yilanKuyruk[kuyrukUzunlugu-1].y;
	YON yon = yilanKuyruk[kuyrukUzunlugu-1].yon;
	char kar = yilanKuyruk[kuyrukUzunlugu-1].karakter;
	switch(yilanKuyruk[kuyrukUzunlugu-1].yon){
		case YON_SAG:
				x--;
				break;
			case YON_SOL:
				x++;
				break;
			case YON_ASAGI:
				y--;
				break;
			case YON_YUKARI:
				y++;
				break;
	}
	yilanKuyruk[kuyrukUzunlugu].x =x;
		yilanKuyruk[kuyrukUzunlugu].y =y;
			yilanKuyruk[kuyrukUzunlugu].yon =yon;
				yilanKuyruk[kuyrukUzunlugu].karakter =kar;
				kuyrukUzunlugu++;
}
void baslangic(){
	kuyrukUzunlugu = 1;
	yilanKuyruk[0].x = 20;
	yilanKuyruk[0].y = 10;
	yilanKuyruk[0].yon = YON_SAG;
	yilanKuyruk[0].karakter = yilanKarakteri;
	yilanKuyrugunaEkle();
	yilanKuyrugunaEkle();
	yilanKuyrugunaEkle();
}
void baslangicyilanigor(){
	int i;
	for(i = 0;i< kuyrukUzunlugu;i++){
		int x  =yilanKuyruk[i].x;
		int y  =yilanKuyruk[i].y;
		sahne[x][y] = yilanKuyruk[i].karakter;
		
	}
}
int yemCoord(){
	for(int i = 0; i < 80 ; i++){
	yemX[i] = i + 1;
	}
	for(int i = 0; i < 25 ; i++){
	yemY[i] = i + 1;
	}
}
void yem(){
	//yemCoord();
	srand(art);
	//int formulX = (art*art)+art;
	//int formulY = (cart+1) +art-1;
	yemboyutu = 1;
	yemler.x = rand() % 70;//yemX[formulX % 75];
	yemler.y = rand() % 10;//yemY[formulY % 15];
	yemler.karakter = yemKarakteri;
	art++;
	//cart--;
	
	 
	
	
}
void yemgoster(){
	int x = yemler.x;
	int y = yemler.y;
	sahne[x][y] = yemler.karakter;
}
void scores(){
	gotoxy(21,2);
	kprint("Your Score is Here :");
	kprintint(score);
	score++;
}
void yemyedikontrol(){
	if(yilanKuyruk[0].x == yemler.x && yilanKuyruk[0].y == yemler.y){
		yem();
		
		yilanKuyrugunaEkle();
		scores();
		yemgoster();
	}
}

void levels(){
	int a;
	clear_screen();
	kprint("Zorluk Seçiniz..");
	gotoxy(1,0);
	kprint("1 , Kolay");
	gotoxy(2,0);
	kprint("2 , Orta");
	gotoxy(3,0);
	kprint("3 , Hizli");
	gotoxy(4,0);
	kprint("Seciniz?");
	
	
}


void gamename(){
char var[2],yok[2];
	var[0] = 254;
	var[1] = 0;
	yok[0] = 32;
	yok[1] = 0;
	
	gotoxy(1,1);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	gotoxy(2,1);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(3,1);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(4,1);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	gotoxy(5,1);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(6,1);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(7,1);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	
	gotoxy(1,7);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(2,7);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(3,7);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(4,7);
	kprint(var);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(5,7);
	kprint(var);
	kprint(yok);
	kprint(var);
	kprint(yok);
	kprint(var);
	gotoxy(6,7);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(var);
	kprint(var);
	gotoxy(7,7);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	
	gotoxy(1,13);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	gotoxy(2,13);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(3,13);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(4,13);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(5,13);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	gotoxy(6,13);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(7,13);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	
	gotoxy(1,19);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(2,19);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(3,19);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(var);
	kprint(yok);
	gotoxy(4,19);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	kprint(yok);
	gotoxy(5,19);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(var);
	kprint(yok);
	gotoxy(6,19);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(7,19);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	
	gotoxy(1,25);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	gotoxy(2,25);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(3,25);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(4,25);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	kprint(yok);
	gotoxy(5,25);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(6,25);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(7,25);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	
	gotoxy(1,33);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	gotoxy(2,33);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(3,33);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(4,33);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(5,33);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(var);
	kprint(var);
	gotoxy(6,33);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(7,33);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	
	gotoxy(1,39);
	kprint(yok);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	gotoxy(2,39);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(3,39);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(4,39);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(5,39);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	gotoxy(6,39);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(7,39);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	
	
	gotoxy(1,45);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(2,45);
	kprint(var);
	kprint(var);
	kprint(yok);
	kprint(var);
	kprint(var);
	gotoxy(3,45);
	kprint(var);
	kprint(yok);
	kprint(var);
	kprint(yok);
	kprint(var);
	gotoxy(4,45);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(5,45);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(6,45);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	gotoxy(7,45);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(var);
	
	
	gotoxy(1,51);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	gotoxy(2,51);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(3,51);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(4,51);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(yok);
	kprint(yok);
	gotoxy(5,51);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(6,51);
	kprint(var);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	kprint(yok);
	gotoxy(7,51);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	kprint(var);
	gotoxy(0,0);
	
	
}

void menu(){
	char solustkose[2],ustalt[2],sagustkose[2],sagsol[2],solaltkose[2],sagualtkose[2],bos[2];
	solustkose[0] = 201;
	solustkose[1] = 0;
	ustalt[0] = 205;
	ustalt[1] = 0;
	sagustkose[0] = 187;
	sagustkose[1] = 0;
	sagsol[0] = 186;
	sagsol[1] = 0;
	solaltkose[0] = 188;
	solaltkose[1] = 0;
	sagualtkose[0] = 200;
	sagualtkose[1] = 0;
	bos[0] = 32;
	bos[1] = 0;
	
	
	
	kprint(solustkose);
	int y,x;
	for(y = 0;y<60;y++){
		
		kprint(ustalt);
	}
	kprint(sagustkose);
	gotoxy(1,0);
	
	
	for(x = 1;x<11;x++){
		int t;
		kprint(sagsol);
		for(t = 0;t < 60;t++){
			kprint(bos);
		}
		kprint(sagsol);
		gotoxy(x,0);
	}
	kprint(sagualtkose);
	
	for(y = 0;y<60;y++){
		
	kprint(ustalt);
	
	}
	kprint(solaltkose);
	gotoxy(9,2);
	kprint("By Sypher(Furkan Say)");
	gotoxy(11,2);
	kprint("Press 'X' to Start The Game");
	gamename();
	}
void delay(){
for(int i = 0; i < 30000000;i++){ //10000000 hizli
	
}
}
void games(){
unsigned char status;
	char keycode;
	unsigned char c;
	
	
	baslangic();
	yem();
	scores();
	yemgoster();
	baslangicyilanigor();
	while(boole){
	write_port(0x20, 0x20);
	
	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		
		
		keycode = read_port(KEYBOARD_DATA_PORT);
		c = keyboard_map[(unsigned char) keycode];
		
		if((c == 'A' || c =='a') && yilanKuyruk[0].yon != YON_SAG){
		yilanKuyruk[0].yon = YON_SOL;
		
		
	
	}
	if((c == 'd' || c =='D')&& yilanKuyruk[0].yon != YON_SOL){
		yilanKuyruk[0].yon = YON_SAG;
		
	
	}
	if((c == 'W' || c =='w')&& yilanKuyruk[0].yon != YON_ASAGI){
		yilanKuyruk[0].yon = YON_YUKARI;
	
	}
	if((c == 's' || c =='S')&& yilanKuyruk[0].yon != YON_YUKARI){
		yilanKuyruk[0].yon = YON_ASAGI;
		
	}if (c == 27){
	
		boole = false;}
		}
		temizle();
		
		sinirlar();
		yilanhareketettir();
		duvarkontrol();
		yilanuzerindemi();
		baslangicyilanigor();
		yemyedikontrol();
		gotoxy(0,0);
		yemgoster();
		ekranaCiz();
		delay();	 
	}
	if(!boole){
	temizle();
	ekranaCiz();
	gotoxy(0,0);
			kprint("Game Over " );
			kprint("Your Score is Here : ");
			kprintint(score-1);
			gotoxy(1,0);
			kprint("Restart Game (R)");
	}
	}
	void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;
	unsigned char c;
	
	
	/* write EOI */
	write_port(0x20, 0x20);
	
	status = read_port(KEYBOARD_STATUS_PORT);
	
	
	
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		
		
		keycode = read_port(KEYBOARD_DATA_PORT);
		c = keyboard_map[(unsigned char) keycode];
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			kprint_newline();
			return;
		}if (c == 'x'){
		clear_screen();
		levels();
		}
		if (c == '1'){
		clear_screen();
		games();
		}
		if (c == '2'){
		clear_screen();
		games();
		}
		if (c == '3'){
		clear_screen();
		games();
		}if((c == 'r' || c =='R')){
		clear_screen();
		boole = true;
		score = 0;
		games();
		}if (c == 27){
		boole = true;
		}
		

		
	}
}

void kmain(void)
{	clear_screen();
	//const char *str = "my first kernel with keyboard support";
	
	
	//kprintint(12);
	//kprintint(0);
	menu();
	idt_init();
	kb_init();

	while(1);
}