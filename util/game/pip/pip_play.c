/*---------------------------------------------------------------------------*/
/* ÍæÀÖÑ¡µ¥:É¢²½ ÂÃÓÎ ÔË¶¯ Ô¼»á ²ÂÈ­                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

int pip_play_stroll()	/*É¢²½*/
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
      pressanykey("Óöµ½ÅóÓÑÂŞ  ÕæºÃ.... ^_^");
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
      pressanykey("¼ñµ½ÁË100ÔªÁË..Ò®Ò®Ò®....");
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
         pressanykey("µôÁË50ÔªÁË..ÎØÎØÎØ....");
        }
       else
        {
         d.money=0;
         d.hp-=(rand()%3+3);
         pressanykey("Ç®µô¹â¹âÁË..ÎØÎØÎØ....");
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
         pressanykey("ÓÃÁË50ÔªÁË..²»¿ÉÒÔÂîÎÒà¸....");
        }
       else
        {
         d.money=0;
         d.hp-=(rand()%3+3);
         pressanykey("Ç®±»ÎÒÍµÓÃ¹â¹âÁË..:p");
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
      pressanykey("ºÃ°ôà¸¼ñµ½Íæ¾ßÁËËµ.....");
     }
    else if(lucky==6)
     {
      d.happy-=(rand()%3+10);
      d.shit+=(rand()%3+20);
      move(4,0);
      show_play_pic(9);
      pressanykey("ÕæÊÇµ¹é¹  ¿ÉÒÔÈ¥Âò°®¹ú½±È¯");
     }
    else
    {
      d.happy+=rand()%3+3;
      d.satisfy+=rand()%2+1;
      d.shit+=rand()%3+2;
      d.hp-=(rand()%3+2);
      move(4,0);
      show_play_pic(8);
      pressanykey("Ã»ÓĞÌØ±ğµÄÊÂ·¢ÉúÀ².....");
    }
    return 0;
}

int pip_play_sport()	/*ÔË¶¯*/
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
    pressanykey("ÔË¶¯ºÃ´¦¶à¶àÀ²...");
    return 0;
}

int pip_play_date()	/*Ô¼»á*/
{
    if(d.money<150)
    {
     pressanykey("ÄãÇ®²»¹»¶àÀ²! Ô¼»á×ÜµÃ»¨µãÇ®Ç®");
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
     pressanykey("Ô¼»áÈ¥  ºôºô");
    }
    return 0;
}
int pip_play_outing()	/*½¼ÓÎ*/
{
    int lucky;
    char buf[256];
    
    if(d.money<250)
    {
     pressanykey("ÄãÇ®²»¹»¶àÀ²! ÂÃÓÎ×ÜµÃ»¨µãÇ®Ç®");
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
         pressanykey("ĞÄÖĞÓĞÒ»¹Éµ­µ­µÄ¸Ğ¾õ  ºÃÊæ·şà¸....");
       else
         pressanykey("ÔÆË® ÏĞÇé ĞÄÇéºÃ¶àÁË.....");
      }
      else if(lucky==1)
      {
       d.art+=rand()%3;
       d.maxmp+=rand()%2;
       show_play_pic(13);
       if(rand()%2>0)
         pressanykey("ÓĞÉ½ÓĞË®ÓĞÂäÈÕ  ĞÎ³ÉÒ»·ùÃÀÀöµÄ»­..");
       else
         pressanykey("¿´×Å¿´×Å  È«ÉíÆ£±¹¶¼²»¼ûÂŞ..");
      }
      else if(lucky==2)
      {
       d.love+=rand()%3;
       show_play_pic(14);
       if(rand()%2>0)
         pressanykey("¿´  Ì«Ñô¿ìÃ»ÈëË®ÖĞÂŞ...");
       else
         pressanykey("ÌıËµÕâÊÇº£±ßÀ²  ÄãËµÄØ?");
      }      
      else if(lucky==3)
      {
       d.maxhp+=rand()%3;
       show_play_pic(15);
       if(rand()%2>0)
         pressanykey("ÈÃÎÒÃÇ·è¿ñÔÚÒ¹ÀïµÄº£Ì²°É....ºôºô..");
       else
         pressanykey("Á¹Ë¬µÄº£·çÓ­ÃæÏ®À´  ×îÏ²»¶ÕâÖÖ¸Ğ¾õÁË....");
      }
      if((rand()%301+rand()%200)%100==12)
      {
        lucky=0;
        clear();
        sprintf(buf,"[1;41m  ĞÇ¿ÕÕ½¶·¼¦ ¡« %-10s                                                    [0m",d.name); 	 
        show_play_pic(0);
        move(17,10);
        prints("[1;36mÇ×°®µÄ [1;33m%s ¡«[0m",d.name);
        move(18,10);
        prints("[1;37m¿´µ½ÄãÕâÑùÅ¬Á¦µÄÅàÑø×Ô¼ºµÄÄÜÁ¦  ÈÃÎÒĞÄÖĞÊ®·ÖµÄ¸ßĞËà¸..[m");
        move(19,10);
        prints("[1;36mĞ¡ÌìÊ¹ÎÒ¾ö¶¨¸øÄã½±ÉÍ¹ÄÀø¹ÄÀø  ÍµÍµµØ°ïÖúÄãÒ»ÏÂ....^_^[0m");
        move(20,10);
        lucky=rand()%7;
        if(lucky==6)
        {
          prints("[1;33mÎÒ½«°ïÄãµÄ¸÷ÏîÄÜÁ¦È«²¿ÌáÉı°Ù·ÖÖ®Îåà¸......[0m");
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
          prints("[1;33mÎÒ½«°ïÄãµÄÕ½¶·ÄÜÁ¦È«²¿ÌáÉı°Ù·ÖÖ®Ê®à¸.......[0m");        
          d.attack=d.attack*110/100;
          d.resist=d.resist*110/100;          
          d.speed=d.speed*110/100;        
          d.brave=d.brave*110/100;                              
        } 
               
        else if(lucky<=3 && lucky>=2)
        {
          prints("[1;33mÎÒ½«°ïÄãµÄÄ§·¨ÄÜÁ¦ºÍÉúÃüÁ¦È«²¿ÌáÉı°Ù·ÖÖ®Ê®à¸.......[0m");        
          d.maxhp=d.maxhp*110/100;
          d.hp=d.maxhp;
          d.maxmp=d.maxmp*110/100;
          d.mp=d.maxmp;                  
        }
        else if(lucky<=1 && lucky>=0)
        {
          prints("[1;33mÎÒ½«°ïÄãµÄ¸ĞÊÜÄÜÁ¦È«²¿ÌáÉı°Ù·ÖÖ®¶şÊ®à¸....[0m");                
          d.character=d.character*110/100;          
          d.love=d.love*110/100;          
          d.wisdom=d.wisdom*110/100;     
          d.art=d.art*110/100;               
          d.homework=d.homework*110/100;                  
        }        
        
        pressanykey("Çë¼ÌĞø¼ÓÓÍà¸..."); 
      }
    }
    return 0;
}

