/* ----------------------------------- */
/* pip.c  ÑøĞ¡¼¦³ÌÊ½                   */
/* Ô­×÷Õß: dsyan   ¸ÄĞ´Õß: fennet      */
/* Í¼Í¼ by tiball.bbs@bbs.nhctc.edu.tw */
/* ----------------------------------- */

//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)
//#define pressanykey(a) prints(a);pressanykey();

#include "bbs.h"
#include <time.h>
#include "pip.h"
struct chicken d;
time_t start_time;
time_t lasttime;

#ifndef MAPLE
extern char BoardName[];
#endif  // END MAPLE
void temppress(char *s)
{
	move(23, 0); clrtoeol();
	prints(s);
	egetch();
}

/*ÓÎÏ·Ö÷³ÌÊ½*/
//int p_pipple()
int mod_default()
{
 FILE *fs;
 time_t now;
 long smoney;
 int pipkey;
 int ok;
 char genbuf[200];

#ifdef MAPLE
 setutmpmode(CHICKEN);
 more("src/maple/pipgame/pip.welcome",YEA);
#else
 modify_user_mode( CHICKEN );
 refresh();
 move(1,0);
 clrtobot();
 //rawmore("game/pipgame/pip.welcome",YEA,0,0,MM_FILE); 
//ansimore("game/pipgame/pip.welcome", NA);
//egetch();
#endif  // END MAPLE
 showtitle("µç×ÓÑøĞ¡¼¦", BoardName);
 srandom(time(0));
#ifdef MAPLE
 sprintf(genbuf,"home/%s/new_chicken",cuser.userid);
#else
 sprintf(genbuf,"home/%c/%s/new_chicken",toupper(cuser.userid[0]),cuser.userid);
#endif  // END MAPLE
 
 pip_read_file();
 if((fs=fopen(genbuf, "r")) == NULL)
 {
//   show_system_pic(11); /* ÔİÊ±ÓÃ½øÓÎÏ·µÄ»­ÃæÀ´´úÌæ */
 //  move(b_lines,0);
ansimore("game/pipgame/pip.welcome", NA);
   pipkey=egetch();   
   if(pipkey=='Q' || pipkey=='q')
     return 0;
   if(d.death!=0 || !d.name[0])
   {
       if(!pip_new_game()) return 0;
   }      
 }
 else
 {
//   show_system_pic(12);
 //  move(b_lines,0);
ansimore("game/pipgame/pip.welcome", NA);
   pipkey=egetch();   
   if(pipkey=='R' || pipkey=='r')
     pip_read_backup();
   else if(pipkey=='Q' || pipkey=='q')
     return 0;
   if(d.death!=0 || !d.name[0])
     {
       if(!pip_new_game()) return 0;
     }
 }
 
 lasttime=time(0);
 start_time=time(0);
 /*pip_do_menu(0,0,pipmainlist);*/
 pip_main_menu();
 d.bbtime+=time(0)-start_time;
 pip_write_file();
 return 0;
}

/*Ê±¼ä±íÊ¾·¨*/
char*
dsyan_time(const time_t *t)
{
  struct tm *tp;
  static char ans[9];

  tp = localtime(t);
  sprintf(ans, "%02d/%02d/%02d", tp->tm_year%100, tp->tm_mon + 1,tp->tm_mday);
  return ans;
}

