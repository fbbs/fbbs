/*---------------------------------------------------------------------------*/
/* 商店选单:食物 零食 大补丸 玩具 书本                                       */
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
/* 商店选单:食物 零食 大补丸 玩具 书本                                       */
/* 资料库                                                                    */
/*---------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*  物品参数设定                                                            */
/*--------------------------------------------------------------------------*/
struct goodsofpip
{
   int num;		/*编号*/
   char *name;		/*名字*/
   char *msgbuy;	/*功用*/
   char *msguse;	/*说明*/
   int money;		/*金钱*/
   int change;		/*改变量*/
   int pic1;
   int pic2;
};
typedef struct goodsofpip goodsofpip;

struct goodsofpip pipfoodlist[] = {
//% 0,"物品名",	"说明buy",	"说明feed",			0,	0,	0,0,
0,"\xce\xef\xc6\xb7\xc3\xfb",	"\xcb\xb5\xc3\xf7buy",	"\xcb\xb5\xc3\xf7feed",			0,	0,	0,0,
//% 1,"好吃的食物",	"体力恢复50",	"每吃一次食物会恢复体力50喔!",	50,	50,	1,1,
1,"\xba\xc3\xb3\xd4\xb5\xc4\xca\xb3\xce\xef",	"\xcc\xe5\xc1\xa6\xbb\xd6\xb8\xb450",	"\xc3\xbf\xb3\xd4\xd2\xbb\xb4\xce\xca\xb3\xce\xef\xbb\xe1\xbb\xd6\xb8\xb4\xcc\xe5\xc1\xa650\xe0\xb8!",	50,	50,	1,1,
//% 2,"美味的零食",	"体力恢复100",	"除了恢复体力，小鸡也会更快乐",	120,	100,	2,3,
2,"\xc3\xc0\xce\xb6\xb5\xc4\xc1\xe3\xca\xb3",	"\xcc\xe5\xc1\xa6\xbb\xd6\xb8\xb4100",	"\xb3\xfd\xc1\xcb\xbb\xd6\xb8\xb4\xcc\xe5\xc1\xa6\xa3\xac\xd0\xa1\xbc\xa6\xd2\xb2\xbb\xe1\xb8\xfc\xbf\xec\xc0\xd6",	120,	100,	2,3,
0,NULL,NULL,NULL,0,0,0,0
};

struct goodsofpip pipmedicinelist[] = {
//% 0,"物品名",	"说明buy",	"说明feed",			0,	0,	0,0,
0,"\xce\xef\xc6\xb7\xc3\xfb",	"\xcb\xb5\xc3\xf7buy",	"\xcb\xb5\xc3\xf7feed",			0,	0,	0,0,
//% 1,"好用大补丸",	"体力恢复600",	"恢复大量流失体力的良方",	500,	600,	4,4,
1,"\xba\xc3\xd3\xc3\xb4\xf3\xb2\xb9\xcd\xe8",	"\xcc\xe5\xc1\xa6\xbb\xd6\xb8\xb4600",	"\xbb\xd6\xb8\xb4\xb4\xf3\xc1\xbf\xc1\xf7\xca\xa7\xcc\xe5\xc1\xa6\xb5\xc4\xc1\xbc\xb7\xbd",	500,	600,	4,4,
//% 2,"珍贵的灵芝",	"法力恢复50",	"每吃一次灵芝会恢复法力50喔!",	100,	50,	7,7,
2,"\xd5\xe4\xb9\xf3\xb5\xc4\xc1\xe9\xd6\xa5",	"\xb7\xa8\xc1\xa6\xbb\xd6\xb8\xb450",	"\xc3\xbf\xb3\xd4\xd2\xbb\xb4\xce\xc1\xe9\xd6\xa5\xbb\xe1\xbb\xd6\xb8\xb4\xb7\xa8\xc1\xa650\xe0\xb8!",	100,	50,	7,7,
//% 3,"千年人参王",	"法力恢复500",	"恢复大量流失法力的良方",	800,	500,	7,7,
3,"\xc7\xa7\xc4\xea\xc8\xcb\xb2\xce\xcd\xf5",	"\xb7\xa8\xc1\xa6\xbb\xd6\xb8\xb4500",	"\xbb\xd6\xb8\xb4\xb4\xf3\xc1\xbf\xc1\xf7\xca\xa7\xb7\xa8\xc1\xa6\xb5\xc4\xc1\xbc\xb7\xbd",	800,	500,	7,7,
//% 4,"天山雪莲",	"法力体力最大",	"这个  好贵......",		10000,	0,	7,7,
4,"\xcc\xec\xc9\xbd\xd1\xa9\xc1\xab",	"\xb7\xa8\xc1\xa6\xcc\xe5\xc1\xa6\xd7\xee\xb4\xf3",	"\xd5\xe2\xb8\xf6  \xba\xc3\xb9\xf3......",		10000,	0,	7,7,
0,NULL,NULL,NULL,0,0,0,0
};

