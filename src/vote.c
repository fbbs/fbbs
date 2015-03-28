#include <signal.h>
#include "bbs.h"
#include "vote.h"
#include "mmap.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/cfg.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/pool.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

//% static const char *vote_type[] = { "是非", "单选", "复选", "数字", "问答" };
static const char *vote_type[] = { "\xca\xc7\xb7\xc7", "\xb5\xa5\xd1\xa1", "\xb8\xb4\xd1\xa1", "\xca\xfd\xd7\xd6", "\xce\xca\xb4\xf0" };

static struct votebal currvote; //当前投票
static char controlfile[STRLEN];
static unsigned int result[33]; //投票结果数组

static int vnum;
static int voted_flag;
static FILE *sug; //投票结果的文件指针
int makevote(struct votebal *ball, const char *bname); //设置投票箱

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
#define BBS_PAGESIZE    (screen_lines() - 4)

static int range, page, readplan;

// add by Flier - 2000.5.12 - Begin
enum sort_type {stUserID, stUserName, stIP, stState} st = stUserID;
// add by Flier - 2000.5.12 - End

void show_message(const char *msg)
{
	screen_move_clear(BBS_PAGESIZE + 2);
	if (msg)
		prints("\033[1m%s\033[m", msg);
	screen_flush();
}

static void setlistrange(int i)
{
	range = i;
}

static int choose(int update, int defaultn, void (*title_show)(),
		int (*key_deal)(), int (*list_show)(), int (*read)())
{
	int num = 0;
	int ch, number, deal;
	readplan = NA;
	(*title_show) ();
	fb_signal(SIGALRM, SIG_IGN);
	page = -1;
	number = 0;
	num = defaultn;
	while (1) {
		if (num <= 0)
		num = 0;
		if (num >= range)
		num = range - 1;
		if (page < 0) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			move(3, 0);
			screen_clrtobot();
			if ((*list_show) () == -1)
			return -1;
			tui_update_status_line();
		}
		if (num < page || num >= page + BBS_PAGESIZE) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			if ((*list_show) () == -1)
			return -1;
			tui_update_status_line();
			continue;
		}
		if (readplan == YEA) {
			if ((*read) (page, num) == -1)
			return num;
		} else {
			move(3 + num - page, 0);
			prints(">", number);
		}
		ch = egetch();
		if (readplan == NA)
		move(3 + num - page, 0);
		prints(" ");
		if (ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF) {
			if (readplan == YEA) {
				readplan = NA;
				move(1, 0);
				screen_clrtobot();
				if ((*list_show) () == -1)
				return -1;
				(*title_show) ();
				continue;
			}
			break;
		}
		deal = (*key_deal) (ch, num, page);
		if (range == 0)
		break;
		if (deal == 1)
		continue;
		else if (deal == -1)
		break;
		switch (ch) {
			case 'b':
			case Ctrl('B'):
			case KEY_PGUP:
			if (num == 0)
			num = range - 1;
			else
			num -= BBS_PAGESIZE;
			break;
			case ' ':
			if (readplan == YEA) {
				if (++num >= range)
				num = 0;
				break;
			}
			case 'N':
			case Ctrl('F'):
			case KEY_PGDN:
			if (num == range - 1)
			num = 0;
			else
			num += BBS_PAGESIZE;
			break;
			case 'p':
			case 'l':
			case KEY_UP:
			if (num-- <= 0)
			num = range - 1;
			break;
			case 'n':
			case 'j':
			case KEY_DOWN:
			if (++num >= range)
			num = 0;
			break;
			case '$':
			case KEY_END:
			num = range - 1;
			break;
			case KEY_HOME:
			num = 0;
			break;
			case '\n':
			case '\r':
			if (number> 0) {
				num = number - 1;
				break;
			}
			/* fall through */
			case KEY_RIGHT:
			{
				if (readplan == YEA) {
					if (++num >= range)
					num = 0;
				} else
				readplan = YEA;
				break;
			}
			default:
			;
		}
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
	}
	fb_signal(SIGALRM, SIG_IGN);
	return -1;
}

//commented by jacobson

//本文件主要处理投票功能

//比较字符串userid和投票者uv 
//userid:用户名 uv:投票者 
//返回值:0不等， 1相等
static int cmpvuid(void *userid, void *uv)
{
	return !strcmp((char *)userid, ((struct ballot *)uv)->uid);
}

//设置版面投票的标志,           
//bname:版面名,flag版面标志
//1:开启投票,0:关闭投票 返回值:无..
static int setvoteflag(const char *bname, int flag)
{
	board_t board;
	get_board(bname, &board);

	if (flag == 0)
		board.flag = board.flag & ~BOARD_FLAG_VOTE;
	else
		board.flag = board.flag | BOARD_FLAG_VOTE;

	db_res_t *res = db_cmd("UPDATE boards SET flag = %d WHERE id = %d",
			board.flag, board.id);
	db_clear(res);
	if (!res)
		prints("Error updating BOARDS file...\n");
	return 0;
}

//显示bug报告(目前好像没有实现)
//str:错误信息字符串
void b_report(char *str) {
	char buf[STRLEN];

	sprintf(buf, "%s %s", currboard, str);
	report(buf, currentuser.userid);
}

//建立目录,目录为 vote/版名,权限为755
//bname:版面名字
void makevdir(const char *bname)
{
	struct stat st;
	char buf[STRLEN];

	sprintf(buf, "vote/%s", bname);
	if (stat(buf, &st) != 0)
		mkdir(buf, 0755);
}

//设置文件名
//bname：版面名
//filename:文件名
//buf:返回的文件名
void setvfile(char *buf, const char *bname, const char *filename)
{
	sprintf(buf, "vote/%s/%s", bname, filename);
}

//设置控制controlfile文件名为 vote\版面名\control
static void setcontrolfile(const char *bname)
{
	setvfile(controlfile, bname, "control");
}

