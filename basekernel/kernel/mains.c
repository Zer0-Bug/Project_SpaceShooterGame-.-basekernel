#include<graphics.h>
#include<time.h>

int i,X[100],Y[100],rx,ry,l=5,d=2,s=16;
int gm,gd;
bool a = true;
int screen(){
	
	//srand(time(NULL));
	detectgraph(&gd,&gm); // Ekran tanitma
	initgraph(&gd,&gm,NULL); // Winbgi aktif hale getirme
}
int sifirla(){
	//X[100],Y[100];
	l=5;
	d=2;
	rx= 0;
	ry= 0;
}
int baslangic(){
	
	setfillstyle(1,1); // Duvarin rengi 1 mavi 2 yesil 
	bar(8,8,608,24);
	bar(8,392,608,408);
	bar(8,8,24,408);
	bar(592,8,608,408);    // duvarlar
	outtextxy(302,410,"0");
	X[0]=s*(600/(2*s)); 
	Y[0]=s*(400/(2*s)); // yilan boyutu
	
    setfillstyle(1,2); // yem rengi
    while(getpixel(rx,ry)!=0)
	{
		rx=s*(1+rand()%(600/s-1));
		ry=s*(1+rand()%(400/s-1));    // rastgelelik saglaniyor yemde
	}
	bar(rx-s/2,ry-s/2,rx+s/2,ry+s/2); // baslangic yemi
	
}

int games(int hizli){
	
	while(a)
	{
    	setfillstyle(1,0);
		bar(X[l-1]-s/2,Y[l-1]-s/2,X[l-1]+s/2,Y[l-1]+s/2); // yilan bkuyrugunu sabit birakmamak icin
		for(i=l-1;i>0;i--)
    	{
    		X[i]=X[i-1];
    		Y[i]=Y[i-1]; // yilan hareket ettirme bolumu
		}
    	if(d==0)
			X[0]=X[0]-s;
		if(d==1)
			Y[0]=Y[0]-s;	             // yilanin yon hareketini saglama if elseleri
    	else if(d==2)
			X[0]=X[0]+s;
		else if(d==3)
			Y[0]=Y[0]+s;
		if(getpixel(X[0],Y[0])==1){
		
		a = false;
		}  //duvar kontrol
			
		if((GetAsyncKeyState( 'D' ) & 0x0001) && (d!=0)) //right
			d=2;
		else if((GetAsyncKeyState( 'W' ) & 0x0001) && (d!=3)) //up
			d=1;             
		else if((GetAsyncKeyState( 'A' ) & 0x0001) && (d!=2)) //left     // hepsi klavye kontrolu
			d=0;
		else if((GetAsyncKeyState( 'S' ) & 0x0001) && (d!=1))  //down
			d=3;
		if(getpixel(X[0],Y[0])==2)  // yem yemdimi kontrol
		{
			rx=s; ry=s;
			setfillstyle(1,2);
			while(getpixel(rx,ry)!=0)    // yemin duvarda olusmasini engelleme
			{
				rx=s*(1+rand()%(600/s-1));
				ry=s*(1+rand()%(400/s-1)); // rastgele yem atama
			}
			bar(rx-s/2,ry-s/2,rx+s/2,ry+s/2);  // yem olusturma
			l=l+1;
			char snum[5];
			itoa(l-5, snum, 10);
			outtextxy(302,410,snum);   // int ifadeyi stringce cevirme score yazdirma
		}
		setfillstyle(1,1);            // renk belirleme fuct
		for(i=0;i<l;i++)
       		bar(X[i]-s/2,Y[i]-s/2,X[i]+s/2,Y[i]+s/2);  // yilanin hareketinis aglayan bas kismi 
		
		delay(hizli);  // yilan hizini belirleme salise basina silip tekrardan yazma
    }	
}
int sekilsukul(){
	outtextxy(0,0,"@");
	outtextxy(0,10,"@");
	getch();
}

int menu(){
	outtextxy(280, 70, "Kolay (1)" );
	outtextxy(280, 220, "Orta (2)" );
	outtextxy(280, 350, "Zor (3)" ); // winbgi text yazdirma 
	
	char c = (char) getch();
		cleardevice();              // winbgi ekran tezmizleme
		closegraph();           //  winbgi ekrani kapatma
		screen();               // winbgi tekrardan ekrani aktif hale getirme
		if(c == '1'){
		baslangic();                  //  oyunu baslatma 
		games(120);
		
		}if(c == '2'){
		baslangic();
		games(90);
	
		}if(c == '3'){
		baslangic();
		games(50);
		
		}
	
	
 
}
int gameover(){
		cleardevice();
		closegraph();
		screen();
	outtextxy(280, 200, "Game OVER" );
	outtextxy(280, 220, "Restart (R)" );
	outtextxy(280, 240, "Exit (X)" );
			char snum[5];
			itoa(l-5, snum, 10);   // int ifadeyi string olarak tanimlar cunku outtextxy aldigi deger string oldugu icin
			outtextxy(352,410,snum);
			outtextxy(270,410,"Your Score :");
		while(1){
			char as = (char) getch();
				if(as == 'r'){
					a = true;
					cleardevice();
				closegraph();
				screen();
				sifirla();
				menu();
				gameover();
				
					
				}if(as == 'x'){
					break;
				}
		}
	

}




int main()
{	screen();
	menu();
	//gameover();
	gameover();

	
	return 0;	
}
