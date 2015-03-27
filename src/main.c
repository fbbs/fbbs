// Handle user login.

#include <signal.h>
#include "bbs.h"

#include "fbbs/dbi.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/msg.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/ucache.h"
#include "fbbs/uinfo.h"
#include "fbbs/user.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif

#define VISITLOG    BBSHOME"/.visitlog"

int RMSG = false;
int msg_num = 0;
int mailXX = 0; // If mail quota is exceeded.
int numofsig = 0;
jmp_buf byebye;
time_t lastnote;
char fromhost[IP_LEN];
char BoardName[STRLEN]; // TODO: Can be replaced by macro.

int utmpent = -1;
time_t login_start_time;
int showansi = 1;

int refscreen = NA;
void msg_handler(int signum);

// Handle giveupBBS(戒网) transactions.
// Return expiration date (in days from epoch).
// TODO: Better rewrite it..
int chk_giveupbbs(void)
{
	int i, j, tmpcount, tmpid, sflag[10][2];
	FILE *fn;
	int lcount = 0;
	char buf[NAME_MAX];
	int recover = 0;

	sethomefile(buf, currentuser.userid, "giveupBBS");
	fn = fopen(buf, "r");
	if (fn) {
		struct userec tmpuserec;
		memcpy(tmpuserec.userid, currentuser.userid, sizeof(tmpuserec.userid));
		tmpid = getuserec(tmpuserec.userid, &tmpuserec);
		while (!feof(fn)) {
			if (fscanf(fn, "%d %d", &i, &j) <= 0)
				break;
			sflag[lcount][0] = i;
			sflag[lcount][1] = j;
			lcount++;
		}
		tmpcount = lcount;
		fclose(fn);
		for (i = 0; i < lcount; i++) {
			if(sflag[i][0] == 1)
				recover = sflag[i][1];
			if (sflag[i][1] <= time(NULL) / 3600 / 24) {
				tmpcount--;
				switch (sflag[i][0]) {
					case 1:
						tmpuserec.userlevel |= PERM_LOGIN;
						recover = 0;
						break;
					case 2:
						tmpuserec.userlevel |= PERM_POST;
						break;
					case 3:
						tmpuserec.userlevel |= PERM_TALK;
						break;
					case 4:
						tmpuserec.userlevel |= PERM_MAIL;
						break;
				}
				sflag[i][1] = 0;
			}
			
		}
		if (tmpuserec.flags[0] & GIVEUPBBS_FLAG && tmpcount == 0)
			tmpuserec.flags[0] &= ~GIVEUPBBS_FLAG;
		substitut_record(PASSFILE, &tmpuserec, sizeof(struct userec),
				tmpid);
		if (tmpcount == 0)
			unlink(buf);
		else {
			fn = fopen(buf, "w");
			for (i = 0; i < lcount; i++)
				if (sflag[i][1] > 0)
					fprintf(fn, "%d %d\n", sflag[i][0], sflag[i][1]);
			fclose(fn);
		}
	}
	return recover;
}

static int load_pager(void)
{
	int pager = 0;

	if (DEFINE(DEF_FRIENDCALL)) {
		pager |= FRIEND_PAGER;
	}
	if (currentuser.flags[0] & PAGER_FLAG) {
		pager |= ALL_PAGER;
		pager |= FRIEND_PAGER;
	}
	if (DEFINE(DEF_FRIENDMSG)) {
		pager |= FRIENDMSG_PAGER;
	}
	if (DEFINE(DEF_ALLMSG)) {
		pager |= ALLMSG_PAGER;
		pager |= FRIENDMSG_PAGER;
	}
	if (DEFINE(DEF_LOGOFFMSG)) {
		pager |= LOGOFFMSG_PAGER;
	}

	return pager;
}

static void set_pager(int pager)
{
	// TODO: this should be in db
	return;
}

