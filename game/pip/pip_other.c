/*---------------------------------------------------------------------------*/
/* º¯Ê½ÌØÇø                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

/*Ãû×Ö        ÌåÁ¦MAX·¨Á¦MAX  ¹¥»÷   ·À»¤     ËÙ¶È    ²Æ±¦   ÌØ±ğ   Í¼µµ*/
struct playrule resultmanlist[] = {
"ÜïÀöÒ¶Ëş",	 60,0,	20,0,	20,	20,	 20,	150, "11101",	0,0,
"·ÆÅ·ÀûÄÈ",	 60,0,	20,0,	30,	30,	 30,	200, "01111",	0,0,
"°¢ÄİË¹",	 80,0,	40,0,	50,	35,	 60,	250, "11110",	0,0,
"ÅÁ¶àÀ×Î÷ÑÇ",	 85,0,	30,0,	80,	90,	 80,	500, "10111",	0,0,
"¿¨ÃÀÀ­ÃÀ", 	 90,0,  50,0,   75,	70,	 60,	550, "11010",	0,0,
"æ©ÄÈÀöÍŞ",	 90,0,  40,0,   10,     30,      50,    880, "10100",   0,0,
//NULL,		  0,0,	 0,0,	 0,	 0,	  0,	  0,	NULL,	0,0
};
/*Çó»é*/
int
pip_marriage_offer()
{
   time_t now;
   char buf[256];
   char ans[4];
   int money;
   int who;
   char *name[5][2]={{"Å®ÉÌÈË£Á","ÉÌÈË£Á"},
                     {"Å®ÉÌÈË£Â","ÉÌÈË£Â"},
                     {"Å®ÉÌÈË£Ã","ÉÌÈË£Ã"},
                     {"Å®ÉÌÈË£Ä","ÉÌÈË£Ä"},
                     {"Å®ÉÌÈË£Å","ÉÌÈË£Å"}};
   do
   { 
     who=rand()%5;
   }while(d.lover==(who+3));
  
   money=rand()%2000+rand()%3000+4000;
   sprintf(buf,"%s´øÀ´ÁË½ğÇ®%d£¬ÒªÏòÄãµÄĞ¡¼¦Çó»é£¬ÄúÔ¸ÒâÂğ£¿[y/N]",name[who][d.sex-1],money);
#ifdef MAPLE
   getdata(b_lines-1, 1,buf, ans, 2, 1, 0);
#else
   getdata(b_lines-1, 1,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
   if(ans[0]=='y' || ans[0]=='Y')
   {
     if(d.wantend!=1 && d.wantend!=4)
     {
       sprintf(buf,"¨Ú¡«Ö®Ç°ÒÑ¾­ÓĞ»éÔ¼ÁË£¬ÄúÈ·¶¨Òª½â³ı¾É»éÔ¼£¬¸Ä¶¨Á¢»éÔ¼Âğ£¿[y/N]");
#ifdef MAPLE
       getdata(b_lines-1, 1,buf, ans, 2, 1, 0);
#else
       getdata(b_lines-1, 1,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
       if(ans[0]!='y' && ans[0]!='Y')
       {
         d.social+=10;
         pressanykey("»¹ÊÇÎ¬³Ö¾É»éÔ¼ºÃÁË..");
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
     pressanykey("ÎÒÏë¶Ô·½ÊÇÒ»¸öºÜºÃµÄ°éÂÂ..");
     now=time(0);
     sprintf(buf, "[1;37m%s %-11sµÄĞ¡¼¦ [%s] ½ÓÊÜÁË %s µÄÇó»é[0m\n", Cdate(&now), cuser.userid,d.name,name[who][d.sex-1]);
     pip_log_record(buf);
   }
   else
   {
     d.charm+=rand()%5+20;
     d.relation+=20;
     if(d.wantend==1 || d.wantend==4){
	     pressanykey("ÎÒ»¹ÄêÇá  ĞÄÇé»¹²»¶¨...");
     } else {
     	     pressanykey("ÎÒÔçÒÑÓĞ»éÔ¼ÁË..¶Ô²»Æğ...");
     }
   }
   d.money+=money;
   return 0;
}

int pip_results_show()  /*ÊÕ»ñ¼¾*/
{
	char *showname[5]={"  ","Îä¶·´ó»á","ÒÕÊõ´óÕ¹","»Ê¼ÒÎè»á","Åëâ¿´óÈü"};
	char buf[256];
	int pipkey,i=0;
	int winorlost=0;
	int a,b[3][2],c[3];

	clear();
	move(10,14);
	prints("[1;33m¶£ßË¶£ßË¡« ĞÁ¿àµÄÓÊ²î°ïÎÒÃÇËÍĞÅÀ´ÁËà¸...[0m");
	pressanykey("àÅ  °ÑĞÅ´ò¿ª¿´¿´°É...");
	clear();
	show_resultshow_pic(0);
	sprintf(buf,"[A]%s [B]%s [C]%s [D]%s [Q]·ÅÆú:",showname[1],showname[2],showname[3],showname[4]);
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
			pressanykey("½ñÄê¹²ÓĞËÄÈË²ÎÈü¡«ÏÖÔÚ±ÈÈü¿ªÊ¼");
			for(i=0;i<3;i++)
			{
				a=0;
				b[i][1]=0;
				sprintf(buf,"ÄãµÄµÚ%d¸ö¶ÔÊÖÊÇ%s",i+1,resultmanlist[b[i][0]].name);
				pressanykey(buf);
				a=pip_vs_man(b[i][0],resultmanlist,2);
				if(a==1)
					b[i][1]=1;/*¶Ô·½ÊäÁË*/
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
			pressanykey("½ñÄê¹²ÓĞËÄÈË²ÎÈü¡«ÏÖÔÚ±ÈÈü¿ªÊ¼");
			show_resultshow_pic(21);
			pressanykey("±ÈÈüÇéĞÎ");
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
			pressanykey("½ñÄê¹²ÓĞËÄÈË²ÎÈü¡«ÏÖÔÚ±ÈÈü¿ªÊ¼");
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
			pressanykey("½ñÄê¹²ÓĞËÄÈË²ÎÈü¡«ÏÖÔÚ±ÈÈü¿ªÊ¼");
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
			pressanykey("½ñÄê²»²Î¼ÓÀ².....:(");
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
	char *resultname[4]={"×îááÒ»Ãû","¼¾¾ü","ÑÇ¾ü","¹Ú¾ü"};
	char *gamename[5]={"  ","Îä¶·´ó»á","ÒÕÊõ´óÕ¹","»Ê¼ÒÎè»á","Åëâ¿´óÈü"};
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
	prints("[1;37m¡«¡«¡« [32m±¾½ì %s ½á¹û½ÒÏş [37m¡«¡«¡«[0m",gamename[mode]);	
	move(8,15);
	prints("[1;41m ¹Ú¾ü [0;1m¡« [1;33m%-10s[36m  ½±½ğ %d[0m",name1,resultmoney[3]);
	move(10,15);
	prints("[1;41m ÑÇ¾ü [0;1m¡« [1;33m%-10s[36m  ½±½ğ %d[0m",name2,resultmoney[2]);
	move(12,15);
	prints("[1;41m ¼¾¾ü [0;1m¡« [1;33m%-10s[36m  ½±½ğ %d[0m",name3,resultmoney[1]);
	move(14,15);
	prints("[1;41m ×îáá [0;1m¡« [1;33m%-10s[36m [0m",name4);	
	sprintf(buf,"½ñÄêµÄ%s½áÊøÂŞ ááÄêÔÙÀ´°É..",gamename[mode]);
	d.money+=resultmoney[winorlost];
	pressanykey(buf);
	return 0;
}
