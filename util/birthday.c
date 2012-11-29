#include <time.h>
#include <stdio.h>
#include "bbs.h"

int main(int argc, char **argv)
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
	//% fprintf(fout, "\n%s明日寿星名表\n\n", BBSNAME);
	fprintf(fout, "\n%s\xc3\xf7\xc8\xd5\xca\xd9\xd0\xc7\xc3\xfb\xb1\xed\n\n", BBSNAME);
	//% fprintf(fout, "以下是 %d 月 %d 日的寿星:\n\n",tmnow->tm_mon + 1, tmnow->tm_mday);
	fprintf(fout, "\xd2\xd4\xcf\xc2\xca\xc7 %d \xd4\xc2 %d \xc8\xd5\xb5\xc4\xca\xd9\xd0\xc7:\n\n",tmnow->tm_mon + 1, tmnow->tm_mday);

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
	//% fprintf(fout, "\n\n总共有 %d 位寿星。\n", j);
	fprintf(fout, "\n\n\xd7\xdc\xb9\xb2\xd3\xd0 %d \xce\xbb\xca\xd9\xd0\xc7\xa1\xa3\n", j);
	fclose(fout);
	fclose(fp);
	return 0;
}
