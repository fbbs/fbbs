#include "bbs.h"
#include "screen.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern int RMSG;
extern int msg_num;
struct user_info *t_search();

char buf2[MAX_MSG_SIZE+2];

//»ñÈ¡msg¼ÇÂ¼ÊýÄ¿
int get_num_msgs(const char *filename)
{
	int i = 0;
	struct stat st;
	if(stat(filename, &st) == -1)
		return 0;
	char head[256], msg[MAX_MSG_SIZE+2];
	FILE *fp;
	if((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (true) {
		if(fgets(head, sizeof(head), fp) != NULL && fgets(msg, MAX_MSG_SIZE + 2, fp) != NULL)
			i++;
		else
			break;
	}
	fclose(fp);
	return i;
}

int get_msg(const char *uid, char *msg, int line)
{
	char buf[3];
	int gdata;
	int msg_line;
	move(line, 0);
	clrtoeol();
	prints("ËÍÒôÐÅ¸ø£º%s  °´Ctrl+QÖØÐ´µ±Ç°ÏûÏ¢.     ÒôÐÅ:", uid);
	msg[0] = 0;
	while (true) {
		msg_line = multi_getdata(line + 1, 0, LINE_LEN - 1, NULL, msg, MAX_MSG_SIZE + 1, MAX_MSG_LINE, 0, 0);
		if (msg[0] == '\0')
			return NA;
		gdata = getdata(line + 4, 0, "È·¶¨ÒªËÍ³öÂð(Y)ÊÇµÄ (N)²»Òª (E)ÔÙ±à¼­? [Y]: ",
				buf, 2, DOECHO, YEA);
		if (gdata == -1)
			return NA;
		if (buf[0] == 'e' || buf[0] == 'E')
			continue;
		if (buf[0] == 'n' || buf[0] == 'N')
			return NA;
		else
			return YEA;
	} //while
}

char msgchar(struct user_info *uin)
{
	if (uin->mode == FIVE|| uin->mode == BBSNET || uin->mode == LOCKSCREEN) 
		return '@';
	if (isreject(uin))
		return '*';
	if ((uin->pager & ALLMSG_PAGER))
		return ' ';
	if (hisfriend(uin)) {
		if ((uin->pager & FRIENDMSG_PAGER))
			return 'O';
		else
			return '#';
	}
	return '*';
}

int canmsg(const struct user_info *uin)
{
	if (isreject(uin))
		return NA;
	if ((uin->pager & ALLMSG_PAGER) || HAS_PERM(PERM_OCHAT))
		return YEA;
	if ((uin->pager & FRIENDMSG_PAGER) && hisfriend(uin))
		return YEA;
	return NA;
}

void s_msg(void)
{
	do_sendmsg(NULL, NULL, 0, 0);
}

int send_msg(int ent, const struct fileheader *fileinfo, char *direct)
{
	struct user_info* uin;
	if (!strcmp(currentuser.userid,"guest"))
		return DONOTHING;
	uin = t_search(fileinfo->owner, NA);
	if (uin == NULL || (uin->invisible && !HAS_PERM(PERM_SEECLOAK))) {
		move(2, 0);
		prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ...\n");
		pressreturn();
	} else {
		do_sendmsg(uin, NULL, 0, uin->pid);
	}
	return FULLUPDATE;
}

int do_sendmsg(const struct user_info *uentp, const char *msgstr, int mode, int userpid)
{
	char uident[STRLEN], ret_str[20];
	time_t now;
	const struct user_info *uin;
	char buf[MAX_MSG_SIZE+2], *msgbuf, *timestr;
#ifdef LOG_MY_MESG
	char *mymsg, buf2[MAX_MSG_SIZE+2];
	int ishelo = 0; /* ÊÇ²»ÊÇºÃÓÑÉÏÕ¾Í¨ÖªÑ¶Ï¢ */
	mymsg = (char *) malloc(MAX_MSG_SIZE + 256);
#endif
	msgbuf = (char *) malloc(MAX_MSG_SIZE + 256);

	char wholebuf[MAX_MSG_SIZE+2];
	if (mode == 0) {
		move(2, 0);
		clrtobot();
		modify_user_mode(MSG);
	}
	if (uentp == NULL) {
		prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n");
		move(1, 0);
		clrtoeol();
		prints("ËÍÑ¶Ï¢¸ø: ");
		creat_list();
		namecomplete(NULL, uident);
		if (uident[0] == '\0') {
			clear();
			return 0;
		}
		if(!strcasecmp(uident, "guest")) {
			clear();
			return 0;
		}
		uin = t_search(uident, NA);
		if (uin == NULL) {
			move(2, 0);
			prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ...\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
		if (is_web_user(uin->mode) || uin->mode == BBSNET
				||uin->mode == LOCKSCREEN || uin->mode == PAGE
				|| uin->mode == FIVE || !canmsg(uin)) {
			move(2, 0);
			prints("Ä¿Ç°ÎÞ·¨´«ËÍÑ¶Ï¢¸ø¶Ô·½.\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
	} else {
		if (uentp->uid == usernum)
			return 0;
		uin = uentp;
		if (is_web_user(uin->mode) || uin->mode == BBSNET
				|| uin->mode == PAGE || uin->mode == LOCKSCREEN 
				|| uin->mode == FIVE || (mode != 2 && !canmsg(uin))) //add mode!=2 by quickmouse
			return 0;
		strcpy(uident, uin->userid);
	}
	if (msgstr == NULL) {
		if (!get_msg(uident, buf, 1)) {
			int i;
			for(i = 1; i <= MAX_MSG_LINE+ 1; i++) {
				move(i, 0);
				clrtoeol();
			}
			return 0;
		}
	}
	now = time(NULL);
	timestr = ctime(&now);
	strcpy(ret_str, "^Z»Ø");
	if (msgstr == NULL || mode == 2) {
		sprintf(msgbuf, "\033[0;1;44;36m%-12.12s\033[33m(\033[36m%-24.24s\033[33m):\033[37m%-34.34s\033[31m(%s)\033[m\033[%05dm\n", currentuser.userid, timestr , " ", ret_str, uinfo.pid);
		sprintf(wholebuf, "%s\n", msgstr == NULL ? buf : msgstr);
		strcat(msgbuf, wholebuf);
#ifdef LOG_MY_MESG
		sprintf(mymsg, "\033[1;32;40mTo \033[1;33;40m%-12.12s\033[m(%-24.24s):%-38.38s\n", uin->userid, timestr, " ");
		sprintf(wholebuf, "%s\n", msgstr == NULL ? buf : msgstr);
		strcat(mymsg, wholebuf);
		sprintf(buf2, "ÄãµÄºÃÅóÓÑ %s ÒÑ¾­ÉÏÕ¾ÂÞ£¡", currentuser.userid);
		if (msgstr != NULL)
			if (strcmp(msgstr, buf2) == 0)
				ishelo = 1;
			else if (strcmp(buf, buf2) == 0)
				ishelo = 1;
#endif
	} else if (mode == 0) {
		sprintf(msgbuf, "\033[0;1;5;44;33mÕ¾³¤ ÓÚ\033[36m %24.24s \033[33m¹ã²¥£º\033[m\033[1;37;44m%-39.39s\033[m\033[%05dm\n", timestr," ",  uinfo.pid); 
		sprintf(wholebuf, "%s\n", msgstr);
		strcat(msgbuf, wholebuf);        

	} else if (mode == 1) {
		sprintf(msgbuf, "\033[0;1;44;36m%-12.12s\033[37m(\033[36m%-24.24s\033[37m) ÑûÇëÄã\033[37m%-34.34s\033[31m(%s)\033[m\033[%05dm\n", currentuser.userid, timestr, " ", ret_str, uinfo.pid); 
		sprintf(wholebuf, "%s\n", msgstr); 
		strcat(msgbuf, wholebuf);
	} else if (mode == 3) {
		sprintf(msgbuf, "\033[0;1;45;36m%-12.12s\033[33m(\033[36m%-24.24s\033[33m):\033[37m%-34.34s\033[31m(%s)\033[m\033[%05dm\n", currentuser.userid, timestr, " ", ret_str, uinfo.pid); 
		sprintf(wholebuf, "%s\n", msgstr == NULL ? buf : msgstr);
		strcat(msgbuf, wholebuf);
	}
	else if (mode == 4) {
		sprintf(msgbuf, "\033[0;1;45;36m%-12.12s\033[36mÏòÄú¸æ±ð(\033[1;36;45m%24.24s\033[36m)£º\033[m\033[1;36;45m%-38.38s\033[m\033[%05dm\n", currentuser.userid, timestr, " ", 0); 
		sprintf(wholebuf, "%s\n", msgstr); 
		strcat(msgbuf, wholebuf);
	}
	if (userpid) {
		if (userpid != uin->pid) {
			saveline(0, 0);	/* Save line */
			move(0, 0);
			clrtoeol();
			prints("[1m¶Ô·½ÒÑ¾­ÀëÏß...[m\n");
			sleep(1);
			saveline(0, 1);	/* restore line */
			return -1;
		}
	}
	if (!uin->active || bbskill(uin, 0) == -1) {
		if (msgstr == NULL) {
			prints("\n¶Ô·½ÒÑ¾­ÀëÏß...\n");
			pressreturn();
			clear();
		}
		return -1;
	}
	sethomefile(buf, uident, "msgfile");
	file_append(buf, msgbuf);
#ifdef LOG_MY_MESG
	if (mode == 2 || (mode == 0 && msgstr == NULL)) {
		if (ishelo == 0) {
			sethomefile(buf, currentuser.userid, "msgfile.me");
			file_append(buf, mymsg);
		}
	}
	sethomefile(buf, uident, "msgfile.me");
	file_append(buf, msgbuf);
	free(mymsg);
#endif
	free(msgbuf);
	if(uin->pid) {
		bbskill(uin, SIGUSR2);
	}
	if (msgstr == NULL) {
		prints("\nÒÑËÍ³öÑ¶Ï¢...\n");
		pressreturn();
		clear();
	}
	return 1;
}

int dowall(const struct user_info *uin)
{
	if (!uin->active || !uin->pid)
		return -1;
	move(1, 0);
	clrtoeol();
	prints("\033[1;32mÕý¶Ô %s ¹ã²¥.... Ctrl-D Í£Ö¹¶Ô´ËÎ» User ¹ã²¥¡£\033[m", uin->userid);
	refresh();
	do_sendmsg(uin, buf2, 0, uin->pid);
}

int myfriend_wall(const struct user_info *uin)
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))
		return -1;
	if (myfriend(uin->uid)) {
		move(1, 0);
		clrtoeol();
		prints("\033[1;32mÕýÔÚËÍÑ¶Ï¢¸ø %s...  \033[m", uin->userid);
		refresh();
		do_sendmsg(uin, buf2, 3, uin->pid);
	}
}

int hisfriend_wall_logout(const struct user_info *uin)
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid 
			|| isreject(uin) || !(uin->pager & LOGOFFMSG_PAGER))
		return -1;
	if (hisfriend(uin)) {
		refresh();
		do_sendmsg(uin, buf2, 4, uin->pid);
	}
}

