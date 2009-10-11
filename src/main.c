// Handle user login.

#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif

#define BADLOGINFILE   "logins.bad"
#define VISITLOG    BBSHOME"/.visitlog"

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif
int RMSG = YEA;
int msg_num = 0;
int count_friends = 0;
int iscolor = 1;
int mailXX = 0; // If mail quota is exceeded.
int numofsig = 0;
jmp_buf byebye;
int talkrequest = NA;
time_t lastnote;
struct user_info uinfo;
char fromhost[60];
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
int friend_login_wall();
struct user_info *t_search();
void r_msg();
void count_msg();

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

// Some initialization when user enters.
static void u_enter(void)
{
	// Initialization.
	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.active = YEA;
	uinfo.pid = getpid();
	uinfo.currbrdnum = 0;
	if (!HAS_PERM(PERM_CLOAK))
		currentuser.flags[0] &= ~CLOAK_FLAG;
	if (HAS_PERM(PERM_LOGINCLOAK) && (currentuser.flags[0] & CLOAK_FLAG))
		uinfo.invisible = YEA;
	uinfo.mode = LOGIN;
	uinfo.pager = 0;

	chk_giveupbbs();

	uinfo.idle_time = time(NULL);

	// Load user preferences.
	if (DEFINE(DEF_DELDBLCHAR))
		enabledbchar = 1;
	else
		enabledbchar = 0;
	if (DEFINE(DEF_FRIENDCALL)) {
		uinfo.pager |= FRIEND_PAGER;
	}
	if (currentuser.flags[0] & PAGER_FLAG) {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}
	if (DEFINE(DEF_FRIENDMSG)) {
		uinfo.pager |= FRIENDMSG_PAGER;
	}
	if (DEFINE(DEF_ALLMSG)) {
		uinfo.pager |= ALLMSG_PAGER;
		uinfo.pager |= FRIENDMSG_PAGER;
	}
	if (DEFINE(DEF_LOGOFFMSG)) {
		uinfo.pager |= LOGOFFMSG_PAGER;
	}
	uinfo.uid = usernum;
	strlcpy(uinfo.from, fromhost, sizeof(uinfo.from));
	// Terrible..
	if (!DEFINE(DEF_NOTHIDEIP)) {
		uinfo.from[22] = 'H';
	}
	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	strlcpy(uinfo.userid, currentuser.userid, sizeof(uinfo.userid));
	strlcpy(uinfo.realname, currentuser.realname, sizeof(uinfo.realname));
	strlcpy(uinfo.username, currentuser.username, sizeof(uinfo.username));
	getfriendstr();
	getrejectstr();

	// Try to get an entry in user cache.
	int ucount = 0;
	while (1) {
		utmpent = getnewutmpent(&uinfo);
		if (utmpent >= 0 || utmpent == -1)
			break;
		if (utmpent == -2 && ucount <= 100) {
			ucount++;
			struct timeval t = {0, 250000};
			select( 0, NULL, NULL, NULL, &t); // wait 0.25s before another try
			continue;
		}
		if (ucount > 100) {
			char buf1[] = "getnewutmpent(): too much times, give up.";
			report(buf1, currentuser.userid);
			prints("getnewutmpent(): 失败太多次, 放弃. 请回报站长.\n");
			sleep(3);
			exit(0);
		}
	}
	if (utmpent < 0) {
		char buf2[STRLEN];
		snprintf(buf2, sizeof(buf2),
			"Fault: No utmpent slot for %s", uinfo.userid);
		report(buf2, currentuser.userid);
	}

	digestmode = NA;
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
	time_t recent;
	time_t stay = 0;
	time_t now;

	// 这些信号的处理要关掉, 否则在离线时等候回车时出现
	// 信号会导致重写名单, 这个导致的名单混乱比kick user更多  (ylsdd)
	signal(SIGHUP, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	setflags(PAGER_FLAG, (uinfo.pager & ALL_PAGER));
	if (HAS_PERM(PERM_LOGINCLOAK))
		setflags(CLOAK_FLAG, uinfo.invisible);

	set_safe_record();
	now = time(NULL);
	recent = login_start_time;
	if (currentuser.lastlogout > recent)
		recent = currentuser.lastlogout;
	if (currentuser.lastlogin > recent)
		recent = currentuser.lastlogin;
	stay = now - recent;
	if (stay < 0)
		stay = 0;
	
	currentuser.lastlogout = now;
	currentuser.stay += stay;
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	uidshm->status[usernum - 1]--;

	uinfo.invisible = YEA;
	uinfo.sockactive = NA;
	uinfo.sockaddr = 0;
	uinfo.destuid = 0;
	uinfo.pid = 0;
	uinfo.active = NA;
	update_ulist(&uinfo, utmpent);
}

// Bell when user receives an talk request.
static void talk_request(int nothing)
{
	signal(SIGUSR1, talk_request);
	talkrequest = YEA;
	bell();
	bell();
	bell();
	sleep(1);
	bell();
	bell();
	bell();
	bell();
	bell();
	return;
}

// Handle abnormal exit.
void abort_bbs(int nothing)
{
	extern int child_pid;

	if (child_pid) {
		kill(child_pid, SIGKILL);
	}

	// Save user's work.
	if (uinfo.mode == POSTING || uinfo.mode == SMAIL || uinfo.mode == EDIT
			|| uinfo.mode == EDITUFILE || uinfo.mode == EDITSFILE
			|| uinfo.mode == EDITANN)
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

// Compare 'unum' to 'urec'->uid. (uid)
static int cmpuids2(int unum, const struct user_info *urec)
{
	return (unum == urec->uid);
}

// Count active logins of a user with 'usernum'(uid).
// Called by count_user().
static int count_multi(const struct user_info *uentp)
{
	static int count;

	if (uentp == NULL) {
		int num = count;
		count = 0;
		return num;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->uid == usernum)
		count++;
	return 1;
}

// Count active logins of a user with 'usernum'(uid).
static int count_user(void)
{
	count_multi(NULL);
	apply_ulist(count_multi);
	return count_multi(NULL);
}

#ifdef IPMAXLOGINS
// Compare 'str' to strings in file 'filename'.
// Return 1 if leading characters of 'str' matches (case insensitive)
// any of those strings, 0 otherwise.
static int IsSpecial(const char *str, const char *filename)
{
	FILE *fp;
	char line[STRLEN];
	char *ptr;
	int i = 0;

	if ((fp = fopen(filename, "r")) != NULL) {
		while (fgets(line, sizeof(line), fp)) {
			ptr = strtok(line, " \r\n\t");
			if (!ptr[0] || ptr[0] == '#')
				continue;
			else if (!strncmp(str, ptr, strlen(ptr))) {
				i = 1;
				break;
			}
		}
		fclose(fp);
	}
	return i;
}

// Count active logins from IP 'fromhost'.
// Called by count_ip().
static int _cnt_ip(struct user_info *uentp)
{
	static int count;

	if (uentp == NULL) {
		int num = count;
		count = 0;
		return num;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (!strcmp(uentp->userid, "guest"))
		return 0;
	if (!strcmp(uentp->from, fromhost))
		count++;
	return 1;
}

// Count active logins from IP 'fromhost'.
static int count_ip(void)
{
	_cnt_ip(NULL);
	apply_ulist(_cnt_ip);
	return _cnt_ip(NULL);
}

// Check if there is greater than or equal to IPMAXLOGINS
// active processes from IP 'fromhost'. If so, deny more logins.
// guest users, or users from IP addresses in "etc/freeip"
// or not in "etc/restrictip" are not checked.
static int iplogins_check(void)
{
	int sameip;

	if (currentuser.userid && !strcmp(currentuser.userid, "guest"))
		return 0;
	if (!IsSpecial(fromhost, "etc/restrictip")
			|| IsSpecial(fromhost, "etc/freeip")) {
		return 0;
	} else {
		sameip = count_ip();
	}
	if (sameip >= IPMAXLOGINS) {
		prints("\033[1;32m为确保他人上站权益, 本站仅允许此IP同时登陆 %d 个。\n\033[m",
				IPMAXLOGINS);
		prints("\033[1;36m您目前已经使用该IP登陆了 %d 个！\n\033[m", sameip);
		return -1;
	}
	return 0;
}
#endif

// Prevent too many logins of same account.
static int multi_user_check(void)
{
	struct user_info uin;
	int logins, mustkick = 0;

	// Don't check sysops.
	if (HAS_PERM(PERM_MULTILOG))
		return 0;

	logins = count_user();

	// Allow no more than MAXGUEST guest users.
	if (!strcasecmp("guest", currentuser.userid)) {
		if (logins > MAXGUEST) {
			prints("\033[1;33m抱歉, 目前已有太多 \033[1;36mguest\033[33m, 请稍后再试。\033[m\n");
			return -1;
		}
		return 0;
	}
	// For users without PERM_SPECIAL0, MULTI_LOGINS logins are allowed.
	// A user with PERM_SPEACIAL0 is allowed up to 6 logins.
	// (actually 4, finding the bug..)
	else if ((!HAS_PERM(PERM_SPECIAL0) && logins >= MULTI_LOGINS)
			|| logins > 5) {
		prints("\033[1;32m为确保他人上站权益, 本站仅允许您用该帐号登录 %d 个。\n\033[m", MULTI_LOGINS);
		prints("\033[1;36m您目前已经使用该帐号登录了 %d 个，您必须断开其他的连接方能进入本站！\n\033[m", logins);
		mustkick = 1;
	}
	if (search_ulist(&uin, cmpuids2, usernum) 
		&& (uin.active || (uin.pid && bbskill(&uin, 0) == -1))) {
		getdata(0, 0, "\033[1;37m您想删除重复的 login 吗 (Y/N)? [N]\033[m", genbuf, 4,
				DOECHO, YEA);

		if (genbuf[0] == 'N' || genbuf[0] == 'n' || genbuf[0] == '\0') {
			if (mustkick) {
				prints("\033[33m很抱歉，您已经用该帐号登录 %d 个，所以，此连线将被取消。\033[m\n", logins);
				return -1;
			}
		} else {
			if (!uin.pid)
				return 0;
			bbskill(&uin, SIGHUP);
			report("kicked (multi-login)", currentuser.userid);
		}
	}
#ifdef IPMAXLOGINS
	return iplogins_check();
#endif
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
	init_alarm();
	uinfo.mode = LOGIN;
	alarm(LOGIN_TIMEOUT);
#else
	signal(SIGALRM, SIG_SIG);
#endif
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
#endif
	signal(SIGUSR1, talk_request);

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = r_msg;
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

// Log login attempts.
static void logattempt(char *uid, char *frm)
{
	char fname[STRLEN];

	snprintf(genbuf, sizeof(genbuf), "%-12.12s  %-30s %s\n", uid,
			getdatestring(time(NULL), DATE_ZH), frm);
	file_append(BADLOGINFILE, genbuf);
	sethomefile(fname, uid, BADLOGINFILE);
	file_append(fname, genbuf);
}

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
	write(STDIN_FILENO, buf1, 3);
	n = read(STDIN_FILENO, buf2, 80);
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

static int login_query(void)
{
	char uid[IDLEN + 2];
	char passbuf[PASSLEN];
	int curr_login_num;
	int attempts;
	char *ptr;
	int recover; // For giveupBBS

	// Deny new logins if too many users (>=MAXACTIVE) online.
	resolve_utmp();
	curr_login_num = count_online();
	if (curr_login_num >= MAXACTIVE) {
		ansimore("etc/loginfull", NA);
		return -1;
	}

#ifdef BBSNAME
	strcpy(BoardName, BBSNAME);
#else
	ptr = sysconf_str("BBSNAME");
	if (ptr == NULL)
		ptr = "尚未命名测试站";
	strcpy(BoardName, ptr);
#endif

	if (fill_shmfile(1, "etc/issue", "ISSUE_SHMKEY")) {
		show_issue();
	}
	prints("\033[1;35m欢迎光临\033[1;40;33m【 %s 】 \033[m"
			"[\033[1;33;41m Add '.' after YourID to login for BIG5 \033[m]\n",
			BoardName);
	utmpshm->total_num = curr_login_num;
	if (utmpshm->max_login_num < utmpshm->total_num)
		utmpshm->max_login_num = utmpshm->total_num;
	if (utmpshm->usersum <= 0)
		utmpshm->usersum = allusers();
	prints("\033[1;32m目前已有帐号数: [\033[1;36m%d\033[32m/\033[36m%d\033[32m] "
			"\033[32m目前上站人数: [\033[36m%d\033[32m/\033[36m%d\033[1;32m]\n",
			utmpshm->usersum, MAXUSERS, utmpshm->total_num, MAXACTIVE);
	visitlog();

	attempts = 0;
	while (1) {
		if (attempts++ >= LOGINATTEMPTS) {
			ansimore("etc/goodbye", NA);
			return -1;
		}
		getdata(0, 0, "\033[1;33m请输入帐号\033[m"
				"(试用请输入'\033[1;36mguest\033[m', "
				"注册请输入'\033[1;31mnew\033[m'): ",
				uid, IDLEN + 1, DOECHO, YEA);
#ifdef ALLOWSWITCHCODE
		ptr = strchr(uid, '.');
		if (ptr) {
			convcode = 1;
			*ptr = '\0';
		}
#endif
		if ((strcasecmp(uid, "guest") == 0)
				&& (MAXACTIVE - curr_login_num < 10)) {
				ansimore("etc/loginfull", NA);
			return -1;
		}
		if (strcasecmp(uid, "new") == 0) {
#ifdef LOGINASNEW
			memset(&currentuser, 0, sizeof(currentuser));
			new_register();
			// TODO: Already exit..
			ansimore3("etc/firstlogin", YEA);
			break;
#else
			prints("\033[1;37m本系统目前无法以 \033[36mnew\033[37m 注册, "
				"请用\033[36m guest\033[37m 进入...\033[m\n");
#endif
		} else if (*uid == '\0')
			;
		else if (!dosearchuser(uid, &currentuser, &usernum)) {
			prints("\033[1;31m经查证，无此 ID。\033[m\n");
		} else if (strcasecmp(uid, "guest") == 0) {
			currentuser.userlevel = 0;
			break;
		} else {
#ifdef ALLOWSWITCHCODE
			if (!convcode)
				convcode = !(currentuser.userdefine & DEF_USEGB);
#endif
			getdata(0, 0, "\033[1;37m请输入密码: \033[m", passbuf, PASSLEN, NOECHO,
					YEA);
			passbuf[8] = '\0';
			if (!checkpasswd(currentuser.passwd, passbuf)) {
				logattempt(currentuser.userid, fromhost);
				prints("\033[1;31m密码输入错误...\033[m\n");
			} else {
				if (strcasecmp(currentuser.userid, "guest")
						&& !HAS_PERM(PERM_LOGIN)) {
					recover = chk_giveupbbs();
					if (recover) {
						prints("\033[33m您正在戒网，"
							"离戒网结束还有%d天\033[m\n",
							recover - time(NULL) / 3600 / 24);
						return -1;
					}
					if (currentuser.userlevel == 0) {
						prints("\033[32m您已经自杀\033[m\n");
						return -1;
					} else {
						prints("\033[32m本帐号已停机。请到 "
							"\033[36mNotice\033[32m版 查询原因\033[m\n");
						return -1;
					}
				}
#ifdef CHECK_FREQUENTLOGIN
				if (!HAS_PERM(PERM_SYSOPS)
						&& strcasecmp(currentuser.userid, "guest") != 0
						&& abs(time(NULL) - currentuser.lastlogin) < 10) {
					prints("登录过于频繁，请稍候再来\n");
					report("Too Frequent", currentuser.userid);
					return -1;
				}
#endif
				memset(passbuf, 0, PASSLEN - 1);
				break;
			}
		}
	}

	if (multi_user_check() == -1)
		return -1;

	if (!term_init(currentuser.termtype)) {
		prints("Bad terminal type. Defaulting to 'vt100'\n");
		strcpy(currentuser.termtype, "vt100");
		term_init(currentuser.termtype);
	}

	check_tty_lines();
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
		fprintf(check, "%d", lastnote);
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
	int logins;

	// SYSOP gets all permission bits when login.
	if (strcmp(currentuser.userid, "SYSOP") == 0) {
		currentuser.userlevel = ~0;
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser),
				usernum);
	}
	fromhost[59] = 0; //added by iamfat 2004.01.05 to avoid overflow
	log_usies("ENTER", fromhost, &currentuser);

	SpecialID(currentuser.userid, fromhost, sizeof(fromhost));

	u_enter();
	report("Enter", currentuser.userid);
	started = 1;

	// Seems unnecessary since having done so in login_query().
	logins = count_user();
	if (! (HAS_PERM(PERM_MULTILOG)
		|| (HAS_PERM(PERM_SPECIAL0) && logins < 5)
		|| (logins <= MULTI_LOGINS))
		&& strcmp(currentuser.userid, "guest")) {
		report("kicked (multi-login)[漏网之鱼]", currentuser.userid);
		abort_bbs(0);
	}

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
		if (fill_shmfile(3, "Welcome2", "WELCOME_SHMKEY"))
			show_welcomeshm();
	}
	show_statshm("etc/posts/day", 1);
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
	check_uinfo(&currentuser, 0);
	strlcpy(currentuser.lasthost, fromhost, sizeof(currentuser.lasthost));
	currentuser.lasthost[15] = '\0'; /* dumb mistake on my part */
	{
		time_t stay, recent;

		if (count_user() > 1) {
			recent = currentuser.lastlogout;
			if (currentuser.lastlogin > recent)
				recent = currentuser.lastlogin;
			stay = login_start_time - recent;
			if (stay < 0)
				stay = 0;
		} else
			stay = 0;

		if (login_start_time - currentuser.lastlogin >= 20 * 60
				|| !strcmp(currentuser.userid, "guest")
				|| currentuser.numlogins < 100){
			currentuser.numlogins++;
		}
		currentuser.lastlogin = login_start_time;
		currentuser.stay += stay;
	}

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

	load_sysconf();
#ifdef ALLOWSWITCHCODE
	if (resolve_gbkbig5_table() < 0)
		exit(1);
#endif
	system_init();

	if (setjmp(byebye)) {
		system_abort();
	}

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
			if (!uinfo.invisible)
				apply_ulist(friend_login_wall);
		clear();
		set_numofsig();
	}

	ActiveBoard_Init();
	fill_date();

	num_alcounter();
	if (count_friends > 0 && DEFINE(DEF_LOGFRIEND))
		t_friends();
	while (1) {
		if (DEFINE(DEF_NORMALSCR))
			domenu("TOPMENU");
		else
			domenu("TOPMENU2");
		Goodbye();
	}
}
