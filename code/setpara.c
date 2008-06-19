#include "bbs.h"

char uid[IDLEN+2];

int report(char *str){}

main(argc,argv)
int argc;
char **argv;
{
	int usernum;
	struct userec currentuser;

	for( usernum=1; usernum<60000; usernum++ ){
		if( get_record( PASSFILE, &currentuser, sizeof(currentuser), usernum) != -1 ){
			currentuser.numposts = 4000;
			substitute_record( PASSFILE, &currentuser, sizeof(currentuser), usernum );
		}
	}
}


