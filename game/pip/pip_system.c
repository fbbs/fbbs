/*---------------------------------------------------------------------------*/
/* ÏµÍ³Ñ¡µ¥:¸öÈË×ÊÁÏ  Ð¡¼¦·ÅÉú  ÌØ±ð·þÎñ                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

char weaponhead[7][10]={
"Ã»ÓÐ×°±¸",
"ËÜ½ºÃ±×Ó", 
"Å£Æ¤Ð¡Ã±",
"  °²È«Ã±",
"¸ÖÌúÍ·¿ø",
"Ä§·¨·¢¹¿",
"»Æ½ðÊ¥¿ø"};


char weaponrhand[10][10]={
"Ã»ÓÐ×°±¸",
"´óÄ¾°ô",  
"½ðÊô°âÊÖ",
"ÇàÍ­½£",  
"ÇçÀ×½£", 
"²õÒíµ¶", 
"ÍüÇé½£", 
"Ê¨Í·±¦µ¶",
"ÍÀÁúµ¶",  
"»Æ½ðÊ¥ÕÈ"
};  

char weaponlhand[8][10]={
"Ã»ÓÐ×°±¸",
"´óÄ¾°ô", 
"½ðÊô°âÊÖ",
"Ä¾¶Ü",
"²»Ðâ¸Ö¶Ü",
"°×½ðÖ®¶Ü",
"Ä§·¨¶Ü",
"»Æ½ðÊ¥¶Ü"
};


char weaponbody[7][10]={
"Ã»ÓÐ×°±¸",
"ËÜ½ºëÐ¼×",
"ÌØ¼¶Æ¤¼×",
"¸ÖÌú¿ø¼×",
"Ä§·¨Åû·ç",
"°×½ð¿ø¼×",
"»Æ½ðÊ¥ÒÂ"};

char weaponfoot[8][12]={
"Ã»ÓÐ×°±¸",
"ËÜ½ºÍÏÐ¬",
"¶«ÑóÄ¾åì",
"ÌØ¼¶ÓêÐ¬",
"NIKEÔË¶¯Ð¬",
"öùÓãÆ¤Ñ¥",
"·ÉÌìÄ§Ñ¥",
"»Æ½ðÊ¥Ñ¥"
};

int 
pip_system_freepip()
{
      char buf[256];
      move(b_lines-1, 0);
      clrtoeol();
#ifdef MAPLE
      getdata(b_lines-1,1, "ÕæµÄÒª·ÅÉúÂð£¿(y/N)", buf, 2, 1, 0);
#else
      getdata(b_lines-1,1, "ÕæµÄÒª·ÅÉúÂð£¿(y/N)", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
      if (buf[0]!='y'&&buf[0]!='Y') return 0;
      sprintf(buf,"%s ±»ºÝÐÄµÄ %s ¶ªµôÁË~",d.name,cuser.userid);
      pressanykey(buf);
      d.death=2;
      pipdie("[1;31m±»ºÝÐÄ¶ªÆú:~~[0m",2);
      return 0;
}


int
pip_system_service()
{
     int pipkey;
     int oldchoice;
     char buf[200];
     char oldname[21];
     time_t now;     

     move(b_lines, 0);
     clrtoeol();
     move(b_lines,0);
     prints("[1;44m  ·þÎñÏîÄ¿  [46m[1]ÃüÃû´óÊ¦ [2]±äÐÔÊÖÊõ [3]½á¾ÖÉè¾Ö                                [0m");
     pipkey=egetch();
     
     switch(pipkey)
     {
     case '1':
       move(b_lines-1,0);
       clrtobot();
#ifdef MAPLE
       getdata(b_lines-1, 1, "°ïÐ¡¼¦ÖØÐÂÈ¡¸öºÃÃû×Ö£º", buf, 11, DOECHO,NULL);
#else
       getdata(b_lines-1, 1, "°ïÐ¡¼¦ÖØÐÂÈ¡¸öºÃÃû×Ö£º", buf, 11, DOECHO,YEA);
#endif  // END MAPLE
       if(!buf[0])
       {
         pressanykey("µÈÒ»ÏÂÏëºÃÔÙÀ´ºÃÁË  :)");
         break;
       }
       else
       {
        strcpy(oldname,d.name);
        strcpy(d.name,buf);
        /*¸ÄÃû¼ÇÂ¼*/
        now=time(0);
        sprintf(buf, "[1;37m%s %-11s°ÑÐ¡¼¦ [%s] ¸ÄÃû³É [%s] [0m\n", Cdate(&now), cuser.userid,oldname,d.name);
        pip_log_record(buf);
        pressanykey("àÅàÅ  »»Ò»¸öÐÂµÄÃû×Öà¸...");
       }
       break;
       
     case '2':  /*±äÐÔ*/
       move(b_lines-1,0);
       clrtobot();
       /*1:¹« 2:Ä¸ */
       if(d.sex==1)
       { 
         oldchoice=2; /*¹«-->Ä¸*/
         move(b_lines-1, 0);
         prints("[1;33m½«Ð¡¼¦ÓÉ[32m¡á[33m±äÐÔ³É[35m¡â[33mµÄÂð£¿ [37m[y/N][0m");
       }
       else
       { 
         oldchoice=1; /*Ä¸-->¹«*/
         move(b_lines-1, 0); 
         prints("[1;33m½«Ð¡¼¦ÓÉ[35m¡â[33m±äÐÔ³É[35m¡á[33mµÄÂð£¿ [37m[y/N][0m");
       }
       move(b_lines,0);
       prints("[1;44m  ·þÎñÏîÄ¿  [46m[1]ÃüÃû´óÊ¦ [2]±äÐÔÊÖÊõ [3]½á¾ÖÉè¾Ö                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         /*¸ÄÃû¼ÇÂ¼*/
         now=time(0);
         if(d.sex==1)
           sprintf(buf,"[1;37m%s %-11s°ÑÐ¡¼¦ [%s] ÓÉ¡á±äÐÔ³É¡âÁË[0m\n",Cdate(&now), cuser.userid,d.name);
         else
           sprintf(buf,"[1;37m%s %-11s°ÑÐ¡¼¦ [%s] ÓÉ¡â±äÐÔ³É¡áÁË[0m\n",Cdate(&now), cuser.userid,d.name);           
         pip_log_record(buf);
         pressanykey("±äÐÔÊÖÊõÍê±Ï...");       
         d.sex=oldchoice;
       }  
       break;
       
     case '3':
       move(b_lines-1,0);
       clrtobot();
       /*1:²»ÒªÇÒÎ´»é 4:ÒªÇÒÎ´»é */
       oldchoice=d.wantend;
       if(d.wantend==1 || d.wantend==2 || d.wantend==3)
       { 
         oldchoice+=3; /*Ã»ÓÐ-->ÓÐ*/
         move(b_lines-1, 0); 
         prints("[1;33m½«Ð¡¼¦ÓÎÏ·¸Ä³É[32m[ÓÐ20Ëê½á¾Ö][33m? [37m[y/N][0m");
	 sprintf(buf,"Ð¡¼¦ÓÎÏ·Éè¶¨³É[ÓÐ20Ëê½á¾Ö]..");         
       }
       else
       { 
         oldchoice-=3; /*ÓÐ-->Ã»ÓÐ*/
         move(b_lines-1, 0); 
         prints("[1;33m½«Ð¡¼¦ÓÎÏ·¸Ä³É[32m[Ã»ÓÐ20Ëê½á¾Ö][33m? [37m[y/N][0m");
         sprintf(buf,"Ð¡¼¦ÓÎÏ·Éè¶¨³É[Ã»ÓÐ20Ëê½á¾Ö]..");
       }
       move(b_lines,0);
       prints("[1;44m  ·þÎñÏîÄ¿  [46m[1]ÃüÃû´óÊ¦ [2]±äÐÔÊÖÊõ [3]½á¾ÖÉè¾Ö                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         d.wantend=oldchoice;
         pressanykey(buf);
       }  
       break;     
     } 
     return 0;
}

