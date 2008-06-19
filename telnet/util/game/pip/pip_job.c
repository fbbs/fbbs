/*---------------------------------------------------------------------------*/
/* 打工选单:家事 苦工 家教 地摊                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

int pip_job_workA()
{
/*  ├————┼——————————————————————┤*/
/*  │家庭管理│待人接物 + N , 扫地洗衣 + N , 烹饪技巧 + N  │*/
/*  │        │和父亲的关系 + N , 疲劳 + 1 , 感受 - 2      │*/
/*  ├————┼——————————————————————┤*/
/*  │家庭管理│若 体    力 - RND (疲劳) >=   5 则工作成功  │*/
/*  ├————┼——————————————————————┤*/  
    float class;
    long workmoney;
    
    workmoney=0;       
    class=(d.hp*100/d.maxhp)-d.tired;        
    d.maxhp+=rand()%2;
    d.shit+=rand()%3+5;
    count_tired(3,7,"Y",100,1);
    d.hp-=(rand()%2+4);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    d.affect-=7+rand()%7;
    if(d.affect<=0)
       d.affect=0;
    show_job_pic(11);
    if(class>=75)
    {
      d.cookskill+=rand()%2+7;    
      d.homework+=rand()%2+7;
      d.family+=rand()%3+4;
      d.relation+=rand()%3+4;
      workmoney=80+(d.cookskill*2+d.homework+d.family)/40;
      pressanykey("家事很成功喔..多一点钱给你..");
    }
    else if(class<75 && class>=50)
    {
      d.cookskill+=rand()%2+5;    
      d.homework+=rand()%2+5;
      d.family+=rand()%3+3;
      d.relation+=rand()%3+3;
      workmoney=60+(d.cookskill*2+d.homework+d.family)/45;
      pressanykey("家事还蛮顺利的唷..嗯嗯..");
    }    
    else if(class<50 && class>=25)
    {
      d.cookskill+=rand()%3+3;    
      d.homework+=rand()%3+3;
      d.family+=rand()%3+2;
      d.relation+=rand()%3+2;
      workmoney=40+(d.cookskill*2+d.homework+d.family)/50;
      pressanykey("家事普普通通啦..可以更好的..加油..");      
    }    
    else if(class<25)
    {
      d.cookskill+=rand()%3+1;    
      d.homework+=rand()%3+1;
      d.family+=rand()%3+1;
      d.relation+=rand()%3+1;
      workmoney=20+(d.cookskill*2+d.homework+d.family)/60;
      pressanykey("家事很糟糕喔..这样不行啦..");      
    }        
    d.money+=workmoney;
    d.workA+=1;
    return 0;
}

