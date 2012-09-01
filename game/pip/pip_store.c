/*---------------------------------------------------------------------------*/
/* ÉÌµêÑ¡µ¥:Ê³Îï ÁãÊ³ ´ó²¹Íè Íæ¾ß Êé±¾                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;

#ifndef MAPLE
extern char BoardName[];
#endif  // END MAPLE
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

/*---------------------------------------------------------------------------*/
/* ÉÌµêÑ¡µ¥:Ê³Îï ÁãÊ³ ´ó²¹Íè Íæ¾ß Êé±¾                                       */
/* ×ÊÁÏ¿â                                                                    */
/*---------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  ÎïÆ·²ÎÊıÉè¶¨                                                            */
/*--------------------------------------------------------------------------*/
struct goodsofpip
{
   int num;		/*±àºÅ*/
   char *name;		/*Ãû×Ö*/
   char *msgbuy;	/*¹¦ÓÃ*/
   char *msguse;	/*ËµÃ÷*/
   int money;		/*½ğÇ®*/
   int change;		/*¸Ä±äÁ¿*/
   int pic1;
   int pic2;
};
typedef struct goodsofpip goodsofpip;

struct goodsofpip pipfoodlist[] = {
0,"ÎïÆ·Ãû",	"ËµÃ÷buy",	"ËµÃ÷feed",			0,	0,	0,0,
1,"ºÃ³ÔµÄÊ³Îï",	"ÌåÁ¦»Ö¸´50",	"Ã¿³ÔÒ»´ÎÊ³Îï»á»Ö¸´ÌåÁ¦50à¸!",	50,	50,	1,1,
2,"ÃÀÎ¶µÄÁãÊ³",	"ÌåÁ¦»Ö¸´100",	"³ıÁË»Ö¸´ÌåÁ¦£¬Ğ¡¼¦Ò²»á¸ü¿ìÀÖ",	120,	100,	2,3,
0,NULL,NULL,NULL,0,0,0,0
};

struct goodsofpip pipmedicinelist[] = {
0,"ÎïÆ·Ãû",	"ËµÃ÷buy",	"ËµÃ÷feed",			0,	0,	0,0,
1,"ºÃÓÃ´ó²¹Íè",	"ÌåÁ¦»Ö¸´600",	"»Ö¸´´óÁ¿Á÷Ê§ÌåÁ¦µÄÁ¼·½",	500,	600,	4,4,
2,"Õä¹óµÄÁéÖ¥",	"·¨Á¦»Ö¸´50",	"Ã¿³ÔÒ»´ÎÁéÖ¥»á»Ö¸´·¨Á¦50à¸!",	100,	50,	7,7,
3,"Ç§ÄêÈË²ÎÍõ",	"·¨Á¦»Ö¸´500",	"»Ö¸´´óÁ¿Á÷Ê§·¨Á¦µÄÁ¼·½",	800,	500,	7,7,
4,"ÌìÉ½Ñ©Á«",	"·¨Á¦ÌåÁ¦×î´ó",	"Õâ¸ö  ºÃ¹ó......",		10000,	0,	7,7,
0,NULL,NULL,NULL,0,0,0,0
};

struct goodsofpip pipotherlist[] = {
0,"ÎïÆ·Ãû",	"ËµÃ÷buy",	"ËµÃ÷feed",			0,	0,	0,0,
1,"ÀÖ¸ßÍæ¾ß×é",	"¿ìÀÖÂúÒâ¶È",	"Íæ¾ßÈÃĞ¡¼¦¸ü¿ìÀÖÀ²...",	50,	0,	5,5,
2,"°Ù¿ÆÈ«Êé",	"ÖªÊ¶µÄÀ´Ô´",	"Êé±¾ÈÃĞ¡¼¦¸ü´ÏÃ÷¸üÓĞÆøÖÊÀ²...",100,	0,	6,6,
0,NULL,NULL,NULL,0,0,0,0
};