static void u_enter(void)
{
	if (!HAS_PERM(PERM_CLOAK))
		currentuser.flags[0] &= ~CLOAK_FLAG;
	session_set_visibility(!(HAS_PERM(PERM_LOGINCLOAK)
				&& (currentuser.flags[0] & CLOAK_FLAG)));

	chk_giveupbbs();

	digestmode = NA;

	session_new(NULL, NULL, 0, session_uid(), currentuser.userid, fromhost,
			SESSION_TELNET,
#ifdef ENABLE_SSH
			SESSION_SECURE
#else
			SESSION_PLAIN
#endif
			, session_visible(), 0);

	int pager = load_pager();
	set_pager(pager);
}

// Set 'mask'ed bit in 'currentuser.flags[0]'  according to 'value'.
static void setflags(int mask, int value)
{
	if (((currentuser.flags[0] & mask) && 1) != value) {
		if (value)
			currentuser.flags[0] |= mask;
		else
			currentuser.flags[0] &= ~mask;
	}
}

// Save user info on exit.
void u_exit(void)
{
	// 这些信号的处理要关掉, 否则在离线时等候回车时出现
	// 信号会导致重写名单, 这个导致的名单混乱比kick user更多  (ylsdd)
	fb_signal(SIGALRM, SIG_DFL);
	fb_signal(SIGPIPE, SIG_DFL);
	fb_signal(SIGTERM, SIG_DFL);
	fb_signal(SIGUSR1, SIG_IGN);
	fb_signal(SIGUSR2, SIG_IGN);

	if (HAS_PERM(PERM_LOGINCLOAK))
		setflags(CLOAK_FLAG, !session_visible());

	set_safe_record();
	update_user_stay(&currentuser, false, false);
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	uidshm->status[usernum - 1]--;

	session_destroy(session_id());
	session_set_pid(0);
}

void abort_bbs(int nothing)
{
	extern int child_pid;

	if (child_pid) {
		kill(child_pid, SIGKILL);
	}

	editor_dump();

	if (session_id()) {
		time_t stay;
		stay = time(0) - login_start_time;
		snprintf(genbuf, sizeof(genbuf), "Stay: %3ld", stay / 60);
		log_usies("AXXED", genbuf, &currentuser);

		u_exit();
	}

	exit(0);
}

// Prevent too many logins of same account.
static int multi_user_check(void)
{
	int max = get_login_quota(&currentuser);
	if (max == INT_MAX)
		return 0;

	int logins = INT_MAX;
	session_basic_info_t *res = get_my_sessions();
	if (res) {
		logins = session_basic_info_count(res);
	}

	if (strcaseeq("guest", currentuser.userid) && logins >= max) {
		//% prints("\033[1;33m抱歉, 目前已有太多 \033[1;36mguest\033[33m, "
		prints("\033[1;33m\xb1\xa7\xc7\xb8, \xc4\xbf\xc7\xb0\xd2\xd1\xd3\xd0\xcc\xab\xb6\xe0 \033[1;36mguest\033[33m, "
				//% "请稍后再试。\033[m\n");
				"\xc7\xeb\xc9\xd4\xba\xf3\xd4\xd9\xca\xd4\xa1\xa3\033[m\n");
		session_basic_info_clear(res);
		return -1;
	}

	if (logins >= max) {
		//% prints("\033[1;32m为确保他人上站权益, "
		prints("\033[1;32m\xce\xaa\xc8\xb7\xb1\xa3\xcb\xfb\xc8\xcb\xc9\xcf\xd5\xbe\xc8\xa8\xd2\xe6, "
				//% "本站仅允许您用该帐号登录 %d 个。\n\033[m"
				"\xb1\xbe\xd5\xbe\xbd\xf6\xd4\xca\xd0\xed\xc4\xfa\xd3\xc3\xb8\xc3\xd5\xca\xba\xc5\xb5\xc7\xc2\xbc %d \xb8\xf6\xa1\xa3\n\033[m"
				//% "\033[1;36m您目前已经使用该帐号登录了 %d 个，"
				"\033[1;36m\xc4\xfa\xc4\xbf\xc7\xb0\xd2\xd1\xbe\xad\xca\xb9\xd3\xc3\xb8\xc3\xd5\xca\xba\xc5\xb5\xc7\xc2\xbc\xc1\xcb %d \xb8\xf6\xa3\xac"
				//% "您必须断开其他的连接方能进入本站！\n\033[m", max, logins);
				"\xc4\xfa\xb1\xd8\xd0\xeb\xb6\xcf\xbf\xaa\xc6\xe4\xcb\xfb\xb5\xc4\xc1\xac\xbd\xd3\xb7\xbd\xc4\xdc\xbd\xf8\xc8\xeb\xb1\xbe\xd5\xbe\xa3\xa1\n\033[m", max, logins);
		//% bool kick = askyn("您想删除重复的连接吗", false, false);
		bool kick = askyn("\xc4\xfa\xcf\xeb\xc9\xbe\xb3\xfd\xd6\xd8\xb8\xb4\xb5\xc4\xc1\xac\xbd\xd3\xc2\xf0", false, false);
		if (kick) {
			bbs_kill(session_basic_info_sid(res, 0),
					session_basic_info_pid(res, 0), SIGHUP);
			report("kicked (multi-login)", currentuser.userid);
			session_basic_info_clear(res);

			sleep(2);
			res = get_my_sessions();
			logins = session_basic_info_count(res);
		}

		session_basic_info_clear(res);
		if (logins >= max) {
			//% prints("\033[33m很抱歉，您已经用该帐号登录 %d 个，"
			prints("\033[33m\xba\xdc\xb1\xa7\xc7\xb8\xa3\xac\xc4\xfa\xd2\xd1\xbe\xad\xd3\xc3\xb8\xc3\xd5\xca\xba\xc5\xb5\xc7\xc2\xbc %d \xb8\xf6\xa3\xac"
					//% "所以，此连线将被取消。\033[m\n", logins);
					"\xcb\xf9\xd2\xd4\xa3\xac\xb4\xcb\xc1\xac\xcf\xdf\xbd\xab\xb1\xbb\xc8\xa1\xcf\xfb\xa1\xa3\033[m\n", logins);
			return -1;
		}
	}
	return 0;
}

