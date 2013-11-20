/*---------------------------------------------------------------------------*/
/* 系统选单:个人资料  小鸡放生  特别服务                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

char weaponhead[7][10]={
//% "没有装备",
"\xc3\xbb\xd3\xd0\xd7\xb0\xb1\xb8",
//% "塑胶帽子", 
"\xcb\xdc\xbd\xba\xc3\xb1\xd7\xd3", 
//% "牛皮小帽",
"\xc5\xa3\xc6\xa4\xd0\xa1\xc3\xb1",
//% "  安全帽",
"  \xb0\xb2\xc8\xab\xc3\xb1",
//% "钢铁头盔",
"\xb8\xd6\xcc\xfa\xcd\xb7\xbf\xf8",
//% "魔法发箍",
"\xc4\xa7\xb7\xa8\xb7\xa2\xb9\xbf",
//% "黄金圣盔"};
"\xbb\xc6\xbd\xf0\xca\xa5\xbf\xf8"};


char weaponrhand[10][10]={
//% "没有装备",
"\xc3\xbb\xd3\xd0\xd7\xb0\xb1\xb8",
//% "大木棒",  
"\xb4\xf3\xc4\xbe\xb0\xf4",  
//% "金属扳手",
"\xbd\xf0\xca\xf4\xb0\xe2\xca\xd6",
//% "青铜剑",  
"\xc7\xe0\xcd\xad\xbd\xa3",  
//% "晴雷剑", 
"\xc7\xe7\xc0\xd7\xbd\xa3", 
//% "蝉翼刀", 
"\xb2\xf5\xd2\xed\xb5\xb6", 
//% "忘情剑", 
"\xcd\xfc\xc7\xe9\xbd\xa3", 
//% "狮头宝刀",
"\xca\xa8\xcd\xb7\xb1\xa6\xb5\xb6",
//% "屠龙刀",  
"\xcd\xc0\xc1\xfa\xb5\xb6",  
//% "黄金圣杖"
"\xbb\xc6\xbd\xf0\xca\xa5\xd5\xc8"
};  

char weaponlhand[8][10]={
//% "没有装备",
"\xc3\xbb\xd3\xd0\xd7\xb0\xb1\xb8",
//% "大木棒", 
"\xb4\xf3\xc4\xbe\xb0\xf4", 
//% "金属扳手",
"\xbd\xf0\xca\xf4\xb0\xe2\xca\xd6",
//% "木盾",
"\xc4\xbe\xb6\xdc",
//% "不锈钢盾",
"\xb2\xbb\xd0\xe2\xb8\xd6\xb6\xdc",
//% "白金之盾",
"\xb0\xd7\xbd\xf0\xd6\xae\xb6\xdc",
//% "魔法盾",
"\xc4\xa7\xb7\xa8\xb6\xdc",
//% "黄金圣盾"
"\xbb\xc6\xbd\xf0\xca\xa5\xb6\xdc"
};


char weaponbody[7][10]={
//% "没有装备",
"\xc3\xbb\xd3\xd0\xd7\xb0\xb1\xb8",
//% "塑胶胄甲",
"\xcb\xdc\xbd\xba\xeb\xd0\xbc\xd7",
//% "特级皮甲",
"\xcc\xd8\xbc\xb6\xc6\xa4\xbc\xd7",
//% "钢铁盔甲",
"\xb8\xd6\xcc\xfa\xbf\xf8\xbc\xd7",
//% "魔法披风",
"\xc4\xa7\xb7\xa8\xc5\xfb\xb7\xe7",
//% "白金盔甲",
"\xb0\xd7\xbd\xf0\xbf\xf8\xbc\xd7",
//% "黄金圣衣"};
"\xbb\xc6\xbd\xf0\xca\xa5\xd2\xc2"};

char weaponfoot[8][12]={
//% "没有装备",
"\xc3\xbb\xd3\xd0\xd7\xb0\xb1\xb8",
//% "塑胶拖鞋",
"\xcb\xdc\xbd\xba\xcd\xcf\xd0\xac",
//% "东洋木屐",
"\xb6\xab\xd1\xf3\xc4\xbe\xe5\xec",
//% "特级雨鞋",
"\xcc\xd8\xbc\xb6\xd3\xea\xd0\xac",
//% "NIKE运动鞋",
"NIKE\xd4\xcb\xb6\xaf\xd0\xac",
//% "鳄鱼皮靴",
"\xf6\xf9\xd3\xe3\xc6\xa4\xd1\xa5",
//% "飞天魔靴",
"\xb7\xc9\xcc\xec\xc4\xa7\xd1\xa5",
//% "黄金圣靴"
"\xbb\xc6\xbd\xf0\xca\xa5\xd1\xa5"
};

int 
pip_system_freepip()
{
      char buf[256];
      move(b_lines-1, 0);
      clrtoeol();
#ifdef MAPLE
      //% getdata(b_lines-1,1, "真的要放生吗？(y/N)", buf, 2, 1, 0);
      getdata(b_lines-1,1, "\xd5\xe6\xb5\xc4\xd2\xaa\xb7\xc5\xc9\xfa\xc2\xf0\xa3\xbf(y/N)", buf, 2, 1, 0);
#else
      //% getdata(b_lines-1,1, "真的要放生吗？(y/N)", buf, 2, DOECHO, YEA);
      getdata(b_lines-1,1, "\xd5\xe6\xb5\xc4\xd2\xaa\xb7\xc5\xc9\xfa\xc2\xf0\xa3\xbf(y/N)", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
      if (buf[0]!='y'&&buf[0]!='Y') return 0;
      //% sprintf(buf,"%s 被狠心的 %s 丢掉了~",d.name,cuser.userid);
      sprintf(buf,"%s \xb1\xbb\xba\xdd\xd0\xc4\xb5\xc4 %s \xb6\xaa\xb5\xf4\xc1\xcb~",d.name,cuser.userid);
      pressanykey(buf);
      d.death=2;
      //% pipdie("[1;31m被狠心丢弃:~~[0m",2);
      pipdie("[1;31m\xb1\xbb\xba\xdd\xd0\xc4\xb6\xaa\xc6\xfa:~~[0m",2);
      return 0;
}


int
pip_system_service()
{
     int pipkey;
     int oldchoice;
     char buf[200];
     char oldname[21];
     time_t now;     

     move(b_lines, 0);
     clrtoeol();
     move(b_lines,0);
     //% prints("[1;44m  服务项目  [46m[1]命名大师 [2]变性手术 [3]结局设局                                [0m");
     prints("[1;44m  \xb7\xfe\xce\xf1\xcf\xee\xc4\xbf  [46m[1]\xc3\xfc\xc3\xfb\xb4\xf3\xca\xa6 [2]\xb1\xe4\xd0\xd4\xca\xd6\xca\xf5 [3]\xbd\xe1\xbe\xd6\xc9\xe8\xbe\xd6                                [0m");
     pipkey=egetch();
     
     switch(pipkey)
     {
     case '1':
       move(b_lines-1,0);
       clrtobot();
#ifdef MAPLE
       //% getdata(b_lines-1, 1, "帮小鸡重新取个好名字：", buf, 11, DOECHO,NULL);
       getdata(b_lines-1, 1, "\xb0\xef\xd0\xa1\xbc\xa6\xd6\xd8\xd0\xc2\xc8\xa1\xb8\xf6\xba\xc3\xc3\xfb\xd7\xd6\xa3\xba", buf, 11, DOECHO,NULL);
#else
       //% getdata(b_lines-1, 1, "帮小鸡重新取个好名字：", buf, 11, DOECHO,YEA);
       getdata(b_lines-1, 1, "\xb0\xef\xd0\xa1\xbc\xa6\xd6\xd8\xd0\xc2\xc8\xa1\xb8\xf6\xba\xc3\xc3\xfb\xd7\xd6\xa3\xba", buf, 11, DOECHO,YEA);
#endif  // END MAPLE
       if(!buf[0])
       {
         //% pressanykey("等一下想好再来好了  :)");
         pressanykey("\xb5\xc8\xd2\xbb\xcf\xc2\xcf\xeb\xba\xc3\xd4\xd9\xc0\xb4\xba\xc3\xc1\xcb  :)");
         break;
       }
       else
       {
        strcpy(oldname,d.name);
        strcpy(d.name,buf);
        /*改名记录*/
        now=time(0);
        //% sprintf(buf, "[1;37m%s %-11s把小鸡 [%s] 改名成 [%s] [0m\n", Cdate(&now), cuser.userid,oldname,d.name);
        sprintf(buf, "[1;37m%s %-11s\xb0\xd1\xd0\xa1\xbc\xa6 [%s] \xb8\xc4\xc3\xfb\xb3\xc9 [%s] [0m\n", Cdate(&now), cuser.userid,oldname,d.name);
        pip_log_record(buf);
        //% pressanykey("嗯嗯  换一个新的名字喔...");
        pressanykey("\xe0\xc5\xe0\xc5  \xbb\xbb\xd2\xbb\xb8\xf6\xd0\xc2\xb5\xc4\xc3\xfb\xd7\xd6\xe0\xb8...");
       }
       break;
       
     case '2':  /*变性*/
       move(b_lines-1,0);
       clrtobot();
       /*1:公 2:母 */
       if(d.sex==1)
       { 
         oldchoice=2; /*公-->母*/
         move(b_lines-1, 0);
         //% prints("[1;33m将小鸡由[32m♂[33m变性成[35m♀[33m的吗？ [37m[y/N][0m");
         prints("[1;33m\xbd\xab\xd0\xa1\xbc\xa6\xd3\xc9[32m\xa1\xe1[33m\xb1\xe4\xd0\xd4\xb3\xc9[35m\xa1\xe2[33m\xb5\xc4\xc2\xf0\xa3\xbf [37m[y/N][0m");
       }
       else
       { 
         oldchoice=1; /*母-->公*/
         move(b_lines-1, 0); 
         //% prints("[1;33m将小鸡由[35m♀[33m变性成[35m♂[33m的吗？ [37m[y/N][0m");
         prints("[1;33m\xbd\xab\xd0\xa1\xbc\xa6\xd3\xc9[35m\xa1\xe2[33m\xb1\xe4\xd0\xd4\xb3\xc9[35m\xa1\xe1[33m\xb5\xc4\xc2\xf0\xa3\xbf [37m[y/N][0m");
       }
       move(b_lines,0);
       //% prints("[1;44m  服务项目  [46m[1]命名大师 [2]变性手术 [3]结局设局                                [0m");
       prints("[1;44m  \xb7\xfe\xce\xf1\xcf\xee\xc4\xbf  [46m[1]\xc3\xfc\xc3\xfb\xb4\xf3\xca\xa6 [2]\xb1\xe4\xd0\xd4\xca\xd6\xca\xf5 [3]\xbd\xe1\xbe\xd6\xc9\xe8\xbe\xd6                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         /*改名记录*/
         now=time(0);
         if(d.sex==1)
           //% sprintf(buf,"[1;37m%s %-11s把小鸡 [%s] 由♂变性成♀了[0m\n",Cdate(&now), cuser.userid,d.name);
           sprintf(buf,"[1;37m%s %-11s\xb0\xd1\xd0\xa1\xbc\xa6 [%s] \xd3\xc9\xa1\xe1\xb1\xe4\xd0\xd4\xb3\xc9\xa1\xe2\xc1\xcb[0m\n",Cdate(&now), cuser.userid,d.name);
         else
           //% sprintf(buf,"[1;37m%s %-11s把小鸡 [%s] 由♀变性成♂了[0m\n",Cdate(&now), cuser.userid,d.name);           
           sprintf(buf,"[1;37m%s %-11s\xb0\xd1\xd0\xa1\xbc\xa6 [%s] \xd3\xc9\xa1\xe2\xb1\xe4\xd0\xd4\xb3\xc9\xa1\xe1\xc1\xcb[0m\n",Cdate(&now), cuser.userid,d.name);           
         pip_log_record(buf);
         //% pressanykey("变性手术完毕...");       
         pressanykey("\xb1\xe4\xd0\xd4\xca\xd6\xca\xf5\xcd\xea\xb1\xcf...");       
         d.sex=oldchoice;
       }  
       break;
       
     case '3':
       move(b_lines-1,0);
       clrtobot();
       /*1:不要且未婚 4:要且未婚 */
       oldchoice=d.wantend;
       if(d.wantend==1 || d.wantend==2 || d.wantend==3)
       { 
         oldchoice+=3; /*没有-->有*/
         move(b_lines-1, 0); 
         //% prints("[1;33m将小鸡游戏改成[32m[有20岁结局][33m? [37m[y/N][0m");
         prints("[1;33m\xbd\xab\xd0\xa1\xbc\xa6\xd3\xce\xcf\xb7\xb8\xc4\xb3\xc9[32m[\xd3\xd020\xcb\xea\xbd\xe1\xbe\xd6][33m? [37m[y/N][0m");
	 //% sprintf(buf,"小鸡游戏设定成[有20岁结局]..");         
	 sprintf(buf,"\xd0\xa1\xbc\xa6\xd3\xce\xcf\xb7\xc9\xe8\xb6\xa8\xb3\xc9[\xd3\xd020\xcb\xea\xbd\xe1\xbe\xd6]..");         
       }
       else
       { 
         oldchoice-=3; /*有-->没有*/
         move(b_lines-1, 0); 
         //% prints("[1;33m将小鸡游戏改成[32m[没有20岁结局][33m? [37m[y/N][0m");
         prints("[1;33m\xbd\xab\xd0\xa1\xbc\xa6\xd3\xce\xcf\xb7\xb8\xc4\xb3\xc9[32m[\xc3\xbb\xd3\xd020\xcb\xea\xbd\xe1\xbe\xd6][33m? [37m[y/N][0m");
         //% sprintf(buf,"小鸡游戏设定成[没有20岁结局]..");
         sprintf(buf,"\xd0\xa1\xbc\xa6\xd3\xce\xcf\xb7\xc9\xe8\xb6\xa8\xb3\xc9[\xc3\xbb\xd3\xd020\xcb\xea\xbd\xe1\xbe\xd6]..");
       }
       move(b_lines,0);
       //% prints("[1;44m  服务项目  [46m[1]命名大师 [2]变性手术 [3]结局设局                                [0m");
       prints("[1;44m  \xb7\xfe\xce\xf1\xcf\xee\xc4\xbf  [46m[1]\xc3\xfc\xc3\xfb\xb4\xf3\xca\xa6 [2]\xb1\xe4\xd0\xd4\xca\xd6\xca\xf5 [3]\xbd\xe1\xbe\xd6\xc9\xe8\xbe\xd6                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         d.wantend=oldchoice;
         pressanykey(buf);
       }  
       break;     
     } 
     return 0;
}

