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
char *classrank[6]={"没有","初级","中级","高级","进阶","专业"};
int classmoney[11][2]={{ 0,  0},
		       {60,110},{70,120},{70,120},{80,130},{70,120},
		       {60,110},{90,140},{70,120},{70,120},{80,130}};		     
int classvariable[11][4]={
{0,0,0,0},
{5,5,4,4},{5,7,6,4},{5,7,6,4},{5,6,5,4},{7,5,4,6},
{7,5,4,6},{6,5,4,6},{6,6,5,4},{5,5,4,7},{7,5,4,7}};


char *classword[11][5]=
{
{"课名","成功一","成功二","失败一","失败二"},
 
{"自然科学","正在用功读书中..","我是聪明鸡 cccc...",
            "这题怎麽看不懂咧..怪了","念不完了 :~~~~~~"},
            
{"唐诗宋词","床前明月光...疑是地上霜...","红豆生南国..春来发几枝..",
            "ㄟ..上课不要流口水","你还混喔..罚你背会唐诗叁百首"},

{"神学教育","哈雷路亚  哈雷路亚","让我们迎接天堂之门",
	    "ㄟ..你在干嘛ㄚ? 还不好好念","神学很严肃的..请好好学..:("},

{"军学教育","孙子兵法是中国兵法书..","从军报国，我要带兵去打仗",
	    "什麽阵形ㄚ?混乱阵形?? @_@","你还以为你在玩叁国志ㄚ?"},

{"剑道技术","看我的厉害  独孤九剑....","我刺 我刺 我刺刺刺..",
	    "剑要拿稳一点啦..","你在刺地鼠ㄚ? 剑拿高一点"},

{"格斗战技","肌肉是肌肉  呼呼..","十八铜人行气散..",
	    "脚再踢高一点啦...","拳头怎麽这麽没力ㄚ.."},

{"魔法教育","我变 我变 我变变变..","蛇胆+蟋蜴尾+鼠牙+蟾蜍=??",
	    "小心你的扫帚啦  不要乱挥..","ㄟ～口水不要流到水晶球上.."},

{"礼仪教育","要当只有礼貌的鸡...","欧嗨唷..ㄚ哩ㄚ豆..",
	    "怎麽学不会ㄚ??天呀..","走起路来没走样..天ㄚ.."},

{"绘画技巧","很不错唷..有美术天份..","这幅画的颜色搭配的很好..",
	    "不要鬼画符啦..要加油..","不要咬画笔啦..坏坏小鸡喔.."},

{"舞蹈技巧","你就像一只天鹅喔..","舞蹈细胞很好喔..",
            "身体再柔软一点..","拜托你优美一点..不要这麽粗鲁.."}};
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
	sprintf(inbuf,"[%8s%4s课程]要花 $%d ,确定要吗??[y/N]",classword[classnum][0],classrank[classgrade],smoney);
#ifdef MAPLE
	getdata(b_lines-2, 1,inbuf, ans, 2, 1, 0);
#else
        getdata(b_lines-2, 1,inbuf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
	if(ans[0]!='y' && ans[0]!='Y')  return 0;
	if(d.money<smoney)
	{
		pressanykey("很抱歉喔...你的钱不够喔");
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
		sprintf(inbuf,"下次换上 [%8s%4s课程]",
		classword[classnum][0],classrank[classgrade+1]);        
		pressanykey(inbuf);
	}
	return 0;
}
