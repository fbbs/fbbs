/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
/*
$Id: goodbye.c 366 2007-05-12 16:35:51Z danielfree $
*/

#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15
#define MAX_PERF (1000)
//#define MAX_LUCKY (20)
//added by iamfat 2002.08.29 for 浮动表现值

extern char BoardName[];
typedef struct {
	char   *match;
	char   *replace;
}
        logout;

int
countlogouts(filename)
char    filename[STRLEN];
{
	FILE   *fp;
	char    buf[256];
	int     count = 0;
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;

	while (fgets(buf, 255, fp) != NULL) {
		if (strstr(buf, "@logout@") || strstr(buf, "@login@"))
			count++;
	}
	return count + 1;
}

void showstuff(char *buf);

user_display(filename, number, mode)
char   *filename;
int     number, mode;
{
	FILE   *fp;
	char    buf[256];
	int     count = 1;
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
				showstuff(buf/*, 0*/);
			else {
				prints("%s", buf);
			}
		} else if (count > number)
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
char   *cexpstr(int exp)
{
	static char ce[11];
	char* c=   "-=+*#A";
	int i;
	int j;
	strcpy(ce, "          ");
	if(exp<0)return ce;
	i=exp/2000;
	i=i>5?5:i;
	j=(exp-i*2000)/200;
	j=j>9?9:j;
	memset(ce,c[i],j+1);
	return ce;
}
/*
{
	int orgdata;
	
	long randval;

	orgdata = exp;
    time(&randval);
    if (orgdata<=4) return "    *";
    if (orgdata<=16) 
        if (orgdata>(randval%12+4)) return "   **";
        else return "    *";
    if (orgdata<64)
        if (orgdata>(randval%48+16)) return "  ***";
        else return "   **";
    if (orgdata<256) 
        if (orgdata>(randval%192+64)) return " ****";
        else return "  ***";
    if (orgdata<640) 
        if (orgdata>(randval%384+256)) return "*****";
        else return " ****";
    return "*****";
//modified by roly 02.03.17

        if (exp == -9999)
                return GLY_CEXP0;
        if (exp <= 100 )
                return GLY_CEXP1;
        if (exp > 100  && exp <= 450 )
                return GLY_CEXP2;
        if (exp > 450 && exp <= 850 )
                return GLY_CEXP3;
        if (exp > 850 && exp <= 1500 )
                return GLY_CEXP4;
        if (exp > 1500 && exp <= 2500 )
                return GLY_CEXP5;
        if (exp > 2500 && exp <= 3000 )
                return GLY_CEXP6;
        if (exp > 3000 && exp <= 5000 )
                return GLY_CEXP7;
        if (exp > 5000 )
                return GLY_CEXP8;
}*/


char *cnumposts(int num)
{

        if(num <= 0 ) //modified by roly 02.01.24 (origin: num==0)
                return GLY_CPOST0;
        if(num>0&&num<=500)
                return GLY_CPOST1;
        if(num>500&&num<=1500)
                return GLY_CPOST2;
        if(num>1500&&num<=4000)
                return GLY_CPOST3;
        if(num>4000&&num<=10000)
                return GLY_CPOST4;
        if(num>10000)
                return GLY_CPOST5;

}

#ifdef ALLOWGAME
char *
cnummedals(num)
int num;
{

        if(num== 0 )
                return  GLY_MEDAL0;
        if(num<=300)
                return  GLY_MEDAL1;
        if(num>300&&num<=1000)
                return  GLY_MEDAL2;
        if(num>1000&&num<=3000)
                return  GLY_MEDAL3;
        if(num>3000)
                return  GLY_MEDAL4;

}

char *
cmoney(num)
int num;
{

        if(num<= 100 )
                return GLY_MONEY0;
        if(num>100&&num<=3000)
                return GLY_MONEY1;
        if(num>3000&&num<=10000)
                return GLY_MONEY2;
        if(num>10000&&num<=50000)
                return GLY_MONEY3;
        if(num>50000&&num<=150000)
                return GLY_MONEY4;
        if(num>150000&&num<=300000)
                return GLY_MONEY5;
        if(num>300000&&num<=500000)
                return GLY_MONEY6;
        if(num>500000)
                return GLY_MONEY7;

}
#endif

