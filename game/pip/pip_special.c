/*---------------------------------------------------------------------------*/
/* ÌØÊâÑ¡µ¥:¿´²¡ ¼õ·Ê Õ½¶· °İ·Ã ³¯¼û                                         */
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
{"T",	"°İ·Ã¶ÔÏó",	  0,	0,	 0,	  0,"","" /*NULL,NULL*/},
{"A",	"ĞÇ¿ÕÆï±øÁ¬",	  1,	10,	15,    	100,"ÄãÕæºÃ£¬À´ÅãÎÒÁÄÌì..","ÊØÎÀĞÇ¿ÕµÄ°²È«ÊÇºÜĞÁ¿àµÄ.."},
{"B",	"ĞÇ¿Õ£°£°£·",	  1,   100,	25,	200,"ÕæÊÇÀñÃ²µÄĞ¡¼¦..ÎÒÏ²»¶...","ÌØÎñ¾ÍÊÇÃØÃÜ±£»¤Õ¾³¤°²È«µÄÈË.."},
{"C",	"Õò¹ú´ó½«¾ü",	  1,   200,	30,	250,"¸æËßÄãà¡£¡µ±ÄêÄÇ¸öÕ½ÒÛºÜ¾«²Êà¸..","ÄãÕæÊÇ¸ß¹óÓÅÑÅµÄĞ¡¼¦..."},
{"D",	"²ÎÄ±×ÜÎñ³¤",	  1,   300,	35,	300,"ÎÒ°ïÕ¾³¤¹ÜÀíÕâ¸ö¹ú¼Òà¡..","ÄãµÄÉùÒôºÜºÃÌıÒ®..ÎÒºÜÏ²»¶à¸...:)"},
{"E",	"Ğ¡ÌìÊ¹Õ¾³¤",	  1,   400,	35,	300,"ÄãºÜÓĞ½ÌÑøà¡£¡ºÜ¸ßĞËÈÏÊ¶Äã...","ÓÅÑÅµÄÄã£¬ÇëÈÃÎÒ°ïÄãÆí¸£...."},
{"F",	"·çóİÊÖÕ¾³¤",	  1,   500,	40,	350,"ÄãºÃ¿É°®à¸..ÎÒÏ²»¶Äãà¡....","¶ÔÀ²..ÒÔááÒª¶à¶àÀ´ºÍÎÒÍæà¸..."},
{"G",	"¹ÔĞ¡º¢Õ¾³¤",	  1,   550,	40,	350, "¸úÄã½²»°ºÜ¿ìÀÖà¸..²»ÏñÕ¾³¤Ò»ÑùÎŞÁÄ..","À´£¬×øÎÒÏ¥¸ÇÉÏ£¬ÌıÎÒ½²¹ÊÊÂ.."},
{"H",	"Ğ¡Ã×¿ËÕ¾³¤",	  1,   600,	50,     400,"Ò»Õ¾Ö®³¤ÔğÈÎÖØ´óÑ½..:)..","Ğ»Ğ»ÄãÌıÎÒ½²»°..ÒÔááÒª¶àÀ´à¸..."},
{"I",	"ĞÇ¿Õ¹àË®Èº",	  2,    60,	 0,	  0,"²»´íà¡..Âù»úÁéµÄà¸..ºÜ¿É°®....","À´  ÎÒÃÇÒ»ÆğÀ´¹àË®°É...."},
{"J",	"ÇàÄêË§Îä¹Ù",	  0,	 0,	 0,	  0,"ÄãºÃ£¬ÎÒÊÇÎä¹Ù£¬¸Õ´ÓÒøºÓ±ß¾³»ØÀ´ĞİÏ¢..","Ï£ÍûÏÂ´Î»¹ÄÜ¼ûµ½Äã...:)"},
//NULL,		NULL,NULL,    NULL,    NULL,NULL,NULL
//{NULL,			0,	0,	0,	0, NULL, NULL}
};

int pip_see_doctor()	/*¿´Ò½Éú*/
{
    char buf[256];
    long savemoney;
    savemoney=d.sick*25;
    if(d.sick<=0)
    {
    pressanykey("ÍÛÁ¨..Ã»²¡À´Ò½Ôº¸ÉÂï..±»ÂîÁË..ÎØ~~");
    d.character-=(rand()%3+1);
    if(d.character<0)
      d.character=0;
    d.happy-=(rand()%3+3);
    d.satisfy-=rand()%3+2;
    }    
    else if(d.money < savemoney)
    {
     sprintf(buf,"ÄãµÄ²¡Òª»¨ %d Ôªà¸....Äã²»¹»Ç®À²...",savemoney);    
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
    pressanykey("Ò©µ½²¡³ı..Ã»ÓĞ¸±×÷ÓÃ!!");
    }
    return 0;
}