int
pip_data_list()  /*看小鸡个人详细资料*/
{
  char buf[256];
  char inbuf1[20];
  char inbuf2[20];
  int tm;
  int pipkey;
  int page=1;
  
  tm=(time(0)-start_time+d.bbtime)/60/30;

  screen_clear();  
  move(1,0);
  //% prints("       [1;33m┏━━━    ━━━  ┏━━━┓  ━━━  [m\n");
  prints("       [1;33m\xa9\xb3\xa9\xa5\xa9\xa5\xa9\xa5    \xa9\xa5\xa9\xa5\xa9\xa5  \xa9\xb3\xa9\xa5\xa9\xa5\xa9\xa5\xa9\xb7  \xa9\xa5\xa9\xa5\xa9\xa5  [m\n");
  //% prints("       [0;37m┃      ┃┃ ━   ┃┗┓┏━┛┃ ━   ┃[m\n");
  prints("       [0;37m\xa9\xa7      \xa9\xa7\xa9\xa7 \xa9\xa5   \xa9\xa7\xa9\xbb\xa9\xb7\xa9\xb3\xa9\xa5\xa9\xbf\xa9\xa7 \xa9\xa5   \xa9\xa7[m\n");
  //% prints("       [1;37m┃      ┃┃┏┓  ┃  ┃┃    ┃┏┓  ┃[m\n");
  prints("       [1;37m\xa9\xa7      \xa9\xa7\xa9\xa7\xa9\xb3\xa9\xb7  \xa9\xa7  \xa9\xa7\xa9\xa7    \xa9\xa7\xa9\xb3\xa9\xb7  \xa9\xa7[m\n");
  //% prints("       [1;34m┗━━━  ┗┛┗━┛  ┗┛    ┗┛┗━┛[32m......................[m");
  prints("       [1;34m\xa9\xbb\xa9\xa5\xa9\xa5\xa9\xa5  \xa9\xbb\xa9\xbf\xa9\xbb\xa9\xa5\xa9\xbf  \xa9\xbb\xa9\xbf    \xa9\xbb\xa9\xbf\xa9\xbb\xa9\xa5\xa9\xbf[32m......................[m");
  do
  { clrchyiuan(5,23);
    switch(page)
    {
     case 1:
       move(5,0);
       sprintf(buf,
       //% "[1;31m ┌┤[41;37m 基本资料 [0;1;31m├—————————————————————————————┐[m\n");  
       "[1;31m \xa9\xb0\xa9\xc8[41;37m \xbb\xf9\xb1\xbe\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m\n");  
       prints(buf);
  
       sprintf(buf,
       //% "[1;31m │[33m＃姓    名 :[37m %-10s [33m＃生    日 :[37m %02d/%02d/%02d   [33m＃年    纪 :[37m %-2d         [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xd0\xd5    \xc3\xfb :[37m %-10s [33m\xa3\xa3\xc9\xfa    \xc8\xd5 :[37m %02d/%02d/%02d   [33m\xa3\xa3\xc4\xea    \xbc\xcd :[37m %-2d         [31m\xa9\xa6[m\n",
       d.name,d.year%100,d.month,d.day,tm);
       prints(buf);  
  
       sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
       sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);  
       sprintf(buf,
       //% "[1;31m │[33m＃体    重 :[37m %-5d(米克)[33m＃体    力 :[37m %-11s[33m＃法    力 :[37m %-11s[31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xcc\xe5    \xd6\xd8 :[37m %-5d(\xc3\xd7\xbf\xcb)[33m\xa3\xa3\xcc\xe5    \xc1\xa6 :[37m %-11s[33m\xa3\xa3\xb7\xa8    \xc1\xa6 :[37m %-11s[31m\xa9\xa6[m\n",
       d.weight,inbuf1,inbuf2);
       prints(buf);  
  
       sprintf(buf,
       //% "[1;31m │[33m＃疲    劳 :[37m %-3d        [33m＃病    气 :[37m %-3d        [33m＃脏    脏 :[37m %-3d        [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xc6\xa3    \xc0\xcd :[37m %-3d        [33m\xa3\xa3\xb2\xa1    \xc6\xf8 :[37m %-3d        [33m\xa3\xa3\xd4\xe0    \xd4\xe0 :[37m %-3d        [31m\xa9\xa6[m\n",
       d.tired,d.sick,d.shit);
       prints(buf);  
   
       sprintf(buf,  
       //% "[1;31m │[33m＃腕    力 :[37m %-7d    [33m＃亲子关系 :[37m %-7d    [33m＃金    钱 :[37m %-11d[31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xcd\xf3    \xc1\xa6 :[37m %-7d    [33m\xa3\xa3\xc7\xd7\xd7\xd3\xb9\xd8\xcf\xb5 :[37m %-7d    [33m\xa3\xa3\xbd\xf0    \xc7\xae :[37m %-11d[31m\xa9\xa6[m\n",
       d.wrist,d.relation,d.money);
       prints(buf);  
  
       sprintf(buf,  
       //% "[1;31m ├┤[41;37m 能力资料 [0;1;31m├—————————————————————————————┤[m\n");
       "[1;31m \xa9\xc0\xa9\xc8[41;37m \xc4\xdc\xc1\xa6\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8[m\n");
       prints(buf);  
   
       sprintf(buf,   
       //% "[1;31m │[33m＃气    质 :[37m %-10d [33m＃智    力 :[37m %-10d [33m＃爱    心 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xc6\xf8    \xd6\xca :[37m %-10d [33m\xa3\xa3\xd6\xc7    \xc1\xa6 :[37m %-10d [33m\xa3\xa3\xb0\xae    \xd0\xc4 :[37m %-10d [31m\xa9\xa6[m\n",
       d.character,d.wisdom,d.love);
       prints(buf);  
   
       sprintf(buf, 
       //% "[1;31m │[33m＃艺    术 :[37m %-10d [33m＃道    德 :[37m %-10d [33m＃家    事 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xd2\xd5    \xca\xf5 :[37m %-10d [33m\xa3\xa3\xb5\xc0    \xb5\xc2 :[37m %-10d [33m\xa3\xa3\xbc\xd2    \xca\xc2 :[37m %-10d [31m\xa9\xa6[m\n",
       d.art,d.etchics,d.homework);
       prints(buf);  
 
       sprintf(buf, 
       //% "[1;31m │[33m＃礼    仪 :[37m %-10d [33m＃应    对 :[37m %-10d [33m＃烹    饪 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xc0\xf1    \xd2\xc7 :[37m %-10d [33m\xa3\xa3\xd3\xa6    \xb6\xd4 :[37m %-10d [33m\xa3\xa3\xc5\xeb    \xe2\xbf :[37m %-10d [31m\xa9\xa6[m\n",
       d.manners,d.speech,d.cookskill);
       prints(buf);    
 
       sprintf(buf,  
       //% "[1;31m ├┤[41;37m 状态资料 [0;1;31m├—————————————————————————————┤[m\n");
       "[1;31m \xa9\xc0\xa9\xc8[41;37m \xd7\xb4\xcc\xac\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8[m\n");
       prints(buf);  
 
       sprintf(buf, 
       //% "[1;31m │[33m＃快    乐 :[37m %-10d [33m＃满    意 :[37m %-10d [33m＃人    际 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xbf\xec    \xc0\xd6 :[37m %-10d [33m\xa3\xa3\xc2\xfa    \xd2\xe2 :[37m %-10d [33m\xa3\xa3\xc8\xcb    \xbc\xca :[37m %-10d [31m\xa9\xa6[m\n",
       d.happy,d.satisfy,d.toman);
       prints(buf);
  
       sprintf(buf, 
       //% "[1;31m │[33m＃魅    力 :[37m %-10d [33m＃勇    敢 :[37m %-10d [33m＃信    仰 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xf7\xc8    \xc1\xa6 :[37m %-10d [33m\xa3\xa3\xd3\xc2    \xb8\xd2 :[37m %-10d [33m\xa3\xa3\xd0\xc5    \xd1\xf6 :[37m %-10d [31m\xa9\xa6[m\n",
       d.charm,d.brave,d.belief);
       prints(buf);  

       sprintf(buf, 
       //% "[1;31m │[33m＃罪    孽 :[37m %-10d [33m＃感    受 :[37m %-10d [33m            [37m            [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xd7\xef    \xc4\xf5 :[37m %-10d [33m\xa3\xa3\xb8\xd0    \xca\xdc :[37m %-10d [33m            [37m            [31m\xa9\xa6[m\n",
       d.offense,d.affect);
       prints(buf);  

       sprintf(buf, 
       //% "[1;31m ├┤[41;37m 评价资料 [0;1;31m├—————————————————————————————┤[m\n");
       "[1;31m \xa9\xc0\xa9\xc8[41;37m \xc6\xc0\xbc\xdb\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8[m\n");
       prints(buf);  

       sprintf(buf, 
       //% "[1;31m │[33m＃社交评价 :[37m %-10d [33m＃战斗评价 :[37m %-10d [33m＃魔法评价 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xc9\xe7\xbd\xbb\xc6\xc0\xbc\xdb :[37m %-10d [33m\xa3\xa3\xd5\xbd\xb6\xb7\xc6\xc0\xbc\xdb :[37m %-10d [33m\xa3\xa3\xc4\xa7\xb7\xa8\xc6\xc0\xbc\xdb :[37m %-10d [31m\xa9\xa6[m\n",
       d.social,d.hexp,d.mexp);
       prints(buf);  

       sprintf(buf, 
       //% "[1;31m │[33m＃家事评价 :[37m %-10d [33m            [37m            [33m            [37m            [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xbc\xd2\xca\xc2\xc6\xc0\xbc\xdb :[37m %-10d [33m            [37m            [33m            [37m            [31m\xa9\xa6[m\n",
       d.family);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m └————————————————————————————————————┘[m\n");
       "[1;31m \xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m\n");
       prints(buf);  
       
       move(b_lines-1,0);       
       sprintf(buf, 
       //% "                                                              [1;36m第一页[37m/[36m共二页[m\n");
       "                                                              [1;36m\xb5\xda\xd2\xbb\xd2\xb3[37m/[36m\xb9\xb2\xb6\xfe\xd2\xb3[m\n");
       prints(buf);  
       break;

     case 2:
       move(5,0);
       sprintf(buf, 
       //% "[1;31m ┌┤[41;37m 物品资料 [0;1;31m├—————————————————————————————┐[m\n");
       "[1;31m \xa9\xb0\xa9\xc8[41;37m \xce\xef\xc6\xb7\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m\n");
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m │[33m＃食    物 :[37m %-10d [33m＃零    食 :[37m %-10d [33m＃大 补 丸 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xca\xb3    \xce\xef :[37m %-10d [33m\xa3\xa3\xc1\xe3    \xca\xb3 :[37m %-10d [33m\xa3\xa3\xb4\xf3 \xb2\xb9 \xcd\xe8 :[37m %-10d [31m\xa9\xa6[m\n",
       d.food,d.cookie,d.bighp);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m │[33m＃药    草 :[37m %-10d [33m＃书    本 :[37m %-10d [33m＃玩    具 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xd2\xa9    \xb2\xdd :[37m %-10d [33m\xa3\xa3\xca\xe9    \xb1\xbe :[37m %-10d [33m\xa3\xa3\xcd\xe6    \xbe\xdf :[37m %-10d [31m\xa9\xa6[m\n",
       d.medicine,d.book,d.playtool);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m ├┤[41;37m 游戏资料 [0;1;31m├—————————————————————————————┤[m\n");
       "[1;31m \xa9\xc0\xa9\xc8[41;37m \xd3\xce\xcf\xb7\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8[m\n");
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m │[33m＃猜 拳 赢 :[37m %-10d [33m＃猜 拳 输 :[37m %-10d                         [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xb2\xc2 \xc8\xad \xd3\xae :[37m %-10d [33m\xa3\xa3\xb2\xc2 \xc8\xad \xca\xe4 :[37m %-10d                         [31m\xa9\xa6[m\n",
       d.winn,d.losee);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m ├┤[41;37m 武力资料 [0;1;31m├—————————————————————————————┤[m\n");
       "[1;31m \xa9\xc0\xa9\xc8[41;37m \xce\xe4\xc1\xa6\xd7\xca\xc1\xcf [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8[m\n");
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m │[33m＃攻 击 力 :[37m %-10d [33m＃防 御 力 :[37m %-10d [33m＃速 度 值 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xb9\xa5 \xbb\xf7 \xc1\xa6 :[37m %-10d [33m\xa3\xa3\xb7\xc0 \xd3\xf9 \xc1\xa6 :[37m %-10d [33m\xa3\xa3\xcb\xd9 \xb6\xc8 \xd6\xb5 :[37m %-10d [31m\xa9\xa6[m\n",
       d.attack,d.resist,d.speed);
       prints(buf);  
       sprintf(buf, 
       //% "[1;31m │[33m＃抗魔能力 :[37m %-10d [33m＃战斗技术 :[37m %-10d [33m＃魔法技术 :[37m %-10d [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xbf\xb9\xc4\xa7\xc4\xdc\xc1\xa6 :[37m %-10d [33m\xa3\xa3\xd5\xbd\xb6\xb7\xbc\xbc\xca\xf5 :[37m %-10d [33m\xa3\xa3\xc4\xa7\xb7\xa8\xbc\xbc\xca\xf5 :[37m %-10d [31m\xa9\xa6[m\n",
       d.mresist,d.hskill,d.mskill);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m │[33m＃头部装备 :[37m %-10s [33m＃右手装备 :[37m %-10s [33m＃左手装备 :[37m %-10s [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xcd\xb7\xb2\xbf\xd7\xb0\xb1\xb8 :[37m %-10s [33m\xa3\xa3\xd3\xd2\xca\xd6\xd7\xb0\xb1\xb8 :[37m %-10s [33m\xa3\xa3\xd7\xf3\xca\xd6\xd7\xb0\xb1\xb8 :[37m %-10s [31m\xa9\xa6[m\n",
       weaponhead[d.weaponhead],weaponrhand[d.weaponrhand],weaponlhand[d.weaponlhand]);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m │[33m＃身体装备 :[37m %-10s [33m＃脚部装备 :[37m %-10s [33m            [37m            [31m│[m\n",
       "[1;31m \xa9\xa6[33m\xa3\xa3\xc9\xed\xcc\xe5\xd7\xb0\xb1\xb8 :[37m %-10s [33m\xa3\xa3\xbd\xc5\xb2\xbf\xd7\xb0\xb1\xb8 :[37m %-10s [33m            [37m            [31m\xa9\xa6[m\n",
       weaponbody[d.weaponbody],weaponfoot[d.weaponfoot]);
       prints(buf);  
  
       sprintf(buf, 
       //% "[1;31m └————————————————————————————————————┘[m\n");
       "[1;31m \xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m\n");
       prints(buf); 

       move(b_lines-1,0);
       sprintf(buf, 
       //% "                                                              [1;36m第二页[37m/[36m共二页[m\n");
       "                                                              [1;36m\xb5\xda\xb6\xfe\xd2\xb3[37m/[36m\xb9\xb2\xb6\xfe\xd2\xb3[m\n");
       prints(buf);          
       break;
    }
    move(b_lines,0);
    //% sprintf(buf,"[1;44;37m  资料选单  [46m  [↑/PAGE UP]往上一页 [↓/PAGE DOWN]往下一页 [Q]离开:            [m");
    sprintf(buf,"[1;44;37m  \xd7\xca\xc1\xcf\xd1\xa1\xb5\xa5  [46m  [\xa1\xfc/PAGE UP]\xcd\xf9\xc9\xcf\xd2\xbb\xd2\xb3 [\xa1\xfd/PAGE DOWN]\xcd\xf9\xcf\xc2\xd2\xbb\xd2\xb3 [Q]\xc0\xeb\xbf\xaa:            [m");
    prints(buf);    
    pipkey=egetch();
    switch(pipkey)
    {
      case KEY_UP:
      case KEY_PGUP:
      case KEY_DOWN:
      case KEY_PGDN:
        if(page==1)
           page=2;
        else if(page==2)
           page=1;
        break;
#ifdef MAPLE
      case Ctrl('R'):
        if (currutmp->msgs[0].last_pid)
        {
          show_last_call_in();
          //% my_write(currutmp->msgs[0].last_pid, "水球丢回去：");
          my_write(currutmp->msgs[0].last_pid, "\xcb\xae\xc7\xf2\xb6\xaa\xbb\xd8\xc8\xa5\xa3\xba");
        }
        break;
#endif  // END MAPLE
    }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
  return 0;
}