int hisfriend_wall(const struct user_info *uin)
{
	if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || isreject(uin))
		return -1;
	if (hisfriend(uin)) {
		refresh();
		do_sendmsg(uin, buf2, 3, uin->pid);
	}
}

int friend_wall(void)
{
	char    buf[3];
	char    msgbuf[MAX_MSG_SIZE + 256], filename[80];
	time_t  now;
	char   *timestr;
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 8) = '\0';

	char wholebuf[MAX_MSG_SIZE+2];

	modify_user_mode(MSG);
	move(2, 0);
	clrtobot();
	getdata(1, 0, "ËÍÑ¶Ï¢¸ø [1] ÎÒµÄºÃÅóÓÑ£¬[2] ÓëÎÒÎªÓÑÕß: ",
			buf, 2, DOECHO, YEA);
	switch (buf[0]) {
		case '1':
			if (!get_msg("ÎÒµÄºÃÅóÓÑ", buf2, 1))
				return 0;
			if (apply_ulist(myfriend_wall) == -1) {
				move(2, 0);
				prints("ÏßÉÏ¿ÕÎÞÒ»ÈË\n");
				pressanykey();
			} else {
				sprintf(msgbuf, "\033[0;1;45;36mËÍÑ¶Ï¢¸øºÃÓÑ\033[33m(\033[36m%-24.24s\033[33m):\033[37m%-39.39s\033[31m(^Z»Ø)\033[m\033[%05dm\n", timestr," ",  uinfo.pid); 
				sprintf(wholebuf, "%s\n", buf2);
				strcat(msgbuf, wholebuf);
				setuserfile(filename, "msgfile.me");
				file_append(filename, msgbuf);
			}
			break;
		case '2':
			if (!get_msg("ÓëÎÒÎªÓÑÕß", buf2, 1))
				return 0;
			if (apply_ulist(hisfriend_wall) == -1) {
				move(2, 0);
				prints("ÏßÉÏ¿ÕÎÞÒ»ÈË\n");
				pressanykey();
			} else {
				sprintf(msgbuf, "[0;1;45;36mËÍ¸øÓëÎÒÎªÓÑ[33m([36m%-24.24s[33m):[37m%-39.39s[31m(^Z»Ø)[m[%05dm\n", timestr," ",  uinfo.pid); 
				sprintf(wholebuf, "%s\n", buf2);
				strcat(msgbuf, wholebuf);
				setuserfile(filename, "msgfile.me");
				file_append(filename, msgbuf);

			}
			break;
		default:
			return 0;
	}
	move(7, 0);
	prints("Ñ¶Ï¢´«ËÍÍê±Ï...");
	pressanykey();
	return 1;
}