//编辑或删除版面备忘录
//返回值:FULLUPDATE
#ifdef ENABLE_PREFIX
int b_notes_edit()
{
	char buf[STRLEN], buf2[STRLEN];
	char ans[4];
	int aborted;
	int notetype;

	if (!am_curr_bm())
		return 0;
	screen_clear();
	move(0, 0);
	//% prints("设定：\n\n  (1)一般备忘录\n  (2)秘密备忘录\n");
	prints("\xc9\xe8\xb6\xa8\xa3\xba\n\n  (1)\xd2\xbb\xb0\xe3\xb1\xb8\xcd\xfc\xc2\xbc\n  (2)\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\n");
	//% prints("  (3)版面前缀表\n  (4)是否强制使用前缀\n");
	prints("  (3)\xb0\xe6\xc3\xe6\xc7\xb0\xd7\xba\xb1\xed\n  (4)\xca\xc7\xb7\xf1\xc7\xbf\xd6\xc6\xca\xb9\xd3\xc3\xc7\xb0\xd7\xba\n");
	while (1) {
		//% getdata(7, 0,"当前选择[1](0~4): ", ans, 2, DOECHO, YEA);
		getdata(7, 0,"\xb5\xb1\xc7\xb0\xd1\xa1\xd4\xf1[1](0~4): ", ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
		return FULLUPDATE;
		if (ans[0] == '\0')
		strcpy(ans, "1");
		if (ans[0] >= '1' && ans[0] <= '4' )
		break;
	}
	makevdir(currboard); //建立备忘录目录
	notetype = ans[0] - '0';
	if (notetype == 2) {
		setvfile(buf, currboard, "secnotes");
	} else if (notetype == 3) {
		setvfile(buf, currboard, "prefix");
	} else if (notetype == 1) {
		setvfile(buf, currboard, "notes");
	} else if (notetype == 4 ) {
		int flag = currbp->flag;
		//% if (askyn("是否强制使用前缀？",
		if (askyn("\xca\xc7\xb7\xf1\xc7\xbf\xd6\xc6\xca\xb9\xd3\xc3\xc7\xb0\xd7\xba\xa3\xbf",
					(currbp->flag & BOARD_FLAG_PREFIX) ? YEA : NA, NA)) {
			flag |= BOARD_FLAG_PREFIX;
		} else {
			flag &= ~BOARD_FLAG_PREFIX;
		}
		db_res_t *res = db_cmd("UPDATE boards SET flag = %d WHERE id = %d",
				flag, currbp->id);
		db_clear(res);
		return FULLUPDATE;
	}
	//% sprintf(buf2, "(E)编辑 (D)删除 %4s? [E]: ",
	sprintf(buf2, "(E)\xb1\xe0\xbc\xad (D)\xc9\xbe\xb3\xfd %4s? [E]: ",
			//% (notetype == 3)?"版面前缀表":(notetype == 1) ? "一般备忘录" : "秘密备忘录");
			(notetype == 3)?"\xb0\xe6\xc3\xe6\xc7\xb0\xd7\xba\xb1\xed":(notetype == 1) ? "\xd2\xbb\xb0\xe3\xb1\xb8\xcd\xfc\xc2\xbc" : "\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc");
	getdata(8, 0, buf2, ans, 2, DOECHO, YEA); //询问编辑或者删除
	if (ans[0] == 'D' || ans[0] == 'd') { //删除备忘录
		move(9, 0);
		//% sprintf(buf2, "真的要删除么？");
		sprintf(buf2, "\xd5\xe6\xb5\xc4\xd2\xaa\xc9\xbe\xb3\xfd\xc3\xb4\xa3\xbf");
		if (askyn(buf2, NA, NA)) {
			move(10, 0);
			//% prints("已经删除...\n");
			prints("\xd2\xd1\xbe\xad\xc9\xbe\xb3\xfd...\n");
			pressanykey();
			unlink(buf);
			aborted = 1;
		} else
		aborted = -1;
	} else
	if (editor(buf, false, false, true, NULL) != EDITOR_SAVE) {
		pressreturn();
	} else {
		if (notetype == 1)
		setvfile(buf, currboard, "noterec");
		else
		setvfile(buf, currboard, "notespasswd");
		unlink(buf);
	}

	return FULLUPDATE;
}
#else
int b_notes_edit() {
	char buf[STRLEN], buf2[STRLEN];
	char ans[4];
	int notetype;
	if (!am_curr_bm())
		return 0;
	screen_clear();
	move(1, 0);
	//% prints("编辑/删除备忘录");
	prints("\xb1\xe0\xbc\xad/\xc9\xbe\xb3\xfd\xb1\xb8\xcd\xfc\xc2\xbc");
	while (1) {
		//% getdata(3, 0, "编辑或删除本讨论区的 (0) 离开  (1) 一般备忘录  (2) 秘密备忘录? [1] ",
		getdata(3, 0, "\xb1\xe0\xbc\xad\xbb\xf2\xc9\xbe\xb3\xfd\xb1\xbe\xcc\xd6\xc2\xdb\xc7\xf8\xb5\xc4 (0) \xc0\xeb\xbf\xaa  (1) \xd2\xbb\xb0\xe3\xb1\xb8\xcd\xfc\xc2\xbc  (2) \xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc? [1] ",
				ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
			return FULLUPDATE;
		if (ans[0] == '\0')
			strcpy(ans, "1");
		if (ans[0] == '1' || ans[0] == '2')
			break;
	}
	makevdir(currboard); //建立备忘录目录
	if (ans[0] == '2') {
		setvfile(buf, currboard, "secnotes");
		notetype = 2;
	} else {
		setvfile(buf, currboard, "notes");
		notetype = 1;
	}
	//% sprintf(buf2, "(E)编辑 (D)删除 %4s备忘录? [E]: ", (notetype == 1) ? "一般"
	sprintf(buf2, "(E)\xb1\xe0\xbc\xad (D)\xc9\xbe\xb3\xfd %4s\xb1\xb8\xcd\xfc\xc2\xbc? [E]: ", (notetype == 1) ? "\xd2\xbb\xb0\xe3"
			//% : "秘密");
			: "\xc3\xd8\xc3\xdc");
	getdata(5, 0, buf2, ans, 2, DOECHO, YEA); //询问编辑或者删除
	editor_e status;
	if (ans[0] == 'D' || ans[0] == 'd') { //删除备忘录
		move(6, 0);
		//% sprintf(buf2, "真的要删除%4s备忘录", (notetype == 1) ? "一般" : "秘密");
		sprintf(buf2, "\xd5\xe6\xb5\xc4\xd2\xaa\xc9\xbe\xb3\xfd%4s\xb1\xb8\xcd\xfc\xc2\xbc", (notetype == 1) ? "\xd2\xbb\xb0\xe3" : "\xc3\xd8\xc3\xdc");
		if (askyn(buf2, NA, NA)) {
			move(7, 0);
			//% prints("备忘录已经删除...\n");
			prints("\xb1\xb8\xcd\xfc\xc2\xbc\xd2\xd1\xbe\xad\xc9\xbe\xb3\xfd...\n");
			pressanykey();
			unlink(buf);
			status = EDITOR_SAVE;
		} else
			status = EDITOR_ABORT;
	} else
		status = editor(buf, false, false, true, NULL); //编辑备忘录
	if (status != EDITOR_SAVE) {
		pressreturn();
	} else {
		if (notetype == 1)
			setvfile(buf, currboard, "noterec");
		else
			setvfile(buf, currboard, "notespasswd");
		unlink(buf);
	}

	return FULLUPDATE;
}
#endif 
//设置秘密备忘录密码
int b_notes_passwd(void)
{
	FILE *pass;
	char passbuf[20], prepass[20];
	char buf[STRLEN];

	if (!am_curr_bm())
		return 0;
	screen_clear();
	move(1, 0);
	//% prints("设定/更改/取消「秘密备忘录」密码...");
	prints("\xc9\xe8\xb6\xa8/\xb8\xfc\xb8\xc4/\xc8\xa1\xcf\xfb\xa1\xb8\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xa1\xb9\xc3\xdc\xc2\xeb...");
	setvfile(buf, currboard, "secnotes");
	if (!dashf(buf)) {
		move(3, 0);
		//% prints("本讨论区尚无「秘密备忘录」。\n\n");
		prints("\xb1\xbe\xcc\xd6\xc2\xdb\xc7\xf8\xc9\xd0\xce\xde\xa1\xb8\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xa1\xb9\xa1\xa3\n\n");
		//% prints("请先用 W 编好「秘密备忘录」再来设定密码...");
		prints("\xc7\xeb\xcf\xc8\xd3\xc3 W \xb1\xe0\xba\xc3\xa1\xb8\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xa1\xb9\xd4\xd9\xc0\xb4\xc9\xe8\xb6\xa8\xc3\xdc\xc2\xeb...");
		pressanykey();
		return FULLUPDATE;
	}
	if (!check_notespasswd())
		return FULLUPDATE;
	//% getdata(3, 0, "请输入新的秘密备忘录密码(Enter 取消密码): ", passbuf, 19, NOECHO, YEA);
	getdata(3, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd0\xc2\xb5\xc4\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb(Enter \xc8\xa1\xcf\xfb\xc3\xdc\xc2\xeb): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		setvfile(buf, currboard, "notespasswd");
		unlink(buf);
		//% prints("已经取消备忘录密码。");
		prints("\xd2\xd1\xbe\xad\xc8\xa1\xcf\xfb\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb\xa1\xa3");
		pressanykey();
		return FULLUPDATE;
	}
	//% getdata(4, 0, "确认新的秘密备忘录密码: ", prepass, 19, NOECHO, YEA);
	getdata(4, 0, "\xc8\xb7\xc8\xcf\xd0\xc2\xb5\xc4\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		//% prints("\n密码不相符, 无法设定或更改....");
		prints("\n\xc3\xdc\xc2\xeb\xb2\xbb\xcf\xe0\xb7\xfb, \xce\xde\xb7\xa8\xc9\xe8\xb6\xa8\xbb\xf2\xb8\xfc\xb8\xc4....");
		pressanykey();
		return FULLUPDATE;
	}
	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "w")) == NULL) {
		move(5, 0);
		//% prints("备忘录密码无法设定....");
		prints("\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb\xce\xde\xb7\xa8\xc9\xe8\xb6\xa8....");
		pressanykey();
		return FULLUPDATE;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(5, 0);
	//% prints("秘密备忘录密码设定完成...");
	prints("\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb\xc9\xe8\xb6\xa8\xcd\xea\xb3\xc9...");
	pressanykey();
	return FULLUPDATE;
}