int pip_job_workB()
{   
/*  ├————┼——————————————————————┤*/
/*  │育幼院  │母性 + N , 感受 + 1 , 魅力 - 1 , 疲劳 + 3   │*/
/*  ├————┼——————————————————————┤*/
/*  │育幼院  │若 体    力 - RND (疲劳) >=  20 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    workmoney=0;       
    class=(d.hp*100/d.maxhp)-d.tired;        
    d.maxhp+=rand()%2+1;
    d.shit+=rand()%3+5;
    d.affect+=rand()%3+4;
    
    count_tired(3,9,"Y",100,1);
    d.hp-=(rand()%3+6);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    d.charm-=rand()%3+4;
    if(d.charm<=0)
       d.charm=0;
    show_job_pic(21);
    if(class>=90)
    {
      d.love+=rand()%2+7;
      d.toman+=rand()%2+2;
      workmoney=150+(d.love+d.toman)/50;
      pressanykey("当保姆很成功喔..下次再来喔..");
    }
    else if(class<90 && class>=75)
    {
      d.love+=rand()%2+5;
      d.toman+=rand()%2+2;
      workmoney=120+(d.love+d.toman)/50;
      pressanykey("保姆还当的不错唷..嗯嗯..");
    }    
    else if(class<75 && class>=50)
    {
      d.love+=rand()%2+3;
      d.toman+=rand()%2+1;
      workmoney=100+(d.love+d.toman)/50;
      pressanykey("小朋友很皮喔..加油..");      
    }    
    else if(class<50)
    {
      d.love+=rand()%2+1;
      d.toman+=rand()%2+1;
      workmoney=80+(d.love+d.toman)/50;
      pressanykey("很糟糕喔..你罩不住小朋友耶...");      
    }        
    d.money+=workmoney;
    d.workB+=1;
    return 0;
}
             
int pip_job_workC()
{
/*  ├————┼——————————————————————┤*/
/*  │旅馆    │扫地洗衣 + N , 战斗技术 - N , 疲劳 + 2      │*/
/*  ├————┼——————————————————————┤*/
/*  │旅馆    │若 体    力 - RND (疲劳) >=  30 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    workmoney=0;       
    class=(d.hp*100/d.maxhp)-d.tired;        
    d.maxhp+=rand()%2+2;
    d.shit+=rand()%3+5;
    count_tired(5,12,"Y",100,1);
    d.hp-=(rand()%4+8);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    show_job_pic(31);
    if(class>=95)
    {
      d.homework+=rand()%2+7;
      d.family+=rand()%2+4;
      d.hskill-=rand()%2+7;
      if(d.hskill<0)
           d.hskill=0;
      workmoney=250+(d.cookskill*2+d.homework*2)/40;
      pressanykey("旅馆事业蒸蒸日上..希望你再过来...");
    }
    else if(class<95 && class>=80)
    {
      d.homework+=rand()%2+5;
      d.family+=rand()%2+3;
      d.hskill-=rand()%2+5;
      if(d.hskill<0)
           d.hskill=0;
      workmoney=200+(d.cookskill*2+d.homework*2)/50;    
      pressanykey("旅馆还蛮顺利的唷..嗯嗯..");
    }    
    else if(class<80 && class>=60)
    {
      d.homework+=rand()%2+3;
      d.family+=rand()%2+3;
      d.hskill-=rand()%2+5;
      if(d.hskill<0)
           d.hskill=0;
      workmoney=150+(d.cookskill*2+d.homework*2)/50;    
      pressanykey("普普通通啦..可以更好的..加油..");      
    }    
    else if(class<60)
    {
      d.homework+=rand()%2+1;
      d.family+=rand()%2+1;
      d.hskill-=rand()%2+1;
      if(d.hskill<0)
           d.hskill=0;
      workmoney=100+(d.cookskill*2+d.homework*2)/50;    
      pressanykey("这个很糟糕喔..你这样不行啦..");      
    }        
    d.money+=workmoney;
    d.workC+=1;
    return 0; 
}  

int pip_job_workD()
{    
/*  ├————┼——————————————————————┤*/
/*  │农场    │体力 + 1 , 腕力 + 1 , 气质 - 1 , 疲劳 + 3   │*/
/*  ├————┼——————————————————————┤*/
/*  │农场    │若 体    力 - RND (疲劳) >=  30 则工作成功  │*/
/*  ├————┼——————————————————————┤*/    
    float class;
    long workmoney;
    
    workmoney=0;       
    class=(d.hp*100/d.maxhp)-d.tired;        
    d.maxhp+=rand()%3+2;
    d.wrist+=rand()%2+2;
    d.shit+=rand()%5+10;
    count_tired(5,15,"Y",100,1);
    d.hp-=(rand()%4+10);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    d.character-=rand()%3+4;
    if(d.character<0)
       d.character=0;
    show_job_pic(41);
    if(class>=95)
    {
      workmoney=250+(d.wrist*2+d.hp*2)/80;
      pressanykey("牛羊长的好好喔..希望你再来帮忙...");
    }
    else if(class<95 && class>=80)
    {
      workmoney=210+(d.wrist*2+d.hp*2)/80;
      pressanykey("呵呵..还不错喔..:)");
    }    
    else if(class<80 && class>=60)
    {
      workmoney=160+(d.wrist*2+d.hp*2)/80;
      pressanykey("普普通通啦..可以更好的..");      
    }    
    else if(class<60)
    {
      workmoney=120+(d.wrist*2+d.hp*2)/80;
      pressanykey("你不太适合农场的工作  -_-...");      
    }        
    d.money+=workmoney;
    d.workD+=1;
    return 0;
}   
         
