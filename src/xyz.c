#define EXTERN
#include <sys/wait.h>
#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
int use_define = 0;
int child_pid = 0;
extern int iscolor;
extern int enabledbchar;

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif

extern struct UCACHE *uidshm;
#define TH_LOW	10
#define TH_HIGH	15

//      对于权限定义值,判断其第i位是否为真,并根据use_define的值来
//      调整其对应位的权限显示字符串
//      最后在由i指示的位置处显示,更新
int showperminfo(int pbits, int i) {
	char buf[STRLEN];
	sprintf(buf, "%c. %-30s %2s", 'A' + i,
			(use_define) ? user_definestr[i] : permstrings[i], ((pbits
					//% >> i) & 1 ? "是" : "×"));
					>> i) & 1 ? "\xca\xc7" : "\xa1\xc1"));
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	outs(buf);
	refresh();
	return YEA;
}

//      更改用户的权限设定
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) ())
{
	int lastperm = numbers - 1;
	int i, done = NA;
	char choice[3], buf[80];
	move(4, 0);
	//% prints("请按下您要的代码来设定%s，按 Enter 结束.\n", prompt);
	prints("\xc7\xeb\xb0\xb4\xcf\xc2\xc4\xfa\xd2\xaa\xb5\xc4\xb4\xfa\xc2\xeb\xc0\xb4\xc9\xe8\xb6\xa8%s\xa3\xac\xb0\xb4 Enter \xbd\xe1\xca\xf8.\n", prompt);
	move(6, 0);
	clrtobot();
	for (i = 0; i <= lastperm; i++) {
		(*showfunc)(pbits, i, NA);
	}
	while (!done) {
		//% sprintf(buf, "选择(ENTER 结束%s): ",
		sprintf(buf, "\xd1\xa1\xd4\xf1(ENTER \xbd\xe1\xca\xf8%s): ",
				//% ((strcmp(prompt, "权限") != 0)) ? "" : "，0 停权");
				((strcmp(prompt, "\xc8\xa8\xcf\xde") != 0)) ? "" : "\xa3\xac""0 \xcd\xa3\xc8\xa8");
		getdata(t_lines - 1, 0, buf, choice, 2, DOECHO, YEA);
		*choice = toupper(*choice);
		/*		if (*choice == '0')
		 return (0);
		 else modified by kit,rem 0停权* remed all by Amigo 2002.04.03*/
		if (*choice == '\n' || *choice == '\0')
		done = YEA;
		else if (*choice < 'A' || *choice> 'A' + lastperm)
		bell ();
		else {
			i = *choice - 'A';
			pbits ^= (1 << i);
			if ((*showfunc) (pbits, i, YEA) == NA) {
				pbits ^= (1 << i);
			} //if
		} //else
	}
	//while !done
	return (pbits);
}

