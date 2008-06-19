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

if (argc!=2) {
    printf("Use: chrecord <userid>.\n");
    exit(-1);
    }
strncpy(uid,argv[1],IDLEN+2);
usernum=0;
apply_record(PASSFILE, getusernum, sizeof(struct userec));
printf("%d\n",usernum++);
if(get_record(PASSFILE,&currentuser,sizeof(currentuser),usernum)==-1){
    printf("Error reading PASSFILE!\n");
    exit(-1);
    }
currentuser.numposts=40000;
substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
}