int friend_login_wall(const struct user_info *pageinfo)
{
	char    msg[MAX_MSG_SIZE+2];
	int     x, y;
	if (!pageinfo->active || !pageinfo->pid || isreject(pageinfo))
		return 0;
	if (hisfriend(pageinfo)) {
		if (getuser(pageinfo->userid) <= 0)
			return 0;
		if (!(lookupuser.userdefine & DEF_LOGINFROM))
			return 0;
		if (pageinfo->uid ==usernum)
			return 0;
		/* edwardc.990427 ºÃÓÑÒþÉí¾Í²»ÏÔÊ¾ËÍ³öÉÏÕ¾Í¨Öª */
		if (pageinfo->invisible)
			return 0;
		getyx(&y, &x);
		if (y > 22) {
			pressanykey();
			move(7, 0);
			clrtobot();
		}
		prints("ËÍ³öºÃÓÑÉÏÕ¾Í¨Öª¸ø %s\n", pageinfo->userid);
		sprintf(msg, "ÄãµÄºÃÅóÓÑ %s ÒÑ¾­ÉÏÕ¾ÂÞ£¡", currentuser.userid);
		do_sendmsg(pageinfo, msg, 2, pageinfo->pid);
	}
	return 0;
}

enum {
	MSG_INIT, MSG_SHOW, MSG_REPLYING,
	MSG_BAK_THRES = 500,
};