//将一个文件全部内容写入已经打开的另一个文件
//fp: 已经打开的文件指针,（被写入文件）
//fname: 需要写入的文件的路径
int b_suckinfile(FILE * fp, char *fname) {
	char inbuf[256];
	FILE *sfp;

	if ((sfp = fopen(fname, "r")) == NULL)
		return -1;
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
		fputs(inbuf, fp);
	fclose(sfp);
	return 0;
}

//将一个文件全部内容写入已经打开的另一个文件,(用于读留言板)
//如果不能打开写入一条横线
//fp: 已经打开的文件指针,（被写入文件）
//fname: 需要写入的文件的路径
int catnotepad(FILE *fp, const char *fname)
{
	char inbuf[256];
	FILE *sfp;
	int count;

	count = 0;
	if ((sfp = fopen(fname, "r")) == NULL) {
		fprintf(fp,
				//% "\033[1;34m  □\033[44m_______________________________________"
				"\033[1;34m  \xa1\xf5\033[44m_______________________________________"
				"___________________________________\033[m \n\n");
		return -1;
	}
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL) {
		if (count != 0)
		fputs(inbuf, fp);
		else
		count++;
	}
	fclose(sfp);
	return 0;
}

//计算一次的投票结果,并放入result数组中,用于mk_result中的apply_record函数中的回调函数 -.-!
//result[32]记录调用次数
//参数ptr:一次的投票结果
//返回值:固定为0
// The 'notused1' and 'notused2' arguemnts are not used,
// just to comply with function prototype.
static int count_result(void *ptrv, int notused1, void *notused2)
{
	int i;
	if (ptrv == NULL)
		return -1;
	struct ballot *ptr = (struct ballot *)ptrv;
	if (ptr->msg[0][0] != '\0') {
		if (currvote.type == VOTE_ASKING) {
			//% fprintf(sug, "[1m%s [m的作答如下：\n", ptr->uid);
			fprintf(sug, "[1m%s [m\xb5\xc4\xd7\xf7\xb4\xf0\xc8\xe7\xcf\xc2\xa3\xba\n", ptr->uid);
		} else
			//% fprintf(sug, "[1m%s [m的建议如下：\n", ptr->uid);
			fprintf(sug, "[1m%s [m\xb5\xc4\xbd\xa8\xd2\xe9\xc8\xe7\xcf\xc2\xa3\xba\n", ptr->uid);
		for (i = 0; i < 3; i++)
			fprintf(sug, "%s\n", ptr->msg[i]);
	}
	result[32]++;
	if (currvote.type == VOTE_ASKING) {
		return 0;
	}
	if (currvote.type != VOTE_VALUE) {
		for (i = 0; i < 32; i++) {
			if ((ptr->voted >> i) & 1)
				(result[i])++;
		}

	} else {
		result[31] += ptr->voted;
		result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
	}
	return 0;
}

//将投票的抬头写入sug投票结果文件
static void get_result_title(void)
{
	char buf[STRLEN];

	//% fprintf(sug, "⊙ 投票开启于：\033[1m%s\033[m  类别：\033[1m%s\033[m\n",
	fprintf(sug, "\xa1\xd1 \xcd\xb6\xc6\xb1\xbf\xaa\xc6\xf4\xd3\xda\xa3\xba\033[1m%s\033[m  \xc0\xe0\xb1\xf0\xa3\xba\033[1m%s\033[m\n",
			format_time(currvote.opendate, TIME_FORMAT_ZH),
			vote_type[currvote.type - 1]);
	//% fprintf(sug, "⊙ 主题：[1m%s[m\n", currvote.title);
	fprintf(sug, "\xa1\xd1 \xd6\xf7\xcc\xe2\xa3\xba[1m%s[m\n", currvote.title);
	if (currvote.type == VOTE_VALUE)
		//% fprintf(sug, "⊙ 此次投票的值不可超过：[1m%d[m\n\n", currvote.maxtkt);
		fprintf(sug, "\xa1\xd1 \xb4\xcb\xb4\xce\xcd\xb6\xc6\xb1\xb5\xc4\xd6\xb5\xb2\xbb\xbf\xc9\xb3\xac\xb9\xfd\xa3\xba[1m%d[m\n\n", currvote.maxtkt);
	//% fprintf(sug, "⊙ 票选题目描述：\n\n");
	fprintf(sug, "\xa1\xd1 \xc6\xb1\xd1\xa1\xcc\xe2\xc4\xbf\xc3\xe8\xca\xf6\xa3\xba\n\n");
	sprintf(buf, "vote/%s/desc.%d", currboard, currvote.opendate);
	b_suckinfile(sug, buf);
}

//删除投票文件
//num 投票controlfile中第几个记录
//返回值 无
int dele_vote(int num)
{
	char buf[STRLEN];

	sprintf(buf, "vote/%s/flag.%d", currboard, currvote.opendate);
	unlink(buf);
	sprintf(buf, "vote/%s/desc.%d", currboard, currvote.opendate);
	unlink(buf);
	if (delete_record(controlfile, sizeof(currvote), num, NULL, NULL) == -1) {
		//% prints("发生错误，请通知站长....");
		prints("\xb7\xa2\xc9\xfa\xb4\xed\xce\xf3\xa3\xac\xc7\xeb\xcd\xa8\xd6\xaa\xd5\xbe\xb3\xa4....");
		pressanykey();
	}
	range--;
	if (get_num_records(controlfile, sizeof(currvote)) == 0) {
		setvoteflag(currboard, 0);
	}
	return 0;
}

