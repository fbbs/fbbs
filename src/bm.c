#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/helper.h"
#include "fbbs/log.h"
#include "fbbs/mail.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

//定义页面大小
#define BBS_PAGESIZE (screen_lines()-4)

/**
 *
 */
static bool club_do_add_equal(const char *buf, size_t size, const char *str,
		size_t len)
{
	if (len >= size)
		return false;
	return (buf[len] == ' ' && !strncmp(buf, str, len));
}

/**
 *
 */
static int club_do_add(const char *user, const char *board, const char *ps)
{
	char buf[LINE_BUFSIZE], file[HOMELEN], title[STRLEN], msg[256];
	time_t now= time(NULL);
	struct tm* t = localtime(&now);
	snprintf(buf, sizeof(buf), "%-12s %-40s %04d.%02d.%02d %-12s\n", user, ps,
			1900 + t->tm_year, t->tm_mon + 1, t->tm_mday, currentuser.userid);

	setbfile(file, board, "club_users");
	int ret = add_to_file(file, buf, strlen(user), false, club_do_add_equal);
	if (ret < 0)
		return ret;

	log_bm(LOG_BM_ADDCLUB, 1);

	//% snprintf(title, sizeof(title), "%s邀请%s加入俱乐部版%s",
	snprintf(title, sizeof(title), "%s\xd1\xfb\xc7\xeb%s\xbc\xd3\xc8\xeb\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6%s",
			currentuser.userid, user, board);
	//% snprintf(msg, sizeof(msg), "%s:\n\n    您被邀请加入俱乐部版 %s\n\n补充说明"
	snprintf(msg, sizeof(msg), "%s:\n\n    \xc4\xfa\xb1\xbb\xd1\xfb\xc7\xeb\xbc\xd3\xc8\xeb\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6 %s\n\n\xb2\xb9\xb3\xe4\xcb\xb5\xc3\xf7"
			//% "：%s\n\n邀请人: %s\n", user, board, ps, currentuser.userid);
			"\xa3\xba%s\n\n\xd1\xfb\xc7\xeb\xc8\xcb: %s\n", user, board, ps, currentuser.userid);
	autoreport(currboard, title, msg, user, POST_FILE_AUTO);
	Poststring(msg, "club", title, 2);
	return 0;
}

/**
 *
 */
static int club_add(void)
{
	struct userec urec;
	char user[IDLEN + 1], ps[40], buf[STRLEN];
	move(1, 0);
	//% usercomplete("增加俱乐部成员: ", user);
	usercomplete("\xd4\xf6\xbc\xd3\xbe\xe3\xc0\xd6\xb2\xbf\xb3\xc9\xd4\xb1: ", user);
	if (*user == '\0' || !getuserec(user, &urec))
		return -1;
	if (!strcasecmp(user, "guest")) {
		//% 不能邀请guest加入俱乐部
		presskeyfor("\xb2\xbb\xc4\xdc\xd1\xfb\xc7\xebguest\xbc\xd3\xc8\xeb\xbe\xe3\xc0\xd6\xb2\xbf", -1);
		return -1;
	}
	//% getdata(1, 0, "输入补充说明:", ps, sizeof(ps), DOECHO, YEA);
	getdata(1, 0, "\xca\xe4\xc8\xeb\xb2\xb9\xb3\xe4\xcb\xb5\xc3\xf7:", ps, sizeof(ps), DOECHO, YEA);
	move(1, 0);
	//% snprintf(buf, sizeof(buf), "邀请 %s 加入俱乐部吗?", urec.userid);
	snprintf(buf, sizeof(buf), "\xd1\xfb\xc7\xeb %s \xbc\xd3\xc8\xeb\xbe\xe3\xc0\xd6\xb2\xbf\xc2\xf0?", urec.userid);
	if (!askyn(buf, YEA, NA))
		return -1;
	return club_do_add(urec.userid, currboard, ps);
}

/**
 *
 */
static int club_del(const char *user, const char *board)
{
	char file[HOMELEN];
	setbfile(file, currboard, "club_users");
	if (del_from_file(file, user) < 0)
		return -1;

	log_bm(LOG_BM_DELCLUB, 1);

	char title[STRLEN];
	char *msg = "";
	//% snprintf(title, sizeof(title), "%s取消%s在俱乐部版%s的权利",
	snprintf(title, sizeof(title), "%s\xc8\xa1\xcf\xfb%s\xd4\xda\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6%s\xb5\xc4\xc8\xa8\xc0\xfb",
			currentuser.userid, user, board);
	autoreport(currboard, title, msg, user, POST_FILE_AUTO);
	Poststring(msg, "club", title, 2);
	return 0;
}

/**
 *
 */
static void club_title_show(void)
{
	move(0, 0);
	//% outs("\033[1;44;36m 设定俱乐部名单\033[K\033[m\n"
	outs("\033[1;44;36m \xc9\xe8\xb6\xa8\xbe\xe3\xc0\xd6\xb2\xbf\xc3\xfb\xb5\xa5\033[K\033[m\n"
			//% "离开[\033[1;32m←\033[m] 选择[\033[1;32m↑\033[m,\033[1;32m↓"
			"\xc0\xeb\xbf\xaa[\033[1;32m\xa1\xfb\033[m] \xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd"
			//% "\033[m] 添加[\033[1;32ma\033[m] 删除[\033[1;32md\033[m] 查找"
			"\033[m] \xcc\xed\xbc\xd3[\033[1;32ma\033[m] \xc9\xbe\xb3\xfd[\033[1;32md\033[m] \xb2\xe9\xd5\xd2"
			"[\033[1;32m/\033[m]\n"
			//% "\033[1;44m 用户代号     附加说明                             "
			"\033[1;44m \xd3\xc3\xbb\xa7\xb4\xfa\xba\xc5     \xb8\xbd\xbc\xd3\xcb\xb5\xc3\xf7                             "
			//% "    邀请日期   邀请人\033[K\033[m\n");
			"    \xd1\xfb\xc7\xeb\xc8\xd5\xc6\xda   \xd1\xfb\xc7\xeb\xc8\xcb\033[K\033[m\n");
}

/**
 *
 */
static int club_key_deal(const char *fname, int ch, const char *line)
{
	char user[IDLEN + 1], buf[STRLEN];
	if (line) {
		strlcpy(user, line, sizeof(user));
		strtok(user, " \n\r\t");
	}
	switch (ch) {
		case 'a':
			club_add();
			break;
		case 'd':
			if (!line)
				return 0;
			move(1, 0);
			//% snprintf(buf, sizeof(buf), "删除俱乐部成员%s吗?", user);
			snprintf(buf, sizeof(buf), "\xc9\xbe\xb3\xfd\xbe\xe3\xc0\xd6\xb2\xbf\xb3\xc9\xd4\xb1%s\xc2\xf0?", user);
			if (!askyn(buf, NA, NA))
				return 1;
			club_del(user, currboard);
			break;
		case Ctrl('A'):
		case KEY_RIGHT: //用户信息
			if (!line)
				return 0;
			t_query(user);
			break;
		default:
			return 0;
	}
	return 1;
}

/**
 *
 */
int club_user(void)
{
	board_t board;
	if (!get_board(currboard, &board))
		return DONOTHING;

	char file[HOMELEN];
	if ((board.flag & BOARD_FLAG_CLUB) && am_curr_bm()) {
		setbfile(file, currboard, "club_users");
		list_text(file, club_title_show, club_key_deal, NULL);
		return FULLUPDATE;
	}
	return DONOTHING;
}
