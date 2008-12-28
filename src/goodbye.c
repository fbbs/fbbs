#include "bbs.h"
#include "glossary.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif

#define MAX_PERF (1000)

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
//	将经验值转换成显示字符,如 [---      ]等,[]不包括在内
//  显示的字符以2000为单位,分别是-=+*#A
//	显示的个数是将超出2000倍数的个数以200为单位再次划分,最多10个
char *cexpstr(int exp) {
	static char ce[11];
	char* c= "-=+*#A";
	int i;
	int j;
	strcpy(ce, "          ");
	if (exp < 0)
		return ce;
	i = exp / 2000;
	i = i > 5 ? 5 : i;
	j = (exp - i * 2000) / 200;
	j = j > 9 ? 9 : j;
	memset(ce, c[i], j + 1);
	return ce;
}

char *cnumposts(int num)
{
	if (num <= 0)
		return GLY_CPOST0;
	if (num > 0 && num <= 500)
		return GLY_CPOST1;
	if (num > 500 && num <= 1500)
		return GLY_CPOST2;
	if (num > 1500 && num <= 4000)
		return GLY_CPOST3;
	if (num > 4000 && num <= 10000)
		return GLY_CPOST4;
	if (num > 10000)
		return GLY_CPOST5;
}

#ifdef ALLOWGAME
char *cnummedals(int num)
{
	if(num == 0)
		return GLY_MEDAL0;
	if(num <= 300)
		return GLY_MEDAL1;
	if(num > 300 && num <= 1000)
		return GLY_MEDAL2;
	if(num > 1000 && num <= 3000)
		return GLY_MEDAL3;
	if(num > 3000)
		return GLY_MEDAL4;
}

char *cmoney(int num)
{
	if(num <= 100 )
		return GLY_MONEY0;
	if(num > 100 && num <= 3000)
		return GLY_MONEY1;
	if(num > 3000 && num <= 10000)
		return GLY_MONEY2;
	if(num > 10000 && num <= 50000)
		return GLY_MONEY3;
	if(num > 50000 && num <= 150000)
		return GLY_MONEY4;
	if(num > 150000 && num <= 300000)
		return GLY_MONEY5;
	if(num > 300000 && num <= 500000)
		return GLY_MONEY6;
	if(num > 500000)
		return GLY_MONEY7;
}
#endif

//	根据表现值perf计算表现值显示形式
char *cperf(int perf) {
	if (perf <= 100)
		return GLY_CPERF0;
	if (perf <= 200)
		return GLY_CPERF1;
	if (perf <= 400)
		return GLY_CPERF2;
	if (perf <= 500)
		return GLY_CPERF3;
	if (perf <= 550)
		return GLY_CPERF4;
	if (perf <= 600)
		return GLY_CPERF5;
	if (perf <= 650)
		return GLY_CPERF6;
	if (perf <= 700)
		return GLY_CPERF7;
	if (perf <= 800)
		return GLY_CPERF8;
	if (perf <= 900)
		return GLY_CPERF9;
	if (perf < 1000)
		return GLY_CPERFA;
	return GLY_CPERFB;
}

//计算经验值,文章数+登陆数的1/5+已注册的天数+停留的时数
//	最大值是12000
int countexp(struct userec *udata) {
	int exp;
	if (!strcmp(udata->userid, "guest"))
		return -9999;
	exp = udata->numposts + udata->numlogins / 5 + (time(0) 
		- udata->firstlogin) / 86400 + udata->stay / 3600;
	if (exp > 12000)
		exp = 12000;
	return exp > 0 ? exp : 0;
}

//	计算 表现值
int countperf(struct userec *udata) {
	int perf;
	int reg_days;
	if (!strcmp(udata->userid, "guest"))
		return -9999;
	reg_days = (time(0) - udata->firstlogin) / 86400 + 1;
	perf = (reg_days / 4 > 250 ? 250 : reg_days / 4);
	perf += (udata->stay / 14400 > 250 ? 250 : (udata->stay / 14400));
	perf += (udata->stay / (36 * reg_days) > 500 ? 500 : (udata->stay / (36
			*reg_days)));
	perf = (perf > MAX_PERF) ? MAX_PERF : perf;
	perf = (perf < 0) ? 0 : perf;

	return perf;
}

// Calculates Julian Date (after Oct. 1582) at noon UTC of given date.
int julian_day(int year, int month, int day)
{
	int a = (14 - month) / 12;
	int y = year + 4800 - a;
	int m = month + 12 * a - 3;
	return day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
}

// Calculates differences between given date (after Oct. 1582) and now.
int days_elapsed(int year, int month, int day, time_t now)
{
	//Epoch 1970.1.1 0:0:0 UTC = 2440587.5 (Julian Day)
	return (now - 43200) / 86400 + 2440588 - julian_day(year, month, day);
}

void showstuff(char *buf) {
	extern time_t login_start_time;
	int frg, i, matchfrg, strlength, cnt, tmpnum;
	static char numlogins[10], numposts[10], nummails[10], rgtday[30],
			lasttime[30], lastjustify[30], thistime[30], stay[10],
			alltime[20], ccperf[20], perf[10], exp[10], ccexp[20], star[5];
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
		sprintf(alltime, "%d小时%d分钟", currentuser.stay / 3600,
				(currentuser.stay / 60) % 60);
		getdatestring(currentuser.firstlogin, NA);
		sprintf(rgtday, "%s", datestring);
		getdatestring(currentuser.lastlogin, NA);
		sprintf(lasttime, "%s", datestring);
		getdatestring(now, NA);
		sprintf(thistime, "%s", datestring);
		getdatestring(currentuser.lastjustify, NA);
		sprintf(lastjustify, "%24.24s", datestring);
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