/*--------------------------------------------------------------------------*/
/*  ÎäÆ÷²ÎÊıÉè¶¨                                                            */
/*--------------------------------------------------------------------------*/
struct weapon
{
  char *name;           /*Ãû×Ö*/  
  int needmaxhp;	/*ĞèÒªhp*/
  int needmaxmp;	/*ĞèÒªmp*/
  int needspeed;	/*ĞèÒªµÄspeed*/
  int attack;		/*¹¥»÷*/
  int resist;		/*·À»¤*/
  int speed;		/*ËÙ¶È*/
  int cost;		/*Âò¼Û*/
  int sell;		/*Âô¼Û*/
  int special;		/*ÌØ±ğ*/
  int map;		/*Í¼µµ*/

};
typedef struct weapon weapon;

/*Ãû×Ö,Ğèhp,Ğèmp,Ğèspeed,¹¥»÷,·À»¤,ËÙ¶È,Âò¼Û,Âô¼Û,ÌØ±ğ,Í¼µµ*/
struct weapon headlist[] = {
"²»Âò×°±¸",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"ËÜ½ºÃ±×Ó",  0,  0,  0,  0,  5,  0,   500,   300,0,0,	
"Å£Æ¤Ğ¡Ã±",  0,  0,  0,  0, 10,  0,  3500,  1000,0,0,
"  °²È«Ã±", 60,  0,  0,  0, 20,  0,  5000,  3500,0,0,
"¸ÖÌúÍ·¿ø",150, 50,  0,  0, 30,  0, 10000,  6000,0,0,
"Ä§·¨·¢¹¿",100,150,  0,  0, 25,  0, 50000, 10000,0,0, 
"»Æ½ğÊ¥¿ø",300,300,300,  0,100,  0,300000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,   0,   0,0,0
};

/*Ãû×Ö,Ğèhp,Ğèmp,Ğèspeed,¹¥»÷,·À»¤,ËÙ¶È,Âò¼Û,Âô¼Û,ÌØ±ğ,Í¼µµ*/
struct weapon rhandlist[] = {
"²»Âò×°±¸",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"´óÄ¾°ô",    0,  0,  0,  5,  0,  0,  1000,   700,0,0,	
"½ğÊô°âÊÖ",  0,  0,  0, 10,  0,  0,  2500,  1000,0,0,
"ÇàÍ­½£",   50,  0,  0, 20,  0,  0,  6000,  4000,0,0,
"ÇçÀ×½£",   80,  0,  0, 30,  0,  0, 10000,  8000,0,0,
"²õÒíµ¶",  100, 20,  0, 40,  0,  0, 15000, 10000,0,0, 
"ÍüÇé½£",  100, 40,  0, 35, 20,  0, 15000, 10000,0,0,
"Ê¨Í·±¦µ¶",150,  0,  0, 60,  0,  0, 35000, 20000,0,0,
"ÍÀÁúµ¶",  200,  0,  0,100,  0,  0, 50000, 25000,0,0,
"»Æ½ğÊ¥ÕÈ",300,300,300,100, 20,  0,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,    0,   0,0,0
};

/*Ãû×Ö,Ğèhp,Ğèmp,Ğèspeed,¹¥»÷,·À»¤,ËÙ¶È,Âò¼Û,Âô¼Û,ÌØ±ğ,Í¼µµ*/
struct weapon lhandlist[] = {
"²»Âò×°±¸",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"´óÄ¾°ô",    0,  0,  0,  5,  0,  0,  1000,   700,0,0,	
"½ğÊô°âÊÖ",  0,  0,  0, 10,  0,  0,  1500,  1000,0,0,
"Ä¾¶Ü",	     0,  0,  0,  0, 10,  0,  2000,  1500,0,0,
"²»Ğâ¸Ö¶Ü", 60,  0,  0,  0, 25,  0,  5000,  3000,0,0,
"°×½ğÖ®¶Ü", 80,  0,  0, 10, 40,  0, 15000, 10000,0,0,
"Ä§·¨¶Ü",   80,100,  0, 20, 60,  0, 80000, 50000,0,0,
"»Æ½ğÊ¥¶Ü",300,300,300, 30,100,  0,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,    0,   0,0,0
};

