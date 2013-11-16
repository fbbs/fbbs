/* 电子鸡 小码力..几a几b游戏.□ */

/* Writed by Birdman From 140.116.102.125 创意哇哈哈*/
  


#include "bbs.h"
#define DATA_FILE  "chicken"


char
//% *cstate[10]={"我在吃饭","偷吃零食","拉便便","笨蛋..输给鸡?","哈..赢小鸡也没多光荣"
*cstate[10]={"\xce\xd2\xd4\xda\xb3\xd4\xb7\xb9","\xcd\xb5\xb3\xd4\xc1\xe3\xca\xb3","\xc0\xad\xb1\xe3\xb1\xe3","\xb1\xbf\xb5\xb0..\xca\xe4\xb8\xf8\xbc\xa6?","\xb9\xfe..\xd3\xae\xd0\xa1\xbc\xa6\xd2\xb2\xc3\xbb\xb6\xe0\xb9\xe2\xc8\xd9"
            //% ,"没食物啦..","疲劳全消!"};
            ,"\xc3\xbb\xca\xb3\xce\xef\xc0\xb2..","\xc6\xa3\xc0\xcd\xc8\xab\xcf\xfb!"};
//% char *cage[9]={"诞生","周岁","幼年","少年","青年","活力","壮年","中年"};
char *cage[9]={"\xb5\xae\xc9\xfa","\xd6\xdc\xcb\xea","\xd3\xd7\xc4\xea","\xc9\xd9\xc4\xea","\xc7\xe0\xc4\xea","\xbb\xee\xc1\xa6","\xd7\xb3\xc4\xea","\xd6\xd0\xc4\xea"};
//% char *menu[8]={"游戏","运动","调教计能","买卖工具","清理鸡舍"};
char *menu[8]={"\xd3\xce\xcf\xb7","\xd4\xcb\xb6\xaf","\xb5\xf7\xbd\xcc\xbc\xc6\xc4\xdc","\xc2\xf2\xc2\xf4\xb9\xa4\xbe\xdf","\xc7\xe5\xc0\xed\xbc\xa6\xc9\xe1"};

time_t birth;
int weight,satis,mon,day,age,angery,sick,oo,happy,clean,tiredstrong,play;
int winn,losee,last,chictime,agetmp,food,zfood;
char Name[20];
FILE *cfp;

int chicken_main()
{
   FILE *fp;
   time_t now = time(0);
   struct tm *ptime;
   char fname[50];
   
   agetmp=1; 
   set_user_status(ST_CHICK);
   time(&now);
   ptime = localtime(&now);
   setuserfile(fname, DATA_FILE);
   if ((fp = fopen(fname, "r+")) == NULL){
      creat_a_egg();
      last=1;
      fp = fopen(fname, "r");
      fscanf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s "
         ,&weight,&mon,&day,&satis,&age,&oo,&happy,&clean,&tiredstrong,&play
         ,&winn,&losee,&food,&zfood,Name);
      fclose(fp); 
   } else {
      last=0;
      fscanf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s "
         ,&weight,&mon,&day,&satis,&age,&oo,&happy,&clean,&tiredstrong,&play
         ,&winn,&losee,&food,&zfood,Name);
      fclose(fp);
   }
/*□*/
  if(day<(ptime->tm_mon+1))
   { age=ptime->tm_mday;
    age=age+31-mon;}
  else
    age=ptime->tm_mday-mon;

  show_chicken();
   select_menu();
    fp = fopen(fname, "r+");
 /* if (last!=1)
    { */
        fprintf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s "
          ,weight,mon,day,satis,age,oo,happy,clean,tiredstrong,play
          ,winn,losee,food,zfood,Name);

  fclose(fp);
  return 0;
}

int creat_a_egg()
{
   char fname[50];
   struct tm *ptime;
   FILE *fp;
   time_t now; 
   
   time(&now);
   ptime = localtime(&now);
   move(2,0);
   clrtobot();
   while(strlen(Name)<1){
      //% strcpy(Name,"宝宝");
      strcpy(Name,"\xb1\xa6\xb1\xa6");
      //% getdata(2, 0, "帮小鸡取个好名字：", Name, 21, DOECHO,NA);
      getdata(2, 0, "\xb0\xef\xd0\xa1\xbc\xa6\xc8\xa1\xb8\xf6\xba\xc3\xc3\xfb\xd7\xd6\xa3\xba", Name, 21, DOECHO,NA);
   } 
   setuserfile(fname, DATA_FILE);
   fp = fopen(fname, "w");
   fprintf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s "
      ,ptime->tm_hour*2,ptime->tm_mday,ptime->tm_mon+1
      ,0,0,0,0,0,0,0,0,0,10,5,Name);
   fclose(fp);
   if((fp = fopen("game/chicken", "a"))==NULL){
      prints("Error!!cannot open then 'game/chicken'!\n");
      return ;
   }   
   //% fprintf(fp,"[32m%s[m 在 [34;43m[%d/%d  %d:%02d][m  养了一只叫 [33m%s[m 的小鸡\n",
   fprintf(fp,"[32m%s[m \xd4\xda [34;43m[%d/%d  %d:%02d][m  \xd1\xf8\xc1\xcb\xd2\xbb\xd6\xbb\xbd\xd0 [33m%s[m \xb5\xc4\xd0\xa1\xbc\xa6\n",
          currentuser.userid,ptime->tm_mon + 1, ptime->tm_mday,
          ptime->tm_hour, ptime->tm_min,Name);
   fclose(fp);
   return;
}