int
pip_data_list()  /*¿´Ð¡¼¦¸öÈËÏêÏ¸×ÊÁÏ*/
{
  char buf[256];
  char inbuf1[20];
  char inbuf2[20];
  int tm;
  int pipkey;
  int page=1;
  
  tm=(time(0)-start_time+d.bbtime)/60/30;

  clear();  
  move(1,0);
  prints("       [1;33m©³©¥©¥©¥    ©¥©¥©¥  ©³©¥©¥©¥©·  ©¥©¥©¥  [m\n");
  prints("       [0;37m©§      ©§©§ ©¥   ©§©»©·©³©¥©¿©§ ©¥   ©§[m\n");
  prints("       [1;37m©§      ©§©§©³©·  ©§  ©§©§    ©§©³©·  ©§[m\n");
  prints("       [1;34m©»©¥©¥©¥  ©»©¿©»©¥©¿  ©»©¿    ©»©¿©»©¥©¿[32m......................[m");
  do
  { clrchyiuan(5,23);
    switch(page)
    {
     case 1:
       move(5,0);
       sprintf(buf,
       "[1;31m ©°©È[41;37m »ù±¾×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m\n");  
       prints(buf);
  
       sprintf(buf,
       "[1;31m ©¦[33m££ÐÕ    Ãû :[37m %-10s [33m££Éú    ÈÕ :[37m %02d/%02d/%02d   [33m££Äê    ¼Í :[37m %-2d         [31m©¦[m\n",
       d.name,d.year%100,d.month,d.day,tm);
       prints(buf);  
  
       sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
       sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);  
       sprintf(buf,
       "[1;31m ©¦[33m££Ìå    ÖØ :[37m %-5d(Ã×¿Ë)[33m££Ìå    Á¦ :[37m %-11s[33m££·¨    Á¦ :[37m %-11s[31m©¦[m\n",
       d.weight,inbuf1,inbuf2);
       prints(buf);  
  
       sprintf(buf,
       "[1;31m ©¦[33m££Æ£    ÀÍ :[37m %-3d        [33m££²¡    Æø :[37m %-3d        [33m££Ôà    Ôà :[37m %-3d        [31m©¦[m\n",
       d.tired,d.sick,d.shit);
       prints(buf);  
   
       sprintf(buf,  
       "[1;31m ©¦[33m££Íó    Á¦ :[37m %-7d    [33m££Ç××Ó¹ØÏµ :[37m %-7d    [33m££½ð    Ç® :[37m %-11d[31m©¦[m\n",
       d.wrist,d.relation,d.money);
       prints(buf);  
  
       sprintf(buf,  
       "[1;31m ©À©È[41;37m ÄÜÁ¦×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[m\n");
       prints(buf);  
   
       sprintf(buf,   
       "[1;31m ©¦[33m££Æø    ÖÊ :[37m %-10d [33m££ÖÇ    Á¦ :[37m %-10d [33m££°®    ÐÄ :[37m %-10d [31m©¦[m\n",
       d.character,d.wisdom,d.love);
       prints(buf);  
   
       sprintf(buf, 
       "[1;31m ©¦[33m££ÒÕ    Êõ :[37m %-10d [33m££µÀ    µÂ :[37m %-10d [33m££¼Ò    ÊÂ :[37m %-10d [31m©¦[m\n",
       d.art,d.etchics,d.homework);
       prints(buf);  
 
       sprintf(buf, 
       "[1;31m ©¦[33m££Àñ    ÒÇ :[37m %-10d [33m££Ó¦    ¶Ô :[37m %-10d [33m££Åë    â¿ :[37m %-10d [31m©¦[m\n",
       d.manners,d.speech,d.cookskill);
       prints(buf);    
 
       sprintf(buf,  
       "[1;31m ©À©È[41;37m ×´Ì¬×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[m\n");
       prints(buf);  
 
       sprintf(buf, 
       "[1;31m ©¦[33m££¿ì    ÀÖ :[37m %-10d [33m££Âú    Òâ :[37m %-10d [33m££ÈË    ¼Ê :[37m %-10d [31m©¦[m\n",
       d.happy,d.satisfy,d.toman);
       prints(buf);
  
       sprintf(buf, 
       "[1;31m ©¦[33m££÷È    Á¦ :[37m %-10d [33m££ÓÂ    ¸Ò :[37m %-10d [33m££ÐÅ    Ñö :[37m %-10d [31m©¦[m\n",
       d.charm,d.brave,d.belief);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ©¦[33m££×ï    Äõ :[37m %-10d [33m££¸Ð    ÊÜ :[37m %-10d [33m            [37m            [31m©¦[m\n",
       d.offense,d.affect);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ©À©È[41;37m ÆÀ¼Û×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[m\n");
       prints(buf);  

       sprintf(buf, 
       "[1;31m ©¦[33m££Éç½»ÆÀ¼Û :[37m %-10d [33m££Õ½¶·ÆÀ¼Û :[37m %-10d [33m££Ä§·¨ÆÀ¼Û :[37m %-10d [31m©¦[m\n",
       d.social,d.hexp,d.mexp);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ©¦[33m££¼ÒÊÂÆÀ¼Û :[37m %-10d [33m            [37m            [33m            [37m            [31m©¦[m\n",
       d.family);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m\n");
       prints(buf);  
       
       move(b_lines-1,0);       
       sprintf(buf, 
       "                                                              [1;36mµÚÒ»Ò³[37m/[36m¹²¶þÒ³[m\n");
       prints(buf);  
       break;

     case 2:
       move(5,0);
       sprintf(buf, 
       "[1;31m ©°©È[41;37m ÎïÆ·×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¦[33m££Ê³    Îï :[37m %-10d [33m££Áã    Ê³ :[37m %-10d [33m££´ó ²¹ Íè :[37m %-10d [31m©¦[m\n",
       d.food,d.cookie,d.bighp);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¦[33m££Ò©    ²Ý :[37m %-10d [33m££Êé    ±¾ :[37m %-10d [33m££Íæ    ¾ß :[37m %-10d [31m©¦[m\n",
       d.medicine,d.book,d.playtool);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©À©È[41;37m ÓÎÏ·×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¦[33m££²Â È­ Ó® :[37m %-10d [33m££²Â È­ Êä :[37m %-10d                         [31m©¦[m\n",
       d.winn,d.losee);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©À©È[41;37m ÎäÁ¦×ÊÁÏ [0;1;31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¦[33m££¹¥ »÷ Á¦ :[37m %-10d [33m££·À Óù Á¦ :[37m %-10d [33m££ËÙ ¶È Öµ :[37m %-10d [31m©¦[m\n",
       d.attack,d.resist,d.speed);
       prints(buf);  
       sprintf(buf, 
       "[1;31m ©¦[33m££¿¹Ä§ÄÜÁ¦ :[37m %-10d [33m££Õ½¶·¼¼Êõ :[37m %-10d [33m££Ä§·¨¼¼Êõ :[37m %-10d [31m©¦[m\n",
       d.mresist,d.hskill,d.mskill);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¦[33m££Í·²¿×°±¸ :[37m %-10s [33m££ÓÒÊÖ×°±¸ :[37m %-10s [33m££×óÊÖ×°±¸ :[37m %-10s [31m©¦[m\n",
       weaponhead[d.weaponhead],weaponrhand[d.weaponrhand],weaponlhand[d.weaponlhand]);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¦[33m££ÉíÌå×°±¸ :[37m %-10s [33m££½Å²¿×°±¸ :[37m %-10s [33m            [37m            [31m©¦[m\n",
       weaponbody[d.weaponbody],weaponfoot[d.weaponfoot]);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m\n");
       prints(buf); 

       move(b_lines-1,0);
       sprintf(buf, 
       "                                                              [1;36mµÚ¶þÒ³[37m/[36m¹²¶þÒ³[m\n");
       prints(buf);          
       break;
    }
    move(b_lines,0);
    sprintf(buf,"[1;44;37m  ×ÊÁÏÑ¡µ¥  [46m  [¡ü/PAGE UP]ÍùÉÏÒ»Ò³ [¡ý/PAGE DOWN]ÍùÏÂÒ»Ò³ [Q]Àë¿ª:            [m");
    prints(buf);    
    pipkey=egetch();
    switch(pipkey)
    {
      case KEY_UP:
      case KEY_PGUP:
      case KEY_DOWN:
      case KEY_PGDN:
        if(page==1)
           page=2;
        else if(page==2)
           page=1;
        break;
#ifdef MAPLE
      case Ctrl('R'):
        if (currutmp->msgs[0].last_pid)
        {
          show_last_call_in();
          my_write(currutmp->msgs[0].last_pid, "Ë®Çò¶ª»ØÈ¥£º");
        }
        break;
#endif  // END MAPLE
    }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
  return 0;
}