//结束投票,计算投票结果
//num:投票control文件中第几个记录
int mk_result(int num)
{
	char fname[STRLEN], nname[STRLEN];
	char sugname[STRLEN];
	char title[STRLEN];
	int i;
	unsigned int total = 0;

	setcontrolfile(currboard);
	sprintf(fname, "vote/%s/flag.%d", currboard, currvote.opendate); //投票记录文件路径为 vote/版名/flag.开启投票日
	/*	count_result(NULL); */
	sug = NULL;
	sprintf(sugname, "vote/%s/tmp.%d", currboard, session_pid()); //投票临时文件路径为 vote/版名/tmp.用户id
	if ((sug = fopen(sugname, "w")) == NULL) {
		report("open vote tmp file error", currentuser.userid);
		pressanykey();
	}
	(void) memset(result, 0, sizeof(result));
	if (apply_record(fname, count_result, sizeof(struct ballot), NULL, 0, 0, true)
			== -1) {
		report("Vote apply flag error", currentuser.userid);
	}
	//% fprintf(sug, "[1;44;36m——————————————┤使用者%s├——————————————[m\n\n\n",
	fprintf(sug, "[1;44;36m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8\xca\xb9\xd3\xc3\xd5\xdf%s\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[m\n\n\n",
			//% (currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
			(currvote.type != VOTE_ASKING) ? "\xbd\xa8\xd2\xe9\xbb\xf2\xd2\xe2\xbc\xfb" : "\xb4\xcb\xb4\xce\xb5\xc4\xd7\xf7\xb4\xf0");
	fclose(sug);
	sprintf(nname, "vote/%s/results", currboard); //投票结果文件路径为 vote/版名/results
	if ((sug = fopen(nname, "w")) == NULL) {
		report("open vote newresult file error", currentuser.userid);
	}
	get_result_title();
	//计算投票结果
	//% fprintf(sug, "** 投票结果:\n\n");
	fprintf(sug, "** \xcd\xb6\xc6\xb1\xbd\xe1\xb9\xfb:\n\n");
	if (currvote.type == VOTE_VALUE) {
		total = result[32];
		for (i = 0; i < 10; i++) {
			fprintf(
					sug,
					//% "[1m  %4d[m 到 [1m%4d[m 之间有 [1m%4d[m 票  约占 [1m%d%%[m\n",
					"[1m  %4d[m \xb5\xbd [1m%4d[m \xd6\xae\xbc\xe4\xd3\xd0 [1m%4d[m \xc6\xb1  \xd4\xbc\xd5\xbc [1m%d%%[m\n",
					(i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1), ((i
							+ 1) * currvote.maxtkt) / 10, result[i],
					(result[i] * 100) / ((total <= 0) ? 1 : total));
		}
		//% fprintf(sug, "此次投票结果平均值是: [1m%d[m\n", result[31]
		fprintf(sug, "\xb4\xcb\xb4\xce\xcd\xb6\xc6\xb1\xbd\xe1\xb9\xfb\xc6\xbd\xbe\xf9\xd6\xb5\xca\xc7: [1m%d[m\n", result[31]
				/ ((total <= 0) ? 1 : total));
	} else if (currvote.type == VOTE_ASKING) {
		total = result[32];
	} else {
		for (i = 0; i < currvote.totalitems; i++) {
			total += result[i];
		}
		for (i = 0; i < currvote.totalitems; i++) {
			//% fprintf(sug, "(%c) %-40s  %4d 票  约占 [1m%d%%[m\n", 'A' + i,
			fprintf(sug, "(%c) %-40s  %4d \xc6\xb1  \xd4\xbc\xd5\xbc [1m%d%%[m\n", 'A' + i,
					currvote.items[i], result[i], (result[i] * 100)
							/ ((total <= 0) ? 1 : total));
		}
	}
	//% fprintf(sug, "\n投票总人数 = [1m%d[m 人\n", result[32]);
	fprintf(sug, "\n\xcd\xb6\xc6\xb1\xd7\xdc\xc8\xcb\xca\xfd = [1m%d[m \xc8\xcb\n", result[32]);
	//% fprintf(sug, "投票总票数 =[1m %d[m 票\n\n", total);
	fprintf(sug, "\xcd\xb6\xc6\xb1\xd7\xdc\xc6\xb1\xca\xfd =[1m %d[m \xc6\xb1\n\n", total);
	//% fprintf(sug, "[1;44;36m——————————————┤使用者%s├——————————————[m\n\n\n",
	fprintf(sug, "[1;44;36m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa9\xc8\xca\xb9\xd3\xc3\xd5\xdf%s\xa9\xc0\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[m\n\n\n",
			//% (currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
			(currvote.type != VOTE_ASKING) ? "\xbd\xa8\xd2\xe9\xbb\xf2\xd2\xe2\xbc\xfb" : "\xb4\xcb\xb4\xce\xb5\xc4\xd7\xf7\xb4\xf0");
	b_suckinfile(sug, sugname);
	unlink(sugname); //删除投票临时文件,并将投票文件写入sug投票结果文件
	fclose(sug);

	//% sprintf(title, "[公告] %s 版的投票结果", currboard);
	sprintf(title, "[\xb9\xab\xb8\xe6] %s \xb0\xe6\xb5\xc4\xcd\xb6\xc6\xb1\xbd\xe1\xb9\xfb", currboard);
	Postfile(nname, "vote", title, 1); //投票结果贴入vote版
	if (strcmp(currboard, "vote"))
		Postfile(nname, currboard, title, 1); //投票结果贴入当前版
	dele_vote(num); //关闭投票,删除临时文件
	return 0;
}

int b_closepolls(void)
{
	time_t now = time(NULL);
	if (resolve_boards() < 0)
		exit(EXIT_FAILURE);

	if (now < brdshm->pollvote) {
		return 0;
	}

	time_t nextpoll = now + 7 * 3600;

	if (!config_load(DEFAULT_CFG_FILE))
		exit(EXIT_FAILURE);

	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	for (int i = 0; i < db_res_rows(res); ++i) {
		board_t board;
		res_to_board(res, i, &board);
		change_board(&board);
		setcontrolfile(board.name);
		int end = get_num_records(controlfile, sizeof(currvote));
		for (vnum = end; vnum >= 1; vnum--) {
			time_t closetime;

			get_record(controlfile, &currvote, sizeof(currvote), vnum);
			closetime = currvote.opendate + currvote.maxdays * 86400;
			if (now > closetime)
				mk_result(vnum);
			else if (nextpoll > closetime)
				nextpoll = closetime + 300;
		}
	}
	db_clear(res);

	brdshm->pollvote = nextpoll;
	return 0;
}

//取得选择题可选项目,放入bal中
//返回值 num：可选项目数
int get_vitems(struct votebal *bal) {
	int num;
	char buf[STRLEN];

	move(3, 0);
	//% prints("请依序输入可选择项, 按 ENTER 完成设定.\n");
	prints("\xc7\xeb\xd2\xc0\xd0\xf2\xca\xe4\xc8\xeb\xbf\xc9\xd1\xa1\xd4\xf1\xcf\xee, \xb0\xb4 ENTER \xcd\xea\xb3\xc9\xc9\xe8\xb6\xa8.\n");
	num = 0;
	for (num = 0; num < 32; num++) {
		sprintf(buf, "%c) ", num + 'A');
		getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36,
				DOECHO, YEA);
		if (strlen(bal->items[num]) == 0) {
			if (num != 0)
				break;
			num = -1;
		}
	}
	bal->totalitems = num;
	return num;
}

