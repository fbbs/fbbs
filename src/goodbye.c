#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif

extern char BoardName[];
typedef struct {
	char *match;
	char *replace;
} logout;

int countlogouts(char *filename)
{
	FILE *fp;
	char buf[256];
	int count = 0;
	if ((fp = fopen(filename, "r")) == NULL)
	return 0;

	while (fgets(buf, 255, fp) != NULL) {
		if (strstr(buf, "@logout@") || strstr(buf, "@login@"))
		count++;
	}
	return count + 1;
}

void showstuff(char *buf);

void user_display(char *filename, int number, int mode)
{
	FILE *fp;
	char buf[256];
	int count = 1;
	clear();
	move(1, 0);
	if ((fp = fopen(filename, "r")) == NULL)
		return;
	while (fgets(buf, 255, fp) != NULL) {
		if (strstr(buf, "@logout@") || strstr(buf, "@login@")) {
			count++;
			continue;
		}
		if (count == number) {
			if (mode == YEA)
				showstuff(buf);
			else {
				prints("%s", buf);
			}
		}
		else if (count> number)
			break;
		else
			continue;
	}
	refresh();
	fclose(fp);
	return;
}

void showstuff(char *buf) {
	extern time_t login_start_time;
	int frg, i, matchfrg, strlength, cnt, tmpnum;
	static char numlogins[10], numposts[10], nummails[10], rgtday[30],
			lasttime[30], lastjustify[30], thistime[30], stay[10],
			alltime[20], ccperf[20], perf[10], exp[10], ccexp[20], star[7];
#ifdef ALLOWGAME
	static char moneys[10];
#endif
	char buf2[STRLEN], *ptr, *ptr2;
	time_t now;

	static logout loglst[] = { "userid", currentuser.userid, "username",
			currentuser.username, "realname", currentuser.realname,
			"address", currentuser.address, "email", currentuser.email,
			"termtype", currentuser.termtype, "realemail",
			currentuser.reginfo, "rgtday",
			rgtday, "login", numlogins, "post", numposts, "mail",
			nummails, "lastlogin", lasttime, "lasthost",
					currentuser.lasthost, "lastjustify", lastjustify,
					"now", thistime, "bbsname", BoardName, "stay", stay,
					"alltime", alltime,
#ifdef SHOWEXP
					"exp", exp,
#endif
#ifdef ALLOWGAME
					"money", moneys,
#endif
					"cexpstr", ccexp,
#ifdef SHOWPERF
					"perf", perf,
#endif
					"cperf", ccperf, "star", star, "pst", numposts, "log",
					numlogins, "bbsip", BBSIP, "bbshost", BBSHOST, NULL,
					NULL };
	if (!strchr(buf, '$')) {
		//if (!limit)
		prints("%s", buf);
		//else
		//	prints("%.82s", buf);
		return;
	}
	now = time(0);
	/* for ansimore3() */

	if (currentuser.numlogins> 0) {
		tmpnum = countexp(&currentuser);
		sprintf(exp, "%d", tmpnum);
		strcpy(ccexp, cexpstr(tmpnum));
		tmpnum = countperf(&currentuser);
		sprintf(perf, "%d", tmpnum);
		strcpy(ccperf, cperf(tmpnum));
		sprintf(alltime, "%d–° ±%d∑÷÷”", currentuser.stay / 3600,
				(currentuser.stay / 60) % 60);
		sprintf(rgtday, "%s", getdatestring(currentuser.firstlogin, DATE_ZH));
		sprintf(lasttime, "%s", getdatestring(currentuser.lastlogin, DATE_ZH));
		sprintf(thistime, "%s", getdatestring(now, DATE_ZH));
		sprintf(lastjustify, "%24.24s", getdatestring(currentuser.lastjustify, DATE_ZH));
		sprintf(stay, "%d", (time(0) - login_start_time) / 60);
		sprintf(numlogins, "%d", currentuser.numlogins);
		sprintf(numposts, "%d", currentuser.numposts);
#ifdef ALLOWGAME
		sprintf(moneys,"%d",currentuser.money);
#endif
		sprintf(nummails, "%d", currentuser.nummails);
		sprintf(star, "%s", horoscope(currentuser.birthmonth,
				currentuser.birthday));
	}
	frg = 1;
	ptr2 = buf;
	do {
		if (ptr = strchr(ptr2, '$')) {
			matchfrg = 0;
			*ptr = '\0';
			prints("%s", ptr2);
			ptr += 1;
			for (i = 0; loglst[i].match != NULL; i++) {
				if (strstr(ptr, loglst[i].match) == ptr) {
					strlength = strlen(loglst[i].match);
					ptr2 = ptr + strlength;
					for (cnt = 0; *(ptr2 + cnt) == ' '; cnt++)
						;
					sprintf(buf2, "%-*.*s", cnt ? strlength + cnt
							: strlength + 1, strlength + cnt,
							loglst[i].replace);
					prints("%s", buf2);
					ptr2 += (cnt ? (cnt - 1) : cnt);
					matchfrg = 1;
					break;
				}
			}
			if (!matchfrg) {
				prints("$");
				ptr2 = ptr;
			}
		} else {
			//if (!limit)
			prints("%s", ptr2);
			//else
			//	prints("%.82s", ptr2);
			frg = 0;
		}
	} while (frg); //do
	return;
}