// Register some signal handlers.
static void system_init(void)
{
	struct sigaction act;

	fb_signal(SIGHUP, terminal_schedule_exit);
#ifndef lint
	fb_signal(SIGINT, SIG_IGN);
	fb_signal(SIGQUIT, SIG_IGN);
	fb_signal(SIGPIPE, SIG_IGN);
#ifdef DOTIMEOUT
	set_user_status(ST_LOGIN);
	alarm(LOGIN_TIMEOUT);
#else
	fb_signal(SIGALRM, SIG_SIG);
#endif
	fb_signal(SIGTERM, SIG_IGN);
	fb_signal(SIGURG, SIG_IGN);
	fb_signal(SIGTSTP, SIG_IGN);
	fb_signal(SIGTTIN, SIG_IGN);
#endif

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = msg_handler;
	sigaction(SIGUSR2, &act, NULL);
	{
		struct itimerval itv;
		memset(&itv,0, sizeof(struct itimerval));
		itv.it_value.tv_sec = 2 * 60;
		setitimer(ITIMER_PROF, &itv, NULL);
		fb_signal(SIGPROF, exit);
	}
}


static void system_abort(void)
{
	if (session_id()) {
		log_usies("ABORT", "", &currentuser);
		u_exit();
	}
	screen_clear();
	screen_flush();
	//% prints("谢谢光临, 记得常来喔 !\n");
	prints("\xd0\xbb\xd0\xbb\xb9\xe2\xc1\xd9, \xbc\xc7\xb5\xc3\xb3\xa3\xc0\xb4\xe0\xb8 !\n");
	exit(0);
}

struct max_log_record {
	int year;
	int month;
	int day;
	int logins;
	unsigned long visit;
};