/*¼õ·Ê*/
int pip_change_weight()
{
    char genbuf[5];
    char inbuf[256];
    int weightmp;
    
    move(b_lines-1, 0);
    clrtoeol();
    show_special_pic(2);
#ifdef MAPLE
    getdata(b_lines-1,1, "ÄãµÄÑ¡ÔñÊÇ? [Q]Àë¿ª:", genbuf, 2, 1, 0);    
#else
    getdata(b_lines-1,1, "ÄãµÄÑ¡ÔñÊÇ? [Q]Àë¿ª:", genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
    if (genbuf[0]=='1'|| genbuf[0]=='2'|| genbuf[0]=='3'|| genbuf[0]=='4')
    { 
      switch(genbuf[0])
      {
        case '1':
          if(d.money<80)
          {
            pressanykey("´«Í³ÔöÅÖÒª80Ôªà¸....Äã²»¹»Ç®À²...");
          }
          else
          {
#ifdef MAPLE
            getdata(b_lines-1,1, "Ğè»¨·Ñ80Ôª(3¡«5¹«½ï)£¬ÄãÈ·¶¨Âğ? [y/N]", genbuf, 2, 1, 0);
#else
            getdata(b_lines-1,1, "Ğè»¨·Ñ80Ôª(3¡«5¹«½ï)£¬ÄãÈ·¶¨Âğ? [y/N]", genbuf, 2, DOECHO, YEA);
#endif  // END MAPLE
            if(genbuf[0]=='Y' || genbuf[0]=='y')
            {
              weightmp=3+rand()%3;
              d.weight+=weightmp;
              d.money-=80;
              d.maxhp-=rand()%2;
              d.hp-=rand()%2+3;
              show_special_pic(3);
              sprintf(inbuf, "×Ü¹²Ôö¼ÓÁË%d¹«½ï",weightmp);
              pressanykey(inbuf);
            }
            else
            {
              pressanykey("»ØĞÄ×ªÒâÂŞ.....");
            }
          }
          break;
          
        case '2':
#ifdef MAPLE
          getdata(b_lines-1,1, "ÔöÒ»¹«½ïÒª30Ôª£¬ÄãÒªÔö¶àÉÙ¹«½ïÄØ? [ÇëÌîÊı×Ö]:", genbuf, 4, 1, 0);
#else
          getdata(b_lines-1,1, "ÔöÒ»¹«½ïÒª30Ôª£¬ÄãÒªÔö¶àÉÙ¹«½ïÄØ? [ÇëÌîÊı×Ö]:", genbuf, 4, DOECHO, YEA);
#endif  // END MAPLE
          weightmp=atoi(genbuf);
          if(weightmp<=0)
          {
            pressanykey("ÊäÈëÓĞÎó..·ÅÆúÂŞ...");          
          }
          else if(d.money>(weightmp*30))
          {
            sprintf(inbuf, "Ôö¼Ó%d¹«½ï£¬×Ü¹²Ğè»¨·ÑÁË%dÔª£¬È·¶¨Âğ? [y/N]",weightmp,weightmp*30);
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
                sprintf(inbuf, "×Ü¹²Ôö¼ÓÁË%d¹«½ï",weightmp);
                pressanykey(inbuf);
            }
            else
            {
              pressanykey("»ØĞÄ×ªÒâÂŞ.....");
            }
          }
          else
          {
            pressanykey("ÄãÇ®Ã»ÄÇ÷á¶àÀ².......");            
          }
          break;        
          
        case '3':
          if(d.money<80)
          {
            pressanykey("´«Í³¼õ·ÊÒª80Ôªà¸....Äã²»¹»Ç®À²...");
          }
          else
          {
#ifdef MAPLE
            getdata(b_lines-1,1, "Ğè»¨·Ñ80Ôª(3¡«5¹«½ï)£¬ÄãÈ·¶¨Âğ? [y/N]", genbuf, 2, 1, 0);
#else
            getdata(b_lines-1,1, "Ğè»¨·Ñ80Ôª(3¡«5¹«½ï)£¬ÄãÈ·¶¨Âğ? [y/N]", genbuf, 2, DOECHO, YEA);
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
              sprintf(inbuf, "×Ü¹²¼õÉÙÁË%d¹«½ï",weightmp);
              pressanykey(inbuf);
            }
            else
            {
              pressanykey("»ØĞÄ×ªÒâÂŞ.....");
            }
          }        
          break;
        case '4':
#ifdef MAPLE
          getdata(b_lines-1,1, "¼õÒ»¹«½ïÒª30Ôª£¬ÄãÒª¼õ¶àÉÙ¹«½ïÄØ? [ÇëÌîÊı×Ö]:", genbuf, 4, 1, 0);
#else
          getdata(b_lines-1,1, "¼õÒ»¹«½ïÒª30Ôª£¬ÄãÒª¼õ¶àÉÙ¹«½ïÄØ? [ÇëÌîÊı×Ö]:", genbuf, 4, DOECHO, YEA);
#endif  // END MAPLE
          weightmp=atoi(genbuf);
          if(weightmp<=0)
          {
            pressanykey("ÊäÈëÓĞÎó..·ÅÆúÂŞ...");
          }          
          else if(d.weight<=weightmp)
          {
            pressanykey("ÄãÃ»ÄÇ÷áÖØà¸.....");
          }
          else if(d.money>(weightmp*30))
          {
            sprintf(inbuf, "¼õÉÙ%d¹«½ï£¬×Ü¹²Ğè»¨·ÑÁË%dÔª£¬È·¶¨Âğ? [y/N]",weightmp,weightmp*30);
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
                sprintf(inbuf, "×Ü¹²¼õÉÙÁË%d¹«½ï",weightmp);
                pressanykey(inbuf);
            }
            else
            {
              pressanykey("»ØĞÄ×ªÒâÂŞ.....");
            }
          }
          else
          {
            pressanykey("ÄãÇ®Ã»ÄÇ÷á¶àÀ².......");            
          }
          break;
      }
    }
    return 0;
}


