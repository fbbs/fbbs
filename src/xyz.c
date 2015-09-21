#include <sys/wait.h>
#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

#define NUMDEFINES 31
/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
 depending on how/if you use them. */
static const char *user_definestr[] = {
	"呼叫器关闭时可让好友呼叫", /* DEF_FRIENDCALL */
	"接受所有人的讯息", /* DEF_ALLMSG */
	"接受好友的讯息", /* DEF_FRIENDMSG */
	"收到讯息发出声音", /* DEF_SOUNDMSG */
	"使用彩色", /* DEF_COLOR */
	"显示活动看板", /* DEF_ACBOARD */
	"显示选单的讯息栏", /* DEF_ENDLINE */
	"编辑时显示状态栏", /* DEF_EDITMSG */
	"讯息栏采用一般/精简模式", /* DEF_NOTMSGFRIEND */
	"选单采用一般/精简模式", /* DEF_NORMALSCR */
	"分类讨论区以 New 显示", /* DEF_NEWPOST */
	"阅读文章是否使用绕卷选择", /* DEF_CIRCLE */
	"阅读文章游标停於第一篇未读", /* DEF_FIRSTNEW */
	"进站时显示好友名单", /* DEF_LOGFRIEND */
	"好友上站通知", /* DEF_LOGINFROM */
	"观看留言板", /* DEF_NOTEPAD*/
	"不要送出上站通知给好友", /* DEF_NOLOGINSEND */
	"主题式看版", /* DEF_THESIS */
	"收到讯息等候回应或清除", /* DEF_MSGGETKEY */
	"进站时观看上站人次图", /* DEF_GRAPH */
	"进站时观看十大排行榜", /* DEF_TOP10 */
	"使用乱数签名档", /* DEF_RANDSIGN */
	"显示星座", /* DEF_S_HOROSCOPE */
	"星座使用颜色来显示性别", /* DEF_COLOREDSEX */
	"使用\'+\'标记未读文章", /* DEF_NOT_N_MASK */
	"汉字整字删除", /* DEF_DELDBLCHAR */
	"自动排版宽度预设为 78 列", /* DEF_AUTOWRAP */
	"使用GB码阅读", /* DEF_USEGB KCN 99.09.03 */
	"不隐藏自己的 IP", /* DEF_NOTHIDEIP */
	"好友离站通知", /* DEF_LOGOFFMSG Amigo 2002.04.03 */
	"使用多丝路(版主权限有效)", /* DEF_MULTANNPATH*/
	NULL
};

/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
 depending on how/if you use them. */
/* skyo.0507 modify 加入後面的 PERM 方便跟 menu.ini 对照） */
static const char *permstrings[] = {
	"上站权力       (PERM_LOGIN)", /* PERM_LOGIN */
	"与他人聊天     (TALK)", /* PERM_TALK */
	"发送信件       (MAIL)", /* PERM_MAIL */
	"发表文章       (POST)", /* PERM_POST */
	"使用者资料正确 (REGISTER)", /* PERM_REGISTER*/
	"绑定邮箱       (BINDMAIL)", /* PERM_BINDMAIL */
	"版主           (BOARDS)", /* PERM_BOARDS */
	"讨论区总管     (OBOARDS)", /* PERM_OBOARDS */
	"俱乐部总管     (OCLUB)", /* PERM_OCLUB */
	"精华区总管     (ANNOUNCE)", /* PERM_ANNOUNCE*/
	"活动看版总管   (OCBOARD)", /* PERM_OCBOARD */
	"帐号管理员     (USER)", /* PERM_USER*/
	"聊天室管理员   (OCHAT)", /* PERM_OCHAT*/
	"系统维护管理员 (SYSOPS)", /* PERM_SYSOPS*/
	"隐身术         (CLOAK)", /* PERM_CLOAK */
	"看穿隐身术     (SEECLOAK)", /* PERM_SEECLOAK */
	"帐号永久保留   (XEMPT)", /* PERM_XEMPT */
	"生命值增强权限 (LONGLIFE)", /* PERM_LONGLIFE */
	"大信箱         (LARGEMAIL)", /* PERM_LARGEMAIL*/
	"仲裁组         (ARBI)", /* PERM_ARBI*/
	"服务组         (SERV)", /* PERM_SERV*/
	"技术组         (TECH)", /* PERM_TECH*/
	"特殊权限 0     (SPECIAL0)", /* PERM_SPECIAL0*/
	"特殊权限 1     (SPECIAL1)", /* PERM_SPECIAL1*/
	"特殊权限 2     (SPECIAL2)", /* PERM_SPECIAL2*/
	"特殊权限 3     (SPECIAL3)", /* PERM_SPECIAL3*/
	"特殊权限 4     (SPECIAL4)", /* PERM_SPECIAL4*/
	"特殊权限 5     (SPECIAL5)", /* PERM_SPECIAL5*/
	"特殊权限 6     (SPECIAL6)", /* PERM_SPECIAL6*/
	"特殊权限 7     (SPECIAL7)", /* PERM_SPECIAL7*/
	"特殊权限 8     (SPECIAL8)", /* PERM_SPECIAL8*/
	"特殊权限 9     (SPECIAL9)", /* PERM_SPECIAL9*/
	NULL
};

