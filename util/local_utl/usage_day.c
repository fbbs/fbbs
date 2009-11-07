/* $Id: usage_day.c 93 2005-10-22 19:50:41Z rygh $ */

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

#define BBSBOARDS BBSHOME"/.BOARDS"
#define LOGDIR "/home/backup"
char    datestring[30];
 
struct binfo
{
  char  boardname[18];
  char  expname[28];
  int times;
  int sum;
} st[MAXBOARD];

int numboards=0;

int 
brd_cmp(b, a)
struct binfo *a, *b;
{
    if(a->times!=b->times)
            return (a->times - b->times);
    return a->sum-b->sum;
}

/* Added by deardragon 1999.12.2 */
void getdatestring( time_t now)
{
        struct tm *tm;
        char weeknum[7][3]={"Ìì","Ò»","¶þ","Èý","ËÄ","Îå","Áù"};

        tm = localtime(&now);
        sprintf(datestring,"%4dÄê%02dÔÂ%02dÈÕ%02d:%02d:%02d ÐÇÆÚ%2s",
                tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
                tm->tm_hour,tm->tm_min,tm->tm_sec,
                weeknum[tm->tm_wday]);
}
/* Added End. */

int
record_data(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].times++;
                        st[i].sum+=sec;
                        return;
                }
        }
        return ;
}

int
fillbcache(fptr)
struct boardheader *fptr ;
{

    if( numboards >= MAXBOARD )
        return 0;
    if(((fptr->flag != 0)&&!(fptr->flag&BOARD_NOZAP_FLAG ||fptr->flag&BOARD_POST_FLAG))||strlen(fptr->filename)==0)
        return;
    strcpy(st[numboards].boardname,fptr->filename);
    strcpy(st[numboards].expname,fptr->title+8);
/*    printf("%s %s\n",st[numboards].boardname,st[numboards].expname); */
    st[numboards].times=0;
    st[numboards].sum=0;
    numboards++;
    return 0 ;
}

int 
fillboard()
{
        apply_record( BBSBOARDS, fillbcache, sizeof(struct boardheader) );
}

char *
timetostr(i)
int i;
{
        static char str[30];
        int minute,sec,hour;

        minute=(i/60);
        hour=minute/60;
        minute=minute%60;
        sec=i&60;
        sprintf(str,"%2d:%2d:%2d",hour,minute,sec);
        return str;
}

