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
int ulistpage;

struct talk_win {
	int curcol, curln;
	int sline, eline;
};

int nowmovie;

extern int t_columns;

extern int t_cmpuids();
int cmpfnames();
char *ModeType();

char pagerchar(int friend, int pager) {
	if (pager & ALL_PAGER)
		return ' ';
	if ((friend)
	   ) {
		if (pager & FRIEND_PAGER) return 'O';
		else return '#';
	}
	return '*';
}

int canpage(int friend, int pager) {
	if ((pager & ALL_PAGER) || HAS_PERM(PERM_OCHAT))
		return YEA;
	if ((pager & FRIEND_PAGER)) {
		if (friend) return YEA;
	}
	return NA;
}

int listcuent(struct user_info *uentp) {
	if (uentp == NULL) {
		CreateNameList();
		return 0;
	}
	if (uentp->uid == usernum)
		return 0;
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->invisible && !(HAS_PERM(PERM_SEECLOAK)))
		return 0;
	AddNameList(uentp->userid);
	return 0;
}

void creat_list() {
	listcuent(NULL);
	apply_ulist(listcuent);
}

int t_pager() {

	if (uinfo.pager & ALL_PAGER) {
		uinfo.pager &= ~ALL_PAGER;
		if (DEFINE(DEF_FRIENDCALL))
			uinfo.pager |= FRIEND_PAGER;
		else
			uinfo.pager &= ~FRIEND_PAGER;
	} else {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}

	if (!uinfo.in_chat && session.status != ST_TALK) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("ÄúµÄºô½ÐÆ÷ (pager) ÒÑ¾­[1m%s[mÁË!",
				(uinfo.pager & ALL_PAGER) ? "´ò¿ª" : "¹Ø±Õ");
		pressreturn();
	}
	update_ulist(&uinfo, utmpent);
	return 0;
}

/*Add by SmallPig*/
/*´Ëº¯ÊýÖ»¸ºÔðÁÐÓ¡ËµÃ÷µµ£¬²¢²»¹ÜÇå³ý»ò¶¨Î»µÄÎÊÌâ¡£*/

