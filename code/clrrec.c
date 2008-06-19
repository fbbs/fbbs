#include "bbs.h"

int usernum=0;
char uid[IDLEN+2];


int
getusernum(uentp)
struct userec *uentp ;
{
    usernum++;
    if(!strcasecmp(uentp->userid,uid)) 
    {
	bzero(uentp, sizeof(struct userec));
	substitute_record(PASSFILE, uentp, sizeof(struct userec), usernum);
	printf("%s\n", uentp->userid);
    }
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
}