struct goodsofpip pipotherlist[] = {
//% 0,"物品名",	"说明buy",	"说明feed",			0,	0,	0,0,
0,"\xce\xef\xc6\xb7\xc3\xfb",	"\xcb\xb5\xc3\xf7buy",	"\xcb\xb5\xc3\xf7feed",			0,	0,	0,0,
//% 1,"乐高玩具组",	"快乐满意度",	"玩具让小鸡更快乐啦...",	50,	0,	5,5,
1,"\xc0\xd6\xb8\xdf\xcd\xe6\xbe\xdf\xd7\xe9",	"\xbf\xec\xc0\xd6\xc2\xfa\xd2\xe2\xb6\xc8",	"\xcd\xe6\xbe\xdf\xc8\xc3\xd0\xa1\xbc\xa6\xb8\xfc\xbf\xec\xc0\xd6\xc0\xb2...",	50,	0,	5,5,
//% 2,"百科全书",	"知识的来源",	"书本让小鸡更聪明更有气质啦...",100,	0,	6,6,
2,"\xb0\xd9\xbf\xc6\xc8\xab\xca\xe9",	"\xd6\xaa\xca\xb6\xb5\xc4\xc0\xb4\xd4\xb4",	"\xca\xe9\xb1\xbe\xc8\xc3\xd0\xa1\xbc\xa6\xb8\xfc\xb4\xcf\xc3\xf7\xb8\xfc\xd3\xd0\xc6\xf8\xd6\xca\xc0\xb2...",100,	0,	6,6,
0,NULL,NULL,NULL,0,0,0,0
};

/*--------------------------------------------------------------------------*/
/*  武器参数设定                                                            */
/*--------------------------------------------------------------------------*/
struct weapon
{
  char *name;           /*名字*/  
  int needmaxhp;	/*需要hp*/
  int needmaxmp;	/*需要mp*/
  int needspeed;	/*需要的speed*/
  int attack;		/*攻击*/
  int resist;		/*防护*/
  int speed;		/*速度*/
  int cost;		/*买价*/
  int sell;		/*卖价*/
  int special;		/*特别*/
  int map;		/*图档*/

};
typedef struct weapon weapon;

/*名字,需hp,需mp,需speed,攻击,防护,速度,买价,卖价,特别,图档*/
struct weapon headlist[] = {
//% "不买装备",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"\xb2\xbb\xc2\xf2\xd7\xb0\xb1\xb8",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
//% "塑胶帽子",  0,  0,  0,  0,  5,  0,   500,   300,0,0,	
"\xcb\xdc\xbd\xba\xc3\xb1\xd7\xd3",  0,  0,  0,  0,  5,  0,   500,   300,0,0,	
//% "牛皮小帽",  0,  0,  0,  0, 10,  0,  3500,  1000,0,0,
"\xc5\xa3\xc6\xa4\xd0\xa1\xc3\xb1",  0,  0,  0,  0, 10,  0,  3500,  1000,0,0,
//% "  安全帽", 60,  0,  0,  0, 20,  0,  5000,  3500,0,0,
"  \xb0\xb2\xc8\xab\xc3\xb1", 60,  0,  0,  0, 20,  0,  5000,  3500,0,0,
//% "钢铁头盔",150, 50,  0,  0, 30,  0, 10000,  6000,0,0,
"\xb8\xd6\xcc\xfa\xcd\xb7\xbf\xf8",150, 50,  0,  0, 30,  0, 10000,  6000,0,0,
//% "魔法发箍",100,150,  0,  0, 25,  0, 50000, 10000,0,0, 
"\xc4\xa7\xb7\xa8\xb7\xa2\xb9\xbf",100,150,  0,  0, 25,  0, 50000, 10000,0,0, 
//% "黄金圣盔",300,300,300,  0,100,  0,300000,100000,0,0,
"\xbb\xc6\xbd\xf0\xca\xa5\xbf\xf8",300,300,300,  0,100,  0,300000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,   0,   0,0,0
};