int use_define = 0;
int child_pid = 0;

//      对于权限定义值,判断其第i位是否为真,并根据use_define的值来
//      调整其对应位的权限显示字符串
//      最后在由i指示的位置处显示,更新
int showperminfo(int pbits, int i)
{
	int line = i < 16 ? i + 6 : i - 10;
	int col = i < 16 ? 0 : 40;
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "%c. %s", 'A' + i,
			(use_define) ? user_definestr[i] : permstrings[i]);
	screen_replace(line, col, buf);
	screen_replace(line, col + 34, (pbits >> i) & 1 ? "是" : "否");
	return true;
}

//      更改用户的权限设定
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) ())
{
	int lastperm = numbers - 1;
	int i, done = NA;
	char choice[3], buf[80];
	screen_move(4, 0);
	screen_printf("\033[m请按下您要的代码来设定%s，按 Enter 结束.\n", prompt);
	screen_move(6, 0);
	screen_clrtobot();

	for (int j = 0; j < 16 && j < numbers; ++j) {
		showfunc(pbits, j, false);
		if (j + 16 < numbers) {
			tui_repeat_char(' ', 4);
			showfunc(pbits, j + 16, false);
		}
	}
	while (!done) {
		//% sprintf(buf, "选择(ENTER 结束%s): ",
		sprintf(buf, "\xd1\xa1\xd4\xf1(ENTER \xbd\xe1\xca\xf8%s): ",
				//% ((strcmp(prompt, "权限") != 0)) ? "" : "，0 停权");
				((strcmp(prompt, "\xc8\xa8\xcf\xde") != 0)) ? "" : "\xa3\xac""0 \xcd\xa3\xc8\xa8");
		getdata(-1, 0, buf, choice, 2, DOECHO, YEA);
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
		screen_move_clear(3);
		screen_printf("错误的使用者 ID...");
		pressreturn();
		screen_clear();
		return 0;
	}
	screen_move(1, 0);
	screen_clrtobot();
	screen_move(2, 0);
	use_define = 1;
	newlevel = setperms(lookupuser.userdefine, "参数", NUMDEFINES,
			showperminfo);
	screen_move(2, 0);
	if (newlevel == lookupuser.userdefine)
		screen_printf("参数没有修改...\n");
	else {
		lookupuser.userdefine = newlevel;
		currentuser.userdefine = newlevel;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		screen_printf("新的参数设定完成...\n\n");
	}
	pressreturn();
	screen_clear();
	use_define = 0;
	return 0;
}

int x_cloak(void)
{
	bool visible = session_toggle_visibility();
	report("toggle cloak", currentuser.userid);

	screen_move_clear(-1);
	//% "隐身术已经 %s 了!" "停止" "启动"
	prints("\xd2\xfe\xc9\xed\xca\xf5\xd2\xd1\xbe\xad %s \xc1\xcb!",
			visible ? "\xcd\xa3\xd6\xb9" : "\xc6\xf4\xb6\xaf");
	pressreturn();
	return 0;
}

