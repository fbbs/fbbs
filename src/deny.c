#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/log.h"
#include "fbbs/mail.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

#define DENY_LEVEL_LIST ".DenyLevel"
#define DENY_LEVEL_FILE "etc/denylevel"
#define DENY_BOARD_FILE "etc/denyboard"
#define DEFAULT_REASON "-------------------------------"
#define BBS_PAGESIZE (screen_lines() - 4)

enum {
	OFFSET_PERM = 48,    ///< Offset of the permission in a ban list.
	OFFSET_DATE = 53,    ///< Offset of the release date in a ban list.
	DENY_FOREVER = 999,  ///< Ban forever.
	DENY_TEMP = 0,       ///< Ban temporarily.
	DENY_BOARD_MAX = 90, ///< Maximum ban days (board-wide).
	DENY_BOARD_MIN = 1,  ///< Minimum ban days (board-wide).
};

static char reason[32];  ///< Reason of banning.

/**
 * Set file to specific line.
 * @param[in,out] fp The file to set.
 * @param[in] line The line to set to.
 * @return 0 if OK, -1 on error.
 */
static int fseekline(FILE *fp, int line)
{
	if (!fp)
		return -1;
	fseek(fp, 0, SEEK_SET);
	char buf[LINE_BUFSIZE];
	while (--line >= 0) {
		if (!fgets(buf, sizeof(buf), fp))
			return -1;
	}
	return 0;
}

/**
 * Search for the line containing specific string.
 * @param[in] fp An opened file to search.
 * @param[in] n Current line (0-based).
 * @param[in] str The search string.
 * @return Line number of first such line,
 *         -1 when not found or an error occured.
 */
static int text_find(FILE *fp, int n, const char *str)
{
	if (!fp)
		return -1;
	char buf[LINE_BUFSIZE];
	while (fgets(buf, sizeof(buf), fp)) {
		if (strcasestr_gbk(buf, str))
			return n;
		else
			n++;
	}
	return -1;
}

/**
 * List file by lines. Providing basic browsing functions.
 * @param[in] file The name of the file.
 * @param[in] title_show The function to show title (3 lines).
 * @param[in] key_deal Additional key handler.
 * @param[in] check The function to determine whether to show a check sign, 
                omit if NULL.
 */
void list_text(const char *file,
		void (*title_show)(void),
		int (*key_deal)(const char *, int, const char *),
		int (*check)(const char *))
{
	bool redraw = true, empty = true;
	int line = 0, from = 0, to = 0, y = 3;
	int ch;
	char buf[LINE_BUFSIZE];
	FILE *fp = NULL;

	while (true) {
		if (fp)
			fclose(fp);
		fp = fopen(file, "r");
		if (fp)
			empty = false;
		else
			empty = true;

		if (redraw) {
			screen_clear();
			title_show();
			if (empty) {
				//% prints("(无内容)\n");
				prints("(\xce\xde\xc4\xda\xc8\xdd)\n");
			} else {
				to = from;
				fseekline(fp, from);
				y = 3;
				while (fgets(buf, sizeof(buf), fp)) {
					strtok(buf, "\n");
					if (check) {
						prints(" %-2s%-76s\n",
								//% (*check)(buf) ? "√" : "  ", buf);
								(*check)(buf) ? "\xa1\xcc" : "  ", buf);
					} else {
						prints(" %-78s\n", buf);
					}
					to++;
					y++;
					if (y > screen_lines() - 2)
						break;
				}
				if (from == to) {
					if (from == 0) {
						empty = true;
					} else {
						from -= (BBS_PAGESIZE - 1);
						if (from < 0)
							from = 0;
						line = from;
						continue;
					}
				}
				if (line > to - 1)
					line = to - 1;
			}
			tui_update_status_line();
		}

		if (!empty) {
			screen_move(line - from + 3, 0);
			prints(">");
		}
		ch = egetch();
		if (!empty) {
			screen_move(line - from + 3, 0);
			prints(" ");
		}

		redraw = false;
		switch (ch) {
			case KEY_UP:
				if (empty || line == 0)
					break;
				line--;
				if (line < from) {
					from -= (BBS_PAGESIZE - 1);
					if (from < 0)
						from = 0;
					if (line < from)
						line = from;
					redraw = true;
				}
				break;
			case KEY_DOWN:
				if (empty)
					break;
				if (line < to - 1) {
					line++;
					if (line == (from + BBS_PAGESIZE - 1)) {
						from += (BBS_PAGESIZE - 1);
						line = from;
						redraw = true;
					}
				}
				break;
			case Ctrl('B'):
			case KEY_PGUP:
				if (empty || line == 0)
					break;
				from -= BBS_PAGESIZE - 1;
				line -= BBS_PAGESIZE - 1;
				if (from < 0)
					from = 0;
				if (line < from)
					line = from;
				redraw = true;
				break;
			case Ctrl('F'):
			case KEY_PGDN:
				if (empty)
					break;
				if (to - from >= BBS_PAGESIZE) {
					from += BBS_PAGESIZE - 1;
					line += BBS_PAGESIZE - 1;
					redraw = true;
				} else {
					line = to - 1;
				}
				break;
			case '/':
			{
				int old;
				if (empty)
					break;
				//% getdata(1, 0, "查找:", buf, 50, DOECHO, YEA);
				getdata(1, 0, "\xb2\xe9\xd5\xd2:", buf, 50, DOECHO, YEA);
				redraw = true;
				old = line;
				line++;
				fseekline(fp, line);
				line = text_find(fp, line, buf);
				if (line < 0)
					line = old;
				else
					from = line - line % (BBS_PAGESIZE - 1);
			}
				break;
			case KEY_LEFT:
			case KEY_ESC:
			case '\n':
				if (fp)
					fclose(fp);
				return;
			default:
				if (key_deal) {
					if (empty) {
						redraw = (*key_deal)(file, ch, NULL);
					} else {
						fseekline(fp, line);
						fgets(buf, sizeof(buf), fp);
						redraw = (*key_deal)(file, ch, buf);
					}
				}
		}
	}
	fclose(fp);
}

