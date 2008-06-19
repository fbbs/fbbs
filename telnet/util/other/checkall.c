#include "bbs.h"

int usernum;
char uid[IDLEN+2];


int
getusernum(uentp)
struct userec *uentp ;
{
    if (strcmp(uentp->userid,uid)) usernum++;
    else return QUIT;
    return 0 ;
}

void report(char *str)
{
}

main(argc,argv)
int argc;
char **argv;
{
  struct userec currentuser;
  char genbuf[33];
  int num;

//  for( usernum=1; usernum<50000; usernum++){
    //if(get_record(PASSFILE,&currentuser,sizeof(currentuser),usernum)!=-1){
  for( usernum=1; usernum<MAXUSERS; usernum++){
    if(getuserbyuid(&currentuser, usernum)!=-1){
      printf("%-12s:",currentuser.userid);
      strcpy( genbuf, "bTCPRp#@XWBA#VS-DEM1234567890D--" );
      for( num = 0; num < 32; num++ )
        if( !(currentuser.userlevel & (1 << num)) )
          genbuf[num] = '-';
      genbuf[num] = '\0';
      printf("%s\n",genbuf);
    }
  }
}