//      pager与msg设定
//
int x_userdefine() {
	int id;
	unsigned int newlevel;
	set_user_status(ST_USERDEF);
	if (!(id = getuser(currentuser.userid))) {
		move(3, 0);
		//% prints("错误的使用者 ID...");
		prints("\xb4\xed\xce\xf3\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf ID...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	use_define = 1;
	//% newlevel = setperms(lookupuser.userdefine, "参数", NUMDEFINES,
	newlevel = setperms(lookupuser.userdefine, "\xb2\xce\xca\xfd", NUMDEFINES,
			showperminfo);
	move(2, 0);
	if (newlevel == lookupuser.userdefine)
		//% prints("参数没有修改...\n");
		prints("\xb2\xce\xca\xfd\xc3\xbb\xd3\xd0\xd0\xde\xb8\xc4...\n");
	else {
#ifdef ALLOWSWITCHCODE
		if ((!convcode && !(newlevel & DEF_USEGB))
				|| (convcode && (newlevel & DEF_USEGB)))
		switch_code ();
#endif
		lookupuser.userdefine = newlevel;
		currentuser.userdefine = newlevel;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		if (DEFINE(DEF_DELDBLCHAR))
			enabledbchar = 1;
		else
			enabledbchar = 0;
		//% prints("新的参数设定完成...\n\n");
		prints("\xd0\xc2\xb5\xc4\xb2\xce\xca\xfd\xc9\xe8\xb6\xa8\xcd\xea\xb3\xc9...\n\n");
	}
	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	pressreturn();
	clear();
	use_define = 0;
	return 0;
}

int x_cloak(void)
{
	if (set_visibility(!session.visible) == 0) {
		session.visible = !session.visible;

		report("toggle cloak", currentuser.userid);

		move(t_lines - 1, 0);
		clrtoeol();
		//% prints("隐身术已经 %s 了!", session.visible ? "停止" : "启动");
		prints("\xd2\xfe\xc9\xed\xca\xf5\xd2\xd1\xbe\xad %s \xc1\xcb!", session.visible ? "\xcd\xa3\xd6\xb9" : "\xc6\xf4\xb6\xaf");
		pressreturn();
	}
	return 0;
}

//修改用户的档案
void x_edits() {
	int aborted;
	char ans[7], buf[STRLEN];
	int ch, num, confirm;
	extern int WishNum;
	static char *e_file[] = { "plans", "signatures", "notes", "logout",
			"GoodWish", NULL };
	//% static char *explain_file[] = { "个人说明档", "签名档", "自己的备忘录", "离站的画面",
	static char *explain_file[] = { "\xb8\xf6\xc8\xcb\xcb\xb5\xc3\xf7\xb5\xb5", "\xc7\xa9\xc3\xfb\xb5\xb5", "\xd7\xd4\xbc\xba\xb5\xc4\xb1\xb8\xcd\xfc\xc2\xbc", "\xc0\xeb\xd5\xbe\xb5\xc4\xbb\xad\xc3\xe6",
			//% "底部流动信息", NULL };
			"\xb5\xd7\xb2\xbf\xc1\xf7\xb6\xaf\xd0\xc5\xcf\xa2", NULL };
	set_user_status(ST_GMENU);
	clear();
	move(1, 0);
	//% prints("编修个人档案\n\n");
	prints("\xb1\xe0\xd0\xde\xb8\xf6\xc8\xcb\xb5\xb5\xb0\xb8\n\n");
	for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
		prints("[[1;32m%d[m] %s\n", num + 1, explain_file[num]);
	}
	//% prints("[[1;32m%d[m] 都不想改\n", num + 1);
	prints("[[1;32m%d[m] \xb6\xbc\xb2\xbb\xcf\xeb\xb8\xc4\n", num + 1);

	//% getdata(num + 5, 0, "您要编修哪一项个人档案: ", ans, 2, DOECHO, YEA);
	getdata(num + 5, 0, "\xc4\xfa\xd2\xaa\xb1\xe0\xd0\xde\xc4\xc4\xd2\xbb\xcf\xee\xb8\xf6\xc8\xcb\xb5\xb5\xb0\xb8: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n'
			|| ans[0] == '\0')
		return;

	ch = ans[0] - '0' - 1;
	setuserfile(genbuf, e_file[ch]);
	move(3, 0);
	clrtobot();
	//% sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
	sprintf(buf, "(E)\xb1\xe0\xbc\xad (D)\xc9\xbe\xb3\xfd %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		//% confirm = askyn("您确定要删除这个档案", NA, NA);
		confirm = askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xc9\xbe\xb3\xfd\xd5\xe2\xb8\xf6\xb5\xb5\xb0\xb8", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			//% prints("取消删除行动\n");
			prints("\xc8\xa1\xcf\xfb\xc9\xbe\xb3\xfd\xd0\xd0\xb6\xaf\n");
			pressreturn();
			clear();
			return;
		}
		unlink(genbuf);
		move(5, 0);
		//% prints("%s 已删除\n", explain_file[ch]);
		prints("%s \xd2\xd1\xc9\xbe\xb3\xfd\n", explain_file[ch]);
		sprintf(buf, "delete %s", explain_file[ch]);
		report(buf, currentuser.userid);
		pressreturn();
		if (ch == 4) {
			WishNum = 9999;
		}
		clear();
		return;
	}
	set_user_status(ST_EDITUFILE);
	aborted = vedit(genbuf, NA, YEA, NULL);
	clear();
	if (!aborted) {
		//% prints("%s 更新过\n", explain_file[ch]);
		prints("%s \xb8\xfc\xd0\xc2\xb9\xfd\n", explain_file[ch]);
		sprintf(buf, "edit %s", explain_file[ch]);
		if (!strcmp(e_file[ch], "signatures")) {
			set_numofsig();
			//% prints("系统重新设定以及读入您的签名档...");
			prints("\xcf\xb5\xcd\xb3\xd6\xd8\xd0\xc2\xc9\xe8\xb6\xa8\xd2\xd4\xbc\xb0\xb6\xc1\xc8\xeb\xc4\xfa\xb5\xc4\xc7\xa9\xc3\xfb\xb5\xb5...");
		}
		report(buf, currentuser.userid);
	} else {
		//% prints("%s 取消修改\n", explain_file[ch]);
		prints("%s \xc8\xa1\xcf\xfb\xd0\xde\xb8\xc4\n", explain_file[ch]);
	}
	pressreturn();
	if (ch == 4) {
		WishNum = 9999;
	}
}