/**
 * Copy ban reason from string.
 * @param[in] r string to copy from.
 */
static void reason_copy(const char *r)
{
	strlcpy(reason, r, sizeof(reason));
}

/**
 * Title of UI for choosing ban reason.
 */
static void reason_title_show(void)
{
	screen_move(0, 0);
	//% prints("\033[1;44;36m封禁原因列表\033[K\033[m\n 封禁原因:%s\n\033[1;44m "
	prints("\033[1;44;36m\xb7\xe2\xbd\xfb\xd4\xad\xd2\xf2\xc1\xd0\xb1\xed\033[K\033[m\n \xb7\xe2\xbd\xfb\xd4\xad\xd2\xf2:%s\n\033[1;44m "
			//% "封禁的原因及处罚建议\033[K\033[m\n", reason);
			"\xb7\xe2\xbd\xfb\xb5\xc4\xd4\xad\xd2\xf2\xbc\xb0\xb4\xa6\xb7\xa3\xbd\xa8\xd2\xe9\033[K\033[m\n", reason);
}

/**
 * Key handler of UI for choosing ban reason.
 * @param[in] file Not used.
 * @param[in] ch The key pressed.
 * @param[in] line Current line content.
 * @return redraw the screen or not (1/0).
 */
static int reason_key_deal(const char *file, int ch, const char *line)
{
	if (!line)
		return 0;
	char reasonlist[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ;'.[]";
	ch = toupper(ch);
	if (ch == ' ')
		ch = line[0];
	char *found = strchr(reasonlist, ch);
	if (found) {
		char *r = reason + (found - reasonlist);
		*r = (*r == '-') ? *found : '-';
	}
	return 1;
}

/**
 * Check whether the initial char of 'line' appears in 'reason'.
 * @param[in] line The string to check.
 * @return 1 if found, 0 otherwise.
 */
static int reason_check(const char *line)
{
    if (strchr(reason, *line))
		return 1;
	return 0;
}

/**
 * Choose a ban reason.
 * @param[in] file ban reason file.
 */
static void reason_select(const char *file)
{
	char f[HOMELEN];
	snprintf(f, sizeof(f), "%s.suggestion", file);
	while (1) {
		list_text(f, reason_title_show,
				reason_key_deal, reason_check);
		if (strncmp(reason, DEFAULT_REASON, sizeof(reason) - 1))
			break;
	}
}

/**
 * Title of board-wide ban list.
 */
static void deny_title_show(void)
{
	screen_move(0, 0);
	//% prints("\033[1;44;36m 设定无法发文的名单\033[K\033[m\n"
	prints("\033[1;44;36m \xc9\xe8\xb6\xa8\xce\xde\xb7\xa8\xb7\xa2\xce\xc4\xb5\xc4\xc3\xfb\xb5\xa5\033[K\033[m\n"
			//% " 离开[\033[1;32m←\033[m] 选择[\033[1;32m↑\033[m,\033[1;32m↓"
			" \xc0\xeb\xbf\xaa[\033[1;32m\xa1\xfb\033[m] \xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd"
			//% "\033[m] 添加[\033[1;32ma\033[m] 修改[\033[1;32mc\033[m] 解封"
			"\033[m] \xcc\xed\xbc\xd3[\033[1;32ma\033[m] \xd0\xde\xb8\xc4[\033[1;32mc\033[m] \xbd\xe2\xb7\xe2"
			//% "[\033[1;32md\033[m] 查找[\033[1;32m/\033[m]\n"
			"[\033[1;32md\033[m] \xb2\xe9\xd5\xd2[\033[1;32m/\033[m]\n"
			//% "\033[1;44m 用户代号     封禁原因(A-Z,;'[])              天数"
			"\033[1;44m \xd3\xc3\xbb\xa7\xb4\xfa\xba\xc5     \xb7\xe2\xbd\xfb\xd4\xad\xd2\xf2(A-Z,;'[])              \xcc\xec\xca\xfd"
			//% "    解封日期       版主      \033[m\n");
			"    \xbd\xe2\xb7\xe2\xc8\xd5\xc6\xda       \xb0\xe6\xd6\xf7      \033[m\n");
}

/**
 * Generate detailed reason for banning.
 * @param fp An opened file for output.
 * @param file Prefix of detail file.
 */
static void deny_generate(FILE *fp, const char *file)
{
	char detail[HOMELEN], buf[LINE_BUFSIZE];
	snprintf(detail, sizeof(detail), "%s.detail", file);
	FILE *fpr = fopen(detail, "r");
	if (fpr) {
		while (fgets(buf, sizeof(buf), fpr)) {
			if (strchr(reason, buf[0]))
				fputs(buf + 1, fp);
		}
		fclose(fpr);
	}
}

/**
 * Tell whether buf equals str.
 * @param buf The buffer.
 * @param size The size of the buffer.
 * @param str The string.
 * @param len The length of the string.
 * @return True if equals, false otherwise.
 */
static bool deny_do_add_eqaul(const char *buf, size_t size, const char *str,
		size_t len)
{
	if (len >= size)
		return false;
	return (buf[len] == ' ' && !strncmp(buf, str, len));
}

/**
 * Ban a user board-wide or change an existing ban.
 * @param[in] board The board.
 * @param[in] user The user to be banned.
 * @param[in] ps PS.
 * @param[in] days Days to ban.
 * @param[in] change Whether changing the penalty.
 * @return 0 on success, bbserrno on error.
 */
static int deny_do_add(const char *board, const char *user, const char *ps,
		int days, bool change)
{
	struct userec urec;
	if (!getuserec(user, &urec))
		return BBS_ENOUSR;
	if (!strcmp(currentuser.userid, urec.userid))
		return BBS_ENOUSR;
	if (!strcmp(urec.userid, "guest"))
		return BBS_EDGUEST;
	if (days > DENY_BOARD_MAX)
		days = DENY_BOARD_MAX;
	if (days < DENY_BOARD_MIN)
		days = DENY_BOARD_MIN;

	time_t date = time(NULL) + days * 24 * 60 * 60;
	struct tm *t = localtime(&date);
	char str[STRLEN];
	//% snprintf(str, sizeof(str), "%-12s %-31s %2d天 %02d年%02d月%02d日解 %-12s\n",
	snprintf(str, sizeof(str), "%-12s %-31s %2d\xcc\xec %02d\xc4\xea%02d\xd4\xc2%02d\xc8\xd5\xbd\xe2 %-12s\n",
			urec.userid, reason, days, t->tm_year % 100, t->tm_mon + 1,
			t->tm_mday, currentuser.userid);

	// Edit ban list.
	char list[HOMELEN];
	setbfile(list, board, "deny_users");
	int ret = add_to_file(list, str, strlen(urec.userid), change,
			deny_do_add_eqaul);
	if (ret < 0)
		return ret;
	
	// Generate notification.
	char file[HOMELEN], title[STRLEN];
	if (change) {
		//% snprintf(title, sizeof(title), "修改%s在%s版的封禁",
		snprintf(title, sizeof(title), "\xd0\xde\xb8\xc4%s\xd4\xda%s\xb0\xe6\xb5\xc4\xb7\xe2\xbd\xfb",
				urec.userid, board);
	} else {
		//% snprintf(title, sizeof(title), "封禁%s在%s版的发文权限",
		snprintf(title, sizeof(title), "\xb7\xe2\xbd\xfb%s\xd4\xda%s\xb0\xe6\xb5\xc4\xb7\xa2\xce\xc4\xc8\xa8\xcf\xde",
				urec.userid, board);
	}
	file_temp_name(file, sizeof(file));
	FILE *fpw = fopen(file, "w");
	//% fprintf(fpw, "%s因:\n", urec.userid);
	fprintf(fpw, "%s\xd2\xf2:\n", urec.userid);
	deny_generate(fpw, DENY_BOARD_FILE);
	//% fprintf(fpw, "\n应被封禁%s版发文权限%d天\n请在处罚期满后(%04d.%02d.%02d)"
	fprintf(fpw, "\n\xd3\xa6\xb1\xbb\xb7\xe2\xbd\xfb%s\xb0\xe6\xb7\xa2\xce\xc4\xc8\xa8\xcf\xde%d\xcc\xec\n\xc7\xeb\xd4\xda\xb4\xa6\xb7\xa3\xc6\xda\xc2\xfa\xba\xf3(%04d.%02d.%02d)"
			//% ", 向%s写信要求解除处罚。\n如不服本决定, 可以联系处罚决定人或"
			", \xcf\xf2%s\xd0\xb4\xd0\xc5\xd2\xaa\xc7\xf3\xbd\xe2\xb3\xfd\xb4\xa6\xb7\xa3\xa1\xa3\n\xc8\xe7\xb2\xbb\xb7\xfe\xb1\xbe\xbe\xf6\xb6\xa8, \xbf\xc9\xd2\xd4\xc1\xaa\xcf\xb5\xb4\xa6\xb7\xa3\xbe\xf6\xb6\xa8\xc8\xcb\xbb\xf2"
			//% "在7日内到Appeal申请复议。\nP.S.: %s\n执行人: %s\n",
			"\xd4\xda""7\xc8\xd5\xc4\xda\xb5\xbd""Appeal\xc9\xea\xc7\xeb\xb8\xb4\xd2\xe9\xa1\xa3\nP.S.: %s\n\xd6\xb4\xd0\xd0\xc8\xcb: %s\n",
			board, days, 1900 + t->tm_year, t->tm_mon + 1, t->tm_mday,
			currentuser.userid, ps, currentuser.userid);
	fclose(fpw);

	Postfile(file, board, title, 1);
	Postfile(file, "Notice", title, 1);
	mail_file(file, urec.userid, title);
	unlink(file);
	log_bm(LOG_BM_DENYPOST, 1);
	return 0;
}

/**
 * Telnet UI for banning a user board-wide.
 * @param[in] line Current line.
 */
static void deny_add(const char *line)
{
	char id[IDLEN + 1], ps[50], ans[5];
	if (line) {
		strlcpy(id, line, sizeof(id));
		strtok(id, " \n\r\t");
		reason_copy(line + IDLEN + 1);
	} else {
		user_complete(1, "封禁使用者: ", id, sizeof(id));
		if (*id == '\0')
			return;
		reason_copy(DEFAULT_REASON);
	}
	reason_select(DENY_BOARD_FILE);
	//% getdata(1, 0, "输入补充说明:", ps, sizeof(ps), DOECHO, YEA);
	getdata(1, 0, "\xca\xe4\xc8\xeb\xb2\xb9\xb3\xe4\xcb\xb5\xc3\xf7:", ps, sizeof(ps), DOECHO, YEA);
	//% getdata(1, 0, "输入天数(默认1天):", ans, sizeof(ans), DOECHO, YEA);
	getdata(1, 0, "\xca\xe4\xc8\xeb\xcc\xec\xca\xfd(\xc4\xac\xc8\xcf""1\xcc\xec):", ans, sizeof(ans), DOECHO, YEA);
	int yes;
	screen_move(1, 0);
	if (line)
		//% yes = askyn("要修改对该用户的封禁吗?", NA, NA);
		yes = askyn("\xd2\xaa\xd0\xde\xb8\xc4\xb6\xd4\xb8\xc3\xd3\xc3\xbb\xa7\xb5\xc4\xb7\xe2\xbd\xfb\xc2\xf0?", NA, NA);
	else
		//% yes = askyn("真的要封禁该用户吗?", NA, NA);
		yes = askyn("\xd5\xe6\xb5\xc4\xd2\xaa\xb7\xe2\xbd\xfb\xb8\xc3\xd3\xc3\xbb\xa7\xc2\xf0?", NA, NA);
	if (yes) {
		switch (deny_do_add(currboard, id, ps, strtol(ans, NULL, 10), line)) {
			case BBS_EDSELF:
				//% presskeyfor("ft! 封自己玩!!!??? NO WAY! :P", 1);
				presskeyfor("ft! \xb7\xe2\xd7\xd4\xbc\xba\xcd\xe6!!!??? NO WAY! :P", 1);
				break;
			case BBS_ELEXIST:
				//% presskeyfor("该用户已在封禁名单中", 1);
				presskeyfor("\xb8\xc3\xd3\xc3\xbb\xa7\xd2\xd1\xd4\xda\xb7\xe2\xbd\xfb\xc3\xfb\xb5\xa5\xd6\xd0", 1);
				break;
			case BBS_EDGUEST:
				//% presskeyfor("你在搞笑吗?封guest?", 1);
				presskeyfor("\xc4\xe3\xd4\xda\xb8\xe3\xd0\xa6\xc2\xf0?\xb7\xe2guest?", 1);
				break;
			default:
				break;
		}
	}
}

/**
 * Release a user.
 * @param[in] line Current line.
 */
static void deny_release(const char *line)
{
	char list[HOMELEN];
	setbfile(list, currboard, "deny_users");
	if (del_from_file(list, line) != 0)
		return;
	log_bm(LOG_BM_UNDENY, 1);

	char title[STRLEN], user[IDLEN + 1], buf[256];
	strlcpy(user, line, sizeof(user));
	strtok(user, " \r\n\t");
	//% snprintf(title, sizeof(title), "恢复%s在%s版的发文权限",
	snprintf(title, sizeof(title), "\xbb\xd6\xb8\xb4%s\xd4\xda%s\xb0\xe6\xb5\xc4\xb7\xa2\xce\xc4\xc8\xa8\xcf\xde",
				user, currboard);
	//% snprintf(buf, sizeof(buf), "%s恢复%s在%s版发文权限.\n",
	snprintf(buf, sizeof(buf), "%s\xbb\xd6\xb8\xb4%s\xd4\xda%s\xb0\xe6\xb7\xa2\xce\xc4\xc8\xa8\xcf\xde.\n",
			currentuser.userid, user, currboard);
	Poststring(buf, currboard, title, 1);
	Poststring(buf, "Notice", title, 1);
	do_mail_file(user, title, NULL, buf, strlen(buf), NULL);
}

/**
 * Key handler for manipulating a board-wide ban list.
 * @param[in] file The board-wide ban list.
 * @param[in] ch The key.
 * @param[in] line Current line.
 * @return Whether redraw the screen.
 */
static int deny_key_deal(const char *file, int ch, const char *line)
{
	switch (ch) {
		case 'a':
			deny_add(NULL);
			break;
		case 'c':
			if (!line)
				return 0;
			deny_add(line);
			break;
		case 'd':
			if (!line)
				return 0;
			screen_move(1, 0);
			//% if (askyn("释放该用户吗?", NA, NA) == NA)
			if (askyn("\xca\xcd\xb7\xc5\xb8\xc3\xd3\xc3\xbb\xa7\xc2\xf0?", NA, NA) == NA)
				return 1;
			deny_release(line);
			break;
		case Ctrl('A'):
		case KEY_RIGHT:
			if (!line)
				return 0;
			char id[IDLEN + 1];
			strlcpy(id, line, sizeof(id));
			strtok(id, " \n\r\t");
			t_query(id);
			break;
		default:
			return 0;
	}
	return 1;
}

/**
 * Entrance to a board-wide ban list.
 * @return Whether redraw the screen.
 */
int deny_user(void)
{
	if (!am_curr_bm())
		return DONOTHING;
	char buf[HOMELEN];
	setbfile(buf, currboard, "deny_users");
	list_text(buf, deny_title_show, deny_key_deal, NULL);
	return FULLUPDATE;
}

/**
 * Compare two dates. DENY_FOREVER / DENY_TEMP > Others.
 * @param s1 String 1.
 * @param s2 String 2.
 * @return an integer less than, equal to or greater than 0 when s1
 *         is less than, matches to or is greater than s2.
 */
static int deny_date_cmp(const char *s1, const char *s2)
{
	//% "待定" "终身"
	if ((strneq2(s1, "\xb4\xfd\xb6\xa8") || strneq2(s1, "\xd6\xd5\xc9\xed"))
			&& (strneq2(s2, "\xb4\xfd\xb6\xa8")
				|| strneq2(s2, "\xd6\xd5\xc9\xed"))) {
		return 0;
	}
	return strncmp(s1, s2, 10);
}

/**
 * Get permission type according to description or user choice.
 * @param[in] orig The desciption.
 * @param[in] type The choice, used when provided no description.
 * @param[out] buf Buffer to return description.
 * @param[in] size The size of the buf.
 * @return respective permission bit, 0 on error.
 */
static unsigned int denylist_get_type(const char *orig, int type,
		char *buf, size_t size)
{
	//% const char *desc[] = {"发文", "上站", "聊天", "发信"};
	const char *desc[] = {"\xb7\xa2\xce\xc4", "\xc9\xcf\xd5\xbe", "\xc1\xc4\xcc\xec", "\xb7\xa2\xd0\xc5"};
	unsigned int perm[] = {PERM_POST, PERM_LOGIN, PERM_TALK, PERM_MAIL};
	if (orig) {
		if (!strncmp(orig, desc[0], strlen(desc[0])))
			type = '1';
		else if (!strncmp(orig, desc[1], strlen(desc[1])))
			type = '2';
		else if (!strncmp(orig, desc[2], strlen(desc[2])))
			type = '3';
		else if (!strncmp(orig, desc[3], strlen(desc[3])))
			type = '4';
		else
			return 0;
	}
	type -= '1';
	strlcpy(buf, desc[type], size);
	return perm[type];
}

/**
 * Release a user from site-wide ban.
 * @param[in] line Current line.
 */
static void denylist_release(const char *line)
{
	char id[IDLEN + 1];
	strlcpy(id, line, sizeof(id));
	strtok(id, " \r\n\t");

	char desc[8];
	unsigned int perm =
			denylist_get_type(line + OFFSET_PERM, 0, desc, sizeof(desc));
	if (!perm)
		return;
	// Re-grant permission.
	struct userec urec;
	int uid = getuserec(id, &urec);
	urec.userlevel |= perm;
	substitut_record(PASSFILE, &urec, sizeof(urec), uid);

	// Mail the released user.
	char file[HOMELEN];
	file_temp_name(file, sizeof(file));
	FILE *fp = fopen(file, "w");
	if (fp) {
		//% fprintf(fp, "执行人: %s\n", currentuser.userid);
		fprintf(fp, "\xd6\xb4\xd0\xd0\xc8\xcb: %s\n", currentuser.userid);
		fclose(fp);
	}
	char title[STRLEN];
	//% snprintf(title, sizeof(title), "[站内公告]恢复%s的%s权限",
	snprintf(title, sizeof(title), "[\xd5\xbe\xc4\xda\xb9\xab\xb8\xe6]\xbb\xd6\xb8\xb4%s\xb5\xc4%s\xc8\xa8\xcf\xde",
			urec.userid, desc);
	mail_file(file, urec.userid, title);
	Postfile(file, "Notice", title, 1);

	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "released %s", id);
	report(buf, currentuser.userid);
}