//开启投票箱并设置投票箱
//bname:版名
//返回值:固定为 FULLUPDATE
int vote_maintain(const char *bname)
{
	char buf[STRLEN];
	struct votebal *ball = &currvote;

	setcontrolfile(bname);
	if (!am_curr_bm())
		return 0;
	//% stand_title("开启投票箱");
	stand_title("\xbf\xaa\xc6\xf4\xcd\xb6\xc6\xb1\xcf\xe4");
	makevdir(bname);
	for (;;) {
		//% getdata(2, 0, "(1)是非, (2)单选, (3)复选, (4)数值 (5)问答 (6)取消 ? : ",
		getdata(2, 0, "(1)\xca\xc7\xb7\xc7, (2)\xb5\xa5\xd1\xa1, (3)\xb8\xb4\xd1\xa1, (4)\xca\xfd\xd6\xb5 (5)\xce\xca\xb4\xf0 (6)\xc8\xa1\xcf\xfb ? : ",
				genbuf, 2, DOECHO, YEA);
		genbuf[0] -= '0';
		if (genbuf[0] < 1 || genbuf[0] > 5) {
			//% prints("取消此次投票\n");
			prints("\xc8\xa1\xcf\xfb\xb4\xcb\xb4\xce\xcd\xb6\xc6\xb1\n");
			return FULLUPDATE;
		}
		ball->type = (int) genbuf[0];
		break;
	}
	ball->opendate = time(NULL);
	if (makevote(ball, bname))
		return FULLUPDATE; //设置投票箱
	setvoteflag(bname, 1);
	screen_clear();
	strcpy(ball->userid, currentuser.userid);
	if (append_record(controlfile, ball, sizeof(*ball)) == -1) {
		//% prints("发生严重的错误，无法开启投票，请通告站长");
		prints("\xb7\xa2\xc9\xfa\xd1\xcf\xd6\xd8\xb5\xc4\xb4\xed\xce\xf3\xa3\xac\xce\xde\xb7\xa8\xbf\xaa\xc6\xf4\xcd\xb6\xc6\xb1\xa3\xac\xc7\xeb\xcd\xa8\xb8\xe6\xd5\xbe\xb3\xa4");
		b_report("Append Control file Error!!");
	} else {
		char votename[STRLEN];
		int i;

		b_report("OPEN");
		//% prints("投票箱开启了！\n");
		prints("\xcd\xb6\xc6\xb1\xcf\xe4\xbf\xaa\xc6\xf4\xc1\xcb\xa3\xa1\n");
		range++;
		file_temp_name(votename, sizeof(votename));
		if ((sug = fopen(votename, "w")) != NULL) {
			strcpy(genbuf, ball->title);
			ellipsis(genbuf, 31 - strlen(bname));
			//% sprintf(buf, "[通知] %s 举办投票: %s", bname, ball->title);
			sprintf(buf, "[\xcd\xa8\xd6\xaa] %s \xbe\xd9\xb0\xec\xcd\xb6\xc6\xb1: %s", bname, ball->title);
			get_result_title();
			if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE) {
				//% fprintf(sug, "\n【[1m选项如下[m】\n");
				fprintf(sug, "\n\xa1\xbe[1m\xd1\xa1\xcf\xee\xc8\xe7\xcf\xc2[m\xa1\xbf\n");
				for (i = 0; i < ball->totalitems; i++) {
					fprintf(sug, "([1m%c[m) %-40s\n", 'A' + i,
							ball->items[i]);
				}
			}
			fclose(sug);
			Postfile(votename, "vote", buf, 1);
			unlink(votename);
		}
	}
	pressreturn();
	return FULLUPDATE;
}

//设置投票箱
//ball: 投票箱
//bname：版名
//返回值0： 正常退出 1：用户取消
int makevote(struct votebal *ball, const char *bname)
{
	char buf[STRLEN];

	//% prints("请按任何键开始编辑此次 [投票的描述]: \n");
	prints("\xc7\xeb\xb0\xb4\xc8\xce\xba\xce\xbc\xfc\xbf\xaa\xca\xbc\xb1\xe0\xbc\xad\xb4\xcb\xb4\xce [\xcd\xb6\xc6\xb1\xb5\xc4\xc3\xe8\xca\xf6]: \n");
	terminal_getchar();
	setvfile(genbuf, bname, "desc");
	sprintf(buf, "%s.%d", genbuf, ball->opendate);
	if (editor(buf, false, false, true, NULL) != EDITOR_SAVE) {
		screen_clear();
		//% prints("取消此次投票设定\n");
		prints("\xc8\xa1\xcf\xfb\xb4\xcb\xb4\xce\xcd\xb6\xc6\xb1\xc9\xe8\xb6\xa8\n");
		pressreturn();
		return 1;
	}
	screen_clear();
	//% getdata(0, 0, "此次投票所须天数 (不可０天): ", buf, 3, DOECHO, YEA);
	getdata(0, 0, "\xb4\xcb\xb4\xce\xcd\xb6\xc6\xb1\xcb\xf9\xd0\xeb\xcc\xec\xca\xfd (\xb2\xbb\xbf\xc9\xa3\xb0\xcc\xec): ", buf, 3, DOECHO, YEA);

	if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
		strcpy(buf, "1");

	ball->maxdays = atoi(buf);
	for (;;) {
		//Modified by IAMFAT 2002.06.13
		//% getdata(1, 0, "投票箱的标题: ", ball->title, 50, DOECHO, YEA);
		getdata(1, 0, "\xcd\xb6\xc6\xb1\xcf\xe4\xb5\xc4\xb1\xea\xcc\xe2: ", ball->title, 50, DOECHO, YEA);
		if (strlen(ball->title) > 0)
			break;
		bell();
	}
	switch (ball->type) {
		case VOTE_YN:
			ball->maxtkt = 0;
			//% strcpy(ball->items[0], "赞成  （是的）");
			strcpy(ball->items[0], "\xd4\xde\xb3\xc9  \xa3\xa8\xca\xc7\xb5\xc4\xa3\xa9");
			//% strcpy(ball->items[1], "不赞成（不是）");
			strcpy(ball->items[1], "\xb2\xbb\xd4\xde\xb3\xc9\xa3\xa8\xb2\xbb\xca\xc7\xa3\xa9");
			//% strcpy(ball->items[2], "没意见（不清楚）");
			strcpy(ball->items[2], "\xc3\xbb\xd2\xe2\xbc\xfb\xa3\xa8\xb2\xbb\xc7\xe5\xb3\xfe\xa3\xa9");
			ball->maxtkt = 1;
			ball->totalitems = 3;
			break;
		case VOTE_SINGLE:
			get_vitems(ball);
			ball->maxtkt = 1;
			break;
		case VOTE_MULTI:
			get_vitems(ball);
			for (;;) {
				//% getdata(21, 0, "一个人最多几票? [1]: ", buf, 5, DOECHO, YEA);
				getdata(21, 0, "\xd2\xbb\xb8\xf6\xc8\xcb\xd7\xee\xb6\xe0\xbc\xb8\xc6\xb1? [1]: ", buf, 5, DOECHO, YEA);
				ball->maxtkt = atoi(buf);
				if (ball->maxtkt <= 0)
					ball->maxtkt = 1;
				if (ball->maxtkt > ball->totalitems)
					continue;
				break;
			}
			break;
		case VOTE_VALUE:
			for (;;) {
				//% getdata(3, 0, "输入数值最大不得超过 [100] : ", buf, 4, DOECHO, YEA);
				getdata(3, 0, "\xca\xe4\xc8\xeb\xca\xfd\xd6\xb5\xd7\xee\xb4\xf3\xb2\xbb\xb5\xc3\xb3\xac\xb9\xfd [100] : ", buf, 4, DOECHO, YEA);
				ball->maxtkt = atoi(buf);
				if (ball->maxtkt <= 0)
					ball->maxtkt = 100;
				break;
			}
			break;
		case VOTE_ASKING:
			ball->maxtkt = 0;
			currvote.totalitems = 0;
			break;
		default:
			ball->maxtkt = 1;
			break;
	}
	return 0;
}