//取得genbuf中保存的用户所在的记录位置到*id中,为零表示不存在
int gettheuserid(int x, char *title, int *id) {
	move(x, 0);
	usercomplete(title, genbuf);
	if (*genbuf == '\0') {
		clear();
		return 0;
	}
	if (!(*id = getuser(genbuf))) {
		move(x + 3, 0);
		//% prints("错误的使用者代号");
		prints("\xb4\xed\xce\xf3\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	return 1;
}

int x_lockscreen(void)
{
	set_user_status(ST_LOCKSCREEN);

	move(9, 0);
	clrtobot();
	move(9, 0);
	prints("\033[1;37m"
			"\n       _       _____   ___     _   _   ___     ___       __"
			"\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |"
			"\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |"
			"\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |"
			"\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|"
			"\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|\n"
			//% "\n\033[1;36m屏幕已在\033[33m %s\033[36m 时被%s暂时锁住了...\033[m",
			"\n\033[1;36m\xc6\xc1\xc4\xbb\xd2\xd1\xd4\xda\033[33m %s\033[36m \xca\xb1\xb1\xbb%s\xd4\xdd\xca\xb1\xcb\xf8\xd7\xa1\xc1\xcb...\033[m",
			format_time(time(NULL), TIME_FORMAT_ZH), currentuser.userid);

	char buf[PASSLEN + 1];
	buf[0] = '\0';
	while (*buf == '\0' || !passwd_check(currentuser.userid, buf)) {
		move(18, 0);
		clrtobot();
		//% getdata(19, 0, "请输入您的密码以解锁: ", buf, PASSLEN, NOECHO, YEA);
		getdata(19, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xc3\xdc\xc2\xeb\xd2\xd4\xbd\xe2\xcb\xf8: ", buf, PASSLEN, NOECHO, YEA);
	}
	return FULLUPDATE;
}

//#define MY_DEBUG
//  执行命令cmdfile,参数为param1
void exec_cmd(int umode, int pager, char *cmdfile, char *param1) {
	char buf[160];
	char *my_argv[18], *ptr;

	signal(SIGALRM, SIG_IGN);
	set_user_status(umode);
	clear();
	move(2, 0);
	if (!dashf(cmdfile)) {
		//% prints("文件 [%s] 不存在！\n", cmdfile);
		prints("\xce\xc4\xbc\xfe [%s] \xb2\xbb\xb4\xe6\xd4\xda\xa3\xa1\n", cmdfile);
		pressreturn();
		return;
	}
	sprintf(buf, "%s %s %s %d", cmdfile, param1, currentuser.userid,
			getpid());
	report(buf, currentuser.userid);
	my_argv[0] = cmdfile;
	strcpy(buf, param1);
	if (buf[0] != '\0')
		ptr = strtok(buf, " \t");
	else
		ptr = NULL;
	for (int i = 1; i < 18; i++) {
		if (ptr) {
			my_argv[i] = ptr;
			ptr = strtok(NULL, " \t");
		} else {
			my_argv[i] = NULL;
		}
	}
#ifdef MY_DEBUG
	for (i = 0; i < 18; i++) {
		if (my_argv[i] != NULL)
		prints ("my_argv[%d] = %s\n", i, my_argv[i]);
		else
		prints ("my_argv[%d] = (none)\n", i);
	}
	pressanykey ();
#else
	child_pid = fork();
	if (child_pid == -1) {
		//% prints("资源紧缺，fork() 失败，请稍后再使用");
		prints("\xd7\xca\xd4\xb4\xbd\xf4\xc8\xb1\xa3\xac""fork() \xca\xa7\xb0\xdc\xa3\xac\xc7\xeb\xc9\xd4\xba\xf3\xd4\xd9\xca\xb9\xd3\xc3");
		child_pid = 0;
		pressreturn();
		return;
	}
	if (child_pid == 0) {
		execv(cmdfile, my_argv);
		exit(0);
	} else {
		waitpid(child_pid, NULL, 0);
	}
#endif
	child_pid = 0;
	clear();
}

//查询使用者资料
void x_showuser() {
	char buf[STRLEN];
	set_user_status(ST_SYSINFO);
	clear();
	//% stand_title("本站使用者资料查询");
	stand_title("\xb1\xbe\xd5\xbe\xca\xb9\xd3\xc3\xd5\xdf\xd7\xca\xc1\xcf\xb2\xe9\xd1\xaf");
	ansimore("etc/showuser.msg", NA);
	getdata(20, 0, "Parameter: ", buf, 30, DOECHO, YEA);
	if ((buf[0] == '\0') || dashf("tmp/showuser.result"))
		return;
	//% securityreport("查询使用者资料", 0, 0);
	securityreport("\xb2\xe9\xd1\xaf\xca\xb9\xd3\xc3\xd5\xdf\xd7\xca\xc1\xcf", 0, 0);
	exec_cmd(ST_SYSINFO, YEA, "bin/showuser", buf);
	sprintf(buf, "tmp/showuser.result");
	if (dashf(buf)) {
		//% mail_file(buf, currentuser.userid, "使用者资料查询结果");
		mail_file(buf, currentuser.userid, "\xca\xb9\xd3\xc3\xd5\xdf\xd7\xca\xc1\xcf\xb2\xe9\xd1\xaf\xbd\xe1\xb9\xfb");
		unlink(buf);
	}
}

//  排雷游戏
int ent_winmine() {
	char buf[80];
	sprintf(buf, "%s %s", currentuser.userid, currentuser.lasthost);
	exec_cmd(ST_WINMINE, NA, "so/winmine", buf);
	return 0;
}

/**
 * Load memorial day info.
 * @return 0 on success, -1 on error.
 */
int fill_date(void)
{
	if (resolve_boards() < 0)
		return -1;

	time_t now = time(NULL);
	if (now < brdshm->fresh_date && brdshm->date[0] != '\0')
		return 0;

	struct tm *mytm = localtime(&now);
	time_t next = now - mytm->tm_hour * 3600 - mytm->tm_min * 60
			- mytm->tm_sec + 86400;

	strlcpy(brdshm->date, DEF_VALUE, sizeof(brdshm->date));

	FILE *fp = fopen(DEF_FILE, "r");
	if (fp == NULL)
		return -1;

	char date[5], index[5], buf[80], msg[30];
	strftime(date, sizeof(date), "%m%d", mytm);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (*buf == '#' || *buf == '\0')
			continue;
		if (strlcpy(index, buf, sizeof(index)) < sizeof(index))
			continue;
		strlcpy(msg, buf + sizeof(index), sizeof(msg));
		char *t = strchr(msg, '\n');
		if (t != NULL)
			*t = '\0';
		if (*index == '\0' || *msg == '\0')
			continue;
		if (strcmp(index, "0000") == 0 || strcmp(date, index) == 0) {
			// align center
			memset(brdshm->date, ' ', sizeof(msg));
			size_t len = strlen(msg);
			memcpy(brdshm->date + (sizeof(msg) - len) / 2, msg, len);
			brdshm->date[sizeof(msg)] = '\0';
		}
	}
	fclose(fp);
	brdshm->fresh_date = next;
	return 0;
}

//  今天是生日?
int is_birth(const struct userec *user)
{
	struct tm *tm;
	time_t now;

	now = time(0);
	tm = localtime(&now);

	if (strcasecmp(user->userid, "guest") == 0)
		return NA;

	if (user->birthmonth == (tm->tm_mon + 1)
			&& user->birthday == tm->tm_mday)
		return YEA;
	else
		return NA;
}

int sendGoodWish(char *userid) {
	FILE *fp;
	int tuid, i, count;
	time_t now;
	char buf[5][STRLEN], tmpbuf[STRLEN];
	char uid[IDLEN + 1], *ptr, *timestr;

	set_user_status(ST_GOODWISH);
	clear();
	move(1, 0);
	//% prints("[0;1;32m留言本[m\n您可以在这里给您的朋友送去您的祝福，");
	prints("[0;1;32m\xc1\xf4\xd1\xd4\xb1\xbe[m\n\xc4\xfa\xbf\xc9\xd2\xd4\xd4\xda\xd5\xe2\xc0\xef\xb8\xf8\xc4\xfa\xb5\xc4\xc5\xf3\xd3\xd1\xcb\xcd\xc8\xa5\xc4\xfa\xb5\xc4\xd7\xa3\xb8\xa3\xa3\xac");
	//% prints("\n也可以为您给他/她捎上一句悄悄话。");
	prints("\n\xd2\xb2\xbf\xc9\xd2\xd4\xce\xaa\xc4\xfa\xb8\xf8\xcb\xfb/\xcb\xfd\xc9\xd3\xc9\xcf\xd2\xbb\xbe\xe4\xc7\xc4\xc7\xc4\xbb\xb0\xa1\xa3");
	move(5, 0);
	if (userid == NULL) {
		//% usercomplete("请输入他的 ID: ", uid);
		usercomplete("\xc7\xeb\xca\xe4\xc8\xeb\xcb\xfb\xb5\xc4 ID: ", uid);
		if (uid[0] == '\0') {
			clear();
			return 0;
		}
	} else {
		strcpy(uid, userid);
	}
	if (!(tuid = getuser(uid))) {
		move(7, 0);
		//% prints("[1m您输入的使用者代号( ID )不存在！[m\n");
		prints("[1m\xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5( ID )\xb2\xbb\xb4\xe6\xd4\xda\xa3\xa1[m\n");
		pressanykey();
		clear();
		return -1;
	}
	move(5, 0);
	clrtoeol();
	//% prints("[m【给 [1m%s[m 留言】", uid);
	prints("[m\xa1\xbe\xb8\xf8 [1m%s[m \xc1\xf4\xd1\xd4\xa1\xbf", uid);
	move(6, 0);
	//% prints("您的留言[直接按 ENTER 结束留言，最多 5 句，每句最长 50 字符]:");
	prints("\xc4\xfa\xb5\xc4\xc1\xf4\xd1\xd4[\xd6\xb1\xbd\xd3\xb0\xb4 ENTER \xbd\xe1\xca\xf8\xc1\xf4\xd1\xd4\xa3\xac\xd7\xee\xb6\xe0 5 \xbe\xe4\xa3\xac\xc3\xbf\xbe\xe4\xd7\xee\xb3\xa4 50 \xd7\xd6\xb7\xfb]:");
	for (count = 0; count < 5; count++) {
		getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
		if (tmpbuf[0] == '\0')
			break;;
		for (ptr = tmpbuf; *ptr == ' ' && *ptr != 0; ptr++)
			;
		if (*ptr == 0) {
			count--;
			continue;
		}
		for (i = strlen(ptr) - 1; i < 0; i--)
			if (ptr[i] != ' ')
				break;
		if (i < 0) {
			count--;
			continue;
		}
		ptr[i + 1] = 0;
		strcpy(buf[count], ptr);
	}
	if (count == 0)
		return 0;
	//% sprintf(genbuf, "您确定要发送这条留言给 [1m%s[m 吗", uid);
	sprintf(genbuf, "\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xb7\xa2\xcb\xcd\xd5\xe2\xcc\xf5\xc1\xf4\xd1\xd4\xb8\xf8 [1m%s[m \xc2\xf0", uid);
	move(9 + count, 0);
	if (askyn(genbuf, YEA, NA) == NA) {
		clear();
		return 0;
	}
	sethomefile(genbuf, uid, "GoodWish");
	if ((fp = fopen(genbuf, "a")) == NULL) {
		//% prints("无法开启该用户的底部流动信息文件，请通知站长...\n");
		prints("\xce\xde\xb7\xa8\xbf\xaa\xc6\xf4\xb8\xc3\xd3\xc3\xbb\xa7\xb5\xc4\xb5\xd7\xb2\xbf\xc1\xf7\xb6\xaf\xd0\xc5\xcf\xa2\xce\xc4\xbc\xfe\xa3\xac\xc7\xeb\xcd\xa8\xd6\xaa\xd5\xbe\xb3\xa4...\n");
		pressanykey();
		return NA;
	}
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 5) = '\0';
	for (i = 0; i < count; i++) {
		//% fprintf(fp, "%s(%s)[%d/%d]：%s\n", currentuser.userid, timestr, i
		fprintf(fp, "%s(%s)[%d/%d]\xa3\xba%s\n", currentuser.userid, timestr, i
				+ 1, count, buf[i]);
	}
	fclose(fp);
	sethomefile(genbuf, uid, "HaveNewWish");
	if ((fp = fopen(genbuf, "w+")) != NULL) {
		fputs("Have New Wish", fp);
		fclose(fp);
	}
	move(11 + count, 0);
	//% prints("已经帮您送出您的留言了。");
	prints("\xd2\xd1\xbe\xad\xb0\xef\xc4\xfa\xcb\xcd\xb3\xf6\xc4\xfa\xb5\xc4\xc1\xf4\xd1\xd4\xc1\xcb\xa1\xa3");
	sprintf(genbuf, "SendGoodWish to %s", uid);
	report(genbuf, currentuser.userid);
	pressanykey();
	clear();
	return 0;
}

//      发送留言
int sendgoodwish(char *uid) {
	return sendGoodWish(NULL);
}
