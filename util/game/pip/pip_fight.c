/*---------------------------------------------------------------------------*/
/* Õ½¶·ÌØÇø                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

#ifndef MAPLE
extern char BoardName[];
#endif  // END MAPLE

struct playrule badmanlist[] = {
"éÅÊ÷¹Ö",	 60,0,	20,0,	20,	20,	 20,	150, "11101",	0,0,
"ÍøÂ·Ä§",	 60,0,	20,0,	30,	30,	 30,	200, "01111",	0,0,
"Ä¢¹½Ğ¡¹Ö",	 80,0,	40,0,	50,	35,	 60,	250, "11110",	0,0,
"¶¾Ğ«",		 85,0,	30,0,	80,	90,	 80,	500, "10111",	0,0,
"¶ñ¹·",		 90,0,  50,0,   75,	70,	 60,	550, "11010",	0,0,
"ºìÑÛ¹íÃ¨",	130,0,	50,0,	75,	90,	 70,	500, "11011",	0,0,
"×ÏÉ«Ä§öè",	140,0,	60,0,	80,	80,	 80,	550, "10101",	0,0,
"¹ÖÎïó¯òë",	150,0,	70,0,	85,	70,	 67,	500, "11110",	0,0,
"Ö©Öë¾«",	180,0,	50,0,   90,	90,	 80,	850, "00111",	0,0,
"Ê³ÈËÎ×Ê¦",	175,0, 100,0,  100,	80,	 60,    800, "11010",	0,0,
"´óé³¹Ö",	240,0,  80,0,  110,    100,	 70,    800, "00111",	0,0,
"°×É«¶ñÄ§",	250,0,  60,0,  120,    110,	 80,    900, "01011",	0,0,
"ËÀÉñÄ§",	280,0,  80,0,  150,    120,	 90,   1200, "00011",   0,0,
"´ó¿ÖÁú",	300,0,	50,0,  160,    120,	 90,   1500, "11001",	0,0,
"³¬¼¶Åç»ğÁú",	500,0, 100,0,  250,    250,	150,   1500, "11000",	0,0,
"÷¼÷ÃÍ·¹Ö",	600,0, 400,0,  350,    400,	250,   2000, "00110",	0,0,
"°¢Ç¿Ò»ºÅ",	700,0, 500,0,  600,    900,     500,   2000, "10011",	0,0,
"Ãæ¾ß¹ÖÈË",	700,0, 500,0,  800,    850,	300,   2000, "11100",	0,0,
"U2ÍâĞÇÈË",	800,0, 600,0,  800,    800,	600,   2000, "11010",	0,0,
"ÖĞ¹ú½®  ",	800,0, 600,0, 1000,   1000,     500,   2000, "10100",	0,0,
"²ÊÉ«Çõ³¤",     900,0, 800,0, 1200,   1200,     600,   3000, "11100",   0,0,
"Ä§Òô¼ªËûÊÖ",  1000,0, 850,0, 1400,   1000,     650,   3000, "11001",   0,0,
"ÍòÄêÀÏ¹ê",    1200,0,1000,0, 1300,   1500,     500,   3000, "01011",   0,0,
"°ËÉñ",	       1200,0, 900,0, 1500,   1300,     800,   3000, "10101",   0,0,
"ÌúÃæÈË",      1500,0,1200,0, 1800,   1800,    1200,   4000, "00011",   0,0,
"´ó×ì",        1600,0,1000,0, 1700,   1800,    1100,   4000, "00110",   0,0,
"÷¼÷Ã±ø",      1700,0,1500,0, 1800,   1800,    1250,  4000, "10110",   0,0,
"ÈÛ»¯Ñı",      1750,0,1300,0, 1800,   2000,    1000,   4000, "01011",	0,0,
"Ê¹Í½",	       2500,0,2500,0, 2500,   2500,    2500,   5000, "10001",   0,0,
"°£¼°Ä¾ÄËÒÁ",  3500,0,3000,0, 3500,   3500,    2000,   5000, "10110",	0,0,
"¹ÅĞ¡ÍÃ",      5000,0,4500,0, 5000,   6000,    4000,   5000, "11100",   0,0,
"Ê®×Ö»úÆ÷ÈË",  6000,0,5000,0, 5500,   6500,    5000,   5000, "01001",	0,0,
"·ÉÌìĞ¡Î×Ê¦",  6500,0,5500,0, 6500,   6000,    6000,   6000, "01101",	0,0,
"Õ¨µ°³¬ÈË",    7000,0,6000,0, 8000,   7000,    6000,   6500, "00001",	0,0,
"Akira",      13500,0,6000,0,15000,  12000,   13000,   8000, "00100",	0,0, 
"Sarah",      15000,0,6000,0,14000,  13000,   12000,   8000, "10000",	0,0, 
"Jacky",      13000,0,6000,0,15000,  12000,   13000,   8000, "01000",	0,0, 
"Wolf",       16000,0,6000,0,13000,  15000,   10000,   8000, "00010",	0,0, 
"Jeffery",    15000,0,6000,0,12000,  14000,   13000,   8000, "00001",	0,0, 
"Kage",       12000,0,6000,0,11000,  11000,   15000,   8000, "10000",	0,0, 
"Lau",        14000,0,6000,0,14000,  13000,   14000,   8000, "01000",	0,0, 
"Lion",       16000,0,6000,0,12000,  15000,   11000,   8000, "00100",	0,0, 
"Shun",       11000,0,6000,0,13000,  12000,   11000,   8000, "00010",	0,0, 
"Aoi",        14000,0,6000,0,13000,  14000,   13000,   8000, "00001",	0,0, 
"Taka",       15000,0,6000,0,11000,  15000,   11000,   8000, "10000",	0,0, 
"Pai",        11000,0,6000,0,11000,  11000,   16000,   8000, "01000",	0,0, 
"NULL",		  0,0,	 0,0,	 0,	 0,	  0,	  0,  "NULL",	0,0
};


struct magicset treatmagiclist[] = {
"ÖÎÁÆ·¨Êõ",	0,	0,	0,	0,	0,	0,
"ÆøÁÆÊõ",	5,	1,	20,	0,	10,	0,
"ÄıÉñ¹éÔª",     20,     1,	100,    0,	40,	0,
"ÔªÁé¹éĞÄ",     50,     1,	400,    0,	60,	0,
"ÎåÆø³¯Ôª",     100,    2,	0,	2,	0,	0,
"NULL",		0,	0,	0,	0,	0,	0
};                            
     
struct magicset thundermagiclist[] = {
"À×Ïµ·¨Êõ",	0,	0,	0,	0,	0,	0,
"À×Öä",		10,	0,	20,	1,	5,	0,
"ÎåÀ×Öä",	20,	0,	50,	1,	5,	0,
"ÌìÀ×Öä",	40,	0,	100,	1,	5,	0,
"À×Ö®Íø",	100,	0,	250,	1,	10,	0,
"·è¿ñÖ®À×",	200,	0,	500,	1,	10,	0,
"À×ÉñÖ®Îè",	600,	0,	1500,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset icemagiclist[] = {
"±ùÏµ·¨Êõ",	0,	0,	0,	0,	0,	0,
"±ùÖä",		10,	0,	20,	1,	5,	0,
"º®±ùÖä",	20,	0,	50,	1,	5,	0,
"Ğş±ùÖä",	40,	0,	100,	1,	5,	0,
"·çÀ×±ùÌì",	100,	0,	250,	1,	10,	0,
"¾ø¶ÔÁã¶ÈÖ®±ù",	200,	0,	500,	1,	10,	0,
"±ùÉñÖ®Îè",	500,	0,	1400,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset firemagiclist[] = {
"Ñ×Ïµ·¨Êõ",	0,	0,	0,	0,	0,	0,
"»ğÊõ",		5,	0,	10,	1,	5,	0,
"Ñ×Öä",		10,	0,	20,	1,	5,	0,
"Ñ×É±Öä",	20,	0,	50,	1,	5,	0,
"Á¶ÓüÕæ»ğ",	40,	0,	100,	1,	5,	0,
"»ğÁúÕÆ",	100,	0,	250,	1,	10,	0,
"»ğÁúÕĞ»½",	200,	0,	500,	1,	10,	0,
"»ğÉñÖ®Îè",	600,	0,	1600,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset earthmagiclist[] = {
"ÍÁÏµ·¨Êõ",	0,	0,	0,	0,	0,	0,
"ÍÁÖä",		10,	0,	20,	1,	5,	0,
"·ÉÑÒÊõ",	20,	0,	50,	1,	5,	0,
"µØÁÑÌì±À",	40,	0,	100,	1,	5,	0,
"Ì©É½Ñ¹¶¥",	100,	0,	250,	1,	10,	0,
"ÍÁÁúÕÙ»½",	200,	0,	500,	1,	10,	0,
"É½ÉñÖ®Îè",	450,	0,	1300,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset windmagiclist[] = {
"·çÏµ·¨Êõ",	0,	0,	0,	0,	0,	0,
"·çÖä",		10,	0,	20,	1,	5,	0,
"Ğı·çÖä",	20,	0,	50,	1,	5,	0,
"¿ñ·çÊõ",	40,	0,	100,	1,	5,	0,
"Áú¾í·ç",	100,	0,	250,	1,	10,	0,
"·ç¾í²ĞÔÆ",	200,	0,	500,	1,	10,	0,
"·çÉñÖ®Îè",	400,	0,	1200,	1,	10,	0,
"NULL",		0,	0,	0,	0,	0,	0
};
/*---------------------------------------------------------------------------*/
/* Õ½¶·ÈËÎï¾ö¶¨º¯Ê½                                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*0~1:2
  2~4:3
  5~9:5
  10~12:3
  13~14:1
  15~19:5
  20~23:4
  24~27:4
  28~33:6
  34~45:12
*/
int
pip_meet_vs_man()
{
  int class;
  int man;
  int lucky;
  class=(d.maxhp*30+d.maxmp*20+d.attack*20+d.resist*15+d.mexp*5+d.hexp*5+d.speed*10)/8500+1;

  if(class==1)
  {
    man=rand()%2;
  }
  else if(class==2)
  { 
    if(rand()%4>0)
    {
      man=rand()%3+2;
    }
    else
    {
      man=rand()%2;
    }
  }
  else if(class==3)
  {
    lucky=rand()%9;
    if(lucky>3)
    {
      man=rand()%5+5;
    }
    else if(lucky<=3 && lucky>0)
    {
      man=rand()%3+2;
    }
    else
    {
      man=rand()%2;
    }
  }  
  else if(class==4)
  {
    lucky=rand()%15;
    if(lucky>5)
    {
      man=rand()%3+10;
    }
    else if(lucky<=5 && lucky>3)
    {
      man=rand()%5+5;
    }
    else 
    {
      man=rand()%5;
    }
  }
  else if(class==5)
  {
    lucky=rand()%20;
    if(lucky>6)
    {
      man=13+rand()%2;
    }
    else if(lucky<=6 && lucky>3)
    {
      man=rand()%3+10;
    }
    else
    {
      man=rand()%10;
    }
  }
  else if(class==6)
  {
    lucky=rand()%25;
    if(lucky>8)
    {
      man=15+rand()%6;
    }
    else if(lucky<=8 &&lucky>4)
    {
      man=13+rand()%2;
    }
    else
    {
      man=rand()%13;
    }
  }  
  else if(class==7)
  {
    lucky=rand()%40;
    if(lucky>12)
    {
      man=21+rand()%3;    
    }
    else if(lucky<=12 &&lucky>4)
    {
      man=15+rand()%6;
    }
    else
    {
      man=rand()%15;
    }
  }  
  else if(class==8)
  {
    lucky=rand()%50;
    if(lucky>25)
    {
      man=24+rand()%4;
    }
    else if(lucky<=25 && lucky>20)
    {
      man=21+rand()%3;    
    }
    else 
    {
      man=rand()%21;
    }
  }        
  else if(class==9)
  {
    lucky=rand()%75;
    if(lucky>20)
    {
      man=28+rand()%6;
    }
    else if(lucky<=20 && lucky>10)    
    {
      man=24+rand()%4;
    }
    else if(lucky<=10 && lucky>5)
    {
      man=21+rand()%3;    
    }
    else
    {
      man=rand()%21;
    }
  }          
  else if(class>=9)
  {
    lucky=rand()%100;
    if(lucky>20)
    {
      man=34+rand()%12;
    }
    else if(lucky<=20 && lucky>10)
    {
      man=28+rand()%6;
    }
    else if(lucky<=10 && lucky>5)    
    {
      man=24+rand()%4;
    }
    else
    {
      man=rand()%24;    
    }
  }            
  pip_fight_bad(man);
  return;
}

