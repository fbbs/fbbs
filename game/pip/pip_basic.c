/*---------------------------------------------------------------------------*/
/* 基本选单:饮食 清洁 亲亲 休息                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)
int pip_basic_takeshower() /*洗澡*/
{
     int lucky;
     d.shit-=20;
     if(d.shit<0) d.shit=0;
     d.hp-=rand()%2+3;
     move(4,0);
     lucky=rand()%3;
     if(lucky==0)
       {
         show_usual_pic(1);
         //% pressanykey("我是乾净的小鸡  cccc....");
         pressanykey("\xce\xd2\xca\xc7\xc7\xac\xbe\xbb\xb5\xc4\xd0\xa1\xbc\xa6  cccc....");
       }
     else if(lucky==1)
       {
         show_usual_pic(7);
         //% pressanykey("马桶 嗯～～");       
         pressanykey("\xc2\xed\xcd\xb0 \xe0\xc5\xa1\xab\xa1\xab");       
       }
     else
       {
         show_usual_pic(2);
         //% pressanykey("我爱洗澡 lalala....");
         pressanykey("\xce\xd2\xb0\xae\xcf\xb4\xd4\xe8 lalala....");
       }
     return 0;
}

int pip_basic_takerest() /*休息*/
{
//     count_tired(5,20,"Y",100,0);
     d.tired-=rand() % 15;
     d.tired = d.tired > 0 ? d.tired : 0;
     
     if(d.hp>d.maxhp)
       d.hp=d.maxhp;
     d.shit+=1;
     move(4,0);
     show_usual_pic(5);
     //% pressanykey("再按一下我就起床罗....");
     pressanykey("\xd4\xd9\xb0\xb4\xd2\xbb\xcf\xc2\xce\xd2\xbe\xcd\xc6\xf0\xb4\xb2\xc2\xde....");
     show_usual_pic(6);
     //% pressanykey("喂喂喂..该起床罗......");
     pressanykey("\xce\xb9\xce\xb9\xce\xb9..\xb8\xc3\xc6\xf0\xb4\xb2\xc2\xde......");
     return 0;
}

