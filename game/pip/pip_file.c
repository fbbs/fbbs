
/*---------------------------------------------------------------------------*/
/*小鸡档案的读写函式							     */
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

/*游戏写资料入档案*/
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

/*游戏读资料出档案*/
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

/*记录到pip.log档*/
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

/*小鸡进度储存*/
int
pip_write_backup()
{
 //% char *files[4]={"没有","进度一","进度二","进度叁"};
 char *files[4]={"\xc3\xbb\xd3\xd0","\xbd\xf8\xb6\xc8\xd2\xbb","\xbd\xf8\xb6\xc8\xb6\xfe","\xbd\xf8\xb6\xc8\xc8\xfe"};
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
    //% prints("储存 [1]进度一 [2]进度二 [3]进度叁 [Q]放弃 [1/2/3/Q]：");
    prints("\xb4\xa2\xb4\xe6 [1]\xbd\xf8\xb6\xc8\xd2\xbb [2]\xbd\xf8\xb6\xc8\xb6\xfe [3]\xbd\xf8\xb6\xc8\xc8\xfe [Q]\xb7\xc5\xc6\xfa [1/2/3/Q]\xa3\xba");
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
    //% pressanykey("放弃储存游戏进度");
    pressanykey("\xb7\xc5\xc6\xfa\xb4\xa2\xb4\xe6\xd3\xce\xcf\xb7\xbd\xf8\xb6\xc8");
    return 0;
 }  
 move(b_lines-2, 1);
 //% prints("储存档案会覆盖存储存於 [%s] 的小鸡的档案喔！请考虑清楚...",files[num]);
 prints("\xb4\xa2\xb4\xe6\xb5\xb5\xb0\xb8\xbb\xe1\xb8\xb2\xb8\xc7\xb4\xe6\xb4\xa2\xb4\xe6\xec\xb6 [%s] \xb5\xc4\xd0\xa1\xbc\xa6\xb5\xc4\xb5\xb5\xb0\xb8\xe0\xb8\xa3\xa1\xc7\xeb\xbf\xbc\xc2\xc7\xc7\xe5\xb3\xfe...",files[num]);
 //% sprintf(buf1,"确定要储存於 [%s] 档案吗？ [y/N]:",files[num]);
 sprintf(buf1,"\xc8\xb7\xb6\xa8\xd2\xaa\xb4\xa2\xb4\xe6\xec\xb6 [%s] \xb5\xb5\xb0\xb8\xc2\xf0\xa3\xbf [y/N]:",files[num]);
#ifdef MAPLE
 getdata(b_lines-1, 1,buf1, ans, 2, 1, 0);