int
show_user_plan(const char *userid)
{
	char pfile[STRLEN];
	sethomefile(pfile, userid, "plans");
	if (show_one_file(pfile)==NA) {
		prints("\033[1;36mÃ»ÓÐ¸öÈËËµÃ÷µµ\033[m\n");
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

int t_search_ulist(struct user_info *uentp, int (*fptr) (), int farg, int show, int doTalk)
{
	int i, num, mode, idle;
	const char *col;

	resolve_utmp();
	num = 0;
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
		if ((*fptr)(farg, uentp)) {
			if (!uentp->active || !uentp->pid) {
				continue;
			}
			if ( (uentp->invisible==0) ||(uentp->uid == usernum)
					|| (uentp->invisible && HAS_PERM(PERM_SEECLOAK))) {
				num++;
			} else {
				continue;
			}
			if (!show)
				continue;
			if (num == 1)
				prints("Ä¿Ç° %s ×´Ì¬ÈçÏÂ£º\n", uentp->userid);
			mode = get_raw_mode(uentp->mode);
			if (uentp->invisible)
				col = "\033[1;30m";
			else if (mode == ST_POSTING || mode == ST_MARKET)
				col = "\033[1;32m";
			else if (mode == ST_FIVE || mode == ST_BBSNET)
				col = "\033[1;33m";
			else if (is_web_user(uentp->mode))
				col = "\033[1;36m";
			else
				col = "\033[1m";
			const char *host;
			if (HAS_PERM2(PERM_OCHAT, &currentuser)) {
				host = uentp->from;
			} else {
				if (is_hide_ip(uentp))
					host = "......";
				else
					host = mask_host(uentp->from);
			}
			if (doTalk) {
				prints("(%d) ×´Ì¬£º%s%-10s\033[m£¬À´×Ô£º%.20s\n", num, col,
						mode_type(uentp->mode), host);
			} else {
				prints("%s%s\033[m", col, mode_type(uentp->mode));
				idle = (time(NULL) - uentp->idle_time) / 60;
				if (idle >= 1 && mode != ST_BBSNET)
					prints("[%d] ", idle);
				else
					prints("    ");
				if ((num) % 5 == 0)
					outc('\n');
			}
		}
	}
	if (show)
		outc('\n');
	return num;
}

enum {
	IPADDR_OMIT_THRES = 36,
};

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
	prints("\033[0;1;37m%s \033[m(\033[1;33m%s\033[m) ¹²ÉÏÕ¾ \033[1;32m%d\033[m "
			"´Î  %s\n", user.userid, user.username, user.numlogins, horo);

	bool self = !strcmp(currentuser.userid, user.userid);
	const char *host;
	if (user.lasthost[0] == '\0') {
		host = "(²»Ïê)";
	} else {
		if (self || HAS_PERM2(PERM_OCHAT, &currentuser))
			host = user.lasthost;
		else 
			host = mask_host(user.lasthost);
	}
	prints("½øÕ¾ [\033[1;32m%s\033[m] %s[\033[1;32m%s\033[m]\n",
			getdatestring(user.lastlogin, DATE_ZH),
			strlen(host) > IPADDR_OMIT_THRES ? "" : "À´×Ô ", host);

	struct user_info uin;
	int num = t_search_ulist(&uin, t_cmpuids, unum, NA, NA);
	if (num) {
		search_ulist(&uin, t_cmpuids, unum);
		prints("ÔÚÏß [\033[1;32mÑ¶Ï¢Æ÷:(\033[36m%s\033[32m) "
				"ºô½ÐÆ÷:(\033[36m%s\033[32m)\033[m] ",
				canmsg(&uin) ? "´ò¿ª" : "¹Ø±Õ",
				canpage(hisfriend(&uin), uin.pager) ? "´ò¿ª" : "¹Ø±Õ");
	} else {
		fb_time_t t = user.lastlogout;
		if (user.lastlogout < user.lastlogin)
			t = ((time(NULL) - user.lastlogin) / 120) % 47 + 1 + user.lastlogin;
		prints("ÀëÕ¾ [\033[1;32m%s\033[m] ", getdatestring(t, DATE_ZH));
	}

	char path[HOMELEN];
	snprintf(path, sizeof(path), "mail/%c/%s/%s",
			toupper(user.userid[0]), user.userid, DOT_DIR);
	int perf = countperf(&user);
	prints("±íÏÖÖµ "
#ifdef SHOW_PERF
			"%d(\033[1;33m%s\033[m)"
#else
			"[\033[1;33m%s\033[m]"
#endif
			" ÐÅÏä [\033[1;5;32m%2s\033[m]\n"
#ifdef SHOW_PERF
			, perf
#endif
			, cperf(perf), (check_query_mail(path) == 1) ? "ÐÅ" : "  ");

	int exp = countexp(&user);

	uinfo_t u;
	uinfo_load(user.userid, &u);

#ifdef ENABLE_BANK
	char rank_buf[8];
	snprintf(rank_buf, sizeof(rank_buf), "%.1f%%", PERCENT_RANK(u.rank));
	prints("¹±Ï× [\033[1;32m%d\033[m](%s) ", TO_YUAN_INT(u.contrib), rank_buf);
	if (self || HAS_PERM2(PERM_OCHAT, &currentuser)) {
		prints("²Æ¸» [\033[1;32m%d\033[m] ", TO_YUAN_INT(u.money));
	}
#endif

#ifdef ALLOWGAME
	prints("´æ´û¿î [\033[1;32m%d\033[m/\033[1;32m%d\033[m]"
			"(\033[1;33m%s\033[m) ¾­ÑéÖµ [\033[1;32m%d\033[m]\n",
			user.money, user.bet, cmoney(user.money - user.bet), exp);
	prints("·¢ÎÄ [\033[1;32m%d\033[m] ½±ÕÂ [\033[1;32m%d\033[m]"
			"(\033[1;33m%s\033[m) ÉúÃüÁ¦ [\033[1;32m%d\033[m]\n",
			user.numposts, user.nummedals, cnummedals(user.nummedals),
			compute_user_value(&user));
#else
	prints("·¢ÎÄ [\033[1;32m%d\033[m] ", user.numposts);
	prints("¾­ÑéÖµ [\033[1;33m%-10s\033[m]", cexpstr(exp));
#ifdef SHOWEXP
	prints("(%d)", exp);
#endif
	prints(" ÉúÃüÁ¦ [\033[1;32m%d\033[m]\n", compute_user_value(&user));
#endif

	char buf[160];
	show_position(&user, buf, sizeof(buf), u.title);
	prints("Éí·Ý %s\n", buf);
	
	uinfo_free(&u);

	t_search_ulist(&uin, t_cmpuids, unum, YEA, NA);
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
			prints("²éÑ¯Ë­:\n<ÊäÈëÊ¹ÓÃÕß´úºÅ, °´¿Õ°×¼ü¿ÉÁÐ³ö·ûºÏ×Ö´®>\n");
			move(1, 8);
			usercomplete(NULL, userid);
			if (*userid == '\0')
				return FULLUPDATE;
			break;
	}

	if (tui_query_result(userid) != 0) {
		move(2, 0);
		clrtoeol();
		prints("\033[1m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ\033[m\n");
		pressanykey();
		return FULLUPDATE;
	}

	if (session.status != ST_LUSERS && session.status != ST_LAUSERS
			&& session.status != ST_FRIEND && session.status != ST_GMENU)
	pressanykey();
	return FULLUPDATE;
}

