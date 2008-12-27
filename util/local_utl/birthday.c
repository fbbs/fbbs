/* $Id: birthday.c 2 2005-07-14 15:06:08Z root $ */

#include <time.h>
#include <stdio.h>
#include "bbs.h"

main(argc, argv)
char   *argv[];
{
	FILE   *fp, *fout;
	time_t  now;
	int     i, j = 0;
	struct userec aman;
	struct tm *tmnow;
	char    buf[256];
	sprintf(buf, "%s/.PASSWDS", BBSHOME);
	if ((fp = fopen(buf, "rb")) == NULL) {
		printf("Can't open record data file.\n");
		return 1;
	}
	sprintf(buf, "%s/0Announce/bbslist/birthday", BBSHOME);
	if ((fout = fopen(buf, "w")) == NULL) {
		printf("Can't write to birthday file.\n");
		return 1;
	}
	now = time(0);
	now += 86400;		/* 直接算明天比较准啦! +1 不准 */

	tmnow = localtime(&now);
	fprintf(fout, "\n%s明日寿星名表\n\n", BBSNAME);
	fprintf(fout, "以下是 %d 月 %d 日的寿星:\n\n",tmnow->tm_mon + 1, tmnow->tm_mday);

	for (i = 0;; i++) {
		if (fread(&aman, sizeof(struct userec), 1, fp) <= 0)
			break;
			
		/* 以下的  *名人* 不需要算在内 */
		if (!strcasecmp(aman.userid, "SYSOP") || !strcasecmp(aman.userid, "guest") )
			continue;
			
		if (aman.birthmonth == 0)
			continue;
		if (aman.birthmonth == tmnow->tm_mon + 1 &&
			aman.birthday == tmnow->tm_mday) {
			fprintf(fout, " ** %-15.15s (%s)\n", aman.userid, aman.username);
			j++;
		}
	}
	fprintf(fout, "\n\n总共有 %d 位寿星。\n", j);
	fclose(fout);
	fclose(fp);
}
