/* 七天未上站的版主统计 by soff of bitbbs.org */

#include <time.h>
#include <stdio.h>
#include "../../include/bbs.h"

int main (int argc, char *argv[])
{
	FILE *fp, *fout, *bmfp;
	time_t now;
	char which[20];
	int n, i, j = 0;
	struct userec aman;
  	char buf[256];

  	now = time (0);
	sprintf (buf, "%s/.PASSWDS", BBSHOME);
	if ((fp = fopen (buf, "rb")) == NULL) {
		printf ("Can't open record data file.\n");
		return 1;
	}
	for (i = 0;; i++)  {
		if (fread (&aman, sizeof (struct userec), 1, fp) <= 0)
			break;
		if (!(aman.userlevel & PERM_BOARDS)|| !strcasecmp (aman.userid, "SYSOP"))
			continue;
		if (aman.lastlogin >= now) 
			printf("bad lastlogin time: %s\t%ddays\n",aman.userid,(aman.lastlogin-now)/86400);
	}
	printf("%d\n",i);
	fclose (fp);
}