// ¼ÆËãÊ¹ÓÃÍâ²¿³ÌÐòµÄÈËÊý
int count_useshell(struct user_info *uentp) {
	static int count;
	if (uentp == NULL) {
		int c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->mode == ST_SYSINFO	|| uentp->mode == ST_DICT
		|| uentp->mode == ST_BBSNET || uentp->mode == ST_FIVE
		|| uentp->mode == ST_LOGIN)
		count++;
	return 1;
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

//	·µ»ØÊ¹ÓÃÍâ²¿³ÌÐòµÄÈËÊý
int num_useshell() {
	count_useshell(NULL);
	apply_ulist(count_useshell);
	return count_useshell(NULL);
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
	if (uinfo.in_chat == YEA)
		return;
	getyx(&x, &y);
	update_endline();
	signal(SIGALRM, endmsg);
	move(x, y);
	refresh();
	alarm(60);
	showansi = tmpansi;
	return;
}

int
shortulist(uentp)
struct user_info *uentp;
{
	int i;
	int pageusers = 60;
	extern struct user_info *user_record[];
	extern int range;
	fill_userlist(-1);
	if (ulistpage> ((range - 1) / pageusers))
	ulistpage = 0;
	if (ulistpage < 0)
	ulistpage = (range - 1) / pageusers;
	move(1, 0);
	clrtoeol();
	prints("Ã¿¸ô [1;32m%d[m Ãë¸üÐÂÒ»´Î£¬[1;32mCtrl-C[m »ò [1;32mCtrl-D[m Àë¿ª£¬[1;32mF[m ¸ü»»Ä£Ê½ [1;32m¡ü¡ý[m ÉÏ¡¢ÏÂÒ»Ò³ µÚ[1;32m %1d[m Ò³", M_INT, ulistpage + 1);
	clrtoeol();
	move(3, 0);
	clrtobot();
	for (i = ulistpage * pageusers; i < (ulistpage + 1) * pageusers && i < range; i++) {
		char ubuf[STRLEN];
		int ovv;
		if (i < numf || friendmode)
		ovv = YEA;
		else
		ovv = NA;
		sprintf(ubuf, "%s%-12.12s %s%-10.10s[m", (ovv) ? "[1;32m¡Ì" : "  ", user_record[i]->userid, (user_record[i]->invisible == YEA) ? "[1;34m" : "",mode_type(user_record[i]->mode));
		//modestring(user_record[i]->mode, user_record[i]->destuid, 0, NULL));
		prints("%s", ubuf);
		if ((i + 1) % 3 == 0)
		outc('\n');
		else
		prints(" |");
	}
	return range;
}

int
do_list(modestr)
char *modestr;
{
	char buf[STRLEN];
	int count;
	extern int RMSG;
	if (RMSG != YEA) { /* Èç¹ûÊÕµ½ Msg µÚÒ»ÐÐ²»ÏÔÊ¾¡£ */
		move(0, 0);
		clrtoeol();
		if (chkmail())
		showtitle(modestr, "[ÄúÓÐÐÅ¼þ]");
		else
		showtitle(modestr, BoardName);
	}
	move(2, 0);
	clrtoeol();
	sprintf(buf, "  %-12s %-10s", "Ê¹ÓÃÕß´úºÅ", "Ä¿Ç°¶¯Ì¬");
	prints("[1;33;44m%s |%s |%s[m", buf, buf, buf);
	count = shortulist();
	if (session.status == ST_MONITOR) {
		move(t_lines - 1, 0);
		sprintf(genbuf,"[1;44;33m  Ä¿Ç°ÓÐ [32m%3d[33m %6sÉÏÏß, Ê±¼ä: [32m%22.22s [33m, Ä¿Ç°×´Ì¬£º[36m%10s   [m"
				,count, friendmode ? "ºÃÅóÓÑ" : "Ê¹ÓÃÕß", getdatestring(time(NULL), DATE_ZH), friendmode ? "ÄãµÄºÃÅóÓÑ" : "ËùÓÐÊ¹ÓÃÕß");
		outs(genbuf);
	}
	refresh();
	return 0;
}

int t_list() {
	set_user_status(ST_LUSERS);
	report("t_list", currentuser.userid);
	do_list("Ê¹ÓÃÕß×´Ì¬");
	pressreturn();
	refresh();
	clear();
	return 0;
}

void sig_catcher() {
	ulistpage++;
	if (session.status != ST_MONITOR) {
#ifdef DOTIMEOUT
#else
		signal(SIGALRM, SIG_IGN);
#endif
		return;
	}
	if (signal(SIGALRM, sig_catcher) == SIG_ERR)
		exit(1);
	do_list("Ì½ÊÓÃñÇé");
	alarm(M_INT);
}

int t_monitor() {
	int i;
	char modestr[] = "Ì½ÊÓÃñÇé";
	alarm(0);
	signal(SIGALRM, sig_catcher);
	/*    idle_monitor_time = 0; */
	report("monitor", currentuser.userid);
	set_user_status(ST_MONITOR);
	ulistpage = 0;
	do_list(modestr);
	alarm(M_INT);
	while (YEA) {
		i = egetch();
		if (i == 'f' || i == 'F') {
			if (friendmode == YEA)
				friendmode = NA;
			else
				friendmode = YEA;
			do_list(modestr);
		}
		if (i == KEY_DOWN) {
			ulistpage++;
			do_list(modestr);
		}
		if (i == KEY_UP) {
			ulistpage--;
			do_list(modestr);
		}
		if (i == Ctrl('D') || i == Ctrl('C') || i == KEY_LEFT)
			break;
		/*        else if (i == -1) {
		 if (errno != EINTR) exit(1);
		 } else idle_monitor_time = 0;*/
	}
	move(2, 0);
	clrtoeol();
	clear();
	return 0;
}

int listfilecontent(char *fname, int y) {
	FILE *fp;
	int x = 0, cnt = 0, max = 0, len;
	//char    u_buf[20], line[STRLEN], *nick;
	char u_buf[20], line[512], *nick;
	//modified by roly 02.03.22 »º´æÇøÒç³ö
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