// Show visit count and save it.
static void visitlog(int peak)
{
	time_t now;
	struct tm *tm;
	struct max_log_record max_log = { 0 };

	FILE *fp;
	fp = fopen(VISITLOG, "r+b");
	if (fp) {
		if (!fread(&max_log, sizeof(max_log), 1, fp)
			|| (max_log.year < 1990 || max_log.year> 2020)) {
			now = time(NULL);
			tm = localtime(&now);
			max_log.year = tm->tm_year + 1900;
			max_log.month = tm->tm_mon + 1;
			max_log.day = tm->tm_mday;
			max_log.visit = 0;
			max_log.logins = 0;
		}
		else {
			max_log.visit++;
			if (peak > max_log.logins)
				max_log.logins = peak;
		}
		fseek(fp, 0, SEEK_SET);
		fwrite(&max_log, sizeof(max_log), 1, fp);
		fclose(fp);
	}
	snprintf(genbuf, sizeof(genbuf), 
		//% "\033[1;32m从 [\033[36m%4d年%2d月%2d日\033[32m] 起, "
		"\033[1;32m\xb4\xd3 [\033[36m%4d\xc4\xea%2d\xd4\xc2%2d\xc8\xd5\033[32m] \xc6\xf0, "
		//% "最高人数记录: [\033[36m%d\033[32m] "
		"\xd7\xee\xb8\xdf\xc8\xcb\xca\xfd\xbc\xc7\xc2\xbc: [\033[36m%d\033[32m] "
		//% "累计访问人次: [\033[36m%lu\033[32m]\033[m\n",
		"\xc0\xdb\xbc\xc6\xb7\xc3\xce\xca\xc8\xcb\xb4\xce: [\033[36m%lu\033[32m]\033[m\n",
		max_log.year, max_log.month, max_log.day, max_log.logins,
		max_log.visit);
	prints("%s", genbuf);
}

enum {
	BBS_EGIVEUP = 1,
	BBS_ESUICIDE = 2,
	BBS_EBANNED = 3,
};

/**
 *
 */
int bbs_auth(const char *name, const char *passwd)
{
	if (!name || *name == '\0')
		return BBS_ENOUSR;

	if (currentuser.userid[0] == '\0') {
		if (session_count_online() > MAXACTIVE)
			return BBS_E2MANY;
		if (!dosearchuser(name, &currentuser, &usernum))
			return BBS_ENOUSR;
	}

	if (!passwd_check(currentuser.userid, passwd)) {
		log_attempt(currentuser.userid, fromhost, "telnet");
		return BBS_EWPSWD;
	}
	if (strcasecmp(currentuser.userid, "guest") && !HAS_PERM(PERM_LOGIN)) {
		if (chk_giveupbbs())
			return BBS_EGIVEUP;
		if (currentuser.userlevel == 0) {
			return BBS_ESUICIDE;
		} else {
			return BBS_EBANNED;
		}
	}
#ifdef CHECK_FREQUENTLOGIN
	if (!HAS_PERM(PERM_SYSOPS)
			&& strcasecmp(currentuser.userid, "guest") != 0
			&& abs(time(NULL) - currentuser.lastlogin) < 10) {
		return BBS_ELFREQ;
	}
#endif

	session_set_uid(get_user_id(name));

	return 0;
}