/*Ãû×Ö,Ğèhp,Ğèmp,Ğèspeed,¹¥»÷,·À»¤,ËÙ¶È,Âò¼Û,Âô¼Û,ÌØ±ğ,Í¼µµ*/
struct weapon bodylist[] = {
"²»Âò×°±¸",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"ËÜ½ºëĞ¼×", 40,  0,  0,  0,  5,  0,  1000,   700,0,0,	
"ÌØ¼¶Æ¤¼×", 50,  0,  0,  0, 10,  0,  2500,  1000,0,0,
"¸ÖÌú¿ø¼×", 80,  0,  0,  0, 25,  0,  5000,  3500,0,0,
"Ä§·¨Åû·ç", 80, 40,  0,  0, 20, 20, 15500, 10000,0,0,
"°×½ğ¿ø¼×",100, 30,  0,  0, 40, 20, 30000, 20000,0,0, 
"»Æ½ğÊ¥ÒÂ",300,300,300, 30,100,  0,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,     0,   0,0,0
};

/*Ãû×Ö,Ğèhp,Ğèmp,Ğèspeed,¹¥»÷,·À»¤,ËÙ¶È,Âò¼Û,Âô¼Û,ÌØ±ğ,Í¼µµ*/
struct weapon footlist[] = {
"²»Âò×°±¸",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"ËÜ½ºÍÏĞ¬",  0,  0,  0,  0,  0, 10,   800,   500,0,0,
"¶«ÑóÄ¾åì",  0,  0,  0, 15,  0, 10,  1000,   700,0,0, 	
"ÌØ¼¶ÓêĞ¬",  0,  0,  0,  0, 10, 10,  1500,  1000,0,0,
"NIKEÔË¶¯Ğ¬",70, 0,  0,  0, 10, 40,  8000,  5000,0,0,
"öùÓãÆ¤Ñ¥", 80, 20,  0, 10, 25, 20, 12000,  8000,0,0,
"·ÉÌìÄ§Ñ¥",100,100,  0, 30, 50, 60, 25000, 10000,0,0,
"»Æ½ğÊ¥Ñ¥",300,300,300, 50,100,100,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,    0,   0,0,0
};

/*---------------------------------------------------------------------------*/
/* ÉÌµêÑ¡µ¥:Ê³Îï ÁãÊ³ ´ó²¹Íè Íæ¾ß Êé±¾                                       */
/* º¯Ê½¿â                                                                    */
/*---------------------------------------------------------------------------*/

int pip_store_food()
{
    int num[3];
    num[0]=2;
    num[1]=d.food;
    num[2]=d.cookie;
    pip_buy_goods_new(1,pipfoodlist,num);
    d.food=num[1];
    d.cookie=num[2];
    return 0;
}

int pip_store_medicine()
{
    int num[5];
    num[0]=4;
    num[1]=d.bighp;
    num[2]=d.medicine;
    num[3]=d.ginseng;
    num[4]=d.snowgrass;
    pip_buy_goods_new(2,pipmedicinelist,num);
    d.bighp=num[1];
    d.medicine=num[2];
    d.ginseng=num[3];
    d.snowgrass=num[4];
    return 0;
}

int pip_store_other()
{
    int num[3];
    num[0]=2;
    num[1]=d.playtool;
    num[2]=d.book;
    pip_buy_goods_new(3,pipotherlist,num);
    d.playtool=num[1];
    d.book=num[2];
    return 0;
}

int pip_store_weapon_head()	/*Í·²¿ÎäÆ÷*/
{
     d.weaponhead=pip_weapon_doing_menu(d.weaponhead,0,headlist);
     return 0; 
}
int pip_store_weapon_rhand()	/*ÓÒÊÖÎäÆ÷*/
{
     d.weaponrhand=pip_weapon_doing_menu(d.weaponrhand,1,rhandlist);
     return 0;
}
int pip_store_weapon_lhand()    /*×óÊÖÎäÆ÷*/
{
     d.weaponlhand=pip_weapon_doing_menu(d.weaponlhand,2,lhandlist);
     return 0;
}
int pip_store_weapon_body()	/*ÉíÌåÎäÆ÷*/
{
     d.weaponbody=pip_weapon_doing_menu(d.weaponbody,3,bodylist);
     return 0;
}
int pip_store_weapon_foot()     /*×ã²¿ÎäÆ÷*/
{
     d.weaponfoot=pip_weapon_doing_menu(d.weaponfoot,4,footlist);
     return 0;
}