/*名字,需hp,需mp,需speed,攻击,防护,速度,买价,卖价,特别,图档*/
struct weapon rhandlist[] = {
//% "不买装备",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"\xb2\xbb\xc2\xf2\xd7\xb0\xb1\xb8",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
//% "大木棒",    0,  0,  0,  5,  0,  0,  1000,   700,0,0,	
"\xb4\xf3\xc4\xbe\xb0\xf4",    0,  0,  0,  5,  0,  0,  1000,   700,0,0,	
//% "金属扳手",  0,  0,  0, 10,  0,  0,  2500,  1000,0,0,
"\xbd\xf0\xca\xf4\xb0\xe2\xca\xd6",  0,  0,  0, 10,  0,  0,  2500,  1000,0,0,
//% "青铜剑",   50,  0,  0, 20,  0,  0,  6000,  4000,0,0,
"\xc7\xe0\xcd\xad\xbd\xa3",   50,  0,  0, 20,  0,  0,  6000,  4000,0,0,
//% "晴雷剑",   80,  0,  0, 30,  0,  0, 10000,  8000,0,0,
"\xc7\xe7\xc0\xd7\xbd\xa3",   80,  0,  0, 30,  0,  0, 10000,  8000,0,0,
//% "蝉翼刀",  100, 20,  0, 40,  0,  0, 15000, 10000,0,0, 
"\xb2\xf5\xd2\xed\xb5\xb6",  100, 20,  0, 40,  0,  0, 15000, 10000,0,0, 
//% "忘情剑",  100, 40,  0, 35, 20,  0, 15000, 10000,0,0,
"\xcd\xfc\xc7\xe9\xbd\xa3",  100, 40,  0, 35, 20,  0, 15000, 10000,0,0,
//% "狮头宝刀",150,  0,  0, 60,  0,  0, 35000, 20000,0,0,
"\xca\xa8\xcd\xb7\xb1\xa6\xb5\xb6",150,  0,  0, 60,  0,  0, 35000, 20000,0,0,
//% "屠龙刀",  200,  0,  0,100,  0,  0, 50000, 25000,0,0,
"\xcd\xc0\xc1\xfa\xb5\xb6",  200,  0,  0,100,  0,  0, 50000, 25000,0,0,
//% "黄金圣杖",300,300,300,100, 20,  0,150000,100000,0,0,
"\xbb\xc6\xbd\xf0\xca\xa5\xd5\xc8",300,300,300,100, 20,  0,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,    0,   0,0,0
};

/*名字,需hp,需mp,需speed,攻击,防护,速度,买价,卖价,特别,图档*/
struct weapon lhandlist[] = {
//% "不买装备",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"\xb2\xbb\xc2\xf2\xd7\xb0\xb1\xb8",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
//% "大木棒",    0,  0,  0,  5,  0,  0,  1000,   700,0,0,	
"\xb4\xf3\xc4\xbe\xb0\xf4",    0,  0,  0,  5,  0,  0,  1000,   700,0,0,	
//% "金属扳手",  0,  0,  0, 10,  0,  0,  1500,  1000,0,0,
"\xbd\xf0\xca\xf4\xb0\xe2\xca\xd6",  0,  0,  0, 10,  0,  0,  1500,  1000,0,0,
//% "木盾",	     0,  0,  0,  0, 10,  0,  2000,  1500,0,0,
"\xc4\xbe\xb6\xdc",	     0,  0,  0,  0, 10,  0,  2000,  1500,0,0,
//% "不锈钢盾", 60,  0,  0,  0, 25,  0,  5000,  3000,0,0,
"\xb2\xbb\xd0\xe2\xb8\xd6\xb6\xdc", 60,  0,  0,  0, 25,  0,  5000,  3000,0,0,
//% "白金之盾", 80,  0,  0, 10, 40,  0, 15000, 10000,0,0,
"\xb0\xd7\xbd\xf0\xd6\xae\xb6\xdc", 80,  0,  0, 10, 40,  0, 15000, 10000,0,0,
//% "魔法盾",   80,100,  0, 20, 60,  0, 80000, 50000,0,0,
"\xc4\xa7\xb7\xa8\xb6\xdc",   80,100,  0, 20, 60,  0, 80000, 50000,0,0,
//% "黄金圣盾",300,300,300, 30,100,  0,150000,100000,0,0,
"\xbb\xc6\xbd\xf0\xca\xa5\xb6\xdc",300,300,300, 30,100,  0,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,    0,   0,0,0
};

/*名字,需hp,需mp,需speed,攻击,防护,速度,买价,卖价,特别,图档*/
struct weapon bodylist[] = {
//% "不买装备",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"\xb2\xbb\xc2\xf2\xd7\xb0\xb1\xb8",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
//% "塑胶胄甲", 40,  0,  0,  0,  5,  0,  1000,   700,0,0,	
"\xcb\xdc\xbd\xba\xeb\xd0\xbc\xd7", 40,  0,  0,  0,  5,  0,  1000,   700,0,0,	
//% "特级皮甲", 50,  0,  0,  0, 10,  0,  2500,  1000,0,0,
"\xcc\xd8\xbc\xb6\xc6\xa4\xbc\xd7", 50,  0,  0,  0, 10,  0,  2500,  1000,0,0,
//% "钢铁盔甲", 80,  0,  0,  0, 25,  0,  5000,  3500,0,0,
"\xb8\xd6\xcc\xfa\xbf\xf8\xbc\xd7", 80,  0,  0,  0, 25,  0,  5000,  3500,0,0,
//% "魔法披风", 80, 40,  0,  0, 20, 20, 15500, 10000,0,0,
"\xc4\xa7\xb7\xa8\xc5\xfb\xb7\xe7", 80, 40,  0,  0, 20, 20, 15500, 10000,0,0,
//% "白金盔甲",100, 30,  0,  0, 40, 20, 30000, 20000,0,0, 
"\xb0\xd7\xbd\xf0\xbf\xf8\xbc\xd7",100, 30,  0,  0, 40, 20, 30000, 20000,0,0, 
//% "黄金圣衣",300,300,300, 30,100,  0,150000,100000,0,0,
"\xbb\xc6\xbd\xf0\xca\xa5\xd2\xc2",300,300,300, 30,100,  0,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,     0,   0,0,0
};