static int login_query(void)
{
#ifndef ENABLE_SSH
	char uname[IDLEN + 2];
	char passbuf[PASSLEN];
	int attempts;
	int recover; // For giveupBBS
	bool auth = false;
#endif // ENABLE_SSH

	// Deny new logins if too many users online.
	int online = session_count_online();
#ifndef ENABLE_SSH
	if (online >= MAXACTIVE) {
		ansimore("etc/loginfull", NA);
		return -1;
	}
#endif // ENABLE_SSH

	ansimore2("etc/issue", false, 0, 0);
	screen_printf("\033[1;35m欢迎光临\033[1;40;33m【 %s 】 \033[m"
			"[\033[1;33;41m Add '.' after YourID to login for BIG5 \033[m]\n",
			BBSNAME_UTF8);

	int peak = session_get_online_record();
	if (peak < online) {
		session_set_online_record(online);
		peak = online;
	}

	screen_printf("\033[1;32m目前已有帐号: [\033[1;36m%d\033[32m/\033[36m%d\033[32m] "
			"\033[32m目前站上人数: [\033[36m%d\033[32m/\033[36m%d\033[1;32m]\n",
			get_user_count(), MAXUSERS, online, MAXACTIVE);
	visitlog(peak);

#ifndef ENABLE_SSH
	attempts = 0;
	while (!auth) {
		if (attempts++ >= LOGINATTEMPTS) {
			ansimore("etc/goodbye", NA);
			return -1;
		}
		//% getdata(0, 0, "\033[1;33m请输入帐号\033[m"
		getdata(0, 0, "\033[1;33m\xc7\xeb\xca\xe4\xc8\xeb\xd5\xca\xba\xc5\033[m"
				//% "(试用请输入'\033[1;36mguest\033[m', "
				"(\xca\xd4\xd3\xc3\xc7\xeb\xca\xe4\xc8\xeb'\033[1;36mguest\033[m', "
				//% "注册请输入'\033[1;31mnew\033[m'): ",
				"\xd7\xa2\xb2\xe1\xc7\xeb\xca\xe4\xc8\xeb'\033[1;31mnew\033[m'): ",
				uname, IDLEN + 1, DOECHO, YEA);
		if (strcaseeq(uname, "guest") && (online > MAXACTIVE - 10)) {
			ansimore("etc/loginfull", NA);
			return -1;
		}
		if (strcaseeq(uname, "new")) {
			memset(&currentuser, 0, sizeof(currentuser));
			new_register();
			terminal_flush();
			exit(1);
		} else if (*uname == '\0')
			;
		else if (!dosearchuser(uname, &currentuser, &usernum)) {
			screen_printf("\033[1;31m经查证，无此 ID。\033[m\n");
		} else if (strcaseeq(uname, "guest")) {
			currentuser.userlevel = 0;
			break;
		} else {
			//% getdata(0, 0, "\033[1;37m请输入密码: \033[m", passbuf, PASSLEN,
			getdata(0, 0, "\033[1;37m\xc7\xeb\xca\xe4\xc8\xeb\xc3\xdc\xc2\xeb: \033[m", passbuf, PASSLEN,
					NOECHO, YEA);
			passbuf[8] = '\0';
			switch (bbs_auth(uname, passbuf)) {
				case BBS_EWPSWD:
					screen_printf("\033[1;31m密码输入错误...\033[m\n");
					break;
				case BBS_EGIVEUP:
					recover = chk_giveupbbs();
					screen_printf("\033[33m您正在戒网，离戒网结束还有%d天\033[m\n",
							recover - fb_time() / 3600 / 24);
					return -1;
				case BBS_ESUICIDE:
					screen_printf("\033[32m您已经自杀\033[m\n");
					return -1;
				case BBS_EBANNED:
					screen_printf("\033[32m本帐号已停机。请到 "
							"\033[36mNotice\033[32m版 查询原因\033[m\n");
					return -1;
				case BBS_ELFREQ:
					screen_printf("登录过于频繁，请稍候再来\n");
					return -1;
				case 0:
					auth = true;
					break;
				default:
					auth = false;
					break;
			}
			memset(passbuf, 0, PASSLEN - 1);
		}
	}
#else // ENABLE_SSH
	//% 欢迎使用ssh方式访问本站，请按任意键继续
	presskeyfor("\033[1;33m\xbb\xb6\xd3\xad\xca\xb9\xd3\xc3ssh\xb7\xbd\xca\xbd\xb7\xc3\xce\xca\xb1\xbe\xd5\xbe\xa3\xac\xc7\xeb\xb0\xb4\xc8\xce\xd2\xe2\xbc\xfc\xbc\xcc\xd0\xf8", -1);
#endif // ENABLE_SSH

	if (multi_user_check() == -1)
		return -1;

	sethomepath(genbuf, currentuser.userid);
	mkdir(genbuf, 0755);
	login_start_time = time(NULL);
	return 0;
}

static void write_defnotepad(void)
{
	currentuser.notedate = time(NULL);
	set_safe_record();
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	return;
}

