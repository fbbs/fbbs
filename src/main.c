// Handle user login.

#include "bbs.h"
#include "sysconf.h"

#include "fbbs/dbi.h"
#include "fbbs/fbbs.h"
#include "fbbs/msg.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/user.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif

#define VISITLOG    BBSHOME"/.visitlog"

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif
int RMSG = false;
int msg_num = 0;
int iscolor = 1;
int mailXX = 0; // If mail quota is exceeded.
int numofsig = 0;
jmp_buf byebye;
time_t lastnote;
struct user_info uinfo;
char fromhost[IP_LEN];
char BoardName[STRLEN]; // TODO: Can be replaced by macro.

int utmpent = -1;
time_t login_start_time;
int showansi = 1;
int started = 0;

char GoodWish[20][STRLEN - 3];
int WishNum = 0;
int orderWish = 0;
extern int enabledbchar;
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
	if (HAS_PERM(PERM_LOGINCLOAK) && (currentuser.flags[0] & CLOAK_FLAG))
		session.visible = false;
	session.status = ST_LOGIN;

	chk_giveupbbs();

	if (DEFINE(DEF_DELDBLCHAR))
		enabledbchar = 1;
	else
		enabledbchar = 0;

	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	digestmode = NA;

	session.id = session_new(NULL, 0, session.uid, fromhost, SESSION_TELNET,
#ifdef ENABLE_SSH
			SESSION_SECURE
#else
			SESSION_PLAIN
#endif
			);

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
	signal(SIGHUP, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	if (HAS_PERM(PERM_LOGINCLOAK))
		setflags(CLOAK_FLAG, !session.visible);

	set_safe_record();
	update_user_stay(&currentuser, false, false);
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	uidshm->status[usernum - 1]--;

	session.pid = 0;
}

// Handle abnormal exit.
void abort_bbs(int nothing)
{
	extern int child_pid;

	if (child_pid) {
		kill(child_pid, SIGKILL);
	}

	// Save user's work.
	if (session.status == ST_POSTING || session.status == ST_SMAIL
			|| session.status == ST_EDIT || session.status == ST_EDITUFILE
			|| session.status == ST_EDITSFILE || session.status == ST_EDITANN)
		keep_fail_post();

	if (started) {
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
	basic_session_info_t *res = get_my_sessions();
	if (res) {
		logins = basic_session_info_count(res);
	}

	if (strcaseeq("guest", currentuser.userid) && logins >= max) {
		prints("\033[1;33m抱歉, 目前已有太多 \033[1;36mguest\033[33m, "
				"请稍后再试。\033[m\n");
		basic_session_info_clear(res);
		return -1;
	}

	if (logins >= max) {
		prints("\033[1;32m为确保他人上站权益, "
				"本站仅允许您用该帐号登录 %d 个。\n\033[m"
				"\033[1;36m您目前已经使用该帐号登录了 %d 个，"
				"您必须断开其他的连接方能进入本站！\n\033[m", max, logins);
		bool kick = askyn("您想删除重复的连接吗", false, false);
		if (kick) {
			bbs_kill(basic_session_info_sid(res, 0),
					basic_session_info_pid(res, 0), SIGHUP);
			report("kicked (multi-login)", currentuser.userid);
			basic_session_info_clear(res);

			sleep(2);
			res = get_my_sessions();
			logins = basic_session_info_count(res);
		}

		basic_session_info_clear(res);
		if (logins >= max) {
			prints("\033[33m很抱歉，您已经用该帐号登录 %d 个，"
					"所以，此连线将被取消。\033[m\n", logins);
			return -1;
		}
	}
	return 0;
}

// Register some signal handlers.
static void system_init(void)
{
	struct sigaction act;

#ifndef lint
	signal(SIGHUP, abort_bbs);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
#ifdef DOTIMEOUT
	session.status = ST_LOGIN;
	alarm(LOGIN_TIMEOUT);
#else
	signal(SIGALRM, SIG_SIG);
#endif
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
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
		signal(SIGPROF, exit);
	}
}


static void system_abort(void)
{
	if (started) {
		log_usies("ABORT", "", &currentuser);
		u_exit();
	}
	clear();
	refresh();
	prints("谢谢光临, 记得常来喔 !\n");
	exit(0);
}

