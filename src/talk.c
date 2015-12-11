#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "fbbs/dbi.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/money.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/uinfo.h"

#define M_INT 8			/* monitor mode update interval */
#define P_INT 20		/* interval to check for page req. in
				 * talk/chat */
extern char BoardName[];
extern int numf, friendmode;
extern int t_columns;
int cmpfnames();

static int show_one_file(const char *filename)
{
	int i, j, ci;
	char pbuf[256];
	FILE *pf;
	if ((pf = fopen(filename, "r")) == NULL) {
		return NA;
	} else {
		for (i = 1; i <= MAXQUERYLINES; i++) {
			if (fgets(pbuf, sizeof(pbuf), pf)) {
				for (j = 0; j < strlen(pbuf); j++)
					//Modified by IAMFAT 2002.06.05
					if (pbuf[j] != '\033') {
						if (pbuf[j]!='\r')
							outc(pbuf[j]);
					} else {
						ci = strspn(&pbuf[j], "\033[0123456789;");
						if (pbuf[ci + j] != 'm')
							j += ci;
						else
							outc(pbuf[j]);
					}
			} else
				break;
		}
		fclose(pf);
		return YEA;
	}
}

/*Add by SmallPig*/
/*此函数只负责列印说明档，并不管清除或定位的问题。*/

int
show_user_plan(const char *userid)
{
	char pfile[STRLEN];
	sethomefile(pfile, userid, "plans");
	if (show_one_file(pfile)==NA) {
		//% prints("\033[1;36m没有个人说明档\033[m\n");
		prints("\033[1;36m\xc3\xbb\xd3\xd0\xb8\xf6\xc8\xcb\xcb\xb5\xc3\xf7\xb5\xb5\033[m\n");
		return NA;
	}
	return YEA;
}

extern char fromhost[60];

enum {
	IPADDR_OMIT_THRES = 36,
};

static void show_statuses(session_basic_info_t *res)
{
	if (session_basic_info_count(res) > 0)
		//% prints("目前状态如下：\n");
		prints("\xc4\xbf\xc7\xb0\xd7\xb4\xcc\xac\xc8\xe7\xcf\xc2\xa3\xba\n");

	for (int i = 0; i < session_basic_info_count(res); ++i) {
		bool visible = session_basic_info_visible(res, i);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		session_id_t sid = session_basic_info_sid(res, i);
		bool web = session_basic_info_web(res, i);
		int status = get_user_status(sid);
		int idle = (time(NULL) - session_get_idle(sid)) / 60;

		const char *color = session_status_color(status, visible, web);
		prints("\033[1m%s%s\033[m", color, session_status_descr(status));

		if (idle >= 1 && status != ST_BBSNET)
			prints("[%d] ", idle);
		else
			prints("    ");

		if ((i + 1) % 5 == 0)
			outc('\n');
	}
	outc('\n');
}

/**
 *
 */
int tui_query_result(const char *userid)
{
	struct userec user;
	int unum = getuserec(userid, &user);
	if (!unum)
		return -1;

	screen_move(0, 0);
	screen_clrtobot();

	int color = 2;
	if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
		color = (user.gender == 'F') ? 5 : 6;
	char horo[32] = "";
	if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)
			&& strcasecmp(user.userid, "guest") != 0) {
		snprintf(horo, sizeof(horo), "[\033[1;3%dm%s\033[m] ",
				color, horoscope(user.birthmonth, user.birthday));
	}
	//% prints("\033[0;1;37m%s \033[m(\033[1;33m%s\033[m) 共上站 \033[1;32m%d\033[m "
	prints("\033[0;1;37m%s \033[m(\033[1;33m%s\033[m) \xb9\xb2\xc9\xcf\xd5\xbe \033[1;32m%d\033[m "
			//% "次  %s\n", user.userid, user.username, user.numlogins, horo);
			"\xb4\xce  %s\n", user.userid, user.username, user.numlogins, horo);

	bool self = !strcmp(currentuser.userid, user.userid);
	const char *host;
	if (user.lasthost[0] == '\0') {
		//% host = "(不详)";
		host = "(\xb2\xbb\xcf\xea)";
	} else {
		if (self || HAS_PERM2(PERM_OCHAT, &currentuser))
			host = user.lasthost;
		else 
			host = mask_host(user.lasthost);
	}
	//% prints("进站 [\033[1;32m%s\033[m] %s[\033[1;32m%s\033[m]\n",
	prints("\xbd\xf8\xd5\xbe [\033[1;32m%s\033[m] %s[\033[1;32m%s\033[m]\n",
			format_time(user.lastlogin, TIME_FORMAT_ZH),
			//% strlen(host) > IPADDR_OMIT_THRES ? "" : "来自 ", host);
			strlen(host) > IPADDR_OMIT_THRES ? "" : "\xc0\xb4\xd7\xd4 ", host);

	user_id_t uid = get_user_id(userid);
	session_basic_info_t *res = get_sessions(uid);

	if (res && session_basic_info_count(res) > 0) {
		//% prints("在线 [\033[1;32m讯息器:(\033[36m%s\033[32m)\033[m] ",
		prints("\xd4\xda\xcf\xdf [\033[1;32m\xd1\xb6\xcf\xa2\xc6\xf7:(\033[36m%s\033[32m)\033[m] ",
				//% "打开");
				"\xb4\xf2\xbf\xaa");
	} else {
		fb_time_t t = user.lastlogout;
		if (user.lastlogout < user.lastlogin)
			t = ((fb_time() - user.lastlogin) / 120) % 47 + 1 + user.lastlogin;
		//% prints("离站 [\033[1;32m%s\033[m] ", format_time(t, TIME_FORMAT_ZH));
		prints("\xc0\xeb\xd5\xbe [\033[1;32m%s\033[m] ", format_time(t, TIME_FORMAT_ZH));
	}

	char path[HOMELEN];
	snprintf(path, sizeof(path), "mail/%c/%s/%s",
			toupper(user.userid[0]), user.userid, DOT_DIR);
	int perf = countperf(&user);
	//% prints("表现值 "
	prints("\xb1\xed\xcf\xd6\xd6\xb5 "
#ifdef SHOW_PERF
			"%d(\033[1;33m%s\033[m)"
#else
			"[\033[1;33m%s\033[m]"
#endif
			//% " 信箱 [\033[1;5;32m%2s\033[m]\n"
			" \xd0\xc5\xcf\xe4 [\033[1;5;32m%2s\033[m]\n"
#ifdef SHOW_PERF
			, perf
#endif
			//% , cperf(perf), (check_query_mail(path) == 1) ? "信" : "  ");
			, cperf(perf), (check_query_mail(path) == 1) ? "\xd0\xc5" : "  ");

	int exp = countexp(&user);

	uinfo_t u;
	uinfo_load(user.userid, &u);