/**
 *
 */
static int get_msg3(const char *user, int *num, char *head, size_t hsize,
		char *buf, size_t size)
{
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, "msgfile");
	int all = get_num_msgs(file);
	if (*num < 1)
		*num = 1;
	if (*num > all)
		*num = all;

	FILE *fp = fopen(file, "r");
	if (!fp)
		return 0;
	int j = all - *num;
	while (j-- >= 0) {
		if (!fgets(head, hsize, fp) || !fgets(buf, size, fp))
			break;
	}
	fclose(fp);

	if (j < 0) {
		char *ptr = strrchr(head, '[');
		if (!ptr)
			return 0;
		return strtol(ptr + 1, NULL, 10);
	}
	return 0;
}

static int show_msg(const char *user, const char *head, const char *buf, int line)
{
	if (!RMSG && DEFINE(DEF_SOUNDMSG))
		bell();
	move(line, 0);
	clrtoeol();

	// This is a temporary solution to Fterm & Cterm message recognition.
	char sender[IDLEN + 1], date[25];
	strlcpy(sender, head + 12, sizeof(sender));
	strlcpy(date, head + 35, sizeof(date));

	prints("\033[1;36;44m%s  \033[33m(%s)\033[37m", sender, date);
	move(line, 93);
	outs("\033[31m(^Z»Ø)\033[37m");
	move(++line, 0);
	clrtoeol();
	line = show_data(buf, LINE_LEN - 1, line, 0);
	move(line, 0);
	clrtoeol();
	prints("\033[mÁ¢¼´»ØÑ¶Ï¢¸ø %s", sender);
	move(++line, 0);
	clrtoeol();
	refresh();
	return line;
}