main(argc, argv)
char *argv[];
{

/* added by roly */
struct tm* tmtime;
char buftime[256];
/* add end */

  FILE *fp;
  FILE *op;

  char buf[256], *p,bname[20];
  char filebuf[256];
  char filebuftmp[256];
  char date[80];
  int mode;
  int c[3];
  int max[3];
  unsigned int ave[3];
  int sec;
time_t now;
  int i, j,k;
  char    *blk[10] =
  {
                "£ß", "£ß", "¨x", "¨y", "¨z",
                "¨{", "¨|", "¨}", "¨~", "¨€",
  };

  mode=atoi(argv[1]);

  sprintf(buf,"%s/use_board", BBSHOME);
  
  if ((fp = fopen(buf, "r")) == NULL)
  {
    printf("cann't open use_board\n");
    return 1;
  }

  sprintf(filebuftmp,"%s/0Announce/bbslist/board_day%d.tmp", BBSHOME, ( mode == 1 ) ? 2 : 1);
  sprintf(filebuf,"%s/0Announce/bbslist/board_day%d", BBSHOME, ( mode == 1 ) ? 2 : 1);

  if ((op = fopen(filebuftmp, "w")) == NULL)
  {
    printf("Can't Write file\n");
    return 1;
  }

  fillboard();
  now=time(0);
  tmtime=gmtime(&now);
  sprintf(buftime,"20%02dÄê%02dÔÂ%02dÈÕ",tmtime->tm_year%100, tmtime->tm_mon+1,tmtime->tm_mday);

  //printf("buftime:%s",buftime);
  getdatestring(now);
  sprintf(date,"%14.14s",datestring);

/* added by roly */


/*
  while (fgets(buf, 256,fp))
  {
	if (strlen(buf)<57)
		continue;
	if (strstr(buf,buftime)) break;	
  }

    if ( !strncmp(buf+23, "USE", 3))
    {
      p=strstr(buf,"USE");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+48, "Stay: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data(bname,sec);
    }


*/

/* add end */


  while (fgets(buf, 256, fp))
  {
    if(strlen(buf)<57)
        continue;
    if ( !strncmp(buf+26, "USE", 3))
    {
      p=strstr(buf,"USE");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
//	  printf("%s", buf+51); 
    if ( p = (char *)strstr(buf+51, "Stay: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data(bname,sec);
    }
   }
   fclose(fp);
   qsort(st, numboards, sizeof( st[0] ), brd_cmp);
   ave[0]=0;
   ave[1]=0;
   ave[2]=0;
   max[1]=0;
   max[0]=0;
   max[2]=0;
   for(i=0;i<numboards;i++)
   {
        ave[0]+=st[i].times;
        ave[1]+=st[i].sum;
        ave[2]+=st[i].times==0?0:st[i].sum/st[i].times;
        if(max[0]<st[i].times)
        {
                max[0]=st[i].times;
        }
        if(max[1]<st[i].sum)
        {
                max[1]=st[i].sum;
        }
        if(max[2]<(st[i].times==0?0:st[i].sum/st[i].times))
        {
                max[2]=(st[i].times==0?0:st[i].sum/st[i].times);
        }
   }
   c[0]=max[0]/30+1;
   c[1]=max[1]/30+1;
   c[2]=max[2]/30+1;
   numboards++;
   st[numboards-1].times=ave[0]/numboards;
   st[numboards-1].sum=ave[1]/numboards;
   strcpy(st[numboards-1].boardname,"Average");
   strcpy(st[numboards-1].expname,"×ÜÆ½¾ù");
   if(mode==1)
   {
        fprintf(op,"[1;37mÃû´Î %-15.15s%-28.28s %5s %8s %10s[m\n","ÌÖÂÛÇøÃû³Æ","ÖÐÎÄÐðÊö","ÈË´Î","ÀÛ»ýÊ±¼ä","Æ½¾ùÊ±¼ä");
   }else
   {
        fprintf(op,"      [1;37m1 [m[34m%2s[1;37m= %d (×ÜÈË´Î) [1;37m1 [m[32m%2s[1;37m= %s (ÀÛ»ý×ÜÊ±Êý) [1;37m1 [m[31m%2s[1;37m= %d Ãë(Æ½¾ùÊ±Êý)\n\n",
                blk[9],c[0],blk[9],timetostr(c[1]),blk[9],c[2]);
   }

   for(i=0;i<numboards;i++)
   {
      if(mode==1)
        fprintf(op,"[1m%4d[m %-15.15s%-28.28s %5d %-.8s %10d\n",i+1,st[i].boardname,st[i].expname,st[i].times,timetostr(st[i].sum),st[i].times==0?0:st[i].sum/st[i].times);
      else
      {
        fprintf(op,"      [1;37mµÚ[31m%3d [37mÃû ÌÖÂÛÇøÃû³Æ£º[31m%s [35m%s[m\n",i+1,st[i].boardname,st[i].expname);
        fprintf(op,"[1;37m    ©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª\n");
        fprintf(op,"[1;37mÈË´Î©¦[m[34m");
        for(j=0;j<st[i].times/c[0];j++)
        {
                fprintf(op,"%2s",blk[9]);
        }
                fprintf(op,"%2s [1;37m%d[m\n",blk[(st[i].times%c[0])*10/c[0]],st[i].times);
        fprintf(op,"[1;37mÊ±¼ä©¦[m[32m");
        for(j=0;j<st[i].sum/c[1];j++)
        {
                fprintf(op,"%2s",blk[9]);
        }
                fprintf(op,"%2s [1;37m%s[m\n",blk[(st[i].sum%c[1])*10/c[1]],timetostr(st[i].sum));
        j=st[i].times==0?0:st[i].sum/st[i].times;
        fprintf(op,"[1;37mÆ½¾ù©¦[m[31m");
        for(k=0;k<j/c[2];k++)
        {
                fprintf(op,"%2s",blk[9]);
        }
                fprintf(op,"%2s [1;37m%s[m\n",blk[(j%c[2])*10/c[2]],timetostr(j));
        fprintf(op,"[1;37m    ©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n");
      }
   }
   fclose(op);

   sprintf(buf,"mv %s %s",filebuftmp,filebuf);
   system(buf);	
}
