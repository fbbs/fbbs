/*---------------------------------------------------------------------------*/
/* 战斗特区                                                                  */
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
//% "榕树怪",	 60,0,	20,0,	20,	20,	 20,	150, "11101",	0,0,
"\xe9\xc5\xca\xf7\xb9\xd6",	 60,0,	20,0,	20,	20,	 20,	150, "11101",	0,0,
//% "网路魔",	 60,0,	20,0,	30,	30,	 30,	200, "01111",	0,0,
"\xcd\xf8\xc2\xb7\xc4\xa7",	 60,0,	20,0,	30,	30,	 30,	200, "01111",	0,0,
//% "蘑菇小怪",	 80,0,	40,0,	50,	35,	 60,	250, "11110",	0,0,
"\xc4\xa2\xb9\xbd\xd0\xa1\xb9\xd6",	 80,0,	40,0,	50,	35,	 60,	250, "11110",	0,0,
//% "毒蝎",		 85,0,	30,0,	80,	90,	 80,	500, "10111",	0,0,
"\xb6\xbe\xd0\xab",		 85,0,	30,0,	80,	90,	 80,	500, "10111",	0,0,
//% "恶狗",		 90,0,  50,0,   75,	70,	 60,	550, "11010",	0,0,
"\xb6\xf1\xb9\xb7",		 90,0,  50,0,   75,	70,	 60,	550, "11010",	0,0,
//% "红眼鬼猫",	130,0,	50,0,	75,	90,	 70,	500, "11011",	0,0,
"\xba\xec\xd1\xdb\xb9\xed\xc3\xa8",	130,0,	50,0,	75,	90,	 70,	500, "11011",	0,0,
//% "紫色魔鲨",	140,0,	60,0,	80,	80,	 80,	550, "10101",	0,0,
"\xd7\xcf\xc9\xab\xc4\xa7\xf6\xe8",	140,0,	60,0,	80,	80,	 80,	550, "10101",	0,0,
//% "怪物蟑螂",	150,0,	70,0,	85,	70,	 67,	500, "11110",	0,0,
"\xb9\xd6\xce\xef\xf3\xaf\xf2\xeb",	150,0,	70,0,	85,	70,	 67,	500, "11110",	0,0,
//% "蜘蛛精",	180,0,	50,0,   90,	90,	 80,	850, "00111",	0,0,
"\xd6\xa9\xd6\xeb\xbe\xab",	180,0,	50,0,   90,	90,	 80,	850, "00111",	0,0,
//% "食人巫师",	175,0, 100,0,  100,	80,	 60,    800, "11010",	0,0,
"\xca\xb3\xc8\xcb\xce\xd7\xca\xa6",	175,0, 100,0,  100,	80,	 60,    800, "11010",	0,0,
//% "大槌怪",	240,0,  80,0,  110,    100,	 70,    800, "00111",	0,0,
"\xb4\xf3\xe9\xb3\xb9\xd6",	240,0,  80,0,  110,    100,	 70,    800, "00111",	0,0,
//% "白色恶魔",	250,0,  60,0,  120,    110,	 80,    900, "01011",	0,0,
"\xb0\xd7\xc9\xab\xb6\xf1\xc4\xa7",	250,0,  60,0,  120,    110,	 80,    900, "01011",	0,0,
//% "死神魔",	280,0,  80,0,  150,    120,	 90,   1200, "00011",   0,0,
"\xcb\xc0\xc9\xf1\xc4\xa7",	280,0,  80,0,  150,    120,	 90,   1200, "00011",   0,0,
//% "大恐龙",	300,0,	50,0,  160,    120,	 90,   1500, "11001",	0,0,
"\xb4\xf3\xbf\xd6\xc1\xfa",	300,0,	50,0,  160,    120,	 90,   1500, "11001",	0,0,
//% "超级喷火龙",	500,0, 100,0,  250,    250,	150,   1500, "11000",	0,0,
"\xb3\xac\xbc\xb6\xc5\xe7\xbb\xf0\xc1\xfa",	500,0, 100,0,  250,    250,	150,   1500, "11000",	0,0,
//% "骷髅头怪",	600,0, 400,0,  350,    400,	250,   2000, "00110",	0,0,
"\xf7\xbc\xf7\xc3\xcd\xb7\xb9\xd6",	600,0, 400,0,  350,    400,	250,   2000, "00110",	0,0,
//% "阿强一号",	700,0, 500,0,  600,    900,     500,   2000, "10011",	0,0,
"\xb0\xa2\xc7\xbf\xd2\xbb\xba\xc5",	700,0, 500,0,  600,    900,     500,   2000, "10011",	0,0,
//% "面具怪人",	700,0, 500,0,  800,    850,	300,   2000, "11100",	0,0,
"\xc3\xe6\xbe\xdf\xb9\xd6\xc8\xcb",	700,0, 500,0,  800,    850,	300,   2000, "11100",	0,0,
//% "U2外星人",	800,0, 600,0,  800,    800,	600,   2000, "11010",	0,0,
"U2\xcd\xe2\xd0\xc7\xc8\xcb",	800,0, 600,0,  800,    800,	600,   2000, "11010",	0,0,
//% "中国疆  ",	800,0, 600,0, 1000,   1000,     500,   2000, "10100",	0,0,
"\xd6\xd0\xb9\xfa\xbd\xae  ",	800,0, 600,0, 1000,   1000,     500,   2000, "10100",	0,0,
//% "彩色酋长",     900,0, 800,0, 1200,   1200,     600,   3000, "11100",   0,0,
"\xb2\xca\xc9\xab\xc7\xf5\xb3\xa4",     900,0, 800,0, 1200,   1200,     600,   3000, "11100",   0,0,
//% "魔音吉他手",  1000,0, 850,0, 1400,   1000,     650,   3000, "11001",   0,0,
"\xc4\xa7\xd2\xf4\xbc\xaa\xcb\xfb\xca\xd6",  1000,0, 850,0, 1400,   1000,     650,   3000, "11001",   0,0,
//% "万年老龟",    1200,0,1000,0, 1300,   1500,     500,   3000, "01011",   0,0,
"\xcd\xf2\xc4\xea\xc0\xcf\xb9\xea",    1200,0,1000,0, 1300,   1500,     500,   3000, "01011",   0,0,
//% "八神",	       1200,0, 900,0, 1500,   1300,     800,   3000, "10101",   0,0,
"\xb0\xcb\xc9\xf1",	       1200,0, 900,0, 1500,   1300,     800,   3000, "10101",   0,0,
//% "铁面人",      1500,0,1200,0, 1800,   1800,    1200,   4000, "00011",   0,0,
"\xcc\xfa\xc3\xe6\xc8\xcb",      1500,0,1200,0, 1800,   1800,    1200,   4000, "00011",   0,0,
//% "大嘴",        1600,0,1000,0, 1700,   1800,    1100,   4000, "00110",   0,0,
"\xb4\xf3\xd7\xec",        1600,0,1000,0, 1700,   1800,    1100,   4000, "00110",   0,0,
//% "骷髅兵",      1700,0,1500,0, 1800,   1800,    1250,  4000, "10110",   0,0,
"\xf7\xbc\xf7\xc3\xb1\xf8",      1700,0,1500,0, 1800,   1800,    1250,  4000, "10110",   0,0,
//% "熔化妖",      1750,0,1300,0, 1800,   2000,    1000,   4000, "01011",	0,0,
"\xc8\xdb\xbb\xaf\xd1\xfd",      1750,0,1300,0, 1800,   2000,    1000,   4000, "01011",	0,0,
//% "使徒",	       2500,0,2500,0, 2500,   2500,    2500,   5000, "10001",   0,0,
"\xca\xb9\xcd\xbd",	       2500,0,2500,0, 2500,   2500,    2500,   5000, "10001",   0,0,
//% "埃及木乃伊",  3500,0,3000,0, 3500,   3500,    2000,   5000, "10110",	0,0,
"\xb0\xa3\xbc\xb0\xc4\xbe\xc4\xcb\xd2\xc1",  3500,0,3000,0, 3500,   3500,    2000,   5000, "10110",	0,0,
//% "古小兔",      5000,0,4500,0, 5000,   6000,    4000,   5000, "11100",   0,0,
"\xb9\xc5\xd0\xa1\xcd\xc3",      5000,0,4500,0, 5000,   6000,    4000,   5000, "11100",   0,0,
//% "十字机器人",  6000,0,5000,0, 5500,   6500,    5000,   5000, "01001",	0,0,
"\xca\xae\xd7\xd6\xbb\xfa\xc6\xf7\xc8\xcb",  6000,0,5000,0, 5500,   6500,    5000,   5000, "01001",	0,0,
//% "飞天小巫师",  6500,0,5500,0, 6500,   6000,    6000,   6000, "01101",	0,0,
"\xb7\xc9\xcc\xec\xd0\xa1\xce\xd7\xca\xa6",  6500,0,5500,0, 6500,   6000,    6000,   6000, "01101",	0,0,
//% "炸蛋超人",    7000,0,6000,0, 8000,   7000,    6000,   6500, "00001",	0,0,
"\xd5\xa8\xb5\xb0\xb3\xac\xc8\xcb",    7000,0,6000,0, 8000,   7000,    6000,   6500, "00001",	0,0,
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
//% "治疗法术",	0,	0,	0,	0,	0,	0,
"\xd6\xce\xc1\xc6\xb7\xa8\xca\xf5",	0,	0,	0,	0,	0,	0,
//% "气疗术",	5,	1,	20,	0,	10,	0,
"\xc6\xf8\xc1\xc6\xca\xf5",	5,	1,	20,	0,	10,	0,
//% "凝神归元",     20,     1,	100,    0,	40,	0,
"\xc4\xfd\xc9\xf1\xb9\xe9\xd4\xaa",     20,     1,	100,    0,	40,	0,
//% "元灵归心",     50,     1,	400,    0,	60,	0,
"\xd4\xaa\xc1\xe9\xb9\xe9\xd0\xc4",     50,     1,	400,    0,	60,	0,
//% "五气朝元",     100,    2,	0,	2,	0,	0,
"\xce\xe5\xc6\xf8\xb3\xaf\xd4\xaa",     100,    2,	0,	2,	0,	0,
"NULL",		0,	0,	0,	0,	0,	0
};                            
     
