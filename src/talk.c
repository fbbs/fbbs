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
#include "fbbs/fbbs.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/uinfo.h"

#define M_INT 8			/* monitor mode update interval */
#define P_INT 20		/* interval to check for page req. in
				 * talk/chat */
extern char BoardName[];
extern int iscolor;
extern int numf, friendmode;
int talkidletime = 0;

struct talk_win {
	int curcol, curln;
	int sline, eline;
};

int nowmovie;

extern int t_columns;

extern int t_cmpuids();
int cmpfnames();
char *ModeType();

int canpage(int friend, int pager) {
	if ((pager & ALL_PAGER) || HAS_PERM(PERM_OCHAT))
		return YEA;
	if ((pager & FRIEND_PAGER)) {
		if (friend) return YEA;
	}
	return NA;
}

/*Add by SmallPig*/
/*此函数只负责列印说明档，并不管清除或定位的问题。*/

int
show_user_plan(const char *userid)
{
	char pfile[STRLEN];
	sethomefile(pfile, userid, "plans");
	if (show_one_file(pfile)==NA) {
		prints("\033[1;36m没有个人说明档\033[m\n");
		return NA;
	}
	return YEA;
}

int show_one_file(char *filename) {
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

extern char fromhost[60];

enum {
	IPADDR_OMIT_THRES = 36,
};

static void show_statuses(db_res_t *res)
{
	if (db_res_rows(res) > 0)
		prints("目前状态如下：\n");

	for (int i = 0; i < db_res_rows(res); ++i) {
		bool visible = db_get_bool(res, i, 2);
		if (!visible && !HAS_PERM(PERM_SEECLOAK))
			continue;

		session_id_t sid = db_get_session_id(res, i, 0);
		bool web = db_get_bool(res, i, 3);
		int status = get_user_status(sid);
		int idle = (time(NULL) - get_idle_time(sid)) / 60;

		const char *color = get_status_color(status, visible, web);
		prints("\033[1m%s%s\033[m", color, mode_type(status));

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

	move(0, 0);
	clrtobot();

	int color = 2;
	if (HAS_DEFINE(user.userdefine, DEF_COLOREDSEX))
		color = (user.gender == 'F') ? 5 : 6;
	char horo[32] = "";
	if (HAS_DEFINE(user.userdefine, DEF_S_HOROSCOPE)
			&& strcasecmp(user.userid, "guest") != 0) {
		snprintf(horo, sizeof(horo), "[\033[1;3%dm%s\033[m] ",
				color, horoscope(user.birthmonth, user.birthday));
	}
	prints("\033[0;1;37m%s \033[m(\033[1;33m%s\033[m) 共上站 \033[1;32m%d\033[m "
			"次  %s\n", user.userid, user.username, user.numlogins, horo);

	bool self = !strcmp(currentuser.userid, user.userid);
	const char *host;
	if (user.lasthost[0] == '\0') {
		host = "(不详)";
	} else {
		if (self || HAS_PERM2(PERM_OCHAT, &currentuser))
			host = user.lasthost;
		else 
			host = mask_host(user.lasthost);
	}
	prints("进站 [\033[1;32m%s\033[m] %s[\033[1;32m%s\033[m]\n",
			getdatestring(user.lastlogin, DATE_ZH),
			strlen(host) > IPADDR_OMIT_THRES ? "" : "来自 ", host);

	user_id_t uid = get_user_id(userid);
	db_res_t *res = get_sessions(uid);

	if (res && db_res_rows(res) > 0) {
		prints("在线 [\033[1;32m讯息器:(\033[36m%s\033[32m)\033[m] ",
				"打开");// : "关闭",
	} else {
		fb_time_t t = user.lastlogout;
		if (user.lastlogout < user.lastlogin)
			t = ((time(NULL) - user.lastlogin) / 120) % 47 + 1 + user.lastlogin;
		prints("离站 [\033[1;32m%s\033[m] ", getdatestring(t, DATE_ZH));
	}

	char path[HOMELEN];
	snprintf(path, sizeof(path), "mail/%c/%s/%s",
			toupper(user.userid[0]), user.userid, DOT_DIR);
	int perf = countperf(&user);
	prints("表现值 "
#ifdef SHOW_PERF
			"%d(\033[1;33m%s\033[m)"
#else
			"[\033[1;33m%s\033[m]"
#endif
			" 信箱 [\033[1;5;32m%2s\033[m]\n"
#ifdef SHOW_PERF
			, perf
#endif
			, cperf(perf), (check_query_mail(path) == 1) ? "信" : "  ");

	int exp = countexp(&user);

	uinfo_t u;
	uinfo_load(user.userid, &u);

#ifdef ENABLE_BANK
	char rank_buf[8];
	snprintf(rank_buf, sizeof(rank_buf), "%.1f%%", PERCENT_RANK(u.rank));
	prints("贡献 [\033[1;32m%d\033[m](%s) ", TO_YUAN_INT(u.contrib), rank_buf);
	if (self || HAS_PERM2(PERM_OCHAT, &currentuser)) {
		prints("财富 [\033[1;32m%d\033[m] ", TO_YUAN_INT(u.money));
	}
#endif

#ifdef ALLOWGAME
	prints("存贷款 [\033[1;32m%d\033[m/\033[1;32m%d\033[m]"
			"(\033[1;33m%s\033[m) 经验值 [\033[1;32m%d\033[m]\n",
			user.money, user.bet, cmoney(user.money - user.bet), exp);
	prints("发文 [\033[1;32m%d\033[m] 奖章 [\033[1;32m%d\033[m]"
			"(\033[1;33m%s\033[m) 生命力 [\033[1;32m%d\033[m]\n",
			user.numposts, user.nummedals, cnummedals(user.nummedals),
			compute_user_value(&user));
#else
	prints("发文 [\033[1;32m%d\033[m] ", user.numposts);
	prints("经验值 [\033[1;33m%-10s\033[m]", cexpstr(exp));
#ifdef SHOWEXP
	prints("(%d)", exp);
#endif
	prints(" 生命力 [\033[1;32m%d\033[m]\n", compute_user_value(&user));
#endif

	char buf[160];
	show_position(&user, buf, sizeof(buf), u.title);
	prints("身份 %s\n", buf);
	
	uinfo_free(&u);

	show_statuses(res);
	db_clear(res);

	show_user_plan(userid);
	return 0;
}

int t_query(const char *user)
{
	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;

	char userid[EXT_IDLEN + 1];
	switch (session.status) {
		case ST_LUSERS:
		case ST_LAUSERS:
		case ST_FRIEND:
		case ST_READING:
		case ST_MAIL:
		case ST_RMAIL:
		case ST_GMENU:
			if (*user == '\0')
				return DONOTHING;
			strlcpy(userid, user, sizeof(userid));
			strtok(userid, " ");
			break;
		default:
			set_user_status(ST_QUERY);
			refresh();
			move(1, 0);
			clrtobot();
			prints("查询谁:\n<输入使用者代号, 按空白键可列出符合字串>\n");
			move(1, 8);
			usercomplete(NULL, userid);
			if (*userid == '\0')
				return FULLUPDATE;
			break;
	}

	if (tui_query_result(userid) != 0) {
		move(2, 0);
		clrtoeol();
		prints("\033[1m不正确的使用者代号\033[m\n");
		pressanykey();
		return FULLUPDATE;
	}

	if (session.status != ST_LUSERS && session.status != ST_LAUSERS
			&& session.status != ST_FRIEND && session.status != ST_GMENU)
	pressanykey();
	return FULLUPDATE;
}

int get_status(int uid)
{
	if (resolve_ucache() == -1)
		return 0;
	if (!HAS_PERM(PERM_SEECLOAK)
			&& (uidshm->passwd[uid - 1].userlevel & PERM_LOGINCLOAK)
			&& (uidshm->passwd[uid - 1].flags[0] & CLOAK_FLAG))
		return 0;
	return uidshm->status[uid - 1];
}

void num_alcounter(void)
{
	static time_t last=0;
	register int i;
	time_t now = time(0);
	if (last+10 > now)
		return;
	last=now;
	count_friends=0;
	for(i = 0; i < MAXFRIENDS && uinfo.friend[i]; i++) {
		count_friends+=get_status(uinfo.friend[i]);
	}
	return;
}

int t_cmpuids(int uid, const struct user_info *up)
{
	return (up->active && uid == up->uid);
}

void endmsg() {
	int x, y;
	int tmpansi;
	tmpansi = showansi;
	showansi = 1;
	talkidletime += 60;
	if (talkidletime >= IDLE_TIMEOUT)
		kill(getpid(), SIGHUP);
	getyx(&x, &y);
	update_endline();
	signal(SIGALRM, endmsg);
	move(x, y);
	refresh();
	alarm(60);
	showansi = tmpansi;
	return;
}

int listfilecontent(char *fname, int y) {
	FILE *fp;
	int x = 0, cnt = 0, max = 0, len;
	//char    u_buf[20], line[STRLEN], *nick;
	char u_buf[20], line[512], *nick;
	//modified by roly 02.03.22 缓存区溢出
	move(y, x);
	CreateNameList();
	strcpy(genbuf, fname);
	if ((fp = fopen(genbuf, "r")) == NULL) {
		prints("(none)\n");
		return 0;
	}
	while (fgets(genbuf, 1024, fp) != NULL) {
		strtok(genbuf, " \n\r\t");
		strlcpy(u_buf, genbuf, 20);
		u_buf[19] = '\0';
		if (!AddNameList(u_buf))
			continue;
		nick = (char *) strtok(NULL, "\n\r\t");
		if (nick != NULL) {
			while (*nick == ' ')
				nick++;
			if (*nick == '\0')
				nick = NULL;
		}
		if (nick == NULL) {
			strcpy(line, u_buf);
		} else {
			sprintf(line, "%-12s%s", u_buf, nick);
		}
		if ((len = strlen(line)) > max)
			max = len;
		if (x + len > 78)
			line[78 - x] = '\0';
		prints("%s", line);
		cnt++;
		if ((++y) >= t_lines - 1) {
			y = 3;
			x += max + 2;
			max = 0;
			if (x > 70)
				break;
		}
		move(y, x);
	}
	fclose(fp);
	if (cnt == 0)
		prints("(none)\n");
	return cnt;
}

struct user_info *t_search(char *sid, int pid) {
	int i;
	struct user_info *cur, *tmp = NULL;
	resolve_utmp();
	/* added by roly 02.06.02 */
	if (pid<0)
		return NULL;
	/* add end */

	for (i = 0; i < USHM_SIZE; i++) {
		cur = &(utmpshm->uinfo[i]);
		if (!cur->active || !cur->pid)
			continue;
		if (!strcasecmp(cur->userid, sid)) {
			if (pid == 0)
				return cur;
			tmp = cur;
			if (pid == cur->pid)
				break;
		}
	}
	/*
	 if (tmp != NULL) {
	 if (tmp->invisible && !HAS_PERM(PERM_SEECLOAK))
	 return NULL;  
	 }
	 */
	return tmp;
}

int
cmpfuid(a, b)
int *a, *b;
{
	return *a - *b;
}

int getfriendstr() {
	int i;
	struct override *tmp;
	memset(uinfo.friend, 0, sizeof(uinfo.friend));
	setuserfile(genbuf, "friends");
	uinfo.fnum = get_num_records(genbuf, sizeof(struct override));
	if (uinfo.fnum <= 0)
		return 0;
	uinfo.fnum = (uinfo.fnum >= MAXFRIENDS) ? MAXFRIENDS : uinfo.fnum;
	tmp = (struct override *) calloc(sizeof(struct override), uinfo.fnum);
	get_records(genbuf, tmp, sizeof(struct override), 1, uinfo.fnum);
	for (i = 0; i < uinfo.fnum; i++) {
		uinfo.friend[i] = searchuser(tmp[i].id);
	}
	free(tmp);
	qsort(&uinfo.friend, uinfo.fnum, sizeof(uinfo.friend[0]), cmpfuid);
	update_ulist(&uinfo, utmpent);
	return 0;
}
