/* $Id: newacct.c 2 2005-07-14 15:06:08Z root $ */

#include <time.h>
#include <stdio.h>
#include "config.h"

#define MAX_LINE        (15)

struct
{
  int no[24];                   /* ¥Œ ˝ */
  int sum[24];                  /* ◊‹∫œ */
}      st;

/* Added by deardragon 1999.12.2 */
char	datestring[30];
void getdatestring( time_t now)
{
        struct tm *tm;
        char weeknum[7][3]={"ÃÏ","“ª","∂˛","»˝","Àƒ","ŒÂ","¡˘"};

        tm = localtime(&now);
        sprintf(datestring,"%4dƒÍ%02d‘¬%02d»’%02d:%02d:%02d –«∆⁄%2s",
                tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
                tm->tm_hour,tm->tm_min,tm->tm_sec,
                weeknum[tm->tm_wday]);
}
/* Added End. */

main(argc, argv)
  char *argv[];
{
  FILE *fp;
  char buf[256], *p;
  char *date;
  time_t now;
  int hour, max = 0, item, total = 0;
  int i, j;
  char    *blk[10] =
  {
                "  ", "  ", "®x", "®y", "®z",
                "®{", "®|", "®}", "®~", "®Ä",
  };

  sprintf(buf,"%s/usies", BBSHOME);
  
  if ((fp = fopen(buf, "r")) == NULL)
  {
    printf("cann't open usies\n");
    return 1;
  }

  now=time(0);
  //getdatestring(now);
  date=ctime(&now);
  while (fgets(buf, 256, fp))
  {
    hour = atoi(buf+7);
    if (hour < 0 || hour > 23)
    {
       printf("%s\n");
       continue;
    }
    if(strncmp(buf,date+4,6))	//e.g. "May 27"
       continue;
    if(strstr(buf, "APPLY"))
    {
      st.no[hour]++;
      continue;
    }
    if (p = (char *)strstr(buf, "Stay:"))
    {
      st.sum[hour] += atoi( p + 6);
      continue;
    }
  }
  fclose(fp);
  for (i = 0; i < 24; i++)
  {
    total += st.no[i];
    if (st.no[i] > max)
      max = st.no[i];
  }

  item = max / MAX_LINE + 1;
  sprintf(buf,"%s/0Announce/bbslist/newacct.today", BBSHOME);
  if ((fp = fopen(buf, "w")) == NULL) 
  {
    printf("Cann't open newacct\n");
    return 1;
  }

  fprintf(fp,"\n[1;36m   ©∞©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©§©¥\n");
  for (i = MAX_LINE ; i >= 0; i--)
  {
    fprintf(fp, "[1;37m%3.d[36m©¶[31m",(i+1)*item);
    for (j = 0; j < 24; j++)
    {
      if ((item * (i) > st.no[j]) && (item * (i-1) <= st.no[j]) && st.no[j])
      {
        fprintf(fp, "[35m%-3d[31m", (st.no[j]));
        continue;
      }
      if(st.no[j]-item*i<item && item*i<st.no[j])
              fprintf(fp,"%s ", blk[((st.no[j]-item * i)*10)/item]);
      else if(st.no[j]-item * i>=item)
              fprintf(fp,"%s ",blk[9]);
      else
           fprintf(fp,"   ");
    }
    fprintf(fp, "[1;36m©¶\n");
  }
  getdatestring(now);
  fprintf(fp,"  [37m0[36m©∏©§©§©§[37m %s ±æ»’–¬‘ˆ»Àø⁄Õ≥º∆ [36m©§©§©§©§[37m%s[36m©§©º\n"
       "   [;36m  00 01 02 03 04 05 06 07 08 09 10 11 [1;32m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
       "                     [33m1 [31m°ˆ [33m= [37m%-5d [33m±æ»’…Í«Î–¬’ ∫≈»À ˝£∫[37m%-9d[m\n"
    ,BBSNAME, datestring,item,total);
  fclose(fp);
}

