/*---------------------------------------------------------------------------*/
/* 特殊选单:看病 减肥 战斗 拜访 朝见                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

struct royalset royallist[] = {
//% {"T",	"拜访对象",	  0,	0,	 0,	  0,"","" /*NULL,NULL*/},
{"T",	"\xb0\xdd\xb7\xc3\xb6\xd4\xcf\xf3",	  0,	0,	 0,	  0,"","" /*NULL,NULL*/},
//% {"A",	"星空骑兵连",	  1,	10,	15,    	100,"你真好，来陪我聊天..","守卫星空的安全是很辛苦的.."},
{"A",	"\xd0\xc7\xbf\xd5\xc6\xef\xb1\xf8\xc1\xac",	  1,	10,	15,    	100,"\xc4\xe3\xd5\xe6\xba\xc3\xa3\xac\xc0\xb4\xc5\xe3\xce\xd2\xc1\xc4\xcc\xec..","\xca\xd8\xce\xc0\xd0\xc7\xbf\xd5\xb5\xc4\xb0\xb2\xc8\xab\xca\xc7\xba\xdc\xd0\xc1\xbf\xe0\xb5\xc4.."},
//% {"B",	"星空００７",	  1,   100,	25,	200,"真是礼貌的小鸡..我喜欢...","特务就是秘密保护站长安全的人.."},
{"B",	"\xd0\xc7\xbf\xd5\xa3\xb0\xa3\xb0\xa3\xb7",	  1,   100,	25,	200,"\xd5\xe6\xca\xc7\xc0\xf1\xc3\xb2\xb5\xc4\xd0\xa1\xbc\xa6..\xce\xd2\xcf\xb2\xbb\xb6...","\xcc\xd8\xce\xf1\xbe\xcd\xca\xc7\xc3\xd8\xc3\xdc\xb1\xa3\xbb\xa4\xd5\xbe\xb3\xa4\xb0\xb2\xc8\xab\xb5\xc4\xc8\xcb.."},
//% {"C",	"镇国大将军",	  1,   200,	30,	250,"告诉你唷！当年那个战役很精彩喔..","你真是高贵优雅的小鸡..."},
{"C",	"\xd5\xf2\xb9\xfa\xb4\xf3\xbd\xab\xbe\xfc",	  1,   200,	30,	250,"\xb8\xe6\xcb\xdf\xc4\xe3\xe0\xa1\xa3\xa1\xb5\xb1\xc4\xea\xc4\xc7\xb8\xf6\xd5\xbd\xd2\xdb\xba\xdc\xbe\xab\xb2\xca\xe0\xb8..","\xc4\xe3\xd5\xe6\xca\xc7\xb8\xdf\xb9\xf3\xd3\xc5\xd1\xc5\xb5\xc4\xd0\xa1\xbc\xa6..."},
//% {"D",	"参谋总务长",	  1,   300,	35,	300,"我帮站长管理这个国家唷..","你的声音很好听耶..我很喜欢喔...:)"},
{"D",	"\xb2\xce\xc4\xb1\xd7\xdc\xce\xf1\xb3\xa4",	  1,   300,	35,	300,"\xce\xd2\xb0\xef\xd5\xbe\xb3\xa4\xb9\xdc\xc0\xed\xd5\xe2\xb8\xf6\xb9\xfa\xbc\xd2\xe0\xa1..","\xc4\xe3\xb5\xc4\xc9\xf9\xd2\xf4\xba\xdc\xba\xc3\xcc\xfd\xd2\xae..\xce\xd2\xba\xdc\xcf\xb2\xbb\xb6\xe0\xb8...:)"},
//% {"E",	"小天使站长",	  1,   400,	35,	300,"你很有教养唷！很高兴认识你...","优雅的你，请让我帮你祈福...."},
{"E",	"\xd0\xa1\xcc\xec\xca\xb9\xd5\xbe\xb3\xa4",	  1,   400,	35,	300,"\xc4\xe3\xba\xdc\xd3\xd0\xbd\xcc\xd1\xf8\xe0\xa1\xa3\xa1\xba\xdc\xb8\xdf\xd0\xcb\xc8\xcf\xca\xb6\xc4\xe3...","\xd3\xc5\xd1\xc5\xb5\xc4\xc4\xe3\xa3\xac\xc7\xeb\xc8\xc3\xce\xd2\xb0\xef\xc4\xe3\xc6\xed\xb8\xa3...."},
//% {"F",	"风筝手站长",	  1,   500,	40,	350,"你好可爱喔..我喜欢你唷....","对啦..以後要多多来和我玩喔..."},
{"F",	"\xb7\xe7\xf3\xdd\xca\xd6\xd5\xbe\xb3\xa4",	  1,   500,	40,	350,"\xc4\xe3\xba\xc3\xbf\xc9\xb0\xae\xe0\xb8..\xce\xd2\xcf\xb2\xbb\xb6\xc4\xe3\xe0\xa1....","\xb6\xd4\xc0\xb2..\xd2\xd4\xe1\xe1\xd2\xaa\xb6\xe0\xb6\xe0\xc0\xb4\xba\xcd\xce\xd2\xcd\xe6\xe0\xb8..."},
//% {"G",	"乖小孩站长",	  1,   550,	40,	350, "跟你讲话很快乐喔..不像站长一样无聊..","来，坐我膝盖上，听我讲故事.."},
{"G",	"\xb9\xd4\xd0\xa1\xba\xa2\xd5\xbe\xb3\xa4",	  1,   550,	40,	350, "\xb8\xfa\xc4\xe3\xbd\xb2\xbb\xb0\xba\xdc\xbf\xec\xc0\xd6\xe0\xb8..\xb2\xbb\xcf\xf1\xd5\xbe\xb3\xa4\xd2\xbb\xd1\xf9\xce\xde\xc1\xc4..","\xc0\xb4\xa3\xac\xd7\xf8\xce\xd2\xcf\xa5\xb8\xc7\xc9\xcf\xa3\xac\xcc\xfd\xce\xd2\xbd\xb2\xb9\xca\xca\xc2.."},
//% {"H",	"小米克站长",	  1,   600,	50,     400,"一站之长责任重大呀..:)..","谢谢你听我讲话..以後要多来喔..."},
{"H",	"\xd0\xa1\xc3\xd7\xbf\xcb\xd5\xbe\xb3\xa4",	  1,   600,	50,     400,"\xd2\xbb\xd5\xbe\xd6\xae\xb3\xa4\xd4\xf0\xc8\xce\xd6\xd8\xb4\xf3\xd1\xbd..:)..","\xd0\xbb\xd0\xbb\xc4\xe3\xcc\xfd\xce\xd2\xbd\xb2\xbb\xb0..\xd2\xd4\xe1\xe1\xd2\xaa\xb6\xe0\xc0\xb4\xe0\xb8..."},
//% {"I",	"星空灌水群",	  2,    60,	 0,	  0,"不错唷..蛮机灵的喔..很可爱....","来  我们一起来灌水吧...."},
{"I",	"\xd0\xc7\xbf\xd5\xb9\xe0\xcb\xae\xc8\xba",	  2,    60,	 0,	  0,"\xb2\xbb\xb4\xed\xe0\xa1..\xc2\xf9\xbb\xfa\xc1\xe9\xb5\xc4\xe0\xb8..\xba\xdc\xbf\xc9\xb0\xae....","\xc0\xb4  \xce\xd2\xc3\xc7\xd2\xbb\xc6\xf0\xc0\xb4\xb9\xe0\xcb\xae\xb0\xc9...."},
//% {"J",	"青年帅武官",	  0,	 0,	 0,	  0,"你好，我是武官，刚从银河边境回来休息..","希望下次还能见到你...:)"},
{"J",	"\xc7\xe0\xc4\xea\xcb\xa7\xce\xe4\xb9\xd9",	  0,	 0,	 0,	  0,"\xc4\xe3\xba\xc3\xa3\xac\xce\xd2\xca\xc7\xce\xe4\xb9\xd9\xa3\xac\xb8\xd5\xb4\xd3\xd2\xf8\xba\xd3\xb1\xdf\xbe\xb3\xbb\xd8\xc0\xb4\xd0\xdd\xcf\xa2..","\xcf\xa3\xcd\xfb\xcf\xc2\xb4\xce\xbb\xb9\xc4\xdc\xbc\xfb\xb5\xbd\xc4\xe3...:)"},
//NULL,		NULL,NULL,    NULL,    NULL,NULL,NULL
//{NULL,			0,	0,	0,	0, NULL, NULL}
};