int pip_job_workE()
{
/*  ├————┼——————————————————————┤*/
/*  │餐厅    │料理 + N , 战斗技术 - N , 疲劳 + 2          │*/
/*  ├————┼——————————————————————┤*/
/*  │餐厅    │若 烹饪技术 - RND (疲劳) >=  50 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    workmoney=0;       
    class=d.cookskill-d.tired;        
    d.maxhp+=rand()%2+1;
    d.shit+=rand()%4+12;
    count_tired(5,9,"Y",100,1);
    d.hp-=(rand()%4+8);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    show_job_pic(51);
    if(class>=80)
    {
      d.homework+=rand()%2+1;
      d.family+=rand()%2+1;
      d.hskill-=rand()%2+5;
      if(d.hskill<0)
           d.hskill=0;
      d.cookskill+=rand()%2+6;
      workmoney=250+(d.cookskill*2+d.homework*2+d.family*2)/80;        
      pressanykey("客人都说太好吃了..再来一盘吧...");
    }
    else if(class<80 && class>=60)
    {
      d.homework+=rand()%2+1;
      d.family+=rand()%2+1;
      d.hskill-=rand()%2+5;
      if(d.hskill<0)
           d.hskill=0;
      d.cookskill+=rand()%2+4;
      workmoney=200+(d.cookskill*2+d.homework*2+d.family*2)/80;        
      pressanykey("煮的还不错吃唷..:)");
    }    
    else if(class<60 && class>=30)
    {
      d.homework+=rand()%2+1;
      d.family+=rand()%2+1;
      d.hskill-=rand()%2+5;
      if(d.hskill<0)
           d.hskill=0;
      d.cookskill+=rand()%2+2;
      workmoney=150+(d.cookskill*2+d.homework*2+d.family*2)/80;            
      pressanykey("普普通通啦..可以更好的..");      
    }    
    else if(class<30)
    {
      d.homework+=rand()%2+1;
      d.family+=rand()%2+1;
      d.hskill-=rand()%2+5;
      if(d.hskill<0)
           d.hskill=0;
      d.cookskill+=rand()%2+1;
      workmoney=100+(d.cookskill*2+d.homework*2+d.family*2)/80;            
      pressanykey("你的厨艺待加强喔...");      
    }        
    d.money+=workmoney;
    d.workE+=1;
    return 0;
}     

int pip_job_workF()
{           
/*  ├————┼——————————————————————┤*/
/*  │教堂    │信仰 + 2 , 道德 + 1 , 罪孽 - 2 , 疲劳 + 1   │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    workmoney=0;
    class=(d.hp*100/d.maxhp)-d.tired;
    count_tired(5,7,"Y",100,1);
    d.love+=rand()%3+4;
    d.belief+=rand()%4+7;
    d.etchics+=rand()%3+7;
    d.shit+=rand()%3+3;
    d.hp-=rand()%3+5;
    d.offense-=rand()%4+7;
    if(d.offense<0)
       d.offense=0;
    show_job_pic(61);
    if(class>=75)
    {
      workmoney=100+(d.belief+d.etchics-d.offense)/20;
      pressanykey("钱很少 但看你这麽认真 给你多一点...");
    }
    else if(class<75 && class>=50)
    {
      workmoney=75+(d.belief+d.etchics-d.offense)/20;
      pressanykey("谢谢你的热心帮忙..:)");
    }    
    else if(class<50 && class>=25)
    {
      workmoney=50+(d.belief+d.etchics-d.offense)/20;
      pressanykey("你真的很有爱心啦..不过有点小累的样子...");      
    }    
    else if(class<25)
    {
      workmoney=25+(d.belief+d.etchics-d.offense)/20;
      pressanykey("来奉献不错..但也不能打混ㄚ....:(");      
    }        
    d.money+=workmoney;
    d.workF+=1;
    return 0;
}               

int pip_job_workG()
{   
/* ├————┼——————————————————————┤*/
/* │地摊    │体力 + 2 , 魅力 + 1 , 疲劳 + 3 ,谈吐 +1     │*/
/* ├————┼——————————————————————┤*/   
    float class;
    long workmoney;
    
    workmoney=0;
    workmoney=200+(d.charm*3+d.speech*2+d.toman)/50;
    count_tired(3,12,"Y",100,1);
    d.shit+=rand()%3+8;
    d.speed+=rand()%2;
    d.weight-=rand()%2;
    d.happy-=(rand()%3+7);
    d.satisfy-=rand()%3+5;
    d.hp-=(rand()%6+6);
    d.charm+=rand()%2+3;
    d.speech+=rand()%2+3;
    d.toman+=rand()%2+3;
    move(4,0);
    show_job_pic(71);
    pressanykey("摆地摊要躲警察啦..:p");
    d.money+=workmoney;
    d.workG+=1;
    return 0;
}   
    