struct magicset thundermagiclist[] = {
//% "雷系法术",	0,	0,	0,	0,	0,	0,
"\xc0\xd7\xcf\xb5\xb7\xa8\xca\xf5",	0,	0,	0,	0,	0,	0,
//% "雷咒",		10,	0,	20,	1,	5,	0,
"\xc0\xd7\xd6\xe4",		10,	0,	20,	1,	5,	0,
//% "五雷咒",	20,	0,	50,	1,	5,	0,
"\xce\xe5\xc0\xd7\xd6\xe4",	20,	0,	50,	1,	5,	0,
//% "天雷咒",	40,	0,	100,	1,	5,	0,
"\xcc\xec\xc0\xd7\xd6\xe4",	40,	0,	100,	1,	5,	0,
//% "雷之网",	100,	0,	250,	1,	10,	0,
"\xc0\xd7\xd6\xae\xcd\xf8",	100,	0,	250,	1,	10,	0,
//% "疯狂之雷",	200,	0,	500,	1,	10,	0,
"\xb7\xe8\xbf\xf1\xd6\xae\xc0\xd7",	200,	0,	500,	1,	10,	0,
//% "雷神之舞",	600,	0,	1500,	1,	10,	0,
"\xc0\xd7\xc9\xf1\xd6\xae\xce\xe8",	600,	0,	1500,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset icemagiclist[] = {
//% "冰系法术",	0,	0,	0,	0,	0,	0,
"\xb1\xf9\xcf\xb5\xb7\xa8\xca\xf5",	0,	0,	0,	0,	0,	0,
//% "冰咒",		10,	0,	20,	1,	5,	0,
"\xb1\xf9\xd6\xe4",		10,	0,	20,	1,	5,	0,
//% "寒冰咒",	20,	0,	50,	1,	5,	0,
"\xba\xae\xb1\xf9\xd6\xe4",	20,	0,	50,	1,	5,	0,
//% "玄冰咒",	40,	0,	100,	1,	5,	0,
"\xd0\xfe\xb1\xf9\xd6\xe4",	40,	0,	100,	1,	5,	0,
//% "风雷冰天",	100,	0,	250,	1,	10,	0,
"\xb7\xe7\xc0\xd7\xb1\xf9\xcc\xec",	100,	0,	250,	1,	10,	0,
//% "绝对零度之冰",	200,	0,	500,	1,	10,	0,
"\xbe\xf8\xb6\xd4\xc1\xe3\xb6\xc8\xd6\xae\xb1\xf9",	200,	0,	500,	1,	10,	0,
//% "冰神之舞",	500,	0,	1400,	1,	10,	0,
"\xb1\xf9\xc9\xf1\xd6\xae\xce\xe8",	500,	0,	1400,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset firemagiclist[] = {
//% "炎系法术",	0,	0,	0,	0,	0,	0,
"\xd1\xd7\xcf\xb5\xb7\xa8\xca\xf5",	0,	0,	0,	0,	0,	0,
//% "火术",		5,	0,	10,	1,	5,	0,
"\xbb\xf0\xca\xf5",		5,	0,	10,	1,	5,	0,
//% "炎咒",		10,	0,	20,	1,	5,	0,
"\xd1\xd7\xd6\xe4",		10,	0,	20,	1,	5,	0,
//% "炎杀咒",	20,	0,	50,	1,	5,	0,
"\xd1\xd7\xc9\xb1\xd6\xe4",	20,	0,	50,	1,	5,	0,
//% "炼狱真火",	40,	0,	100,	1,	5,	0,
"\xc1\xb6\xd3\xfc\xd5\xe6\xbb\xf0",	40,	0,	100,	1,	5,	0,
//% "火龙掌",	100,	0,	250,	1,	10,	0,
"\xbb\xf0\xc1\xfa\xd5\xc6",	100,	0,	250,	1,	10,	0,
//% "火龙招唤",	200,	0,	500,	1,	10,	0,
"\xbb\xf0\xc1\xfa\xd5\xd0\xbb\xbd",	200,	0,	500,	1,	10,	0,
//% "火神之舞",	600,	0,	1600,	1,	10,	0,
"\xbb\xf0\xc9\xf1\xd6\xae\xce\xe8",	600,	0,	1600,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset earthmagiclist[] = {
//% "土系法术",	0,	0,	0,	0,	0,	0,
"\xcd\xc1\xcf\xb5\xb7\xa8\xca\xf5",	0,	0,	0,	0,	0,	0,
//% "土咒",		10,	0,	20,	1,	5,	0,
"\xcd\xc1\xd6\xe4",		10,	0,	20,	1,	5,	0,
//% "飞岩术",	20,	0,	50,	1,	5,	0,
"\xb7\xc9\xd1\xd2\xca\xf5",	20,	0,	50,	1,	5,	0,
//% "地裂天崩",	40,	0,	100,	1,	5,	0,
"\xb5\xd8\xc1\xd1\xcc\xec\xb1\xc0",	40,	0,	100,	1,	5,	0,
//% "泰山压顶",	100,	0,	250,	1,	10,	0,
"\xcc\xa9\xc9\xbd\xd1\xb9\xb6\xa5",	100,	0,	250,	1,	10,	0,
//% "土龙召唤",	200,	0,	500,	1,	10,	0,
"\xcd\xc1\xc1\xfa\xd5\xd9\xbb\xbd",	200,	0,	500,	1,	10,	0,
//% "山神之舞",	450,	0,	1300,	1,	10,	0,
"\xc9\xbd\xc9\xf1\xd6\xae\xce\xe8",	450,	0,	1300,	1,	10,	0,
"NULL",           0,      0,      0,      0,      0,      0
};

struct magicset windmagiclist[] = {
//% "风系法术",	0,	0,	0,	0,	0,	0,
"\xb7\xe7\xcf\xb5\xb7\xa8\xca\xf5",	0,	0,	0,	0,	0,	0,
//% "风咒",		10,	0,	20,	1,	5,	0,
"\xb7\xe7\xd6\xe4",		10,	0,	20,	1,	5,	0,
//% "旋风咒",	20,	0,	50,	1,	5,	0,
"\xd0\xfd\xb7\xe7\xd6\xe4",	20,	0,	50,	1,	5,	0,
//% "狂风术",	40,	0,	100,	1,	5,	0,
"\xbf\xf1\xb7\xe7\xca\xf5",	40,	0,	100,	1,	5,	0,
//% "龙卷风",	100,	0,	250,	1,	10,	0,
"\xc1\xfa\xbe\xed\xb7\xe7",	100,	0,	250,	1,	10,	0,
//% "风卷残云",	200,	0,	500,	1,	10,	0,
"\xb7\xe7\xbe\xed\xb2\xd0\xd4\xc6",	200,	0,	500,	1,	10,	0,
//% "风神之舞",	400,	0,	1200,	1,	10,	0,
"\xb7\xe7\xc9\xf1\xd6\xae\xce\xe8",	400,	0,	1200,	1,	10,	0,
"NULL",		0,	0,	0,	0,	0,	0
};
/*---------------------------------------------------------------------------*/
/* 战斗人物决定函式                                                          */
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
/* 战斗战斗函式                                                              */
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
 int dinjure=0;		/*小鸡伤害力*/
 int minjure=0;		/*对方伤害力*/
 int dresistmore=0;	/*小鸡加强防御*/
 int mresistmore=0;	/*对方加强防御*/
 int oldhexp;		/*未战斗前格斗经验*/
 int oldmexp;		/*未战斗前魔法经验*/
 int oldbrave;		/*未战斗前勇敢*/
 int oldhskill;		/*未战斗前战斗技术*/
 int oldmskill;		/*未战斗前魔法技术*/
 int oldetchics;	/*未战斗前道德*/
 int oldmoney;		/*未战斗前金钱*/
 int oldtired;
 int oldhp;
 int winorlose=0;		/*1:you win 0:you loss*/
 
 /*随机产生人物 并且存好战斗前的一些数值*/
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
   if(m.hp<=0) /*敌人死掉了*/
   {
     m.hp=0;
     d.money+=m.money;
     m.death=1;
     d.brave+=rand()%4+3;
   }
   if(d.hp<=0 || d.tired>=100)  /*小鸡阵亡*/
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
   /*showtitle("电子养小鸡", BoardName);*/
   move(0,0);
   if(d.sex==1)
     //% sprintf(buf,"[1;41m  星空战斗鸡 ～ [32m♂ [37m%-10s                                                  [0m",d.name); 	 
     sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab [32m\xa1\xe1 [37m%-10s                                                  [0m",d.name); 	 
   else if(d.sex==2)
     //% sprintf(buf,"[1;41m  星空战斗鸡 ～ [33m♀ [37m%-10s                                                  [0m",d.name); 	 
     sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab [33m\xa1\xe2 [37m%-10s                                                  [0m",d.name); 	 
   else 
     //% sprintf(buf,"[1;41m  星空战斗鸡 ～ [34m？ [37m%-10s                                                  [0m",d.name); 	 
     sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab [34m\xa3\xbf [37m%-10s                                                  [0m",d.name); 	 
   prints(buf);    
   move(6,0);
   if(mode==1)
	   show_badman_pic(n);
   move(1,0);
   //% sprintf(buf,"[1;31m┌—————————————————————————————————————┐[m");
   sprintf(buf,"[1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
   prints(buf);
   move(2,0);
   /* lucky拿来当color用*/
   if(d.tired>=80)
      lucky=31;
   else if(d.tired>=60 && d.tired< 80)
      lucky=33;
   else
      lucky=37;
   sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
   sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);    
   //% sprintf(buf,"[1;31m│[33m生  命:[37m%-12s[33m法  力:[37m%-12s[33m疲  劳:[%dm%-12d[33m金  钱:[37m%-10d[31m│[m",
   sprintf(buf,"[1;31m\xa9\xa6[33m\xc9\xfa  \xc3\xfc:[37m%-12s[33m\xb7\xa8  \xc1\xa6:[37m%-12s[33m\xc6\xa3  \xc0\xcd:[%dm%-12d[33m\xbd\xf0  \xc7\xae:[37m%-10d[31m\xa9\xa6[m",
  	 inbuf1,inbuf2,lucky,d.tired,d.money);
   prints(buf);
   move(3,0);
   sprintf(inbuf1,"%d/%d",d.hexp,d.mexp);  
   //% sprintf(buf,"[1;31m│[33m攻  击:[37m%-10d  [33m防  御:[37m%-10d  [33m速  度:[37m%-5d       [33m经  验:[37m%-10s[31m│[m",
   sprintf(buf,"[1;31m\xa9\xa6[33m\xb9\xa5  \xbb\xf7:[37m%-10d  [33m\xb7\xc0  \xd3\xf9:[37m%-10d  [33m\xcb\xd9  \xb6\xc8:[37m%-5d       [33m\xbe\xad  \xd1\xe9:[37m%-10s[31m\xa9\xa6[m",
 	 d.attack,d.resist,d.speed,inbuf1);
   prints(buf);
   move(4,0);
   //% sprintf(buf,"[1;31m│[33m食  物:[37m%-5d       [33m大补丸:[37m%-5d       [33m零  食:[37m%-5d       [33m药  草:[37m%-5d     [31m│[m",
   sprintf(buf,"[1;31m\xa9\xa6[33m\xca\xb3  \xce\xef:[37m%-5d       [33m\xb4\xf3\xb2\xb9\xcd\xe8:[37m%-5d       [33m\xc1\xe3  \xca\xb3:[37m%-5d       [33m\xd2\xa9  \xb2\xdd:[37m%-5d     [31m\xa9\xa6[m",
 	 d.food,d.bighp,d.cookie,d.medicine);
   prints(buf);	 
   move(5,0);
   //% sprintf(buf,"[1;31m└—————————————————————————————————————┘[m");
   sprintf(buf,"[1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");
   prints(buf); 
   move(19,0);
   //% sprintf(buf,"[1;34m┌—————————————————————————————————————┐[m");
   sprintf(buf,"[1;34m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
   prints(buf);
   move(20,0);
   sprintf(inbuf1,"%d/%d",m.hp,m.maxhp);  
   sprintf(inbuf2,"%d/%d",m.mp,m.maxmp);     
   //% sprintf(buf,"[1;34m│[32m姓  名:[37m%-10s  [32m生  命:[37m%-11s [32m法  力:[37m%-11s                  [34m│[m",
   sprintf(buf,"[1;34m\xa9\xa6[32m\xd0\xd5  \xc3\xfb:[37m%-10s  [32m\xc9\xfa  \xc3\xfc:[37m%-11s [32m\xb7\xa8  \xc1\xa6:[37m%-11s                  [34m\xa9\xa6[m",
 	 p[n].name,inbuf1,inbuf2);
   prints(buf);
   move(21,0);
   //% sprintf(buf,"[1;34m│[32m攻  击:[37m%-6d      [32m防  御:[37m%-6d      [32m速  度:[37m%-6d      [32m金  钱:[37m%-6d    [34m│[m",
   sprintf(buf,"[1;34m\xa9\xa6[32m\xb9\xa5  \xbb\xf7:[37m%-6d      [32m\xb7\xc0  \xd3\xf9:[37m%-6d      [32m\xcb\xd9  \xb6\xc8:[37m%-6d      [32m\xbd\xf0  \xc7\xae:[37m%-6d    [34m\xa9\xa6[m",
 	 m.attack,m.resist,m.speed,m.money);
   prints(buf);
   move(22,0);
   //% sprintf(buf,"[1;34m└—————————————————————————————————————┘[m");
   sprintf(buf,"[1;34m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");
   prints(buf);
   move(b_lines,0);   
   //% sprintf(buf,"[1;44;37m  战斗命令  [46m  [1]普通  [2]全力  [3]魔法  [4]防御  [5]补充  [6]逃命         [m");
   sprintf(buf,"[1;44;37m  \xd5\xbd\xb6\xb7\xc3\xfc\xc1\xee  [46m  [1]\xc6\xd5\xcd\xa8  [2]\xc8\xab\xc1\xa6  [3]\xc4\xa7\xb7\xa8  [4]\xb7\xc0\xd3\xf9  [5]\xb2\xb9\xb3\xe4  [6]\xcc\xd3\xc3\xfc         [m");
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
        //% pressanykey("竟然没打中..:~~~"); 
        pressanykey("\xbe\xb9\xc8\xbb\xc3\xbb\xb4\xf2\xd6\xd0..:~~~"); 
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
        //% sprintf(buf,"普通攻击,对方生命力减低%d",dinjure);
        sprintf(buf,"\xc6\xd5\xcd\xa8\xb9\xa5\xbb\xf7,\xb6\xd4\xb7\xbd\xc9\xfa\xc3\xfc\xc1\xa6\xbc\xf5\xb5\xcd%d",dinjure);
        pressanykey(buf);
      }
      d.tired+=rand()%(n+1)/15+2;
      break;
     
     case '2':
      show_fight_pic(2);
      if(rand()%11==0)
      { 
        //% pressanykey("竟然没打中..:~~~");
        pressanykey("\xbe\xb9\xc8\xbb\xc3\xbb\xb4\xf2\xd6\xd0..:~~~");
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
          //% sprintf(buf,"全力攻击,对方生命力减低%d",dinjure);
          sprintf(buf,"\xc8\xab\xc1\xa6\xb9\xa5\xbb\xf7,\xb6\xd4\xb7\xbd\xc9\xfa\xc3\xfc\xc1\xa6\xbc\xf5\xb5\xcd%d",dinjure);
          pressanykey(buf);
        }
        else
        { 
          d.nodone=1;
          //% pressanykey("你的HP小於5啦..不行啦...");
          pressanykey("\xc4\xe3\xb5\xc4HP\xd0\xa1\xec\xb65\xc0\xb2..\xb2\xbb\xd0\xd0\xc0\xb2...");
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
          //% sprintf(buf,"治疗後,生命力提高%d 疲劳降低%d",oldhp,oldtired);
          sprintf(buf,"\xd6\xce\xc1\xc6\xe1\xe1,\xc9\xfa\xc3\xfc\xc1\xa6\xcc\xe1\xb8\xdf%d \xc6\xa3\xc0\xcd\xbd\xb5\xb5\xcd%d",oldhp,oldtired);
          pressanykey(buf);
        }
        else
        {
          if(rand()%15==0)
             //% pressanykey("竟然没打中..:~~~");  
             pressanykey("\xbe\xb9\xc8\xbb\xc3\xbb\xb4\xf2\xd6\xd0..:~~~");  
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
             //% sprintf(buf,"魔法攻击,对方生命力减低%d",dinjure);
             sprintf(buf,"\xc4\xa7\xb7\xa8\xb9\xa5\xbb\xf7,\xb6\xd4\xb7\xbd\xc9\xfa\xc3\xfc\xc1\xa6\xbc\xf5\xb5\xcd%d",dinjure);
             pressanykey(buf);           
          }
        }
      }
      break;    
     case '4':
      dresistmore=1;
      d.tired+=rand()%(n+1)/20+1;
      //% pressanykey("小鸡加强防御啦....");
      pressanykey("\xd0\xa1\xbc\xa6\xbc\xd3\xc7\xbf\xb7\xc0\xd3\xf9\xc0\xb2....");
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
      //% showtitle("电子养小鸡", BoardName); 
      showtitle("\xb5\xe7\xd7\xd3\xd1\xf8\xd0\xa1\xbc\xa6", BoardName); 
      move(10,0);
      //% prints("            [1;31m┌——————————————————————┐[m");
      prints("            [1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
      move(11,0);
      //% prints("            [1;31m│  [37m实力不强的小鸡 [33m%-10s                 [31m│[m",d.name);
      prints("            [1;31m\xa9\xa6  [37m\xca\xb5\xc1\xa6\xb2\xbb\xc7\xbf\xb5\xc4\xd0\xa1\xbc\xa6 [33m%-10s                 [31m\xa9\xa6[m",d.name);
      move(12,0);
      //% prints("            [1;31m│  [37m在与对手 [32m%-10s [37m战斗後落跑啦          [31m│[m",p[n].name);
      prints("            [1;31m\xa9\xa6  [37m\xd4\xda\xd3\xeb\xb6\xd4\xca\xd6 [32m%-10s [37m\xd5\xbd\xb6\xb7\xe1\xe1\xc2\xe4\xc5\xdc\xc0\xb2          [31m\xa9\xa6[m",p[n].name);
      move(13,0);
      sprintf(inbuf1,"%d/%d",d.hexp-oldhexp,d.mexp-oldmexp);  
      //% prints("            [1;31m│  [37m评价增加了 [36m%-5s [37m点  技术增加了 [36m%-2d/%-2d [37m点  [31m│[m",inbuf1,d.hskill-oldhskill,d.mskill-oldmskill);
      prints("            [1;31m\xa9\xa6  [37m\xc6\xc0\xbc\xdb\xd4\xf6\xbc\xd3\xc1\xcb [36m%-5s [37m\xb5\xe3  \xbc\xbc\xca\xf5\xd4\xf6\xbc\xd3\xc1\xcb [36m%-2d/%-2d [37m\xb5\xe3  [31m\xa9\xa6[m",inbuf1,d.hskill-oldhskill,d.mskill-oldmskill);
      move(14,0);
      //% sprintf(inbuf1,"%d [37m元",oldmoney-d.money);
      sprintf(inbuf1,"%d [37m\xd4\xaa",oldmoney-d.money);
      //% prints("            [1;31m│  [37m勇敢降低了 [36m%-5d [37m点  金钱减少了 [36m%-13s  [31m│[m",oldbrave-d.brave,inbuf1);
      prints("            [1;31m\xa9\xa6  [37m\xd3\xc2\xb8\xd2\xbd\xb5\xb5\xcd\xc1\xcb [36m%-5d [37m\xb5\xe3  \xbd\xf0\xc7\xae\xbc\xf5\xc9\xd9\xc1\xcb [36m%-13s  [31m\xa9\xa6[m",oldbrave-d.brave,inbuf1);
      move(15,0);
      //% prints("            [1;31m└——————————————————————┘[m");             
      prints("            [1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");             
      //% pressanykey("叁十六计 走为上策...");
      pressanykey("\xc8\xfe\xca\xae\xc1\xf9\xbc\xc6 \xd7\xdf\xce\xaa\xc9\xcf\xb2\xdf...");
      winorlose=0;
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
   }
   clear(); 
   move(0,0);
   if(d.sex==1)
     //% sprintf(buf,"[1;41m  星空战斗鸡 ～ [32m♂ [37m%-10s                                                  [0m",d.name); 	 
     sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab [32m\xa1\xe1 [37m%-10s                                                  [0m",d.name); 	 
   else if(d.sex==2)
     //% sprintf(buf,"[1;41m  星空战斗鸡 ～ [33m♀ [37m%-10s                                                  [0m",d.name); 	 
     sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab [33m\xa1\xe2 [37m%-10s                                                  [0m",d.name); 	 
   else 
     //% sprintf(buf,"[1;41m  星空战斗鸡 ～ [34m？ [37m%-10s                                                  [0m",d.name); 	 
     sprintf(buf,"[1;41m  \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6 \xa1\xab [34m\xa3\xbf [37m%-10s                                                  [0m",d.name); 	 
   prints(buf);    
   move(1,0);
   //% sprintf(buf,"[1;31m┌—————————————————————————————————————┐[m");
   sprintf(buf,"[1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
   prints(buf);
   move(2,0);
   /* lucky拿来当color用*/
   if(d.tired>=80)
      lucky=31;
   else if(d.tired>=60 && d.tired< 80)
      lucky=33;
   else
      lucky=37;
   sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
   sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);    
   //% sprintf(buf,"[1;31m│[33m生  命:[37m%-12s[33m法  力:[37m%-12s[33m疲  劳:[%dm%-12d[33m金  钱:[37m%-10d[31m│[m",
   sprintf(buf,"[1;31m\xa9\xa6[33m\xc9\xfa  \xc3\xfc:[37m%-12s[33m\xb7\xa8  \xc1\xa6:[37m%-12s[33m\xc6\xa3  \xc0\xcd:[%dm%-12d[33m\xbd\xf0  \xc7\xae:[37m%-10d[31m\xa9\xa6[m",
  	 inbuf1,inbuf2,lucky,d.tired,d.money);
   prints(buf);
   
   move(3,0);
   sprintf(inbuf1,"%d/%d",d.hexp,d.mexp);     
   //% sprintf(buf,"[1;31m│[33m攻  击:[37m%-6d      [33m防  御:[37m%-6d      [33m速  度:[37m%-5d       [33m经  验:[37m%-10s[31m│[m",
   sprintf(buf,"[1;31m\xa9\xa6[33m\xb9\xa5  \xbb\xf7:[37m%-6d      [33m\xb7\xc0  \xd3\xf9:[37m%-6d      [33m\xcb\xd9  \xb6\xc8:[37m%-5d       [33m\xbe\xad  \xd1\xe9:[37m%-10s[31m\xa9\xa6[m",
 	 d.attack,d.resist,d.speed,inbuf1);
   prints(buf);
   move(4,0);
   //% sprintf(buf,"[1;31m│[33m食  物:[37m%-5d       [33m大补丸:[37m%-5d       [33m零  食:[37m%-5d       [33m药  草:[37m%-5d     [31m│[m",
   sprintf(buf,"[1;31m\xa9\xa6[33m\xca\xb3  \xce\xef:[37m%-5d       [33m\xb4\xf3\xb2\xb9\xcd\xe8:[37m%-5d       [33m\xc1\xe3  \xca\xb3:[37m%-5d       [33m\xd2\xa9  \xb2\xdd:[37m%-5d     [31m\xa9\xa6[m",
 	 d.food,d.bighp,d.cookie,d.medicine);
   prints(buf);	 
   move(5,0);
   //% sprintf(buf,"[1;31m└—————————————————————————————————————┘[m");
   sprintf(buf,"[1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");
   prints(buf);    
   move(6,0);
   if(mode==1)
	   show_badman_pic(n);
   move(19,0);
   //% sprintf(buf,"[1;34m┌—————————————————————————————————————┐[m");
   sprintf(buf,"[1;34m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
   prints(buf);
   move(20,0);
   sprintf(inbuf1,"%d/%d",m.hp,m.maxhp);  
   sprintf(inbuf2,"%d/%d",m.mp,m.maxmp);     
   //% sprintf(buf,"[1;34m│[32m姓  名:[37m%-10s  [32m生  命:[37m%-11s [32m法  力:[37m%-11s                  [34m│[m",
   sprintf(buf,"[1;34m\xa9\xa6[32m\xd0\xd5  \xc3\xfb:[37m%-10s  [32m\xc9\xfa  \xc3\xfc:[37m%-11s [32m\xb7\xa8  \xc1\xa6:[37m%-11s                  [34m\xa9\xa6[m",
 	 p[n].name,inbuf1,inbuf2);
   prints(buf);
   move(21,0);
   //% sprintf(buf,"[1;34m│[32m攻  击:[37m%-6d      [32m防  御:[37m%-6d      [32m速  度:[37m%-6d      [32m金  钱:[37m%-6d    [34m│[m",
   sprintf(buf,"[1;34m\xa9\xa6[32m\xb9\xa5  \xbb\xf7:[37m%-6d      [32m\xb7\xc0  \xd3\xf9:[37m%-6d      [32m\xcb\xd9  \xb6\xc8:[37m%-6d      [32m\xbd\xf0  \xc7\xae:[37m%-6d    [34m\xa9\xa6[m",
 	 m.attack,m.resist,m.speed,m.money);
   prints(buf);
   move(22,0);
   //% sprintf(buf,"[1;34m└—————————————————————————————————————┘[m");
   sprintf(buf,"[1;34m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");
   prints(buf);
   move(b_lines,0);   
   //% sprintf(buf,"[1;41;37m  [37m攻击命令  [47m  [31m[1][30m普通  [31m[2][30m全力  [31m[3][30m魔法  [31m[4][30m防御  [31m[5][30m逃命                     [m");
   sprintf(buf,"[1;41;37m  [37m\xb9\xa5\xbb\xf7\xc3\xfc\xc1\xee  [47m  [31m[1][30m\xc6\xd5\xcd\xa8  [31m[2][30m\xc8\xab\xc1\xa6  [31m[3][30m\xc4\xa7\xb7\xa8  [31m[4][30m\xb7\xc0\xd3\xf9  [31m[5][30m\xcc\xd3\xc3\xfc                     [m");
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
         //% pressanykey("对方没打中..:~~~");
         pressanykey("\xb6\xd4\xb7\xbd\xc3\xbb\xb4\xf2\xd6\xd0..:~~~");
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
        //% sprintf(buf,"对方普通攻击,生命力减低%d",minjure);
        sprintf(buf,"\xb6\xd4\xb7\xbd\xc6\xd5\xcd\xa8\xb9\xa5\xbb\xf7,\xc9\xfa\xc3\xfc\xc1\xa6\xbc\xf5\xb5\xcd%d",minjure);
        pressanykey(buf);
       }             
       break;
       
      case 2:       
       if(rand()%11==10)
       { 
         //% pressanykey("对方没打中..:~~~");
         pressanykey("\xb6\xd4\xb7\xbd\xc3\xbb\xb4\xf2\xd6\xd0..:~~~");
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
          //% sprintf(buf,"对方全力攻击, 生命力减低%d",minjure);
          sprintf(buf,"\xb6\xd4\xb7\xbd\xc8\xab\xc1\xa6\xb9\xa5\xbb\xf7, \xc9\xfa\xc3\xfc\xc1\xa6\xbc\xf5\xb5\xcd%d",minjure);
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
          //% sprintf(buf,"对方普通攻击,生命力减低%d",minjure);
          sprintf(buf,"\xb6\xd4\xb7\xbd\xc6\xd5\xcd\xa8\xb9\xa5\xbb\xf7,\xc9\xfa\xc3\xfc\xc1\xa6\xbc\xf5\xb5\xcd%d",minjure);
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
	      //% sprintf(inbuf1,"热火魔");
	      sprintf(inbuf1,"\xc8\xc8\xbb\xf0\xc4\xa7");
	   else
	      //% sprintf(inbuf1,"寒气鬼");
	      sprintf(inbuf1,"\xba\xae\xc6\xf8\xb9\xed");
         }
         else if(m.mp<1000 && m.mp>=500)
         {
           minjure=300;
           m.mp-=(300+rand()%200);
           if(rand()%2)
	      //% sprintf(inbuf1,"狂水怪");
	      sprintf(inbuf1,"\xbf\xf1\xcb\xae\xb9\xd6");
	   else
	      //% sprintf(inbuf1,"怒土虫");
	      sprintf(inbuf1,"\xc5\xad\xcd\xc1\xb3\xe6");
         }
         else if(m.mp<500 && m.mp>=200)
         {
           minjure=100;
           m.mp-=(100+rand()%100);
           if(rand()%2)
	      //% sprintf(inbuf1,"迷魂鬼差");
	      sprintf(inbuf1,"\xc3\xd4\xbb\xea\xb9\xed\xb2\xee");
	   else
	      //% sprintf(inbuf1,"石怪");
	      sprintf(inbuf1,"\xca\xaf\xb9\xd6");
         }
         else if(m.mp<200 && m.mp>=50)
         {
           minjure=50;
           m.mp-=50;
           if(rand()%2)
	      //% sprintf(inbuf1,"鬼木魂");
	      sprintf(inbuf1,"\xb9\xed\xc4\xbe\xbb\xea");
	   else
	      //% sprintf(inbuf1,"风妖");
	      sprintf(inbuf1,"\xb7\xe7\xd1\xfd");
         }
         minjure=minjure-d.resist/50-d.mresist/10-d.mskill/200-d.mexp/200+rand()%10;
         if(minjure<0)
         	minjure=15;
         d.hp-=minjure;
         d.mresist+=rand()%2+1;
         //% sprintf(buf,"对方招换了%s,你受伤了%d点",inbuf1,minjure);
         sprintf(buf,"\xb6\xd4\xb7\xbd\xd5\xd0\xbb\xbb\xc1\xcb%s,\xc4\xe3\xca\xdc\xc9\xcb\xc1\xcb%d\xb5\xe3",inbuf1,minjure);
         pressanykey(buf);
        }
        else
        {
         m.mp-=20;
         m.hp+=130+rand()%20;
         if(m.hp>m.maxhp)
            m.hp=m.maxhp;
         //% pressanykey("对方使用魔法治疗了自己...");
         pressanykey("\xb6\xd4\xb7\xbd\xca\xb9\xd3\xc3\xc4\xa7\xb7\xa8\xd6\xce\xc1\xc6\xc1\xcb\xd7\xd4\xbc\xba...");
        }
       }
       else
       {
         mresistmore=1;
         //% pressanykey("对方加强防御....");
         pressanykey("\xb6\xd4\xb7\xbd\xbc\xd3\xc7\xbf\xb7\xc0\xd3\xf9....");
       }
       break;
      
      case 4:
       d.money+=(m.money+m.money/2)/3+rand()%10;
       d.hskill+=rand()%4+3;
       d.brave+=rand()%3+2;
       m.death=1;
       //% sprintf(buf,"对方先闪了..但掉了一些钱给你...");
       sprintf(buf,"\xb6\xd4\xb7\xbd\xcf\xc8\xc9\xc1\xc1\xcb..\xb5\xab\xb5\xf4\xc1\xcb\xd2\xbb\xd0\xa9\xc7\xae\xb8\xf8\xc4\xe3...");
       pressanykey(buf);       
       break;      
     }
   }
   
   if(m.death==1)
   {
     clear();
     //% showtitle("电子养小鸡", BoardName);
     showtitle("\xb5\xe7\xd7\xd3\xd1\xf8\xd0\xa1\xbc\xa6", BoardName);
     if(mode==1)
     { 
       move(10,0);
       //% prints("            [1;31m┌——————————————————————┐[m");
       prints("            [1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
       move(11,0);
       //% prints("            [1;31m│  [37m英勇的小鸡 [33m%-10s                     [31m│[m",d.name);
       prints("            [1;31m\xa9\xa6  [37m\xd3\xa2\xd3\xc2\xb5\xc4\xd0\xa1\xbc\xa6 [33m%-10s                     [31m\xa9\xa6[m",d.name);
       move(12,0);
       //% prints("            [1;31m│  [37m打败了邪恶的怪物 [32m%-10s               [31m│[m",p[n].name);
       prints("            [1;31m\xa9\xa6  [37m\xb4\xf2\xb0\xdc\xc1\xcb\xd0\xb0\xb6\xf1\xb5\xc4\xb9\xd6\xce\xef [32m%-10s               [31m\xa9\xa6[m",p[n].name);
     }  
     else
     {
       move(10,0);
       //% prints("            [1;31m┌——————————————————————┐[m");
       prints("            [1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
       move(11,0);
       //% prints("            [1;31m│  [37m武术大会的小鸡 [33m%-10s                 [31m│[m",d.name);
       prints("            [1;31m\xa9\xa6  [37m\xce\xe4\xca\xf5\xb4\xf3\xbb\xe1\xb5\xc4\xd0\xa1\xbc\xa6 [33m%-10s                 [31m\xa9\xa6[m",d.name);
       move(12,0);
       //% prints("            [1;31m│  [37m打败了强劲的对手 [32m%-10s               [31m│[m",p[n].name);
       prints("            [1;31m\xa9\xa6  [37m\xb4\xf2\xb0\xdc\xc1\xcb\xc7\xbf\xbe\xa2\xb5\xc4\xb6\xd4\xca\xd6 [32m%-10s               [31m\xa9\xa6[m",p[n].name);
     }
     move(13,0);
     sprintf(inbuf1,"%d/%d",d.hexp-oldhexp,d.mexp-oldmexp);  
     //% prints("            [1;31m│  [37m评价提升了 %-5s 点  技术增加了 %-2d/%-2d 点  [31m│[m",inbuf1,d.hskill-oldhskill,d.mskill-oldmskill);
     prints("            [1;31m\xa9\xa6  [37m\xc6\xc0\xbc\xdb\xcc\xe1\xc9\xfd\xc1\xcb %-5s \xb5\xe3  \xbc\xbc\xca\xf5\xd4\xf6\xbc\xd3\xc1\xcb %-2d/%-2d \xb5\xe3  [31m\xa9\xa6[m",inbuf1,d.hskill-oldhskill,d.mskill-oldmskill);
     move(14,0);
     //% sprintf(inbuf1,"%d 元",d.money-oldmoney);
     sprintf(inbuf1,"%d \xd4\xaa",d.money-oldmoney);
     //% prints("            [1;31m│  [37m勇敢提升了 %-5d 点  金钱增加了 %-9s [31m│[m",d.brave-oldbrave,inbuf1);
     prints("            [1;31m\xa9\xa6  [37m\xd3\xc2\xb8\xd2\xcc\xe1\xc9\xfd\xc1\xcb %-5d \xb5\xe3  \xbd\xf0\xc7\xae\xd4\xf6\xbc\xd3\xc1\xcb %-9s [31m\xa9\xa6[m",d.brave-oldbrave,inbuf1);
     move(15,0);
     //% prints("            [1;31m└——————————————————————┘[m");     
     prints("            [1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");     
     
     if(m.hp<=0)     
        //% pressanykey("对方死掉罗..所以你赢罗..");   
        pressanykey("\xb6\xd4\xb7\xbd\xcb\xc0\xb5\xf4\xc2\xde..\xcb\xf9\xd2\xd4\xc4\xe3\xd3\xae\xc2\xde..");   
     else if(m.hp>0)
        //% pressanykey("对方落跑罗..所以算你赢罗.....");   
        pressanykey("\xb6\xd4\xb7\xbd\xc2\xe4\xc5\xdc\xc2\xde..\xcb\xf9\xd2\xd4\xcb\xe3\xc4\xe3\xd3\xae\xc2\xde.....");   
     winorlose=1;
   }
   if(d.death==1 && mode==1)
   {   
       clear();
       //% showtitle("电子养小鸡", BoardName);       
       showtitle("\xb5\xe7\xd7\xd3\xd1\xf8\xd0\xa1\xbc\xa6", BoardName);       
       move(10,0);
       //% prints("            [1;31m┌——————————————————————┐[m");
       prints("            [1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
       move(11,0);
       //% prints("            [1;31m│  [37m可怜的小鸡 [33m%-10s                     [31m│[m",d.name);
       prints("            [1;31m\xa9\xa6  [37m\xbf\xc9\xc1\xaf\xb5\xc4\xd0\xa1\xbc\xa6 [33m%-10s                     [31m\xa9\xa6[m",d.name);
       move(12,0);
       //% prints("            [1;31m│  [37m在与 [32m%-10s [37m的战斗中，                [31m│[m",p[n].name);
       prints("            [1;31m\xa9\xa6  [37m\xd4\xda\xd3\xeb [32m%-10s [37m\xb5\xc4\xd5\xbd\xb6\xb7\xd6\xd0\xa3\xac                [31m\xa9\xa6[m",p[n].name);
       move(13,0);
       //% prints("            [1;31m│  [37m不幸地阵亡了，在此特别默哀..........      [31m│[m");
       prints("            [1;31m\xa9\xa6  [37m\xb2\xbb\xd0\xd2\xb5\xd8\xd5\xf3\xcd\xf6\xc1\xcb\xa3\xac\xd4\xda\xb4\xcb\xcc\xd8\xb1\xf0\xc4\xac\xb0\xa7..........      [31m\xa9\xa6[m");
       move(14,0);
       //% prints("            [1;31m└——————————————————————┘[m");        
       prints("            [1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");        
       //% pressanykey("小鸡阵亡了....");      
       pressanykey("\xd0\xa1\xbc\xa6\xd5\xf3\xcd\xf6\xc1\xcb....");      
       //% pipdie("[1;31m战斗中被打死了...[m  ",1);       
       pipdie("[1;31m\xd5\xbd\xb6\xb7\xd6\xd0\xb1\xbb\xb4\xf2\xcb\xc0\xc1\xcb...[m  ",1);       
   }
   else if(d.death==1 && mode==2)
   {
       clear();
       //% showtitle("电子养小鸡", BoardName);       
       showtitle("\xb5\xe7\xd7\xd3\xd1\xf8\xd0\xa1\xbc\xa6", BoardName);       
       move(10,0);
       //% prints("            [1;31m┌——————————————————————┐[m");
       prints("            [1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[m");
       move(11,0);
       //% prints("            [1;31m│  [37m可怜的小鸡 [33m%-10s                     [31m│[m",d.name);
       prints("            [1;31m\xa9\xa6  [37m\xbf\xc9\xc1\xaf\xb5\xc4\xd0\xa1\xbc\xa6 [33m%-10s                     [31m\xa9\xa6[m",d.name);
       move(12,0);
       //% prints("            [1;31m│  [37m在与 [32m%-10s [37m的战斗中，                [31m│[m",p[n].name);
       prints("            [1;31m\xa9\xa6  [37m\xd4\xda\xd3\xeb [32m%-10s [37m\xb5\xc4\xd5\xbd\xb6\xb7\xd6\xd0\xa3\xac                [31m\xa9\xa6[m",p[n].name);
       move(13,0);
       //% prints("            [1;31m│  [37m不幸地打输了，记者现场特别报导.........   [31m│[m");
       prints("            [1;31m\xa9\xa6  [37m\xb2\xbb\xd0\xd2\xb5\xd8\xb4\xf2\xca\xe4\xc1\xcb\xa3\xac\xbc\xc7\xd5\xdf\xcf\xd6\xb3\xa1\xcc\xd8\xb1\xf0\xb1\xa8\xb5\xbc.........   [31m\xa9\xa6[m");
       move(14,0);
       //% prints("            [1;31m└——————————————————————┘[m");        
       prints("            [1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[m");        
       //% pressanykey("小鸡打输了....");     
       pressanykey("\xd0\xa1\xbc\xa6\xb4\xf2\xca\xe4\xc1\xcb....");     
   }
 }while((pipkey!='6')&&(d.death!=1)&&(m.death!=1)&&(mankey!=8)); 
 return winorlose;    
}                 


/*---------------------------------------------------------------------------*/
/* 战斗魔法函式                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*进入使用魔法选单*/
int
pip_magic_menu()   /*战斗中法术的应用*/
{
  char buf[256];
  int magicnum=0;
  int injure;		/*伤害力*/
  int pipkey;
  
  do
  {
   move(b_lines, 0);
   clrtoeol();
   move(b_lines,0);
   sprintf(buf,
   //% "[1;44;37m  魔法选单  [46m  [1]治疗 [2]雷系 [3]冰系 [4]火系 [5]土系 [6]风系 [Q]放弃: [m");
   "[1;44;37m  \xc4\xa7\xb7\xa8\xd1\xa1\xb5\xa5  [46m  [1]\xd6\xce\xc1\xc6 [2]\xc0\xd7\xcf\xb5 [3]\xb1\xf9\xcf\xb5 [4]\xbb\xf0\xcf\xb5 [5]\xcd\xc1\xcf\xb5 [6]\xb7\xe7\xcf\xb5 [Q]\xb7\xc5\xc6\xfa: [m");
   move(b_lines,0);
   prints(buf);   
   pipkey=egetch();
   switch(pipkey)
   {
    case '1':  /*治疗法术*/
     d.magicmode=1;
     injure=pip_magic_doing_menu(treatmagiclist);
     break;

    case '2':  /*雷系法术*/
     d.magicmode=2;
     injure=pip_magic_doing_menu(thundermagiclist);
     break;

    case '3': /*冰系法术*/
     d.magicmode=3;
     injure=pip_magic_doing_menu(icemagiclist);
     break;

    case '4': /*炎系法术*/
     d.magicmode=4;
     injure=pip_magic_doing_menu(firemagiclist);
     show_fight_pic(341);
     //% pressanykey("小鸡使用了炎系法术");
     pressanykey("\xd0\xa1\xbc\xa6\xca\xb9\xd3\xc3\xc1\xcb\xd1\xd7\xcf\xb5\xb7\xa8\xca\xf5");
     break;

    case '5': /*土系法术*/
     d.magicmode=5;
     injure=pip_magic_doing_menu(earthmagiclist);
     break;

    case '6': /*风系法术*/
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
     //% my_write(currutmp->msgs[0].last_pid, "水球丢回去：");
     my_write(currutmp->msgs[0].last_pid, "\xcb\xae\xc7\xf2\xb6\xaa\xbb\xd8\xc8\xa5\xa3\xba");

     memcpy(big_picture, screen0, t_lines * sizeof(screenline));
     free(screen0);
     screen_redraw();
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

/*魔法视窗*/
int
pip_magic_doing_menu(p)   /*魔法画面*/
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
 //% sprintf(buf,"[1;31m┤[37;41m   可用[%s]一览表   [0;1;31m├————————————[m",p[0].name);
 sprintf(buf,"[1;31m\xa9\xc8[37;41m   \xbf\xc9\xd3\xc3[%s]\xd2\xbb\xc0\xc0\xb1\xed   [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[m",p[0].name);
 prints(buf);
 while ((s = p[n].name)&&strcmp(s,"NULL")&& (p[n].needmp<=d.mp))
 {
  move(7+n,4);
  if(p[n].hpmode==1)
  {   
      sprintf(buf,
      //% "[1;37m[[36m%d[37m] [33m%-10s  [37m需要法力: [32m%-6d  [37m恢复体力: [32m%-6d [37m恢复疲劳: [32m%-6d[m   ",n,p[n].name,p[n].needmp,p[n].hp,p[n].tired);
      "[1;37m[[36m%d[37m] [33m%-10s  [37m\xd0\xe8\xd2\xaa\xb7\xa8\xc1\xa6: [32m%-6d  [37m\xbb\xd6\xb8\xb4\xcc\xe5\xc1\xa6: [32m%-6d [37m\xbb\xd6\xb8\xb4\xc6\xa3\xc0\xcd: [32m%-6d[m   ",n,p[n].name,p[n].needmp,p[n].hp,p[n].tired);
      prints(buf);
  }
  else if(p[n].hpmode==2)
  {
      sprintf(buf,
      //% "[1;37m[[36m%d[37m] [33m%-10s  [37m需要法力: [32m%-6d  [37m恢复体力到[35m最大值[37m 恢复疲劳到[35m最小值[m  ",n,p[n].name,p[n].needmp);
      "[1;37m[[36m%d[37m] [33m%-10s  [37m\xd0\xe8\xd2\xaa\xb7\xa8\xc1\xa6: [32m%-6d  [37m\xbb\xd6\xb8\xb4\xcc\xe5\xc1\xa6\xb5\xbd[35m\xd7\xee\xb4\xf3\xd6\xb5[37m \xbb\xd6\xb8\xb4\xc6\xa3\xc0\xcd\xb5\xbd[35m\xd7\xee\xd0\xa1\xd6\xb5[m  ",n,p[n].name,p[n].needmp);
      prints(buf);
  }  
  else if(p[n].hpmode==0)
  {
      sprintf(buf,
      //% "[1;37m[[36m%d[37m] [33m%-10s  [37m需要法力: [32m%-6d [m             ",n,p[n].name,p[n].needmp);
      "[1;37m[[36m%d[37m] [33m%-10s  [37m\xd0\xe8\xd2\xaa\xb7\xa8\xc1\xa6: [32m%-6d [m             ",n,p[n].name,p[n].needmp);
      prints(buf);
  }
  n++;
 }  
 n-=1;
 
 do{
   move(16,4);
   //% sprintf(buf,"你想使用那一个%8s呢?  [Q]放弃:",p[0].name);
   sprintf(buf,"\xc4\xe3\xcf\xeb\xca\xb9\xd3\xc3\xc4\xc7\xd2\xbb\xb8\xf6%8s\xc4\xd8?  [Q]\xb7\xc5\xc6\xfa:",p[0].name);
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
   //% getdata(17,4,"确定使用吗? [Y/n]", ans, 2, 1, 0);
   getdata(17,4,"\xc8\xb7\xb6\xa8\xca\xb9\xd3\xc3\xc2\xf0? [Y/n]", ans, 2, 1, 0);
#else
   //% getdata(17,4,"确定使用吗? [Y/n]", ans, 2, DOECHO, YEA);
   getdata(17,4,"\xc8\xb7\xb6\xa8\xca\xb9\xd3\xc3\xc2\xf0? [Y/n]", ans, 2, DOECHO, YEA);
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
