
/*---------------------------------------------------------------------------*/
/*Ğ¡¼¦µµ°¸µÄ¶ÁĞ´º¯Ê½							     */
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

/*ÓÎÏ·Ğ´×ÊÁÏÈëµµ°¸*/
pip_write_file()
{
 FILE *ff;
 char buf[200];
#ifdef MAPLE
 sprintf(buf,"home/%s/new_chicken",cuser.userid);
#else
 sprintf(buf,"home/%c/%s/new_chicken",toupper(cuser.userid[0]),cuser.userid);
#endif  // END MAPLE

 if(ff=fopen(buf,"w"))
 {
  fwrite(&d,sizeof(d),1,ff);
/*
  fprintf(ff, "%lu\n", d.bbtime);
  fprintf(ff,
  "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
  d.year,d.month,d.day,d.sex,d.death,d.nodone,d.relation,d.liveagain,d.dataB,d.dataC,d.dataD,d.dataE,
  d.hp,d.maxhp,d.weight,d.tired,d.sick,d.shit,d.wrist,d.bodyA,d.bodyB,d.bodyC,d.bodyD,d.bodyE,
  d.social,d.family,d.hexp,d.mexp,d.tmpA,d.tmpB,d.tmpC,d.tmpD,d.tmpE,
  d.mp,d.maxmp,d.attack,d.resist,d.speed,d.hskill,d.mskill,d.mresist,d.magicmode,d.fightB,d.fightC,d.fightD,d.fightE,
  d.weaponhead,d.weaponrhand,d.weaponlhand,d.weaponbody,d.weaponfoot,d.weaponA,d.weaponB,d.weaponC,d.weaponD,d.weaponE,
  d.toman,d.character,d.love,d.wisdom,d.art,d.etchics,d.brave,d.homework,d.charm,d.manners,d.speech,d.cookskill,d.learnA,d.learnB,d.learnC,d.learnD,d.learnE,
  d.happy,d.satisfy,d.fallinlove,d.belief,d.offense,d.affect,d.stateA,d.stateB,d.stateC,d.stateD,d.stateE,
  d.food,d.medicine,d.bighp,d.cookie,d.ginseng,d.snowgrass,d.eatC,d.eatD,d.eatE,
  d.book,d.playtool,d.money,d.thingA,d.thingB,d.thingC,d.thingD,d.thingE,
  d.winn,d.losee,
  d.royalA,d.royalB,d.royalC,d.royalD,d.royalE,d.royalF,d.royalG,d.royalH,d.royalI,d.royalJ,d.seeroyalJ,d.seeA,d.seeB,d.seeC,d.seeD,d.seeE,
  d.wantend,d.lover,d.name,
  d.classA,d.classB,d.classC,d.classD,d.classE,
  d.classF,d.classG,d.classH,d.classI,d.classJ,
  d.classK,d.classL,d.classM,d.classN,d.classO,
  d.workA,d.workB,d.workC,d.workD,d.workE,
  d.workF,d.workG,d.workH,d.workI,d.workJ,
  d.workK,d.workL,d.workM,d.workN,d.workO,
  d.workP,d.workQ,d.workR,d.workS,d.workT,
  d.workU,d.workV,d.workW,d.workX,d.workY,d.workZ
  );
*/
  fclose(ff);
 }
}