int pip_job_workH()
{
/*  ├————┼——————————————————————┤*/
/*  │伐木场  │腕力 + 2 , 气质 - 2 , 疲劳 + 4              │*/
/*  ├————┼——————————————————————┤*/
/*  │伐木场  │若 腕    力 - RND (疲劳) >=  80 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    if((d.bbtime/60/30)<1) /*一岁才行*/
    {
      pressanykey("小鸡太小了,一岁以後再来吧...");
      return 0;
    }
    workmoney=0;       
    class=d.wrist-d.tired;        
    d.maxhp+=rand()%2+3;
    d.shit+=rand()%7+15;
    d.wrist+=rand()%3+4;
    count_tired(5,15,"Y",100,1);
    d.hp-=(rand()%4+10);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    d.character-=rand()%3+7;
    if(d.character<0)
       d.character=0;    
    show_job_pic(81);
    if(class>=70)
    {
      workmoney=350+d.wrist/20+d.maxhp/80;        
      pressanykey("你腕力很好唷..:)");
    }
    else if(class<70 && class>=50)
    {
      workmoney=300+d.wrist/20+d.maxhp/80;
      pressanykey("砍了不少树喔.....:)");
    }    
    else if(class<50 && class>=20)
    {
      workmoney=250+d.wrist/20+d.maxhp/80;
      pressanykey("普普通通啦..可以更好的..");      
    }    
    else if(class<20)
    {
      workmoney=200+d.wrist/20+d.maxhp/80;
      pressanykey("待加强喔..锻  再来吧....");      
    }        
    d.money+=workmoney;
    d.workH+=1;
    return 0;     
}

int pip_job_workI()
{
/*  ├————┼——————————————————————┤*/
/*  │美容院  │感受 + 1 , 腕力 - 1 , 疲劳 + 3              │*/
/*  ├————┼——————————————————————┤*/
/*  │美容院  │若 艺术修养 - RND (疲劳) >=  40 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;

    if((d.bbtime/60/30)<1) /*一岁才行*/
    {
      pressanykey("小鸡太小了,一岁以後再来吧...");
      return 0;
    }
    workmoney=0;    
    class=d.art-d.tired;        
    d.maxhp+=rand()%2;
    d.affect+=rand()%2+3;
    count_tired(3,11,"Y",100,1);
    d.shit+=rand()%4+8;
    d.hp-=(rand()%4+10);
    d.happy-=(rand()%3+4);
    d.satisfy-=rand()%3+4;
    d.wrist-=rand()%+3;
    if(d.wrist<0)
       d.wrist=0;
    /*show_job_pic(4);*/
    if(class>=80)
    {
      workmoney=400+d.art/10+d.affect/20;        
      pressanykey("客人都很喜欢让你做造型唷..:)");
    }
    else if(class<80 && class>=60)
    {
      workmoney=360+d.art/10+d.affect/20;        
      pressanykey("做的不错喔..颇有天份...:)");
    }    
    else if(class<60 && class>=40)
    {
      workmoney=320+d.art/10+d.affect/20;        
      pressanykey("马马虎虎啦..再加油一点..");      
    }    
    else if(class<40)
    {
      workmoney=250+d.art/10+d.affect/20;        
      pressanykey("待加强喔..以後再来吧....");      
    }        
    d.money+=workmoney;
    d.workI+=1;
    return 0;
}