/*名字,需hp,需mp,需speed,攻击,防护,速度,买价,卖价,特别,图档*/
struct weapon footlist[] = {
//% "不买装备",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
"\xb2\xbb\xc2\xf2\xd7\xb0\xb1\xb8",  0,  0,  0,  0,  0,  0,     0,     0,0,0,
//% "塑胶拖鞋",  0,  0,  0,  0,  0, 10,   800,   500,0,0,
"\xcb\xdc\xbd\xba\xcd\xcf\xd0\xac",  0,  0,  0,  0,  0, 10,   800,   500,0,0,
//% "东洋木屐",  0,  0,  0, 15,  0, 10,  1000,   700,0,0, 	
"\xb6\xab\xd1\xf3\xc4\xbe\xe5\xec",  0,  0,  0, 15,  0, 10,  1000,   700,0,0, 	
//% "特级雨鞋",  0,  0,  0,  0, 10, 10,  1500,  1000,0,0,
"\xcc\xd8\xbc\xb6\xd3\xea\xd0\xac",  0,  0,  0,  0, 10, 10,  1500,  1000,0,0,
//% "NIKE运动鞋",70, 0,  0,  0, 10, 40,  8000,  5000,0,0,
"NIKE\xd4\xcb\xb6\xaf\xd0\xac",70, 0,  0,  0, 10, 40,  8000,  5000,0,0,
//% "鳄鱼皮靴", 80, 20,  0, 10, 25, 20, 12000,  8000,0,0,
"\xf6\xf9\xd3\xe3\xc6\xa4\xd1\xa5", 80, 20,  0, 10, 25, 20, 12000,  8000,0,0,
//% "飞天魔靴",100,100,  0, 30, 50, 60, 25000, 10000,0,0,
"\xb7\xc9\xcc\xec\xc4\xa7\xd1\xa5",100,100,  0, 30, 50, 60, 25000, 10000,0,0,
//% "黄金圣靴",300,300,300, 50,100,100,150000,100000,0,0,
"\xbb\xc6\xbd\xf0\xca\xa5\xd1\xa5",300,300,300, 50,100,100,150000,100000,0,0,
NULL,        0,  0,  0,  0,  0,  0,    0,   0,0,0
};

/*---------------------------------------------------------------------------*/
/* 商店选单:食物 零食 大补丸 玩具 书本                                       */
/* 函式库                                                                    */
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

