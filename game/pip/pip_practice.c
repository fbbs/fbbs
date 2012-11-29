/*---------------------------------------------------------------------------*/
/* 修行选单:念书 练武 修行                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

/*---------------------------------------------------------------------------*/
/* 修行选单:念书 练武 修行                                                   */
/* 资料库                                                                    */
/*---------------------------------------------------------------------------*/
//% char *classrank[6]={"没有","初级","中级","高级","进阶","专业"};
char *classrank[6]={"\xc3\xbb\xd3\xd0","\xb3\xf5\xbc\xb6","\xd6\xd0\xbc\xb6","\xb8\xdf\xbc\xb6","\xbd\xf8\xbd\xd7","\xd7\xa8\xd2\xb5"};
int classmoney[11][2]={{ 0,  0},
		       {60,110},{70,120},{70,120},{80,130},{70,120},
		       {60,110},{90,140},{70,120},{70,120},{80,130}};		     
int classvariable[11][4]={
{0,0,0,0},
{5,5,4,4},{5,7,6,4},{5,7,6,4},{5,6,5,4},{7,5,4,6},
{7,5,4,6},{6,5,4,6},{6,6,5,4},{5,5,4,7},{7,5,4,7}};


char *classword[11][5]=
{
//% {"课名","成功一","成功二","失败一","失败二"},
{"\xbf\xce\xc3\xfb","\xb3\xc9\xb9\xa6\xd2\xbb","\xb3\xc9\xb9\xa6\xb6\xfe","\xca\xa7\xb0\xdc\xd2\xbb","\xca\xa7\xb0\xdc\xb6\xfe"},
 
//% {"自然科学","正在用功读书中..","我是聪明鸡 cccc...",
{"\xd7\xd4\xc8\xbb\xbf\xc6\xd1\xa7","\xd5\xfd\xd4\xda\xd3\xc3\xb9\xa6\xb6\xc1\xca\xe9\xd6\xd0..","\xce\xd2\xca\xc7\xb4\xcf\xc3\xf7\xbc\xa6 cccc...",
            //% "这题怎麽看不懂咧..怪了","念不完了 :~~~~~~"},
            "\xd5\xe2\xcc\xe2\xd4\xf5\xf7\xe1\xbf\xb4\xb2\xbb\xb6\xae\xdf\xd6..\xb9\xd6\xc1\xcb","\xc4\xee\xb2\xbb\xcd\xea\xc1\xcb :~~~~~~"},
            
//% {"唐诗宋词","床前明月光...疑是地上霜...","红豆生南国..春来发几枝..",
{"\xcc\xc6\xca\xab\xcb\xce\xb4\xca","\xb4\xb2\xc7\xb0\xc3\xf7\xd4\xc2\xb9\xe2...\xd2\xc9\xca\xc7\xb5\xd8\xc9\xcf\xcb\xaa...","\xba\xec\xb6\xb9\xc9\xfa\xc4\xcf\xb9\xfa..\xb4\xba\xc0\xb4\xb7\xa2\xbc\xb8\xd6\xa6..",
            //% "ㄟ..上课不要流口水","你还混喔..罚你背会唐诗叁百首"},
            "\xa8\xdf..\xc9\xcf\xbf\xce\xb2\xbb\xd2\xaa\xc1\xf7\xbf\xda\xcb\xae","\xc4\xe3\xbb\xb9\xbb\xec\xe0\xb8..\xb7\xa3\xc4\xe3\xb1\xb3\xbb\xe1\xcc\xc6\xca\xab\xc8\xfe\xb0\xd9\xca\xd7"},

//% {"神学教育","哈雷路亚  哈雷路亚","让我们迎接天堂之门",
{"\xc9\xf1\xd1\xa7\xbd\xcc\xd3\xfd","\xb9\xfe\xc0\xd7\xc2\xb7\xd1\xc7  \xb9\xfe\xc0\xd7\xc2\xb7\xd1\xc7","\xc8\xc3\xce\xd2\xc3\xc7\xd3\xad\xbd\xd3\xcc\xec\xcc\xc3\xd6\xae\xc3\xc5",
	    //% "ㄟ..你在干嘛ㄚ? 还不好好念","神学很严肃的..请好好学..:("},
	    "\xa8\xdf..\xc4\xe3\xd4\xda\xb8\xc9\xc2\xef\xa8\xda? \xbb\xb9\xb2\xbb\xba\xc3\xba\xc3\xc4\xee","\xc9\xf1\xd1\xa7\xba\xdc\xd1\xcf\xcb\xe0\xb5\xc4..\xc7\xeb\xba\xc3\xba\xc3\xd1\xa7..:("},

//% {"军学教育","孙子兵法是中国兵法书..","从军报国，我要带兵去打仗",
{"\xbe\xfc\xd1\xa7\xbd\xcc\xd3\xfd","\xcb\xef\xd7\xd3\xb1\xf8\xb7\xa8\xca\xc7\xd6\xd0\xb9\xfa\xb1\xf8\xb7\xa8\xca\xe9..","\xb4\xd3\xbe\xfc\xb1\xa8\xb9\xfa\xa3\xac\xce\xd2\xd2\xaa\xb4\xf8\xb1\xf8\xc8\xa5\xb4\xf2\xd5\xcc",
	    //% "什麽阵形ㄚ?混乱阵形?? @_@","你还以为你在玩叁国志ㄚ?"},
	    "\xca\xb2\xf7\xe1\xd5\xf3\xd0\xce\xa8\xda?\xbb\xec\xc2\xd2\xd5\xf3\xd0\xce?? @_@","\xc4\xe3\xbb\xb9\xd2\xd4\xce\xaa\xc4\xe3\xd4\xda\xcd\xe6\xc8\xfe\xb9\xfa\xd6\xbe\xa8\xda?"},

//% {"剑道技术","看我的厉害  独孤九剑....","我刺 我刺 我刺刺刺..",
{"\xbd\xa3\xb5\xc0\xbc\xbc\xca\xf5","\xbf\xb4\xce\xd2\xb5\xc4\xc0\xf7\xba\xa6  \xb6\xc0\xb9\xc2\xbe\xc5\xbd\xa3....","\xce\xd2\xb4\xcc \xce\xd2\xb4\xcc \xce\xd2\xb4\xcc\xb4\xcc\xb4\xcc..",
	    //% "剑要拿稳一点啦..","你在刺地鼠ㄚ? 剑拿高一点"},
	    "\xbd\xa3\xd2\xaa\xc4\xc3\xce\xc8\xd2\xbb\xb5\xe3\xc0\xb2..","\xc4\xe3\xd4\xda\xb4\xcc\xb5\xd8\xca\xf3\xa8\xda? \xbd\xa3\xc4\xc3\xb8\xdf\xd2\xbb\xb5\xe3"},

//% {"格斗战技","肌肉是肌肉  呼呼..","十八铜人行气散..",
{"\xb8\xf1\xb6\xb7\xd5\xbd\xbc\xbc","\xbc\xa1\xc8\xe2\xca\xc7\xbc\xa1\xc8\xe2  \xba\xf4\xba\xf4..","\xca\xae\xb0\xcb\xcd\xad\xc8\xcb\xd0\xd0\xc6\xf8\xc9\xa2..",
	    //% "脚再踢高一点啦...","拳头怎麽这麽没力ㄚ.."},
	    "\xbd\xc5\xd4\xd9\xcc\xdf\xb8\xdf\xd2\xbb\xb5\xe3\xc0\xb2...","\xc8\xad\xcd\xb7\xd4\xf5\xf7\xe1\xd5\xe2\xf7\xe1\xc3\xbb\xc1\xa6\xa8\xda.."},

//% {"魔法教育","我变 我变 我变变变..","蛇胆+蟋蜴尾+鼠牙+蟾蜍=??",
{"\xc4\xa7\xb7\xa8\xbd\xcc\xd3\xfd","\xce\xd2\xb1\xe4 \xce\xd2\xb1\xe4 \xce\xd2\xb1\xe4\xb1\xe4\xb1\xe4..","\xc9\xdf\xb5\xa8+\xf3\xac\xf2\xe6\xce\xb2+\xca\xf3\xd1\xc0+\xf3\xb8\xf2\xdc=??",
	    //% "小心你的扫帚啦  不要乱挥..","ㄟ～口水不要流到水晶球上.."},
	    "\xd0\xa1\xd0\xc4\xc4\xe3\xb5\xc4\xc9\xa8\xd6\xe3\xc0\xb2  \xb2\xbb\xd2\xaa\xc2\xd2\xbb\xd3..","\xa8\xdf\xa1\xab\xbf\xda\xcb\xae\xb2\xbb\xd2\xaa\xc1\xf7\xb5\xbd\xcb\xae\xbe\xa7\xc7\xf2\xc9\xcf.."},

//% {"礼仪教育","要当只有礼貌的鸡...","欧嗨唷..ㄚ哩ㄚ豆..",
{"\xc0\xf1\xd2\xc7\xbd\xcc\xd3\xfd","\xd2\xaa\xb5\xb1\xd6\xbb\xd3\xd0\xc0\xf1\xc3\xb2\xb5\xc4\xbc\xa6...","\xc5\xb7\xe0\xcb\xe0\xa1..\xa8\xda\xc1\xa8\xa8\xda\xb6\xb9..",
	    //% "怎麽学不会ㄚ??天呀..","走起路来没走样..天ㄚ.."},
	    "\xd4\xf5\xf7\xe1\xd1\xa7\xb2\xbb\xbb\xe1\xa8\xda??\xcc\xec\xd1\xbd..","\xd7\xdf\xc6\xf0\xc2\xb7\xc0\xb4\xc3\xbb\xd7\xdf\xd1\xf9..\xcc\xec\xa8\xda.."},

//% {"绘画技巧","很不错唷..有美术天份..","这幅画的颜色搭配的很好..",
{"\xbb\xe6\xbb\xad\xbc\xbc\xc7\xc9","\xba\xdc\xb2\xbb\xb4\xed\xe0\xa1..\xd3\xd0\xc3\xc0\xca\xf5\xcc\xec\xb7\xdd..","\xd5\xe2\xb7\xf9\xbb\xad\xb5\xc4\xd1\xd5\xc9\xab\xb4\xee\xc5\xe4\xb5\xc4\xba\xdc\xba\xc3..",
	    //% "不要鬼画符啦..要加油..","不要咬画笔啦..坏坏小鸡喔.."},
	    "\xb2\xbb\xd2\xaa\xb9\xed\xbb\xad\xb7\xfb\xc0\xb2..\xd2\xaa\xbc\xd3\xd3\xcd..","\xb2\xbb\xd2\xaa\xd2\xa7\xbb\xad\xb1\xca\xc0\xb2..\xbb\xb5\xbb\xb5\xd0\xa1\xbc\xa6\xe0\xb8.."},

//% {"舞蹈技巧","你就像一只天鹅喔..","舞蹈细胞很好喔..",
{"\xce\xe8\xb5\xb8\xbc\xbc\xc7\xc9","\xc4\xe3\xbe\xcd\xcf\xf1\xd2\xbb\xd6\xbb\xcc\xec\xb6\xec\xe0\xb8..","\xce\xe8\xb5\xb8\xcf\xb8\xb0\xfb\xba\xdc\xba\xc3\xe0\xb8..",
            //% "身体再柔软一点..","拜托你优美一点..不要这麽粗鲁.."}};
            "\xc9\xed\xcc\xe5\xd4\xd9\xc8\xe1\xc8\xed\xd2\xbb\xb5\xe3..","\xb0\xdd\xcd\xd0\xc4\xe3\xd3\xc5\xc3\xc0\xd2\xbb\xb5\xe3..\xb2\xbb\xd2\xaa\xd5\xe2\xf7\xe1\xb4\xd6\xc2\xb3.."}};