int pip_job_workJ()
{           
/*  ├————┼——————————————————————┤*/
/*  │狩猎区  │体力 + 1 , 气质 - 1 , 母性 - 1 , 疲劳 + 3   │*/
/*  │        │战斗技术 + N                                │*/
/*  ├————┼——————————————————————┤*/
/*  │狩猎区  │若 体    力 - RND (疲劳) >=  80 ＆          │*/
/*  │        │若 智    力 - RND (疲劳) >=  40 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    float class1;
    long workmoney;

    /*两岁以上才行*/
    if((d.bbtime/60/30)<2)
    {
      pressanykey("小鸡太小了,两岁以後再来吧...");
      return 0;
    }          
    workmoney=0;
    class=(d.hp*100/d.maxhp)-d.tired;            
    class1=d.wisdom-d.tired;
    count_tired(5,15,"Y",100,1);
    d.shit+=rand()%4+13;
    d.weight-=(rand()%2+1);
    d.maxhp+=rand()%2+3;
    d.speed+=rand()%2+3;
    d.hp-=(rand()%6+8);
    d.character-=rand()%3+4;
    d.happy-=rand()%5+8;
    d.satisfy-=rand()%5+6;
    d.love-=rand()%3+4;
    if(d.character<0)
       d.character=0;
    if(d.love<0)
       d.love=0;
    move(4,0);
    show_job_pic(101);    
    if(class>=80 && class1>=80)
    {
       d.hskill+=rand()%2+7;
       workmoney=300+d.maxhp/50+d.hskill/20;
       pressanykey("你是完美的猎人..");
    }
    else if((class<75 && class>=50) && class1>=60 )  
    {
       d.hskill+=rand()%2+5;
       workmoney=270+d.maxhp/45+d.hskill/20;   
       pressanykey("收获还不错喔..可以饱餐一顿了..:)");
    }
    else if((class<50 && class>=25) && class1 >=40 )
    {
       d.hskill+=rand()%2+3;
       workmoney=240+d.maxhp/40+d.hskill/20;
       pressanykey("技术差强人意  再加油喔..");
    }    
    else if((class<25 && class>=0) && class1 >=20)  
    {
       d.hskill+=rand()%2+1;
       workmoney=210+d.maxhp/30+d.hskill/20;    
       pressanykey("狩猎是体力与智力的结合....");
    }    
    else if(class<0)
    {
       d.hskill+=rand()%2;
       workmoney=190+d.hskill/20;
       pressanykey("要多多锻  和增进智慧啦....");
    }    
    d.money=d.money+workmoney;        
    d.workJ+=1;
    return 0;   
}