#ifdef ENABLE_BANK
	//% prints("贡献 [\033[1;32m%d\033[m", TO_YUAN_INT(u.contrib));
	prints("\xb9\xb1\xcf\xd7 [\033[1;32m%d\033[m", TO_YUAN_INT(u.contrib));
	if (self || HAS_PERM2(PERM_OCHAT, &currentuser)) {
		prints("/\033[1;33m%d\033[m", TO_YUAN_INT(u.money));
	}
	{
		char rank_buf[8];
		snprintf(rank_buf, sizeof(rank_buf), "%.1f%%", PERCENT_RANK(u.rank));
		prints("](%s) ", rank_buf);
	}

#endif

#ifdef ALLOWGAME
	//% prints("存贷款 [\033[1;32m%d\033[m/\033[1;32m%d\033[m]"
	prints("\xb4\xe6\xb4\xfb\xbf\xee [\033[1;32m%d\033[m/\033[1;32m%d\033[m]"
			//% "(\033[1;33m%s\033[m) 经验值 [\033[1;32m%d\033[m]\n",
			"(\033[1;33m%s\033[m) \xbe\xad\xd1\xe9\xd6\xb5 [\033[1;32m%d\033[m]\n",
			user.money, user.bet, cmoney(user.money - user.bet), exp);
	//% prints("发文 [\033[1;32m%d\033[m] 奖章 [\033[1;32m%d\033[m]"
	prints("\xb7\xa2\xce\xc4 [\033[1;32m%d\033[m] \xbd\xb1\xd5\xc2 [\033[1;32m%d\033[m]"
			//% "(\033[1;33m%s\033[m) 生命力 [\033[1;32m%d\033[m]\n",
			"(\033[1;33m%s\033[m) \xc9\xfa\xc3\xfc\xc1\xa6 [\033[1;32m%d\033[m]\n",
			user.numposts, user.nummedals, cnummedals(user.nummedals),
			compute_user_value(&user));
#else
	//% prints("发文 [\033[1;32m%d\033[m] ", user.numposts);
	prints("\xb7\xa2\xce\xc4 [\033[1;32m%d\033[m] ", user.numposts);
	//% prints("经验值 [\033[1;33m%-10s\033[m]", cexpstr(exp));
	prints("\xbe\xad\xd1\xe9\xd6\xb5 [\033[1;33m%-10s\033[m]", cexpstr(exp));
#ifdef SHOWEXP
	prints("(%d)", exp);
#endif
	//% prints(" 生命力 [\033[1;32m%d\033[m]\n", compute_user_value(&user));
	prints(" \xc9\xfa\xc3\xfc\xc1\xa6 [\033[1;32m%d\033[m]\n", compute_user_value(&user));
#endif

	char buf[320];
	show_position(&user, buf, sizeof(buf), u.title);
	screen_printf("身份 %s\n", buf);
	
	uinfo_free(&u);

	show_statuses(res);
	session_basic_info_clear(res);

	show_user_plan(userid);
	return 0;
}

int t_query(const char *uname)
{
	if (streq(currentuser.userid, "guest"))
		return DONOTHING;

	char userid[EXT_IDLEN + 1];
	if (uname) {
		if (*uname == '\0')
			return DONOTHING;
		strlcpy(userid, uname, sizeof(userid));
		strtok(userid, " ");
	} else {
		set_user_status(ST_QUERY);
		screen_flush();
		screen_move(2, 0);
		screen_clrtobot();
		screen_printf("<输入使用者代号, 按空白键可列出符合字串>");
		user_complete(1, "查询谁: ", userid, sizeof(userid));
		if (*userid == '\0')
			return FULLUPDATE;
	}

	if (tui_query_result(userid) != 0) {
		screen_move_clear(2);
		screen_printf("\033[1m不正确的使用者代号\033[m\n");
		pressanykey();
		return FULLUPDATE;
	}

	if (session_status() != ST_LUSERS && session_status() != ST_LAUSERS
			&& session_status() != ST_FRIEND && session_status() != ST_GMENU)
	pressanykey();
	return FULLUPDATE;
}

int tui_query(void)
{
	return t_query(NULL);
}