int 
pip_buy_goods_new(mode,p,oldnum)
int mode;
int oldnum[];
struct goodsofpip *p;
{
    char *shopname[4]={"µêÃû","±ãÀûÉÌµê","ĞÇ¿ÕÒ©ÆÌ","Ò¹ÀïÊé¾Ö"};
    char inbuf[256];
    char genbuf[20];
    long smoney;
    int oldmoney;
    int i,pipkey,choice;
    oldmoney=d.money;
    do
    {
	    clrchyiuan(6,18);
	    move(6,0);
	    sprintf(inbuf,"[1;31m  ¡ª[41;37m ±àºÅ [0;1;31m¡ª[41;37m ÉÌ      Æ· [0;1;31m¡ª¡ª[41;37m Ğ§            ÄÜ [0;1;31m¡ª¡ª[41;37m ¼Û     ¸ñ [0;1;31m¡ª[37;41m ÓµÓĞÊıÁ¿ [0;1;31m¡ª[0m  ");
	    prints(inbuf);
	    for(i=1;i<=oldnum[0];i++)
	    {
		    move(7+i,0);
		    sprintf(inbuf,"     [1;35m[[37m%2d[35m]     [36m%-10s      [37m%-14s        [1;33m%-10d   [1;32m%-9d    [0m",
		    p[i].num,p[i].name,p[i].msgbuy,p[i].money,oldnum[i]);
		    prints(inbuf);
	    }
	    clrchyiuan(19,24);
	    move(b_lines,0); 
	    sprintf(inbuf,"[1;44;37m  %8sÑ¡µ¥  [46m  [B]ÂòÈëÎïÆ·  [S]Âô³öÎïÆ·  [Q]Ìø³ö£º                         [m",shopname[mode]);
	    prints(inbuf);
	    pipkey=egetch(); 
	    switch(pipkey)  
	    {
		case 'B':
		case 'b':      
			move(b_lines-1,1);
			sprintf(inbuf,"ÏëÒªÂòÈëÉ¶ÄØ? [0]·ÅÆúÂòÈë [1¡«%d]ÎïÆ·ÉÌºÅ",oldnum[0]);
#ifdef MAPLE
			getdata(b_lines-1,1,inbuf,genbuf, 3, LCECHO,"0");
#else
                        getdata(b_lines-1,1,inbuf,genbuf, 3, DOECHO,YEA);
                        if ((genbuf[0] >= 'A') && (genbuf[0] <= 'Z'))
                                genbuf[0] = genbuf[0] | 32;
#endif  // END MAPLE

     			choice=atoi(genbuf);
			if(choice>=1 && choice<=oldnum[0])
			{
				clrchyiuan(6,18);
				if(rand()%2>0)
					show_buy_pic(p[choice].pic1);
				else
					show_buy_pic(p[choice].pic2);
				move(b_lines-1,0);
				clrtoeol();  
				move(b_lines-1,1);       
				smoney=0;
				if(mode==3)
					smoney=1;
				else
				{
					sprintf(inbuf,"ÄãÒªÂòÈëÎïÆ· [%s] ¶àÉÙ¸öÄØ?(ÉÏÏŞ %d)",p[choice].name,d.money/p[choice].money);
#ifdef MAPLE
					getdata(b_lines-1,1,inbuf,genbuf,6, 1, 0);
#else
                                        getdata(b_lines-1,1,inbuf,genbuf,6, DOECHO, YEA);
#endif  // END MAPLE
					smoney=atoi(genbuf);
				}
				if(smoney<0)
				{
					pressanykey("·ÅÆúÂòÈë...");
				}
				else if(d.money<smoney*p[choice].money)
				{
					pressanykey("ÄãµÄÇ®Ã»ÓĞÄÇ÷á¶àà¸..");
				}
				else
				{
					sprintf(inbuf,"È·¶¨ÂòÈëÎïÆ· [%s] ÊıÁ¿ %d ¸öÂğ?(µê¼ÒÂô¼Û %d) [y/N]:",p[choice].name,smoney,smoney*p[choice].money);
#ifdef MAPLE
					getdata(b_lines-1,1,inbuf,genbuf, 2, 1, 0); 
#else
                                        getdata(b_lines-1,1,inbuf,genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
					if(genbuf[0]=='y' || genbuf[0]=='Y')
					{
						oldnum[choice]+=smoney;
						d.money-=smoney*p[choice].money;
						sprintf(inbuf,"ÀÏ°å¸øÁËÄã%d¸ö%s",smoney,p[choice].name);
						pressanykey(inbuf);
						pressanykey(p[choice].msguse);
						if(mode==3 && choice==1)
						{
							d.happy+=rand()%10+20*smoney;
							d.satisfy+=rand()%10+20*smoney;
						}
						if(mode==3 && choice==2)
						{
							d.happy+=(rand()%2+2)*smoney;
							d.wisdom+=(2+10/(d.wisdom/100+1))*smoney;
							d.character+=(rand()%4+2)*smoney;
							d.art+=(rand()%2+1)*smoney;
						}
					}
					else
					{
						pressanykey("·ÅÆúÂòÈë...");
					}
				}
			}
			else
			{
				sprintf(inbuf,"·ÅÆúÂòÈë.....");
				pressanykey(inbuf);            
			}
			break;     
     
 		case 'S':
 		case 's':
 			if(mode==3)
 			{
 				pressanykey("ÕâĞ©¶«Î÷²»ÄÜÂôà¸....");
 				break;
 			}
			move(b_lines-1,1);
			sprintf(inbuf,"ÏëÒªÂô³öÉ¶ÄØ? [0]·ÅÆúÂô³ö [1¡«%d]ÎïÆ·ÉÌºÅ",oldnum[0]);
#ifdef MAPLE
			getdata(b_lines-1,1,inbuf,genbuf, 3, LCECHO,"0");
#else
                        getdata(b_lines-1,1,inbuf,genbuf, 3, DOECHO,YEA);
                        if ((genbuf[0] >= 'A') && (genbuf[0] <= 'Z'))
                                genbuf[0] = genbuf[0] | 32;
#endif  // END MAPLE
     			choice=atoi(genbuf);
			if(choice>=1 && choice<=oldnum[0])
			{
				clrchyiuan(6,18);
				if(rand()%2>0)
					show_buy_pic(p[choice].pic1);
				else
					show_buy_pic(p[choice].pic2);
				move(b_lines-1,0);
				clrtoeol();  
				move(b_lines-1,1);       
				smoney=0;
				sprintf(inbuf,"ÄãÒªÂô³öÎïÆ· [%s] ¶àÉÙ¸öÄØ?(ÉÏÏŞ %d)",p[choice].name,oldnum[choice]);
#ifdef MAPLE
				getdata(b_lines-1,1,inbuf,genbuf,6, 1, 0); 
#else
                                getdata(b_lines-1,1,inbuf,genbuf,6, DOECHO, YEA);
#endif  // END MAPLE
				smoney=atoi(genbuf);
				if(smoney<0)
				{
					pressanykey("·ÅÆúÂô³ö...");
				}
				else if(smoney>oldnum[choice])
				{
					sprintf(inbuf,"ÄãµÄ [%s] Ã»ÓĞÄÇ÷á¶à¸öà¸",p[choice].name);
					pressanykey(inbuf);
				}
				else
				{
					sprintf(inbuf,"È·¶¨Âô³öÎïÆ· [%s] ÊıÁ¿ %d ¸öÂğ?(µê¼ÒÂò¼Û %d) [y/N]:",p[choice].name,smoney,smoney*p[choice].money*8/10);
#ifdef MAPLE
					getdata(b_lines-1,1,inbuf,genbuf, 2, 1, 0);
#else
                                        getdata(b_lines-1,1,inbuf,genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
					if(genbuf[0]=='y' || genbuf[0]=='Y')
					{
						oldnum[choice]-=smoney;
						d.money+=smoney*p[choice].money*8/10;
						sprintf(inbuf,"ÀÏ°åÄÃ×ßÁËÄãµÄ%d¸ö%s",smoney,p[choice].name);
						pressanykey(inbuf);
					}
					else
					{
						pressanykey("·ÅÆúÂô³ö...");
					}
				}
			}
			else
			{
				sprintf(inbuf,"·ÅÆúÂô³ö.....");
				pressanykey(inbuf);            
			}
			break;
		case 'Q':
		case 'q':
			sprintf(inbuf,"½ğÇ®½»Ò×¹² %d Ôª,Àë¿ª %s ",d.money-oldmoney,shopname[mode]);
			pressanykey(inbuf);
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
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));    
  return 0;
}

int
pip_weapon_doing_menu(variance,type,p)               /* ÎäÆ÷¹ºÂò»­Ãæ */
int variance;
int type;
struct weapon *p;
{
  time_t now;
  register int n=0;
  register char *s;
  char buf[256];
  char ans[5];
  char shortbuf[100];
  char menutitle[5][11]={"Í·²¿×°±¸Çø","ÓÒÊÖ×°±¸Çø","×óÊÖ×°±¸Çø","ÉíÌå×°±¸Çø","×ã²¿×°±¸Çø"};  
  int pipkey;
  char choicekey[5];
  int choice;
  
  do{
   clear();
   showtitle(menutitle[type], BoardName);
   show_weapon_pic(0);
/*   move(10,2); 
   sprintf(buf,"[1;37mÏÖ½ñÄÜÁ¦:ÌåÁ¦Max:[36m%-5d[37m  ·¨Á¦Max:[36m%-5d[37m  ¹¥»÷:[36m%-5d[37m  ·ÀÓù:[36m%-5d[37m  ËÙ¶È:[36m%-5d [m",
           d.maxhp,d.maxmp,d.attack,d.resist,d.speed);
   prints(buf);*/
   move(11,2);
   sprintf(buf,"[1;37;41m [NO]  [Æ÷¾ßÃû]  [ÌåÁ¦]  [·¨Á¦]  [ËÙ¶È]  [¹¥»÷]  [·ÀÓù]  [ËÙ¶È]  [ÊÛ  ¼Û] [m");
   prints(buf);
   move(12,2);
   sprintf(buf," [1;31m¡ª¡ª[37m°×É« ¿ÉÒÔ¹ºÂò[31m¡ª¡ª[32mÂÌÉ« ÓµÓĞ×°±¸[31m¡ª¡ª[33m»ÆÉ« Ç®Ç®²»¹»[31m¡ª¡ª[35m×ÏÉ« ÄÜÁ¦²»×ã[31m¡ª¡ª[m");
   prints(buf); 

   n=0;
   while (s = p[n].name)
   {   
     move(13+n,2);
     if(variance!=0 && variance==(n))/*±¾ÉíÓĞµÄ*/
     {
      sprintf(buf, 
      "[1;32m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",     
      n,p[n].name,p[n].needmaxhp,p[n].needmaxmp,p[n].needspeed,
      p[n].attack,p[n].resist,p[n].speed,p[n].cost);        
     }
     else if(d.maxhp < p[n].needmaxhp || d.maxmp < p[n].needmaxmp || d.speed < p[n].needspeed )/*ÄÜÁ¦²»×ã*/
     {
      sprintf(buf, 
      "[1;35m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
      n,p[n].name,p[n].needmaxhp,p[n].needmaxmp,p[n].needspeed,
      p[n].attack,p[n].resist,p[n].speed,p[n].cost);
     }

     else if(d.money < p[n].cost)  /*Ç®²»¹»µÄ*/
     {
      sprintf(buf, 
      "[1;33m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",     
      n,p[n].name,p[n].needmaxhp,p[n].needmaxmp,p[n].needspeed,
      p[n].attack,p[n].resist,p[n].speed,p[n].cost);    
     }    
     else
     {
      sprintf(buf, 
      "[1;37m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",     
      n,p[n].name,p[n].needmaxhp,p[n].needmaxmp,p[n].needspeed,
      p[n].attack,p[n].resist,p[n].speed,p[n].cost);        
     } 
     prints(buf);
     n++;
   }
   move(b_lines,0); 
   sprintf(buf,"[1;44;37m  ÎäÆ÷¹ºÂòÑ¡µ¥  [46m  [B]¹ºÂòÎäÆ÷  [S]Âôµô×°±¸  [W]¸öÈË×ÊÁÏ  [Q]Ìø³ö£º            [m");
   prints(buf);   
   now=time(0);
   pip_time_change(now);
   pipkey=egetch();
   pip_time_change(now);

   switch(pipkey)  
   {
    case 'B':      
    case 'b':
     move(b_lines-1,1);
     sprintf(shortbuf,"ÏëÒª¹ºÂòÉ¶ÄØ? ÄãµÄÇ®Ç®[%d]Ôª:[Êı×Ö]",d.money);
     prints(shortbuf);
#ifdef MAPLE
     getdata(b_lines-1,1,shortbuf,choicekey, 4, LCECHO,"0");
#else
     getdata(b_lines-1,1,shortbuf,choicekey, 4, DOECHO,YEA);
     if ((choicekey[0] >= 'A') && (choicekey[0] <= 'Z'))
             choicekey[0] = choicekey[0] | 32;
#endif  // END MAPLE
     choice=atoi(choicekey);
     if(choice>=0 && choice<=n)
     {
       move(b_lines-1,0);
       clrtoeol();  
       move(b_lines-1,1);       
       if(choice==0)     /*½â³ı*/
       { 
          sprintf(shortbuf,"·ÅÆú¹ºÂò...");
          pressanykey(shortbuf);
       }
      
       else if(variance==choice)  /*ÔçÒÑ¾­ÓĞÀ²*/
       {
          sprintf(shortbuf,"ÄãÔçÒÑ¾­ÓĞ %s ÂŞ",p[variance].name);
          pressanykey(shortbuf);      
       }
      
       else if(p[choice].cost >= (d.money+p[variance].sell))  /*Ç®²»¹»*/
       {
          sprintf(shortbuf,"Õâ¸öÒª %d Ôª£¬ÄãµÄÇ®²»¹»À²!",p[choice].cost);
          pressanykey(shortbuf);      
       }      
     
       else if(d.maxhp < p[choice].needmaxhp || d.maxmp < p[choice].needmaxmp 
               || d.speed < p[choice].needspeed ) /*ÄÜÁ¦²»×ã*/
       {
          sprintf(shortbuf,"ĞèÒªHP %d MP %d SPEED %d à¸",
                p[choice].needmaxhp,p[choice].needmaxmp,p[choice].needspeed);
          pressanykey(shortbuf);            
       }
       else  /*Ë³Àû¹ºÂò*/
       {
          sprintf(shortbuf,"ÄãÈ·¶¨Òª¹ºÂò %s Âğ?($%d) [y/N]",p[choice].name,p[choice].cost);
#ifdef MAPLE
          getdata(b_lines-1,1,shortbuf, ans, 2, 1, 0); 
#else
          getdata(b_lines-1,1,shortbuf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
          if(ans[0]=='y' || ans[0]=='Y')
          {
              sprintf(shortbuf,"Ğ¡¼¦ÒÑ¾­×°ÅäÉÏ %s ÁË",p[choice].name);
              pressanykey(shortbuf);
              d.attack+=(p[choice].attack-p[variance].attack);
              d.resist+=(p[choice].resist-p[variance].resist);
              d.speed+=(p[choice].speed-p[variance].speed);
              d.money-=(p[choice].cost-p[variance].sell);
              variance=choice;
          }
          else
          {
              sprintf(shortbuf,"·ÅÆú¹ºÂò.....");
              pressanykey(shortbuf);            
          }
       }
     }       
     break;     
     
   case 'S':
   case 's':
     if(variance!=0)
     { 
        sprintf(shortbuf,"ÄãÈ·¶¨ÒªÂôµô%sÂğ? Âô¼Û:%d [y/N]",p[variance].name,p[variance].sell);
#ifdef MAPLE
        getdata(b_lines-1,1,shortbuf, ans, 2, 1, 0); 
#else
        getdata(b_lines-1,1,shortbuf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
        if(ans[0]=='y' || ans[0]=='Y')
        {
           sprintf(shortbuf,"×°±¸ %s ÂôÁË %d",p[variance].name,p[variance].sell);
           d.attack-=p[variance].attack;
           d.resist-=p[variance].resist;
           d.speed-=p[variance].speed;
           d.money+=p[variance].sell;
           pressanykey(shortbuf);
           variance=0;
        }
        else
        {
           sprintf(shortbuf,"ccc..ÎÒ»ØĞÄ×ªÒâÁË...");
           pressanykey(shortbuf);         
        }
     }
     else if(variance==0)
     {
        sprintf(shortbuf,"Äã±¾À´¾ÍÃ»ÓĞ×°±¸ÁË...");
        pressanykey(shortbuf);
        variance=0;
     }
     break;
                      
   case 'W':
   case 'w':
     pip_data_list();   
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
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
    
  return variance;
}