int pip_job_workK()
{ 
/* ├————┼——————————————————————┤*/
/* │工地    │体力 + 2 , 魅力 - 1 , 疲劳 + 3              │*/
/* ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    /*两岁以上才行*/
    if((d.bbtime/60/30)<2)
    {
      pressanykey("小鸡太小了,两岁以後再来吧...");
      return 0;
    }
    workmoney=0;
    class=(d.hp*100/d.maxhp)-d.tired;            
    count_tired(5,15,"Y",100,1);
    d.shit+=rand()%4+16;
    d.weight-=(rand()%2+2);
    d.maxhp+=rand()%2+1;
    d.speed+=rand()%2+2;
    d.hp-=(rand()%6+10);
    d.charm-=rand()%3+6;
    d.happy-=(rand()%5+10);
    d.satisfy-=rand()%5+6;
    if(d.charm<0)
       d.charm=0;
    move(4,0);
    show_job_pic(111);    
    if(class>=75)
    {
       workmoney=250+d.maxhp/50;
       pressanykey("工程很完美  谢谢你了..");
    }
    else if(class<75 && class>=50)  
    {
       workmoney=220+d.maxhp/45;    
       pressanykey("工程尚称顺利  辛苦你了..");
    }
    else if(class<50 && class>=25)  
    {
       workmoney=200+d.maxhp/40;    
       pressanykey("工程差强人意  再加油喔..");
    }    
    else if(class<25 && class>=0)  
    {
       workmoney=180+d.maxhp/30;    
       pressanykey("ㄜ  待加强待加强....");
    }    
    else if(class<0)
    {
       workmoney=160;
       pressanykey("下次体力好一点..疲劳度低一点再来....");
    }
    
    d.money=d.money+workmoney;
    d.workK+=1;
    return 0;
}

int pip_job_workL()
{   
/*  ├————┼——————————————————————┤*/
/*  │墓园    │抗魔能力 + N , 感受 + 1 , 魅力 - 1          │*/
/*  │        │疲劳 + 2                                    │*/
/*  ├————┼——————————————————————┤*/
    float class;
    float class1;
    long workmoney;
    
    /*叁岁才行*/
    if((d.bbtime/60/30)<3)
    {
      pressanykey("小鸡现在还太小了,叁岁以後再来吧...");
      return 0;
    }
    workmoney=0;
    class=(d.hp*100/d.maxhp)-d.tired;
    class1=d.belief-d.tired;
    d.shit+=rand()%5+8;
    d.maxmp+=rand()%2;
    d.affect+=rand()%2+2;    
    d.brave+=rand()%2+2;    
    count_tired(5,12,"Y",100,1);    
    d.hp-=(rand()%3+7);
    d.happy-=(rand()%4+6);
    d.satisfy-=rand()%3+5;    
    d.charm-=rand()%3+6;         
    if(d.charm<0)
       d.charm=0;
    show_job_pic(121);                
    if(class>=75 && class1>=75)
    {
      d.mresist+=rand()%2+7;
      workmoney=200+(d.affect+d.brave)/40;
      pressanykey("守墓成功喔  给你多点钱");
    }
    else if((class<75 && class>=50)&& class1 >=50)
    {
      d.mresist+=rand()%2+5;
      workmoney=150+(d.affect+d.brave)/50;      
      pressanykey("守墓还算成功喔..谢啦..");    
    }    
    else if((class<50 && class>=25)&& class1 >=25)
    {
      d.mresist+=rand()%2+3;
      workmoney=120+(d.affect+d.brave)/60;
      pressanykey("守墓还算差强人意喔..加油..");    
    }    
    else
    {
      d.mresist+=rand()%2+1;
      workmoney=80+(d.affect+d.brave)/70;
      pressanykey("我也不方便说啥了..请再加油..");    
    }           
    if(rand()%10==5)
    {
       pressanykey("真是倒楣  竟遇到死神魔..");
       pip_fight_bad(12);
    }
    d.money+=workmoney;
    d.workL+=1;
    return 0;   
}