//	根据表现值perf计算表现值显示形式
//	可以先将perf/100,再用enum枚举GLY来直接跳转
char   * cperf(int     perf)
{

/* Commented by Amigo 2002.06.16. Change exp description. */
/* Add by shun .1999.6.6 1999.6.19*/
//    if (perf==100) return "好样的";
//    return "加油加油";
/* End */

/* Following if criterias modified by Amigo 2002.06.16. */
/* Modified by Amigo 2002.06.23. Change perf description. */
// modified by iamfat 2002.08.20
/*
        if (perf == -9999) return GLY_CPERF0;
        if (perf <= 100) return GLY_CPERF0;
        if (perf <= 200) return GLY_CPERF1;
        if (perf <= 400) return GLY_CPERF2;
        if (perf <= 500) return GLY_CPERF3;
        if (perf <= 550) return GLY_CPERF4;
        if (perf <= 600) return GLY_CPERF5;
        if (perf <= 650) return GLY_CPERF6;
        if (perf <= 700) return GLY_CPERF7;
        if (perf <= 800) return GLY_CPERF8;
        if (perf <= 900) return GLY_CPERF9;
        if (perf < 1000) return GLY_CPERFA;
        if (perf >= 1000)return GLY_CPERFB;
        return "机器人！";
		*/
        if (perf <= 100) return GLY_CPERF0;
        if (perf <= 200) return GLY_CPERF1;
        if (perf <= 400) return GLY_CPERF2;
        if (perf <= 500) return GLY_CPERF3;
        if (perf <= 550) return GLY_CPERF4;
        if (perf <= 600) return GLY_CPERF5;
        if (perf <= 650) return GLY_CPERF6;
        if (perf <= 700) return GLY_CPERF7;
        if (perf <= 800) return GLY_CPERF8;
        if (perf <= 900) return GLY_CPERF9;
        if (perf < 1000) return GLY_CPERFA;
		return GLY_CPERFB;
}

//计算经验值,文章数+登陆数的1/5+已注册的天数+停留的时数
//	最大值是12000
int countexp(struct userec *udata)
{
	int     exp;
	if (!strcmp(udata->userid, "guest"))
		return -9999;
	exp = 	udata->numposts 
		+	udata->numlogins / 5 
		+	(time(0) - udata->firstlogin) / 86400 
		+	udata->stay / 3600;
	//added by iamfat 2002.07.25
	if(exp>12000)exp=12000;
	return exp > 0 ? exp : 0;
}

//	计算 表现值
//
int	countperf(struct userec *udata)
{
	int     perf;
	int     reg_days;
	if (!strcmp(udata->userid, "guest"))//guest的表现值为-9999
		return -9999;
	reg_days = (time(0) - udata->firstlogin) / 86400 + 1;//已注册天数
	
/* Add by shun  1999.6.19 */
/*
   perf=(reg_days/20>20?20:reg_days/20);
   perf=perf+(udata->stay/36000>30?30:(udata->stay/36000));
   perf=perf+(udata->stay/(36*reg_days)>50?50:(udata->stay/(36*reg_days)));
*/
/* Modified by Amigo 2002.06.07. Change exp formula. */
   perf=(reg_days/4>250?250:reg_days/4);//已注册天数所得的表现值最多为250,每四天
   										//表现值加一点
   perf=perf+(udata->stay/14400>250?250:(udata->stay/14400));
   perf=perf+(udata->stay/(36*reg_days)>500?500:(udata->stay/(36*reg_days)));

   //added by iamfat 2002.08.29 for 浮动表现值
   /*
   randomize();
   perf=perf + rand()%(MAX_LUCKY*2)-MAX_LUCKY;
  */
 
   perf=(perf>MAX_PERF)?MAX_PERF:perf;
   perf=(perf<0)?0:perf;

   return perf;
/* End */

	/*
	 * 990530.edwardc 注册没成功或还在注册的人的人会导致 reg_days = 0,
	 * 然後在下面会产生 SIGFPE, 除数为零的错误 ..
	 */

	 /*
	if (reg_days <= 0)
		return 0;
	perf = (((udata->numposts>=0)?((float) udata->numposts):0) / (float) udata->numlogins +
		(float) udata->numlogins / (float) reg_days) * 10;
	return perf > 0 ? perf : 0;*/
}

