/*---------------------------------------------------------------------------*/
/* ϵͳѡ��:��������  С������  �ر����                                     */
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
"û��װ��",
"�ܽ�ñ��", 
"ţƤСñ",
"  ��ȫñ",
"����ͷ��",
"ħ������",
"�ƽ�ʥ��"};


char weaponrhand[10][10]={
"û��װ��",
"��ľ��",  
"��������",
"��ͭ��",  
"���׽�", 
"������", 
"���齣", 
"ʨͷ����",
"������",  
"�ƽ�ʥ��"
};  

char weaponlhand[8][10]={
"û��װ��",
"��ľ��", 
"��������",
"ľ��",
"����ֶ�",
"�׽�֮��",
"ħ����",
"�ƽ�ʥ��"
};


char weaponbody[7][10]={
"û��װ��",
"�ܽ��м�",
"�ؼ�Ƥ��",
"��������",
"ħ������",
"�׽����",
"�ƽ�ʥ��"};

char weaponfoot[8][12]={
"û��װ��",
"�ܽ���Ь",
"����ľ��",
"�ؼ���Ь",
"NIKE�˶�Ь",
"����Ƥѥ",
"����ħѥ",
"�ƽ�ʥѥ"
};

int 
pip_system_freepip()
{
      char buf[256];
      move(b_lines-1, 0);
      clrtoeol();
#ifdef MAPLE
      getdata(b_lines-1,1, "���Ҫ������(y/N)", buf, 2, 1, 0);
#else
      getdata(b_lines-1,1, "���Ҫ������(y/N)", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
      if (buf[0]!='y'&&buf[0]!='Y') return 0;
      sprintf(buf,"%s �����ĵ� %s ������~",d.name,cuser.userid);
      pressanykey(buf);
      d.death=2;
      pipdie("[1;31m�����Ķ���:~~[0m",2);
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
     prints("[1;44m  ������Ŀ  [46m[1]������ʦ [2]�������� [3]������                                [0m");
     pipkey=egetch();
     
     switch(pipkey)
     {
     case '1':
       move(b_lines-1,0);
       clrtobot();
#ifdef MAPLE
       getdata(b_lines-1, 1, "��С������ȡ�������֣�", buf, 11, DOECHO,NULL);
#else
       getdata(b_lines-1, 1, "��С������ȡ�������֣�", buf, 11, DOECHO,YEA);
#endif  // END MAPLE
       if(!buf[0])
       {
         pressanykey("��һ�������������  :)");
         break;
       }
       else
       {
        strcpy(oldname,d.name);
        strcpy(d.name,buf);
        /*������¼*/
        now=time(0);
        sprintf(buf, "[1;37m%s %-11s��С�� [%s] ������ [%s] [0m\n", Cdate(&now), cuser.userid,oldname,d.name);
        pip_log_record(buf);
        pressanykey("����  ��һ���µ������...");
       }
       break;
       
     case '2':  /*����*/
       move(b_lines-1,0);
       clrtobot();
       /*1:�� 2:ĸ */
       if(d.sex==1)
       { 
         oldchoice=2; /*��-->ĸ*/
         move(b_lines-1, 0);
         prints("[1;33m��С����[32m��[33m���Գ�[35m��[33m���� [37m[y/N][0m");
       }
       else
       { 
         oldchoice=1; /*ĸ-->��*/
         move(b_lines-1, 0); 
         prints("[1;33m��С����[35m��[33m���Գ�[35m��[33m���� [37m[y/N][0m");
       }
       move(b_lines,0);
       prints("[1;44m  ������Ŀ  [46m[1]������ʦ [2]�������� [3]������                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         /*������¼*/
         now=time(0);
         if(d.sex==1)
           sprintf(buf,"[1;37m%s %-11s��С�� [%s] �ɡ���Գɡ���[0m\n",Cdate(&now), cuser.userid,d.name);
         else
           sprintf(buf,"[1;37m%s %-11s��С�� [%s] �ɡ���Գɡ���[0m\n",Cdate(&now), cuser.userid,d.name);           
         pip_log_record(buf);
         pressanykey("�����������...");       
         d.sex=oldchoice;
       }  
       break;
       
     case '3':
       move(b_lines-1,0);
       clrtobot();
       /*1:��Ҫ��δ�� 4:Ҫ��δ�� */
       oldchoice=d.wantend;
       if(d.wantend==1 || d.wantend==2 || d.wantend==3)
       { 
         oldchoice+=3; /*û��-->��*/
         move(b_lines-1, 0); 
         prints("[1;33m��С����Ϸ�ĳ�[32m[��20����][33m? [37m[y/N][0m");
	 sprintf(buf,"С����Ϸ�趨��[��20����]..");         
       }
       else
       { 
         oldchoice-=3; /*��-->û��*/
         move(b_lines-1, 0); 
         prints("[1;33m��С����Ϸ�ĳ�[32m[û��20����][33m? [37m[y/N][0m");
         sprintf(buf,"С����Ϸ�趨��[û��20����]..");
       }
       move(b_lines,0);
       prints("[1;44m  ������Ŀ  [46m[1]������ʦ [2]�������� [3]������                                [0m");
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
pip_data_list()  /*��С��������ϸ����*/
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
  prints("       [1;33m��������    ������  ����������  ������  [m\n");
  prints("       [0;37m��      ���� ��   �������������� ��   ��[m\n");
  prints("       [1;37m��      ��������  ��  ����    ������  ��[m\n");
  prints("       [1;34m��������  ����������  ����    ����������[32m......................[m");
  do
  { clrchyiuan(5,23);
    switch(page)
    {
     case 1:
       move(5,0);
       sprintf(buf,
       "[1;31m ����[41;37m �������� [0;1;31m��������������������������������������������������������������[m\n");  
       prints(buf);
  
       sprintf(buf,
       "[1;31m ��[33m����    �� :[37m %-10s [33m����    �� :[37m %02d/%02d/%02d   [33m����    �� :[37m %-2d         [31m��[m\n",
       d.name,d.year%100,d.month,d.day,tm);
       prints(buf);  
  
       sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
       sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);  
       sprintf(buf,
       "[1;31m ��[33m����    �� :[37m %-5d(�׿�)[33m����    �� :[37m %-11s[33m����    �� :[37m %-11s[31m��[m\n",
       d.weight,inbuf1,inbuf2);
       prints(buf);  
  
       sprintf(buf,
       "[1;31m ��[33m��ƣ    �� :[37m %-3d        [33m����    �� :[37m %-3d        [33m����    �� :[37m %-3d        [31m��[m\n",
       d.tired,d.sick,d.shit);
       prints(buf);  
   
       sprintf(buf,  
       "[1;31m ��[33m����    �� :[37m %-7d    [33m�����ӹ�ϵ :[37m %-7d    [33m����    Ǯ :[37m %-11d[31m��[m\n",
       d.wrist,d.relation,d.money);
       prints(buf);  
  
       sprintf(buf,  
       "[1;31m ����[41;37m �������� [0;1;31m��������������������������������������������������������������[m\n");
       prints(buf);  
   
       sprintf(buf,   
       "[1;31m ��[33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [31m��[m\n",
       d.character,d.wisdom,d.love);
       prints(buf);  
   
       sprintf(buf, 
       "[1;31m ��[33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [31m��[m\n",
       d.art,d.etchics,d.homework);
       prints(buf);  
 
       sprintf(buf, 
       "[1;31m ��[33m����    �� :[37m %-10d [33m��Ӧ    �� :[37m %-10d [33m����    � :[37m %-10d [31m��[m\n",
       d.manners,d.speech,d.cookskill);
       prints(buf);    
 
       sprintf(buf,  
       "[1;31m ����[41;37m ״̬���� [0;1;31m��������������������������������������������������������������[m\n");
       prints(buf);  
 
       sprintf(buf, 
       "[1;31m ��[33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [31m��[m\n",
       d.happy,d.satisfy,d.toman);
       prints(buf);
  
       sprintf(buf, 
       "[1;31m ��[33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [31m��[m\n",
       d.charm,d.brave,d.belief);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ��[33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [33m            [37m            [31m��[m\n",
       d.offense,d.affect);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ����[41;37m �������� [0;1;31m��������������������������������������������������������������[m\n");
       prints(buf);  

       sprintf(buf, 
       "[1;31m ��[33m���罻���� :[37m %-10d [33m��ս������ :[37m %-10d [33m��ħ������ :[37m %-10d [31m��[m\n",
       d.social,d.hexp,d.mexp);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ��[33m���������� :[37m %-10d [33m            [37m            [33m            [37m            [31m��[m\n",
       d.family);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ����������������������������������������������������������������������������[m\n");
       prints(buf);  
       
       move(b_lines-1,0);       
       sprintf(buf, 
       "                                                              [1;36m��һҳ[37m/[36m����ҳ[m\n");
       prints(buf);  
       break;

     case 2:
       move(5,0);
       sprintf(buf, 
       "[1;31m ����[41;37m ��Ʒ���� [0;1;31m��������������������������������������������������������������[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ��[33m��ʳ    �� :[37m %-10d [33m����    ʳ :[37m %-10d [33m���� �� �� :[37m %-10d [31m��[m\n",
       d.food,d.cookie,d.bighp);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ��[33m��ҩ    �� :[37m %-10d [33m����    �� :[37m %-10d [33m����    �� :[37m %-10d [31m��[m\n",
       d.medicine,d.book,d.playtool);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ����[41;37m ��Ϸ���� [0;1;31m��������������������������������������������������������������[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ��[33m���� ȭ Ӯ :[37m %-10d [33m���� ȭ �� :[37m %-10d                         [31m��[m\n",
       d.winn,d.losee);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ����[41;37m �������� [0;1;31m��������������������������������������������������������������[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ��[33m���� �� �� :[37m %-10d [33m���� �� �� :[37m %-10d [33m���� �� ֵ :[37m %-10d [31m��[m\n",
       d.attack,d.resist,d.speed);
       prints(buf);  
       sprintf(buf, 
       "[1;31m ��[33m����ħ���� :[37m %-10d [33m��ս������ :[37m %-10d [33m��ħ������ :[37m %-10d [31m��[m\n",
       d.mresist,d.hskill,d.mskill);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ��[33m��ͷ��װ�� :[37m %-10s [33m������װ�� :[37m %-10s [33m������װ�� :[37m %-10s [31m��[m\n",
       weaponhead[d.weaponhead],weaponrhand[d.weaponrhand],weaponlhand[d.weaponlhand]);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ��[33m������װ�� :[37m %-10s [33m���Ų�װ�� :[37m %-10s [33m            [37m            [31m��[m\n",
       weaponbody[d.weaponbody],weaponfoot[d.weaponfoot]);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ����������������������������������������������������������������������������[m\n");
       prints(buf); 

       move(b_lines-1,0);
       sprintf(buf, 
       "                                                              [1;36m�ڶ�ҳ[37m/[36m����ҳ[m\n");
       prints(buf);          
       break;
    }
    move(b_lines,0);
    sprintf(buf,"[1;44;37m  ����ѡ��  [46m  [��/PAGE UP]����һҳ [��/PAGE DOWN]����һҳ [Q]�뿪:            [m");
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
          my_write(currutmp->msgs[0].last_pid, "ˮ�򶪻�ȥ��");
        }
        break;
#endif  // END MAPLE
    }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
  return 0;
}