int pip_job_workM()
{
/*  ├————┼——————————————————————┤*/
/*  │家庭教师│道德 + 1 , 母性 + N , 魅力 - 1 , 疲劳 + 7   │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;    

    if((d.bbtime/60/30)<4)
    {
      pressanykey("小鸡太小了,四岁以後再来吧...");
      return 0;
    }    
    workmoney=0;
    class=(d.hp*100/d.maxhp)-d.tired;            
    workmoney=50+d.wisdom/20+d.character/20;
    count_tired(5,10,"Y",100,1);
    d.shit+=rand()%3+8;
    d.character+=rand()%2;
    d.wisdom+=rand()%2;
    d.happy-=(rand()%3+6);
    d.satisfy-=rand()%3+5;
    d.hp-=(rand()%3+8);
    d.money=d.money+workmoney;
    move(4,0);
    show_job_pic(131);
    pressanykey("家教轻松 当然钱就少一点罗");
    d.workM+=1;
    return;
}

int pip_job_workN()
{   
/*  ├————┼——————————————————————┤*/
/*  │酒店    │烹饪技巧 + N , 谈话技巧 + N , 智力 - 2      │*/
/*  │        │疲劳 + 5                                    │*/
/*  ├————┼——————————————————————┤*/
/*  │酒店    │若 体    力 - RND (疲劳) >=  60 ＆          │*/
/*  │        │若 魅    力 - RND (疲劳) >=  50 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    float class1;
    long workmoney;
    
    if((d.bbtime/60/30)<5)
    {
      pressanykey("小鸡太小了,五岁以後再来吧...");
      return 0;
    }         
    workmoney=0;
    class=(d.hp*100/d.maxhp)-d.tired;
    class1=d.charm-d.tired;
    d.shit+=rand()%5+5;
    count_tired(5,14,"Y",100,1);
    d.hp-=(rand()%3+5);
    d.social-=rand()%5+6;
    d.happy-=(rand()%4+6);
    d.satisfy-=rand()%3+5;    
    d.wisdom-=rand()%3+4;         
    if(d.wisdom<0)
       d.wisdom=0;
    /*show_job_pic(6);*/
    if(class>=75 && class1>=75)
    {
      d.cookskill+=rand()%2+7;
      d.speech+=rand()%2+5;
      workmoney=500+(d.charm)/5;
      pressanykey("你很红唷  :)");
    }
    else if((class<75 && class>=50)&& class1 >=50)
    {
      d.cookskill+=rand()%2+5;
      d.speech+=rand()%2+5;
      workmoney=400+(d.charm)/5;    
      pressanykey("蛮受欢迎的耶....");    
    }    
    else if((class<50 && class>=25)&& class1 >=25)
    {
      d.cookskill+=rand()%2+4;
      d.speech+=rand()%2+3;
      workmoney=300+(d.charm)/5;    
      pressanykey("很平凡啦..但马马虎虎...");    
    }    
    else
    {
      d.cookskill+=rand()%2+2;
      d.speech+=rand()%2+2;
      workmoney=200+(d.charm)/5;    
      pressanykey("你的媚力不够啦..请加油....");    
    }           
    d.money+=workmoney;
    d.workN+=1;
    return 0;        
}