//修改用户的档案
void x_edits() {
	char ans[7], buf[STRLEN];
	int ch, num, confirm;

	const char *e_file[] = {
		"plans", "signatures", "notes", "logout", NULL
	};

	//% "个人说明档", "签名档", "自己的备忘录", "离站的画面",
	//% "底部流动信息"
	const char *explain_file[] = {
		"\xb8\xf6\xc8\xcb\xcb\xb5\xc3\xf7\xb5\xb5",
		"\xc7\xa9\xc3\xfb\xb5\xb5",
		"\xd7\xd4\xbc\xba\xb5\xc4\xb1\xb8\xcd\xfc\xc2\xbc",
		"\xc0\xeb\xd5\xbe\xb5\xc4\xbb\xad\xc3\xe6",
		"\xb5\xd7\xb2\xbf\xc1\xf7\xb6\xaf\xd0\xc5\xcf\xa2",
		NULL
	};

	set_user_status(ST_GMENU);
	screen_clear();
	screen_move(1, 0);
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
	screen_move(3, 0);
	screen_clrtobot();
	//% sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
	sprintf(buf, "(E)\xb1\xe0\xbc\xad (D)\xc9\xbe\xb3\xfd %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		//% confirm = askyn("您确定要删除这个档案", NA, NA);
		confirm = askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xc9\xbe\xb3\xfd\xd5\xe2\xb8\xf6\xb5\xb5\xb0\xb8", NA, NA);
		if (confirm != 1) {
			screen_move(5, 0);
			//% prints("取消删除行动\n");
			prints("\xc8\xa1\xcf\xfb\xc9\xbe\xb3\xfd\xd0\xd0\xb6\xaf\n");
			pressreturn();
			screen_clear();
			return;
		}
		unlink(genbuf);
		screen_move(5, 0);
		//% prints("%s 已删除\n", explain_file[ch]);
		prints("%s \xd2\xd1\xc9\xbe\xb3\xfd\n", explain_file[ch]);
		sprintf(buf, "delete %s", explain_file[ch]);
		report(buf, currentuser.userid);
		pressreturn();
		screen_clear();
		return;
	}
	set_user_status(ST_EDITUFILE);
	editor_e status = editor(genbuf, false, false, true, NULL);
	screen_clear();
	if (status == EDITOR_SAVE) {
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
}

int x_lockscreen(void)
{
	set_user_status(ST_LOCKSCREEN);

	screen_move(9, 0);
	screen_clrtobot();
	screen_move(9, 0);
	prints("\033[1;37m"
			"\n       _       _____   ___     _   _   ___     ___       __"
			"\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |"
			"\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |"
			"\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |"
			"\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|"
			"\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|\n"
			//% "\n\033[1;36m屏幕已在\033[33m %s\033[36m 时被%s暂时锁住了...\033[m",
			"\n\033[1;36m\xc6\xc1\xc4\xbb\xd2\xd1\xd4\xda\033[33m %s\033[36m \xca\xb1\xb1\xbb%s\xd4\xdd\xca\xb1\xcb\xf8\xd7\xa1\xc1\xcb...\033[m",
			format_time(fb_time(), TIME_FORMAT_ZH), currentuser.userid);

	char buf[PASSLEN + 1];
	buf[0] = '\0';
	while (*buf == '\0' || !passwd_check(currentuser.userid, buf)) {
		screen_move(18, 0);
		screen_clrtobot();
		//% getdata(19, 0, "请输入您的密码以解锁: ", buf, PASSLEN, NOECHO, YEA);
		getdata(19, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xc3\xdc\xc2\xeb\xd2\xd4\xbd\xe2\xcb\xf8: ", buf, PASSLEN, NOECHO, YEA);
	}
	return FULLUPDATE;
}

//#define MY_DEBUG
//  执行命令cmdfile,参数为param1
static void exec_cmd(int umode, int pager, char *cmdfile, char *param1) {
	char buf[160];
	char *my_argv[18], *ptr;

	fb_signal(SIGALRM, SIG_IGN);
	set_user_status(umode);
	screen_clear();
	screen_move(2, 0);
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
	screen_clear();
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

	char date[5], index[5], buf[80], msg[29];
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