// Do autoposting according to "etc/autopost",
// if no such action has been taken since last 00:00 UTC.
static void notepad_init(void)
{
	FILE *check;
	char notetitle[STRLEN];
	char tmp[STRLEN * 2];
	char *fname, *bname, *ntitle;
	long int maxsec;

	maxsec = 86400;
	lastnote = 0;
	if ((check = fopen("etc/checknotepad", "r")) != NULL) {
		fgets(tmp, sizeof(tmp), check);
		lastnote = atol(tmp);
		fclose(check);
	}
	fb_time_t now = fb_time();
	if ((now - lastnote) >= maxsec) {
		move(-1, 0);
		//% prints("对不起，系统自动发信，请稍候.....");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xcf\xb5\xcd\xb3\xd7\xd4\xb6\xaf\xb7\xa2\xd0\xc5\xa3\xac\xc7\xeb\xc9\xd4\xba\xf2.....");
		screen_flush();
		check = fopen("etc/checknotepad", "w");
		lastnote = now - (now % maxsec);
		fprintf(check, "%ld", lastnote);
		fclose(check);
		if ((check = fopen("etc/autopost", "r")) != NULL) {
			while (fgets(tmp, STRLEN, check) != NULL) {
				fname = strtok(tmp, " \n\t:@");
				bname = strtok(NULL, " \n\t:@");
				ntitle = strtok(NULL, " \n\t:@");
				if (fname == NULL || bname == NULL || ntitle == NULL)
					continue;
				else {
					char *str = format_time(now, TIME_FORMAT_ZH);
					snprintf(notetitle, sizeof(notetitle), "[%14.14s %6.6s] %s",
							str, str + 23, ntitle);
					if (dashf(fname)) {
						Postfile(fname, bname, notetitle, 1);
						//% sprintf(tmp, "%s 自动张贴", ntitle);
						sprintf(tmp, "%s \xd7\xd4\xb6\xaf\xd5\xc5\xcc\xf9", ntitle);
						report(tmp, currentuser.userid);
					}
				}
			}
			fclose(check);
		}
		char *str = format_time(now, TIME_FORMAT_ZH);
		//% snprintf(notetitle, sizeof(notetitle), "[%14.14s %6.6s] 留言板记录",
		snprintf(notetitle, sizeof(notetitle), "[%14.14s %6.6s] \xc1\xf4\xd1\xd4\xb0\xe5\xbc\xc7\xc2\xbc",
				str, str + 23);
		if (dashf("etc/notepad")) {
			Postfile("etc/notepad", "Notepad", notetitle, 1);
			unlink("etc/notepad");
		}
		//% report("自动发信时间更改", currentuser.userid);
		report("\xd7\xd4\xb6\xaf\xb7\xa2\xd0\xc5\xca\xb1\xbc\xe4\xb8\xfc\xb8\xc4", currentuser.userid);
	}
	return;
}

static void user_login(void)
{
	char fname[STRLEN];

	// SYSOP gets all permission bits when login.
	if (strcmp(currentuser.userid, "SYSOP") == 0) {
		currentuser.userlevel = ~0;
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser),
				usernum);
	}
	fromhost[sizeof(fromhost) - 1] = 0; //added by iamfat 2004.01.05 to avoid overflow
	log_usies("ENTER", fromhost, &currentuser);

	SpecialID(currentuser.userid, fromhost, sizeof(fromhost));

	u_enter();
	report("Enter", currentuser.userid);

#ifdef USE_NOTEPAD
	notepad_init();
	if (strcmp(currentuser.userid, "guest") != 0) {
		if (DEFINE(DEF_NOTEPAD)) {
			int noteln;

			if (lastnote> currentuser.notedate)
			currentuser.noteline = 0;
			noteln = countln("etc/notepad");
			if (currentuser.noteline == 0) {
				shownotepad();
			} else if ((noteln - currentuser.noteline)> 0) {
				move(0, 0);
				ansimore2("etc/notepad", NA, 0, noteln - currentuser.noteline + 1);
				terminal_getchar();
				screen_clear();
			}
			currentuser.noteline = noteln;
			write_defnotepad();
		}
	}