int pip_see_doctor()	/*看医生*/
{
    char buf[256];
    long savemoney;
    savemoney=d.sick*25;
    if(d.sick<=0)
    {
    //% pressanykey("哇哩..没病来医院干嘛..被骂了..呜~~");
    pressanykey("\xcd\xdb\xc1\xa8..\xc3\xbb\xb2\xa1\xc0\xb4\xd2\xbd\xd4\xba\xb8\xc9\xc2\xef..\xb1\xbb\xc2\xee\xc1\xcb..\xce\xd8~~");
    d.character-=(rand()%3+1);
    if(d.character<0)
      d.character=0;
    d.happy-=(rand()%3+3);
    d.satisfy-=rand()%3+2;
    }    
    else if(d.money < savemoney)
    {
     //% sprintf(buf,"你的病要花 %d 元喔....你不够钱啦...",savemoney);    
     sprintf(buf,"\xc4\xe3\xb5\xc4\xb2\xa1\xd2\xaa\xbb\xa8 %d \xd4\xaa\xe0\xb8....\xc4\xe3\xb2\xbb\xb9\xbb\xc7\xae\xc0\xb2...",savemoney);    
     pressanykey(buf);
    }
    else if(d.sick>0 && d.money >=savemoney)
    {
    d.tired-=rand()%10+20;
    if(d.tired<0)
       d.tired=0;
    d.sick=0;
    d.money=d.money-savemoney;
    move(4,0);
    show_special_pic(1);
    //% pressanykey("药到病除..没有副作用!!");
    pressanykey("\xd2\xa9\xb5\xbd\xb2\xa1\xb3\xfd..\xc3\xbb\xd3\xd0\xb8\xb1\xd7\xf7\xd3\xc3!!");
    }
    return 0;
}