/*²Î¼û*/

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
  int prince;  /*Íõ×Ó»á²»»á³öÏÖ*/
  int pipkey;
  int change;
  char buf[256];
  char inbuf1[20];
  char inbuf2[20];
  char ans[5];
  char *needmode[3]={"      ","ÀñÒÇ±íÏÖ£¾","Ì¸ÍÂ¼¼ÇÉ£¾"};
  int save[11]={0,0,0,0,0,0,0,0,0,0,0};

  d.nodone=0; 
  do
  {
  clear();
  show_palace_pic(0);
  move(13,4);
  sprintf(buf,"[1;31m©°¡ª¡ª¡ª¡ª¡ª¡ª©È[37;41m À´µ½×ÜË¾Áî²¿ÁË  ÇëÑ¡ÔñÄãÓû°İ·ÃµÄ¶ÔÏó [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª©´[0m");
  prints(buf);
  move(14,4);
  sprintf(buf,"[1;31m©¦                                                                  ©¦[0m");
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
      sprintf(buf,"[1;31m©¦ [36m([37m%s[36m) [33m%-10s  [37m%-14s     [36m([37m%s[36m) [33m%-10s  [37m%-14s[31m©¦[0m",    
             p[a].num,p[a].name,inbuf1,p[b].num,p[b].name,inbuf2);
    else
      sprintf(buf,"[1;31m©¦ [36m([37m%s[36m) [33m%-10s  [37m%-14s                                   [31m©¦[0m",    
             p[a].num,p[a].name,inbuf1);             
    prints(buf);
  }
  move(20,4);
  sprintf(buf,"[1;31m©¦                                                                  ©¦[0m");
  prints(buf);
  move(21,4);
  sprintf(buf,"[1;31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[0m");
  prints(buf);
  

   if(d.death==1 || d.death==2 || d.death==3)
     return 0;    
  /*½«¸÷ÈËÎñÒÑ¾­¸øÓëµÄÊıÖµ½Ğ»ØÀ´*/
   save[1]=d.royalA;            /*fromÊØÎÀ*/
   save[2]=d.royalB;            /*from½üÎÀ*/
   save[3]=d.royalC;		/*from½«¾ü*/
   save[4]=d.royalD;            /*from´ó³¼*/
   save[5]=d.royalE;            /*from¼ÀË¾*/
   save[6]=d.royalF;            /*from³èåú*/
   save[7]=d.royalG;            /*fromÍõåú*/
   save[8]=d.royalH;            /*from¹úÍõ*/
   save[9]=d.royalI;            /*fromĞ¡³ó*/
   save[10]=d.royalJ;           /*fromÍõ×Ó*/

   move(b_lines-1, 0);
   clrtoeol();
   move(b_lines-1,0);
   prints("[1;33m [ÉúÃüÁ¦] %d/%d  [Æ£ÀÍ¶È] %d [0m",d.hp,d.maxhp,d.tired);
             
   move(b_lines, 0);
   clrtoeol();
   move(b_lines,0);
   prints(
   "[1;37;46m  ²Î¼ûÑ¡µ¥  [44m [×ÖÄ¸]Ñ¡ÔñÓû°İ·ÃµÄÈËÎï  [Q]Àë¿ªĞÇ¿Õ×ÜË¾Áî²¿£º                    [0m");
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
       pipdie("[1;31mÀÛËÀÁË...[m  ",1);
    }
    if(d.hp<0)
    {
       d.death=1;
       pipdie("[1;31m¶öËÀÁË...[m  ",1);
    }
    if(d.death==1)
    {
      sprintf(buf,"êşêşÁË...ÕæÊÇ±¯Çé..");
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
            sprintf(buf,"ÄÜºÍÕâ÷áÎ°´óµÄÄã½²»°ÕæÊÇÈÙĞÒ¨Ú...");
        else
            sprintf(buf,"ºÜ¸ßĞËÄãÀ´°İ·ÃÎÒ£¬µ«ÎÒ²»ÄÜ¸øÄãÊ²÷áÁË..");
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
          /*Èç¹û´óì¶Ã¿´ÎµÄÔö¼Ó×î´óÁ¿*/
          if(change > p[choice].addtoman)
             change=p[choice].addtoman;
          /*Èç¹û¼ÓÉÏÔ­ÏÈµÄÖ®áá´óì¶ËùÄÜ¸øµÄËùÓĞÖµÊ±*/
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
            sprintf(buf,"ÎÒ²»ºÍÄãÕâÑùµÄ¼¦Ì¸»°....");
      else
            sprintf(buf,"ÄãÕâÖ»Ã»½ÌÑøµÄ¼¦£¬ÔÙÈ¥Ñ§Ñ§ÀñÒÇ°É....");    
    
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

  pressanykey("Àë¿ªĞÇ¿Õ×ÜË¾Áî²¿.....");  
  return 0;
}