#endif

	if (show_statshm("etc/hotspot", 0)) {
		screen_flush();
		pressanykey();
	}

	if ((vote_flag(NULL, '\0', 2 /* 检查读过新的Welcome 没 */) == 0)) {
		if (dashf("Welcome")) {
			ansimore("Welcome", YEA);
			vote_flag(NULL, 'R', 2 /* 写入读过新的Welcome */);
		}
	} else {
		ansimore("Welcome2", YEA);
	}
	show_statshm("0Announce/bbslist/day", 1);
	screen_flush();
	screen_move_clear(-2);
	if (currentuser.numlogins < 1) {
		currentuser.numlogins = 0;
		//% prints("\033[1;36m☆ 这是您第 \033[33m1\033[36m 次拜访本站，请记住今天吧。\n");
		prints("\033[1;36m\xa1\xee \xd5\xe2\xca\xc7\xc4\xfa\xb5\xda \033[33m1\033[36m \xb4\xce\xb0\xdd\xb7\xc3\xb1\xbe\xd5\xbe\xa3\xac\xc7\xeb\xbc\xc7\xd7\xa1\xbd\xf1\xcc\xec\xb0\xc9\xa1\xa3\n");
		//% prints("☆ 您第一次连入本站的时间为 \033[33m%s\033[m ", format_time(time(NULL), TIME_FORMAT_ZH));
		prints("\xa1\xee \xc4\xfa\xb5\xda\xd2\xbb\xb4\xce\xc1\xac\xc8\xeb\xb1\xbe\xd5\xbe\xb5\xc4\xca\xb1\xbc\xe4\xce\xaa \033[33m%s\033[m ", format_time(fb_time(), TIME_FORMAT_ZH));
	} else {
		prints(
				//% "\033[1;36m☆ 这是您第 \033[33m%d\033[36m 次拜访本站，上次您是从 \033[33m%s\033[36m 连往本站。\n",
				"\033[1;36m\xa1\xee \xd5\xe2\xca\xc7\xc4\xfa\xb5\xda \033[33m%d\033[36m \xb4\xce\xb0\xdd\xb7\xc3\xb1\xbe\xd5\xbe\xa3\xac\xc9\xcf\xb4\xce\xc4\xfa\xca\xc7\xb4\xd3 \033[33m%s\033[36m \xc1\xac\xcd\xf9\xb1\xbe\xd5\xbe\xa1\xa3\n",
				currentuser.numlogins + 1, currentuser.lasthost);
		//% prints("☆ 上次连线时间为 \033[33m%s\033[m ", format_time(currentuser.lastlogin, TIME_FORMAT_ZH));
		prints("\xa1\xee \xc9\xcf\xb4\xce\xc1\xac\xcf\xdf\xca\xb1\xbc\xe4\xce\xaa \033[33m%s\033[m ", format_time(currentuser.lastlogin, TIME_FORMAT_ZH));
	}
	terminal_getchar();
	setuserfile(fname, BADLOGINFILE);
	if (ansimore(fname, NA) != -1) {
		//% if (askyn("您要删除以上密码输入错误的记录吗", NA, NA) == YEA)
		if (askyn("\xc4\xfa\xd2\xaa\xc9\xbe\xb3\xfd\xd2\xd4\xc9\xcf\xc3\xdc\xc2\xeb\xca\xe4\xc8\xeb\xb4\xed\xce\xf3\xb5\xc4\xbc\xc7\xc2\xbc\xc2\xf0", NA, NA) == YEA)
			unlink(fname);
	}

	set_safe_record();
	tui_check_uinfo(&currentuser);
	strlcpy(currentuser.lasthost, fromhost, sizeof(currentuser.lasthost));
	if (login_start_time - currentuser.lastlogin >= 20 * 60
			|| !strcmp(currentuser.userid, "guest")
			|| currentuser.numlogins < 100) {
		currentuser.numlogins++;
	}

	session_basic_info_t *res = get_my_sessions();
	update_user_stay(&currentuser, true, session_basic_info_count(res) > 1);
	session_basic_info_clear(res);