int show_chicken()
{
  //int diff;
  /*int chictime;*/

  //diff = (time(0)/* - login_start_time*/) / 120;

  if(chictime>=200)
      {
        weight-=5;
        clean+=3;
        if(tiredstrong>2)
        tiredstrong-=2;
       }
    /*food=food-diff*3;*/
  if(weight<0)
    death();
/*  if((diff-age)>1 && agetmp) 还是有问题
   {age++;
    agetmp=0;}
*/
  move(1,0);
  clrtobot();
  prints(
     "[33mName:%s[m"
     //% "  [45mAge :%d岁[m"
     "  [45mAge :%d\xcb\xea[m"
     //% "  重量:%d"
     "  \xd6\xd8\xc1\xbf:%d"
     //% "  食物:%d"
     "  \xca\xb3\xce\xef:%d"
     //% "  零食:%d"
     "  \xc1\xe3\xca\xb3:%d"
     //% "  疲劳:%d"
     "  \xc6\xa3\xc0\xcd:%d"
     //% "  便便:%d\n"
     "  \xb1\xe3\xb1\xe3:%d\n"
     //% "  生日:%d月%d日"
     "  \xc9\xfa\xc8\xd5:%d\xd4\xc2%d\xc8\xd5"
     //% "  糖糖:%8d"
     "  \xcc\xc7\xcc\xc7:%8d"
     //% "  快乐度:%d"
     "  \xbf\xec\xc0\xd6\xb6\xc8:%d"
     //% "  满意度:%d",
     "  \xc2\xfa\xd2\xe2\xb6\xc8:%d",
    // "  大补丸:%d\n",
     Name,age,weight,food,zfood,tiredstrong,clean,day
     ,mon,currentuser.money,happy,satis);//,oo);

  move(3,0);
 if(age<=16){
  switch(age)
  {
     case 0:
     case 1:
      //% outs("  ●●●●\n"
      outs("  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "●  ●●  ●\n"
"\xa1\xf1  \xa1\xf1\xa1\xf1  \xa1\xf1\n"
//% "●●●●●●\n"
"\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "●●    ●●\n"
"\xa1\xf1\xa1\xf1    \xa1\xf1\xa1\xf1\n"
//% "●●●●●●\n"
"\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●●●●   ");
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1   ");

      break;
     case 2:
     case 3:
      //% outs("    ●●●●●●\n"
      outs("    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●            ●\n"
"  \xa1\xf1            \xa1\xf1\n"
//% "●    ●    ●    ●\n"
"\xa1\xf1    \xa1\xf1    \xa1\xf1    \xa1\xf1\n"
//% "●                ●\n"
"\xa1\xf1                \xa1\xf1\n"
//% "●      ●●      ●\n"
"\xa1\xf1      \xa1\xf1\xa1\xf1      \xa1\xf1\n"
//% "●                ●\n"
"\xa1\xf1                \xa1\xf1\n"
//% "●                ●\n"
"\xa1\xf1                \xa1\xf1\n"
//% "  ●            ●\n"
"  \xa1\xf1            \xa1\xf1\n"
//% "    ●●●●●●   ");
"    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1   ");

       break;
     case 4:
     case 5:

        //% outs("      ●●●●●●\n"
        outs("      \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "    ●            ●\n"
"    \xa1\xf1            \xa1\xf1\n"
//% "  ●  ●        ●  ●\n"
"  \xa1\xf1  \xa1\xf1        \xa1\xf1  \xa1\xf1\n"
//% "  ●                ●\n"
"  \xa1\xf1                \xa1\xf1\n"
//% "  ●      ●●      ●\n"
"  \xa1\xf1      \xa1\xf1\xa1\xf1      \xa1\xf1\n"
//% "●●●    ●●      ●●\n"
"\xa1\xf1\xa1\xf1\xa1\xf1    \xa1\xf1\xa1\xf1      \xa1\xf1\xa1\xf1\n"
//% "  ●                ●\n"
"  \xa1\xf1                \xa1\xf1\n"
//% "  ●                ●\n"
"  \xa1\xf1                \xa1\xf1\n"
//% "    ●  ●●●●  ●\n"
"    \xa1\xf1  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1  \xa1\xf1\n"
//% "      ●      ●  ●\n"
"      \xa1\xf1      \xa1\xf1  \xa1\xf1\n"
//% "                ●    ");
"                \xa1\xf1    ");
        break;
        case 6:
        case 7:
         //% outs("   ●    ●●●●●●\n"
         outs("   \xa1\xf1    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "●  ●●  ●        ●\n"
"\xa1\xf1  \xa1\xf1\xa1\xf1  \xa1\xf1        \xa1\xf1\n"
//% "●              ●    ●\n"
"\xa1\xf1              \xa1\xf1    \xa1\xf1\n"
//% "  ●●●                ●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1                \xa1\xf1\n"
//% "●                      ●\n"
"\xa1\xf1                      \xa1\xf1\n"
//% "●  ●●                ●\n"
"\xa1\xf1  \xa1\xf1\xa1\xf1                \xa1\xf1\n"
//% "  ●  ●                ●\n"
"  \xa1\xf1  \xa1\xf1                \xa1\xf1\n"
//% "      ●                ●\n"
"      \xa1\xf1                \xa1\xf1\n"
//% "        ●            ●\n"
"        \xa1\xf1            \xa1\xf1\n"
//% "          ●●●●●●        ");
"          \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1        ");
        break;

        case 8:
        case 9:
        case 10:
         //% outs("    ●●          ●●\n"
         outs("    \xa1\xf1\xa1\xf1          \xa1\xf1\xa1\xf1\n"
//% "  ●●●●      ●●●●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1      \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●●●●●●●●●●●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●                  ●\n"
"  \xa1\xf1                  \xa1\xf1\n"
//% "  ●    ●      ●    ●\n"
"  \xa1\xf1    \xa1\xf1      \xa1\xf1    \xa1\xf1\n"
//% "●                      ●\n"
"\xa1\xf1                      \xa1\xf1\n"
//% "●        ●●●        ●\n"
"\xa1\xf1        \xa1\xf1\xa1\xf1\xa1\xf1        \xa1\xf1\n"
//% "  ●                  ●\n"
"  \xa1\xf1                  \xa1\xf1\n"
//% "●    ●          ●  ●\n"
"\xa1\xf1    \xa1\xf1          \xa1\xf1  \xa1\xf1\n"
//% "  ●●            ●●●\n"
"  \xa1\xf1\xa1\xf1            \xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●                  ●\n"
"  \xa1\xf1                  \xa1\xf1\n"
//% "    ●              ●\n"
"    \xa1\xf1              \xa1\xf1\n"
//% "      ●  ●●●  ●\n"
"      \xa1\xf1  \xa1\xf1\xa1\xf1\xa1\xf1  \xa1\xf1\n"
//% "      ●  ●    ●\n"
"      \xa1\xf1  \xa1\xf1    \xa1\xf1\n"
//% "        ●               ");
"        \xa1\xf1               ");

        break;

        case 11:
        case 12:
        //% outs("        ●●●●●●\n"
        outs("        \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "      ●    ●    ●●\n"
"      \xa1\xf1    \xa1\xf1    \xa1\xf1\xa1\xf1\n"
//% "    ●  ●      ●  ●●\n"
"    \xa1\xf1  \xa1\xf1      \xa1\xf1  \xa1\xf1\xa1\xf1\n"
//% "  ●●              ●●●\n"
"  \xa1\xf1\xa1\xf1              \xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "●              ●    ●●\n"
"\xa1\xf1              \xa1\xf1    \xa1\xf1\xa1\xf1\n"
//% "●●●●●●●●      ●●\n"
"\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1      \xa1\xf1\xa1\xf1\n"
//% "  ●                  ●●\n"
"  \xa1\xf1                  \xa1\xf1\xa1\xf1\n"
//% "    ●        ●  ●    ●\n"
"    \xa1\xf1        \xa1\xf1  \xa1\xf1    \xa1\xf1\n"
//% "    ●        ●  ●    ●\n"
"    \xa1\xf1        \xa1\xf1  \xa1\xf1    \xa1\xf1\n"
//% "    ●          ●      ●\n"
"    \xa1\xf1          \xa1\xf1      \xa1\xf1\n"
//% "      ●              ●\n"
"      \xa1\xf1              \xa1\xf1\n"
//% "        ●  ●●●  ●\n"
"        \xa1\xf1  \xa1\xf1\xa1\xf1\xa1\xf1  \xa1\xf1\n"
//% "        ●  ●  ●  ●\n"
"        \xa1\xf1  \xa1\xf1  \xa1\xf1  \xa1\xf1\n"
//% "          ●      ●             ");
"          \xa1\xf1      \xa1\xf1             ");

        break;
        case 13:
        case 14:
        //% outs("              ●●●●\n"
        outs("              \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "      ●●●●●●●●\n"
"      \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "    ●●●●●●●●●●\n"
"    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●●●●●●●●●●●●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●    ●●●●●●●●●\n"
"  \xa1\xf1    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "●●    ●            ●●\n"
"\xa1\xf1\xa1\xf1    \xa1\xf1            \xa1\xf1\xa1\xf1\n"
//% "●●●●                ●\n"
"\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1                \xa1\xf1\n"
//% "  ●                    ●\n"
"  \xa1\xf1                    \xa1\xf1\n"
//% "    ●●            ●●\n"
"    \xa1\xf1\xa1\xf1            \xa1\xf1\xa1\xf1\n"
//% "  ●    ●●●●●●  ●\n"
"  \xa1\xf1    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1  \xa1\xf1\n"
//% "  ●                  ●\n"
"  \xa1\xf1                  \xa1\xf1\n"
//% "    ●                  ●\n"
"    \xa1\xf1                  \xa1\xf1\n"
//% "      ●                ●\n"
"      \xa1\xf1                \xa1\xf1\n"
//% "    ●●●            ●●●        ");
"    \xa1\xf1\xa1\xf1\xa1\xf1            \xa1\xf1\xa1\xf1\xa1\xf1        ");
        break;
        case 15:
        case 16:
        //% outs("  ●    ●●●●●●\n"
        outs("  \xa1\xf1    \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "●  ●●  ●        ●\n"
"\xa1\xf1  \xa1\xf1\xa1\xf1  \xa1\xf1        \xa1\xf1\n"
//% "●              ●    ●\n"
"\xa1\xf1              \xa1\xf1    \xa1\xf1\n"
//% "  ●●●                ●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1                \xa1\xf1\n"
//% "●                      ●\n"
"\xa1\xf1                      \xa1\xf1\n"
//% "●  ●●                ●\n"
"\xa1\xf1  \xa1\xf1\xa1\xf1                \xa1\xf1\n"
//% "  ●  ●                ●\n"
"  \xa1\xf1  \xa1\xf1                \xa1\xf1\n"
//% "      ●        ●  ●    ●\n"
"      \xa1\xf1        \xa1\xf1  \xa1\xf1    \xa1\xf1\n"
//% "      ●          ●      ●\n"
"      \xa1\xf1          \xa1\xf1      \xa1\xf1\n"
//% "      ●                  ●\n"
"      \xa1\xf1                  \xa1\xf1\n"
//% "        ●              ●\n"
"        \xa1\xf1              \xa1\xf1\n"
//% "        ●  ●●  ●●●\n"
"        \xa1\xf1  \xa1\xf1\xa1\xf1  \xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "        ●  ●●  ●\n"
"        \xa1\xf1  \xa1\xf1\xa1\xf1  \xa1\xf1\n"
//% "          ●    ●             ");
"          \xa1\xf1    \xa1\xf1             ");

       break;
        }
    }
     else{
        //% outs("          ●●●●●●●\n"
        outs("          \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "        ●          ●●●\n"
"        \xa1\xf1          \xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "      ●    ●    ●  ●●●\n"
"      \xa1\xf1    \xa1\xf1    \xa1\xf1  \xa1\xf1\xa1\xf1\xa1\xf1\n"
//% "  ●●●●●●●        ●●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1        \xa1\xf1\xa1\xf1\n"
//% "  ●          ●          ●\n"
"  \xa1\xf1          \xa1\xf1          \xa1\xf1\n"
//% "  ●●●●●●●          ●            [1;5;31m我是大怪鸟[m\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1          \xa1\xf1            [1;5;31m\xce\xd2\xca\xc7\xb4\xf3\xb9\xd6\xc4\xf1[m\n"
//% "  ●        ●            ●\n"
"  \xa1\xf1        \xa1\xf1            \xa1\xf1\n"
//% "  ●●●●●●            ●\n"
"  \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1            \xa1\xf1\n"
//% "  ●                    ●\n"
"  \xa1\xf1                    \xa1\xf1\n"
//% "  ●                    ●\n"
"  \xa1\xf1                    \xa1\xf1\n"
//% "    ●                ●\n"
"    \xa1\xf1                \xa1\xf1\n"
//% "●●  ●            ●\n"
"\xa1\xf1\xa1\xf1  \xa1\xf1            \xa1\xf1\n"
//% "●      ●●●●●●  ●●\n"
"\xa1\xf1      \xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1\xa1\xf1  \xa1\xf1\xa1\xf1\n"
//% "  ●                      ●\n"
"  \xa1\xf1                      \xa1\xf1\n"
//% "●●●    我是大怪鸟       ●●● ");
"\xa1\xf1\xa1\xf1\xa1\xf1    \xce\xd2\xca\xc7\xb4\xf3\xb9\xd6\xc4\xf1       \xa1\xf1\xa1\xf1\xa1\xf1 ");

   }
  if(clean>10)
    {
        move(10,30);
        //% outs("便便好多..臭臭...");
        outs("\xb1\xe3\xb1\xe3\xba\xc3\xb6\xe0..\xb3\xf4\xb3\xf4...");
        if(clean>15)
          death();
        pressanykey();
     }

   move(17,0);
   //% outs("[32m[1]-吃饭     [2]-吃零食   [3]-清理鸡舍   [4]-跟小鸡猜拳  [5]-目前战绩[m");
   outs("[32m[1]-\xb3\xd4\xb7\xb9     [2]-\xb3\xd4\xc1\xe3\xca\xb3   [3]-\xc7\xe5\xc0\xed\xbc\xa6\xc9\xe1   [4]-\xb8\xfa\xd0\xa1\xbc\xa6\xb2\xc2\xc8\xad  [5]-\xc4\xbf\xc7\xb0\xd5\xbd\xbc\xa8[m");
   //% outs("\n[32m[6]-买鸡饲料$20  [7]-买零食$30  [8]-吃大补丸  [9]-卖鸡喔 [m");
   outs("\n[32m[6]-\xc2\xf2\xbc\xa6\xcb\xc7\xc1\xcf$20  [7]-\xc2\xf2\xc1\xe3\xca\xb3$30  [8]-\xb3\xd4\xb4\xf3\xb2\xb9\xcd\xe8  [9]-\xc2\xf4\xbc\xa6\xe0\xb8 [m");
  //pressanykey();
  return;
}