static int string_remove(char *str, int bytes)
{
	int len = strlen(str);
	if (len < bytes)
		return 0;
	memmove(str, str + bytes, len - bytes);
	str[len - bytes] = '\0';
	return bytes;
}

static int string_delete(char *str, int pos, bool backspace)
{
	bool ingbk = false;
	char *ptr = str + pos - (backspace ? 1 : 0);
	if (ptr < str)
		return 0;
	while (*str != '\0' && str != ptr) {
		if (ingbk)
			ingbk = false;
		else if (*str & 0x80)
			ingbk = true;
		str++;
	}
	if (ingbk) {
		return string_remove(str - 1, 2);
	} else {
		if (*str & 0x80)
			return string_remove(str, 2);
		else
			return string_remove(str, 1);
	}
}

static void pos_change(size_t max, int base, int width, int height,
		int *x, int *y, int change)
{
	int pos = (*y - base) * width + *x + change;
	if (pos < 0)
		pos = 0;
	if (pos > max)
		pos = max;
	if (pos > width * height)
		pos = width * height;
	*y = base + pos / width;
	*x = pos % width;
}

static void getdata_r(char *buf, size_t size, size_t *len,
		int ch, int base, int *ht)
{
	int pos, ret, redraw = false, x, y;
	getyx(&y, &x);
	pos = (y - base) * LINE_LEN + x;
	switch (ch) {
		case KEY_LEFT:
			pos_change(*len, base, LINE_LEN, *ht, &x, &y, -1);
			break;
		case KEY_RIGHT:
			pos_change(*len, base, LINE_LEN, *ht, &x, &y, 1);
			break;
		case KEY_DOWN:
			pos_change(*len, base, LINE_LEN, *ht, &x, &y, LINE_LEN);
			break;
		case KEY_UP:
			pos_change(*len, base, LINE_LEN, *ht, &x, &y, -LINE_LEN);
			break;
		case KEY_HOME:
			y = base;
			x = 0;
			break;
		case KEY_END:
			pos_change(*len, base, LINE_LEN, *ht, &x, &y, MAX_MSG_SIZE);
			break;
		case Ctrl('H'):
		case KEY_DEL:
			ret = string_delete(buf, pos, ch != KEY_DEL);
			if (ch != KEY_DEL)
				pos_change(*len, base, LINE_LEN, *ht, &x, &y, -ret);
			*len -= ret;
			redraw = true;
			break;
		default:
			if (*len < size - 1 && isprint2(ch)) {
				pos = (y - base) * LINE_LEN + x;
				if (pos < *len)
					memmove(buf + pos + 1, buf + pos, *len - pos);
				buf[pos] = ch;
				buf[*len + 1] = '\0';
				*ht = (*len)++ / LINE_LEN + 1;
				pos_change(*len, base, LINE_LEN, *ht, &x, &y, 1);
				redraw = true;
			}
			break;
	}
	move(y, x);
	if (redraw) {
		show_data(buf, LINE_LEN - 1, base, 0);
	}
}

/**
 *
 */
static void send_msg3(const char *receiver, int pid, const char *msg, int line)
{
	char buf[STRLEN];
	bool success = false;

	if (*msg != '\0') {
		struct user_info *uin = t_search(receiver, pid);
		if (!uin) {
			snprintf(buf, sizeof(buf), "\033[1;32mÕÒ²»µ½·¢Ñ¶Ï¢µÄ %s.\033[m",
					receiver);
		} else if (do_sendmsg(uin, msg, 2, uin->pid)) {
			success = true;
		} else {
			strlcpy(buf, "\033[1;32mÑ¶Ï¢ÎÞ·¨ËÍ³ö.\033[m", sizeof(buf));
		}
	} else {
		strlcpy(buf, "\033[1;33m¿ÕÑ¶Ï¢, ËùÒÔ²»ËÍ³ö.\033[m", sizeof(buf));
	}

	move(line, 0);
	clrtoeol();
	if (!success) {
		outs(buf);
		refresh();
		sleep(1);
	}

	int i;
	for (i = 0; i < MAX_MSG_LINE * 2 + 2; i++) {
		saveline_buf(i, 1);
	}
	refresh();
}