//	返回年月日为*year,*month,*day的日子与now所代表的日子之间
//		相差的天数
void countdays(int *year,int *month,int *day,time_t now)
{
	struct tm *GoodTime;
    time_t tmptime;
	int	t = 0;
    GoodTime = localtime(&now);
    GoodTime->tm_year = *year - 1900;
    GoodTime->tm_mon = *month-1;
    GoodTime->tm_mday = *day;
    GoodTime->tm_hour = 0;
    GoodTime->tm_min = 0;
    /* mktime函数在tm_year<70时，返回-1 */
	/* 因此需要判断年份小于70年的情况 modified by money 04.02.18*/
	if (GoodTime->tm_year < 70)
	{
		t = 70 - GoodTime->tm_year;
		GoodTime->tm_year = 70;
		tmptime = mktime(GoodTime);
		t = t*365;
	}
	else
		tmptime = mktime(GoodTime);
	*year = (tmptime-now)/86400 - t;
    *month = (tmptime-now-*year*86400)/3600;
    *day = (tmptime-now-*year*86400-*month*3600)/60;
}
/*
void showstuff(buf, limit)
char    buf[256];
int     limit;
*/
void showstuff(char *buf)
{
	extern time_t login_start_time;
	int     frg, i, matchfrg, strlength, cnt, tmpnum;
	static char numlogins[10], numposts[10], nummails[10], rgtday[30], 
			lasttime[30],lastjustify[30], thistime[30], stay[10], 
	        alltime[20], ccperf[20],perf[10], exp[10], ccexp[20],
	        star[5];
#ifdef ALLOWGAME
	static char moneys[10];
#endif
	char    buf2[STRLEN], *ptr, *ptr2;
	time_t  now;

	static logout loglst[] =	{
		"userid", 		currentuser.userid,
		"username", 	currentuser.username,
		"realname", 	currentuser.realname,
		"address", 		currentuser.address,
		"email", 		currentuser.email,
		"termtype", 	currentuser.termtype,
		"realemail",	currentuser.reginfo,
		"ident", 		currentuser.ident,
		"rgtday", 		rgtday,
		"login", 		numlogins,
		"post", 		numposts,
		"mail", 		nummails,
		"lastlogin",	lasttime,
		"lasthost", 	currentuser.lasthost,
		"lastjustify", 	lastjustify,
		"now", 			thistime,
		"bbsname", 		BoardName,
		"stay", 		stay,
		"alltime", 		alltime,
#ifdef SHOWEXP
		"exp", 			exp,
#endif
#ifdef ALLOWGAME
		"money",		moneys,
#endif
		"cexpstr", 		ccexp,
#ifdef SHOWPERF
		"perf", 		perf,
#endif
		"cperf", 		ccperf,
		"star", 		star,
		"pst", 			numposts,
		"log", 			numlogins,
		"bbsip", 		BBSIP,
		"bbshost", 		BBSHOST,
		NULL, 			NULL,
	};
	if (!strchr(buf, '$')) {
		//if (!limit)
			prints("%s", buf);
		//else
		//	prints("%.82s", buf);
		return;
	}
	now = time(0);
	/* for ansimore3() */

	if (currentuser.numlogins > 0) {
		tmpnum = countexp(&currentuser);
		sprintf(exp, "%d", tmpnum);
		strcpy(ccexp, cexpstr(tmpnum));
		tmpnum = countperf(&currentuser);
		sprintf(perf, "%d", tmpnum);
		strcpy(ccperf, cperf(tmpnum));
		sprintf (	alltime, "%d小时%d分钟", 
					currentuser.stay / 3600, 
					(currentuser.stay / 60) % 60
				);
		getdatestring(currentuser.firstlogin,NA);
		sprintf(rgtday, "%s", datestring);
		getdatestring(currentuser.lastlogin,NA);
		sprintf(lasttime, "%s", datestring);
		getdatestring(now,NA);
		sprintf(thistime, "%s", datestring);
		getdatestring(currentuser.lastjustify,NA);
		sprintf(lastjustify, "%24.24s", datestring);
		sprintf(stay, "%d", (time(0) - login_start_time) / 60);
		sprintf(numlogins, "%d", currentuser.numlogins);
		sprintf(numposts, "%d", currentuser.numposts);
#ifdef ALLOWGAME
		sprintf(moneys,"%d",currentuser.money);
#endif
		sprintf(nummails, "%d", currentuser.nummails);
		sprintf(star, "%s", horoscope(currentuser.birthmonth, currentuser.birthday));
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
					for (cnt = 0; *(ptr2 + cnt) == ' '; cnt++);
					sprintf(buf2, "%-*.*s", cnt ? strlength + cnt : strlength + 1, 
							strlength + cnt, loglst[i].replace);
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