int select_menu()
{
  char inbuf[80];
  //int diff;
  struct tm *ptime;
  time_t now;

  time(&now);
  ptime = localtime(&now);
  //diff = (time(0) /*- login_start_time*/) / 60;
 move(23,0);
 //% prints("[0;46;31m  使用帮助  [0;47;34m c 改名字   k 杀鸡   t 消除非疲劳($50)        [m"); 
 prints("[0;46;31m  \xca\xb9\xd3\xc3\xb0\xef\xd6\xfa  [0;47;34m c \xb8\xc4\xc3\xfb\xd7\xd6   k \xc9\xb1\xbc\xa6   t \xcf\xfb\xb3\xfd\xb7\xc7\xc6\xa3\xc0\xcd($50)        [m"); 
  while(1)
{
        //% getdata(22, 0, "要做些什麽呢?：[0]", inbuf, 4, DOECHO,YEA);
        getdata(22, 0, "\xd2\xaa\xd7\xf6\xd0\xa9\xca\xb2\xf7\xe1\xc4\xd8?\xa3\xba[0]", inbuf, 4, DOECHO,YEA);
        if(tiredstrong>20)
          {
           clear();
           move(15,30);
           //% outs("呜~~~小鸡会累坏的...要先去休息一下..");
           outs("\xce\xd8~~~\xd0\xa1\xbc\xa6\xbb\xe1\xc0\xdb\xbb\xb5\xb5\xc4...\xd2\xaa\xcf\xc8\xc8\xa5\xd0\xdd\xcf\xa2\xd2\xbb\xcf\xc2..");
           //% outs("\n\n休    养     中");
           outs("\n\n\xd0\xdd    \xd1\xf8     \xd6\xd0");
          }
     switch(inbuf[0])
     {   case '1':
          if (food<=0){
                        pressany(5);
                         break;
                       }
        move(10,0);
        //% outs("       □□□□□□\n"
        outs("       \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\n"
//% "         ∵∴ □  □\n"
"         \xa1\xdf\xa1\xe0 \xa1\xf5  \xa1\xf5\n"
//% "              □  □                             □□□□  □\n"
"              \xa1\xf5  \xa1\xf5                             \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5  \xa1\xf5\n"
//% "              □  □     □              □      □□□□□□□\n"
"              \xa1\xf5  \xa1\xf5     \xa1\xf5              \xa1\xf5      \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\n"
//% "         Ｃｏｋｅ □    _□□□□□□□□□_    □□□□□□□□\n"
"         \xa3\xc3\xa3\xef\xa3\xeb\xa3\xe5 \xa1\xf5    _\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5_    \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\n"
//% "             □   □     ％％％％％％％％％       □—∩∩—□\n"
"             \xa1\xf5   \xa1\xf5     \xa3\xa5\xa3\xa5\xa3\xa5\xa3\xa5\xa3\xa5\xa3\xa5\xa3\xa5\xa3\xa5\xa3\xa5       \xa1\xf5\xa1\xaa\xa1\xc9\xa1\xc9\xa1\xaa\xa1\xf5\n"
//% "            □    □     □□□□□□□□□       │Mcdonald│      　　　　\n"
"            \xa1\xf5    \xa1\xf5     \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5       \xa9\xa6Mcdonald\xa9\xa6      \xa1\xa1\xa1\xa1\xa1\xa1\xa1\xa1\n"
//% "           □     □     ※※※※※※※※※　     □————□\n"
"           \xa1\xf5     \xa1\xf5     \xa1\xf9\xa1\xf9\xa1\xf9\xa1\xf9\xa1\xf9\xa1\xf9\xa1\xf9\xa1\xf9\xa1\xf9\xa1\xa1     \xa1\xf5\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xf5\n"
//% "       □□□□□□      □□□□□□□□□     □□□□□□□□ ");
"       \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5      \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5     \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5 ");

        pressany(0);
        refresh();
        sleep(1);
        food--;
        tiredstrong++;
        satis++;
        if(age<5)
          weight=weight+(5-age);
        else
          weight++;
        if(weight>100)
          { move(9,30);
            //% outs("太重了啦..肥鸡~~你想撑死鸡啊？....哇咧○●××");
            outs("\xcc\xab\xd6\xd8\xc1\xcb\xc0\xb2..\xb7\xca\xbc\xa6~~\xc4\xe3\xcf\xeb\xb3\xc5\xcb\xc0\xbc\xa6\xb0\xa1\xa3\xbf....\xcd\xdb\xdf\xd6\xa1\xf0\xa1\xf1\xa1\xc1\xa1\xc1");
            pressanykey();
           }
        if(weight>150)
          {move(9,30);
           //% outs("鸡撑晕了~~");
           outs("\xbc\xa6\xb3\xc5\xd4\xce\xc1\xcb~~");
           pressanykey();
           }
        if(weight>200)
           death();
          break;
     case '2':
        if (zfood<=0){
         pressany(5);
         break;}
        move(10,0);
        //% outs("             □\n"
        outs("             \xa1\xf5\n"
//% "       [33;1m□[m□○\n"
"       [33;1m\xa1\xf5[m\xa1\xf5\xa1\xf0\n"
//% "       [37;42m■■[m\n"
"       [37;42m\xa1\xf6\xa1\xf6[m\n"
//% "       [32m□□[m\n"
"       [32m\xa1\xf5\xa1\xf5[m\n"
//% "       [32;40;1m□□[m\n"
"       [32;40;1m\xa1\xf5\xa1\xf5[m\n"
//% "       [31m □ [m\n"
"       [31m \xa1\xf5 [m\n"
//% "      [31m □□[m   [32;1m水果酒冰淇淋苏打[m   嗯!好喝!   ");
"      [31m \xa1\xf5\xa1\xf5[m   [32;1m\xcb\xae\xb9\xfb\xbe\xc6\xb1\xf9\xe4\xbf\xc1\xdc\xcb\xd5\xb4\xf2[m   \xe0\xc5!\xba\xc3\xba\xc8!   ");
        pressany(1);
        zfood--;
        tiredstrong++;
        happy++;
        weight+=2;
        if(weight>100)
          {move(9,30);
           //% outs("太重了啦..肥鸡~~");
           outs("\xcc\xab\xd6\xd8\xc1\xcb\xc0\xb2..\xb7\xca\xbc\xa6~~");
           pressanykey();
           }
        if(weight>200)
          death();
        break;
     case '3':
        move(10,0);
        //% outs("[1;36m                              □□□□□[m\n"
        outs("[1;36m                              \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "[1;33m                             『[37m□□□□[m\n"
"[1;33m                             \xa1\xba[37m\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "[1;37m                               □□□□[m\n"
"[1;37m                               \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "[1;37m             □□□□□□□□[32m◎[37m□□□□[m\n"
"[1;37m             \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[32m\xa1\xf2[37m\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "[37m             □□□□□□□□□[1;37m□□□□[m\n"
"[37m             \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[1;37m\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "[37m             □□□□□□□□□[1;33m □[m\n"
"[37m             \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[1;33m \xa1\xf5[m\n"
//% "[36m                  □□□□□□[1;33m□□[m\n"
"[36m                  \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[1;33m\xa1\xf5\xa1\xf5[m\n"
//% "[1;36m                  □□□□□□[m\n"
"[1;36m                  \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "  [1;37m                □□□□□□[m\n"
"  [1;37m                \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5[m\n"
//% "                  耶耶耶...便便拉光光...                              ");
"                  \xd2\xae\xd2\xae\xd2\xae...\xb1\xe3\xb1\xe3\xc0\xad\xb9\xe2\xb9\xe2...                              ");

        pressany(2);
        tiredstrong+=5;
        clean=0;
        break;
     case '4':
        guess();
        satis+= (ptime->tm_sec%2);
        tiredstrong++;
        break;
     case '5':
        situ();
        break;
     case '6':
          move(20,0);
        if(currentuser.money<20)
          //% {    outs("糖果不足!!");
          {    outs("\xcc\xc7\xb9\xfb\xb2\xbb\xd7\xe3!!");
        pressanykey();
        break;}
        food+=5;
        //% prints("\n食物有 [33;41m %d [m份",food);
        prints("\n\xca\xb3\xce\xef\xd3\xd0 [33;41m %d [m\xb7\xdd",food);
        //% prints("   剩下 [33;41m %d [m糖",demoney(20));
        prints("   \xca\xa3\xcf\xc2 [33;41m %d [m\xcc\xc7",demoney(20));
        pressanykey();
        break;

     case '7':
          move(20,0);
        if(currentuser.money<30)
          //% {    outs("糖果不足!!");
          {    outs("\xcc\xc7\xb9\xfb\xb2\xbb\xd7\xe3!!");
        pressanykey();
        break;}
        zfood+=5;
        //% prints("\n零食有 [33;41m %d [m份",zfood);
        prints("\n\xc1\xe3\xca\xb3\xd3\xd0 [33;41m %d [m\xb7\xdd",zfood);
        //% prints("  剩下 [33;41m %d [m糖",demoney(30));
        prints("  \xca\xa3\xcf\xc2 [33;41m %d [m\xcc\xc7",demoney(30));
        pressanykey();
        break;
     case '8':
       move(21,0);
       if(oo>0)
          {
        move(10,0);
        outs("\n"
//% "               □□□□\n"
"               \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\n"
//% "               □□□□\n"
"               \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\n"
//% "               □□□□\n"
"               \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\n"
//% "                          偷吃禁药大补丸.....");
"                          \xcd\xb5\xb3\xd4\xbd\xfb\xd2\xa9\xb4\xf3\xb2\xb9\xcd\xe8.....");
        tiredstrong = 0;
        happy+=3;
        oo--;
        pressany(6);
        break;  }
        move(20,4);
        //% outs("没大补丸啦!!");
        outs("\xc3\xbb\xb4\xf3\xb2\xb9\xcd\xe8\xc0\xb2!!");
        pressanykey();
        break;

     case '9':
        if(age<5)
        { move(20,4);
          //% prints("太小了...不得贩售未成年小鸡.....");
          prints("\xcc\xab\xd0\xa1\xc1\xcb...\xb2\xbb\xb5\xc3\xb7\xb7\xca\xdb\xce\xb4\xb3\xc9\xc4\xea\xd0\xa1\xbc\xa6.....");
          pressanykey();
          break;
        }
        sell();
        break;
     case 'k':
        death();
        break;
     case 't':
        tiredstrong = 0;
        currentuser.money-=50;
        break;
     case 'c':
        //% getdata(22, 0, "帮小鸡取个好名字：", Name, 21, DOECHO,YEA);
        getdata(22, 0, "\xb0\xef\xd0\xa1\xbc\xa6\xc8\xa1\xb8\xf6\xba\xc3\xc3\xfb\xd7\xd6\xa3\xba", Name, 21, DOECHO,YEA);
        break;
     default:
        return;
     break;
    }
    show_chicken();
   }
  return;
}

int death()
{
  char fname[50];
  FILE *fp;
  struct tm *ptime;

      time_t now;

        time(&now);
          ptime = localtime(&now);
  clear();
  move(5,0);
  clrtobot();
  if((fp = fopen("game/chicken", "a"))!=NULL) 
   prints("Error!\n");
     /*fp=fopen("game/chicken,"ab");*/
//% fprintf(fp,"[32m%s[m 在 [34;43m[%d/%d  %d:%02d][m  的小鸡 [33m%s  [36m挂了~~[m \n",
fprintf(fp,"[32m%s[m \xd4\xda [34;43m[%d/%d  %d:%02d][m  \xb5\xc4\xd0\xa1\xbc\xa6 [33m%s  [36m\xb9\xd2\xc1\xcb~~[m \n",
                 currentuser.userid,ptime->tm_mon + 1, ptime->tm_mday,
                           ptime->tm_hour, ptime->tm_min,Name);
                             fclose(fp);
    //% outs("呜...小鸡挂了....");
    outs("\xce\xd8...\xd0\xa1\xbc\xa6\xb9\xd2\xc1\xcb....");
  //% outs("\n笨史了...赶出系统...");
  outs("\n\xb1\xbf\xca\xb7\xc1\xcb...\xb8\xcf\xb3\xf6\xcf\xb5\xcd\xb3...");
  pressanykey();
  setuserfile(fname, DATA_FILE);

  unlink(fname);
//strcpy(Name,"");
  creat_a_egg();
  chicken_main();
  //abort_bbs();
}

/*int comeclear()
{
   extern struct commands cmdlist[];

  //% domenu(MMENU, "主功\能表", (chkmail(0) ? 'M' : 'C'), cmdlist);
  domenu(MMENU, "\xd6\xf7\xb9\xa6\\xc4\xdc\xb1\xed", (chkmail(0) ? 'M' : 'C'), cmdlist);
}
*/

int
pressany(i)
{
  int ch;
  move(23,0);
  prints("[33;46;1m                           [34m%s[37m                         [0m",cstate[i]);
  do
  {
    ch = terminal_getchar();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      /* capture_screen()*/ clear();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));
  move(23, 0);
  clrtoeol();
  refresh();

}

int guess()
{
   int ch,com;
   struct tm *qtime;
   time_t now;

   time(&now);
   qtime = localtime(&now);

   do
   {
    //% /*getdata(22, 0, "[1]-剪刀 [2]-石头 [3]-布：", inbuf, 4,
    /*getdata(22, 0, "[1]-\xbc\xf4\xb5\xb6 [2]-\xca\xaf\xcd\xb7 [3]-\xb2\xbc\xa3\xba", inbuf, 4,
DOECHO,NULL);*/
    move(23,0);
    //% outs("[1]-剪刀 [2]-石头 [3]-布：");
    outs("[1]-\xbc\xf4\xb5\xb6 [2]-\xca\xaf\xcd\xb7 [3]-\xb2\xbc\xa3\xba");
    ch = terminal_getchar();
    }while((ch!='1')&&(ch!='2')&&(ch!='3'));

   /* com=qtime->tm_sec%3;*/
    com=rand()%3;
    move(21,35);
    switch(com){
        case 0:
          //% outs("小鸡:剪刀");
          outs("\xd0\xa1\xbc\xa6:\xbc\xf4\xb5\xb6");
         break;
        case 1:
          //% outs("小鸡:石头");
          outs("\xd0\xa1\xbc\xa6:\xca\xaf\xcd\xb7");
         break;
        case 2:
          //% outs("小鸡:布");
          outs("\xd0\xa1\xbc\xa6:\xb2\xbc");
         break;
     }

    move(19,0);


    switch(ch){
    case '1':
      //% outs("笨鸡---看我捡来的刀---");
      outs("\xb1\xbf\xbc\xa6---\xbf\xb4\xce\xd2\xbc\xf1\xc0\xb4\xb5\xc4\xb5\xb6---");
      if (com==0)
        tie();
     else  if (com==1)
        lose();
     else if (com==2)
        win_c();
    break;
    case '2':
      //% outs("呆鸡---砸你一块石头!!---");
      outs("\xb4\xf4\xbc\xa6---\xd4\xd2\xc4\xe3\xd2\xbb\xbf\xe9\xca\xaf\xcd\xb7!!---");
      if (com==0)
        win_c();
     else if (com==1)
        tie();
     else if (com==2)
        lose();
    break;
    case '3':
      //% outs("蠢鸡---送你一堆破布!---");
      outs("\xb4\xc0\xbc\xa6---\xcb\xcd\xc4\xe3\xd2\xbb\xb6\xd1\xc6\xc6\xb2\xbc!---");
      if (com==0)
        lose();
     else if (com==1)
        win_c();
     else if (com==2)
        tie();
    break;
  }
  /* sleep(1);*/
   pressanykey();

}

int win_c()
{
        winn++;
       /* sleep(1);*/
        move (20,0);
        //% outs("判定:小鸡输了....    >_<~~~~~\n"
        outs("\xc5\xd0\xb6\xa8:\xd0\xa1\xbc\xa6\xca\xe4\xc1\xcb....    >_<~~~~~\n"
"\n"
"                                 ");
        return;
}
int tie()
{
       /* sleep(0);*/
         move (21,0);
        //% outs("判定:平手                    -_-\n"
        outs("\xc5\xd0\xb6\xa8:\xc6\xbd\xca\xd6                    -_-\n"
"\n"
"                                              ");
        return;
}
int lose()
{
        losee++;
        happy+=2;
        /*sleep(0);*/
         move (21,0);
        //% outs("小鸡赢罗                      ∩∩\n"
        outs("\xd0\xa1\xbc\xa6\xd3\xae\xc2\xde                      \xa1\xc9\xa1\xc9\n"
//% "                               □       ");
"                               \xa1\xf5       ");
        return;
}

int situ()
{

        move(16,0);
        outs("           ");
        move(17,0);
        //% prints("你:[44m %d胜 %d负[m                   ",winn,losee);
        prints("\xc4\xe3:[44m %d\xca\xa4 %d\xb8\xba[m                   ",winn,losee);
        move(18,0);
        //% prints("鸡:[44m %d胜 %d负[m                   ",losee,winn);
        prints("\xbc\xa6:[44m %d\xca\xa4 %d\xb8\xba[m                   ",losee,winn);

       if (winn>=losee)
        pressany(4);
       else
        pressany(3);

       return;
}

void
p_bf()
{
  FILE *fp;
  char fname[50];
  set_user_status(ST_CHICK);
  clear();
  move(21,0);
  if(currentuser.money<100){
    //% outs("糖果不足!!");
    outs("\xcc\xc7\xb9\xfb\xb2\xbb\xd7\xe3!!");
        pressanykey();
        return;}
     setuserfile(fname, "chicken");
  if ((fp = fopen(fname, "r+")) == NULL)
   {
      //% outs("没养鸡..不给你买..哈哈...");
      outs("\xc3\xbb\xd1\xf8\xbc\xa6..\xb2\xbb\xb8\xf8\xc4\xe3\xc2\xf2..\xb9\xfe\xb9\xfe...");
       pressanykey();
        return;
    }
   else{
        fp = fopen(fname, "r");
        fscanf(fp,"%d %d %d %d %d %d %d %d %d %d %s %d %d"
,&weight,&mon,&day,&satis,&age,&oo,&happy,&clean,&tiredstrong,&play,Name
         ,&winn,&losee);
        fclose(fp);
      oo++;
      //% prints("\n大补丸有 %d 颗",oo);
      prints("\n\xb4\xf3\xb2\xb9\xcd\xe8\xd3\xd0 %d \xbf\xc5",oo);
      //% prints("  剩下 %d 糖,花钱100",demoney(100));
      prints("  \xca\xa3\xcf\xc2 %d \xcc\xc7,\xbb\xa8\xc7\xae100",demoney(100));
      pressanykey();
    fp = fopen(fname, "r+");
  /*if (last!=1)
    { */
        fprintf(fp,"%d %d %d %d %d %d %d %d %d %d %s %d %d"
          ,weight,mon,day,satis,age,oo,happy,clean,tiredstrong,play,Name
          ,winn,losee);
  fclose(fp);
    }
        return;
}


int year(char *useri)
{
  FILE *fp;
  char fname[50];
        getuser(useri);
        sethomefile(fname, useri, "chicken");
  if ((fp = fopen(fname, "r+")) == NULL)
   {
       return ;
    }
        fscanf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s "
         ,&weight,&mon,&day,&satis,&age,&oo,&happy,&clean,&tiredstrong,&play
         ,&winn,&losee,&food,&zfood,Name);
        fclose(fp);
  return age;

    }
int sell()
{
  int sel=0;
  char ans[5];
  struct tm *ptime;
  FILE *fp;
    time_t now;

      time(&now);
        ptime = localtime(&now);

  //% getdata(20, 0, "确定要卖掉小鸡?[y/N]",ans,3,DOECHO,YEA);
  getdata(20, 0, "\xc8\xb7\xb6\xa8\xd2\xaa\xc2\xf4\xb5\xf4\xd0\xa1\xbc\xa6?[y/N]",ans,3,DOECHO,YEA);
  if(ans[0]!='y') return;
  sel+=(happy*10);
  sel+=(satis*7);
  sel+= ((ptime->tm_sec%9)*10);
  sel+= weight;
  sel+=age*10;
  move(20,0);
  //% prints("小鸡值[33;45m$$ %d [m糖糖",sel);
  prints("\xd0\xa1\xbc\xa6\xd6\xb5[33;45m$$ %d [m\xcc\xc7\xcc\xc7",sel);
    //% getdata(19, 0, "真的要卖掉小鸡?[y/N]",ans,3,DOECHO,YEA);
    getdata(19, 0, "\xd5\xe6\xb5\xc4\xd2\xaa\xc2\xf4\xb5\xf4\xd0\xa1\xbc\xa6?[y/N]",ans,3,DOECHO,YEA);
  if(ans[0]!='y') return;

  if((fp = fopen("game/chicken", "a"))!=NULL);
//% fprintf(fp,"[32m%s[m 在 [34;43m[%d/%d  %d:%02d][m  把小鸡 [33m%s  [31m以 [37;44m%d[m [31m糖果卖了[m\n",
fprintf(fp,"[32m%s[m \xd4\xda [34;43m[%d/%d  %d:%02d][m  \xb0\xd1\xd0\xa1\xbc\xa6 [33m%s  [31m\xd2\xd4 [37;44m%d[m [31m\xcc\xc7\xb9\xfb\xc2\xf4\xc1\xcb[m\n",
                 currentuser.userid,ptime->tm_mon + 1, ptime->tm_mday,
                           ptime->tm_hour, ptime->tm_min,Name,sel);
                             fclose(fp);
  clear();

  inmoney(sel);
  strcpy(Name,"");
  creat_a_egg();
  chicken_main();

}

int gagb_c()
{
  char abuf[5],buf1[6];
  char ans[5];
  int i,k,flag[11],count=0,GET=0;
  int l=1,money=0;

  //setutmpmode(NANB);
    clear();
  do{
   /* while(strlen(buf1)<1)*/
  //% getdata(0, 0, "要押多少糖果啊(最大2000)：", buf1, 5, DOECHO,YEA);
  getdata(0, 0, "\xd2\xaa\xd1\xba\xb6\xe0\xc9\xd9\xcc\xc7\xb9\xfb\xb0\xa1(\xd7\xee\xb4\xf32000)\xa3\xba", buf1, 5, DOECHO,YEA);
  money=atoi(buf1);
     if(currentuser.money<money){
      //% outs("不够$$");
      outs("\xb2\xbb\xb9\xbb$$");
      pressanykey();
      return 0;}
     }while((money<=0)||(money>2000));
  demoney(money);
  for(i=0;i<11;i++)
   flag[i]=0;
  for(i=0;i<4;i++){
   do{
     k=rand()%9;
     } while (flag[k]!=0);
   flag[k]=1;
   ans[i]=k+'0';
   }
  while(!GET)
  {
   ga(abuf,l);
   if (abuf[0]=='q'&&abuf[1]=='k'){
     //% prints("投降..猜了 %d次",count);
     prints("\xcd\xb6\xbd\xb5..\xb2\xc2\xc1\xcb %d\xb4\xce",count);
     //% prints("\n答案是:%s",ans);
     prints("\n\xb4\xf0\xb0\xb8\xca\xc7:%s",ans);
     pressanykey();
    /*return 0*/;}
   if(abuf[0]=='q'){
     //% prints("\n答案是:%s",ans);
     prints("\n\xb4\xf0\xb0\xb8\xca\xc7:%s",ans);
     pressanykey();
    return 0;}
   if(compare(ans,abuf,count))
   /* GET=1;*/break;
   if(count>8){
     //% outs("[1;32m哇咧..猜十次还不对...糖糖没收..[m");
     outs("[1;32m\xcd\xdb\xdf\xd6..\xb2\xc2\xca\xae\xb4\xce\xbb\xb9\xb2\xbb\xb6\xd4...\xcc\xc7\xcc\xc7\xc3\xbb\xca\xd5..[m");
     pressanykey();
     return 0;}
   count++;
   l+=2;
  }
  count++;
  switch(count){
   case 1:
     money*=10;
     break;
   case 2:
   case 3:
     money*=6;
     break;
   case 4:
   case 5:
     money*=3;
     break;
   case 6:
     money*=2;
     break;
   case 7:
     money*=2;
     break;
   case 8:
     money*=1.1;
     break;
   case 9:
     money+=10;
     break;
/*   case 8:
     money*=2;
     break;*/
   default:
 /*    money/=2;*/
     money=1;
     break;}
   inmoney(money);

  //% prints("\n终於对了..猜了[32m %d[m 次 赏糖糖 [33;45m%d[m 颗",count,money);
  prints("\n\xd6\xd5\xec\xb6\xb6\xd4\xc1\xcb..\xb2\xc2\xc1\xcb[32m %d[m \xb4\xce \xc9\xcd\xcc\xc7\xcc\xc7 [33;45m%d[m \xbf\xc5",count,money);
  pressanykey();

  return 0;
}

int compare(char ans[],char buf[],int c)
{
 int i,j,A,B;

 A=0;
 B=0;
 for(i=0;i<4;i++)
  if(ans[i]==buf[i])
  A++;
 for(i=0;i<4;i++)
  for(j=0;j<4;j++)
   if((ans[i]==buf[j])&&(i!=j))
    B++;
 prints("%s",buf);
 //% prints("  结果: %d A %d B 剩 %d 次\n",A,B,9-c);
 prints("  \xbd\xe1\xb9\xfb: %d A %d B \xca\xa3 %d \xb4\xce\n",A,B,9-c);
 /*  pressanykey(); */
  if(A==4)
   return 1;
 else
   return 0;
}

int ga(char buf[],int l)
{
  int q,ok=0;

  //% getdata(l, 0, "输入所猜的数字(四位不重覆)：", buf, 5, DOECHO,YEA);
  getdata(l, 0, "\xca\xe4\xc8\xeb\xcb\xf9\xb2\xc2\xb5\xc4\xca\xfd\xd7\xd6(\xcb\xc4\xce\xbb\xb2\xbb\xd6\xd8\xb8\xb2)\xa3\xba", buf, 5, DOECHO,YEA);
  if (q=(strlen(buf))!=4){
       if (buf[0]=='z'&&buf[1]=='k')
   return 0;
       if (buf[0]=='q')
   return 0;
    //% outs("乱来..不足4位");
    outs("\xc2\xd2\xc0\xb4..\xb2\xbb\xd7\xe34\xce\xbb");
   /* pressanykey();*/
    return 0; }
  if((buf[0]!=buf[1])&&(buf[0]!=buf[2])&&(buf[0]!=buf[3])
   &&(buf[1]!=buf[2])&&
     (buf[1]!=buf[3])&&(buf[2]!=buf[3])) ok=1;
   if(ok!=1){
    //% outs("重覆罗");
    outs("\xd6\xd8\xb8\xb2\xc2\xde");
    /*pressanykey();*/
    return 0;}

  return 0;
}
int nam(char *useri)
{
  FILE *fp;
  char fname[50];
        getuser(useri);
      sethomefile(fname, useri, "chicken");
  if ((fp = fopen(fname, "r+")) == NULL)
   {
       return ;
    }
        fscanf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %s "
         ,&weight,&mon,&day,&satis,&age,&oo,&happy,&clean,&tiredstrong,&play
         ,&winn,&losee,&food,&zfood,Name);
        fclose(fp);
  //return Name;
    return 1;
    }


  int gold,x[9]={0},ran,q_mon,p_mon;
  unsigned long int bank;
  char buf[1],buf1[6];

int mary_m()
{
   FILE *fp;
   set_user_status(ST_MARY);
     if ((fp = fopen("game/bank", "r")) == NULL){
            fp = fopen("game/bank", "w");
                 fprintf(fp,"%ld",1000000);
            fclose(fp);}
     fp = fopen("game/bank", "r");
     fscanf(fp,"%ld",&bank);
     fclose(fp);
   clear();
   clrtobot();
   p_mon=0;
   q_mon=currentuser.money;
   show_m();

   fp = fopen("game/bank", "r+");
   fprintf(fp,"%ld",bank);
   fclose(fp);
   return;
}

int show_m()
{
   int i,j,k,m;

   move(0,0);
   clear();
   //% outs("              □□    □□\n"
   outs("              \xa1\xf5\xa1\xf5    \xa1\xf5\xa1\xf5\n"
//% "            ／    ＼／    ＼\n"
"            \xa3\xaf    \xa3\xdc\xa3\xaf    \xa3\xdc\n"
//% "           ｜ □□ ｜ □□ ｜\n"
"           \xa3\xfc \xa1\xf5\xa1\xf5 \xa3\xfc \xa1\xf5\xa1\xf5 \xa3\xfc\n"
//% "            ＼___／　＼□□／\n"
"            \xa3\xdc___\xa3\xaf\xa1\xa1\xa3\xdc\xa1\xf5\xa1\xf5\xa3\xaf\n"
//% "            │  ___  ___  │\n"
"            \xa9\xa6  ___  ___  \xa9\xa6\n"
//% "          （│□_■□_■  │）\n"
"          \xa3\xa8\xa9\xa6\xa1\xf5_\xa1\xf6\xa1\xf5_\xa1\xf6  \xa9\xa6\xa3\xa9\n"
//% "        (~~.│   ＼ｗ／    │.~~)\n"
"        (~~.\xa9\xa6   \xa3\xdc\xa3\xf7\xa3\xaf    \xa9\xa6.~~)\n"
//% "       `＼／ ＼    ｏ    ／ ＼／\n"
"       `\xa3\xdc\xa3\xaf \xa3\xdc    \xa3\xef    \xa3\xaf \xa3\xdc\xa3\xaf\n"
//% "   　     ＼   ＼□□□／   ／\n"
"   \xa1\xa1     \xa3\xdc   \xa3\xdc\xa1\xf5\xa1\xf5\xa1\xf5\xa3\xaf   \xa3\xaf\n"
//% "   　       ＼／｜ ｜ ｜＼／\n"
"   \xa1\xa1       \xa3\xdc\xa3\xaf\xa3\xfc \xa3\xfc \xa3\xfc\xa3\xdc\xa3\xaf\n"
//% "     　      │  □Ｏ□  │\n"
"     \xa1\xa1      \xa9\xa6  \xa1\xf5\xa3\xcf\xa1\xf5  \xa9\xa6\n"
//% "     　     □___／Ｏ＼___□\n"
"     \xa1\xa1     \xa1\xf5___\xa3\xaf\xa3\xcf\xa3\xdc___\xa1\xf5\n"
//% "       　      ＼__｜__／    [31;40m欢迎光临小玛莉..[m");
"       \xa1\xa1      \xa3\xdc__\xa3\xfc__\xa3\xaf    [31;40m\xbb\xb6\xd3\xad\xb9\xe2\xc1\xd9\xd0\xa1\xc2\xea\xc0\xf2..[m");

      move(13,0);
   //% sprintf(genbuf,"现有糖果: %-d            本机台内现金: %-ld",q_mon,bank);
   sprintf(genbuf,"\xcf\xd6\xd3\xd0\xcc\xc7\xb9\xfb: %-d            \xb1\xbe\xbb\xfa\xcc\xa8\xc4\xda\xcf\xd6\xbd\xf0: %-ld",q_mon,bank);
   prints(genbuf);
   move(14,0);


//% prints("[36m——————————————————————————————————————[m\n");
prints("[36m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[m\n");

   //% prints("苹果-1 bar-2  777-3  王冠-4 BAR-5  铃铛-6 西瓜-7 橘子-8 荔枝-9\n");
   prints("\xc6\xbb\xb9\xfb-1 bar-2  777-3  \xcd\xf5\xb9\xda-4 BAR-5  \xc1\xe5\xee\xf5-6 \xce\xf7\xb9\xcf-7 \xe9\xd9\xd7\xd3-8 \xc0\xf3\xd6\xa6-9\n");
   //% prints("x5     x40    x30    x25    x50    x20    x15    x10    x2倍\n");
   prints("x5     x40    x30    x25    x50    x20    x15    x10    x2\xb1\xb6\n");
   prints("%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d\n",x[0],x[1],x[2],x[3],x[4],x[5],
           x[6],x[7],x[8]);

//% prints("\n[36m————————————————————按a全压————按s开始——按q离开——[m");
prints("\n[36m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xb0\xb4a\xc8\xab\xd1\xb9\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xb0\xb4s\xbf\xaa\xca\xbc\xa1\xaa\xa1\xaa\xb0\xb4q\xc0\xeb\xbf\xaa\xa1\xaa\xa1\xaa[m");
   //% getdata(20, 0, "要押几号(可押多次)：", buf, 2, DOECHO,YEA);
   getdata(20, 0, "\xd2\xaa\xd1\xba\xbc\xb8\xba\xc5(\xbf\xc9\xd1\xba\xb6\xe0\xb4\xce)\xa3\xba", buf, 2, DOECHO,YEA);
   switch(buf[0]){
   case 's':
        doit();
        return;
        break;
   case 'a':
        //% getdata(21, 0, "要押多少糖：", buf1, 6, DOECHO,YEA);
        getdata(21, 0, "\xd2\xaa\xd1\xba\xb6\xe0\xc9\xd9\xcc\xc7\xa3\xba", buf1, 6, DOECHO,YEA);
        for(i=0;i<=8;i++)
         x[i]=atoi(buf1);
         j=(x[0]*9);
         j=abs(j);
        if(q_mon<j)
          //% {outs("糖果不足");
          {outs("\xcc\xc7\xb9\xfb\xb2\xbb\xd7\xe3");
           pressanykey();
           for(i=0;i<=8;i++)
                x[i]=0;
           show_m();
           return;
           j=0;
           }
     /*    demoney(j);*/
        q_mon-=j;
        p_mon+=j;
 /*       strcpy(buf," ");*/
        show_m();
        return;
        break;
   case 'q':
     for(i=0;i<=8;i++)
      x[i]=0;
        return;
        break;
   case 't':
        m=10000000;
        for(i=1;i<=5;i++){
         clear();
         move(20,i);
         outs("x");
         if(i%3==0)
          m*=10;
          for(j=1;j<=m;j++)
          k=0;

          refresh();

         }
        return;
        break;
   default:
   i=atoi(buf);
        break;
   }
   k=x[i-1];
   do{
   //% getdata(21, 0, "要押多少糖：", buf1, 6, DOECHO,YEA);
   getdata(21, 0, "\xd2\xaa\xd1\xba\xb6\xe0\xc9\xd9\xcc\xc7\xa3\xba", buf1, 6, DOECHO,YEA);
   x[i-1]+=atoi(buf1);
        j=atoi(buf1); }while(x[i-1]<0);

/*   j=x[i-1];*/
   if(j<0)
        j=abs(j);
   if(q_mon<j)
        //% {outs("糖果不足");
        {outs("\xcc\xc7\xb9\xfb\xb2\xbb\xd7\xe3");
         pressanykey();
         x[i-1]=k;
         clear();
         j=0;}
      q_mon-=j;
      p_mon+=j;
  /* demoney(j);*/
   show_m();
   return;
}

int doit()
{
   int i,j,k,m,seed,flag=0,flag1=0;
   int g[10]={5,40,30,25,50,20,15,10,2,0};

   clear();
   move(0,0);
/*   outs("
                       //% □□□□□□□□□□□
                       \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5
                       //% □                  □
                       \xa1\xf5                  \xa1\xf5
                       //% □                  □
                       \xa1\xf5                  \xa1\xf5
                       //% □ ＮＩＮＴＥＮＤＯ □
                       \xa1\xf5 \xa3\xce\xa3\xc9\xa3\xce\xa3\xd4\xa3\xc5\xa3\xce\xa3\xc4\xa3\xcf \xa1\xf5
                       //% □  正在转当中      □
                       \xa1\xf5  \xd5\xfd\xd4\xda\xd7\xaa\xb5\xb1\xd6\xd0      \xa1\xf5
                       //% □      自行想像    □
                       \xa1\xf5      \xd7\xd4\xd0\xd0\xcf\xeb\xcf\xf1    \xa1\xf5
                       //% □                  □
                       \xa1\xf5                  \xa1\xf5
                       //% □□□□□□□□□□□
                       \xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5\xa1\xf5
                              NINTENDO

                          //% ↑
                          \xa1\xfc
                        //% ←◎→           ●
                        \xa1\xfb\xa1\xf2\xa1\xfa           \xa1\xf1
                          //% ↓          ●
                          \xa1\xfd          \xa1\xf1
                               //% □  □    .....
                               \xa1\xf5  \xa1\xf5    .....
                                        .......
                                        //% .....□
                                        .....\xa1\xf5
                                                              ");
*/
        m=1000000;
        for(i=1;i<=30;i++){
         clear();
         move(10,i);
         //% outs("●");
         outs("\xa1\xf1");
         if(i%23==0)
          m*=10;
          for(j=1;j<=m;j++)
          k=0;

          refresh();

         }
   demoney(p_mon);
   refresh();
   sleep(1);
   clear();
   move(10,31);
   gold=0;
   seed=time(0)%10000;
   //if(p_mon>=50000)
    // seed=1500;

   do{
   ran=rand()%seed;
   flag1=0;

   move(10,31);
   if(ran<=480)
     //% {  outs("荔枝");
     {  outs("\xc0\xf3\xd6\xa6");
        j=8;}
    else if(ran<=670){
        //% outs("苹果");
        outs("\xc6\xbb\xb9\xfb");
        j=0;}
    else if(ran<=765){
        //% outs("橘子");
        outs("\xe9\xd9\xd7\xd3");
        j=7;}
    else if(ran<=830){
        //% outs("西瓜");
        outs("\xce\xf7\xb9\xcf");
        j=6;}
    else if(ran<=875){
        //% outs("铃铛");
        outs("\xc1\xe5\xee\xf5");
        j=5;}
    else if(ran<=910){
        //% outs("王冠");
        outs("\xcd\xf5\xb9\xda");
        j=3;}
    else if(ran<=940){
        outs("777!");
        j=2;}
    else if(ran<=960){
        outs("bar!");
        j=1;}
    else if(ran<=975){
        outs("BAR!");
        j=4;}
    else {
      /*  outs("test          消去右边  再跑一次\n");
        for(i=4;i<=8;i++)*/
          //% outs("铭谢惠顾");
          outs("\xc3\xfa\xd0\xbb\xbb\xdd\xb9\xcb");
       /* for(i=0;i<=8;i++)
         x[i]=0;*/
        j=9;
          }
   gold=x[j]*g[j];
   if(!flag)
    if(gold>=10000){
       flag=1;
       flag1=1;
     }
            /*    } while( ran>976 || flag1 );*/
                  } while(flag1);
   refresh();
   sleep(1);
   move(11,25);
   //% prints("[32;40m你可得[33;41m %d [32;40m糖糖[m",gold);
   prints("[32;40m\xc4\xe3\xbf\xc9\xb5\xc3[33;41m %d [32;40m\xcc\xc7\xcc\xc7[m",gold);
   refresh();
   if (gold>0){
      bank-=gold;
      bank+=p_mon;
      }
   else
      bank+=p_mon;

   inmoney(gold);
   pressanykey();
   for(i=0;i<=8;i++)
   x[i]=0;
   p_mon=0;
   q_mon=currentuser.money;

   show_m();
   return;
}