/**
 *
 */
static void msg_backup(const char *user)
{
	char file[HOMELEN];
	sethomefile(file, user, "msgfile.me");

	int num = get_num_msgs(file);
	if (num > MSG_BAK_THRES) {
		char title[STRLEN];
		snprintf(title, sizeof(title), "[%s] Ç¿ÖÆÑ¶Ï¢±¸·Ý%dÌõ",
				getdatestring(time(NULL), DATE_ZH), num);
		mail_file(file, user, title);
		unlink(file);
	}
}

typedef struct {
	int status;
	int x;
	int y;
	int cury;
	int height;
	int rpid;
	int num;
	int sa;
	size_t len;
	char msg[MAX_MSG_LINE * LINE_LEN + 1];
	char receiver[IDLEN + 1];
} msg_status_t;

static int msg_show(msg_status_t *st, char *head, size_t hsize,
		char *buf, size_t size)
{
	st->rpid = get_msg3(currentuser.userid, &st->num, head, hsize, buf, size);
	if (st->rpid) {
		strlcpy(st->receiver, head + 12, sizeof(st->receiver));
		strtok(st->receiver, " ");
		int line = (uinfo.mode == TALK ? t_lines / 2 - 1 : 0);
		st->cury = show_msg(currentuser.userid, head, buf, line);
	}
	st->status = MSG_REPLYING;
	return st->rpid;
}

int msg_reply(int ch)
{
	static msg_status_t st = { .status = MSG_INIT, .height = 1,
			.num = 0, .len = 0};

	int k;
	char buf[LINE_BUFSIZE], head[LINE_BUFSIZE];
		
	switch (st.status) {
		case MSG_INIT:
			getyx(&st.y, &st.x);
			st.sa = showansi;
			showansi = true;
			if (DEFINE(DEF_MSGGETKEY)) {
				for (k = 0; k < MAX_MSG_LINE * 2 + 2; k++)
					saveline_buf(k, 0);
			}
			if (RMSG)
				st.num++;
			else
				st.num = msg_num;
			// fall through
		case MSG_SHOW:
			msg_show(&st, head, sizeof(head), buf, sizeof(buf));
			ch = 0;
			// fall through
		case MSG_REPLYING:
			switch (ch) {
				case '\r':
				case '\n':
					send_msg3(st.receiver, st.rpid, st.msg, st.cury);
					msg_backup(currentuser.userid);
					st.msg[0] = '\0';
					st.len = 0;
					st.height = 1;
					st.status = MSG_SHOW;

					if (!RMSG) {
						msg_num--;
						st.num--;
					} else {
						RMSG = false;
						st.num = 0;
					}

					if (!msg_num) {
						st.status = MSG_INIT;
						for (k = 0; k < MAX_MSG_LINE + 2; k++)
							saveline_buf(k, 1);
						move(st.y, st.x);
						showansi = st.sa;
					} else {
						msg_show(&st, head, sizeof(head), buf, sizeof(buf));
					}
					break;
				case '\0':
					break;
				case Ctrl('Z'):
				case Ctrl('A'):
					st.num += (ch == Ctrl('Z') ? 1 : -1);
					if (st.num < 1)
						st.num = 1;
					st.msg[0] = '\0';
					st.len = 0;
					st.height = 1;
					msg_show(&st, head, sizeof(head), buf, sizeof(buf));
					break;
				default:
					getdata_r(st.msg, sizeof(st.msg), &st.len, ch, st.cury,
							&st.height);
					break;
			}
			break;
		default:
			break;
	}
	return 0;
}

void msg_handler(int signum)
{
	if (msg_num++ == 0)
		msg_reply(0);
}
