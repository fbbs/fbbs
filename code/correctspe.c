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

int report(char *str)
{
}

main(argc,argv)
int argc;
char **argv;
{
  struct userec currentuser;
  char genbuf[33];
  int num;

  for( usernum=1; usernum<50000; usernum++){
    if(get_record(PASSFILE,&currentuser,sizeof(currentuser),usernum)!=-1){
      if( currentuser.userlevel & 010000000000 ){
        if( strcmp(currentuser.userid,"SYSOP")!=0 ){
          currentuser.userlevel = currentuser.userlevel & ~036300000000;
          printf("%-12s:",currentuser.userid);
          strcpy( genbuf, "bTCPRp#@XWBA#VSaDEM1234567890Daa" );
          for( num = 0; num < 32; num++ )
            if( !(currentuser.userlevel & (1 << num)) )
              genbuf[num] = '-';
          genbuf[num] = '\0';
          printf("%s\n",genbuf);
          substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
        }
      }
    }
  }
}
