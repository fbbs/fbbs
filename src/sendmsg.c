#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

enum {
	NORMAL_MSG = 0,
	LOGIN_MSG,
	LOGOUT_MSG,
	BROADCAST_MSG,
};

typedef db_res_t msg_session_info_t;

#define MSG_SESSION_INFO_FIELDS  "s.id, s.pid, s.web, s.visible, s.user_name"
#define MSG_SESSION_INFO_QUERY  \
	"SELECT "MSG_SESSION_INFO_FIELDS " FROM sessions s"

#define msg_session_info_sid(s, i)  db_get_session_id(s, i, 0)
#define msg_session_info_pid(s, i)  db_get_integer(s, i, 1)
#define msg_session_info_web(s, i)  db_get_bool(s, i, 2)
#define msg_session_info_visible(s, i)  db_get_bool(s, i, 3)
#define msg_session_info_uname(s, i)  db_get_value(s, i, 4)

#define msg_session_info_count(s)  db_res_rows(s)
#define msg_session_info_clear(s)  db_clear(s)

extern int RMSG;
extern int msg_num;

static int get_num_msgs(const char *filename)
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
	screen_move_clear(line);
	//% prints("送音信给：%s  按Ctrl+Q重写当前消息.     音信:", uid);
	prints("\xcb\xcd\xd2\xf4\xd0\xc5\xb8\xf8\xa3\xba%s  \xb0\xb4""Ctrl+Q\xd6\xd8\xd0\xb4\xb5\xb1\xc7\xb0\xcf\xfb\xcf\xa2.     \xd2\xf4\xd0\xc5:", uid);
	msg[0] = 0;
	while (true) {
		multi_getdata(line + 1, 0, LINE_LEN - 1, NULL, msg, MAX_MSG_SIZE + 1, MAX_MSG_LINE, 0, 0);
		if (msg[0] == '\0')
			return NA;
		//% gdata = getdata(line + 4, 0, "确定要送出吗(Y)是的 (N)不要 (E)再编辑? [Y]: ",
		gdata = getdata(line + 4, 0, "\xc8\xb7\xb6\xa8\xd2\xaa\xcb\xcd\xb3\xf6\xc2\xf0(Y)\xca\xc7\xb5\xc4 (N)\xb2\xbb\xd2\xaa (E)\xd4\xd9\xb1\xe0\xbc\xad? [Y]: ",
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

static msg_session_info_t *get_msg_sessions(const char *user_name)
{
	user_id_t user_id = get_user_id(user_name);
	if (user_id <= 0)
		return NULL;
	return db_query(MSG_SESSION_INFO_QUERY " WHERE s.user_id = %"DBIdUID,
			user_id);
}

static void generate_full_msg(const char *msg, int type,
		char *full, size_t fsize)
{
	time_t now = time(NULL);
	const char *timestr = ctime(&now);
	//% const char *ret_str = "^Z回";
	const char *ret_str = "^Z\xbb\xd8";

	char head[MAX_MSG_SIZE + 2];
	switch (type) {
		case NORMAL_MSG:
		case LOGIN_MSG:
			snprintf(head, sizeof(head), "\033[0;1;44;36m%-12.12s\033[33m"
					"(\033[36m%-24.24s\033[33m):\033[37m%-34.34s"
					"\033[31m(%s)\033[m\033[%05dm\n",
					currentuser.userid, timestr , " ", ret_str, session_pid());
			break;
		case LOGOUT_MSG:
			snprintf(head, sizeof(head), "\033[0;1;45;36m%-12.12s\033[36m"
					//% "向您告别(\033[1;36;45m%24.24s\033[36m)：\033[m"
					"\xcf\xf2\xc4\xfa\xb8\xe6\xb1\xf0(\033[1;36;45m%24.24s\033[36m)\xa3\xba\033[m"
					"\033[1;36;45m%-38.38s\033[m\033[%05dm\n",
					currentuser.userid, timestr, " ", 0);
			break;
		case BROADCAST_MSG:
			//% snprintf(head, sizeof(head), "\033[1;5;44;33m站长 于\033[36m "
			snprintf(head, sizeof(head), "\033[1;5;44;33m\xd5\xbe\xb3\xa4 \xd3\xda\033[36m "
					//% "%24.24s \033[33m广播：\033[m\033[1;37;44m%-39.39s\033[m"
					"%24.24s \033[33m\xb9\xe3\xb2\xa5\xa3\xba\033[m\033[1;37;44m%-39.39s\033[m"
					"\033[%05dm\n", timestr," ",  session_pid());
			break;
	}
	snprintf(full, fsize, "%s%s\n", head, msg);
}

static void log_my_msg(const char *uname, const char *msg, int type)
{
#ifdef LOG_MY_MESG
	if (type == LOGIN_MSG || type == LOGOUT_MSG)
		return;

	time_t now = time(NULL);
	const char *timestr = ctime(&now);
	char buf[MAX_MSG_SIZE + 256];
	snprintf(buf, sizeof(buf), "\033[1;32;40mTo \033[1;33;40m%-12.12s\033[m"
			"(%-24.24s):%-38.38s\n%s\n", uname, timestr, " ", msg);

	char file[HOMELEN];
	sethomefile(file, currentuser.userid, "msgfile.me");
	file_append(file, buf);
#endif
	return;
}

static bool session_msgable(const msg_session_info_t *s, int i, int type)
{
	const char *uname = msg_session_info_uname(s, i);
	if (streq(uname, currentuser.userid))
		return false;

	if (!msg_session_info_visible(s, i) && !HAS_PERM(PERM_SEECLOAK))
		return false;

	if (msg_session_info_web(s, i))
		return false;

	int status = get_user_status(msg_session_info_sid(s, i));
	if (status == ST_BBSNET || status == ST_LOCKSCREEN)
		return false;

	if (type == LOGIN_MSG || type == LOGOUT_MSG) {
		struct userec receiver;
		if (!getuserec(uname, &receiver))
			return false;
		if (type == LOGIN_MSG
				&& !HAS_DEFINE(receiver.userdefine, DEF_LOGINFROM))
			return false;
		if (type == LOGOUT_MSG
				&& !HAS_DEFINE(receiver.userdefine, DEF_LOGOFFMSG))
			return false;
	}
	return true;
}

static bool user_msgable(const msg_session_info_t *s)
{
	if (!s || !msg_session_info_count(s))
		return false;

	const char *uname = msg_session_info_uname(s, 0);
	if (is_blocked(uname))
		return false;

	if (!HAS_PERM(PERM_OCHAT)) {
		if (!getuser(uname))
			return false;

		if (!HAS_DEFINE(lookupuser.userdefine, DEF_ALLMSG)) {
			if (!HAS_DEFINE(lookupuser.userdefine, DEF_FRIENDMSG)
				|| !am_followed_by(uname)) {
				return false;
			}
		}
	}

	for (int i = 0; i < msg_session_info_count(s); ++i) {
		if (session_msgable(s, i, NORMAL_MSG))
			return true;
	}
	return false;
}

static int send_one_msg(int pid, const char *uname, const char *full)
{
	char file[HOMELEN];
	sethomefile(file, uname, "msgfile");
	file_append(file, full);
	sethomefile(file, uname, "msgfile.me");
	file_append(file, full);

	return !bbs_kill(0, pid, SIGUSR2);
}

static int send_msg(const msg_session_info_t *s, const char *msg, int type)
{
	char full[MAX_MSG_SIZE + 2];
	generate_full_msg(msg, type, full, sizeof(full));

	int sent = 0;
	for (int i = 0; i < msg_session_info_count(s); ++i) {
		if (session_msgable(s, i, type)
				&& send_one_msg(msg_session_info_pid(s, i), 
					msg_session_info_uname(s, i), full))
			++sent;
	}

	if (sent && type == NORMAL_MSG)
		log_my_msg(msg_session_info_uname(s, 0), msg, type);

	return sent;
}

int tui_send_msg(const char *uname)
{
	char name[IDLEN + 1];
	if (!uname || !*uname) {
		//% prints("<输入使用者代号>\n");
		prints("<\xca\xe4\xc8\xeb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5>\n");
		screen_move_clear(1);
		//% prints("送讯息给: ");
		prints("\xcb\xcd\xd1\xb6\xcf\xa2\xb8\xf8: ");

		*name = '\0';
		usercomplete(NULL, name);
		uname = name;
	}

	if (!*uname || strcaseeq(uname, "guest")
			|| strcaseeq(uname, currentuser.userid)) {
		screen_clear();
		return 0;
	}

	msg_session_info_t *s = get_msg_sessions(uname);

	if (!user_msgable(s)) {
		msg_session_info_clear(s);
		screen_move(2, 0);
		//% prints("对方目前不在线或无法接受讯息...\n");
		prints("\xb6\xd4\xb7\xbd\xc4\xbf\xc7\xb0\xb2\xbb\xd4\xda\xcf\xdf\xbb\xf2\xce\xde\xb7\xa8\xbd\xd3\xca\xdc\xd1\xb6\xcf\xa2...\n");
		pressreturn();
		screen_move_clear(2);
		return 0;
	}

	char msg[MAX_MSG_SIZE + 2];
	if (!get_msg(uname, msg, 1)) {
		for (int i = 1; i <= MAX_MSG_LINE+ 1; i++) {
			screen_move_clear(i);
		}
		msg_session_info_clear(s);
		return 0;
	}

	int r = send_msg(s, msg, NORMAL_MSG);
	msg_session_info_clear(s);

	if (r)
		//% prints("\n已送出讯息...\n");
		prints("\n\xd2\xd1\xcb\xcd\xb3\xf6\xd1\xb6\xcf\xa2...\n");
	else
		//% prints("\033[1m对方已经离线...\033[m\n");
		prints("\033[1m\xb6\xd4\xb7\xbd\xd2\xd1\xbe\xad\xc0\xeb\xcf\xdf...\033[m\n");
	pressreturn();
	screen_clear();
	return r;
}

int s_msg(void)
{
	return tui_send_msg(NULL);
}

int msg_author(int ent, const struct fileheader *fileinfo, char *direct)
{
	if (streq(currentuser.userid, "guest"))
		return DONOTHING;
	tui_send_msg(fileinfo->owner);
	return FULLUPDATE;
}

int broadcast_msg(const char *msg)
{
	msg_session_info_t *all = db_query(MSG_SESSION_INFO_QUERY);
	int r = send_msg(all, msg, BROADCAST_MSG);
	msg_session_info_clear(all);
	return r;
}

static msg_session_info_t *get_sessions_of_followers(void)
{
	return db_query(MSG_SESSION_INFO_QUERY
			" JOIN follows f ON s.user_id = f.follower"
			" WHERE f.user_id = %"DBIdUID, session_uid());
}

int logout_msg(const char *msg)
{
	msg_session_info_t *s = get_sessions_of_followers();
	int r = send_msg(s, msg, LOGOUT_MSG);
	msg_session_info_clear(s);
	return r;
}

int login_msg(void)
{
	msg_session_info_t *s = get_sessions_of_followers();
	int r = send_msg(s, NULL, LOGIN_MSG);
	msg_session_info_clear(s);
	return r;
}

enum {
	MSG_INIT, MSG_SHOW, MSG_WAIT, MSG_REPLYING,
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

static int show_msg(const char *user, const char *head, const char *buf,
		int line, bool reply)
{
	if (!RMSG && !reply && DEFINE(DEF_SOUNDMSG))
		bell();
	screen_move_clear(line);

	// This is a temporary solution to Fterm & Cterm message recognition.
	char sender[IDLEN + 1], date[25];
	strlcpy(sender, head + 12, sizeof(sender));
	strlcpy(date, head + 35, sizeof(date));

	prints("\033[1;36;44m%s  \033[33m(%s)\033[37m", sender, date);
	screen_move(line, 93);
	//% outs("\033[31m(^Z回) \033[37m");
	outs("\033[31m(^Z\xbb\xd8) \033[37m");
	screen_move_clear(++line);
	line = show_data(buf, LINE_LEN - 1, line, 0);
	screen_move_clear(line);
	if (!reply) {
		//% outs("\033[m按^Z回复");
		outs("\033[m\xb0\xb4^Z\xbb\xd8\xb8\xb4");
	} else {
		//% prints("\033[m立即回讯息给 %s", sender);
		prints("\033[m\xc1\xa2\xbc\xb4\xbb\xd8\xd1\xb6\xcf\xa2\xb8\xf8 %s", sender);
		screen_move_clear(++line);
	}
	screen_flush();
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
	screen_coordinates(&y, &x);
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
	screen_move(y, x);
	if (redraw) {
		show_data(buf, LINE_LEN - 1, base, 0);
	}
}

static void _msg_reply(const char *receiver, int pid, const char *msg, int line)
{
	char buf[STRLEN];
	bool success = false;

	if (*msg != '\0') {
		msg_session_info_t *s = get_msg_sessions(receiver);
		if (!user_msgable(s)) {
			//% snprintf(buf, sizeof(buf), "\033[1;32m找不到发讯息的 %s.\033[m",
			snprintf(buf, sizeof(buf), "\033[1;32m\xd5\xd2\xb2\xbb\xb5\xbd\xb7\xa2\xd1\xb6\xcf\xa2\xb5\xc4 %s.\033[m",
					receiver);
		} else if (send_msg(s, msg, NORMAL_MSG)) {
			success = true;
		} else {
			//% strlcpy(buf, "\033[1;32m讯息无法送出.\033[m", sizeof(buf));
			strlcpy(buf, "\033[1;32m\xd1\xb6\xcf\xa2\xce\xde\xb7\xa8\xcb\xcd\xb3\xf6.\033[m", sizeof(buf));
		}
	} else {
		//% strlcpy(buf, "\033[1;33m空讯息, 所以不送出.\033[m", sizeof(buf));
		strlcpy(buf, "\033[1;33m\xbf\xd5\xd1\xb6\xcf\xa2, \xcb\xf9\xd2\xd4\xb2\xbb\xcb\xcd\xb3\xf6.\033[m", sizeof(buf));
	}

	screen_move_clear(line);
	if (!success) {
		outs(buf);
		screen_flush();
		sleep(1);
	}

	int i;
	for (i = 0; i < MAX_MSG_LINE * 2 + 2; i++) {
		saveline_buf(i, 1);
	}
	screen_flush();
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
		//% snprintf(title, sizeof(title), "[%s] 强制讯息备份%d条",
		snprintf(title, sizeof(title), "[%s] \xc7\xbf\xd6\xc6\xd1\xb6\xcf\xa2\xb1\xb8\xb7\xdd%d\xcc\xf5",
				format_time(fb_time(), TIME_FORMAT_ZH), num);
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
		int line = (session_status() == ST_TALK ? screen_lines() / 2 - 1 : 0);
		st->cury = show_msg(currentuser.userid, head, buf, line,
				st->status == MSG_REPLYING);
	}
	if (st->status < MSG_WAIT)
		st->status = MSG_WAIT;
	return st->rpid;
}

static int msg_next(msg_status_t *st, char *head, size_t hsize,
		char *buf, size_t size)
{
	if (!RMSG) {
		msg_num--;
		st->num--;
	} else {
		RMSG = false;
		st->num = 0;
	}
	
	if (!msg_num) {
		st->status = MSG_INIT;
		for (int k = 0; k < MAX_MSG_LINE + 2; k++)
			saveline_buf(k, 1);
		screen_move(st->y, st->x);
		showansi = st->sa;
	} else {
		msg_show(st, head, hsize, buf, size);
	}
	return 0;
}

void msg_reply(int ch)
{
	static msg_status_t st = { .status = MSG_INIT, .height = 1,
			.num = 0, .len = 0};

	int k;
	char buf[LINE_BUFSIZE], head[LINE_BUFSIZE];
		
	switch (st.status) {
		case MSG_INIT:
			screen_coordinates(&st.y, &st.x);
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
			if (!msg_show(&st, head, sizeof(head), buf, sizeof(buf))) {
				msg_next(&st, head, sizeof(head), buf, sizeof(buf));
				break;
			}
			// fall through
		case MSG_WAIT:
			switch (ch) {
				case Ctrl('Z'):
					st.status = MSG_REPLYING;
					msg_show(&st, head, sizeof(head), buf, sizeof(buf));
					ch = 0;
					break;
				case '\r':
				case '\n':
					msg_next(&st, head, sizeof(head), buf, sizeof(buf));
					return;
				default:
					return;
			}
			// fall through
		case MSG_REPLYING:
			switch (ch) {
				case '\r':
				case '\n':
					_msg_reply(st.receiver, st.rpid, st.msg, st.cury);
					msg_backup(currentuser.userid);
					st.msg[0] = '\0';
					st.len = 0;
					st.height = 1;
					st.status = MSG_SHOW;
					msg_next(&st, head, sizeof(head), buf, sizeof(buf));
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
}

void msg_handler(int signum)
{
	if (msg_num++ == 0)
		msg_reply(0);
}
