#include <math.h>
#include "bbs.h"
#include "glossary.h"
#include "fbbs/convert.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/title.h"
#include "fbbs/uinfo.h"

#define MAX_PERF (1000)

enum {
	MIN_BIRTH_YEAR = 1901,
	MAX_BIRTH_YEAR = 2009,
};

const char *cexpstr(int exp)
{
	const char *c = "-=+*#A";
	static char ce[11];
	memset(ce, ' ', 10);

	if (exp < 0)
		return ce;
	exp = sqrt(exp / 5);
	int i, j;
	i = exp / 10;
	i = i > 5 ? 5 : i;
	j = exp - i * 10;
	j = j > 9 ? 9 : j;
	memset(ce, c[i], j + 1);
	return ce;
}

#ifdef ALLOWGAME
char *cnummedals(int num)
{
	if(num <= 0)
		return GLY_MEDAL0;
	if(num <= 300)
		return GLY_MEDAL1;
	if(num <= 1000)
		return GLY_MEDAL2;
	if(num <= 3000)
		return GLY_MEDAL3;
	return GLY_MEDAL4;
}

char *cmoney(int num)
{
	if(num <= 100)
		return GLY_MONEY0;
	if(num <= 3000)
		return GLY_MONEY1;
	if(num <= 10000)
		return GLY_MONEY2;
	if(num <= 50000)
		return GLY_MONEY3;
	if(num <= 150000)
		return GLY_MONEY4;
	if(num <= 300000)
		return GLY_MONEY5;
	if(num <= 500000)
		return GLY_MONEY6;
	return GLY_MONEY7;
}
#endif

