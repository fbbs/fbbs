/*---------------------------------------------------------------------------*/
/* ª˘±æ—°µ•:“˚ ≥ «ÂΩ‡ «◊«◊ –›œ¢                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)
int pip_basic_takeshower() /*œ¥‘Ë*/
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
         pressanykey("Œ“ ««¨æªµƒ–°º¶  cccc....");
       }
     else if(lucky==1)
       {
         show_usual_pic(7);
         pressanykey("¬ÌÕ∞ ‡≈°´°´");       
       }
     else
       {
         show_usual_pic(2);
         pressanykey("Œ“∞Æœ¥‘Ë lalala....");
       }
     return 0;
}

int pip_basic_takerest() /*–›œ¢*/
{
//     count_tired(5,20,"Y",100,0);
     d.tired-=rand() % 15;
     d.tired = d.tired > 0 ? d.tired : 0;
     
     if(d.hp>d.maxhp)
       d.hp=d.maxhp;
     d.shit+=1;
     move(4,0);
     show_usual_pic(5);
     pressanykey("‘Ÿ∞¥“ªœ¬Œ“æÕ∆¥≤¬ﬁ....");
     show_usual_pic(6);
     pressanykey("ŒπŒπŒπ..∏√∆¥≤¬ﬁ......");
     return 0;
}

int pip_basic_kiss()/*«◊«◊*/
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
       pressanykey("¿¥¬Ô! ‡£“ª∏ˆ.....");
      }
     else
      {
       pressanykey("«◊Ã´∂‡“≤ «ª·‘‡À¿µƒ‡∏....");
      }
     return 0;
}
int pip_basic_feed()     /* “˚ ≥*/
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
   sprintf(buf,"%s∏√◊ˆ ≤˜· ¬ƒÿ?",d.name);    
   prints(buf);   
   now=time(0);   
   move(b_lines, 0);
   clrtoeol();   
   move(b_lines, 0);
   prints("[1;44;37m  “˚ ≥—°µ•  [46m[1]≥‘∑π [2]¡„ ≥ [3]≤πÕË [4]¡È÷• [5]»À≤Œ [6]—©¡´ [Q]Ã¯≥ˆ£∫         [m");   
   pip_time_change(now);
   pipkey=egetch();
   pip_time_change(now);

   switch(pipkey)
   {
   case '1':
    if(d.food<=0)
     {
      move(b_lines,0);
      pressanykey("√ª”– ≥ŒÔ¬ﬁ..øÏ»•¬Ú∞…£°");
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
    pressanykey("√ø≥‘“ª¥Œ ≥ŒÔª·ª÷∏¥ÃÂ¡¶50‡∏!");
    break;

   case '2':
    if(d.cookie<=0)
    {
      move(b_lines,0);
      pressanykey("¡„ ≥≥‘π‚¬ﬁ..øÏ»•¬Ú∞…£°");
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
    pressanykey("≥‘¡„ ≥»›“◊≈÷‡∏...");
    break;

   case '3':
    if(d.bighp<=0)
    {
      move(b_lines,0);
      pressanykey("√ª”–¥Û≤πÕË¡À“Æ! øÏ¬Ú∞…..");
      break;
    }
    d.bighp--;
    d.hp+=600;
    d.tired-=20;
    d.weight+=rand()%2;
    move(4,0);
    show_feed_pic(4);
    d.nodone=0;
    pressanykey("≤πÕË..≥¨º´∞Ùµƒ‡°...");
    break;

   case '4':
    if(d.medicine<=0)
     {
      move(b_lines,0);
      pressanykey("√ª”–¡È÷•¬ﬁ..øÏ»•¬Ú∞…£°");
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
    pressanykey("√ø≥‘“ª¥Œ¡È÷•ª·ª÷∏¥∑®¡¶50‡∏!");
    break;

   case '5':
    if(d.ginseng<=0)
    {
      move(b_lines,0);
      pressanykey("√ª”–«ßƒÍ»À≤Œ“Æ! øÏ¬Ú∞…..");
      break;
    }
    d.ginseng--;
    d.mp+=500;
    d.tired-=20;
    move(4,0);
    show_feed_pic(1);
    d.nodone=0;
    pressanykey("«ßƒÍ»À  ..≥¨º´∞Ùµƒ‡°...");
    break;

   case '6':
    if(d.snowgrass<=0)
    {
      move(b_lines,0);
      pressanykey("√ª”–ÃÏ…Ω—©¡´“Æ! øÏ¬Ú∞…..");
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
    pressanykey("ÃÏ…Ω—©¡´..≥¨º´∞Ùµƒ‡°...");
    break;

#ifdef MAPLE   
   case Ctrl('R'):
     if (currutmp->msgs[0].last_pid)
     {
     show_last_call_in();
     my_write(currutmp->msgs[0].last_pid, "ÀÆ«Ú∂™ªÿ»•£∫");
     }
    d.nodone=0;
    break;
#endif  // END MAPLE
   }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
  
  return 0;
}
