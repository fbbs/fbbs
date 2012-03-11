#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/bbs.h"
//use this to convert all users' old .goodbrd file into new version
//to support custom folder 
//author: cometcaptor 2007.4
//
//NOTICE!! u should only use this ONCE or u will make .goodbrd file
//blank. Remember to backup first
struct userec user[MAXUSERS];
struct boardheader bh[MAXBOARD];
struct goodbrdheader gbhd[GOOD_BRC_NUM];


int main(int argc, char *argv[])
{
  char path[STRLEN],bname[STRLEN];
  FILE *fp;
  int ui, bi, gi, uim, bim;
  printf("Loading .PASSWDS file...");
  fp = fopen("/home/bbs/.PASSWDS","rb");
  if (fp == NULL)
  {
    printf("\033[1;31m[FAILED]\033[m\n");
    exit(1);
  }
  uim = fread(user, sizeof(struct userec), MAXUSERS, fp);
  fclose(fp);
  printf("\033[1;32mOK\033[m\n");
  printf("Loading .BOARDS file...");
  fp = fopen("/home/bbs/.BOARDS","rb");
  if (fp == NULL)
  {
    printf("\033[1;31m[FAILED]\033[m\n");
    exit(1);
  }
  bim = fread(bh, sizeof(struct boardheader), MAXBOARD, fp);
  fclose(fp);
  printf("\033[1;32mOK\033[m\n");
  for (ui = 0; ui < uim; ui++)
  {
if (user[ui].userid[0] == '\0')
	continue;
sprintf(path,"/home/bbs/home/%c/%s/.goodbrd",toupper(user[ui].userid[0]),user[ui].userid);
    printf("Converting: %s ",user[ui].userid);
    fp = fopen(path,"r");
    if (fp == NULL)
    {
      printf("\033[1;33mSkipped\033[m\n");
      continue;
    }
    memset(gbhd, 0, sizeof(struct goodbrdheader)*GOOD_BRC_NUM);
    gi = 0;
    while (fgets(bname, STRLEN, fp) != NULL)
    {
	    if (strlen(bname)<2)
		    continue;
	    bname[strlen(bname)-1]='\0';
	    for (bi = 0; bi < bim; bi++)
        if (!(strcmp(bh[bi].filename,bname)))
        {
          gbhd[gi].id = gi+1;
          gbhd[gi].pid = 0;
          gbhd[gi].flag = bh[bi].flag;
          gbhd[gi].pos = bi;
          strcpy(gbhd[gi].filename, bh[bi].filename);
          strcpy(gbhd[gi].title, bh[bi].title);
          gi++;
          break;
        }
	    if (gi == GOOD_BRC_NUM)
		    break;
    }
    fclose(fp);
    fp = fopen(path,"wb");
    if (fp == NULL)
    {
      printf("\033[1;31mFAILED\033[m\n");
      system("pause");
      continue;
    }
    fwrite(gbhd, sizeof(struct goodbrdheader), gi, fp);
    fclose(fp);
    printf("\033[1;32mOK\033[m\n");
  }
  return 0;
}