int pip_job_workO()
{         
/*  ├————┼——————————————————————┤*/
/*  │酒家    │魅力 + 2 , 罪孽 + 2 , 道德 - 3 , 信仰 - 3   │*/
/*  │        │待人接物 - N , 和父亲的关系 - N , 疲劳 + 12 │*/
/*  ├————┼——————————————————————┤*/
/*  │酒家    │若 魅    力 - RND (疲劳) >=  70 则工作成功  │*/
/*  ├————┼——————————————————————┤*/
    float class;
    long workmoney;
    
    if((d.bbtime/60/30)<4)
    {
      pressanykey("小鸡太小了,四岁以後再来吧...");
      return 0;
    }                 
    workmoney=0;
    class=d.charm-d.tired;
    d.shit+=rand()%5+14;
    d.charm+=rand()%3+8;
    d.offense+=rand()%3+8;
    count_tired(5,22,"Y",100,1);
    d.hp-=(rand()%3+8);
    d.social-=rand()%6+12;
    d.happy-=(rand()%4+8);
    d.satisfy-=rand()%3+8;    
    d.etchics-=rand()%6+10;
    d.belief-=rand()%6+10;
    if(d.etchics<0)
       d.etchics=0;
    if(d.belief<0)
       d.belief=0;       
    
    /*show_job_pic(6);*/
    if(class>=75)
    {
      d.relation-=rand()%5+12;
      d.toman-=rand()%5+12;
      workmoney=600+(d.charm)/5;
      pressanykey("你是本店的红牌唷  :)");
    }
    else if(class<75 && class>=50)
    {
      d.relation-=rand()%5+8;
      d.toman-=rand()%5+8;
      workmoney=500+(d.charm)/5;    
      pressanykey("你蛮受欢迎的耶..:)");    
    }    
    else if(class<50 && class>=25)
    {
      d.relation-=rand()%5+5;
      d.toman-=rand()%5+5;
      workmoney=400+(d.charm)/5;
      pressanykey("你很平凡..但马马虎虎啦...");    
    }    
    else
    {
      d.relation-=rand()%5+1;
      d.toman-=rand()%5+1;
      workmoney=300+(d.charm)/5;    
      pressanykey("唉..你的媚力不够啦....");    
    }           
    d.money+=workmoney;
    if(d.relation<0)
       d.relation=0;
    if(d.toman<0)
       d.toman=0;
    d.workO+=1;
    return 0;
}        
    
int pip_job_workP()
{
/*  ├————┼——————————————————————┤*/
/*  │大夜总会│魅力 + 3 , 罪孽 + 1 , 气质 - 2 , 智力 - 1   │*/
/*  │        │待人接物 - N , 疲劳 + 8                     │*/
/*  ├————┼——————————————————————┤*/
/*  │大夜总会│若 魅    力 - RND (疲劳) >=  70 ＆          │*/
/*  │        │若 艺术修养 - RND (疲劳) >=  30 则工作成功  │*/
/*  └————┴——————————————————————┘*/
    float class;
    float class1;
    long workmoney;
    
    if((d.bbtime/60/30)<6)
    {
      pressanykey("小鸡太小了,六岁以後再来吧...");
      return;
    }                
    workmoney=0;
    class=d.charm-d.tired;
    class1=d.art-d.tired;
    d.shit+=rand()%5+7;
    d.charm+=rand()%3+8;
    d.offense+=rand()%3+8;
    count_tired(5,22,"Y",100,1);
    d.hp-=(rand()%3+8);
    d.social-=rand()%6+12;
    d.happy-=(rand()%4+8);
    d.satisfy-=rand()%3+8;    
    d.character-=rand()%3+8;
    d.wisdom-=rand()%3+5;
    if(d.character<0)
       d.character=0;
    if(d.wisdom<0)
       d.wisdom=0;       
    /*show_job_pic(6);*/
    if(class>=75 && class1>30)
    {
      d.speech+=rand()%5+12;
      d.toman-=rand()%5+12;
      workmoney=1000+(d.charm)/5;
      pressanykey("你是夜总会最闪亮的星星唷  :)");
    }
    else if((class<75 && class>=50) && class1>20)
    {
      d.speech+=rand()%5+8;
      d.toman-=rand()%5+8;
      workmoney=800+(d.charm)/5;    
      pressanykey("嗯嗯..你蛮受欢迎的耶..:)");    
    }    
    else if((class<50 && class>=25) && class1>10)
    {
      d.speech+=rand()%5+5;
      d.toman-=rand()%5+5;
      workmoney=600+(d.charm)/5;
      pressanykey("你要加油了啦..但普普啦...");    
    }    
    else
    {
      d.speech+=rand()%5+1;
      d.toman-=rand()%5+1;
      workmoney=400+(d.charm)/5;    
      pressanykey("唉..你不行啦....");    
    }           
    d.money+=workmoney;
    if(d.toman<0)
       d.toman=0;
    d.workP+=1;
    return;    
}