int pip_basic_kiss()/*亲亲*/
{
     if(rand()%2>0)
       {
          d.happy+=rand()%3+4;
          d.satisfy+=rand()%2+1;
       }
     else
       {
          d.happy+=rand()%2+1;
          d.satisfy+=rand()%3+4;
       }
     count_tired(1,2,"N",100,1);
     d.shit+=rand()%5+4;
     d.relation+=rand()%2;
     move(4,0);
     show_usual_pic(3);
     if(d.shit<60)
      {
       //% pressanykey("来嘛! 啵一个.....");
       pressanykey("\xc0\xb4\xc2\xef! \xe0\xa3\xd2\xbb\xb8\xf6.....");
      }
     else
      {
       //% pressanykey("亲太多也是会脏死的喔....");
       pressanykey("\xc7\xd7\xcc\xab\xb6\xe0\xd2\xb2\xca\xc7\xbb\xe1\xd4\xe0\xcb\xc0\xb5\xc4\xe0\xb8....");
      }
     return 0;
}
int pip_basic_feed()     /* 饮食*/
{
  time_t now;
  char inbuf[80];
  char genbuf[200];
  char buf[256];
  long smoney;
  int pipkey;

  d.nodone=1;
  
  do
  {
   if(d.death==1 || d.death==2 || d.death==3)
     return 0;   
   if(pip_mainmenu(1)) return 0;
   move(b_lines-2, 0);
   clrtoeol();
   move(b_lines-2, 1);  
   //% sprintf(buf,"%s该做什麽事呢?",d.name);    
   sprintf(buf,"%s\xb8\xc3\xd7\xf6\xca\xb2\xf7\xe1\xca\xc2\xc4\xd8?",d.name);    
   prints(buf);   
   now=time(0);   
   move(b_lines, 0);
   clrtoeol();   
   move(b_lines, 0);
   //% prints("[1;44;37m  饮食选单  [46m[1]吃饭 [2]零食 [3]补丸 [4]灵芝 [5]人参 [6]雪莲 [Q]跳出：         [m");   
   prints("[1;44;37m  \xd2\xfb\xca\xb3\xd1\xa1\xb5\xa5  [46m[1]\xb3\xd4\xb7\xb9 [2]\xc1\xe3\xca\xb3 [3]\xb2\xb9\xcd\xe8 [4]\xc1\xe9\xd6\xa5 [5]\xc8\xcb\xb2\xce [6]\xd1\xa9\xc1\xab [Q]\xcc\xf8\xb3\xf6\xa3\xba         [m");   
   pip_time_change(now);
   pipkey=egetch();
   pip_time_change(now);

   switch(pipkey)
   {
   case '1':
    if(d.food<=0)
     {
      move(b_lines,0);
      //% pressanykey("没有食物罗..快去买吧！");
      pressanykey("\xc3\xbb\xd3\xd0\xca\xb3\xce\xef\xc2\xde..\xbf\xec\xc8\xa5\xc2\xf2\xb0\xc9\xa3\xa1");
      break;
     }
    move(4,0);
    if((d.bbtime/60/30)<3)
       show_feed_pic(0);
    else
       show_feed_pic(1);
    d.food--;
    d.hp+=50;
    if(d.hp >=d.maxhp)
     {
       d.hp=d.maxhp;
       d.weight+=rand()%2;
     }
    d.nodone=0;
    //% pressanykey("每吃一次食物会恢复体力50喔!");
    pressanykey("\xc3\xbf\xb3\xd4\xd2\xbb\xb4\xce\xca\xb3\xce\xef\xbb\xe1\xbb\xd6\xb8\xb4\xcc\xe5\xc1\xa650\xe0\xb8!");
    break;

   case '2':
    if(d.cookie<=0)
    {
      move(b_lines,0);
      //% pressanykey("零食吃光罗..快去买吧！");
      pressanykey("\xc1\xe3\xca\xb3\xb3\xd4\xb9\xe2\xc2\xde..\xbf\xec\xc8\xa5\xc2\xf2\xb0\xc9\xa3\xa1");
      break;
    }
    move(4,0);    
    d.cookie--;
    d.hp+=100;
    if(d.hp >=d.maxhp)
     {
       d.hp=d.maxhp;
       d.weight+=(rand()%2+2);
     }
    else
     {
       d.weight+=(rand()%2+1);
     }
    if(rand()%2>0)
       show_feed_pic(2);
    else
       show_feed_pic(3);
    d.happy+=(rand()%3+4);
    d.satisfy+=rand()%3+2;
    d.nodone=0;
    //% pressanykey("吃零食容易胖喔...");
    pressanykey("\xb3\xd4\xc1\xe3\xca\xb3\xc8\xdd\xd2\xd7\xc5\xd6\xe0\xb8...");
    break;

   case '3':
    if(d.bighp<=0)
    {
      move(b_lines,0);
      //% pressanykey("没有大补丸了耶! 快买吧..");
      pressanykey("\xc3\xbb\xd3\xd0\xb4\xf3\xb2\xb9\xcd\xe8\xc1\xcb\xd2\xae! \xbf\xec\xc2\xf2\xb0\xc9..");
      break;
    }
    d.bighp--;
    d.hp+=600;
    d.tired-=20;
    d.weight+=rand()%2;
    move(4,0);
    show_feed_pic(4);
    d.nodone=0;
    //% pressanykey("补丸..超极棒的唷...");
    pressanykey("\xb2\xb9\xcd\xe8..\xb3\xac\xbc\xab\xb0\xf4\xb5\xc4\xe0\xa1...");
    break;

   case '4':
    if(d.medicine<=0)
     {
      move(b_lines,0);
      //% pressanykey("没有灵芝罗..快去买吧！");
      pressanykey("\xc3\xbb\xd3\xd0\xc1\xe9\xd6\xa5\xc2\xde..\xbf\xec\xc8\xa5\xc2\xf2\xb0\xc9\xa3\xa1");
      break;
     }
    move(4,0);
    show_feed_pic(1);
    d.medicine--;
    d.mp+=50;
    if(d.mp >=d.maxmp)
     {
       d.mp=d.maxmp;
     }
    d.nodone=0;
    //% pressanykey("每吃一次灵芝会恢复法力50喔!");
    pressanykey("\xc3\xbf\xb3\xd4\xd2\xbb\xb4\xce\xc1\xe9\xd6\xa5\xbb\xe1\xbb\xd6\xb8\xb4\xb7\xa8\xc1\xa650\xe0\xb8!");
    break;

   case '5':
    if(d.ginseng<=0)
    {
      move(b_lines,0);
      //% pressanykey("没有千年人参耶! 快买吧..");
      pressanykey("\xc3\xbb\xd3\xd0\xc7\xa7\xc4\xea\xc8\xcb\xb2\xce\xd2\xae! \xbf\xec\xc2\xf2\xb0\xc9..");
      break;
    }
    d.ginseng--;
    d.mp+=500;
    d.tired-=20;
    move(4,0);
    show_feed_pic(1);
    d.nodone=0;
    //% pressanykey("千年人  ..超极棒的唷...");
    pressanykey("\xc7\xa7\xc4\xea\xc8\xcb  ..\xb3\xac\xbc\xab\xb0\xf4\xb5\xc4\xe0\xa1...");
    break;

   case '6':
    if(d.snowgrass<=0)
    {
      move(b_lines,0);
      //% pressanykey("没有天山雪莲耶! 快买吧..");
      pressanykey("\xc3\xbb\xd3\xd0\xcc\xec\xc9\xbd\xd1\xa9\xc1\xab\xd2\xae! \xbf\xec\xc2\xf2\xb0\xc9..");
      break;
    }
    d.snowgrass--;
    d.mp=d.maxmp;
    d.hp=d.maxhp;
    d.tired-=0;
    d.sick=0;
    move(4,0);
    show_feed_pic(1);
    d.nodone=0;
    //% pressanykey("天山雪莲..超极棒的唷...");
    pressanykey("\xcc\xec\xc9\xbd\xd1\xa9\xc1\xab..\xb3\xac\xbc\xab\xb0\xf4\xb5\xc4\xe0\xa1...");
    break;

#ifdef MAPLE   
   case Ctrl('R'):
     if (currutmp->msgs[0].last_pid)
     {
     show_last_call_in();
     //% my_write(currutmp->msgs[0].last_pid, "水球丢回去：");
     my_write(currutmp->msgs[0].last_pid, "\xcb\xae\xc7\xf2\xb6\xaa\xbb\xd8\xc8\xa5\xa3\xba");
     }
    d.nodone=0;
    break;
#endif  // END MAPLE
   }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
  
  return 0;
}
