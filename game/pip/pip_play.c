/*---------------------------------------------------------------------------*/
/* 玩乐选单:散步 旅游 运动 约会 猜拳                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

int pip_play_stroll()	/*散步*/
{
    int lucky;
    count_tired(3,3,"Y",100,0);
    lucky=rand()%7;
    if(lucky==2)
     {
      d.happy+=rand()%3+rand()%3+9;
      d.satisfy+=rand()%3+rand()%3+3;
      d.shit+=rand()%3+3;
      d.hp-=(rand()%3+5);
      move(4,0);
      if(rand()%2>0)
        show_play_pic(1);
      else
        show_play_pic(2);
      //% pressanykey("遇到朋友罗  真好.... ^_^");
      pressanykey("\xd3\xf6\xb5\xbd\xc5\xf3\xd3\xd1\xc2\xde  \xd5\xe6\xba\xc3.... ^_^");
     }
    else if(lucky==3)
     {
      d.money+=100;
      d.happy+=rand()%3+6;
      d.satisfy+=rand()%3+4;
      d.shit+=rand()%3+3;
      d.hp-=(rand()%3+4);
      move(4,0);
      show_play_pic(3);
      //% pressanykey("捡到了100元了..耶耶耶....");
      pressanykey("\xbc\xf1\xb5\xbd\xc1\xcb100\xd4\xaa\xc1\xcb..\xd2\xae\xd2\xae\xd2\xae....");
     }

    else if(lucky==4)
     {
      if(rand()%2>0)
      {
       d.happy-=(rand()%2+5);
       move(4,0);
       d.hp-=(rand()%3+3);
       show_play_pic(4);
       if(d.money>=50)
        {
         d.money-=50;
         //% pressanykey("掉了50元了..呜呜呜....");
         pressanykey("\xb5\xf4\xc1\xcb50\xd4\xaa\xc1\xcb..\xce\xd8\xce\xd8\xce\xd8....");
        }
       else
        {
         d.money=0;
         d.hp-=(rand()%3+3);
         //% pressanykey("钱掉光光了..呜呜呜....");
         pressanykey("\xc7\xae\xb5\xf4\xb9\xe2\xb9\xe2\xc1\xcb..\xce\xd8\xce\xd8\xce\xd8....");
        }
       d.shit+=rand()%3+2;
      }
      else
      {
       d.happy+=rand()%3+5;
       move(4,0);
       show_play_pic(5);
       if(d.money>=50)
        {
         d.money-=50;
         d.hp-=(rand()%3+3);
         //% pressanykey("用了50元了..不可以骂我喔....");
         pressanykey("\xd3\xc3\xc1\xcb50\xd4\xaa\xc1\xcb..\xb2\xbb\xbf\xc9\xd2\xd4\xc2\xee\xce\xd2\xe0\xb8....");
        }
       else
        {
         d.money=0;
         d.hp-=(rand()%3+3);
         //% pressanykey("钱被我偷用光光了..:p");
         pressanykey("\xc7\xae\xb1\xbb\xce\xd2\xcd\xb5\xd3\xc3\xb9\xe2\xb9\xe2\xc1\xcb..:p");
        }
       d.shit+=rand()%3+2;
      }
     }
    else if(lucky==5)
     {
      d.happy+=rand()%3+6;
      d.satisfy+=rand()%3+5;
      d.shit+=2;
      move(4,0);
      if(rand()%2>0)
        show_play_pic(6);
      else
        show_play_pic(7);
      //% pressanykey("好棒喔捡到玩具了说.....");
      pressanykey("\xba\xc3\xb0\xf4\xe0\xb8\xbc\xf1\xb5\xbd\xcd\xe6\xbe\xdf\xc1\xcb\xcb\xb5.....");
     }
    else if(lucky==6)
     {
      d.happy-=(rand()%3+10);
      d.shit+=(rand()%3+20);
      move(4,0);
      show_play_pic(9);
      //% pressanykey("真是倒楣  可以去买爱国奖券");
      pressanykey("\xd5\xe6\xca\xc7\xb5\xb9\xe9\xb9  \xbf\xc9\xd2\xd4\xc8\xa5\xc2\xf2\xb0\xae\xb9\xfa\xbd\xb1\xc8\xaf");
     }
    else
    {
      d.happy+=rand()%3+3;
      d.satisfy+=rand()%2+1;
      d.shit+=rand()%3+2;
      d.hp-=(rand()%3+2);
      move(4,0);
      show_play_pic(8);
      //% pressanykey("没有特别的事发生啦.....");
      pressanykey("\xc3\xbb\xd3\xd0\xcc\xd8\xb1\xf0\xb5\xc4\xca\xc2\xb7\xa2\xc9\xfa\xc0\xb2.....");
    }
    return 0;
}