/**
 * Check site-wide ban list for expired users and release them.
 * @param file The ban list.
 * @param line User on current line will be released,
               all expired users if NULL.
 * @return 0 if OK, -1 on error.
 */
static int denylist_remove(const char *file, const char *line)
{
	FILE *fpr = fopen(file, "r");
	if (!fpr)
		return -1;

	char fnew[HOMELEN];
	snprintf(fnew, sizeof(fnew), "%s.%d", file, session_get_pid());
	FILE *fpw = fopen(fnew, "w");
	if (!fpw) {
		fclose(fpr);
		return -1;
	}

	char buf[LINE_BUFSIZE];
	if (line) {
		bool deleted = false;
		while (fgets(buf, sizeof(buf), fpr)) {
			if (!deleted && !strcmp(buf, line)) {
				deleted = true;
				denylist_release(line);
			} else {
				fputs(buf, fpw);
			}
		}
	} else {
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		char today[11];
		snprintf(today, sizeof(today), "%04d.%02d.%02d",
				1900 + t->tm_year, t->tm_mon + 1, t->tm_mday);
		while (fgets(buf, sizeof(buf), fpr)) {
			if (deny_date_cmp(today, buf + OFFSET_DATE) > 0)
				denylist_release(buf);
			else
				fputs(buf, fpw);
		}
	}
	bool empty = (ftell(fpw) <= 0);
	fclose(fpw);
	fclose(fpr);
	if (empty) {
		unlink(fnew);
		return unlink(file);
	}
	return rename(fnew, file);
}