int pip_store_weapon_head()	/*头部武器*/
{
     d.weaponhead=pip_weapon_doing_menu(d.weaponhead,0,headlist);
     return 0; 
}
int pip_store_weapon_rhand()	/*右手武器*/
{
     d.weaponrhand=pip_weapon_doing_menu(d.weaponrhand,1,rhandlist);
     return 0;
}
int pip_store_weapon_lhand()    /*左手武器*/
{
     d.weaponlhand=pip_weapon_doing_menu(d.weaponlhand,2,lhandlist);
     return 0;
}
int pip_store_weapon_body()	/*身体武器*/
{
     d.weaponbody=pip_weapon_doing_menu(d.weaponbody,3,bodylist);
     return 0;
}
int pip_store_weapon_foot()     /*足部武器*/
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
    //% char *shopname[4]={"店名","便利商店","星空药铺","夜里书局"};
    char *shopname[4]={"\xb5\xea\xc3\xfb","\xb1\xe3\xc0\xfb\xc9\xcc\xb5\xea","\xd0\xc7\xbf\xd5\xd2\xa9\xc6\xcc","\xd2\xb9\xc0\xef\xca\xe9\xbe\xd6"};
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
	    //% sprintf(inbuf,"[1;31m  —[41;37m 编号 [0;1;31m—[41;37m 商      品 [0;1;31m——[41;37m 效            能 [0;1;31m——[41;37m 价     格 [0;1;31m—[37;41m 拥有数量 [0;1;31m—[0m  ");
	    sprintf(inbuf,"[1;31m  \xa1\xaa[41;37m \xb1\xe0\xba\xc5 [0;1;31m\xa1\xaa[41;37m \xc9\xcc      \xc6\xb7 [0;1;31m\xa1\xaa\xa1\xaa[41;37m \xd0\xa7            \xc4\xdc [0;1;31m\xa1\xaa\xa1\xaa[41;37m \xbc\xdb     \xb8\xf1 [0;1;31m\xa1\xaa[37;41m \xd3\xb5\xd3\xd0\xca\xfd\xc1\xbf [0;1;31m\xa1\xaa[0m  ");
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
	    //% sprintf(inbuf,"[1;44;37m  %8s选单  [46m  [B]买入物品  [S]卖出物品  [Q]跳出：                         [m",shopname[mode]);
	    sprintf(inbuf,"[1;44;37m  %8s\xd1\xa1\xb5\xa5  [46m  [B]\xc2\xf2\xc8\xeb\xce\xef\xc6\xb7  [S]\xc2\xf4\xb3\xf6\xce\xef\xc6\xb7  [Q]\xcc\xf8\xb3\xf6\xa3\xba                         [m",shopname[mode]);
	    prints(inbuf);
	    pipkey=egetch(); 
	    switch(pipkey)  
	    {
		case 'B':
		case 'b':      
			move(b_lines-1,1);
			//% sprintf(inbuf,"想要买入啥呢? [0]放弃买入 [1～%d]物品商号",oldnum[0]);
			sprintf(inbuf,"\xcf\xeb\xd2\xaa\xc2\xf2\xc8\xeb\xc9\xb6\xc4\xd8? [0]\xb7\xc5\xc6\xfa\xc2\xf2\xc8\xeb [1\xa1\xab%d]\xce\xef\xc6\xb7\xc9\xcc\xba\xc5",oldnum[0]);
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
					//% sprintf(inbuf,"你要买入物品 [%s] 多少个呢?(上限 %d)",p[choice].name,d.money/p[choice].money);
					sprintf(inbuf,"\xc4\xe3\xd2\xaa\xc2\xf2\xc8\xeb\xce\xef\xc6\xb7 [%s] \xb6\xe0\xc9\xd9\xb8\xf6\xc4\xd8?(\xc9\xcf\xcf\xde %d)",p[choice].name,d.money/p[choice].money);
#ifdef MAPLE
					getdata(b_lines-1,1,inbuf,genbuf,6, 1, 0);
#else
                                        getdata(b_lines-1,1,inbuf,genbuf,6, DOECHO, YEA);
#endif  // END MAPLE
					smoney=atoi(genbuf);
				}
				if(smoney<0)
				{
					//% pressanykey("放弃买入...");
					pressanykey("\xb7\xc5\xc6\xfa\xc2\xf2\xc8\xeb...");
				}
				else if(d.money<smoney*p[choice].money)
				{
					//% pressanykey("你的钱没有那麽多喔..");
					pressanykey("\xc4\xe3\xb5\xc4\xc7\xae\xc3\xbb\xd3\xd0\xc4\xc7\xf7\xe1\xb6\xe0\xe0\xb8..");
				}
				else
				{
					//% sprintf(inbuf,"确定买入物品 [%s] 数量 %d 个吗?(店家卖价 %d) [y/N]:",p[choice].name,smoney,smoney*p[choice].money);
					sprintf(inbuf,"\xc8\xb7\xb6\xa8\xc2\xf2\xc8\xeb\xce\xef\xc6\xb7 [%s] \xca\xfd\xc1\xbf %d \xb8\xf6\xc2\xf0?(\xb5\xea\xbc\xd2\xc2\xf4\xbc\xdb %d) [y/N]:",p[choice].name,smoney,smoney*p[choice].money);
#ifdef MAPLE
					getdata(b_lines-1,1,inbuf,genbuf, 2, 1, 0); 
#else
                                        getdata(b_lines-1,1,inbuf,genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
					if(genbuf[0]=='y' || genbuf[0]=='Y')
					{
						oldnum[choice]+=smoney;
						d.money-=smoney*p[choice].money;
						//% sprintf(inbuf,"老板给了你%d个%s",smoney,p[choice].name);
						sprintf(inbuf,"\xc0\xcf\xb0\xe5\xb8\xf8\xc1\xcb\xc4\xe3%d\xb8\xf6%s",smoney,p[choice].name);
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
						//% pressanykey("放弃买入...");
						pressanykey("\xb7\xc5\xc6\xfa\xc2\xf2\xc8\xeb...");
					}
				}
			}
			else
			{
				//% sprintf(inbuf,"放弃买入.....");
				sprintf(inbuf,"\xb7\xc5\xc6\xfa\xc2\xf2\xc8\xeb.....");
				pressanykey(inbuf);            
			}
			break;     
     
 		case 'S':
 		case 's':
 			if(mode==3)
 			{
 				//% pressanykey("这些东西不能卖喔....");
 				pressanykey("\xd5\xe2\xd0\xa9\xb6\xab\xce\xf7\xb2\xbb\xc4\xdc\xc2\xf4\xe0\xb8....");
 				break;
 			}
			move(b_lines-1,1);
			//% sprintf(inbuf,"想要卖出啥呢? [0]放弃卖出 [1～%d]物品商号",oldnum[0]);
			sprintf(inbuf,"\xcf\xeb\xd2\xaa\xc2\xf4\xb3\xf6\xc9\xb6\xc4\xd8? [0]\xb7\xc5\xc6\xfa\xc2\xf4\xb3\xf6 [1\xa1\xab%d]\xce\xef\xc6\xb7\xc9\xcc\xba\xc5",oldnum[0]);
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
				//% sprintf(inbuf,"你要卖出物品 [%s] 多少个呢?(上限 %d)",p[choice].name,oldnum[choice]);
				sprintf(inbuf,"\xc4\xe3\xd2\xaa\xc2\xf4\xb3\xf6\xce\xef\xc6\xb7 [%s] \xb6\xe0\xc9\xd9\xb8\xf6\xc4\xd8?(\xc9\xcf\xcf\xde %d)",p[choice].name,oldnum[choice]);