/*ĞÂÓÎÏ·µÄÉè¶¨*/
int
pip_new_game()
{
  char buf[256];
  time_t now;
  char *pipsex[3]={"£¿","¡á","¡â"};
  struct tm *ptime;
  ptime = localtime(&now);
  
  if(d.death==1 && !(!d.name[0]))
  {
     clear();
     showtitle("ÍâĞÇÕ½¶·¼¦", BoardName); 
     move(4,6);
     prints("»¶Ó­À´µ½ [1;5;33mĞÇ¿ÕÉúÎï¿Æ¼¼ÑĞ¾¿Ôº[0m");
     move(6,6);
     prints("¾­ÎÒÃÇµ÷²éÏÔÊ¾  ÏÈÇ°ÄãÓĞÑø¹ıĞ¡¼¦à¸  ¿ÉÊÇ±»ÄãÑøËÀÁË...");
     move(8,6);
     if(d.liveagain<4)
     {
       prints("ÎÒÃÇ¿ÉÒÔ°ïÄã°ïĞ¡¼¦¸´»î  µ«ÊÇĞèÒª¸¶³öÒ»µã´ú¼Û");
#ifdef MAPLE
       getdata(10, 6, "ÄãÒªÎÒÃÇÈÃËûÖØÉúÂğ? [y/N]:", buf, 2, 1, 0);
#else
       getdata(10, 6, "ÄãÒªÎÒÃÇÈÃËûÖØÉúÂğ? [y/N]:", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
       if(buf[0]=='y' || buf[0]=='Y')
       {
         pip_live_again();
       }
     }
     else if(d.liveagain>=4)
     {
       prints("¿ÉÊÇÄã¸´»îÊÖÊõÌ«¶à´ÎÁË  Ğ¡¼¦ÉíÉÏ¶¼ÊÇ¿ªµ¶ºÛ¼£");     
       move(10,6);
       prints("ÎÒÃÇÕÒ²»µ½¿ÉÒÔÊÖÊõµÄµØ·½ÁË  ËùÒÔ....");
       pressanykey("ÖØĞÂÔÙÀ´°É....°¦....");    
     }
  }
  if(d.death!=0 || !d.name[0])
  {
    clear();
    showtitle("ÍâĞÇÕ½¶·¼¦", BoardName);   
    /*Ğ¡¼¦ÃüÃû*/
    strcpy(buf,"±´±´");
#ifdef MAPLE
    getdata(2, 3, "°ïĞ¡¼¦È¡¸öºÃÌıµÄÃû×Ö°É(Çë²»ÒªÓĞ¿Õ¸ñ):", buf, 11, 1, 0);
#else
    getdata(2, 3, "°ïĞ¡¼¦È¡¸öºÃÌıµÄÃû×Ö°É(Çë²»ÒªÓĞ¿Õ¸ñ):", buf, 11, DOECHO, NA);
#endif  // END MAPLE
    if(!buf[0]) return 0;
    strcpy(d.name,buf);
    /*1:¹« 2:Ä¸ */
#ifdef MAPLE
    getdata(4, 3, "[Boy]Ğ¡¹«¼¦¡á or [Girl]Ğ¡Ä¸¼¦¡â [b/G]", buf, 2, 1, 0);
#else
    getdata(4, 3, "[Boy]Ğ¡¹«¼¦¡á or [Girl]Ğ¡Ä¸¼¦¡â [b/G]", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(buf[0]=='b' || buf[0]=='B')
    {
      d.sex=1;
    }  
    else
    {
      d.sex=2; 
    }        
    move(6,3);
    prints("ĞÇ¿ÕÕ½¶·¼¦µÄÓÎÏ·ÏÖ½ñ·Ö³ÉÁ½ÖÖÍæ·¨");
    move(7,3);
    prints("Ñ¡ÓĞ½á¾Ö»áÔÚĞ¡¼¦20ËêÊ±½áÊøÓÎÏ·£¬²¢¸æÖªĞ¡¼¦ááĞøµÄ·¢Õ¹");
    move(8,3);
    prints("Ñ¡Ã»ÓĞ½á¾ÖÔòÒ»Ö±Ñøµ½Ğ¡¼¦ËÀÍö²Å½áÊøÓÎÏ·....");
    /*1:²»ÒªÇÒÎ´»é 4:ÒªÇÒÎ´»é */
#ifdef MAPLE
    getdata(9, 3, "ÄãÏ£ÍûĞ¡¼¦ÓÎÏ·ÊÇ·ñÒªÓĞ20Ëê½á¾Ö? [Y/n]", buf, 2, 1, 0);
#else
    getdata(9, 3, "ÄãÏ£ÍûĞ¡¼¦ÓÎÏ·ÊÇ·ñÒªÓĞ20Ëê½á¾Ö? [Y/n]", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(buf[0]=='n' || buf[0]=='N')
    {
      d.wantend=1;
    }  
    else
    {
      d.wantend=4; 
    }        
    /*¿ªÍ·»­Ãæ*/
    show_basic_pic(0);
    pressanykey("Ğ¡¼¦ÖÕì¶µ®ÉúÁË£¬ÇëºÃºÃ°®Ëû....");

    /*¿ªÍ·Éè¶¨*/
    now=time(0);
    strcpy(d.birth,dsyan_time(&now));
    d.bbtime=0;

    /*»ù±¾×ÊÁÏ*/
    d.year=ptime->tm_year%100;
    d.month=ptime->tm_mon + 1;
    d.day=ptime->tm_mday;
    d.death=d.nodone=d.relation=0;
    d.liveagain=d.dataB=d.dataC=d.dataD=d.dataE=0;
          
    /*ÉíÌå²ÎÊı*/
    d.hp=rand()%15+20;
    d.maxhp=rand()%20+20;
    if(d.hp>d.maxhp) d.hp=d.maxhp;
    d.weight=rand()%10+50;
    d.tired=d.sick=d.shit=d.wrist=0;
    d.bodyA=d.bodyB=d.bodyC=d.bodyD=d.bodyE=0;
  
    /*ÆÀ¼Û²ÎÊı*/
    d.social=d.family=d.hexp=d.mexp=0;
    d.tmpA=d.tmpB=d.tmpC=d.tmpD=d.tmpE=0;
         
    /*Õ½¶·²ÎÊı*/
    d.mp=d.maxmp=d.attack=d.resist=d.speed=d.hskill=d.mskill=d.mresist=0;
    d.magicmode=d.fightB=d.fightC=d.fightD=d.fightE=0;
  
    /*ÎäÆ÷²ÎÊı*/
    d.weaponhead=d.weaponrhand=d.weaponlhand=d.weaponbody=d.weaponfoot=0;
    d.weaponA=d.weaponB=d.weaponC=d.weaponD=d.weaponE=0;
    
    /*ÄÜÁ¦²ÎÊı*/
    d.toman=d.character=d.love=d.wisdom=d.art=d.etchics=0;
    d.brave=d.homework=d.charm=d.manners=d.speech=d.cookskill=0;
    d.learnA=d.learnB=d.learnC=d.learnD=d.learnE=0;
  
    /*×´Ì¬ÊıÖµ*/
    d.happy=rand()%10+20;
    d.satisfy=rand()%10+20;
    d.fallinlove=d.belief=d.offense=d.affect=0;
    d.stateA=d.stateB=d.stateC=d.stateD=d.stateE=0;

    /*Ê³Îï²ÎÊı:Ê³Îï ÁãÊ³ Ò©Æ· ´ó²¹Íè*/
    d.food=10;
    d.medicine=d.cookie=d.bighp=2;
    d.ginseng=d.snowgrass=d.eatC=d.eatD=d.eatE=0;

    /*ÎïÆ·²ÎÊı:Êé Íæ¾ß*/
    d.book=d.playtool=0;
    d.money=1500;
    d.thingA=d.thingB=d.thingC=d.thingD=d.thingE=0;

    /*²ÂÈ­²ÎÊı:Ó® ¸º*/
    d.winn=d.losee=0;

    /*²Î¼ûÍõ³¼*/
    d.royalA=d.royalB=d.royalC=d.royalD=d.royalE=0;
    d.royalF=d.royalG=d.royalH=d.royalI=d.royalJ=0;
    d.seeroyalJ=1;
    d.seeA=d.seeB=d.seeC=d.seeD=d.seeE;
    /*½ÓÊÜÇó»é°®ÈË*/        
    d.lover=0;
    /*0:Ã»ÓĞ 1:Ä§Íõ 2:Áú×å 3:A 4:B 5:C 6:D 7:E */
    d.classA=d.classB=d.classC=d.classD=d.classE=0;
    d.classF=d.classG=d.classH=d.classI=d.classJ=0;
    d.classK=d.classL=d.classM=d.classN=d.classO=0;

    d.workA=d.workB=d.workC=d.workD=d.workE=0;
    d.workF=d.workG=d.workH=d.workI=d.workJ=0;
    d.workK=d.workL=d.workM=d.workN=d.workO=0;
    d.workP=d.workQ=d.workR=d.workS=d.workT=0;
    d.workU=d.workV=d.workW=d.workX=d.workY=d.workZ=0;
    /*Ñø¼¦¼ÇÂ¼*/
    now=time(0);
    sprintf(buf, "[1;36m%s %-11sÑøÁËÒ»Ö»½Ğ [%s] µÄ %s Ğ¡¼¦ [0m\n", Cdate(&now), cuser.userid,d.name,pipsex[d.sex]);
    pip_log_record(buf);
  }  
  pip_write_file();
  return 1;
}

/*Ğ¡¼¦ËÀÍöº¯Ê½*/
pipdie(msg,mode)
char *msg;
int mode;
{
 char buf[100];
 char genbuf[200];
 time_t now;
 clear();
 showtitle("µç×ÓÑøĞ¡¼¦", BoardName); 
 if(mode==1)
 {
   show_die_pic(1);
   pressanykey("ËÀÉñÀ´´ø×ßĞ¡¼¦ÁË");
   clear();
   showtitle("µç×ÓÑøĞ¡¼¦", BoardName); 
   show_die_pic(2);
   move(14,20);
   prints("¿ÉÁ¯µÄĞ¡¼¦[1;31m%s[m",msg);
   pressanykey("ĞÇ¿Õ°§µ¿ÖĞ....");
 }
 else if(mode==2)
 {
   show_die_pic(3);
   pressanykey("ÎØÎØÎØ..ÎÒ±»¶ªÆúÁË.....");
 } 
 else if(mode==3)
 {
   show_die_pic(0);
   pressanykey("ÓÎÏ·½áÊøÂŞ.."); 
 }
 
 now=time(0);
 sprintf(genbuf, "[1;31m%s %-11sµÄĞ¡¼¦ [%s] %s[m\n", Cdate(&now), cuser.userid,d.name, msg);
 pip_log_record(genbuf);
 pip_write_file();
}


/*pro:»úÂÊ base:µ×Êı mode:ÀàĞÍ mul:¼ÓÈ¨100=1 cal:¼Ó¼õ*/
int
count_tired(prob,base,mode,mul,cal)
int prob,base;
char *mode;
int mul;
int cal;
{
 int tiredvary=0;
 int tm;
 /*time_t now;*/
 tm=(time(0)-start_time+d.bbtime)/60/30;
 if(mode=="Y")
 {
  if(tm>=0 && tm <=3)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp)*120/100;
     else if(cal==0)
        tiredvary=(rand()%prob+base)*4/3;
  }
  else if(tm >=4 && tm <=7)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp);
     else if(cal==0)
        tiredvary=(rand()%prob+base)*3/2;
  }
  else if(tm >=8 && tm <=10)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp)*110/100;
     else if(cal==0)
        tiredvary=(rand()%prob+base)*5/4;
  }
  else if(tm >=11)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp)*150/100;
     else if(cal==0)
        tiredvary=(rand()%prob+base)*1;
  }
 }
 else if(mode=="N")
 {
  tiredvary=rand()%prob+base;
 }

 if(cal==1)
 {
   d.tired+=(tiredvary*mul/100);
   if(d.tired>100)
     d.tired=100;
 }
 else if(cal==0)
 {
   d.tired-=(tiredvary*mul/100);
   if(d.tired<=0)
     {d.tired=0;}
 }
 tiredvary=0;
 return;
}



