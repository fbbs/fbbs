#include "bbs.h"
struct userec old;

void report(char *str)
{
}

int
countperf(udata)
struct userec *udata;
{
	int     perf;
	int     reg_days;
	if (!strcmp(udata->userid, "guest"))
		return -9999;
	reg_days = (time(0) - udata->firstlogin) / 86400 + 1;
	
   perf=(reg_days/20>20?20:reg_days/20);
   perf=perf+(udata->stay/36000>30?30:(udata->stay/36000));
   perf=perf+(udata->stay/(36*reg_days)>50?50:(udata->stay/(36*reg_days)));

   return perf;
}

int
doit(buf)
char * buf;
{
        int i, j=0;
        FILE *src;

        src = fopen(buf, "rb");
        for ( i = 0 ; ; i++ ) {


                if ( fread(&old,sizeof(old),1,src) <= 0 )
                        break;
                if ( strlen(old.userid) <= 0 )
                        continue;       /* drop out! */
				
				if ((countperf(old)==100) && (!(old.userlevel & (PERM_BOARDS)))) {
					j++;
					printf("%s\n",old.userid);
				}
        }

        fclose(src);
		printf("total %d",j);
        return 0;
}

int
main(int argc, char **argv)
{
		char buf[STRLEN];
		sprintf(buf,"%s/.PASSWDS",BBSHOME);
		printf("%s",buf);
        if ( !dashf(buf) ) {
                printf("Error(3), you must put your old .PASSWDS into this directory.\n");
                exit(3);
        } 


        return doit(buf);
}