#ifdef ALLOWGAME
	if (currentuser.money> 1000000) {
		currentuser.nummedals += currentuser.money / 10000;
		currentuser.money %= 1000000;
	}
	if ((signed int) (currentuser.money - currentuser.bet) < -4990
			&& currentuser.numlogins < 10 && currentuser.numposts < 10
			&& currentuser.nummedals == 0)
	currentuser.money += 1000;
#endif
	if (currentuser.firstlogin == 0) {
		currentuser.firstlogin = time(NULL) - 7 * 86400;
	}
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	extern char currmaildir[];
	setmdir(currmaildir, currentuser.userid);
	check_register_info();
}

// Calculate numbers of signatures.
void set_numofsig(void)
{
	int sigln;
	char signame[NAME_MAX];

	setuserfile(signame, "signatures");
	sigln = countln(signame);
	numofsig = sigln / MAXSIGLINES;
	if ((sigln % MAXSIGLINES) != 0)
		++numofsig;
	return;
}

#ifdef TALK_LOG
// Recover user's talk log from abnormal exit.
void tlog_recover(void)
{
	char buf[256];

	sethomefile(buf, currentuser.userid, "talk_log");
	if (strcasecmp(currentuser.userid, "guest") == 0 || !dashf(buf))
		return;

	screen_clear();
	genbuf[0] = '\0';
	//% getdata(0, 0, "\033[1;32m您有一个不正常断线所留下来的聊天记录, "
	getdata(0, 0, "\033[1;32m\xc4\xfa\xd3\xd0\xd2\xbb\xb8\xf6\xb2\xbb\xd5\xfd\xb3\xa3\xb6\xcf\xcf\xdf\xcb\xf9\xc1\xf4\xcf\xc2\xc0\xb4\xb5\xc4\xc1\xc4\xcc\xec\xbc\xc7\xc2\xbc, "
			//% "您要 .. (M) 寄回信箱 (Q) 算了？[Q]：\033[m",
			"\xc4\xfa\xd2\xaa .. (M) \xbc\xc4\xbb\xd8\xd0\xc5\xcf\xe4 (Q) \xcb\xe3\xc1\xcb\xa3\xbf[Q]\xa3\xba\033[m",
			genbuf, 2, DOECHO, YEA);

	if (genbuf[0] == 'M' || genbuf[0] == 'm') {
		//% mail_file(buf, currentuser.userid, "聊天记录");
		mail_file(buf, currentuser.userid, "\xc1\xc4\xcc\xec\xbc\xc7\xc2\xbc");
	}
	unlink(buf);
	return;
}
#endif

extern void active_board_init(bool);

void start_client(void)
{
	extern char currmaildir[];

	initialize_mdb();
#ifndef ENABLE_SSH
	initialize_db();
#endif

	initialize_convert_env();
	system_init();

	if (setjmp(byebye)) {
		system_abort();
	}

	strlcpy(BoardName, BBSNAME, sizeof(BoardName));

	if (login_query() == -1) {
		terminal_flush();
		sleep(3);
		exit(1);
	}
#ifndef ENABLE_SSH
	screen_negotiate_size();
#endif // ENABLE_SSH
	screen_init(0);

	user_login();

	setmdir(currmaildir, currentuser.userid);
	RMSG = NA;
	screen_clear();
	editor_restore();
#ifdef TALK_LOG
	tlog_recover();
#endif

	if (strcmp(currentuser.userid, "guest")) {
		if (check_maxmail())
			pressanykey();
		move(9, 0);
		screen_clrtobot();
		if (!DEFINE(DEF_NOLOGINSEND))
			if (session_visible())
				login_msg();
		screen_clear();
		set_numofsig();
	}

	active_board_init(false);
	fill_date();

	if (DEFINE(DEF_LOGFRIEND)
			&& session_count_online_followed(!HAS_PERM(PERM_SEECLOAK)) > 0)
		show_online_followings();

	menu_load("menu.img");
	while (1) {
		if (DEFINE(DEF_NORMALSCR))
			menu_loop("TOPMENU");
		else
			menu_loop("TOPMENU2");
		Goodbye();
	}
}