#ifndef ENABLE_SSH
// Get height of client window.
// See RFC 1073 "Telnet Window Size Option"
static void check_tty_lines(void)
{
	// An example: Server suggest and client agrees to use NAWS.
	//             (Negotiate About Window Size)
	//    (server sends)  IAC DO  NAWS
	//                    255 253 31
	//    (client sends)  IAC WILL NAWS
	//                    255 251 31
	//	  (client sends)  IAC SB  NAWS 0 80 0 24 IAC SE
	//                    255 250 31   0 80 0 24 255 240
	static unsigned char buf1[] = { 255, 253, 31 };
	unsigned char buf2[100];
	int n;

	if (ttyname(STDIN_FILENO))
		return;
	write_stdout(buf1, 3);
	n = read_stdin(buf2, 80);
	if (n == 12) {
		if (buf2[0] != 255 || buf2[1] != 251 || buf2[2] != 31)
			return;
		if (buf2[3] != 255 || buf2[4] != 250 || buf2[5] != 31 || buf2[10]
				!= 255 || buf2[11] != 240)
			return;
		t_lines = buf2[9];
	}
	if (n == 9) {
		if (buf2[0] != 255 || buf2[1] != 250 || buf2[2] != 31 || buf2[7]
				!= 255 || buf2[8] != 240)
			return;
		t_lines = buf2[6];
	}
	if (t_lines < 24 || t_lines > 100)
		t_lines = 24;
	return;
}
#endif // ENABLE_SSH

struct max_log_record {
	int year;
	int month;
	int day;
	int logins;
	unsigned long visit;
};

// Show visit count and save it.
static void visitlog(void)
{
	time_t now;
	struct tm *tm;
	struct max_log_record max_log;

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
			if (max_log.logins > utmpshm->max_login_num)
				utmpshm->max_login_num = max_log.logins;
			else
				max_log.logins = utmpshm->max_login_num;
		}
		fseek(fp, 0, SEEK_SET);
		fwrite(&max_log, sizeof(max_log), 1, fp);
		fclose(fp);
	}
	snprintf(genbuf, sizeof(genbuf), 
		"\033[1;32m从 [\033[36m%4d年%2d月%2d日\033[32m] 起, "
		"最高人数记录: [\033[36m%d\033[32m] "
		"累计访问人次: [\033[36m%lu\033[32m]\033[m\n",
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
	resolve_utmp();

	if (!name || *name == '\0')
		return BBS_ENOUSR;

	if (currentuser.userid[0] == '\0') {
		if (count_online() > MAXACTIVE)
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

	session.uid = get_user_id(name);

	return 0;
}

static int login_query(void)
{
#ifndef ENABLE_SSH
	char uname[IDLEN + 2];
	char passbuf[PASSLEN];
	int attempts;
	char *ptr;
	int recover; // For giveupBBS
	bool auth = false;
#endif // ENABLE_SSH

	resolve_utmp();
	// Deny new logins if too many users online.
	int online = count_online();
#ifndef ENABLE_SSH
	if (online >= MAXACTIVE) {
		ansimore("etc/loginfull", NA);
		return -1;
	}
#endif // ENABLE_SSH

	if (fill_shmfile(1, "etc/issue", "ISSUE_SHMKEY")) {
		show_issue();
	}
	prints("\033[1;35m欢迎光临\033[1;40;33m【 %s 】 \033[m"
			"[\033[1;33;41m Add '.' after YourID to login for BIG5 \033[m]\n",
			BBSNAME);
	
	utmpshm->total_num = online;
	if (utmpshm->max_login_num < utmpshm->total_num)
		utmpshm->max_login_num = utmpshm->total_num;
	if (utmpshm->usersum <= 0)
		utmpshm->usersum = allusers();

	prints("\033[1;32m目前已有帐号数: [\033[1;36m%d\033[32m/\033[36m%d\033[32m] "
			"\033[32m目前上站人数: [\033[36m%d\033[32m/\033[36m%d\033[1;32m]\n",
			utmpshm->usersum, MAXUSERS, online, MAXACTIVE);
	visitlog();

#ifndef ENABLE_SSH
	attempts = 0;
	while (!auth) {
		if (attempts++ >= LOGINATTEMPTS) {
			ansimore("etc/goodbye", NA);
			return -1;
		}
		getdata(0, 0, "\033[1;33m请输入帐号\033[m"
				"(试用请输入'\033[1;36mguest\033[m', "
				"注册请输入'\033[1;31mnew\033[m'): ",
				uname, IDLEN + 1, DOECHO, YEA);
#ifdef ALLOWSWITCHCODE
		ptr = strchr(uname, '.');
		if (ptr) {
			convcode = 1;
			*ptr = '\0';
		}
#endif
		if (strcaseeq(uname, "guest") && (online > MAXACTIVE - 10)) {
			ansimore("etc/loginfull", NA);
			return -1;
		}
		if (strcaseeq(uname, "new")) {
			memset(&currentuser, 0, sizeof(currentuser));
			new_register();
			oflush();
			exit(1);
		} else if (*uname == '\0')
			;
		else if (!dosearchuser(uname, &currentuser, &usernum)) {
			prints("\033[1;31m经查证，无此 ID。\033[m\n");
		} else if (strcaseeq(uname, "guest")) {
			currentuser.userlevel = 0;
			break;
		} else {
#ifdef ALLOWSWITCHCODE
			if (!convcode)
				convcode = !(currentuser.userdefine & DEF_USEGB);
#endif
			getdata(0, 0, "\033[1;37m请输入密码: \033[m", passbuf, PASSLEN,
					NOECHO, YEA);
			passbuf[8] = '\0';
			switch (bbs_auth(uname, passbuf)) {
				case BBS_EWPSWD:
					prints("\033[1;31m密码输入错误...\033[m\n");
					break;
				case BBS_EGIVEUP:
					recover = chk_giveupbbs();
					prints("\033[33m您正在戒网，离戒网结束还有%d天\033[m\n",
							recover - time(NULL) / 3600 / 24);
					return -1;
				case BBS_ESUICIDE:
					prints("\033[32m您已经自杀\033[m\n");
					return -1;
				case BBS_EBANNED:
					prints("\033[32m本帐号已停机。请到 "
							"\033[36mNotice\033[32m版 查询原因\033[m\n");
					return -1;
				case BBS_ELFREQ:
					prints("登录过于频繁，请稍候再来\n");
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
	presskeyfor("\033[1;33m欢迎使用ssh方式访问本站，请按任意键继续", t_lines - 1);
#endif // ENABLE_SSH

	if (multi_user_check() == -1)
		return -1;

	dumb_term = false;
#ifndef ENABLE_SSH
	check_tty_lines();
#endif // ENABLE_SSH
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
	time_t now;

	maxsec = 86400;
	lastnote = 0;
	if ((check = fopen("etc/checknotepad", "r")) != NULL) {
		fgets(tmp, sizeof(tmp), check);
		lastnote = atol(tmp);
		fclose(check);
	}
	now = time(NULL);
	if ((now - lastnote) >= maxsec) {
		move(t_lines - 1, 0);
		prints("对不起，系统自动发信，请稍候.....");
		refresh();
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
					char *str = getdatestring(now, DATE_ZH);
					snprintf(notetitle, sizeof(notetitle), "[%14.14s %6.6s] %s",
							str, str + 23, ntitle);
					if (dashf(fname)) {
						Postfile(fname, bname, notetitle, 1);
						sprintf(tmp, "%s 自动张贴", ntitle);
						report(tmp, currentuser.userid);
					}
				}
			}
			fclose(check);
		}
		char *str = getdatestring(now, DATE_ZH);
		snprintf(notetitle, sizeof(notetitle), "[%14.14s %6.6s] 留言板记录",
				str, str + 23);
		if (dashf("etc/notepad")) {
			Postfile("etc/notepad", "Notepad", notetitle, 1);
			unlink("etc/notepad");
		}
		report("自动发信时间更改", currentuser.userid);
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
	started = 1;

	initscr();
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
				igetkey();
				clear();
			}
			currentuser.noteline = noteln;
			write_defnotepad();
		}
	}