//	根据表现值perf计算表现值显示形式
char *cperf(int perf)
{
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
int countexp(const struct userec *udata)
{
	int exp;
	if (!strcmp(udata->userid, "guest"))
		return 0;
	exp = udata->numposts + udata->numlogins / 5 + (time(0) 
		- udata->firstlogin) / 86400 + udata->stay / 3600;
	if (exp > 18000)
		exp = 18000;
	return exp > 0 ? exp : 0;
}

//	计算 表现值
int countperf(const struct userec *udata) {
	int perf;
	int reg_days;
	if (!strcmp(udata->userid, "guest"))
		return 0;
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

const char *horoscope(char month, char day)
{
	static const char *name[12] = {
		//% "摩羯座" "水瓶座"
		"\xc4\xa6\xf4\xc9\xd7\xf9", "\xcb\xae\xc6\xbf\xd7\xf9",
		//% "双鱼座" "牡羊座"
		"\xcb\xab\xd3\xe3\xd7\xf9", "\xc4\xb5\xd1\xf2\xd7\xf9",
		//% "金牛座" "双子座"
		"\xbd\xf0\xc5\xa3\xd7\xf9", "\xcb\xab\xd7\xd3\xd7\xf9",
		//% "巨蟹座" "狮子座"
		"\xbe\xde\xd0\xb7\xd7\xf9", "\xca\xa8\xd7\xd3\xd7\xf9",
		//% "处女座" "天秤座"
		"\xb4\xa6\xc5\xae\xd7\xf9", "\xcc\xec\xb3\xd3\xd7\xf9",
		//% "天蝎座" "射手座"
		"\xcc\xec\xd0\xab\xd7\xf9", "\xc9\xe4\xca\xd6\xd7\xf9"
	};
	switch (month) {
		case 1:
			if (day < 21)
				return (name[0]);
			else
				return (name[1]);
		case 2:
			if (day < 19)
				return (name[1]);
			else
				return (name[2]);
		case 3:
			if (day < 21)
				return (name[2]);
			else
				return (name[3]);
		case 4:
			if (day < 21)
				return (name[3]);
			else
				return (name[4]);
		case 5:
			if (day < 21)
				return (name[4]);
			else
				return (name[5]);
		case 6:
			if (day < 22)
				return (name[5]);
			else
				return (name[6]);
		case 7:
			if (day < 23)
				return (name[6]);
			else
				return (name[7]);
		case 8:
			if (day < 23)
				return (name[7]);
			else
				return (name[8]);
		case 9:
			if (day < 23)
				return (name[8]);
			else
				return (name[9]);
		case 10:
			if (day < 24)
				return (name[9]);
			else
				return (name[10]);
		case 11:
			if (day < 23)
				return (name[10]);
			else
				return (name[11]);
		case 12:
			if (day < 22)
				return (name[11]);
			else
				return (name[0]);
	}
	//% "不详座"
	return "\xb2\xbb\xcf\xea\xd7\xf9";
}

enum {
	HP_NEW = 14,
#ifdef FDQUAN
	HP_NORMAL1 = 150,
	HP_NORMAL2 = 365,
	HP_NORMAL3 = 666,
	HP_PERMANENT = 999,
#else
	HP_NORMAL1 = 180,
	HP_NORMAL2 = 240,
	HP_LONG1 = 365,
	HP_LONG2 = 527,
	HP_PERMANENT1 = 666,
	HP_PERMANENT2 = 999,
#endif
};

/**
 * Compute user's hp.
 * @param urec User record.
 * @return user's hp.
 */
int compute_user_value(const struct userec *urec)
{
	time_t now = time(NULL);
	int deduction = (now - urec->lastlogin) / (24 * 60 * 60);
	int rdays = (now - urec->firstlogin) / (24 * 60 * 60);

#ifdef FDQUAN
	if ((urec->userlevel & PERM_XEMPT)
			|| streq(urec->userid, "SYSOP") || streq(urec->userid, "guest"))
		return HP_PERMANENT;
	if (!(urec->userlevel & PERM_REGISTER))
		return HP_NEW - deduction;
	if (rdays >= 5 * 365)
		return HP_NORMAL3 - deduction;
	if (rdays >= 2 * 365)
		return HP_NORMAL2 - deduction;
	return HP_NORMAL1 - deduction;
#else
	if (((urec->userlevel & PERM_XEMPT) 
			&& (urec->userlevel & PERM_LONGLIFE))
			|| streq(urec->userid, "SYSOP") || streq(urec->userid, "guest"))
		return HP_PERMANENT2;
	if ((urec->userlevel & PERM_XEMPT) 
			&& !(urec->userlevel & PERM_LONGLIFE))
		return HP_PERMANENT1;
	if (!(urec->userlevel & PERM_REGISTER))
		return HP_NEW - deduction;
	if (urec->userlevel & PERM_SPECIAL1 && !(urec->userlevel & PERM_SPECIAL0))
		return HP_LONG2 - deduction;
	if (!(urec->userlevel & PERM_XEMPT)
			&& (urec->userlevel	& PERM_LONGLIFE))
		return HP_LONG1 - deduction;
	if (rdays >= 2 * 365)
		return HP_NORMAL2 - deduction;
	return HP_NORMAL1 - deduction;
#endif
}

static int show_bm(const char *userid, char **buf, size_t *size)
{
	char file[HOMELEN], tmp[STRLEN];
	sethomefile(file, userid, ".bmfile");
	FILE *fp = fopen(file, "r");
	if (fp) {
		strappend(buf, size, "[\033[1;33m");
		while (fgets(tmp, sizeof(tmp), fp) != NULL) {
			tmp[strlen(tmp) - 1] = ' ';
			strappend(buf, size, tmp);
		}
		//% "\033[32m版版主\033[m]"
		strappend(buf, size, "\033[32m\xb0\xe6\xb0\xe6\xd6\xf7\033[m]");
		fclose(fp);
		return 1;
	}
	return 0;
}

void show_position(const struct userec *user, char *buf, size_t size, const char *title)
{
	buf[0] = '\0';
	const char *orig = buf;

	if (user->userlevel & PERM_SPECIAL9) {
		if (user->userlevel & PERM_SYSOPS) {
			strappend(&buf, &size, "[\033[1;32m站长\033[m]");
		} else if (user->userlevel & PERM_ANNOUNCE) {
			//% "[\033[1;32m站务\033[m]"
			strappend(&buf, &size, "[\033[1;32m站务\033[m]");
		} else if (user->userlevel & PERM_OCHAT) {
			//% "[\033[1;32m实习站务\033[m]"
			strappend(&buf, &size, "[\033[1;32m实习站务\033[m]");
		} else if (user->userlevel & PERM_SPECIAL0) {
			//% "[\033[1;32m站务委员会秘书\033[m]"
			strappend(&buf, &size, "[\033[1;32m站务委员会秘书\033[m]");
		} else {
			//% "[\033[1;32m离任站务\033[m]"
			strappend(&buf, &size, "[\033[1;32m离任站务\033[m]");
		}
	} else {
		if ((user->userlevel & PERM_XEMPT)
				&& (user->userlevel & PERM_LONGLIFE)
				&& (user->userlevel & PERM_LARGEMAIL)) {
			//% "[\033[1;32m荣誉版主\033[m]"
			strappend(&buf, &size, "[\033[1;32m荣誉版主\033[m]");
		}
		if (user->userlevel & PERM_BOARDS)
			show_bm(user->userid, &buf, &size);
		if (user->userlevel & PERM_ARBI)
			//% "[\033[1;32m仲裁组\033[m]"
			strappend(&buf, &size, "[\033[1;32m仲裁组\033[m]");
		if (user->userlevel & PERM_SERV)
			//% "[\033[1;32m培训组\033[m]"
			strappend(&buf, &size, "[\033[1;32m培训组\033[m]");
		if (user->userlevel & PERM_SPECIAL2)
			//% "[\033[1;32m服务组\033[m]"
			strappend(&buf, &size, "[\033[1;32m服务组\033[m]");
		if (user->userlevel & PERM_SPECIAL3)
			//% "[\033[1;32m美工组\033[m]"
			strappend(&buf, &size, "[\033[1;32m美工组\033[m]");
		if (user->userlevel & PERM_TECH)
			//% "[\033[1;32m技术组\033[m]"
			strappend(&buf, &size, "[\033[1;32m技术组\033[m]");
	}

	if (title && *title) {
		char tbuf[TITLE_CCHARS * 4 + 13];
		snprintf(tbuf, sizeof(tbuf), "[\033[1;33m%s\033[m]", title);
		strappend(&buf, &size, tbuf);
	}

	if (!*orig)
		snprintf(buf, size, "[\033[1;32m" BBSNAME_SHORT "网友\033[m]");
}

/**
 *
 *
 */
int check_user_profile(const struct userec *u)
{
	if (strlen(u->username) < 2 || strstr(u->username, "  ")
			//% "　"
			|| (strstr(u->username, "\xa1\xa1")))
		return UINFO_ENICK;

	if (strchr("MF", u->gender) == NULL)
		return UINFO_EGENDER;

	if (u->birthyear + 1900 < MIN_BIRTH_YEAR || u->birthyear + 1900 > MAX_BIRTH_YEAR)
		return UINFO_EBIRTH;
	if (!valid_date(u->birthyear + 1900, u->birthmonth, u->birthday))
		return UINFO_EBIRTH;

	return 0;
}

int update_user_stay(struct userec *u, bool is_login, bool is_dup)
{
	time_t now = time(NULL);
	time_t last = u->lastlogout > u->lastlogin ? u->lastlogout : u->lastlogin;
	int stay = now - last;
	if (stay < 0)
		stay = 0;
	if (!is_login || is_dup)
		u->stay += stay;
	if (is_login)
		u->lastlogin = now;
	else
		u->lastlogout = now;
	return stay;
}

int uinfo_load(const char *name, uinfo_t *u)
{
	u->contrib = u->money = u->rank = 0;
	u->title = NULL;

	u->res = db_query("SELECT title"
#ifdef ENABLE_BANK
			", money, rank, contrib"
#endif
			" FROM alive_users WHERE lower(name) = lower(%s)", name);
	if (!u->res)
		return -1;

	u->title = db_get_value(u->res, 0, 0);
#ifdef ENABLE_BANK
	u->money = db_get_bigint(u->res, 0, 1);
	u->rank = db_get_float(u->res, 0, 2);
	u->contrib = db_get_bigint(u->res, 0, 3);
#endif
	return 0;
}

void uinfo_free(uinfo_t *u)
{
	if (u && u->res)
		db_clear(u->res);
}