/**
 * Ban a user site-wide or change an existing ban.
 * @param[in] userid The user.
 * @param[in] type The type of the permission banned.
 * @param[in] days Days to ban.
 * @param[in] ps PS.
 * @param[in] orig Origin record of permission type, NULL if adding a new one.
 * @return 0 on success, -1 on error.
 */
static int denylist_do_add(const char *userid, int type, int days,
		const char *ps, const char *orig)
{
	char desc[8];
	unsigned int perm = denylist_get_type(orig, type, desc, sizeof(desc));
	if (!perm)
		return -1;

	if (days > DENY_FOREVER)
		days = DENY_FOREVER;

	// Deprive permission.
	struct userec urec;
	int uid = getuserec(userid, &urec);
	if (!uid)
		return -1;
	urec.userlevel &= ~perm;
	substitut_record(PASSFILE, &urec, sizeof(urec), uid);

	// Cancel giving up BBS.
	char file[HOMELEN];
	sethomefile(file, urec.userid, "giveupBBS");
	unlink(file);

	// Generate notification.
	file_temp_name(file, sizeof(file));
	FILE *fpw = fopen(file, "w");
	//% fprintf(fpw, "%s因:\n", urec.userid);
	fprintf(fpw, "%s\xd2\xf2:\n", urec.userid);
	deny_generate(fpw, DENY_LEVEL_FILE);

	char str[STRLEN], title[STRLEN];
	if (days == DENY_FOREVER) {
		//% snprintf(str, sizeof(str), "%-12s %-34s %-4s 终身       %-12s\n",
		snprintf(str, sizeof(str), "%-12s %-34s %-4s \xd6\xd5\xc9\xed       %-12s\n",
				urec.userid, reason, desc, currentuser.userid);
		//% snprintf(title, sizeof(title), "[站内公告]封禁%s%s权限终身",
		snprintf(title, sizeof(title), "[\xd5\xbe\xc4\xda\xb9\xab\xb8\xe6]\xb7\xe2\xbd\xfb%s%s\xc8\xa8\xcf\xde\xd6\xd5\xc9\xed",
				urec.userid, desc);
		//% fprintf(fpw, "\n应被封禁 %s 权限终身\n", desc);
		fprintf(fpw, "\n\xd3\xa6\xb1\xbb\xb7\xe2\xbd\xfb %s \xc8\xa8\xcf\xde\xd6\xd5\xc9\xed\n", desc);
	} else if (days > DENY_TEMP) {
		time_t date = time(NULL) + days * 24 * 60 * 60;
		struct tm *t = localtime(&date);
		snprintf(str, sizeof(str), "%-12s %-34s %-4s %04d.%02d.%02d %-12s\n",
				urec.userid, reason, desc, 1900 + t->tm_year,
				t->tm_mon + 1, t->tm_mday, currentuser.userid);
		//% snprintf(title, sizeof(title), "[站内公告]封禁%s%s权限%d天",
		snprintf(title, sizeof(title), "[\xd5\xbe\xc4\xda\xb9\xab\xb8\xe6]\xb7\xe2\xbd\xfb%s%s\xc8\xa8\xcf\xde%d\xcc\xec",
				urec.userid, desc, days);
		//% fprintf(fpw, "\n应被封禁 %s 权限 %d 天\n\n"
		fprintf(fpw, "\n\xd3\xa6\xb1\xbb\xb7\xe2\xbd\xfb %s \xc8\xa8\xcf\xde %d \xcc\xec\n\n"
				//% "请在处罚期满后(%04d.%02d.%02d)，向%s写信要求解除处罚。",
				"\xc7\xeb\xd4\xda\xb4\xa6\xb7\xa3\xc6\xda\xc2\xfa\xba\xf3(%04d.%02d.%02d)\xa3\xac\xcf\xf2%s\xd0\xb4\xd0\xc5\xd2\xaa\xc7\xf3\xbd\xe2\xb3\xfd\xb4\xa6\xb7\xa3\xa1\xa3",
				desc, days, 1900 + t->tm_year, t->tm_mon + 1,
				t->tm_mday, currentuser.userid);
	} else {
		//% snprintf(str, sizeof(str), "%-12s %-34s %-4s 待定       %-12s\n",
		snprintf(str, sizeof(str), "%-12s %-34s %-4s \xb4\xfd\xb6\xa8       %-12s\n",
				urec.userid, reason, desc, currentuser.userid);
		//% snprintf(title, sizeof(title), "[站内公告]暂时封禁%s%s权限",
		snprintf(title, sizeof(title), "[\xd5\xbe\xc4\xda\xb9\xab\xb8\xe6]\xd4\xdd\xca\xb1\xb7\xe2\xbd\xfb%s%s\xc8\xa8\xcf\xde",
				urec.userid, desc);
		//% fprintf(fpw, "\n暂时被封禁 %s 权限\n\n处罚决定稍后作出。\n", desc);
		fprintf(fpw, "\n\xd4\xdd\xca\xb1\xb1\xbb\xb7\xe2\xbd\xfb %s \xc8\xa8\xcf\xde\n\n\xb4\xa6\xb7\xa3\xbe\xf6\xb6\xa8\xc9\xd4\xba\xf3\xd7\xf7\xb3\xf6\xa1\xa3\n", desc);
	}
	//% fprintf(fpw, "\n如不服本决定，可以联系处罚决定人或在7日内到Appeal版"
	fprintf(fpw, "\n\xc8\xe7\xb2\xbb\xb7\xfe\xb1\xbe\xbe\xf6\xb6\xa8\xa3\xac\xbf\xc9\xd2\xd4\xc1\xaa\xcf\xb5\xb4\xa6\xb7\xa3\xbe\xf6\xb6\xa8\xc8\xcb\xbb\xf2\xd4\xda""7\xc8\xd5\xc4\xda\xb5\xbd""Appeal\xb0\xe6"
			//% "申请复议。\nP.S.: %s\n\n执行人: %s\n", ps, currentuser.userid);
			"\xc9\xea\xc7\xeb\xb8\xb4\xd2\xe9\xa1\xa3\nP.S.: %s\n\n\xd6\xb4\xd0\xd0\xc8\xcb: %s\n", ps, currentuser.userid);
	fclose(fpw);
	if (orig) {
		//% snprintf(title, sizeof(title), "[站内公告]修改%s的封禁时间",
		snprintf(title, sizeof(title), "[\xd5\xbe\xc4\xda\xb9\xab\xb8\xe6]\xd0\xde\xb8\xc4%s\xb5\xc4\xb7\xe2\xbd\xfb\xca\xb1\xbc\xe4",
				urec.userid);
	}

	// Modify ban list.
	FILE *fpr = fopen(DENY_LEVEL_LIST, "r");
	char list[HOMELEN];
	sprintf(list, DENY_LEVEL_LIST".%d", session_get_pid());
	fpw = fopen(list, "w");
	size_t len = strlen(urec.userid);
	char buf[256];
	if (fpw) {
		bool added = false;
		if (fpr) {
			while (fgets(buf, sizeof(buf), fpr)) {
				if (buf[len] == ' ' && !strncmp(buf, urec.userid, len)
						&& !strncmp(buf + OFFSET_PERM, desc, strlen(desc))) {
					continue;
				}
				if (!added && deny_date_cmp(str + OFFSET_DATE,
						buf + OFFSET_DATE) < 0) {
					added = true;
					fputs(str, fpw);
				}
				fputs(buf, fpw);
			}
		}
		if (!added)
			fputs(str, fpw);
		fclose(fpw);
	}
	if (fpr)
		fclose(fpr);
	rename(list, DENY_LEVEL_LIST);

	Postfile(file, "Notice", title, 1);
	mail_file(file, urec.userid, title);
	unlink(file);
	return 0;
}