/*ÓÎÏ·¶Á×ÊÁÏ³öµµ°¸*/
pip_read_file()
{
 FILE *fs;
 char buf[200];
#ifdef MAPLE
 sprintf(buf,"home/%s/new_chicken",cuser.userid);
#else
 sprintf(buf,"home/%c/%s/new_chicken",toupper(cuser.userid[0]),cuser.userid);
#endif  // END MAPLE
 if(fs=fopen(buf,"r"))
 {
   fread(&d,sizeof(d),1,fs);
/*
  fgets(buf, 80, fs);
  d.bbtime = (time_t) atol(buf);

  fscanf(fs,
  "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
  &(d.year),&(d.month),&(d.day),&(d.sex),&(d.death),&(d.nodone),&(d.relation),&(d.liveagain),&(d.dataB),&(d.dataC),&(d.dataD),&(d.dataE),
  &(d.hp),&(d.maxhp),&(d.weight),&(d.tired),&(d.sick),&(d.shit),&(d.wrist),&(d.bodyA),&(d.bodyB),&(d.bodyC),&(d.bodyD),&(d.bodyE),
  &(d.social),&(d.family),&(d.hexp),&(d.mexp),&(d.tmpA),&(d.tmpB),&(d.tmpC),&(d.tmpD),&(d.tmpE),
  &(d.mp),&(d.maxmp),&(d.attack),&(d.resist),&(d.speed),&(d.hskill),&(d.mskill),&(d.mresist),&(d.magicmode),&(d.fightB),&(d.fightC),&(d.fightD),&(d.fightE),
  &(d.weaponhead),&(d.weaponrhand),&(d.weaponlhand),&(d.weaponbody),&(d.weaponfoot),&(d.weaponA),&(d.weaponB),&(d.weaponC),&(d.weaponD),&(d.weaponE),
  &(d.toman),&(d.character),&(d.love),&(d.wisdom),&(d.art),&(d.etchics),&(d.brave),&(d.homework),&(d.charm),&(d.manners),&(d.speech),&(d.cookskill),&(d.learnA),&(d.learnB),&(d.learnC),&(d.learnD),&(d.learnE),
  &(d.happy),&(d.satisfy),&(d.fallinlove),&(d.belief),&(d.offense),&(d.affect),&(d.stateA),&(d.stateB),&(d.stateC),&(d.stateD),&(d.stateE),
  &(d.food),&(d.medicine),&(d.bighp),&(d.cookie),&(d.ginseng),&(d.snowgrass),&(d.eatC),&(d.eatD),&(d.eatE),
  &(d.book),&(d.playtool),&(d.money),&(d.thingA),&(d.thingB),&(d.thingC),&(d.thingD),&(d.thingE),
  &(d.winn),&(d.losee),
  &(d.royalA),&(d.royalB),&(d.royalC),&(d.royalD),&(d.royalE),&(d.royalF),&(d.royalG),&(d.royalH),&(d.royalI),&(d.royalJ),&(d.seeroyalJ),&(d.seeA),&(d.seeB),&(d.seeC),&(d.seeD),&(d.seeE),
  &(d.wantend),&(d.lover),d.name,
  &(d.classA),&(d.classB),&(d.classC),&(d.classD),&(d.classE),
  &(d.classF),&(d.classG),&(d.classH),&(d.classI),&(d.classJ),
  &(d.classK),&(d.classL),&(d.classM),&(d.classN),&(d.classO),
  &(d.workA),&(d.workB),&(d.workC),&(d.workD),&(d.workE),
  &(d.workF),&(d.workG),&(d.workH),&(d.workI),&(d.workJ),
  &(d.workK),&(d.workL),&(d.workM),&(d.workN),&(d.workO),
  &(d.workP),&(d.workQ),&(d.workR),&(d.workS),&(d.workT),
  &(d.workU),&(d.workV),&(d.workW),&(d.workX),&(d.workY),&(d.workZ)
  );  

*/
  fclose(fs);
 }
}

/*¼ÇÂ¼µ½pip.logµµ*/
int 
pip_log_record(msg)
char *msg;
{
  FILE *fs;
  
  //fs=fopen("log/pip.log","a+");
  fs=fopen("game/pipgame/pip.log","a+");
  fprintf(fs,"%s",msg);
  fclose(fs);
}