// 检查是否读过新的备忘录或者进站welcome 或者写入
// bname:版名, mode =2时设为NULL
// val:  0：检查模式    不等于0:写入模式
// mode: 1:检查备忘录   2:检查进站Welcome
// 返回值 0:未读 1:已读
int vote_flag(const char *bname, char val, int mode) {
	char buf[STRLEN], flag;
	int fd, num, size;

	num = usernum - 1;

	switch (mode) {
		case 2:
			sprintf(buf, "Welcome.rec"); /* 进站的 Welcome 画面 */
			break;
		case 1:
			setvfile(buf, bname, "noterec"); /* 讨论区备忘录的旗标 */
			break;
		default:
			return -1;
	}

	if (num >= MAXUSERS) {
		report("Vote Flag, Out of User Numbers", currentuser.userid);
		return -1;
	}

	if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) {
		return -1;
	}

	file_lock_all(fd, FILE_WRLCK);
	size = (int) lseek(fd, 0, SEEK_END);
	memset(buf, 0, sizeof(buf));
	while (size <= num) {
		file_write(fd, buf, sizeof(buf));
		size += sizeof(buf);
	}
	lseek(fd, (off_t) num, SEEK_SET);
	read(fd, &flag, 1); //读是否已经读过的标志flag
	if ((flag == 0 && val != 0)) {
		lseek(fd, (off_t) num, SEEK_SET);
		file_write(fd, &val, 1);
	}
	file_lock_all(fd, FILE_UNLCK);
	close(fd);

	return flag;
}

//检查投了几票
//bits: 32位的值
//返回值 二进制32位bits中 等于1的位数的数量
int vote_check(int bits) {
	int i, count;

	for (i = count = 0; i < 32; i++) {
		if ((bits >> i) & 1)
			count++;
	}
	return count;
}

//显示用户投过的票，以及可选项
//pbits:票数字段 i:显示位置 flag:是否显示你已经投了几票 YEA:显示 NO:不显示
//返回值:固定为YEA
int showvoteitems(unsigned int pbits, int i, int flag) {
	char buf[STRLEN];
	int count;

	if (flag == YEA) {
		count = vote_check(pbits);
		if (count > currvote.maxtkt)
			return NA;
		screen_move_clear(2);
		//% prints("您已经投了 %d 票", count);
		prints("\xc4\xfa\xd2\xd1\xbe\xad\xcd\xb6\xc1\xcb \033[1m%d\033[m \xc6\xb1", count);
	}
	//% sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i, ((pbits >> i) & 1 ? "√"
	sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i, ((pbits >> i) & 1 ? "\xa1\xcc"
			: "  "), currvote.items[i]);
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	outs(buf);
	screen_flush();
	return YEA;
}

//显示投票内容
void show_voteing_title() {
	time_t closedate;
	char buf[STRLEN];

	if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
		//% sprintf(buf, "可投票数: [1m%d[m 票", currvote.maxtkt);
		sprintf(buf, "\xbf\xc9\xcd\xb6\xc6\xb1\xca\xfd: [1m%d[m \xc6\xb1", currvote.maxtkt);
	else
		buf[0] = '\0';
	closedate = currvote.opendate + currvote.maxdays * 86400;
	//% prints("投票将结束于: \033[1m%s\033[m  %s  %s\n",
	prints("\xcd\xb6\xc6\xb1\xbd\xab\xbd\xe1\xca\xf8\xd3\xda: \033[1m%s\033[m  %s  %s\n",
			format_time(closedate, TIME_FORMAT_ZH), buf,
			//% (voted_flag) ? "(\033[5;1m修改前次投票\033[m)" : "");
			(voted_flag) ? "(\033[5;1m\xd0\xde\xb8\xc4\xc7\xb0\xb4\xce\xcd\xb6\xc6\xb1\033[m)" : "");
	//% prints("投票主题是: [1m%-50s[m类型: [1m%s[m \n", currvote.title,
	prints("\xcd\xb6\xc6\xb1\xd6\xf7\xcc\xe2\xca\xc7: [1m%-50s[m\xc0\xe0\xd0\xcd: [1m%s[m \n", currvote.title,
			vote_type[currvote.type - 1]);
}

//取得提问型投票答案
//uv:用户投票的数据,返回后用户输入的答案放在 uv->msg里,最多3行
//返回值: 用户输入的答案行数
int getsug(struct ballot *uv) {
	int i, line;

	move(0, 0);
	screen_clrtobot();
	if (currvote.type == VOTE_ASKING) {
		show_voteing_title();
		line = 3;
		//% prints("请填入您的作答(三行):\n");
		prints("\xc7\xeb\xcc\xee\xc8\xeb\xc4\xfa\xb5\xc4\xd7\xf7\xb4\xf0(\xc8\xfd\xd0\xd0):\n");
	} else {
		line = 1;
		//% prints("请填入您宝贵的意见(三行):\n");
		prints("\xc7\xeb\xcc\xee\xc8\xeb\xc4\xfa\xb1\xa6\xb9\xf3\xb5\xc4\xd2\xe2\xbc\xfb(\xc8\xfd\xd0\xd0):\n");
	}
	move(line, 0);
	for (i = 0; i < 3; i++) {
		prints(": %s\n", uv->msg[i]);
	}
	for (i = 0; i < 3; i++) {
		getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NA);
		if (uv->msg[i][0] == '\0')
			break;
	}
	return i;
}

//输入多选/单选/是非的答案
//uv:用户投票的数据,返回后用户输入的答案放在 uv->msg里
//返回值: 成功1 用户取消-1
int multivote(struct ballot *uv) {
	unsigned int i;

	i = uv->voted;
	move(0, 0);
	show_voteing_title();
	//% uv->voted = setperms(uv->voted, "选票", currvote.totalitems,
	uv->voted = setperms(uv->voted, "\xd1\xa1\xc6\xb1", currvote.totalitems,
			showvoteitems);
	if (uv->voted == i)
		return -1;
	return 1;
}