/*减肥*/
int pip_change_weight()
{
    char genbuf[5];
    char inbuf[256];
    int weightmp;
    
    move(b_lines-1, 0);
    clrtoeol();
    show_special_pic(2);
#ifdef MAPLE
    //% getdata(b_lines-1,1, "你的选择是? [Q]离开:", genbuf, 2, 1, 0);    
    getdata(b_lines-1,1, "\xc4\xe3\xb5\xc4\xd1\xa1\xd4\xf1\xca\xc7? [Q]\xc0\xeb\xbf\xaa:", genbuf, 2, 1, 0);    
#else
    //% getdata(b_lines-1,1, "你的选择是? [Q]离开:", genbuf, 2, DOECHO, YEA);
    getdata(b_lines-1,1, "\xc4\xe3\xb5\xc4\xd1\xa1\xd4\xf1\xca\xc7? [Q]\xc0\xeb\xbf\xaa:", genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
    if (genbuf[0]=='1'|| genbuf[0]=='2'|| genbuf[0]=='3'|| genbuf[0]=='4')
    { 
      switch(genbuf[0])
      {
        case '1':
          if(d.money<80)
          {
            //% pressanykey("传统增胖要80元喔....你不够钱啦...");
            pressanykey("\xb4\xab\xcd\xb3\xd4\xf6\xc5\xd6\xd2\xaa80\xd4\xaa\xe0\xb8....\xc4\xe3\xb2\xbb\xb9\xbb\xc7\xae\xc0\xb2...");
          }
          else
          {
#ifdef MAPLE
            //% getdata(b_lines-1,1, "需花费80元(3～5公斤)，你确定吗? [y/N]", genbuf, 2, 1, 0);
            getdata(b_lines-1,1, "\xd0\xe8\xbb\xa8\xb7\xd180\xd4\xaa(3\xa1\xab5\xb9\xab\xbd\xef)\xa3\xac\xc4\xe3\xc8\xb7\xb6\xa8\xc2\xf0? [y/N]", genbuf, 2, 1, 0);
#else
            //% getdata(b_lines-1,1, "需花费80元(3～5公斤)，你确定吗? [y/N]", genbuf, 2, DOECHO, YEA);
            getdata(b_lines-1,1, "\xd0\xe8\xbb\xa8\xb7\xd180\xd4\xaa(3\xa1\xab5\xb9\xab\xbd\xef)\xa3\xac\xc4\xe3\xc8\xb7\xb6\xa8\xc2\xf0? [y/N]", genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
            if(genbuf[0]=='Y' || genbuf[0]=='y')
            {
              weightmp=3+rand()%3;
              d.weight+=weightmp;
              d.money-=80;
              d.maxhp-=rand()%2;
              d.hp-=rand()%2+3;
              show_special_pic(3);
              //% sprintf(inbuf, "总共增加了%d公斤",weightmp);
              sprintf(inbuf, "\xd7\xdc\xb9\xb2\xd4\xf6\xbc\xd3\xc1\xcb%d\xb9\xab\xbd\xef",weightmp);
              pressanykey(inbuf);
            }
            else
            {
              //% pressanykey("回心转意罗.....");
              pressanykey("\xbb\xd8\xd0\xc4\xd7\xaa\xd2\xe2\xc2\xde.....");
            }
          }
          break;
          
        case '2':
#ifdef MAPLE
          //% getdata(b_lines-1,1, "增一公斤要30元，你要增多少公斤呢? [请填数字]:", genbuf, 4, 1, 0);
          getdata(b_lines-1,1, "\xd4\xf6\xd2\xbb\xb9\xab\xbd\xef\xd2\xaa30\xd4\xaa\xa3\xac\xc4\xe3\xd2\xaa\xd4\xf6\xb6\xe0\xc9\xd9\xb9\xab\xbd\xef\xc4\xd8? [\xc7\xeb\xcc\xee\xca\xfd\xd7\xd6]:", genbuf, 4, 1, 0);
#else
          //% getdata(b_lines-1,1, "增一公斤要30元，你要增多少公斤呢? [请填数字]:", genbuf, 4, DOECHO, YEA);
          getdata(b_lines-1,1, "\xd4\xf6\xd2\xbb\xb9\xab\xbd\xef\xd2\xaa30\xd4\xaa\xa3\xac\xc4\xe3\xd2\xaa\xd4\xf6\xb6\xe0\xc9\xd9\xb9\xab\xbd\xef\xc4\xd8? [\xc7\xeb\xcc\xee\xca\xfd\xd7\xd6]:", genbuf, 4, DOECHO, YEA);
#endif  // END MAPLE
          weightmp=atoi(genbuf);
          if(weightmp<=0)
          {
            //% pressanykey("输入有误..放弃罗...");          
            pressanykey("\xca\xe4\xc8\xeb\xd3\xd0\xce\xf3..\xb7\xc5\xc6\xfa\xc2\xde...");          
          }
          else if(d.money>(weightmp*30))
          {
            //% sprintf(inbuf, "增加%d公斤，总共需花费了%d元，确定吗? [y/N]",weightmp,weightmp*30);
            sprintf(inbuf, "\xd4\xf6\xbc\xd3%d\xb9\xab\xbd\xef\xa3\xac\xd7\xdc\xb9\xb2\xd0\xe8\xbb\xa8\xb7\xd1\xc1\xcb%d\xd4\xaa\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0? [y/N]",weightmp,weightmp*30);
#ifdef MAPLE
            getdata(b_lines-1,1,inbuf, genbuf, 2, 1, 0);
#else
            getdata(b_lines-1,1,inbuf, genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
            if(genbuf[0]=='Y' || genbuf[0]=='y')
            {
                d.money-=weightmp*30;
                d.weight+=weightmp;
                d.maxhp-=(rand()%2+2);
                count_tired(5,8,"N",100,1);
                d.hp-=(rand()%2+3);
                d.sick+=rand()%10+5;
                show_special_pic(3);
                //% sprintf(inbuf, "总共增加了%d公斤",weightmp);
                sprintf(inbuf, "\xd7\xdc\xb9\xb2\xd4\xf6\xbc\xd3\xc1\xcb%d\xb9\xab\xbd\xef",weightmp);
                pressanykey(inbuf);
            }
            else
            {
              //% pressanykey("回心转意罗.....");
              pressanykey("\xbb\xd8\xd0\xc4\xd7\xaa\xd2\xe2\xc2\xde.....");
            }
          }
          else
          {
            //% pressanykey("你钱没那麽多啦.......");            
            pressanykey("\xc4\xe3\xc7\xae\xc3\xbb\xc4\xc7\xf7\xe1\xb6\xe0\xc0\xb2.......");            
          }
          break;        
          
        case '3':
          if(d.money<80)
          {
            //% pressanykey("传统减肥要80元喔....你不够钱啦...");
            pressanykey("\xb4\xab\xcd\xb3\xbc\xf5\xb7\xca\xd2\xaa80\xd4\xaa\xe0\xb8....\xc4\xe3\xb2\xbb\xb9\xbb\xc7\xae\xc0\xb2...");
          }
          else
          {
#ifdef MAPLE
            //% getdata(b_lines-1,1, "需花费80元(3～5公斤)，你确定吗? [y/N]", genbuf, 2, 1, 0);
            getdata(b_lines-1,1, "\xd0\xe8\xbb\xa8\xb7\xd180\xd4\xaa(3\xa1\xab5\xb9\xab\xbd\xef)\xa3\xac\xc4\xe3\xc8\xb7\xb6\xa8\xc2\xf0? [y/N]", genbuf, 2, 1, 0);
#else
            //% getdata(b_lines-1,1, "需花费80元(3～5公斤)，你确定吗? [y/N]", genbuf, 2, DOECHO, YEA);
            getdata(b_lines-1,1, "\xd0\xe8\xbb\xa8\xb7\xd180\xd4\xaa(3\xa1\xab5\xb9\xab\xbd\xef)\xa3\xac\xc4\xe3\xc8\xb7\xb6\xa8\xc2\xf0? [y/N]", genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
            if(genbuf[0]=='Y' || genbuf[0]=='y')
            {
              weightmp=3+rand()%3;
              d.weight-=weightmp;
              if(d.weight<0)
                   d.weight=0;
              d.money-=100;
              d.maxhp+=rand()%2;
              d.hp-=rand()%2+3;
              show_special_pic(4);
              //% sprintf(inbuf, "总共减少了%d公斤",weightmp);
              sprintf(inbuf, "\xd7\xdc\xb9\xb2\xbc\xf5\xc9\xd9\xc1\xcb%d\xb9\xab\xbd\xef",weightmp);
              pressanykey(inbuf);
            }
            else
            {
              //% pressanykey("回心转意罗.....");
              pressanykey("\xbb\xd8\xd0\xc4\xd7\xaa\xd2\xe2\xc2\xde.....");
            }
          }        
          break;
        case '4':
#ifdef MAPLE
          //% getdata(b_lines-1,1, "减一公斤要30元，你要减多少公斤呢? [请填数字]:", genbuf, 4, 1, 0);
          getdata(b_lines-1,1, "\xbc\xf5\xd2\xbb\xb9\xab\xbd\xef\xd2\xaa30\xd4\xaa\xa3\xac\xc4\xe3\xd2\xaa\xbc\xf5\xb6\xe0\xc9\xd9\xb9\xab\xbd\xef\xc4\xd8? [\xc7\xeb\xcc\xee\xca\xfd\xd7\xd6]:", genbuf, 4, 1, 0);
#else
          //% getdata(b_lines-1,1, "减一公斤要30元，你要减多少公斤呢? [请填数字]:", genbuf, 4, DOECHO, YEA);
          getdata(b_lines-1,1, "\xbc\xf5\xd2\xbb\xb9\xab\xbd\xef\xd2\xaa30\xd4\xaa\xa3\xac\xc4\xe3\xd2\xaa\xbc\xf5\xb6\xe0\xc9\xd9\xb9\xab\xbd\xef\xc4\xd8? [\xc7\xeb\xcc\xee\xca\xfd\xd7\xd6]:", genbuf, 4, DOECHO, YEA);
#endif  // END MAPLE
          weightmp=atoi(genbuf);
          if(weightmp<=0)
          {
            //% pressanykey("输入有误..放弃罗...");
            pressanykey("\xca\xe4\xc8\xeb\xd3\xd0\xce\xf3..\xb7\xc5\xc6\xfa\xc2\xde...");
          }          
          else if(d.weight<=weightmp)
          {
            //% pressanykey("你没那麽重喔.....");
            pressanykey("\xc4\xe3\xc3\xbb\xc4\xc7\xf7\xe1\xd6\xd8\xe0\xb8.....");
          }
          else if(d.money>(weightmp*30))
          {
            //% sprintf(inbuf, "减少%d公斤，总共需花费了%d元，确定吗? [y/N]",weightmp,weightmp*30);
            sprintf(inbuf, "\xbc\xf5\xc9\xd9%d\xb9\xab\xbd\xef\xa3\xac\xd7\xdc\xb9\xb2\xd0\xe8\xbb\xa8\xb7\xd1\xc1\xcb%d\xd4\xaa\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0? [y/N]",weightmp,weightmp*30);
#ifdef MAPLE
            getdata(b_lines-1,1,inbuf, genbuf, 2, 1, 0);
#else
            getdata(b_lines-1,1,inbuf, genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
            if(genbuf[0]=='Y' || genbuf[0]=='y')
            {
                d.money-=weightmp*30;
                d.weight-=weightmp;
                d.maxhp-=(rand()%2+2);
                count_tired(5,8,"N",100,1);
                d.hp-=(rand()%2+3);
                d.sick+=rand()%10+5;
                show_special_pic(4);
                //% sprintf(inbuf, "总共减少了%d公斤",weightmp);
                sprintf(inbuf, "\xd7\xdc\xb9\xb2\xbc\xf5\xc9\xd9\xc1\xcb%d\xb9\xab\xbd\xef",weightmp);
                pressanykey(inbuf);
            }
            else
            {
              //% pressanykey("回心转意罗.....");
              pressanykey("\xbb\xd8\xd0\xc4\xd7\xaa\xd2\xe2\xc2\xde.....");
            }
          }
          else
          {
            //% pressanykey("你钱没那麽多啦.......");            
            pressanykey("\xc4\xe3\xc7\xae\xc3\xbb\xc4\xc7\xf7\xe1\xb6\xe0\xc0\xb2.......");            
          }
          break;
      }
    }
    return 0;
}


/*参见*/

int
pip_go_palace()
{
  pip_go_palace_screen(royallist);
  return 0;
}

int
pip_go_palace_screen(p) 
struct royalset *p;
{
  int n;
  int a;
  int b;
  int choice;
  int prince;  /*王子会不会出现*/
  int pipkey;
  int change;
  char buf[256];
  char inbuf1[20];
  char inbuf2[20];
  char ans[5];
  //% char *needmode[3]={"      ","礼仪表现＞","谈吐技巧＞"};
  char *needmode[3]={"      ","\xc0\xf1\xd2\xc7\xb1\xed\xcf\xd6\xa3\xbe","\xcc\xb8\xcd\xc2\xbc\xbc\xc7\xc9\xa3\xbe"};
  int save[11]={0,0,0,0,0,0,0,0,0,0,0};

  d.nodone=0; 
  do
  {
  screen_clear();
  show_palace_pic(0);
  move(13,4);
  //% sprintf(buf,"[1;31m┌——————┤[37;41m 来到总司令部了  请选择你欲拜访的对象 [0;1;31m├——————┐[0m");
  sprintf(buf,"[1;31m\xa9\xb0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8[37;41m \xc0\xb4\xb5\xbd\xd7\xdc\xcb\xbe\xc1\xee\xb2\xbf\xc1\xcb  \xc7\xeb\xd1\xa1\xd4\xf1\xc4\xe3\xd3\xfb\xb0\xdd\xb7\xc3\xb5\xc4\xb6\xd4\xcf\xf3 [0;1;31m\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xb4[0m");
  prints(buf);
  move(14,4);
  //% sprintf(buf,"[1;31m│                                                                  │[0m");
  sprintf(buf,"[1;31m\xa9\xa6                                                                  \xa9\xa6[0m");
  prints(buf);

  for(n=0;n<5;n++)  
  {
    a=2*n+1;
    b=2*n+2;
    move(15+n,4);
    sprintf(inbuf1,"%-10s%3d",needmode[p[a].needmode],p[a].needvalue);
    if(n==4)
    { 
      sprintf(inbuf2,"%-10s",needmode[p[b].needmode]);
    }
    else
    {
      sprintf(inbuf2,"%-10s%3d",needmode[p[b].needmode],p[b].needvalue);
    }
    if((d.seeroyalJ==1 && n==4)||(n!=4))
      //% sprintf(buf,"[1;31m│ [36m([37m%s[36m) [33m%-10s  [37m%-14s     [36m([37m%s[36m) [33m%-10s  [37m%-14s[31m│[0m",    
      sprintf(buf,"[1;31m\xa9\xa6 [36m([37m%s[36m) [33m%-10s  [37m%-14s     [36m([37m%s[36m) [33m%-10s  [37m%-14s[31m\xa9\xa6[0m",    
             p[a].num,p[a].name,inbuf1,p[b].num,p[b].name,inbuf2);
    else
      //% sprintf(buf,"[1;31m│ [36m([37m%s[36m) [33m%-10s  [37m%-14s                                   [31m│[0m",    
      sprintf(buf,"[1;31m\xa9\xa6 [36m([37m%s[36m) [33m%-10s  [37m%-14s                                   [31m\xa9\xa6[0m",    
             p[a].num,p[a].name,inbuf1);             
    prints(buf);
  }
  move(20,4);
  //% sprintf(buf,"[1;31m│                                                                  │[0m");
  sprintf(buf,"[1;31m\xa9\xa6                                                                  \xa9\xa6[0m");
  prints(buf);
  move(21,4);
  //% sprintf(buf,"[1;31m└—————————————————————————————————┘[0m");
  sprintf(buf,"[1;31m\xa9\xb8\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xbc[0m");
  prints(buf);
  

   if(d.death==1 || d.death==2 || d.death==3)
     return 0;    
  /*将各人务已经给与的数值叫回来*/
   save[1]=d.royalA;            /*from守卫*/
   save[2]=d.royalB;            /*from近卫*/
   save[3]=d.royalC;		/*from将军*/
   save[4]=d.royalD;            /*from大臣*/
   save[5]=d.royalE;            /*from祭司*/
   save[6]=d.royalF;            /*from宠妃*/
   save[7]=d.royalG;            /*from王妃*/
   save[8]=d.royalH;            /*from国王*/
   save[9]=d.royalI;            /*from小丑*/
   save[10]=d.royalJ;           /*from王子*/

   move(b_lines-1, 0);
   clrtoeol();
   move(b_lines-1,0);
   //% prints("[1;33m [生命力] %d/%d  [疲劳度] %d [0m",d.hp,d.maxhp,d.tired);
   prints("[1;33m [\xc9\xfa\xc3\xfc\xc1\xa6] %d/%d  [\xc6\xa3\xc0\xcd\xb6\xc8] %d [0m",d.hp,d.maxhp,d.tired);
             
   move(b_lines, 0);
   clrtoeol();
   move(b_lines,0);
   prints(
   //% "[1;37;46m  参见选单  [44m [字母]选择欲拜访的人物  [Q]离开星空总司令部：                    [0m");
   "[1;37;46m  \xb2\xce\xbc\xfb\xd1\xa1\xb5\xa5  [44m [\xd7\xd6\xc4\xb8]\xd1\xa1\xd4\xf1\xd3\xfb\xb0\xdd\xb7\xc3\xb5\xc4\xc8\xcb\xce\xef  [Q]\xc0\xeb\xbf\xaa\xd0\xc7\xbf\xd5\xd7\xdc\xcb\xbe\xc1\xee\xb2\xbf\xa3\xba                    [0m");
   pipkey=egetch();
   choice=pipkey-64;
   if(choice<1 || choice>10)
      choice=pipkey-96;

   if((choice>=1 && choice<=10 && d.seeroyalJ==1)||(choice>=1 && choice<=9 && d.seeroyalJ==0))
   {
    d.social+=rand()%3+3;
    d.hp-=rand()%5+6;
    d.tired+=rand()%5+8;
    if(d.tired>=100)
    {
       d.death=1;
       //% pipdie("[1;31m累死了...[m  ",1);
       pipdie("[1;31m\xc0\xdb\xcb\xc0\xc1\xcb...[m  ",1);
    }
    if(d.hp<0)
    {
       d.death=1;
       //% pipdie("[1;31m饿死了...[m  ",1);
       pipdie("[1;31m\xb6\xf6\xcb\xc0\xc1\xcb...[m  ",1);
    }
    if(d.death==1)
    {
      //% sprintf(buf,"掰掰了...真是悲情..");
      sprintf(buf,"\xea\xfe\xea\xfe\xc1\xcb...\xd5\xe6\xca\xc7\xb1\xaf\xc7\xe9..");
    }
    else
    {
    if((p[choice].needmode==0)||
       (p[choice].needmode==1 && d.manners>=p[choice].needvalue)||
       (p[choice].needmode==2 && d.speech>=p[choice].needvalue))    
    {
      if(choice>=1 && choice<=9 && save[choice] >= p[choice].maxtoman)
      {
        if(rand()%2>0)
            //% sprintf(buf,"能和这麽伟大的你讲话真是荣幸ㄚ...");
            sprintf(buf,"\xc4\xdc\xba\xcd\xd5\xe2\xf7\xe1\xce\xb0\xb4\xf3\xb5\xc4\xc4\xe3\xbd\xb2\xbb\xb0\xd5\xe6\xca\xc7\xc8\xd9\xd0\xd2\xa8\xda...");
        else
            //% sprintf(buf,"很高兴你来拜访我，但我不能给你什麽了..");
            sprintf(buf,"\xba\xdc\xb8\xdf\xd0\xcb\xc4\xe3\xc0\xb4\xb0\xdd\xb7\xc3\xce\xd2\xa3\xac\xb5\xab\xce\xd2\xb2\xbb\xc4\xdc\xb8\xf8\xc4\xe3\xca\xb2\xf7\xe1\xc1\xcb..");
      }
      else
      {
        change=0;
        if(choice>=1 && choice <=8 )
        {
          switch(choice)
          {
            case 1:
              change=d.character/5;
              break;
            case 2:
              change=d.character/8;
              break;
            case 3:
              change=d.charm/5;
              break;              
            case 4:
              change=d.wisdom/10;
              break;
            case 5:
              change=d.belief/10;
              break;
            case 6:
              change=d.speech/10;
              break;
            case 7:
              change=d.social/10;
              break;
            case 8:
              change=d.hexp/10;
              break;            
          }
          /*如果大於每次的增加最大量*/
          if(change > p[choice].addtoman)
             change=p[choice].addtoman;
          /*如果加上原先的之後大於所能给的所有值时*/
          if((change+save[choice])>= p[choice].maxtoman)
             change=p[choice].maxtoman-save[choice];
          save[choice]+=change;
          d.toman+=change;
        }
        else if(choice==9)
        {
          save[9]=0;
          d.social-=13+rand()%4;
          d.affect+=13+rand()%4;
        }
        else if(choice==10 && d.seeroyalJ==1)
        {
            save[10]+=15+rand()%4;
            d.seeroyalJ=0;
        }
        if(rand()%2>0)
            sprintf(buf,"%s",p[choice].words1);
        else    
            sprintf(buf,"%s",p[choice].words2);
      }
    }
    else
    {
      if(rand()%2>0)
            //% sprintf(buf,"我不和你这样的鸡谈话....");
            sprintf(buf,"\xce\xd2\xb2\xbb\xba\xcd\xc4\xe3\xd5\xe2\xd1\xf9\xb5\xc4\xbc\xa6\xcc\xb8\xbb\xb0....");
      else
            //% sprintf(buf,"你这只没教养的鸡，再去学学礼仪吧....");    
            sprintf(buf,"\xc4\xe3\xd5\xe2\xd6\xbb\xc3\xbb\xbd\xcc\xd1\xf8\xb5\xc4\xbc\xa6\xa3\xac\xd4\xd9\xc8\xa5\xd1\xa7\xd1\xa7\xc0\xf1\xd2\xc7\xb0\xc9....");    
    
    }
    }    
    pressanykey(buf);
   }
   d.royalA=save[1];
   d.royalB=save[2];
   d.royalC=save[3];
   d.royalD=save[4];
   d.royalE=save[5];
   d.royalF=save[6];
   d.royalG=save[7];
   d.royalH=save[8];
   d.royalI=save[9];
   d.royalJ=save[10];
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));

  //% pressanykey("离开星空总司令部.....");  
  pressanykey("\xc0\xeb\xbf\xaa\xd0\xc7\xbf\xd5\xd7\xdc\xcb\xbe\xc1\xee\xb2\xbf.....");  
  return 0;
}

int pip_query()  /*拜访小鸡*/
{

#ifdef MAPLE
  userec muser;
#endif  // END MAPLE
  int id;
  char genbuf[STRLEN];

#ifndef MAPLE
  char *msg_uid = MSG_UID;
  char *err_uid = ERR_UID;
#endif  // END MAPLE

  //% stand_title("拜访同伴");
  stand_title("\xb0\xdd\xb7\xc3\xcd\xac\xb0\xe9");
  usercomplete(msg_uid, genbuf);
  if (genbuf[0])
  {
    move(2, 0);
    if (id = getuser(genbuf))
    {
        pip_read(genbuf);
        //% pressanykey("观摩一下别人的小鸡...:p");
        pressanykey("\xb9\xdb\xc4\xa6\xd2\xbb\xcf\xc2\xb1\xf0\xc8\xcb\xb5\xc4\xd0\xa1\xbc\xa6...:p");
    }
    else
    {
      outs(err_uid);
      clrtoeol();
    }
  }
  return 0;
}

int
pip_read(genbuf)
char *genbuf;
{
  FILE *fs;
  struct chicken d1;
  char buf[200];
  //% /*char yo[14][5]={"诞生","婴儿","幼儿","儿童","青年","少年","成年",
  /*char yo[14][5]={"\xb5\xae\xc9\xfa","\xd3\xa4\xb6\xf9","\xd3\xd7\xb6\xf9","\xb6\xf9\xcd\xaf","\xc7\xe0\xc4\xea","\xc9\xd9\xc4\xea","\xb3\xc9\xc4\xea",
                  //% "壮年","壮年","壮年","更年","老年","老年","古稀"};*/
                  "\xd7\xb3\xc4\xea","\xd7\xb3\xc4\xea","\xd7\xb3\xc4\xea","\xb8\xfc\xc4\xea","\xc0\xcf\xc4\xea","\xc0\xcf\xc4\xea","\xb9\xc5\xcf\xa1"};*/
  //% char yo[12][5]={"诞生","婴儿","幼儿","儿童","少年","青年",
  char yo[12][5]={"\xb5\xae\xc9\xfa","\xd3\xa4\xb6\xf9","\xd3\xd7\xb6\xf9","\xb6\xf9\xcd\xaf","\xc9\xd9\xc4\xea","\xc7\xe0\xc4\xea",
                  //% "成年","壮年","更年","老年","古稀","神仙"};                  
                  "\xb3\xc9\xc4\xea","\xd7\xb3\xc4\xea","\xb8\xfc\xc4\xea","\xc0\xcf\xc4\xea","\xb9\xc5\xcf\xa1","\xc9\xf1\xcf\xc9"};                  
  int pc1,age1,age=0;
  
  int year1,month1,day1,sex1,death1,nodone1,relation1,liveagain1,dataB1,dataC1,dataD1,dataE1;
  int hp1,maxhp1,weight1,tired1,sick1,shit1,wrist1,bodyA1,bodyB1,bodyC1,bodyD1,bodyE1;
  int social1,family1,hexp1,mexp1,tmpA1,tmpB1,tmpC1,tmpD1,tmpE1;
  int mp1,maxmp1,attack1,resist1,speed1,hskill1,mskill1,mresist1,magicmode1,fightB1,fightC1,fightD1,fightE1;
  int weaponhead1,weaponrhand1,weaponlhand1,weaponbody1,weaponfoot1,weaponA1,weaponB1,weaponC1,weaponD1,weaponE1;
  int toman1,character1,love1,wisdom1,art1,etchics1,brave1,homework1,charm1,manners1,speech1,cookskill1,learnA1,learnB1,learnC1,learnD1,learnE1;
  int happy1,satisfy1,fallinlove1,belief1,offense1,affect1,stateA1,stateB1,stateC1,stateD1,stateE1;
  int food1,medicine1,bighp1,cookie1,ginseng1,snowgrass1,eatC1,eatD1,eatE1;
  int book1,playtool1,money1,thingA1,thingB1,thingC1,thingD1,thingE1;
  int winn1,losee1;
  int royalA1,royalB1,royalC1,royalD1,royalE1,royalF1,royalG1,royalH1,royalI1,royalJ1,seeroyalJ1,seeA1,seeB1,seeC1,seeD1,seeE1;
  int wantend1,lover1;
  char name1[200];
  int classA1,classB1,classC1,classD1,classE1;
  int classF1,classG1,classH1,classI1,classJ1;
  int classK1,classL1,classM1,classN1,classO1;
  int workA1,workB1,workC1,workD1,workE1;
  int workF1,workG1,workH1,workI1,workJ1;
  int workK1,workL1,workM1,workN1,workO1;
  int workP1,workQ1,workR1,workS1,workT1;
  int workU1,workV1,workW1,workX1,workY1,workZ1;

#ifdef MAPLE
  sprintf(buf,"home/%s/new_chicken",genbuf);
  currutmp->destuid = genbuf;
#else
  sprintf(buf,"home/%c/%s/new_chicken",toupper(genbuf[0]),genbuf);
#endif  // END MAPLE

  if(fs=fopen(buf,"r"))
  {
    fread(&d1,sizeof(d1),1,fs);
    //fgets(buf, 80, fs);
    //age = ((time_t) atol(buf))/60/30;
	age = d1.bbtime/1800;
  
    if(age==0) /*诞生*/
       age1=0;
    else if( age==1) /*婴儿*/
       age1=1;
    else if( age>=2 && age<=5 ) /*幼儿*/
       age1=2;
    else if( age>=6 && age<=12 ) /*儿童*/
       age1=3;
    else if( age>=13 && age<=15 ) /*少年*/
       age1=4;     
    else if( age>=16 && age<=18 ) /*青年*/
       age1=5;     
    else if( age>=19 && age<=35 ) /*成年*/
       age1=6;
    else if( age>=36 && age<=45 ) /*壮年*/
       age1=7;
    else if( age>=45 && age<=60 ) /*更年*/
       age1=8;
    else if( age>=60 && age<=70 ) /*老年*/
       age1=9;
    else if( age>=70 && age<=100 ) /*古稀*/
       age1=10;
    else if( age>100 ) /*神仙*/
       age1=11;
/*
    fscanf(fs,
    "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
    &(year1),&(month1),&(day1),&(sex1),&(death1),&(nodone1),&(relation1),&(liveagain1),&(dataB1),&(dataC1),&(dataD1),&(dataE1),
    &(hp1),&(maxhp1),&(weight1),&(tired1),&(sick1),&(shit1),&(wrist1),&(bodyA1),&(bodyB1),&(bodyC1),&(bodyD1),&(bodyE1),
    &(social1),&(family1),&(hexp1),&(mexp1),&(tmpA1),&(tmpB1),&(tmpC1),&(tmpD1),&(tmpE1),
    &(mp1),&(maxmp1),&(attack1),&(resist1),&(speed1),&(hskill1),&(mskill1),&(mresist1),&(magicmode1),&(fightB1),&(fightC1),&(fightD1),&(fightE1),
    &(weaponhead1),&(weaponrhand1),&(weaponlhand1),&(weaponbody1),&(weaponfoot1),&(weaponA1),&(weaponB1),&(weaponC1),&(weaponD1),&(weaponE1),
    &(toman1),&(character1),&(love1),&(wisdom1),&(art1),&(etchics1),&(brave1),&(homework1),&(charm1),&(manners1),&(speech1),&(cookskill1),&(learnA1),&(learnB1),&(learnC1),&(learnD1),&(learnE1),
    &(happy1),&(satisfy1),&(fallinlove1),&(belief1),&(offense1),&(affect1),&(stateA1),&(stateB1),&(stateC1),&(stateD1),&(stateE1),
    &(food1),&(medicine1),&(bighp1),&(cookie1),&(ginseng1),&(snowgrass1),&(eatC1),&(eatD1),&(eatE1),
    &(book1),&(playtool1),&(money1),&(thingA1),&(thingB1),&(thingC1),&(thingD1),&(thingE1),
    &(winn1),&(losee1),
    &(royalA1),&(royalB1),&(royalC1),&(royalD1),&(royalE1),&(royalF1),&(royalG1),&(royalH1),&(royalI1),&(royalJ1),&(seeroyalJ1),&(seeA1),&(seeB1),&(seeC1),&(seeD1),&(seeE1),
    &(wantend1),&(lover1),
    name1,
    &(classA1),&(classB1),&(classC1),&(classD1),&(classE1),
    &(classF1),&(classG1),&(classH1),&(classI1),&(classJ1),
    &(classK1),&(classL1),&(classM1),&(classN1),&(classO1),
    &(workA1),&(workB1),&(workC1),&(workD1),&(workE1),
    &(workF1),&(workG1),&(workH1),&(workI1),&(workJ1),
    &(workK1),&(workL1),&(workM1),&(workN1),&(workO1),
    &(workP1),&(workQ1),&(workR1),&(workS1),&(workT1),
    &(workU1),&(workV1),&(workW1),&(workX1),&(workY1),&(workZ1)
  );
*/
  fclose(fs);

  move(1,0);
  screen_clrtobot();
#ifdef MAPLE
  //% prints("这是%s养的小鸡：\n",xuser.userid);
  prints("\xd5\xe2\xca\xc7%s\xd1\xf8\xb5\xc4\xd0\xa1\xbc\xa6\xa3\xba\n",xuser.userid);
#else
  //% prints("这是%s养的小鸡：\n",genbuf);
  prints("\xd5\xe2\xca\xc7%s\xd1\xf8\xb5\xc4\xd0\xa1\xbc\xa6\xa3\xba\n",genbuf);
#endif  // END MAPLE

  if (d1.death==0)
  {
   //% prints("[1;32mName：%-10s[m  生日：%02d年%02d月%2d日   年龄：%2d岁  状态：%s  钱钱：%d\n"
   prints("[1;32mName\xa3\xba%-10s[m  \xc9\xfa\xc8\xd5\xa3\xba%02d\xc4\xea%02d\xd4\xc2%2d\xc8\xd5   \xc4\xea\xc1\xe4\xa3\xba%2d\xcb\xea  \xd7\xb4\xcc\xac\xa3\xba%s  \xc7\xae\xc7\xae\xa3\xba%d\n"
          //% "生命：%3d/%-3d  快乐：%-4d  满意：%-4d  气质：%-4d  智慧：%-4d  体重：%-4d\n"
          "\xc9\xfa\xc3\xfc\xa3\xba%3d/%-3d  \xbf\xec\xc0\xd6\xa3\xba%-4d  \xc2\xfa\xd2\xe2\xa3\xba%-4d  \xc6\xf8\xd6\xca\xa3\xba%-4d  \xd6\xc7\xbb\xdb\xa3\xba%-4d  \xcc\xe5\xd6\xd8\xa3\xba%-4d\n"
          //% "大补丸：%-4d   食物：%-4d  零食：%-4d  疲劳：%-4d  脏脏：%-4d  病气：%-4d\n",
          "\xb4\xf3\xb2\xb9\xcd\xe8\xa3\xba%-4d   \xca\xb3\xce\xef\xa3\xba%-4d  \xc1\xe3\xca\xb3\xa3\xba%-4d  \xc6\xa3\xc0\xcd\xa3\xba%-4d  \xd4\xe0\xd4\xe0\xa3\xba%-4d  \xb2\xa1\xc6\xf8\xa3\xba%-4d\n",
        d1.name,d1.year,d1.month,d1.day,age,yo[age1],d1.money,
        d1.hp,d1.maxhp,d1.happy,d1.satisfy,d1.character,d1.wisdom,d1.weight,
        d1.bighp,d1.food,d1.cookie,d1.tired,d1.shit,d1.sick);

    move(5,0);
    switch(age1)
    {
     case 0:       
     case 1:
     case 2:
       if(d1.weight<=(60+10*age-30))
          show_basic_pic(1);
       else if(d1.weight>(60+10*age-30) && d1.weight<(60+10*age+30))
          show_basic_pic(2);
       else if(d1.weight>=(60+10*age+30))
          show_basic_pic(3);
       break;
     case 3:
     case 4:
       if(d1.weight<=(60+10*age-30))
          show_basic_pic(4);
       else if(d1.weight>(60+10*age-30) && d1.weight<(60+10*age+30))
          show_basic_pic(5);
       else if(d1.weight>=(60+10*age+30))
          show_basic_pic(6);
       break;
     case 5:
     case 6:
       if(d1.weight<=(60+10*age-30))
          show_basic_pic(7);
       else if(d1.weight>(60+10*age-30) && d1.weight<(60+10*age+30))
          show_basic_pic(8);
       else if(d1.weight>=(60+10*age+30))
          show_basic_pic(9);
       break;     
     case 7:
     case 8:
       if(d1.weight<=(60+10*age-30))
          show_basic_pic(10);
       else if(d1.weight>(60+10*age-30) && d1.weight<(60+10*age+30))
          show_basic_pic(11);
       else if(d1.weight>=(60+10*age+30))
          show_basic_pic(12);
       break;     
     case 9:
       show_basic_pic(13);
       break;
     case 10:
     case 11:
       show_basic_pic(13);
       break;
    }
   move(18,0);
   //% if(d1.shit==0) prints("很乾净..");
   if(d1.shit==0) prints("\xba\xdc\xc7\xac\xbe\xbb..");
   //% if(d1.shit>40&&d1.shit<60) prints("臭臭的..");
   if(d1.shit>40&&d1.shit<60) prints("\xb3\xf4\xb3\xf4\xb5\xc4..");
   //% if(d1.shit>=60&&d1.shit<80) prints("好臭喔..");
   if(d1.shit>=60&&d1.shit<80) prints("\xba\xc3\xb3\xf4\xe0\xb8..");
   //% if(d1.shit>=80&&d1.shit<100) prints("[1;34m快臭死了..[m");
   if(d1.shit>=80&&d1.shit<100) prints("[1;34m\xbf\xec\xb3\xf4\xcb\xc0\xc1\xcb..[m");
   //% if(d1.shit>=100) {prints("[1;31m臭死了..[m"); return -1;}
   if(d1.shit>=100) {prints("[1;31m\xb3\xf4\xcb\xc0\xc1\xcb..[m"); return -1;}

   pc1=hp1*100/d1.maxhp;
   //% if(pc1==0) {prints("饿死了.."); return -1;}
   if(pc1==0) {prints("\xb6\xf6\xcb\xc0\xc1\xcb.."); return -1;}
   //% if(pc1<20) prints("[1;35m全身无力中.快饿死了.[m");
   if(pc1<20) prints("[1;35m\xc8\xab\xc9\xed\xce\xde\xc1\xa6\xd6\xd0.\xbf\xec\xb6\xf6\xcb\xc0\xc1\xcb.[m");
   //% if(pc1<40&&pc1>=20) prints("体力不太够..想吃点东西..");
   if(pc1<40&&pc1>=20) prints("\xcc\xe5\xc1\xa6\xb2\xbb\xcc\xab\xb9\xbb..\xcf\xeb\xb3\xd4\xb5\xe3\xb6\xab\xce\xf7..");
   //% if(pc1<100&&pc1>=80) prints("嗯～肚子饱饱有体力..");
   if(pc1<100&&pc1>=80) prints("\xe0\xc5\xa1\xab\xb6\xc7\xd7\xd3\xb1\xa5\xb1\xa5\xd3\xd0\xcc\xe5\xc1\xa6..");
   //% if(pc1>=100) prints("[1;34m快撑死了..[m");
   if(pc1>=100) prints("[1;34m\xbf\xec\xb3\xc5\xcb\xc0\xc1\xcb..[m");

   pc1=d1.tired;
   //% if(pc1<20) prints("精神抖抖中..");
   if(pc1<20) prints("\xbe\xab\xc9\xf1\xb6\xb6\xb6\xb6\xd6\xd0..");
   //% if(pc1<80&&pc1>=60) prints("[1;34m有点小累..[m");
   if(pc1<80&&pc1>=60) prints("[1;34m\xd3\xd0\xb5\xe3\xd0\xa1\xc0\xdb..[m");
   //% if(pc1<100&&pc1>=80) {prints("[1;31m好累喔，快不行了..[m"); }
   if(pc1<100&&pc1>=80) {prints("[1;31m\xba\xc3\xc0\xdb\xe0\xb8\xa3\xac\xbf\xec\xb2\xbb\xd0\xd0\xc1\xcb..[m"); }
   //% if(pc1>=100) {prints("累死了..."); return -1;}
   if(pc1>=100) {prints("\xc0\xdb\xcb\xc0\xc1\xcb..."); return -1;}

   pc1=60+10*age;
   //% if(d1.weight<(pc1+30) && d1.weight>=(pc1+10)) prints("有点小胖..");
   if(d1.weight<(pc1+30) && d1.weight>=(pc1+10)) prints("\xd3\xd0\xb5\xe3\xd0\xa1\xc5\xd6..");
   //% if(d1.weight<(pc1+50) && d1.weight>=(pc1+30)) prints("太胖了..");
   if(d1.weight<(pc1+50) && d1.weight>=(pc1+30)) prints("\xcc\xab\xc5\xd6\xc1\xcb..");
   //% if(d1.weight>(pc1+50)) {prints("胖死了..."); return -1;}
   if(d1.weight>(pc1+50)) {prints("\xc5\xd6\xcb\xc0\xc1\xcb..."); return -1;}

   //% if(d1.weight<(pc1-50)) {prints("瘦死了.."); return -1;}
   if(d1.weight<(pc1-50)) {prints("\xca\xdd\xcb\xc0\xc1\xcb.."); return -1;}
   //% if(d1.weight>(pc1-30) && d1.weight<=(pc1-10)) prints("有点小瘦..");
   if(d1.weight>(pc1-30) && d1.weight<=(pc1-10)) prints("\xd3\xd0\xb5\xe3\xd0\xa1\xca\xdd..");
   //% if(d1.weight>(pc1-50) && d1.weight<=(pc1-30)) prints("太瘦了..");
   if(d1.weight>(pc1-50) && d1.weight<=(pc1-30)) prints("\xcc\xab\xca\xdd\xc1\xcb..");

   //% if(d1.sick<75&&d1.sick>=50) prints("[1;34m生病了..[m");
   if(d1.sick<75&&d1.sick>=50) prints("[1;34m\xc9\xfa\xb2\xa1\xc1\xcb..[m");
   //% if(d1.sick<100&&d1.sick>=75) {prints("[1;31m病重!!..[m"); }
   if(d1.sick<100&&d1.sick>=75) {prints("[1;31m\xb2\xa1\xd6\xd8!!..[m"); }
   //% if(d1.sick>=100) {prints("病死了.!."); return -1;}
   if(d1.sick>=100) {prints("\xb2\xa1\xcb\xc0\xc1\xcb.!."); return -1;}

   pc1=d1.happy;
   //% if(pc1<20) prints("[1;31m很不快乐..[m");
   if(pc1<20) prints("[1;31m\xba\xdc\xb2\xbb\xbf\xec\xc0\xd6..[m");
   //% if(pc1<40&&pc1>=20) prints("不快乐..");
   if(pc1<40&&pc1>=20) prints("\xb2\xbb\xbf\xec\xc0\xd6..");
   //% if(pc1<95&&pc1>=80) prints("快乐..");
   if(pc1<95&&pc1>=80) prints("\xbf\xec\xc0\xd6..");
   //% if(pc1<=100&&pc1>=95) prints("很快乐..");
   if(pc1<=100&&pc1>=95) prints("\xba\xdc\xbf\xec\xc0\xd6..");

   pc1=d1.satisfy;
   //% if(pc1<40) prints("[31;1m不满足..[m");
   if(pc1<40) prints("[31;1m\xb2\xbb\xc2\xfa\xd7\xe3..[m");
   //% if(pc1<95&&pc1>=80) prints("满足..");
   if(pc1<95&&pc1>=80) prints("\xc2\xfa\xd7\xe3..");
   //% if(pc1<=100&&pc1>=95) prints("很满足..");
   if(pc1<=100&&pc1>=95) prints("\xba\xdc\xc2\xfa\xd7\xe3..");
  }
  else if(d1.death==1)
  {
     show_die_pic(2);
     move(14,20);
     //% prints("可怜的小鸡呜呼哀哉了");
     prints("\xbf\xc9\xc1\xaf\xb5\xc4\xd0\xa1\xbc\xa6\xce\xd8\xba\xf4\xb0\xa7\xd4\xd5\xc1\xcb");
  } 
  else if(d1.death==2)
  {
     show_die_pic(3);
  }
  else if(d1.death==3)
  {
    move(5,0);
    //% outs("游戏已经玩到结局罗....");
    outs("\xd3\xce\xcf\xb7\xd2\xd1\xbe\xad\xcd\xe6\xb5\xbd\xbd\xe1\xbe\xd6\xc2\xde....");
  }
  else
  {
    //% pressanykey("档案损毁了....");
    pressanykey("\xb5\xb5\xb0\xb8\xcb\xf0\xbb\xd9\xc1\xcb....");
  }
 }   /* 有养小鸡 */
 else
 {
   move(1,0);
   screen_clrtobot();
   //% pressanykey("这一家的人没有养小鸡......");
   pressanykey("\xd5\xe2\xd2\xbb\xbc\xd2\xb5\xc4\xc8\xcb\xc3\xbb\xd3\xd0\xd1\xf8\xd0\xa1\xbc\xa6......");
 }
}