int pip_play_sport()	/*运动*/
{
    count_tired(3,8,"Y",100,1);
    d.weight-=(rand()%3+2);
    d.satisfy+=rand()%2+3;
    if(d.satisfy>100)
      d.satisfy=100;
    d.shit+=rand()%5+10;
    d.hp-=(rand()%2+8);
    d.maxhp+=rand()%2;
    d.speed+=(2+rand()%3);
    move(4,0);
    show_play_pic(10);
    //% pressanykey("运动好处多多啦...");
    pressanykey("\xd4\xcb\xb6\xaf\xba\xc3\xb4\xa6\xb6\xe0\xb6\xe0\xc0\xb2...");
    return 0;
}

int pip_play_date()	/*约会*/
{
    if(d.money<150)
    {
     //% pressanykey("你钱不够多啦! 约会总得花点钱钱");
     pressanykey("\xc4\xe3\xc7\xae\xb2\xbb\xb9\xbb\xb6\xe0\xc0\xb2! \xd4\xbc\xbb\xe1\xd7\xdc\xb5\xc3\xbb\xa8\xb5\xe3\xc7\xae\xc7\xae");
    }
    else
    {
     count_tired(3,6,"Y",100,1);
     d.happy+=rand()%5+12;
     d.shit+=rand()%3+5;
     d.hp-=rand()%4+8;
     d.satisfy+=rand()%5+7;
     d.character+=rand()%3+1;
     d.money=d.money-150;
     move(4,0);
     show_play_pic(11);
     //% pressanykey("约会去  呼呼");
     pressanykey("\xd4\xbc\xbb\xe1\xc8\xa5  \xba\xf4\xba\xf4");
    }
    return 0;
}
int pip_play_outing()	/*郊游*/
{
    int lucky;
    char buf[256];
    
    if(d.money<250)
    {
     //% pressanykey("你钱不够多啦! 旅游总得花点钱钱");
     pressanykey("\xc4\xe3\xc7\xae\xb2\xbb\xb9\xbb\xb6\xe0\xc0\xb2! \xc2\xc3\xd3\xce\xd7\xdc\xb5\xc3\xbb\xa8\xb5\xe3\xc7\xae\xc7\xae");
    }
    else
    { 
      d.weight+=rand()%2+1;
      d.money-=250;   
      count_tired(10,45,"N",100,0);
      d.hp-=rand()%10+20;
      if(d.hp>=d.maxhp)
           d.hp=d.maxhp;
      d.happy+=rand()%10+12;
      d.character+=rand()%5+5;
      d.satisfy+=rand()%10+10;
      lucky=rand()%4;
      if(lucky==0)
      {
       d.maxmp+=rand()%3;
       d.art+=rand()%2;
       show_play_pic(12);
       if(rand()%2>0)
         //% pressanykey("心中有一股淡淡的感觉  好舒服喔....");
         pressanykey("\xd0\xc4\xd6\xd0\xd3\xd0\xd2\xbb\xb9\xc9\xb5\xad\xb5\xad\xb5\xc4\xb8\xd0\xbe\xf5  \xba\xc3\xca\xe6\xb7\xfe\xe0\xb8....");
       else
         //% pressanykey("云水 闲情 心情好多了.....");
         pressanykey("\xd4\xc6\xcb\xae \xcf\xd0\xc7\xe9 \xd0\xc4\xc7\xe9\xba\xc3\xb6\xe0\xc1\xcb.....");
      }
      else if(lucky==1)
      {
       d.art+=rand()%3;
       d.maxmp+=rand()%2;
       show_play_pic(13);
       if(rand()%2>0)
         //% pressanykey("有山有水有落日  形成一幅美丽的画..");
         pressanykey("\xd3\xd0\xc9\xbd\xd3\xd0\xcb\xae\xd3\xd0\xc2\xe4\xc8\xd5  \xd0\xce\xb3\xc9\xd2\xbb\xb7\xf9\xc3\xc0\xc0\xf6\xb5\xc4\xbb\xad..");
       else
         //% pressanykey("看着看着  全身疲惫都不见罗..");
         pressanykey("\xbf\xb4\xd7\xc5\xbf\xb4\xd7\xc5  \xc8\xab\xc9\xed\xc6\xa3\xb1\xb9\xb6\xbc\xb2\xbb\xbc\xfb\xc2\xde..");
      }
      else if(lucky==2)
      {
       d.love+=rand()%3;
       show_play_pic(14);
       if(rand()%2>0)
         //% pressanykey("看  太阳快没入水中罗...");
         pressanykey("\xbf\xb4  \xcc\xab\xd1\xf4\xbf\xec\xc3\xbb\xc8\xeb\xcb\xae\xd6\xd0\xc2\xde...");
       else
         //% pressanykey("听说这是海边啦  你说呢?");
         pressanykey("\xcc\xfd\xcb\xb5\xd5\xe2\xca\xc7\xba\xa3\xb1\xdf\xc0\xb2  \xc4\xe3\xcb\xb5\xc4\xd8?");
      }      
      else if(lucky==3)
      {
       d.maxhp+=rand()%3;
       show_play_pic(15);
       if(rand()%2>0)
         //% pressanykey("让我们疯狂在夜里的海滩吧....呼呼..");
         pressanykey("\xc8\xc3\xce\xd2\xc3\xc7\xb7\xe8\xbf\xf1\xd4\xda\xd2\xb9\xc0\xef\xb5\xc4\xba\xa3\xcc\xb2\xb0\xc9....\xba\xf4\xba\xf4..");
       else
         //% pressanykey("凉爽的海风迎面袭来  最喜欢这种感觉了....");
         pressanykey("\xc1\xb9\xcb\xac\xb5\xc4\xba\xa3\xb7\xe7\xd3\xad\xc3\xe6\xcf\xae\xc0\xb4  \xd7\xee\xcf\xb2\xbb\xb6\xd5\xe2\xd6\xd6\xb8\xd0\xbe\xf5\xc1\xcb....");
      }
      if((rand()%301+rand()%200)%100==12)
      {
        lucky=0;
        clear();
        //% sprintf(buf,"[1;41m  星空战斗鸡 ～ %-10s                                                    [0m",d.name); 	 
        sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab %-10s                                                    [0m",d.name); 	 
        show_play_pic(0);
        move(17,10);
        //% prints("[1;36m亲爱的 [1;33m%s ～[0m",d.name);
        prints("[1;36m\xc7\xd7\xb0\xae\xb5\xc4 [1;33m%s \xa1\xab[0m",d.name);
        move(18,10);
        //% prints("[1;37m看到你这样努力的培养自己的能力  让我心中十分的高兴喔..[m");
        prints("[1;37m\xbf\xb4\xb5\xbd\xc4\xe3\xd5\xe2\xd1\xf9\xc5\xac\xc1\xa6\xb5\xc4\xc5\xe0\xd1\xf8\xd7\xd4\xbc\xba\xb5\xc4\xc4\xdc\xc1\xa6  \xc8\xc3\xce\xd2\xd0\xc4\xd6\xd0\xca\xae\xb7\xd6\xb5\xc4\xb8\xdf\xd0\xcb\xe0\xb8..[m");
        move(19,10);
        //% prints("[1;36m小天使我决定给你奖赏鼓励鼓励  偷偷地帮助你一下....^_^[0m");
        prints("[1;36m\xd0\xa1\xcc\xec\xca\xb9\xce\xd2\xbe\xf6\xb6\xa8\xb8\xf8\xc4\xe3\xbd\xb1\xc9\xcd\xb9\xc4\xc0\xf8\xb9\xc4\xc0\xf8  \xcd\xb5\xcd\xb5\xb5\xd8\xb0\xef\xd6\xfa\xc4\xe3\xd2\xbb\xcf\xc2....^_^[0m");
        move(20,10);
        lucky=rand()%7;
        if(lucky==6)
        {
          //% prints("[1;33m我将帮你的各项能力全部提升百分之五喔......[0m");
          prints("[1;33m\xce\xd2\xbd\xab\xb0\xef\xc4\xe3\xb5\xc4\xb8\xf7\xcf\xee\xc4\xdc\xc1\xa6\xc8\xab\xb2\xbf\xcc\xe1\xc9\xfd\xb0\xd9\xb7\xd6\xd6\xae\xce\xe5\xe0\xb8......[0m");
          d.maxhp=d.maxhp*105/100;
          d.hp=d.maxhp;
          d.maxmp=d.maxmp*105/100;
          d.mp=d.maxmp;          
          d.attack=d.attack*105/100;
          d.resist=d.resist*105/100;          
          d.speed=d.speed*105/100;          
          d.character=d.character*105/100;          
          d.love=d.love*105/100;          
          d.wisdom=d.wisdom*105/100;     
          d.art=d.art*105/100;               
          d.brave=d.brave*105/100;          
          d.homework=d.homework*105/100;          
        }
        
        else if(lucky<=5 && lucky>=4)
        {
          //% prints("[1;33m我将帮你的战斗能力全部提升百分之十喔.......[0m");        
          prints("[1;33m\xce\xd2\xbd\xab\xb0\xef\xc4\xe3\xb5\xc4\xd5\xbd\xb6\xb7\xc4\xdc\xc1\xa6\xc8\xab\xb2\xbf\xcc\xe1\xc9\xfd\xb0\xd9\xb7\xd6\xd6\xae\xca\xae\xe0\xb8.......[0m");        
          d.attack=d.attack*110/100;
          d.resist=d.resist*110/100;          
          d.speed=d.speed*110/100;        
          d.brave=d.brave*110/100;                              
        } 
               
        else if(lucky<=3 && lucky>=2)
        {
          //% prints("[1;33m我将帮你的魔法能力和生命力全部提升百分之十喔.......[0m");        
          prints("[1;33m\xce\xd2\xbd\xab\xb0\xef\xc4\xe3\xb5\xc4\xc4\xa7\xb7\xa8\xc4\xdc\xc1\xa6\xba\xcd\xc9\xfa\xc3\xfc\xc1\xa6\xc8\xab\xb2\xbf\xcc\xe1\xc9\xfd\xb0\xd9\xb7\xd6\xd6\xae\xca\xae\xe0\xb8.......[0m");        
          d.maxhp=d.maxhp*110/100;
          d.hp=d.maxhp;
          d.maxmp=d.maxmp*110/100;
          d.mp=d.maxmp;                  
        }
        else if(lucky<=1 && lucky>=0)
        {
          //% prints("[1;33m我将帮你的感受能力全部提升百分之二十喔....[0m");                
          prints("[1;33m\xce\xd2\xbd\xab\xb0\xef\xc4\xe3\xb5\xc4\xb8\xd0\xca\xdc\xc4\xdc\xc1\xa6\xc8\xab\xb2\xbf\xcc\xe1\xc9\xfd\xb0\xd9\xb7\xd6\xd6\xae\xb6\xfe\xca\xae\xe0\xb8....[0m");                
          d.character=d.character*110/100;          
          d.love=d.love*110/100;          
          d.wisdom=d.wisdom*110/100;     
          d.art=d.art*110/100;               
          d.homework=d.homework*110/100;                  
        }        
        
        //% pressanykey("请继续加油喔..."); 
        pressanykey("\xc7\xeb\xbc\xcc\xd0\xf8\xbc\xd3\xd3\xcd\xe0\xb8..."); 
      }
    }
    return 0;
}

int pip_play_kite()	/*风筝*/
{
    count_tired(4,4,"Y",100,0);
    d.weight+=(rand()%2+2);
    d.satisfy+=rand()%3+12;
    if(d.satisfy>100)
      d.satisfy=100;
    d.happy+=rand()%5+10;
    d.shit+=rand()%5+6;
    d.hp-=(rand()%2+7);
    d.affect+=rand()%4+6;
    move(4,0);
    show_play_pic(16);
    //% pressanykey("放风筝真好玩啦...");
    pressanykey("\xb7\xc5\xb7\xe7\xf3\xdd\xd5\xe6\xba\xc3\xcd\xe6\xc0\xb2...");
    return 0;
}

int pip_play_KTV()	/*KTV*/
{
    if(d.money<250)
    {
     //% pressanykey("你钱不够多啦! 唱歌总得花点钱钱");
     pressanykey("\xc4\xe3\xc7\xae\xb2\xbb\xb9\xbb\xb6\xe0\xc0\xb2! \xb3\xaa\xb8\xe8\xd7\xdc\xb5\xc3\xbb\xa8\xb5\xe3\xc7\xae\xc7\xae");
    }
    else
    {
     count_tired(10,10,"Y",100,0);
     d.satisfy+=rand()%2+20;
     if(d.satisfy>100)
       d.satisfy=100;
     d.happy+=rand()%3+20;
     d.shit+=rand()%5+6;
     d.money-=250;
     d.hp+=(rand()%2+6);
     d.art+=rand()%4+3;
     move(4,0);
     show_play_pic(17);
     //% pressanykey("你说你  想要逃...");
     pressanykey("\xc4\xe3\xcb\xb5\xc4\xe3  \xcf\xeb\xd2\xaa\xcc\xd3...");
    }
    return 0;
}

int pip_play_guess()   /* 猜拳程式 */
{
   int ch,com;
   int pipkey;
   char inbuf[10];
   struct tm *qtime;
   time_t now;

   time(&now);
   qtime = localtime(&now);
   d.satisfy+=(rand()%3+2);
   count_tired(2,2,"Y",100,1);
   d.shit+=rand()%3+2;
   do
   {
    if(d.death==1 || d.death==2 || d.death==3)
      return 0;  
    if(pip_mainmenu(0)) return 0;
    move(b_lines-2,0);
    clrtoeol();  
    move(b_lines, 0);
    clrtoeol();
    move(b_lines,0);
    //% prints("[1;44;37m  猜拳选单  [46m[1]我出剪刀 [2]我出石头 [3]我出布啦 [4]猜拳记录 [Q]跳出：         [m");   
    prints("[1;44;37m  \xb2\xc2\xc8\xad\xd1\xa1\xb5\xa5  [46m[1]\xce\xd2\xb3\xf6\xbc\xf4\xb5\xb6 [2]\xce\xd2\xb3\xf6\xca\xaf\xcd\xb7 [3]\xce\xd2\xb3\xf6\xb2\xbc\xc0\xb2 [4]\xb2\xc2\xc8\xad\xbc\xc7\xc2\xbc [Q]\xcc\xf8\xb3\xf6\xa3\xba         [m");   
    move(b_lines-1, 0);
    clrtoeol();
    pipkey=egetch();
    switch(pipkey)
    {
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
      case '4':
             situ();
             break;
     }
   }while((pipkey!='1')&&(pipkey!='2')&&(pipkey!='3')&&(pipkey !='q')&&(pipkey !='Q'));

    com=rand()%3;
    move(18,0);
    clrtobot();
    switch(com){
        case 0:
          //% outs("小鸡：剪刀\n");
          outs("\xd0\xa1\xbc\xa6\xa3\xba\xbc\xf4\xb5\xb6\n");
         break;
        case 1:
          //% outs("小鸡：石头\n");
          outs("\xd0\xa1\xbc\xa6\xa3\xba\xca\xaf\xcd\xb7\n");
         break;
        case 2:
          //% outs("小鸡：布\n");
          outs("\xd0\xa1\xbc\xa6\xa3\xba\xb2\xbc\n");
         break;
     }

    move(17,0);

    switch(pipkey){
    case '1':
      //% outs("你  ：剪刀\n");
      outs("\xc4\xe3  \xa3\xba\xbc\xf4\xb5\xb6\n");
      if (com==0)
        tie();
      else  if (com==1)
        lose();
      else if (com==2)
        win();
      break;
    case '2':
      //% outs("你　：石头\n");
      outs("\xc4\xe3\xa1\xa1\xa3\xba\xca\xaf\xcd\xb7\n");
      if (com==0)
        win();
      else if (com==1)
        tie();
      else if (com==2)
        lose();
      break;
    case '3':
      //% outs("你　：布\n");
      outs("\xc4\xe3\xa1\xa1\xa3\xba\xb2\xbc\n");
      if (com==0)
        lose();
      else if (com==1)
        win();
      else if (com==2)
        tie();
      break;
    case 'q':
      break;
  }

}

int win()
{
    d.winn++;
    d.hp-=rand()%2+3;
    move(4,0);
    show_guess_pic(2);
    move(b_lines,0);
    //% pressanykey("小鸡输了....~>_<~");
    pressanykey("\xd0\xa1\xbc\xa6\xca\xe4\xc1\xcb....~>_<~");
    return;
}
int tie()
{
    d.hp-=rand()%2+3;
    d.happy+=rand()%3+5;
    move(4,0);
    show_guess_pic(3);
    move(b_lines,0);
    //% pressanykey("平手........-_-");
    pressanykey("\xc6\xbd\xca\xd6........-_-");
        return;
}
int lose()
{
    d.losee++;
    d.happy+=rand()%3+5;
    d.hp-=rand()%2+3;
    move(4,0);
    show_guess_pic(1);
    move(b_lines,0);
    //% pressanykey("小鸡赢罗....*^_^*");
    pressanykey("\xd0\xa1\xbc\xa6\xd3\xae\xc2\xde....*^_^*");
    return;
}

int situ()
{
        clrchyiuan(19,21);
        move(19,0);
        //% prints("你:[44m %d胜 %d负[m                     \n",d.winn,d.losee);
        prints("\xc4\xe3:[44m %d\xca\xa4 %d\xb8\xba[m                     \n",d.winn,d.losee);
        move(20,0);
        //% prints("鸡:[44m %d胜 %d负[m                     \n",d.losee,d.winn);
        prints("\xbc\xa6:[44m %d\xca\xa4 %d\xb8\xba[m                     \n",d.losee,d.winn);

       if (d.winn>=d.losee)
       {
        move(b_lines,0);
        //% pressanykey("哈..赢小鸡也没多光荣");
        pressanykey("\xb9\xfe..\xd3\xae\xd0\xa1\xbc\xa6\xd2\xb2\xc3\xbb\xb6\xe0\xb9\xe2\xc8\xd9");
       }
       else
       {
        move(b_lines,0);
        //% pressanykey("笨蛋..竟输给了鸡....ㄜ...");
        pressanykey("\xb1\xbf\xb5\xb0..\xbe\xb9\xca\xe4\xb8\xf8\xc1\xcb\xbc\xa6....\xa8\xdc...");
       }
       return;
}