#endif

	if (show_statshm("etc/hotspot", 0)) {
		refresh();
		pressanykey();
	}

	if ((vote_flag(NULL, '\0', 2 /* 检查读过新的Welcome 没 */) == 0)) {
		if (dashf("Welcome")) {
			ansimore("Welcome", YEA);
			vote_flag(NULL, 'R', 2 /* 写入读过新的Welcome */);
		}
	} else {
		//if (fill_shmfile(3, "Welcome2", "WELCOME_SHMKEY"))
			//show_welcomeshm();
		ansimore("Welcome2", YEA);
	}
	show_statshm("0Announce/bbslist/day", 1);
	refresh();
	move(t_lines - 2, 0);
	clrtoeol();
	if (currentuser.numlogins < 1) {
		currentuser.numlogins = 0;
		prints("\033[1;36m☆ 这是您第 \033[33m1\033[36m 次拜访本站，请记住今天吧。\n");
		prints("☆ 您第一次连入本站的时间为 \033[33m%s\033[m ", getdatestring(time(NULL), DATE_ZH));
	} else {
		prints(
				"\033[1;36m☆ 这是您第 \033[33m%d\033[36m 次拜访本站，上次您是从 \033[33m%s\033[36m 连往本站。\n",
				currentuser.numlogins + 1, currentuser.lasthost);
		prints("☆ 上次连线时间为 \033[33m%s\033[m ", getdatestring(currentuser.lastlogin, DATE_ZH));
	}
	igetkey();
	WishNum = 9999;
	setuserfile(fname, BADLOGINFILE);
	if (ansimore(fname, NA) != -1) {
		if (askyn("您要删除以上密码输入错误的记录吗", NA, NA) == YEA)
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

	basic_session_info_t *res = get_my_sessions();
	update_user_stay(&currentuser, true, basic_session_info_count(res) > 1);
	basic_session_info_clear(res);

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

// Recover user's work from abnormal exit.
static void c_recover(void)
{
	char fname[STRLEN], buf[STRLEN];
	int a;

	sprintf(fname, "home/%c/%s/%s.deadve", toupper(currentuser.userid[0]),
			currentuser.userid, currentuser.userid);
	if (!dashf(fname) || strcmp(currentuser.userid, "guest") == 0)
		return;
	clear();
	genbuf[0] = '\0';
	getdata(0, 0, "\033[1;32m您有一个编辑作业不正常中断，"
			"(S) 写入暂存档 (M) 寄回信箱 (Q) 算了？[M]：\033[m",
			genbuf, 2, DOECHO, YEA);
	switch (genbuf[0]) {
		case 'Q':
		case 'q':
			unlink(fname);
			break;
		case 'S':
		case 's':
			while (1) {
				genbuf[0] = '\0';
				getdata(2, 0, "\033[1;33m请选择暂存档 [0-7] [0]：\033[m",
					genbuf, 2, DOECHO, YEA);
				if (genbuf[0] == '\0')
					a = 0;
				else
					a = atoi(genbuf);
				if (a >= 0 && a <= 7) {
					sprintf(buf, "home/%c/%s/clip_%d",
						toupper(currentuser.userid[0]),
						currentuser.userid, a);
					if (dashf(buf)) {
						getdata(
							3, 0, "\033[1;31m暂存档已存在，覆盖或附加? "
							"(O)覆盖 (A)附加 [O]：\033[m",
							genbuf, 2, DOECHO, YEA);
						switch (genbuf[0]) {
							case 'A':
							case 'a':
								f_cp(fname, buf, O_APPEND);
								unlink(fname);
								break;
							default:
								unlink(buf);
								rename(fname, buf);
								break;
						}
					} else
						rename(fname, buf);
					break;
				}
			}
			break;
		default:
			mail_file(fname, currentuser.userid,
				"不正常断线所保留的部份...");
			unlink(fname);
			break;
	}
}

#ifdef TALK_LOG
// Recover user's talk log from abnormal exit.
void tlog_recover(void)
{
	char buf[256];

	sethomefile(buf, currentuser.userid, "talk_log");
	if (strcasecmp(currentuser.userid, "guest") == 0 || !dashf(buf))
		return;

	clear();
	genbuf[0] = '\0';
	getdata(0, 0, "\033[1;32m您有一个不正常断线所留下来的聊天记录, "
			"您要 .. (M) 寄回信箱 (Q) 算了？[Q]：\033[m",
			genbuf, 2, DOECHO, YEA);

	if (genbuf[0] == 'M' || genbuf[0] == 'm') {
		mail_file(buf, currentuser.userid, "聊天记录");
	}
	unlink(buf);
	return;
}
#endif

void start_client(void)
{
	extern char currmaildir[];

	initialize_db();
	initialize_mdb();

	initialize_convert_env();
#ifdef ALLOWSWITCHCODE
	if (resolve_gbkbig5_table() < 0)
		exit(1);
#endif
	system_init();

	if (setjmp(byebye)) {
		system_abort();
	}

	strlcpy(BoardName, BBSNAME, sizeof(BoardName));

	if (login_query() == -1) {
		oflush();
		sleep(3);
		exit(1);
	}
	user_login();

	setmdir(currmaildir, currentuser.userid);
	RMSG = NA;
	clear();
	c_recover();
#ifdef TALK_LOG
	tlog_recover();
#endif

	if (strcmp(currentuser.userid, "guest")) {
		if (check_maxmail())
			pressanykey();
		move(9, 0);
		clrtobot();
		if (!DEFINE(DEF_NOLOGINSEND))
			if (session.visible)
				login_msg();
		clear();
		set_numofsig();
	}

	ActiveBoard_Init();
	fill_date();

	if (DEFINE(DEF_LOGFRIEND)
			&& online_follows_count(!HAS_PERM(PERM_SEECLOAK)) > 0)
		show_online_followings();

	sysconf_load(false);
	while (1) {
		if (DEFINE(DEF_NORMALSCR))
			domenu("TOPMENU");
		else
			domenu("TOPMENU2");
		Goodbye();
	}
}