#else
 getdata(b_lines-1, 1,buf1, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
 if (ans[0]!='y'&&ans[0]!='Y') 
 {
    //% pressanykey("放弃储存档案");
    pressanykey("\xb7\xc5\xc6\xfa\xb4\xa2\xb4\xe6\xb5\xb5\xb0\xb8");
    return 0;
 }
 
 move(b_lines-1,0);
 clrtobot();
 //% sprintf(buf1,"储存 [%s] 档案完成了",files[num]);
 sprintf(buf1,"\xb4\xa2\xb4\xe6 [%s] \xb5\xb5\xb0\xb8\xcd\xea\xb3\xc9\xc1\xcb",files[num]);
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
 //% char *files[4]={"没有","进度一","进度二","进度叁"};
 char *files[4]={"\xc3\xbb\xd3\xd0","\xbd\xf8\xb6\xc8\xd2\xbb","\xbd\xf8\xb6\xc8\xb6\xfe","\xbd\xf8\xb6\xc8\xc8\xfe"};
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
    //% prints("读取 [1]进度一 [2]进度二 [3]进度叁 [Q]放弃 [1/2/3/Q]：");
    prints("\xb6\xc1\xc8\xa1 [1]\xbd\xf8\xb6\xc8\xd2\xbb [2]\xbd\xf8\xb6\xc8\xb6\xfe [3]\xbd\xf8\xb6\xc8\xc8\xfe [Q]\xb7\xc5\xc6\xfa [1/2/3/Q]\xa3\xba");
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
        //% sprintf(buf,"档案 [%s] 不存在",files[num]);
        sprintf(buf,"\xb5\xb5\xb0\xb8 [%s] \xb2\xbb\xb4\xe6\xd4\xda",files[num]);
        pressanykey(buf);
        ok=0;
      }
      else 
      {
         
	 move(b_lines-2, 1);
	 //% prints("读取出档案会覆盖现在正在玩的小鸡的档案喔！请考虑清楚...");
	 prints("\xb6\xc1\xc8\xa1\xb3\xf6\xb5\xb5\xb0\xb8\xbb\xe1\xb8\xb2\xb8\xc7\xcf\xd6\xd4\xda\xd5\xfd\xd4\xda\xcd\xe6\xb5\xc4\xd0\xa1\xbc\xa6\xb5\xc4\xb5\xb5\xb0\xb8\xe0\xb8\xa3\xa1\xc7\xeb\xbf\xbc\xc2\xc7\xc7\xe5\xb3\xfe...");
	 //% sprintf(buf,"确定要读取出 [%s] 档案吗？ [y/N]:",files[num]);
	 sprintf(buf,"\xc8\xb7\xb6\xa8\xd2\xaa\xb6\xc1\xc8\xa1\xb3\xf6 [%s] \xb5\xb5\xb0\xb8\xc2\xf0\xa3\xbf [y/N]:",files[num]);
#ifdef MAPLE
	 getdata(b_lines-1, 1,buf, ans, 2, 1, 0);
#else
         getdata(b_lines-1, 1,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
	 if (ans[0]!='y'&&ans[0]!='Y') {
	    //% pressanykey("让我再决定一下...");
	    pressanykey("\xc8\xc3\xce\xd2\xd4\xd9\xbe\xf6\xb6\xa8\xd2\xbb\xcf\xc2...");
	 } else ok=1;
      }
    }
 }while(pipkey!='Q' && pipkey!='q' && ok!=1);
 if(pipkey=='q' ||pipkey=='Q')
 {
    //% pressanykey("还是玩原本的游戏");
    pressanykey("\xbb\xb9\xca\xc7\xcd\xe6\xd4\xad\xb1\xbe\xb5\xc4\xd3\xce\xcf\xb7");
    return 0;
 }
 
 move(b_lines-1,0);
 clrtobot();
 //% sprintf(buf,"读取 [%s] 档案完成了",files[num]);
 sprintf(buf,"\xb6\xc1\xc8\xa1 [%s] \xb5\xb5\xb0\xb8\xcd\xea\xb3\xc9\xc1\xcb",files[num]);
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
   //% showtitle("小鸡复活手术中", BoardName);
   showtitle("\xd0\xa1\xbc\xa6\xb8\xb4\xbb\xee\xca\xd6\xca\xf5\xd6\xd0", BoardName);

   now = time(0);
   //% sprintf(genbuf, "[1;33m%s %-11s的小鸡 [%s二代] 复活了！[m\n", Cdate(&now), cuser.userid,d.name);
   sprintf(genbuf, "[1;33m%s %-11s\xb5\xc4\xd0\xa1\xbc\xa6 [%s\xb6\xfe\xb4\xfa] \xb8\xb4\xbb\xee\xc1\xcb\xa3\xa1[m\n", Cdate(&now), cuser.userid,d.name);
   pip_log_record(genbuf);
   
   /*身体上的设定*/
   d.death=0;
   d.maxhp=d.maxhp*3/4+1;
   d.hp=d.maxhp/2+1;
   d.tired=20;
   d.shit=20;
   d.sick=20;   
   d.wrist=d.wrist*3/4;
   d.weight=45+10*tm;
   
   /*钱减到五分之一*/
   d.money=d.money/5;
   
   /*战斗能力降一半*/
   d.attack=d.attack*3/4;
   d.resist=d.resist*3/4;
   d.maxmp=d.maxmp*3/4;
   d.mp=d.maxmp/2;
   
   /*变的不快乐*/
   d.happy=0;
   d.satisfy=0;
   
   /*评价减半*/
   d.social=d.social*3/4;
   d.family=d.family*3/4;
   d.hexp=d.hexp*3/4;
   d.mexp=d.mexp*3/4;

   /*武器掉光光*/   
   d.weaponhead=0;
   d.weaponrhand=0;
   d.weaponlhand=0;
   d.weaponbody=0;
   d.weaponfoot=0;
   
   /*食物剩一半*/
   d.food=d.food/2;
   d.medicine=d.medicine/2;
   d.bighp=d.bighp/2;
   d.cookie=d.cookie/2;

   d.liveagain+=1;
   
   //% pressanykey("小鸡器官重建中！");
   pressanykey("\xd0\xa1\xbc\xa6\xc6\xf7\xb9\xd9\xd6\xd8\xbd\xa8\xd6\xd0\xa3\xa1");
   //% pressanykey("小鸡体质恢复中！");
   pressanykey("\xd0\xa1\xbc\xa6\xcc\xe5\xd6\xca\xbb\xd6\xb8\xb4\xd6\xd0\xa3\xa1");
   //% pressanykey("小鸡能力调整中！");
   pressanykey("\xd0\xa1\xbc\xa6\xc4\xdc\xc1\xa6\xb5\xf7\xd5\xfb\xd6\xd0\xa3\xa1");
   //% pressanykey("恭禧您，你的小鸡又复活罗！");
   pressanykey("\xb9\xa7\xec\xfb\xc4\xfa\xa3\xac\xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6\xd3\xd6\xb8\xb4\xbb\xee\xc2\xde\xa3\xa1");
   pip_write_file();
   return 0;
}