/**
 * UI for banning a user site-wide
 * @param line Current line.
 * @return No sense now.
 */
// TODO: Write lock?
static int denylist_add(const char *line)
{
	char user[IDLEN + 1], ans[7];
	int type = 0;

	screen_move(1, 0);
	if (line != NULL) {
		strlcpy(user, line, sizeof(user));
		strtok(user, " \n\r\t");
		reason_copy(line + IDLEN + 1);
		//% prints("修改[%s]的封禁时间\n", user);
		prints("\xd0\xde\xb8\xc4[%s]\xb5\xc4\xb7\xe2\xbd\xfb\xca\xb1\xbc\xe4\n", user);
	} else {
		user_complete(1, "封禁使用者: ", user, sizeof(user));
		if (*user == '\0')
			return 0;
		reason_copy(DEFAULT_REASON);
		reason_select(DENY_LEVEL_FILE);
		screen_clear();
		screen_move(2, 0);
		//% prints("封禁用户[%s]\n\n(1)发文 (2)登录 (3)聊天 (4)发信\n", user);
		prints("\xb7\xe2\xbd\xfb\xd3\xc3\xbb\xa7[%s]\n\n(1)\xb7\xa2\xce\xc4 (2)\xb5\xc7\xc2\xbc (3)\xc1\xc4\xcc\xec (4)\xb7\xa2\xd0\xc5\n", user);
		while (!type) {
			screen_move(12, 0);
			screen_clrtobot();
			//% getdata(10, 0, "请输入你的处理: ", ans, 3, DOECHO, YEA);
			getdata(10, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xe3\xb5\xc4\xb4\xa6\xc0\xed: ", ans, 3, DOECHO, YEA);
			if (ans[0] >= '1' && ans[1] <= '4')
				type = ans[0];
		}
	}
	//% getdata(11, 0, "封禁天数(999-终身, 0-待定): ", ans, 5, DOECHO, YEA);
	getdata(11, 0, "\xb7\xe2\xbd\xfb\xcc\xec\xca\xfd(999-\xd6\xd5\xc9\xed, 0-\xb4\xfd\xb6\xa8): ", ans, 5, DOECHO, YEA);
	int days = strtol(ans, NULL, 10);
	char ps[40];
	//% getdata(12, 0, "输入说明: ", ps, sizeof(ps), DOECHO, YEA);
	getdata(12, 0, "\xca\xe4\xc8\xeb\xcb\xb5\xc3\xf7: ", ps, sizeof(ps), DOECHO, YEA);
	screen_move(13, 0);
	//% if (askyn("您确定吗?", NA, NA) == NA)
	if (askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xc2\xf0?", NA, NA) == NA)
		return 1;

	const char *orig = line ? line + OFFSET_PERM : NULL;
	denylist_do_add(user, type, days, ps, orig);
	return 1;
}

/**
 * Key handler for manipulating a site-wide ban list.
 * @param file The list.
 * @param ch The key.
 * @param line Current line.
 * @return Whether redraw the screen.
 */
int denylist_key_deal(const char *file, int ch, const char *line)
{
	switch (ch) {
		case 'a':
			denylist_add(NULL);
			break;
		case 'c':
			if (!line)
				break;
			screen_clear();
			denylist_add(line);
			break;
		case 'd':
			if (!line)
				break;
			//% if (askyn("您确定吗?", NA, YEA) == NA) {
			if (askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xc2\xf0?", NA, YEA) == NA) {
				tui_update_status_line();
				return 0;
			}
			denylist_remove(file, line);
			break;
		case 'x':
			//% if (askyn("您确定吗?", NA, YEA) == NA) {
			if (askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xc2\xf0?", NA, YEA) == NA) {
				tui_update_status_line();
				return 0;
			}
			denylist_remove(file, NULL);
			break;
		case Ctrl('A'):
		case KEY_RIGHT:
			if (!line)
				return 0;
			t_query(line);
			break;
		default:
			return 0;
	}
	return 1;
}