/*---------------------------------------------------------------------------*/
/* 修行选单:念书 练武 修行                                                   */
/* 函式库                                                                    */
/*---------------------------------------------------------------------------*/

int pip_practice_classA()
{
/*  ├————┼——————————————————————┤*/
/*  │自然科学│智力 + 1~ 4 , 信仰 - 0~0 , 抗魔能力 - 0~0   │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 2~ 6 , 信仰 - 0~1 , 抗魔能力 - 0~1   │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 3~ 8 , 信仰 - 0~2 , 抗魔能力 - 0~1   │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 4~12 , 信仰 - 1~3 , 抗魔能力 - 0~1   │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=d.wisdom/200+1; /*科学*/
	if(class>5) class=5;

	body=pip_practice_function(1,class,11,12,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;      
	d.wisdom+=change4;
	if(body==1)
	{ 
		d.belief-=rand()%(2+class*2);
		d.mresist-=rand()%4;
	}
	else
	{
		d.belief-=rand()%(2+class*2);
		d.mresist-=rand()%3;      
	}
	pip_practice_gradeup(1,class,d.wisdom/200+1);
	if(d.belief<0)  d.belief=0;
	if(d.mresist<0) d.mresist=0;
	d.classA+=1;
	return 0;
}   

int pip_practice_classB()
{
/*  ├————┼——————————————————————┤*/
/*  │诗词    │感受 + 1~1 , 智力 + 0~1 , 艺术修养 + 0~1    │*/
/*  │        │气质 + 0~1                                  │*/
/*  │        ├——————————————————————┤*/
/*  │        │感受 + 1~2 , 智力 + 0~2 , 艺术修养 + 0~1    │*/
/*  │        │气质 + 0~1                                  │*/
/*  │        ├——————————————————————┤*/
/*  │        │感受 + 1~4 , 智力 + 0~3 , 艺术修养 + 0~1    │*/
/*  │        │气质 + 0~1                                  │*/
/*  │        ├——————————————————————┤*/
/*  │        │感受 + 2~5 , 智力 + 0~4 , 艺术修养 + 0~1    │*/
/*  │        │气质 + 0~1                                  │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.affect*2+d.wisdom+d.art*2+d.character)/400+1; /*诗词*/
	if(class>5) class=5;
     
	body=pip_practice_function(2,class,21,21,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;            
	d.affect+=change3;
	if(body==1)
	{ 
		d.wisdom+=rand()%(class+3);
		d.character+=rand()%(class+3);
		d.art+=rand()%(class+3);
	}
	else
	{
		d.wisdom+=rand()%(class+2);
		d.character+=rand()%(class+2);
		d.art+=rand()%(class+2);      
	}
	body=(d.affect*2+d.wisdom+d.art*2+d.character)/400+1;
	pip_practice_gradeup(2,class,body);
	d.classB+=1;
	return 0;
}

int pip_practice_classC()
{
/*  ├————┼——————————————————————┤*/
/*  │神学    │智力 + 1~1 , 信仰 + 1~2 , 抗魔能力 + 0~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 1~1 , 信仰 + 1~3 , 抗魔能力 + 0~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 1~2 , 信仰 + 1~4 , 抗魔能力 + 0~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 1~3 , 信仰 + 1~5 , 抗魔能力 + 0~1    │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.belief*2+d.wisdom)/400+1; /*神学*/
	if(class>5) class=5;

	body=pip_practice_function(3,class,31,31,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;            
	d.wisdom+=change2;
	d.belief+=change3;
	if(body==1)
	{ 
		d.mresist+=rand()%5;
	}
	else
	{
		d.mresist+=rand()%3;
	}
	body=(d.belief*2+d.wisdom)/400+1;
	pip_practice_gradeup(3,class,body);
	d.classC+=1;
	return 0;        
}

int pip_practice_classD()
{    
/*  ├————┼——————————————————————┤*/
/*  │军学    │智力 + 1~2 , 战斗技术 + 0~1 , 感受 - 0~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 2~4 , 战斗技术 + 0~1 , 感受 - 0~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 3~4 , 战斗技术 + 0~1 , 感受 - 0~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │智力 + 4~5 , 战斗技术 + 0~1 , 感受 - 0~1    │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.hskill*2+d.wisdom)/400+1;
	if(class>5) class=5;
	body=pip_practice_function(4,class,41,41,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;            
	d.wisdom+=change2;
	if(body==1)
	{ 
		d.hskill+=rand()%3+4;
		d.affect-=rand()%3+6;
	}
	else
	{
		d.hskill+=rand()%3+2;
		d.affect-=rand()%3+6;
	}
	body=(d.hskill*2+d.wisdom)/400+1;
	pip_practice_gradeup(4,class,body);
	if(d.affect<0)  d.affect=0;
	d.classD+=1;
	return 0;    
}

int pip_practice_classE()
{ 
/*  ├————┼——————————————————————┤*/
/*  │剑术    │战斗技术 + 0~1 , 攻击能力 + 1~1             │*/
/*  │        ├——————————————————————┤*/
/*  │        │战斗技术 + 0~1 , 攻击能力 + 1~2             │*/
/*  │        ├——————————————————————┤*/
/*  │        │战斗技术 + 0~1 , 攻击能力 + 1~3             │*/
/*  │        ├——————————————————————┤*/
/*  │        │战斗技术 + 0~1 , 攻击能力 + 1~4             │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.hskill+d.attack)/400+1;
	if(class>5) class=5;
     
	body=pip_practice_function(5,class,51,51,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;      
	d.speed+=rand()%3+2;
	d.hexp+=rand()%2+2;
	d.attack+=change4;
	if(body==1)
	{ 
		d.hskill+=rand()%3+5;
	}
	else
	{
		d.hskill+=rand()%3+3;
	}
	body=(d.hskill+d.attack)/400+1;
	pip_practice_gradeup(5,class,body);
	d.classE+=1;
	return 0;        
}

int pip_practice_classF()
{     
/*  ├————┼——————————————————————┤*/
/*  │格斗术  │战斗技术 + 1~1 , 防御能力 + 0~0             │*/
/*  │        ├——————————————————————┤*/
/*  │        │战斗技术 + 1~1 , 防御能力 + 0~1             │*/
/*  │        ├——————————————————————┤*/
/*  │        │战斗技术 + 1~2 , 防御能力 + 0~1             │*/
/*  │        ├——————————————————————┤*/
/*  │        │战斗技术 + 1~3 , 防御能力 + 0~1             │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.hskill+d.resist)/400+1;
	if(class>5) class=5;
     
	body=pip_practice_function(6,class,61,61,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;
	d.hexp+=rand()%2+2;
	d.speed+=rand()%3+2;
	d.resist+=change2;
	if(body==1)
	{ 
		d.hskill+=rand()%3+5;
	}
	else
	{
		d.hskill+=rand()%3+3;
	}
	body=(d.hskill+d.resist)/400+1;
	pip_practice_gradeup(6,class,body);
	d.classF+=1;
	return 0;             
}

int pip_practice_classG()
{     
/*  ├————┼——————————————————————┤*/
/*  │魔法    │魔法技术 + 1~1 , 魔法能力 + 0~2             │*/
/*  │        ├——————————————————————┤*/
/*  │        │魔法技术 + 1~2 , 魔法能力 + 0~3             │*/
/*  │        ├——————————————————————┤*/
/*  │        │魔法技术 + 1~3 , 魔法能力 + 0~4             │*/
/*  │        ├——————————————————————┤*/
/*  │        │魔法技术 + 2~4 , 魔法能力 + 0~5             │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];

	class=(d.mskill+d.maxmp)/400+1;
	if(class>5) class=5;

	body=pip_practice_function(7,class,71,72,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;     
	d.maxmp+=change3;
	d.mexp+=rand()%2+2;
	if(body==1)
	{ 
		d.mskill+=rand()%3+7;
	}
	else
	{
		d.mskill+=rand()%3+4;
	}
	
	body=(d.mskill+d.maxmp)/400+1;
	pip_practice_gradeup(7,class,body);	
	d.classG+=1;
	return 0;                  
}

int pip_practice_classH()
{     
/*  ├————┼——————————————————————┤*/
/*  │礼仪    │礼仪表现 + 1~1 , 气质 + 1~1                 │*/
/*  │        ├——————————————————————┤*/
/*  │        │礼仪表现 + 1~2 , 气质 + 1~2                 │*/
/*  │        ├——————————————————————┤*/
/*  │        │礼仪表现 + 1~3 , 气质 + 1~3                 │*/
/*  │        ├——————————————————————┤*/
/*  │        │礼仪表现 + 2~4 , 气质 + 1~4                 │*/
/*  ├————┼——————————————————————┤*/    
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];

	class=(d.manners*2+d.character)/400+1;
	if(class>5) class=5;
	
	body=pip_practice_function(8,class,0,0,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;     
	d.social+=rand()%2+2;
	d.manners+=change1+rand()%2;
	d.character+=change1+rand()%2;
	body=(d.character+d.manners)/400+1;
	pip_practice_gradeup(8,class,body);
	d.classH+=1;
	return 0;  
}

int pip_practice_classI()
{          
/*  ├————┼——————————————————————┤*/
/*  │绘画    │艺术修养 + 1~1 , 感受 + 0~1                 │*/
/*  │        ├——————————————————————┤*/
/*  │        │艺术修养 + 1~2 , 感受 + 0~1                 │*/
/*  │        ├——————————————————————┤*/
/*  │        │艺术修养 + 1~3 , 感受 + 0~1                 │*/
/*  │        ├——————————————————————┤*/
/*  │        │艺术修养 + 2~4 , 感受 + 0~1                 │*/
/*  ├————┼——————————————————————┤*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.art*2+d.character)/400+1;
	if(class>5) class=5;
     
	body=pip_practice_function(9,class,91,91,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;
	d.art+=change4;
	d.affect+=change2;
	body=(d.affect+d.art)/400+1;
	pip_practice_gradeup(9,class,body);
	d.classI+=1;
	return 0;        
}

int pip_practice_classJ()
{    
/*  ├————┼——————————————————————┤*/
/*  │舞蹈    │艺术修养 + 0~1 , 魅力 + 0~1 , 体力 + 1~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │艺术修养 + 1~1 , 魅力 + 0~1 , 体力 + 1~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │艺术修养 + 1~2 , 魅力 + 0~2 , 体力 + 1~1    │*/
/*  │        ├——————————————————————┤*/
/*  │        │艺术修养 + 1~3 , 魅力 + 1~2 , 体力 + 1~1    │*/
/*  └————┴——————————————————————┘*/
	int body,class;
	int change1,change2,change3,change4,change5;
	char inbuf[256];
     
	class=(d.art*2+d.charm)/400+1;
	if(class>5) class=5;

	body=pip_practice_function(10,class,0,0,&change1,&change2,&change3,&change4,&change5);
	if(body==0) return 0;
	d.art+=change2;
	d.maxhp+=rand()%3+2;
	if(body==1)
	{ 
		d.charm+=rand()%(4+class);
	}
	else if(body==2)
	{
		d.charm+=rand()%(2+class);
	}
	body=(d.art*2+d.charm)/400+1;
	pip_practice_gradeup(10,class,body);
	d.classJ+=1;
	return 0;            
}

/*传入:课号 等级 生命 快乐 满足 脏脏 传回:变数12345 return:body*/
int
pip_practice_function(classnum,classgrade,pic1,pic2,change1,change2,change3,change4,change5)
int classnum,classgrade,pic1,pic2;
int *change1,*change2,*change3,*change4,*change5;
{
	int  a,b,body,health;
	char inbuf[256],ans[5];
	long smoney;

	/*钱的算法*/
	smoney=classgrade*classmoney[classnum][0]+classmoney[classnum][1];
	move(b_lines-2, 0);
	clrtoeol();
	//% sprintf(inbuf,"[%8s%4s课程]要花 $%d ,确定要吗??[y/N]",classword[classnum][0],classrank[classgrade],smoney);
	sprintf(inbuf,"[%8s%4s\xbf\xce\xb3\xcc]\xd2\xaa\xbb\xa8 $%d ,\xc8\xb7\xb6\xa8\xd2\xaa\xc2\xf0??[y/N]",classword[classnum][0],classrank[classgrade],smoney);
#ifdef MAPLE
	getdata(b_lines-2, 1,inbuf, ans, 2, 1, 0);
#else
        getdata(b_lines-2, 1,inbuf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
	if(ans[0]!='y' && ans[0]!='Y')  return 0;
	if(d.money<smoney)
	{
		//% pressanykey("很抱歉喔...你的钱不够喔");
		pressanykey("\xba\xdc\xb1\xa7\xc7\xb8\xe0\xb8...\xc4\xe3\xb5\xc4\xc7\xae\xb2\xbb\xb9\xbb\xe0\xb8");
		return 0;
	}
	count_tired(4,5,"Y",100,1);
	d.money=d.money-smoney;
	/*成功与否的判断*/
	health=d.hp*1/2+rand()%20 - d.tired;
	if(health>0) body=1;
	else body=2;

	a=rand()%3+2;
	b=(rand()%12+rand()%13)%2;
	d.hp-=rand()%(3+rand()%3)+classvariable[classnum][0];
	d.happy-=rand()%(3+rand()%3)+classvariable[classnum][1];
	d.satisfy-=rand()%(3+rand()%3)+classvariable[classnum][2];
	d.shit+=rand()%(3+rand()%3)+classvariable[classnum][3];
	*change1=rand()%a+2+classgrade*2/(body+1);	/* rand()%3+3 */
	*change2=rand()%a+4+classgrade*2/(body+1);	/* rand()%3+5 */
	*change3=rand()%a+5+classgrade*3/(body+1);	/* rand()%3+7 */
	*change4=rand()%a+7+classgrade*3/(body+1);	/* rand()%3+9 */
	*change5=rand()%a+9+classgrade*3/(body+1);	/* rand()%3+11 */
	if(rand()%2>0 && pic1>0)
		show_practice_pic(pic1);
	else if(pic2>0)
		show_practice_pic(pic2);
	pressanykey(classword[classnum][body+b]);
	return body;	
}

int pip_practice_gradeup(classnum,classgrade,data)
int classnum,classgrade,data;
{
	char inbuf[256];
	
	if((data==(classgrade+1)) && classgrade<5 )
	{
		//% sprintf(inbuf,"下次换上 [%8s%4s课程]",
		sprintf(inbuf,"\xcf\xc2\xb4\xce\xbb\xbb\xc9\xcf [%8s%4s\xbf\xce\xb3\xcc]",
		classword[classnum][0],classrank[classgrade+1]);        
		pressanykey(inbuf);
	}
	return 0;
}