//输入值型选项的答案
//uv:用户投票的数据,返回后用户输入的答案放在 uv->msg里
//返回值: 成功1 用户取消-1
int valuevote(struct ballot *uv) {
	unsigned int chs;
	char buf[10];

	chs = uv->voted;
	move(0, 0);
	show_voteing_title();
	//% prints("此次作答的值不能超过 [1m%d[m", currvote.maxtkt);
	prints("\xb4\xcb\xb4\xce\xd7\xf7\xb4\xf0\xb5\xc4\xd6\xb5\xb2\xbb\xc4\xdc\xb3\xac\xb9\xfd [1m%d[m", currvote.maxtkt);
	if (uv->voted != 0)
		sprintf(buf, "%d", uv->voted);
	else
		memset(buf, 0, sizeof(buf));
	do {
		//% getdata(3, 0, "请输入一个值? [0]: ", buf, 5, DOECHO, NA);
		getdata(3, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd2\xbb\xb8\xf6\xd6\xb5? [0]: ", buf, 5, DOECHO, NA);
		uv->voted = abs(atoi(buf));
	} while (uv->voted > currvote.maxtkt && buf[0] != '\n' && buf[0]
			!= '\0');
	if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
		return -1;
	return 1;
}

//用户进行投票,由vote_key,b_vote函数调用
//num:投票controlfile中第几个记录
//返回值:无
int user_vote(int num) {
	char fname[STRLEN], bname[STRLEN];
	char buf[STRLEN];
	struct ballot uservote, tmpbal;
	int votevalue;
	int aborted = NA, pos;

	move(-2, 0);
	get_record(controlfile, &currvote, sizeof(struct votebal), num);
	if (currentuser.firstlogin > currvote.opendate) { //注册日在投票开启日前不能投票
		//% prints("对不起, 投票名册上找不到您的大名\n");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0, \xcd\xb6\xc6\xb1\xc3\xfb\xb2\xe1\xc9\xcf\xd5\xd2\xb2\xbb\xb5\xbd\xc4\xfa\xb5\xc4\xb4\xf3\xc3\xfb\n");
		pressanykey();
		return 0;
	}
	sprintf(fname, "vote/%s/flag.%d", currboard, currvote.opendate);
	if ((pos = search_record(fname, &uservote, sizeof(uservote), cmpvuid,
			currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof(uservote));
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	strcpy(uservote.uid, currentuser.userid);
	sprintf(bname, "desc.%d", currvote.opendate);
	setvfile(buf, currboard, bname);
	ansimore(buf, YEA);
	move(0, 0);
	screen_clrtobot();
	switch (currvote.type) {
		case VOTE_SINGLE:
		case VOTE_MULTI:
		case VOTE_YN:
			votevalue = multivote(&uservote);
			if (votevalue == -1)
				aborted = YEA;
			break;
		case VOTE_VALUE:
			votevalue = valuevote(&uservote);
			if (votevalue == -1)
				aborted = YEA;
			break;
		case VOTE_ASKING:
			uservote.voted = 0;
			aborted = !getsug(&uservote);
			break;
	}
	screen_clear();
	if (aborted == YEA) {
		//% prints("保留 【[1m%s[m】原来的的投票。\n", currvote.title);
		prints("\xb1\xa3\xc1\xf4 \xa1\xbe[1m%s[m\xa1\xbf\xd4\xad\xc0\xb4\xb5\xc4\xb5\xc4\xcd\xb6\xc6\xb1\xa1\xa3\n", currvote.title);
	} else {
		if (currvote.type != VOTE_ASKING)
			getsug(&uservote);
		pos = search_record(fname, &tmpbal, sizeof(tmpbal), cmpvuid,
				currentuser.userid);
		if (pos) {
			substitute_record(fname, &uservote, sizeof(uservote), pos);
		} else if (append_record(fname, &uservote, sizeof(uservote)) == -1) {
			screen_move_clear(2);
			//% prints("投票失败! 请通知站长参加那一个选项投票\n");
			prints("\xcd\xb6\xc6\xb1\xca\xa7\xb0\xdc! \xc7\xeb\xcd\xa8\xd6\xaa\xd5\xbe\xb3\xa4\xb2\xce\xbc\xd3\xc4\xc7\xd2\xbb\xb8\xf6\xd1\xa1\xcf\xee\xcd\xb6\xc6\xb1\n");
			pressreturn();
		}
		//% prints("\n已经帮您投入票箱中...\n");
		prints("\n\xd2\xd1\xbe\xad\xb0\xef\xc4\xfa\xcd\xb6\xc8\xeb\xc6\xb1\xcf\xe4\xd6\xd0...\n");
	}
	pressanykey();
	return 0;
}

//显示投票箱信息的头部
void voteexp() {
	clrtoeol();
	//% prints("[1;44m编号 开启投票箱者 开启日 %-39s 类别 天数 人数[m\n", "投票主题");
	prints("[1;44m\xb1\xe0\xba\xc5 \xbf\xaa\xc6\xf4\xcd\xb6\xc6\xb1\xcf\xe4\xd5\xdf \xbf\xaa\xc6\xf4\xc8\xd5 %-39s \xc0\xe0\xb1\xf0 \xcc\xec\xca\xfd \xc8\xcb\xca\xfd[m\n", "\xcd\xb6\xc6\xb1\xd6\xf7\xcc\xe2");
}

//显示投票箱信息
//ent 投票信息
// The 'notused1' and 'notused2' arguemnts are not used,
// just to comply with function prototype.
static int printvote(void *entv, int notused1, void *notused2)
{
	static int i;
	struct ballot uservote;
	char buf[STRLEN + 10];
	char flagname[STRLEN];
	int num_voted;

	//Added by IAMFAT 2002.06.13
	char title[STRLEN];
	//Added End

	if (entv == NULL) {
		move(2, 0);
		voteexp();
		i = 0;
		return 0;
	}
	struct votebal *ent = (struct votebal *)entv;
	i++;
	if (i > page + 19 || i > range)
		return QUIT;
	else if (i <= page)
		return 0;
	sprintf(buf, "flag.%d", ent->opendate);
	setvfile(flagname, currboard, buf);
	if (search_record(flagname, &uservote, sizeof(uservote), cmpvuid,
			currentuser.userid) <= 0) {
		voted_flag = NA;
	} else
		voted_flag = YEA;
	num_voted = get_num_records(flagname, sizeof(struct ballot));
	strcpy(title, ent->title);
	ellipsis(title, 39);
	sprintf(buf, " %s%3d %-12.12s %6.6s %-39.39s %-4.4s %3d  %4d\033[m\n",
			(voted_flag == NA) ? "[1m" : "", i, ent->userid,
			format_time(ent->opendate, TIME_FORMAT_ZH) + 6, title,
			vote_type[ent->type - 1], ent->maxdays, num_voted);
	//Ended IAMFAT
	prints("%s", buf);
	return 0;
}

//显示投票结果
//bname:版名
//返回值:固定为FULLUPDATE
int vote_results(const char *bname)
{
	char buf[STRLEN];

	setvfile(buf, bname, "results");
	if (ansimore(buf, YEA) == -1) {
		move(3, 0);
		//% prints("目前没有任何投票的结果。\n");
		prints("\xc4\xbf\xc7\xb0\xc3\xbb\xd3\xd0\xc8\xce\xba\xce\xcd\xb6\xc6\xb1\xb5\xc4\xbd\xe1\xb9\xfb\xa1\xa3\n");
		screen_clrtobot();
		pressreturn();
	} else
		screen_clear();
	return FULLUPDATE;
}

//显示投票箱选项
void vote_title() {

	docmdtitle(
			//% "[投票箱列表]",
			"[\xcd\xb6\xc6\xb1\xcf\xe4\xc1\xd0\xb1\xed]",
			//% "[[1;32m←[m,[1;32me[m] 离开 [[1;32mh[m] 求助 [[1;32m→[m,[1;32mr <cr>[m] 进行投票 [[1;32m↑[m,[1;32m↓[m] 上,下选择 [1m高亮度[m表示尚未投票");
			"[[1;32m\xa1\xfb[m,[1;32me[m] \xc0\xeb\xbf\xaa [[1;32mh[m] \xc7\xf3\xd6\xfa [[1;32m\xa1\xfa[m,[1;32mr <cr>[m] \xbd\xf8\xd0\xd0\xcd\xb6\xc6\xb1 [[1;32m\xa1\xfc[m,[1;32m\xa1\xfd[m] \xc9\xcf,\xcf\xc2\xd1\xa1\xd4\xf1 [1m\xb8\xdf\xc1\xc1\xb6\xc8[m\xb1\xed\xca\xbe\xc9\xd0\xce\xb4\xcd\xb6\xc6\xb1");
	tui_update_status_line();
}

//显示投票箱信息
int Show_Votes(void)
{
	move(3, 0);
	screen_clrtobot();
	printvote(NULL, 0, NULL);
	setcontrolfile(currboard);
	if (apply_record(controlfile, printvote, sizeof(struct votebal), NULL, 0,
			0, true) == -1) {
		//% prints("错误，没有投票箱开启....");
		prints("\xb4\xed\xce\xf3\xa3\xac\xc3\xbb\xd3\xd0\xcd\xb6\xc6\xb1\xcf\xe4\xbf\xaa\xc6\xf4....");
		pressreturn();
		return 0;
	}
	screen_clrtobot();
	return 0;
}

//根据用户的按键对投票箱进行操作,可以结束/修改/强制关闭/显示投票结果
//ch: 用户的按键
//allnum:投票controlfile的第几个记录
//pagenum:未使用
//返回值 0:失败 1:成功
int vote_key(int ch, int allnum, int pagenum) {
	int deal = 0, ans;
	char buf[STRLEN];

	switch (ch) {
		case 'v':
		case 'V':
		case '\n':
		case '\r':
		case 'r':
		case KEY_RIGHT:
			user_vote(allnum + 1);
			deal = 1;
			break;
		case 'R':
			vote_results(currboard);
			deal = 1;
			break;
		case 'H':
		case 'h':
			show_help("help/votehelp");
			deal = 1;
			break;
		case 'A':
		case 'a':
			if (!am_curr_bm())
				return YEA;
			vote_maintain(currboard);
			deal = 1;
			break;
		case 'O':
		case 'o':
			if (!am_curr_bm())
				return YEA;
			screen_clear();
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			//% prints("[5;1;31m警告!![m\n");
			prints("[5;1;31m\xbe\xaf\xb8\xe6!![m\n");
			//% prints("投票箱标题：[1m%s[m\n", currvote.title);
			prints("\xcd\xb6\xc6\xb1\xcf\xe4\xb1\xea\xcc\xe2\xa3\xba[1m%s[m\n", currvote.title);
			//% ans = askyn("您确定要提早结束这个投票吗", NA, NA);
			ans = askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xcc\xe1\xd4\xe7\xbd\xe1\xca\xf8\xd5\xe2\xb8\xf6\xcd\xb6\xc6\xb1\xc2\xf0", NA, NA);

			if (ans != 1) {
				move(2, 0);
				//% prints("取消删除行动\n");
				prints("\xc8\xa1\xcf\xfb\xc9\xbe\xb3\xfd\xd0\xd0\xb6\xaf\n");
				pressreturn();
				screen_clear();
				break;
			}
			mk_result(allnum + 1);
			//% sprintf(buf, "[结束] 提早结束投票 %s", currvote.title);
			sprintf(buf, "[\xbd\xe1\xca\xf8] \xcc\xe1\xd4\xe7\xbd\xe1\xca\xf8\xcd\xb6\xc6\xb1 %s", currvote.title);
			securityreport(buf, 0, 4);
			break;
		case 'M':
		case 'm':
			if (!am_curr_bm())
				return YEA;
			screen_clear();
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			//% prints("[5;1;31m警告!![m\n");
			prints("[5;1;31m\xbe\xaf\xb8\xe6!![m\n");
			//% prints("投票箱标题：[1m%s[m\n", currvote.title);
			prints("\xcd\xb6\xc6\xb1\xcf\xe4\xb1\xea\xcc\xe2\xa3\xba[1m%s[m\n", currvote.title);
			//% ans = askyn("您确定要修改这个投票的设定吗", NA, NA);
			ans = askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xd0\xde\xb8\xc4\xd5\xe2\xb8\xf6\xcd\xb6\xc6\xb1\xb5\xc4\xc9\xe8\xb6\xa8\xc2\xf0", NA, NA);

			if (ans != 1) {
				move(2, 0);
				//% prints("取消修改行动\n");
				prints("\xc8\xa1\xcf\xfb\xd0\xde\xb8\xc4\xd0\xd0\xb6\xaf\n");
				pressreturn();
				screen_clear();
				break;
			}
			makevote(&currvote, currboard);
			substitute_record(controlfile, &currvote,
					sizeof(struct votebal), allnum + 1);
			//% sprintf(buf, "[修改] 修改投票设定 %s", currvote.title);
			sprintf(buf, "[\xd0\xde\xb8\xc4] \xd0\xde\xb8\xc4\xcd\xb6\xc6\xb1\xc9\xe8\xb6\xa8 %s", currvote.title);
			securityreport(buf, 0, 4);
			break;
		case 'D':
		case 'd':
			if (!am_curr_bm())
				return 1;
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			screen_clear();
			//% prints("[5;1;31m警告!![m\n");
			prints("[5;1;31m\xbe\xaf\xb8\xe6!![m\n");
			//% prints("投票箱标题：[1m%s[m\n", currvote.title);
			prints("\xcd\xb6\xc6\xb1\xcf\xe4\xb1\xea\xcc\xe2\xa3\xba[1m%s[m\n", currvote.title);
			//% ans = askyn("您确定要强制关闭这个投票吗", NA, NA);
			ans = askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xc7\xbf\xd6\xc6\xb9\xd8\xb1\xd5\xd5\xe2\xb8\xf6\xcd\xb6\xc6\xb1\xc2\xf0", NA, NA);

			if (ans != 1) {
				move(2, 0);
				//% prints("取消删除行动\n");
				prints("\xc8\xa1\xcf\xfb\xc9\xbe\xb3\xfd\xd0\xd0\xb6\xaf\n");
				pressreturn();
				screen_clear();
				break;
			}
			//% sprintf(buf, "[关闭] 强制关闭投票 %s", currvote.title);
			sprintf(buf, "[\xb9\xd8\xb1\xd5] \xc7\xbf\xd6\xc6\xb9\xd8\xb1\xd5\xcd\xb6\xc6\xb1 %s", currvote.title);
			securityreport(buf, 0, 4);
			dele_vote(allnum + 1);
			break;
		default:
			return 0;
	}
	if (deal) {
		Show_Votes();
		vote_title();
	}
	return 1;
}

//用户对本版进行投票，bbs.c调用
//返回值:固定为FULLUPDATE
int b_vote(const char *bname)
{
	int num_of_vote;

	if (!HAS_PERM(PERM_VOTE) || (currentuser.stay < 1800)) {
		return 0;
	}
	setcontrolfile(bname);
	num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
	if (num_of_vote == 0) {
		move(2, 0);
		screen_clrtobot();
		//% prints("\n抱歉, 目前并没有任何投票举行。\n");
		prints("\n\xb1\xa7\xc7\xb8, \xc4\xbf\xc7\xb0\xb2\xa2\xc3\xbb\xd3\xd0\xc8\xce\xba\xce\xcd\xb6\xc6\xb1\xbe\xd9\xd0\xd0\xa1\xa3\n");
		pressreturn();
		setvoteflag(bname, 0);
		return FULLUPDATE;
	}
	setlistrange(num_of_vote);
	screen_clear();
	choose(NA, 0, vote_title, vote_key, Show_Votes, user_vote); //?
	screen_clear();
	return FULLUPDATE;
}

//SYSOP版开启投票箱
int m_vote() {
	set_user_status(ST_ADMIN);
	vote_maintain(DEFAULTBOARD);
	return 0;
}

//对SYSOP版进行投票
int x_vote() {
	set_user_status(ST_XMENU);
	b_vote(DEFAULTBOARD);
	return 0;
}

//显示sysop版投票结果
int x_results() {
	set_user_status(ST_XMENU); //更改用户 模式状态至
	return vote_results(DEFAULTBOARD); //显示sysop版投票结果
}
