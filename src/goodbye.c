#include "bbs.h"
#include "fbbs/terminal.h"
#include "fbbs/uinfo.h"

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

int countlogouts(char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
		return 0;

	char buf[256];
	int count = 0;
	while (fgets(buf, 255, fp)) {
		if (strstr(buf, "@logout@") || strstr(buf, "@login@"))
			++count;
	}
	fclose(fp);
	return count + 1;
}

void showstuff(char *buf);

void user_display(char *filename, int number, int mode)
{
	FILE *fp;
	char buf[256];
	int count = 1;
	screen_clear();
	screen_move(1, 0);
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
	screen_flush();
	fclose(fp);
	return;
}

void showstuff(char *buf) {
	extern time_t login_start_time;
	int frg, i, matchfrg, strlength, cnt, tmpnum;
	static char numlogins[10], numposts[10], nummails[10], rgtday[30],
			lasttime[30], thistime[30], stay[10],
			alltime[20], ccperf[20], perf[10], exp[10], ccexp[20], star[7];
#ifdef ALLOWGAME
	static char moneys[10];
#endif
	char buf2[STRLEN], *ptr, *ptr2;

	static logout loglst[] = {
			{ "userid", currentuser.userid },
			{ "username", currentuser.username },
			{ "email", currentuser.email },
			{ "rgtday", rgtday },
			{ "login", numlogins },
			{ "post", numposts },
			{ "mail", nummails },
			{ "lastlogin", lasttime },
			{ "lasthost", currentuser.lasthost },
			{ "now", thistime },
			{ "bbsname", BoardName },
			{ "stay", stay },
			{ "alltime", alltime },
#ifdef SHOWEXP
			{ "exp", exp },
#endif
#ifdef ALLOWGAME
			{ "money", moneys },
#endif
			{ "cexpstr", ccexp },
#ifdef SHOWPERF
			{ "perf", perf },
#endif
			{ "cperf", ccperf },
			{ "star", star },
			{ "pst", numposts },
			{ "log", numlogins },
			{ "bbsip", BBSIP },
			{ "bbshost", BBSHOST },
			{ NULL, NULL }
	};
	if (!strchr(buf, '$')) {
		//if (!limit)
		prints("%s", buf);
		//else
		//	prints("%.82s", buf);
		return;
	}
	fb_time_t now = fb_time();

	if (currentuser.numlogins> 0) {
		tmpnum = countexp(&currentuser);
		sprintf(exp, "%d", tmpnum);
		strcpy(ccexp, cexpstr(tmpnum));
		tmpnum = countperf(&currentuser);
		sprintf(perf, "%d", tmpnum);
		strcpy(ccperf, cperf(tmpnum));
		//% sprintf(alltime, "%d小时%d分钟", currentuser.stay / 3600,
		sprintf(alltime, "%d\xd0\xa1\xca\xb1%d\xb7\xd6\xd6\xd3", currentuser.stay / 3600,
				(currentuser.stay / 60) % 60);
		sprintf(rgtday, "%s", format_time(currentuser.firstlogin, TIME_FORMAT_ZH));
		sprintf(lasttime, "%s", format_time(currentuser.lastlogin, TIME_FORMAT_ZH));
		sprintf(thistime, "%s", format_time(now, TIME_FORMAT_ZH));
		sprintf(stay, "%ld", (time(0) - login_start_time) / 60);
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
		if ((ptr = strchr(ptr2, '$'))) {
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
