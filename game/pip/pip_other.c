/*---------------------------------------------------------------------------*/
/* 函式特区                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

/*名字        体力MAX法力MAX  攻击   防护     速度    财宝   特别   图档*/
struct playrule resultmanlist[] = {
//% "茱丽叶塔",	 60,0,	20,0,	20,	20,	 20,	150, "11101",	0,0,
"\xdc\xef\xc0\xf6\xd2\xb6\xcb\xfe",	 60,0,	20,0,	20,	20,	 20,	150, "11101",	0,0,
//% "菲欧利娜",	 60,0,	20,0,	30,	30,	 30,	200, "01111",	0,0,
"\xb7\xc6\xc5\xb7\xc0\xfb\xc4\xc8",	 60,0,	20,0,	30,	30,	 30,	200, "01111",	0,0,
//% "阿妮斯",	 80,0,	40,0,	50,	35,	 60,	250, "11110",	0,0,
"\xb0\xa2\xc4\xdd\xcb\xb9",	 80,0,	40,0,	50,	35,	 60,	250, "11110",	0,0,
//% "帕多雷西亚",	 85,0,	30,0,	80,	90,	 80,	500, "10111",	0,0,
"\xc5\xc1\xb6\xe0\xc0\xd7\xce\xf7\xd1\xc7",	 85,0,	30,0,	80,	90,	 80,	500, "10111",	0,0,
//% "卡美拉美", 	 90,0,  50,0,   75,	70,	 60,	550, "11010",	0,0,
"\xbf\xa8\xc3\xc0\xc0\xad\xc3\xc0", 	 90,0,  50,0,   75,	70,	 60,	550, "11010",	0,0,
//% "姗娜丽娃",	 90,0,  40,0,   10,     30,      50,    880, "10100",   0,0,
"\xe6\xa9\xc4\xc8\xc0\xf6\xcd\xde",	 90,0,  40,0,   10,     30,      50,    880, "10100",   0,0,
//NULL,		  0,0,	 0,0,	 0,	 0,	  0,	  0,	NULL,	0,0
};
/*求婚*/
int
pip_marriage_offer()
{
   time_t now;
   char buf[256];
   char ans[4];
   int money;
   int who;
   //% char *name[5][2]={{"女商人Ａ","商人Ａ"},
   char *name[5][2]={{"\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc1","\xc9\xcc\xc8\xcb\xa3\xc1"},
                     //% {"女商人Ｂ","商人Ｂ"},
                     {"\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc2","\xc9\xcc\xc8\xcb\xa3\xc2"},
                     //% {"女商人Ｃ","商人Ｃ"},
                     {"\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc3","\xc9\xcc\xc8\xcb\xa3\xc3"},
                     //% {"女商人Ｄ","商人Ｄ"},
                     {"\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc4","\xc9\xcc\xc8\xcb\xa3\xc4"},
                     //% {"女商人Ｅ","商人Ｅ"}};
                     {"\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc5","\xc9\xcc\xc8\xcb\xa3\xc5"}};
   do
   { 
     who=rand()%5;
   }while(d.lover==(who+3));
  
   money=rand()%2000+rand()%3000+4000;
   //% sprintf(buf,"%s带来了金钱%d，要向你的小鸡求婚，您愿意吗？[y/N]",name[who][d.sex-1],money);
   sprintf(buf,"%s\xb4\xf8\xc0\xb4\xc1\xcb\xbd\xf0\xc7\xae%d\xa3\xac\xd2\xaa\xcf\xf2\xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6\xc7\xf3\xbb\xe9\xa3\xac\xc4\xfa\xd4\xb8\xd2\xe2\xc2\xf0\xa3\xbf[y/N]",name[who][d.sex-1],money);
#ifdef MAPLE
   getdata(b_lines-1, 1,buf, ans, 2, 1, 0);
#else
   getdata(b_lines-1, 1,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
   if(ans[0]=='y' || ans[0]=='Y')
   {
     if(d.wantend!=1 && d.wantend!=4)
     {
       //% sprintf(buf,"ㄚ～之前已经有婚约了，您确定要解除旧婚约，改定立婚约吗？[y/N]");
       sprintf(buf,"\xa8\xda\xa1\xab\xd6\xae\xc7\xb0\xd2\xd1\xbe\xad\xd3\xd0\xbb\xe9\xd4\xbc\xc1\xcb\xa3\xac\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xbd\xe2\xb3\xfd\xbe\xc9\xbb\xe9\xd4\xbc\xa3\xac\xb8\xc4\xb6\xa8\xc1\xa2\xbb\xe9\xd4\xbc\xc2\xf0\xa3\xbf[y/N]");
#ifdef MAPLE
       getdata(b_lines-1, 1,buf, ans, 2, 1, 0);
#else
       getdata(b_lines-1, 1,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
       if(ans[0]!='y' && ans[0]!='Y')
       {
         d.social+=10;
         //% pressanykey("还是维持旧婚约好了..");
         pressanykey("\xbb\xb9\xca\xc7\xce\xac\xb3\xd6\xbe\xc9\xbb\xe9\xd4\xbc\xba\xc3\xc1\xcb..");
         return 0;
       }
       d.social-=rand()%50+100;
     }
     d.charm-=rand()%5+20;
     d.lover=who+3;
     d.relation-=20;
     if(d.relation<0)
        d.relation=0;
     if(d.wantend<4)
     	d.wantend=2;
     else
        d.wantend=5;
     //% pressanykey("我想对方是一个很好的伴侣..");
     pressanykey("\xce\xd2\xcf\xeb\xb6\xd4\xb7\xbd\xca\xc7\xd2\xbb\xb8\xf6\xba\xdc\xba\xc3\xb5\xc4\xb0\xe9\xc2\xc2..");
     now=time(0);
     //% sprintf(buf, "[1;37m%s %-11s的小鸡 [%s] 接受了 %s 的求婚[0m\n", Cdate(&now), cuser.userid,d.name,name[who][d.sex-1]);
     sprintf(buf, "[1;37m%s %-11s\xb5\xc4\xd0\xa1\xbc\xa6 [%s] \xbd\xd3\xca\xdc\xc1\xcb %s \xb5\xc4\xc7\xf3\xbb\xe9[0m\n", Cdate(&now), cuser.userid,d.name,name[who][d.sex-1]);
     pip_log_record(buf);
   }
   else
   {
     d.charm+=rand()%5+20;
     d.relation+=20;
     if(d.wantend==1 || d.wantend==4){
	     //% pressanykey("我还年轻  心情还不定...");
	     pressanykey("\xce\xd2\xbb\xb9\xc4\xea\xc7\xe1  \xd0\xc4\xc7\xe9\xbb\xb9\xb2\xbb\xb6\xa8...");
     } else {
     	     //% pressanykey("我早已有婚约了..对不起...");
     	     pressanykey("\xce\xd2\xd4\xe7\xd2\xd1\xd3\xd0\xbb\xe9\xd4\xbc\xc1\xcb..\xb6\xd4\xb2\xbb\xc6\xf0...");
     }
   }
   d.money+=money;
   return 0;
}

int pip_results_show()  /*收获季*/
{
	//% char *showname[5]={"  ","武斗大会","艺术大展","皇家舞会","烹饪大赛"};
	char *showname[5]={"  ","\xce\xe4\xb6\xb7\xb4\xf3\xbb\xe1","\xd2\xd5\xca\xf5\xb4\xf3\xd5\xb9","\xbb\xca\xbc\xd2\xce\xe8\xbb\xe1","\xc5\xeb\xe2\xbf\xb4\xf3\xc8\xfc"};
	char buf[256];
	int pipkey,i=0;
	int winorlost=0;
	int a,b[3][2],c[3];

	clear();
	move(10,14);
	//% prints("[1;33m叮咚叮咚～ 辛苦的邮差帮我们送信来了喔...[0m");
	prints("[1;33m\xb6\xa3\xdf\xcb\xb6\xa3\xdf\xcb\xa1\xab \xd0\xc1\xbf\xe0\xb5\xc4\xd3\xca\xb2\xee\xb0\xef\xce\xd2\xc3\xc7\xcb\xcd\xd0\xc5\xc0\xb4\xc1\xcb\xe0\xb8...[0m");
	//% pressanykey("嗯  把信打开看看吧...");
	pressanykey("\xe0\xc5  \xb0\xd1\xd0\xc5\xb4\xf2\xbf\xaa\xbf\xb4\xbf\xb4\xb0\xc9...");
	clear();
	show_resultshow_pic(0);
	//% sprintf(buf,"[A]%s [B]%s [C]%s [D]%s [Q]放弃:",showname[1],showname[2],showname[3],showname[4]);
	sprintf(buf,"[A]%s [B]%s [C]%s [D]%s [Q]\xb7\xc5\xc6\xfa:",showname[1],showname[2],showname[3],showname[4]);
	move(b_lines,0);
	prints(buf);	
	do
	{
		pipkey=egetch();
	}
	while(pipkey!='q' && pipkey!='Q' && pipkey!='A' && pipkey!='a' &&
	       pipkey!='B' && pipkey!='b' && pipkey!='C' && pipkey!='c'&&
	       pipkey!='D' && pipkey!='d');
	a=rand()%4+1;
	b[0][0]=a-1;
	b[1][0]=a+1;
	b[2][0]=a;
	switch(pipkey)
	{
		case 'A':
		case 'a':
			//% pressanykey("今年共有四人参赛～现在比赛开始");
			pressanykey("\xbd\xf1\xc4\xea\xb9\xb2\xd3\xd0\xcb\xc4\xc8\xcb\xb2\xce\xc8\xfc\xa1\xab\xcf\xd6\xd4\xda\xb1\xc8\xc8\xfc\xbf\xaa\xca\xbc");
			for(i=0;i<3;i++)
			{
				a=0;
				b[i][1]=0;
				//% sprintf(buf,"你的第%d个对手是%s",i+1,resultmanlist[b[i][0]].name);
				sprintf(buf,"\xc4\xe3\xb5\xc4\xb5\xda%d\xb8\xf6\xb6\xd4\xca\xd6\xca\xc7%s",i+1,resultmanlist[b[i][0]].name);
				pressanykey(buf);
				a=pip_vs_man(b[i][0],resultmanlist,2);
				if(a==1)
					b[i][1]=1;/*对方输了*/
				winorlost+=a;	
				d.death=0;
			}
			switch(winorlost)
			{
				case 3:
					pip_results_show_ending(3,1,b[1][0],b[0][0],b[2][0]);
					d.hexp+=rand()%10+50;
					break;
				case 2:
					if(b[0][1]!=1) 
					{
						c[0]=b[0][0];
						c[1]=b[1][0];
						c[2]=b[2][0];
					}
					else if(b[1][1]!=1) 
					{
						c[0]=b[1][0];
						c[1]=b[2][0];
						c[2]=b[0][0];
					}
					else if(b[2][1]!=1) 
					{
						c[0]=b[2][0];
						c[1]=b[0][0];
						c[2]=b[1][0];
					}
					pip_results_show_ending(2,1,c[0],c[1],c[2]);
					d.hexp+=rand()%10+30;
					break;
				case 1:
					if(b[0][1]==1) 
					{
						c[0]=b[2][0];
						c[1]=b[1][0];
						c[2]=b[0][0];
					}
					else if(b[1][1]==1) 
					{
						c[0]=b[0][0];
						c[1]=b[2][0];
						c[2]=b[1][0];
					}
					else if(b[2][1]==1) 
					{
						c[0]=b[1][0];
						c[1]=b[0][0];
						c[2]=b[2][0];
					}
					pip_results_show_ending(1,1,c[0],c[1],c[2]);
					d.hexp+=rand()%10+10;
					break;
				case 0:
					pip_results_show_ending(0,1,b[0][0],b[1][0],b[2][0]);
					d.hexp-=rand()%10+10;
					break;
			}
			break;
		case 'B':
		case 'b':
			//% pressanykey("今年共有四人参赛～现在比赛开始");
			pressanykey("\xbd\xf1\xc4\xea\xb9\xb2\xd3\xd0\xcb\xc4\xc8\xcb\xb2\xce\xc8\xfc\xa1\xab\xcf\xd6\xd4\xda\xb1\xc8\xc8\xfc\xbf\xaa\xca\xbc");
			show_resultshow_pic(21);
			//% pressanykey("比赛情形");
			pressanykey("\xb1\xc8\xc8\xfc\xc7\xe9\xd0\xce");
			if((d.art*2+d.character)/400>=5)
			{
				winorlost=3;
			}
			else if((d.art*2+d.character)/400>=4)
			{
				winorlost=2;
			}
			else if((d.art*2+d.character)/400>=3)
			{
				winorlost=1;
			}
			else
			{
				winorlost=0;
			}
			pip_results_show_ending(winorlost,2,rand()%2,rand()%2+2,rand()%2+4);
			d.art+=rand()%10+20*winorlost;
			d.character+=rand()%10+20*winorlost;
			break;
		case 'C':
		case 'c':
			//% pressanykey("今年共有四人参赛～现在比赛开始");
			pressanykey("\xbd\xf1\xc4\xea\xb9\xb2\xd3\xd0\xcb\xc4\xc8\xcb\xb2\xce\xc8\xfc\xa1\xab\xcf\xd6\xd4\xda\xb1\xc8\xc8\xfc\xbf\xaa\xca\xbc");
			if((d.art*2+d.charm)/400>=5)
			{				
				winorlost=3;
			}
			else if((d.art*2+d.charm)/400>=4)
			{
				winorlost=2;
			}
			else if((d.art*2+d.charm)/400>=3)
			{
				winorlost=1;
			}
			else
			{
				winorlost=0;
			}
			d.art+=rand()%10+20*winorlost;
			d.charm+=rand()%10+20*winorlost;
			pip_results_show_ending(winorlost,3,rand()%2,rand()%2+4,rand()%2+2);
			break;
		case 'D':
		case 'd':
			//% pressanykey("今年共有四人参赛～现在比赛开始");
			pressanykey("\xbd\xf1\xc4\xea\xb9\xb2\xd3\xd0\xcb\xc4\xc8\xcb\xb2\xce\xc8\xfc\xa1\xab\xcf\xd6\xd4\xda\xb1\xc8\xc8\xfc\xbf\xaa\xca\xbc");
			if((d.affect+d.cookskill*2)/200>=4)
			{
				winorlost=3;
			}
			else if((d.affect+d.cookskill*2)/200>=3)
			{
				winorlost=2;
			}
			else if((d.affect+d.cookskill*2)/200>=2)
			{
				winorlost=1;
			}
			else
			{
				winorlost=0;
			}
			d.cookskill+=rand()%10+20*winorlost;
			d.family+=rand()%10+20*winorlost;
			pip_results_show_ending(winorlost,4,rand()%2+2,rand()%2,rand()%2+4);
			break;
		case 'Q':
		case 'q':
			//% pressanykey("今年不参加啦.....:(");
			pressanykey("\xbd\xf1\xc4\xea\xb2\xbb\xb2\xce\xbc\xd3\xc0\xb2.....:(");
			d.happy-=rand()%10+10;
			d.satisfy-=rand()%10+10;
			d.relation-=rand()%10;
			break;
	}
	if(pipkey!='Q' && pipkey!='q')
	{
		d.tired=0;
		d.hp=d.maxhp;
		d.happy+=rand()%20;
		d.satisfy+=rand()%20;
		d.relation+=rand()%10;
	}
	return 0;
}

int pip_results_show_ending(winorlost,mode,a,b,c)
int winorlost,mode,a,b,c;
{
	//% char *resultname[4]={"最後一名","季军","亚军","冠军"};
	char *resultname[4]={"\xd7\xee\xe1\xe1\xd2\xbb\xc3\xfb","\xbc\xbe\xbe\xfc","\xd1\xc7\xbe\xfc","\xb9\xda\xbe\xfc"};
	//% char *gamename[5]={"  ","武斗大会","艺术大展","皇家舞会","烹饪大赛"};
	char *gamename[5]={"  ","\xce\xe4\xb6\xb7\xb4\xf3\xbb\xe1","\xd2\xd5\xca\xf5\xb4\xf3\xd5\xb9","\xbb\xca\xbc\xd2\xce\xe8\xbb\xe1","\xc5\xeb\xe2\xbf\xb4\xf3\xc8\xfc"};
	int resultmoney[4]={0,3000,5000,8000};
	char name1[25],name2[25],name3[25],name4[25];
	char buf[256];
	
	if(winorlost==3)
	{
		strcpy(name1,d.name);
		strcpy(name2,resultmanlist[a].name);
		strcpy(name3,resultmanlist[b].name);
		strcpy(name4,resultmanlist[c].name);
	}
	else if(winorlost==2)
	{
		strcpy(name1,resultmanlist[a].name);
		strcpy(name2,d.name);
		strcpy(name3,resultmanlist[b].name);
		strcpy(name4,resultmanlist[c].name);
	}
	else if(winorlost==1)
	{
		strcpy(name1,resultmanlist[a].name);
		strcpy(name2,resultmanlist[b].name);
		strcpy(name3,d.name);
		strcpy(name4,resultmanlist[c].name);
	}
	else
	{
		strcpy(name1,resultmanlist[a].name);
		strcpy(name2,resultmanlist[b].name);
		strcpy(name3,resultmanlist[c].name);
		strcpy(name4,d.name);
	}	
	clear();
	move(6,13);
	//% prints("[1;37m～～～ [32m本届 %s 结果揭晓 [37m～～～[0m",gamename[mode]);	
	prints("[1;37m\xa1\xab\xa1\xab\xa1\xab [32m\xb1\xbe\xbd\xec %s \xbd\xe1\xb9\xfb\xbd\xd2\xcf\xfe [37m\xa1\xab\xa1\xab\xa1\xab[0m",gamename[mode]);	
	move(8,15);
	//% prints("[1;41m 冠军 [0;1m～ [1;33m%-10s[36m  奖金 %d[0m",name1,resultmoney[3]);
	prints("[1;41m \xb9\xda\xbe\xfc [0;1m\xa1\xab [1;33m%-10s[36m  \xbd\xb1\xbd\xf0 %d[0m",name1,resultmoney[3]);
	move(10,15);
	//% prints("[1;41m 亚军 [0;1m～ [1;33m%-10s[36m  奖金 %d[0m",name2,resultmoney[2]);
	prints("[1;41m \xd1\xc7\xbe\xfc [0;1m\xa1\xab [1;33m%-10s[36m  \xbd\xb1\xbd\xf0 %d[0m",name2,resultmoney[2]);
	move(12,15);
	//% prints("[1;41m 季军 [0;1m～ [1;33m%-10s[36m  奖金 %d[0m",name3,resultmoney[1]);
	prints("[1;41m \xbc\xbe\xbe\xfc [0;1m\xa1\xab [1;33m%-10s[36m  \xbd\xb1\xbd\xf0 %d[0m",name3,resultmoney[1]);
	move(14,15);
	//% prints("[1;41m 最後 [0;1m～ [1;33m%-10s[36m [0m",name4);	
	prints("[1;41m \xd7\xee\xe1\xe1 [0;1m\xa1\xab [1;33m%-10s[36m [0m",name4);	
	//% sprintf(buf,"今年的%s结束罗 後年再来吧..",gamename[mode]);
	sprintf(buf,"\xbd\xf1\xc4\xea\xb5\xc4%s\xbd\xe1\xca\xf8\xc2\xde \xe1\xe1\xc4\xea\xd4\xd9\xc0\xb4\xb0\xc9..",gamename[mode]);
	d.money+=resultmoney[winorlost];
	pressanykey(buf);
	return 0;
}