int pip_play_kite()	/*·çóİ*/
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
    pressanykey("·Å·çóİÕæºÃÍæÀ²...");
    return 0;
}

int pip_play_KTV()	/*KTV*/
{
    if(d.money<250)
    {
     pressanykey("ÄãÇ®²»¹»¶àÀ²! ³ª¸è×ÜµÃ»¨µãÇ®Ç®");
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
     pressanykey("ÄãËµÄã  ÏëÒªÌÓ...");
    }
    return 0;
}

int pip_play_guess()   /* ²ÂÈ­³ÌÊ½ */
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
    prints("[1;44;37m  ²ÂÈ­Ñ¡µ¥  [46m[1]ÎÒ³ö¼ôµ¶ [2]ÎÒ³öÊ¯Í· [3]ÎÒ³ö²¼À² [4]²ÂÈ­¼ÇÂ¼ [Q]Ìø³ö£º         [m");   
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
	      my_write(currutmp->msgs[0].last_pid, "Ë®Çò¶ª»ØÈ¥£º");
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
          outs("Ğ¡¼¦£º¼ôµ¶\n");
         break;
        case 1:
          outs("Ğ¡¼¦£ºÊ¯Í·\n");
         break;
        case 2:
          outs("Ğ¡¼¦£º²¼\n");
         break;
     }

    move(17,0);

    switch(pipkey){
    case '1':
      outs("Äã  £º¼ôµ¶\n");
      if (com==0)
        tie();
      else  if (com==1)
        lose();
      else if (com==2)
        win();
      break;
    case '2':
      outs("Äã¡¡£ºÊ¯Í·\n");
      if (com==0)
        win();
      else if (com==1)
        tie();
      else if (com==2)
        lose();
      break;
    case '3':
      outs("Äã¡¡£º²¼\n");
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
    pressanykey("Ğ¡¼¦ÊäÁË....~>_<~");
    return;
}
int tie()
{
    d.hp-=rand()%2+3;
    d.happy+=rand()%3+5;
    move(4,0);
    show_guess_pic(3);
    move(b_lines,0);
    pressanykey("Æ½ÊÖ........-_-");
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
    pressanykey("Ğ¡¼¦Ó®ÂŞ....*^_^*");
    return;
}

int situ()
{
        clrchyiuan(19,21);
        move(19,0);
        prints("Äã:[44m %dÊ¤ %d¸º[m                     \n",d.winn,d.losee);
        move(20,0);
        prints("¼¦:[44m %dÊ¤ %d¸º[m                     \n",d.losee,d.winn);

       if (d.winn>=d.losee)
       {
        move(b_lines,0);
        pressanykey("¹ş..Ó®Ğ¡¼¦Ò²Ã»¶à¹âÈÙ");
       }
       else
       {
        move(b_lines,0);
        pressanykey("±¿µ°..¾¹Êä¸øÁË¼¦....¨Ü...");
       }
       return;
}