int 
pip_fight_bad(n)
int n;
{
  pip_fight_main(n,badmanlist,1);
  return;
}


int
pip_fight_main(n,list,mode)
int n;
struct playrule list[];   
int mode;
{
  pip_vs_man(n,list,mode);
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Õ½¶·Õ½¶·º¯Ê½                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int 
pip_vs_man(n,p,mode)
int n;
struct playrule *p;  
int mode;
{
 /* p[n].name hp mp speed attack resist money special map */ 
 struct playrule m; 
 char buf[256];
 char inbuf1[20];
 char inbuf2[20]; 
 int pipkey; 
 int mankey;
 int lucky;
 int dinjure=0;		/*Ğ¡¼¦ÉËº¦Á¦*/
 int minjure=0;		/*¶Ô·½ÉËº¦Á¦*/
 int dresistmore=0;	/*Ğ¡¼¦¼ÓÇ¿·ÀÓù*/
 int mresistmore=0;	/*¶Ô·½¼ÓÇ¿·ÀÓù*/
 int oldhexp;		/*Î´Õ½¶·Ç°¸ñ¶·¾­Ñé*/
 int oldmexp;		/*Î´Õ½¶·Ç°Ä§·¨¾­Ñé*/
 int oldbrave;		/*Î´Õ½¶·Ç°ÓÂ¸Ò*/
 int oldhskill;		/*Î´Õ½¶·Ç°Õ½¶·¼¼Êõ*/
 int oldmskill;		/*Î´Õ½¶·Ç°Ä§·¨¼¼Êõ*/
 int oldetchics;	/*Î´Õ½¶·Ç°µÀµÂ*/
 int oldmoney;		/*Î´Õ½¶·Ç°½ğÇ®*/
 int oldtired;
 int oldhp;
 int winorlose=0;		/*1:you win 0:you loss*/
  
 /*Ëæ»ú²úÉúÈËÎï ²¢ÇÒ´æºÃÕ½¶·Ç°µÄÒ»Ğ©ÊıÖµ*/
 oldhexp=d.hexp;
 oldmexp=d.mexp;
 oldbrave=d.brave;
 oldhskill=d.hskill;
 oldmskill=d.mskill;
 oldetchics=d.etchics;
 oldmoney=d.money;
 if(mode==1)
 {
	 m.hp=p[n].hp-rand()%10;
	 m.maxhp=(m.hp+p[n].hp)/2;
	 m.mp=p[n].mp-rand()%10;
	 m.maxmp=(m.mp+p[n].mp)/2;
	 m.speed=p[n].speed-rand()%4-1;
	 m.attack=p[n].attack-rand()%10;
	 m.resist=p[n].resist-rand()%10;
	 m.money=p[n].money-rand()%50;
	 m.death=p[n].death;
 }
 else
 {
 	 m.maxhp=d.maxhp*(80+rand()%50)/100+20;;
 	 m.hp=m.maxhp-rand()%10+20;
 	 m.maxmp=d.maxmp*(80+rand()%50)/100+10;
 	 m.mp= m.maxmp-rand()%20+10;
 	 m.speed=d.speed*(80+rand()%50)/100+10;
 	 m.attack=d.attack*(80+rand()%50)/100+10;
 	 m.resist=d.resist*(80+rand()%50)/100+10;
 	 m.money=0;
 	 m.death=0;
 }
 /*d.tired+=rand()%(n+1)/4+2;*/
 /*d.shit+=rand()%(n+1)/4+2;*/
 do
 { 
   if(m.hp<=0) /*µĞÈËËÀµôÁË*/
   {
     m.hp=0;
     d.money+=m.money;
     m.death=1;
     d.brave+=rand()%4+3;
   }
   if(d.hp<=0 || d.tired>=100)  /*Ğ¡¼¦ÕóÍö*/
   {
     if(mode==1)
     {
       d.hp=0;
       d.tired=0;
       d.death=1;
     }
     else
     {
       d.hp=d.maxhp/3+10;
       d.hexp-=rand()%3+2;
       d.mexp-=rand()%3+2;
       d.tired=50;
       d.death=1;
     }
   }        
   clear(); 
   /*showtitle("µç×ÓÑøĞ¡¼¦", BoardName);*/
   move(0,0);
   if(d.sex==1)
     sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« [32m¡á [37m%-10s                                                  [0m",d.name); 	 
   else if(d.sex==2)
     sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« [33m¡â [37m%-10s                                                  [0m",d.name); 	 
   else 
     sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« [34m£¿ [37m%-10s                                                  [0m",d.name); 	 
   prints(buf);    
   move(6,0);
   if(mode==1)
	   show_badman_pic(n);
   move(1,0);
   sprintf(buf,"[1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
   prints(buf);
   move(2,0);
   /* luckyÄÃÀ´µ±colorÓÃ*/
   if(d.tired>=80)
      lucky=31;
   else if(d.tired>=60 && d.tired< 80)
      lucky=33;
   else
      lucky=37;
   sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
   sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);    
   sprintf(buf,"[1;31m©¦[33mÉú  Ãü:[37m%-12s[33m·¨  Á¦:[37m%-12s[33mÆ£  ÀÍ:[%dm%-12d[33m½ğ  Ç®:[37m%-10d[31m©¦[m",
  	 inbuf1,inbuf2,lucky,d.tired,d.money);
   prints(buf);
   move(3,0);
   sprintf(inbuf1,"%d/%d",d.hexp,d.mexp);  
   sprintf(buf,"[1;31m©¦[33m¹¥  »÷:[37m%-10d  [33m·À  Óù:[37m%-10d  [33mËÙ  ¶È:[37m%-5d       [33m¾­  Ñé:[37m%-10s[31m©¦[m",
 	 d.attack,d.resist,d.speed,inbuf1);
   prints(buf);
   move(4,0);
   sprintf(buf,"[1;31m©¦[33mÊ³  Îï:[37m%-5d       [33m´ó²¹Íè:[37m%-5d       [33mÁã  Ê³:[37m%-5d       [33mÒ©  ²İ:[37m%-5d     [31m©¦[m",
 	 d.food,d.bighp,d.cookie,d.medicine);
   prints(buf);	 
   move(5,0);
   sprintf(buf,"[1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
   prints(buf); 
   move(19,0);
   sprintf(buf,"[1;34m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
   prints(buf);
   move(20,0);
   sprintf(inbuf1,"%d/%d",m.hp,m.maxhp);  
   sprintf(inbuf2,"%d/%d",m.mp,m.maxmp);     
   sprintf(buf,"[1;34m©¦[32mĞÕ  Ãû:[37m%-10s  [32mÉú  Ãü:[37m%-11s [32m·¨  Á¦:[37m%-11s                  [34m©¦[m",
 	 p[n].name,inbuf1,inbuf2);
   prints(buf);
   move(21,0);
   sprintf(buf,"[1;34m©¦[32m¹¥  »÷:[37m%-6d      [32m·À  Óù:[37m%-6d      [32mËÙ  ¶È:[37m%-6d      [32m½ğ  Ç®:[37m%-6d    [34m©¦[m",
 	 m.attack,m.resist,m.speed,m.money);
   prints(buf);
   move(22,0);
   sprintf(buf,"[1;34m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
   prints(buf);
   move(b_lines,0);   
   sprintf(buf,"[1;44;37m  Õ½¶·ÃüÁî  [46m  [1]ÆÕÍ¨  [2]È«Á¦  [3]Ä§·¨  [4]·ÀÓù  [5]²¹³ä  [6]ÌÓÃü         [m");
   prints(buf);    
   
   if(m.death==0 && d.death==0)
   {
    dresistmore=0;
    d.nodone=0;
    pipkey=egetch(); 
    switch(pipkey)
    {
     case '1': 
      if(rand()%9==0)
      {
        pressanykey("¾¹È»Ã»´òÖĞ..:~~~"); 
      }
      else
      { 
      	if(mresistmore==0)
          dinjure=(d.hskill/100+d.hexp/100+d.attack/9-m.resist/12+rand()%12+2-m.speed/30+d.speed/30);
        else
	  dinjure=(d.hskill/100+d.hexp/100+d.attack/9-m.resist/8+rand()%12+2-m.speed/30+d.speed/30);            
	if(dinjure<=0)
	  dinjure=9;
        m.hp-=dinjure;
        d.hexp+=rand()%2+2;
        d.hskill+=rand()%2+1;  
        sprintf(buf,"ÆÕÍ¨¹¥»÷,¶Ô·½ÉúÃüÁ¦¼õµÍ%d",dinjure);
        pressanykey(buf);
      }
      d.tired+=rand()%(n+1)/15+2;
      break;
     
     case '2':
      show_fight_pic(2);
      if(rand()%11==0)
      { 
        pressanykey("¾¹È»Ã»´òÖĞ..:~~~");
      }     
      else 
      { 
        if(mresistmore==0)      
          dinjure=(d.hskill/100+d.hexp/100+d.attack/5-m.resist/12+rand()%12+6-m.speed/50+d.speed/30);
        else
          dinjure=(d.hskill/100+d.hexp/100+d.attack/5-m.resist/8+rand()%12+6-m.speed/40+d.speed/30);                  
        if(dinjure<=15)
          dinjure=20;  
        if(d.hp>5)
        { 
          m.hp-=dinjure;
          d.hp-=5;
          d.hexp+=rand()%3+3;
          d.hskill+=rand()%2+2; 
          d.tired+=rand()%(n+1)/10+3; 
          sprintf(buf,"È«Á¦¹¥»÷,¶Ô·½ÉúÃüÁ¦¼õµÍ%d",dinjure);
          pressanykey(buf);
        }
        else
        { 
          d.nodone=1;
          pressanykey("ÄãµÄHPĞ¡ì¶5À²..²»ĞĞÀ²...");
        }
      }
      break;
     
     case '3':
      oldtired=d.tired;
      oldhp=d.hp;     
      d.magicmode=0;
      dinjure=pip_magic_menu(); 
      if(dinjure<0)
        dinjure=5;
      if(d.nodone==0)
      {
        if(d.magicmode==1)
        {
          oldtired=oldtired-d.tired;
          oldhp=d.hp-oldhp;
          sprintf(buf,"ÖÎÁÆáá,ÉúÃüÁ¦Ìá¸ß%d Æ£ÀÍ½µµÍ%d",oldhp,oldtired);
          pressanykey(buf);
        }
        else
        {
          if(rand()%15==0)
             pressanykey("¾¹È»Ã»´òÖĞ..:~~~");  
          else
          {  
             if(d.mexp<=100)
             {
                if(rand()%4>0)
                    dinjure=dinjure*60/100;
                else
                    dinjure=dinjure*80/100;
             }
             else if(d.mexp<=250 && d.mexp>100)
             {
                if(rand()%4>0)
                    dinjure=dinjure*70/100;
                else
                    dinjure=dinjure*85/100;           
             }
             else if(d.mexp<=500 && d.mexp>250)
             {
                if(rand()%4>0)
                    dinjure=dinjure*85/100;
                else
                    dinjure=dinjure*95/100;           
             }
             else if(d.mexp>500)
             {
                if(rand()%10>0)
                    dinjure=dinjure*90/100;
                else
                    dinjure=dinjure*99/100;           
             }
             if((p[n].special[d.magicmode-2]-48)==1)
             {
                if(rand()%2>0)
                {
                  dinjure=dinjure*125/100;
                }
                else
                {
                  dinjure=dinjure*110/100;
                }
             }
             else
             {
                if(rand()%2>0)
                {
                  dinjure=dinjure*60/100;
                }
                else
                {  
                  dinjure=dinjure*75/100;           
                }
             }
             d.tired+=rand()%(n+1)/12+2;
             m.hp-=dinjure; 
             /*d.mexp+=rand()%2+2;*/
             d.mskill+=rand()%2+2;  
             sprintf(buf,"Ä§·¨¹¥»÷,¶Ô·½ÉúÃüÁ¦¼õµÍ%d",dinjure);
             pressanykey(buf);           
          }
        }
      }
      break;    
     case '4':
      dresistmore=1;
      d.tired+=rand()%(n+1)/20+1;
      pressanykey("Ğ¡¼¦¼ÓÇ¿·ÀÓùÀ²....");
      break;

     case '5':
      
      pip_basic_feed();
      break;
      
     case '6':
      d.money-=(rand()%100+30);
      d.brave-=(rand()%3+2);      
      if(d.money<0)
      	d.money=0;
      if(d.hskill<0)
      	d.hskill=0;
      if(d.brave<0)
        d.brave=0;
      clear(); 
      showtitle("µç×ÓÑøĞ¡¼¦", BoardName); 
      move(10,0);
      prints("            [1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
      move(11,0);
      prints("            [1;31m©¦  [37mÊµÁ¦²»Ç¿µÄĞ¡¼¦ [33m%-10s                 [31m©¦[m",d.name);
      move(12,0);
      prints("            [1;31m©¦  [37mÔÚÓë¶ÔÊÖ [32m%-10s [37mÕ½¶·ááÂäÅÜÀ²          [31m©¦[m",p[n].name);
      move(13,0);
      sprintf(inbuf1,"%d/%d",d.hexp-oldhexp,d.mexp-oldmexp);  
      prints("            [1;31m©¦  [37mÆÀ¼ÛÔö¼ÓÁË [36m%-5s [37mµã  ¼¼ÊõÔö¼ÓÁË [36m%-2d/%-2d [37mµã  [31m©¦[m",inbuf1,d.hskill-oldhskill,d.mskill-oldmskill);
      move(14,0);
      sprintf(inbuf1,"%d [37mÔª",oldmoney-d.money);
      prints("            [1;31m©¦  [37mÓÂ¸Ò½µµÍÁË [36m%-5d [37mµã  ½ğÇ®¼õÉÙÁË [36m%-13s  [31m©¦[m",oldbrave-d.brave,inbuf1);
      move(15,0);
      prints("            [1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");             
      pressanykey("ÈşÊ®Áù¼Æ ×ßÎªÉÏ²ß...");
      winorlose=0;
      break;

#ifdef MAPLE     
     case Ctrl('R'):
      if (currutmp->msgs[0].last_pid)
      {
      show_last_call_in();
      my_write(currutmp->msgs[0].last_pid, "Ë®Çò¶ª»ØÈ¥£º");
      }
      break;
#endif  // END MAPLE
    }   
   }
   clear(); 
   move(0,0);
   if(d.sex==1)
     sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« [32m¡á [37m%-10s                                                  [0m",d.name); 	 
   else if(d.sex==2)
     sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« [33m¡â [37m%-10s                                                  [0m",d.name); 	 
   else 
     sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« [34m£¿ [37m%-10s                                                  [0m",d.name); 	 
   prints(buf);    
   move(1,0);
   sprintf(buf,"[1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
   prints(buf);
   move(2,0);
   /* luckyÄÃÀ´µ±colorÓÃ*/
   if(d.tired>=80)
      lucky=31;
   else if(d.tired>=60 && d.tired< 80)
      lucky=33;
   else
      lucky=37;
   sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
   sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);    
   sprintf(buf,"[1;31m©¦[33mÉú  Ãü:[37m%-12s[33m·¨  Á¦:[37m%-12s[33mÆ£  ÀÍ:[%dm%-12d[33m½ğ  Ç®:[37m%-10d[31m©¦[m",
  	 inbuf1,inbuf2,lucky,d.tired,d.money);
   prints(buf);
   
   move(3,0);
   sprintf(inbuf1,"%d/%d",d.hexp,d.mexp);     
   sprintf(buf,"[1;31m©¦[33m¹¥  »÷:[37m%-6d      [33m·À  Óù:[37m%-6d      [33mËÙ  ¶È:[37m%-5d       [33m¾­  Ñé:[37m%-10s[31m©¦[m",
 	 d.attack,d.resist,d.speed,inbuf1);
   prints(buf);
   move(4,0);
   sprintf(buf,"[1;31m©¦[33mÊ³  Îï:[37m%-5d       [33m´ó²¹Íè:[37m%-5d       [33mÁã  Ê³:[37m%-5d       [33mÒ©  ²İ:[37m%-5d     [31m©¦[m",
 	 d.food,d.bighp,d.cookie,d.medicine);
   prints(buf);	 
   move(5,0);
   sprintf(buf,"[1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
   prints(buf);    
   move(6,0);
   if(mode==1)
	   show_badman_pic(n);
   move(19,0);
   sprintf(buf,"[1;34m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
   prints(buf);
   move(20,0);
   sprintf(inbuf1,"%d/%d",m.hp,m.maxhp);  
   sprintf(inbuf2,"%d/%d",m.mp,m.maxmp);     
   sprintf(buf,"[1;34m©¦[32mĞÕ  Ãû:[37m%-10s  [32mÉú  Ãü:[37m%-11s [32m·¨  Á¦:[37m%-11s                  [34m©¦[m",
 	 p[n].name,inbuf1,inbuf2);
   prints(buf);
   move(21,0);
   sprintf(buf,"[1;34m©¦[32m¹¥  »÷:[37m%-6d      [32m·À  Óù:[37m%-6d      [32mËÙ  ¶È:[37m%-6d      [32m½ğ  Ç®:[37m%-6d    [34m©¦[m",
 	 m.attack,m.resist,m.speed,m.money);
   prints(buf);
   move(22,0);
   sprintf(buf,"[1;34m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
   prints(buf);
   move(b_lines,0);   
   sprintf(buf,"[1;41;37m  [37m¹¥»÷ÃüÁî  [47m  [31m[1][30mÆÕÍ¨  [31m[2][30mÈ«Á¦  [31m[3][30mÄ§·¨  [31m[4][30m·ÀÓù  [31m[5][30mÌÓÃü                     [m");
   prints(buf);       
   
   if((m.hp>0)&&(pipkey!='6')&& (pipkey=='1'||pipkey=='2'||pipkey=='3'||pipkey=='4'||pipkey=='5')&&(d.death==0)&&(d.nodone==0))
   {
     mresistmore=0;
     lucky=rand()%100;
     if(lucky>=0 && lucky<=50)
        mankey=1;
     else if(lucky>=51 && lucky<=84)
        mankey=2;  
     else if(lucky>=85 && lucky<=97)
        mankey=3;
     else if(lucky>=98)
        mankey=4;   
     switch(mankey)
     {    
      case 1:
       if(rand()%6==5)
       { 
         pressanykey("¶Ô·½Ã»´òÖĞ..:~~~");
       }     
       else
       {
        if(dresistmore==0)      
          minjure=(m.attack/9-d.resist/12+rand()%15+4-d.speed/30+m.speed/30-d.hskill/200-d.hexp/200);
        else
          minjure=(m.attack/9-d.resist/8+rand()%12+4-d.speed/50+m.speed/20-d.hskill/200-d.hexp/200);
        if(minjure<=0)
          minjure=8;  
        d.hp-=minjure;
        d.tired+=rand()%3+2;
        sprintf(buf,"¶Ô·½ÆÕÍ¨¹¥»÷,ÉúÃüÁ¦¼õµÍ%d",minjure);
        pressanykey(buf);
       }             
       break;
       
      case 2:       
       if(rand()%11==10)
       { 
         pressanykey("¶Ô·½Ã»´òÖĞ..:~~~");
       }     
       else
       {
        if(m.hp>5)
        {
          if(dresistmore==0)      
            minjure=(m.attack/5-d.resist/12+rand()%12+6-d.speed/30+m.speed/30-d.hskill/200-d.hexp/200);
          else
            minjure=(m.attack/5-d.resist/8+rand()%12+6-d.speed/30+m.speed/30-d.hskill/200-d.hexp/200);                  
	  if(minjure<=15)
	    minjure=20;
          d.hp-=minjure;
          m.hp-=5;
          sprintf(buf,"¶Ô·½È«Á¦¹¥»÷, ÉúÃüÁ¦¼õµÍ%d",minjure);
          d.tired+=rand()%4+4;
          pressanykey(buf);
        }
        else
        {
          if(dresistmore==0)      
            minjure=(m.attack/9-d.resist/12+rand()%12+4-d.speed/30+m.speed/25-d.hexp/200-d.hskill/200);
          else
            minjure=(m.attack/9-d.resist/8+rand()%12+3-d.speed/30+m.speed/25-d.hexp/200-d.hskill/200);        
          if(minjure<=0)
            minjure=4;
          d.hp-=minjure;
          d.tired+=rand()%3+2;
          sprintf(buf,"¶Ô·½ÆÕÍ¨¹¥»÷,ÉúÃüÁ¦¼õµÍ%d",minjure);
          pressanykey(buf);        
        }
       }             
       break;             
       
      case 3:
       if(rand()%5>0 && m.mp>20)
       {
        if(rand()%6>0 && m.mp>=50)
        {
         if(m.mp>=1000)
         {
           minjure=500;
           m.mp-=(500+rand()%300);
           if(rand()%2)
	      sprintf(inbuf1,"ÈÈ»ğÄ§");
	   else
	      sprintf(inbuf1,"º®Æø¹í");
         }
         else if(m.mp<1000 && m.mp>=500)
         {
           minjure=300;
           m.mp-=(300+rand()%200);
           if(rand()%2)
	      sprintf(inbuf1,"¿ñË®¹Ö");
	   else
	      sprintf(inbuf1,"Å­ÍÁ³æ");
         }
         else if(m.mp<500 && m.mp>=200)
         {
           minjure=100;
           m.mp-=(100+rand()%100);
           if(rand()%2)
	      sprintf(inbuf1,"ÃÔ»ê¹í²î");
	   else
	      sprintf(inbuf1,"Ê¯¹Ö");
         }
         else if(m.mp<200 && m.mp>=50)
         {
           minjure=50;
           m.mp-=50;
           if(rand()%2)
	      sprintf(inbuf1,"¹íÄ¾»ê");
	   else
	      sprintf(inbuf1,"·çÑı");
         }
         minjure=minjure-d.resist/50-d.mresist/10-d.mskill/200-d.mexp/200+rand()%10;
         if(minjure<0)
         	minjure=15;
         d.hp-=minjure;
         d.mresist+=rand()%2+1;
         sprintf(buf,"¶Ô·½ÕĞ»»ÁË%s,ÄãÊÜÉËÁË%dµã",inbuf1,minjure);
         pressanykey(buf);
        }
        else
        {
         m.mp-=20;
         m.hp+=130+rand()%20;
         if(m.hp>m.maxhp)
            m.hp=m.maxhp;
         pressanykey("¶Ô·½Ê¹ÓÃÄ§·¨ÖÎÁÆÁË×Ô¼º...");
        }
       }
       else
       {
         mresistmore=1;
         pressanykey("¶Ô·½¼ÓÇ¿·ÀÓù....");
       }
       break;
      
      case 4:
       d.money+=(m.money+m.money/2)/3+rand()%10;
       d.hskill+=rand()%4+3;
       d.brave+=rand()%3+2;
       m.death=1;
       sprintf(buf,"¶Ô·½ÏÈÉÁÁË..µ«µôÁËÒ»Ğ©Ç®¸øÄã...");
       pressanykey(buf);       
       break;      
     }
   }
   
   if(m.death==1)
   {
     clear();
     showtitle("µç×ÓÑøĞ¡¼¦", BoardName);
     if(mode==1)
     { 
       move(10,0);
       prints("            [1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
       move(11,0);
       prints("            [1;31m©¦  [37mÓ¢ÓÂµÄĞ¡¼¦ [33m%-10s                     [31m©¦[m",d.name);
       move(12,0);
       prints("            [1;31m©¦  [37m´ò°ÜÁËĞ°¶ñµÄ¹ÖÎï [32m%-10s               [31m©¦[m",p[n].name);
     }  
     else
     {
       move(10,0);
       prints("            [1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
       move(11,0);
       prints("            [1;31m©¦  [37mÎäÊõ´ó»áµÄĞ¡¼¦ [33m%-10s                 [31m©¦[m",d.name);
       move(12,0);
       prints("            [1;31m©¦  [37m´ò°ÜÁËÇ¿¾¢µÄ¶ÔÊÖ [32m%-10s               [31m©¦[m",p[n].name);
     }
     move(13,0);
     sprintf(inbuf1,"%d/%d",d.hexp-oldhexp,d.mexp-oldmexp);  
     prints("            [1;31m©¦  [37mÆÀ¼ÛÌáÉıÁË %-5s µã  ¼¼ÊõÔö¼ÓÁË %-2d/%-2d µã  [31m©¦[m",inbuf1,d.hskill-oldhskill,d.mskill-oldmskill);
     move(14,0);
     sprintf(inbuf1,"%d Ôª",d.money-oldmoney);
     prints("            [1;31m©¦  [37mÓÂ¸ÒÌáÉıÁË %-5d µã  ½ğÇ®Ôö¼ÓÁË %-9s [31m©¦[m",d.brave-oldbrave,inbuf1);
     move(15,0);
     prints("            [1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");     
     
     if(m.hp<=0)     
        pressanykey("¶Ô·½ËÀµôÂŞ..ËùÒÔÄãÓ®ÂŞ..");   
     else if(m.hp>0)
        pressanykey("¶Ô·½ÂäÅÜÂŞ..ËùÒÔËãÄãÓ®ÂŞ.....");   
     winorlose=1;
   }
   if(d.death==1 && mode==1)
   {   
       clear();
       showtitle("µç×ÓÑøĞ¡¼¦", BoardName);       
       move(10,0);
       prints("            [1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
       move(11,0);
       prints("            [1;31m©¦  [37m¿ÉÁ¯µÄĞ¡¼¦ [33m%-10s                     [31m©¦[m",d.name);
       move(12,0);
       prints("            [1;31m©¦  [37mÔÚÓë [32m%-10s [37mµÄÕ½¶·ÖĞ£¬                [31m©¦[m",p[n].name);
       move(13,0);
       prints("            [1;31m©¦  [37m²»ĞÒµØÕóÍöÁË£¬ÔÚ´ËÌØ±ğÄ¬°§..........      [31m©¦[m");
       move(14,0);
       prints("            [1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");        
       pressanykey("Ğ¡¼¦ÕóÍöÁË....");      
       pipdie("[1;31mÕ½¶·ÖĞ±»´òËÀÁË...[m  ",1);       
   }
   else if(d.death==1 && mode==2)
   {
       clear();
       showtitle("µç×ÓÑøĞ¡¼¦", BoardName);       
       move(10,0);
       prints("            [1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
       move(11,0);
       prints("            [1;31m©¦  [37m¿ÉÁ¯µÄĞ¡¼¦ [33m%-10s                     [31m©¦[m",d.name);
       move(12,0);
       prints("            [1;31m©¦  [37mÔÚÓë [32m%-10s [37mµÄÕ½¶·ÖĞ£¬                [31m©¦[m",p[n].name);
       move(13,0);
       prints("            [1;31m©¦  [37m²»ĞÒµØ´òÊäÁË£¬¼ÇÕßÏÖ³¡ÌØ±ğ±¨µ¼.........   [31m©¦[m");
       move(14,0);
       prints("            [1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");        
       pressanykey("Ğ¡¼¦´òÊäÁË....");     
   }
 }while((pipkey!='6')&&(d.death!=1)&&(m.death!=1)&&(mankey!=8)); 
 return winorlose;    
}                 


/*---------------------------------------------------------------------------*/
/* Õ½¶·Ä§·¨º¯Ê½                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*½øÈëÊ¹ÓÃÄ§·¨Ñ¡µ¥*/
int
pip_magic_menu()   /*Õ½¶·ÖĞ·¨ÊõµÄÓ¦ÓÃ*/
{
  char buf[256];
  int magicnum=0;
  int injure;		/*ÉËº¦Á¦*/
  int pipkey;
  
  do
  {
   move(b_lines, 0);
   clrtoeol();
   move(b_lines,0);
   sprintf(buf,
   "[1;44;37m  Ä§·¨Ñ¡µ¥  [46m  [1]ÖÎÁÆ [2]À×Ïµ [3]±ùÏµ [4]»ğÏµ [5]ÍÁÏµ [6]·çÏµ [Q]·ÅÆú: [m");
   move(b_lines,0);
   prints(buf);   
   pipkey=egetch();
   switch(pipkey)
   {
    case '1':  /*ÖÎÁÆ·¨Êõ*/
     d.magicmode=1;
     injure=pip_magic_doing_menu(treatmagiclist);
     break;

    case '2':  /*À×Ïµ·¨Êõ*/
     d.magicmode=2;
     injure=pip_magic_doing_menu(thundermagiclist);
     break;

    case '3': /*±ùÏµ·¨Êõ*/
     d.magicmode=3;
     injure=pip_magic_doing_menu(icemagiclist);
     break;

    case '4': /*Ñ×Ïµ·¨Êõ*/
     d.magicmode=4;
     injure=pip_magic_doing_menu(firemagiclist);
     show_fight_pic(341);
     pressanykey("Ğ¡¼¦Ê¹ÓÃÁËÑ×Ïµ·¨Êõ");
     break;

    case '5': /*ÍÁÏµ·¨Êõ*/
     d.magicmode=5;
     injure=pip_magic_doing_menu(earthmagiclist);
     break;

    case '6': /*·çÏµ·¨Êõ*/
     d.magicmode=6;
     injure=pip_magic_doing_menu(windmagiclist);
     break;

#ifdef MAPLE     
    case Ctrl('R'):
     if (currutmp->msgs[0].last_pid)
     {
     extern screenline* big_picture;
     screenline* screen0 = calloc(t_lines, sizeof(screenline));
     memcpy(screen0, big_picture, t_lines * sizeof(screenline));

     show_last_call_in();
     my_write(currutmp->msgs[0].last_pid, "Ë®Çò¶ª»ØÈ¥£º");

     memcpy(big_picture, screen0, t_lines * sizeof(screenline));
     free(screen0);
     redoscr();
     }
     break;
#endif  // END MAPLE
   }
  }while((pipkey!='1')&&(pipkey!='2')&&(pipkey!='3')&&(pipkey!='4')&&(pipkey!='5')&&(pipkey!='6')&&(pipkey!='Q')&&(pipkey!='q')&&(d.nodone==0));

  if((pipkey=='Q')||(pipkey=='q'))
  {
   d.nodone=1;
  }
  return injure;    
}

/*Ä§·¨ÊÓ´°*/
int
pip_magic_doing_menu(p)   /*Ä§·¨»­Ãæ*/
struct magicset *p;
{
 register int n=1;
 register char *s;
 char buf[256];
 char ans[5];
 int pipkey;
 int injure=0;

 d.nodone=0; 

 clrchyiuan(6,18);
 move(7,0);
 sprintf(buf,"[1;31m©È[37;41m   ¿ÉÓÃ[%s]Ò»ÀÀ±í   [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m",p[0].name);
 prints(buf);
 while ((s = p[n].name)&&strcmp(s,"NULL")&& (p[n].needmp<=d.mp))
 {
  move(7+n,4);
  if(p[n].hpmode==1)
  {   
      sprintf(buf,
      "[1;37m[[36m%d[37m] [33m%-10s  [37mĞèÒª·¨Á¦: [32m%-6d  [37m»Ö¸´ÌåÁ¦: [32m%-6d [37m»Ö¸´Æ£ÀÍ: [32m%-6d[m   ",n,p[n].name,p[n].needmp,p[n].hp,p[n].tired);
      prints(buf);
  }
  else if(p[n].hpmode==2)
  {
      sprintf(buf,
      "[1;37m[[36m%d[37m] [33m%-10s  [37mĞèÒª·¨Á¦: [32m%-6d  [37m»Ö¸´ÌåÁ¦µ½[35m×î´óÖµ[37m »Ö¸´Æ£ÀÍµ½[35m×îĞ¡Öµ[m  ",n,p[n].name,p[n].needmp);
      prints(buf);
  }  
  else if(p[n].hpmode==0)
  {
      sprintf(buf,
      "[1;37m[[36m%d[37m] [33m%-10s  [37mĞèÒª·¨Á¦: [32m%-6d [m             ",n,p[n].name,p[n].needmp);
      prints(buf);
  }
  n++;
 }  
 n-=1;
 
 do{
   move(16,4);
   sprintf(buf,"ÄãÏëÊ¹ÓÃÄÇÒ»¸ö%8sÄØ?  [Q]·ÅÆú:",p[0].name);
#ifdef MAPLE
   getdata(16,4,buf, ans, 2, 1, 0);
#else
   getdata(16,4,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
   if( ans[0]!='q' && ans[0]!='Q')
   {
     pipkey=atoi(ans);
   }
 }while( ans[0]!='q' && ans[0]!='Q' && (pipkey>n || pipkey <=0));
 
 if(ans[0]!='q' && ans[0]!='Q')
 {
#ifdef MAPLE
   getdata(17,4,"È·¶¨Ê¹ÓÃÂğ? [Y/n]", ans, 2, 1, 0);
#else
   getdata(17,4,"È·¶¨Ê¹ÓÃÂğ? [Y/n]", ans, 2, DOECHO, YEA);
#endif  // END MAPLE
   if(ans[0]!='n' && ans[0]!='N')
   {
     if(p[pipkey].hpmode==1)
     {
      d.hp+=p[pipkey].hp;
      d.tired-=p[pipkey].tired;      
      d.mp-=p[pipkey].needmp;
      if(d.hp>d.maxhp)
        d.hp=d.maxhp;
      if(d.tired<0)
        d.tired=0;
      injure=0;
     }
     else if(p[pipkey].hpmode==2)
     {
      d.hp=d.maxhp;
      d.mp-=p[pipkey].needmp;
      d.tired=0;
      injure=0;
     } 
     else
     {
      injure=(p[pipkey].hp-rand()%5);               
      d.mp-=p[pipkey].needmp;
     }
     d.mexp+=rand()%3+pipkey;
   }
   else
   {
     d.nodone=1;
     injure=0;
   }
 }
 else
 {
   d.nodone=1;
   injure=0; 
 }
 return injure;
}