#ifdef MAPLE
				getdata(b_lines-1,1,inbuf,genbuf,6, 1, 0); 
#else
                                getdata(b_lines-1,1,inbuf,genbuf,6, DOECHO, YEA);
#endif  // END MAPLE
				smoney=atoi(genbuf);
				if(smoney<0)
				{
					//% pressanykey("放弃卖出...");
					pressanykey("\xb7\xc5\xc6\xfa\xc2\xf4\xb3\xf6...");
				}
				else if(smoney>oldnum[choice])
				{
					//% sprintf(inbuf,"你的 [%s] 没有那麽多个喔",p[choice].name);
					sprintf(inbuf,"\xc4\xe3\xb5\xc4 [%s] \xc3\xbb\xd3\xd0\xc4\xc7\xf7\xe1\xb6\xe0\xb8\xf6\xe0\xb8",p[choice].name);
					pressanykey(inbuf);
				}
				else
				{
					//% sprintf(inbuf,"确定卖出物品 [%s] 数量 %d 个吗?(店家买价 %d) [y/N]:",p[choice].name,smoney,smoney*p[choice].money*8/10);
					sprintf(inbuf,"\xc8\xb7\xb6\xa8\xc2\xf4\xb3\xf6\xce\xef\xc6\xb7 [%s] \xca\xfd\xc1\xbf %d \xb8\xf6\xc2\xf0?(\xb5\xea\xbc\xd2\xc2\xf2\xbc\xdb %d) [y/N]:",p[choice].name,smoney,smoney*p[choice].money*8/10);
#ifdef MAPLE
					getdata(b_lines-1,1,inbuf,genbuf, 2, 1, 0);
#else
                                        getdata(b_lines-1,1,inbuf,genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
					if(genbuf[0]=='y' || genbuf[0]=='Y')
					{
						oldnum[choice]-=smoney;
						d.money+=smoney*p[choice].money*8/10;
						//% sprintf(inbuf,"老板拿走了你的%d个%s",smoney,p[choice].name);
						sprintf(inbuf,"\xc0\xcf\xb0\xe5\xc4\xc3\xd7\xdf\xc1\xcb\xc4\xe3\xb5\xc4%d\xb8\xf6%s",smoney,p[choice].name);
						pressanykey(inbuf);
					}
					else
					{
						//% pressanykey("放弃卖出...");
						pressanykey("\xb7\xc5\xc6\xfa\xc2\xf4\xb3\xf6...");
					}
				}
			}
			else
			{
				//% sprintf(inbuf,"放弃卖出.....");
				sprintf(inbuf,"\xb7\xc5\xc6\xfa\xc2\xf4\xb3\xf6.....");
				pressanykey(inbuf);            
			}
			break;
		case 'Q':
		case 'q':
			//% sprintf(inbuf,"金钱交易共 %d 元,离开 %s ",d.money-oldmoney,shopname[mode]);
			sprintf(inbuf,"\xbd\xf0\xc7\xae\xbd\xbb\xd2\xd7\xb9\xb2 %d \xd4\xaa,\xc0\xeb\xbf\xaa %s ",d.money-oldmoney,shopname[mode]);
			pressanykey(inbuf);
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