/*Ğ¡¼¦½ø¶È´¢´æ*/
int
pip_write_backup()
{
 char *files[4]={"Ã»ÓĞ","½ø¶ÈÒ»","½ø¶È¶ş","½ø¶ÈÈş"};
 char buf[200],buf1[200];
 char ans[3];
 int num=0;
 int pipkey;

 show_system_pic(21);
 pip_write_file();
 do{ 
    move(b_lines-2, 0);
    clrtoeol();
    move(b_lines-1, 0);
    clrtoeol();
    move(b_lines-1,1);
    prints("´¢´æ [1]½ø¶ÈÒ» [2]½ø¶È¶ş [3]½ø¶ÈÈş [Q]·ÅÆú [1/2/3/Q]£º");
    pipkey=egetch();
    
    if (pipkey=='1')
      num=1;
    else if(pipkey=='2') 
      num=2;
    else if(pipkey=='3') 
      num=3;     
    else
      num=0;
    
 }while(pipkey!='Q' && pipkey!='q' && num!=1 && num!=2 && num!=3);
 if(pipkey=='q' ||pipkey=='Q')
 {
    pressanykey("·ÅÆú´¢´æÓÎÏ·½ø¶È");
    return 0;
 }  
 move(b_lines-2, 1);
 prints("´¢´æµµ°¸»á¸²¸Ç´æ´¢´æì¶ [%s] µÄĞ¡¼¦µÄµµ°¸à¸£¡Çë¿¼ÂÇÇå³ş...",files[num]);
 sprintf(buf1,"È·¶¨Òª´¢´æì¶ [%s] µµ°¸Âğ£¿ [y/N]:",files[num]);
#ifdef MAPLE
 getdata(b_lines-1, 1,buf1, ans, 2, 1, 0);
#else
 getdata(b_lines-1, 1,buf1, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
 if (ans[0]!='y'&&ans[0]!='Y') 
 {
    pressanykey("·ÅÆú´¢´æµµ°¸");
    return 0;
 }
 
 move(b_lines-1,0);
 clrtobot();
 sprintf(buf1,"´¢´æ [%s] µµ°¸Íê³ÉÁË",files[num]);
 pressanykey(buf1);
#ifdef MAPLE
 sprintf(buf,"/bin/cp home/%s/new_chicken home/%s/new_chicken.bak%d",cuser.userid,cuser.userid,num);
#else
 sprintf(buf,"/bin/cp home/%c/%s/new_chicken home/%c/%s/new_chicken.bak%d",toupper(cuser.userid[0]),cuser.userid,toupper(cuser.userid[0]),cuser.userid,num);
#endif  // END MAPLE
 system(buf);
 return 0;
}

int
pip_read_backup()
{
 char buf[200],buf1[200],buf2[200];
 char *files[4]={"Ã»ÓĞ","½ø¶ÈÒ»","½ø¶È¶ş","½ø¶ÈÈş"};
 char ans[3];
 int pipkey;
 int num=0;
 int ok=0;
 FILE *fs;
 show_system_pic(22);
 do{ 
    move(b_lines-2, 0);
    clrtoeol();
    move(b_lines-1, 0);
    clrtoeol();
    move(b_lines-1,1);
    prints("¶ÁÈ¡ [1]½ø¶ÈÒ» [2]½ø¶È¶ş [3]½ø¶ÈÈş [Q]·ÅÆú [1/2/3/Q]£º");
    pipkey=egetch();
    
    if (pipkey=='1')
      num=1;
    else if(pipkey=='2') 
      num=2;
    else if(pipkey=='3') 
      num=3;     
    else
      num=0;
    
    if(num>0)
    {
#ifdef MAPLE
      sprintf(buf,"home/%s/new_chicken.bak%d",cuser.userid,num);
#else
      sprintf(buf,"home/%c/%s/new_chicken.bak%d",toupper(cuser.userid[0]),cuser.userid,num);
#endif  // END MAPLE
      if((fs=fopen(buf,"r")) == NULL)
      {
        sprintf(buf,"µµ°¸ [%s] ²»´æÔÚ",files[num]);
        pressanykey(buf);
        ok=0;
      }
      else 
      {
         
	 move(b_lines-2, 1);
	 prints("¶ÁÈ¡³öµµ°¸»á¸²¸ÇÏÖÔÚÕıÔÚÍæµÄĞ¡¼¦µÄµµ°¸à¸£¡Çë¿¼ÂÇÇå³ş...");
	 sprintf(buf,"È·¶¨Òª¶ÁÈ¡³ö [%s] µµ°¸Âğ£¿ [y/N]:",files[num]);
#ifdef MAPLE
	 getdata(b_lines-1, 1,buf, ans, 2, 1, 0);
#else
         getdata(b_lines-1, 1,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
	 if (ans[0]!='y'&&ans[0]!='Y') {
	    pressanykey("ÈÃÎÒÔÙ¾ö¶¨Ò»ÏÂ...");
	 } else ok=1;
      }
    }
 }while(pipkey!='Q' && pipkey!='q' && ok!=1);
 if(pipkey=='q' ||pipkey=='Q')
 {
    pressanykey("»¹ÊÇÍæÔ­±¾µÄÓÎÏ·");
    return 0;
 }
 
 move(b_lines-1,0);
 clrtobot();
 sprintf(buf,"¶ÁÈ¡ [%s] µµ°¸Íê³ÉÁË",files[num]);
 pressanykey(buf);

#ifdef MAPLE
 sprintf(buf1,"/bin/touch home/%s/new_chicken.bak%d",cuser.userid,num);
 sprintf(buf2,"/bin/cp home/%s/new_chicken.bak%d home/%s/new_chicken",cuser.userid,num,cuser.userid);
#else
 sprintf(buf1,"/bin/touch home/%c/%s/new_chicken.bak%d",toupper(cuser.userid[0]),cuser.userid,num);
 sprintf(buf2,"/bin/cp home/%c/%s/new_chicken.bak%d home/%c/%s/new_chicken",toupper(cuser.userid[0]),cuser.userid,num,toupper(cuser.userid[0]),cuser.userid);
#endif  // END MAPLE
 system(buf1);
 system(buf2);
 pip_read_file();
 return 0;
}



int
pip_live_again()
{
   char genbuf[80];
   time_t now;
   int tm;
   
   tm=(d.bbtime)/60/30;

   clear();
   showtitle("Ğ¡¼¦¸´»îÊÖÊõÖĞ", BoardName);

   now = time(0);
   sprintf(genbuf, "[1;33m%s %-11sµÄĞ¡¼¦ [%s¶ş´ú] ¸´»îÁË£¡[m\n", Cdate(&now), cuser.userid,d.name);
   pip_log_record(genbuf);
   
   /*ÉíÌåÉÏµÄÉè¶¨*/
   d.death=0;
   d.maxhp=d.maxhp*3/4+1;
   d.hp=d.maxhp/2+1;
   d.tired=20;
   d.shit=20;
   d.sick=20;   
   d.wrist=d.wrist*3/4;
   d.weight=45+10*tm;
   
   /*Ç®¼õµ½Îå·ÖÖ®Ò»*/
   d.money=d.money/5;
   
   /*Õ½¶·ÄÜÁ¦½µÒ»°ë*/
   d.attack=d.attack*3/4;
   d.resist=d.resist*3/4;
   d.maxmp=d.maxmp*3/4;
   d.mp=d.maxmp/2;
   
   /*±äµÄ²»¿ìÀÖ*/
   d.happy=0;
   d.satisfy=0;
   
   /*ÆÀ¼Û¼õ°ë*/
   d.social=d.social*3/4;
   d.family=d.family*3/4;
   d.hexp=d.hexp*3/4;
   d.mexp=d.mexp*3/4;

   /*ÎäÆ÷µô¹â¹â*/   
   d.weaponhead=0;
   d.weaponrhand=0;
   d.weaponlhand=0;
   d.weaponbody=0;
   d.weaponfoot=0;
   
   /*Ê³ÎïÊ£Ò»°ë*/
   d.food=d.food/2;
   d.medicine=d.medicine/2;
   d.bighp=d.bighp/2;
   d.cookie=d.cookie/2;

   d.liveagain+=1;
   
   pressanykey("Ğ¡¼¦Æ÷¹ÙÖØ½¨ÖĞ£¡");
   pressanykey("Ğ¡¼¦ÌåÖÊ»Ö¸´ÖĞ£¡");
   pressanykey("Ğ¡¼¦ÄÜÁ¦µ÷ÕûÖĞ£¡");
   pressanykey("¹§ìûÄú£¬ÄãµÄĞ¡¼¦ÓÖ¸´»îÂŞ£¡");
   pip_write_file();
   return 0;
}