int pip_query()  /*°İ·ÃĞ¡¼¦*/
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

  stand_title("°İ·ÃÍ¬°é");
  usercomplete(msg_uid, genbuf);
  if (genbuf[0])
  {
    move(2, 0);
    if (id = getuser(genbuf))
    {
        pip_read(genbuf);
        pressanykey("¹ÛÄ¦Ò»ÏÂ±ğÈËµÄĞ¡¼¦...:p");
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
  /*char yo[14][5]={"µ®Éú","Ó¤¶ù","Ó×¶ù","¶ùÍ¯","ÇàÄê","ÉÙÄê","³ÉÄê",
                  "×³Äê","×³Äê","×³Äê","¸üÄê","ÀÏÄê","ÀÏÄê","¹ÅÏ¡"};*/
  char yo[12][5]={"µ®Éú","Ó¤¶ù","Ó×¶ù","¶ùÍ¯","ÉÙÄê","ÇàÄê",
                  "³ÉÄê","×³Äê","¸üÄê","ÀÏÄê","¹ÅÏ¡","ÉñÏÉ"};                  
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
  
    if(age==0) /*µ®Éú*/
       age1=0;
    else if( age==1) /*Ó¤¶ù*/
       age1=1;
    else if( age>=2 && age<=5 ) /*Ó×¶ù*/
       age1=2;
    else if( age>=6 && age<=12 ) /*¶ùÍ¯*/
       age1=3;
    else if( age>=13 && age<=15 ) /*ÉÙÄê*/
       age1=4;     
    else if( age>=16 && age<=18 ) /*ÇàÄê*/
       age1=5;     
    else if( age>=19 && age<=35 ) /*³ÉÄê*/
       age1=6;
    else if( age>=36 && age<=45 ) /*×³Äê*/
       age1=7;
    else if( age>=45 && age<=60 ) /*¸üÄê*/
       age1=8;
    else if( age>=60 && age<=70 ) /*ÀÏÄê*/
       age1=9;
    else if( age>=70 && age<=100 ) /*¹ÅÏ¡*/
       age1=10;
    else if( age>100 ) /*ÉñÏÉ*/
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
  clrtobot();
#ifdef MAPLE
  prints("ÕâÊÇ%sÑøµÄĞ¡¼¦£º\n",xuser.userid);
#else
  prints("ÕâÊÇ%sÑøµÄĞ¡¼¦£º\n",genbuf);
#endif  // END MAPLE

  if (d1.death==0)
  {
   prints("[1;32mName£º%-10s[m  ÉúÈÕ£º%02dÄê%02dÔÂ%2dÈÕ   ÄêÁä£º%2dËê  ×´Ì¬£º%s  Ç®Ç®£º%d\n"
          "ÉúÃü£º%3d/%-3d  ¿ìÀÖ£º%-4d  ÂúÒâ£º%-4d  ÆøÖÊ£º%-4d  ÖÇ»Û£º%-4d  ÌåÖØ£º%-4d\n"
          "´ó²¹Íè£º%-4d   Ê³Îï£º%-4d  ÁãÊ³£º%-4d  Æ£ÀÍ£º%-4d  ÔàÔà£º%-4d  ²¡Æø£º%-4d\n",
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
   if(d1.shit==0) prints("ºÜÇ¬¾»..");
   if(d1.shit>40&&d1.shit<60) prints("³ô³ôµÄ..");
   if(d1.shit>=60&&d1.shit<80) prints("ºÃ³ôà¸..");
   if(d1.shit>=80&&d1.shit<100) prints("[1;34m¿ì³ôËÀÁË..[m");
   if(d1.shit>=100) {prints("[1;31m³ôËÀÁË..[m"); return -1;}

   pc1=hp1*100/d1.maxhp;
   if(pc1==0) {prints("¶öËÀÁË.."); return -1;}
   if(pc1<20) prints("[1;35mÈ«ÉíÎŞÁ¦ÖĞ.¿ì¶öËÀÁË.[m");
   if(pc1<40&&pc1>=20) prints("ÌåÁ¦²»Ì«¹»..Ïë³Ôµã¶«Î÷..");
   if(pc1<100&&pc1>=80) prints("àÅ¡«¶Ç×Ó±¥±¥ÓĞÌåÁ¦..");
   if(pc1>=100) prints("[1;34m¿ì³ÅËÀÁË..[m");

   pc1=d1.tired;
   if(pc1<20) prints("¾«Éñ¶¶¶¶ÖĞ..");
   if(pc1<80&&pc1>=60) prints("[1;34mÓĞµãĞ¡ÀÛ..[m");
   if(pc1<100&&pc1>=80) {prints("[1;31mºÃÀÛà¸£¬¿ì²»ĞĞÁË..[m"); }
   if(pc1>=100) {prints("ÀÛËÀÁË..."); return -1;}

   pc1=60+10*age;
   if(d1.weight<(pc1+30) && d1.weight>=(pc1+10)) prints("ÓĞµãĞ¡ÅÖ..");
   if(d1.weight<(pc1+50) && d1.weight>=(pc1+30)) prints("Ì«ÅÖÁË..");
   if(d1.weight>(pc1+50)) {prints("ÅÖËÀÁË..."); return -1;}

   if(d1.weight<(pc1-50)) {prints("ÊİËÀÁË.."); return -1;}
   if(d1.weight>(pc1-30) && d1.weight<=(pc1-10)) prints("ÓĞµãĞ¡Êİ..");
   if(d1.weight>(pc1-50) && d1.weight<=(pc1-30)) prints("Ì«ÊİÁË..");

   if(d1.sick<75&&d1.sick>=50) prints("[1;34mÉú²¡ÁË..[m");
   if(d1.sick<100&&d1.sick>=75) {prints("[1;31m²¡ÖØ!!..[m"); }
   if(d1.sick>=100) {prints("²¡ËÀÁË.!."); return -1;}

   pc1=d1.happy;
   if(pc1<20) prints("[1;31mºÜ²»¿ìÀÖ..[m");
   if(pc1<40&&pc1>=20) prints("²»¿ìÀÖ..");
   if(pc1<95&&pc1>=80) prints("¿ìÀÖ..");
   if(pc1<=100&&pc1>=95) prints("ºÜ¿ìÀÖ..");

   pc1=d1.satisfy;
   if(pc1<40) prints("[31;1m²»Âú×ã..[m");
   if(pc1<95&&pc1>=80) prints("Âú×ã..");
   if(pc1<=100&&pc1>=95) prints("ºÜÂú×ã..");
  }
  else if(d1.death==1)
  {
     show_die_pic(2);
     move(14,20);
     prints("¿ÉÁ¯µÄĞ¡¼¦ÎØºô°§ÔÕÁË");
  } 
  else if(d1.death==2)
  {
     show_die_pic(3);
  }
  else if(d1.death==3)
  {
    move(5,0);
    outs("ÓÎÏ·ÒÑ¾­Íæµ½½á¾ÖÂŞ....");
  }
  else
  {
    pressanykey("µµ°¸Ëğ»ÙÁË....");
  }
 }   /* ÓĞÑøĞ¡¼¦ */
 else
 {
   move(1,0);
   clrtobot();
   pressanykey("ÕâÒ»¼ÒµÄÈËÃ»ÓĞÑøĞ¡¼¦......");
 }
}