int
pip_weapon_doing_menu(variance,type,p)               /* 武器购买画面 */
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
  //% char menutitle[5][11]={"头部装备区","右手装备区","左手装备区","身体装备区","足部装备区"};  
  char menutitle[5][11]={"\xcd\xb7\xb2\xbf\xd7\xb0\xb1\xb8\xc7\xf8","\xd3\xd2\xca\xd6\xd7\xb0\xb1\xb8\xc7\xf8","\xd7\xf3\xca\xd6\xd7\xb0\xb1\xb8\xc7\xf8","\xc9\xed\xcc\xe5\xd7\xb0\xb1\xb8\xc7\xf8","\xd7\xe3\xb2\xbf\xd7\xb0\xb1\xb8\xc7\xf8"};  
  int pipkey;
  char choicekey[5];
  int choice;
  
  do{
   clear();
   showtitle(menutitle[type], BoardName);
   show_weapon_pic(0);
/*   move(10,2); 
   //% sprintf(buf,"[1;37m现今能力:体力Max:[36m%-5d[37m  法力Max:[36m%-5d[37m  攻击:[36m%-5d[37m  防御:[36m%-5d[37m  速度:[36m%-5d [m",
   sprintf(buf,"[1;37m\xcf\xd6\xbd\xf1\xc4\xdc\xc1\xa6:\xcc\xe5\xc1\xa6Max:[36m%-5d[37m  \xb7\xa8\xc1\xa6Max:[36m%-5d[37m  \xb9\xa5\xbb\xf7:[36m%-5d[37m  \xb7\xc0\xd3\xf9:[36m%-5d[37m  \xcb\xd9\xb6\xc8:[36m%-5d [m",
           d.maxhp,d.maxmp,d.attack,d.resist,d.speed);
   prints(buf);*/
   move(11,2);
   //% sprintf(buf,"[1;37;41m [NO]  [器具名]  [体力]  [法力]  [速度]  [攻击]  [防御]  [速度]  [售  价] [m");
   sprintf(buf,"[1;37;41m [NO]  [\xc6\xf7\xbe\xdf\xc3\xfb]  [\xcc\xe5\xc1\xa6]  [\xb7\xa8\xc1\xa6]  [\xcb\xd9\xb6\xc8]  [\xb9\xa5\xbb\xf7]  [\xb7\xc0\xd3\xf9]  [\xcb\xd9\xb6\xc8]  [\xca\xdb  \xbc\xdb] [m");
   prints(buf);
   move(12,2);
   //% sprintf(buf," [1;31m——[37m白色 可以购买[31m——[32m绿色 拥有装备[31m——[33m黄色 钱钱不够[31m——[35m紫色 能力不足[31m——[m");
   sprintf(buf," [1;31m\xa1\xaa\xa1\xaa[37m\xb0\xd7\xc9\xab \xbf\xc9\xd2\xd4\xb9\xba\xc2\xf2[31m\xa1\xaa\xa1\xaa[32m\xc2\xcc\xc9\xab \xd3\xb5\xd3\xd0\xd7\xb0\xb1\xb8[31m\xa1\xaa\xa1\xaa[33m\xbb\xc6\xc9\xab \xc7\xae\xc7\xae\xb2\xbb\xb9\xbb[31m\xa1\xaa\xa1\xaa[35m\xd7\xcf\xc9\xab \xc4\xdc\xc1\xa6\xb2\xbb\xd7\xe3[31m\xa1\xaa\xa1\xaa[m");
   prints(buf); 

   n=0;
   while (s = p[n].name)
   {   
     move(13+n,2);
     if(variance!=0 && variance==(n))/*本身有的*/
     {
      sprintf(buf, 
      "[1;32m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",     
      n,p[n].name,p[n].needmaxhp,p[n].needmaxmp,p[n].needspeed,
      p[n].attack,p[n].resist,p[n].speed,p[n].cost);        
     }
     else if(d.maxhp < p[n].needmaxhp || d.maxmp < p[n].needmaxmp || d.speed < p[n].needspeed )/*能力不足*/
     {
      sprintf(buf, 
      "[1;35m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
      n,p[n].name,p[n].needmaxhp,p[n].needmaxmp,p[n].needspeed,
      p[n].attack,p[n].resist,p[n].speed,p[n].cost);
     }

     else if(d.money < p[n].cost)  /*钱不够的*/
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
   //% sprintf(buf,"[1;44;37m  武器购买选单  [46m  [B]购买武器  [S]卖掉装备  [W]个人资料  [Q]跳出：            [m");
   sprintf(buf,"[1;44;37m  \xce\xe4\xc6\xf7\xb9\xba\xc2\xf2\xd1\xa1\xb5\xa5  [46m  [B]\xb9\xba\xc2\xf2\xce\xe4\xc6\xf7  [S]\xc2\xf4\xb5\xf4\xd7\xb0\xb1\xb8  [W]\xb8\xf6\xc8\xcb\xd7\xca\xc1\xcf  [Q]\xcc\xf8\xb3\xf6\xa3\xba            [m");
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
     //% sprintf(shortbuf,"想要购买啥呢? 你的钱钱[%d]元:[数字]",d.money);
     sprintf(shortbuf,"\xcf\xeb\xd2\xaa\xb9\xba\xc2\xf2\xc9\xb6\xc4\xd8? \xc4\xe3\xb5\xc4\xc7\xae\xc7\xae[%d]\xd4\xaa:[\xca\xfd\xd7\xd6]",d.money);
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
       if(choice==0)     /*解除*/
       { 
          //% sprintf(shortbuf,"放弃购买...");
          sprintf(shortbuf,"\xb7\xc5\xc6\xfa\xb9\xba\xc2\xf2...");
          pressanykey(shortbuf);
       }
      
       else if(variance==choice)  /*早已经有啦*/
       {
          //% sprintf(shortbuf,"你早已经有 %s 罗",p[variance].name);
          sprintf(shortbuf,"\xc4\xe3\xd4\xe7\xd2\xd1\xbe\xad\xd3\xd0 %s \xc2\xde",p[variance].name);
          pressanykey(shortbuf);      
       }
      
       else if(p[choice].cost >= (d.money+p[variance].sell))  /*钱不够*/
       {
          //% sprintf(shortbuf,"这个要 %d 元，你的钱不够啦!",p[choice].cost);
          sprintf(shortbuf,"\xd5\xe2\xb8\xf6\xd2\xaa %d \xd4\xaa\xa3\xac\xc4\xe3\xb5\xc4\xc7\xae\xb2\xbb\xb9\xbb\xc0\xb2!",p[choice].cost);
          pressanykey(shortbuf);      
       }      
     
       else if(d.maxhp < p[choice].needmaxhp || d.maxmp < p[choice].needmaxmp 
               || d.speed < p[choice].needspeed ) /*能力不足*/
       {
          //% sprintf(shortbuf,"需要HP %d MP %d SPEED %d 喔",
          sprintf(shortbuf,"\xd0\xe8\xd2\xaaHP %d MP %d SPEED %d \xe0\xb8",
                p[choice].needmaxhp,p[choice].needmaxmp,p[choice].needspeed);
          pressanykey(shortbuf);            
       }
       else  /*顺利购买*/
       {
          //% sprintf(shortbuf,"你确定要购买 %s 吗?($%d) [y/N]",p[choice].name,p[choice].cost);
          sprintf(shortbuf,"\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xb9\xba\xc2\xf2 %s \xc2\xf0?($%d) [y/N]",p[choice].name,p[choice].cost);
#ifdef MAPLE
          getdata(b_lines-1,1,shortbuf, ans, 2, 1, 0); 
#else
          getdata(b_lines-1,1,shortbuf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
          if(ans[0]=='y' || ans[0]=='Y')
          {
              //% sprintf(shortbuf,"小鸡已经装配上 %s 了",p[choice].name);
              sprintf(shortbuf,"\xd0\xa1\xbc\xa6\xd2\xd1\xbe\xad\xd7\xb0\xc5\xe4\xc9\xcf %s \xc1\xcb",p[choice].name);
              pressanykey(shortbuf);
              d.attack+=(p[choice].attack-p[variance].attack);
              d.resist+=(p[choice].resist-p[variance].resist);
              d.speed+=(p[choice].speed-p[variance].speed);
              d.money-=(p[choice].cost-p[variance].sell);
              variance=choice;
          }
          else
          {
              //% sprintf(shortbuf,"放弃购买.....");
              sprintf(shortbuf,"\xb7\xc5\xc6\xfa\xb9\xba\xc2\xf2.....");
              pressanykey(shortbuf);            
          }
       }
     }       
     break;     
     
   case 'S':
   case 's':
     if(variance!=0)
     { 
        //% sprintf(shortbuf,"你确定要卖掉%s吗? 卖价:%d [y/N]",p[variance].name,p[variance].sell);
        sprintf(shortbuf,"\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xc2\xf4\xb5\xf4%s\xc2\xf0? \xc2\xf4\xbc\xdb:%d [y/N]",p[variance].name,p[variance].sell);
#ifdef MAPLE
        getdata(b_lines-1,1,shortbuf, ans, 2, 1, 0); 
#else
        getdata(b_lines-1,1,shortbuf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
        if(ans[0]=='y' || ans[0]=='Y')
        {
           //% sprintf(shortbuf,"装备 %s 卖了 %d",p[variance].name,p[variance].sell);
           sprintf(shortbuf,"\xd7\xb0\xb1\xb8 %s \xc2\xf4\xc1\xcb %d",p[variance].name,p[variance].sell);
           d.attack-=p[variance].attack;
           d.resist-=p[variance].resist;
           d.speed-=p[variance].speed;
           d.money+=p[variance].sell;
           pressanykey(shortbuf);
           variance=0;
        }
        else
        {
           //% sprintf(shortbuf,"ccc..我回心转意了...");
           sprintf(shortbuf,"ccc..\xce\xd2\xbb\xd8\xd0\xc4\xd7\xaa\xd2\xe2\xc1\xcb...");
           pressanykey(shortbuf);         
        }
     }
     else if(variance==0)
     {
        //% sprintf(shortbuf,"你本来就没有装备了...");
        sprintf(shortbuf,"\xc4\xe3\xb1\xbe\xc0\xb4\xbe\xcd\xc3\xbb\xd3\xd0\xd7\xb0\xb1\xb8\xc1\xcb...");
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
       //% my_write(currutmp->msgs[0].last_pid, "水球丢回去：");
       my_write(currutmp->msgs[0].last_pid, "\xcb\xae\xc7\xf2\xb6\xaa\xbb\xd8\xc8\xa5\xa3\xba");
     }
     break;
#endif  // END MAPLE
   }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
    
  return variance;
}
