#include "bbs.h"
#include "record.c"
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
      if( currentuser.userlevel & ~037777777377 ){
        printf("%-12s:",currentuser.userid);
        strcpy( genbuf, "bTCPRD#@XWBA#VS-DOM-F0s2345678aa" );
        for( num = 0; num < 32; num++ )
          if( !(currentuser.userlevel & (1 << num)) )
            genbuf[num] = '-';
        genbuf[num] = '\0';
        printf("%s\n",genbuf);
      }
    }
  }
}
