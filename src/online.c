#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

enum {
	REFRESH_TIME = 30,
};

typedef struct {
	session_id_t sid;
	const char *name;
	const char *nick;
	const char *note;
	const char *host;
	user_id_t uid;
	int pid;
	int idle;
	int status;
	int flag;
} online_user_info_t;

typedef struct {
	db_res_t *res;
	fb_time_t uptime;
	online_user_info_t *users;
	int num;
	int bid;
	bool follow;
	bool show_note;
} online_users_t;

static void _fill_session_array(tui_list_t *p, int i)
{
	online_users_t *up = p->data;
	db_res_t *res = up->res;

	bool visible = db_get_bool(res, i, 3);
	if (!visible && !HAS_PERM(PERM_SEECLOAK))
		return;

	session_id_t sid = db_get_session_id(res, i, 0);
	if (up->bid && get_current_board(sid) != up->bid)
		return;

	online_user_info_t *ip = up->users + up->num;
	memset(ip, 0, sizeof(*ip));

	ip->sid = sid;
	ip->name = db_get_value(res, i, 2);
	ip->host = db_get_value(res, i, 4);
	ip->note = up->follow ? db_get_value(res, i, 6) : NULL;
	ip->uid = db_get_user_id(res, i, 1);
	ip->flag = (db_get_bool(res, i, 5) ? SESSION_FLAG_WEB : 0)
			| (visible ? 0 : SESSION_FLAG_INVISIBLE);

	++up->num;
}

static int session_cmp(const void *s1, const void *s2)
{
	const online_user_info_t *i1 = s1, *i2 = s2;
	int diff = strcmp(i1->name, i2->name);
	if (diff)
		return diff;
	else
		return i1->sid - i2->sid;
}

static tui_list_loader_t fill_session_array(tui_list_t *p)
{
	online_users_t *up = p->data;
	db_res_t *res = up->res;

	p->all = up->num = 0;

	if (!res || db_res_rows(res) < 1) {
		free(up->users);
		up->users = NULL;
		return 0;
	}

	int count = db_res_rows(res);
	up->users = malloc(count * sizeof(*up->users));
	for (int i = 0; i < count; ++i) {
		_fill_session_array(p, i);
	}
	qsort(up->users, count, sizeof(*up->users), session_cmp);

	return p->all = up->num;
}

static tui_list_loader_t online_users_load_followings(tui_list_t *p)
{
	online_users_t *up = p->data;
	up->res = get_sessions_of_followings();
	return fill_session_array(p);
}

static tui_list_loader_t online_users_load_board(tui_list_t *p)
{
	// TODO: load from redis first
	online_users_t *up = p->data;
	up->res = get_active_sessions();
	return fill_session_array(p);
}

static tui_list_loader_t online_users_load_all(tui_list_t *p)
{
	online_users_t *up = p->data;
	up->res = get_active_sessions();
	return fill_session_array(p);
}

static void online_users_free(online_users_t *up)
{
	db_clear(up->res);
	free(up->users);
	up->users = NULL;
}

static tui_list_loader_t online_users_load(tui_list_t *p)
{
	online_users_t *up = p->data;

	fb_time_t now = time(NULL);
	if (now < up->uptime + REFRESH_TIME)
		return up->num;
	up->uptime = now;

	online_users_free(up);

	if (up->follow) {
		return online_users_load_followings(p);
	} else {
		if (up->bid)
			return online_users_load_board(p);
		else
			return online_users_load_all(p);
	}
}

static tui_list_title_t online_users_title(tui_list_t *p)
{
	online_users_t *up = p->data;
	docmdtitle(up->follow ? "[关注列表]" : "[使用者列表]",
			" 寄信[\033[1;32mm\033[m] 送讯息[\033[1;32ms\033[m]"
			" 加,减关注[\033[1;32mo\033[m,\033[1;32md\033[m]"
			" 看说明档[\033[1;32m→\033[m,\033[1;32mRtn\033[m]"
			" 切换模式 [\033[1;32mf\033[m] 求救[\033[1;32mh\033[m]");
	
	const char *field = up->show_note ? "备注" : "昵称";

	char title[256];
	snprintf(title, sizeof(title), "\033[1;44m 编号  使用者代号   %s        "
			"         上站位置            目前动态   发呆   \033[m\n",
			field);

	move(2, 0);
	clrtoeol();
	prints("%s", title);
}

static void get_nick(char *nick, size_t size, const online_users_t *up,
		const online_user_info_t *ip)
{
	if (up->follow && up->show_note) {
		strlcpy(nick, ip->note, size);
	} else {
		if (!ip->nick) {
			getuser(ip->name);
			strlcpy(nick, lookupuser.username, size);
			// don't save nick here
		} else {
			strlcpy(nick, ip->nick, size);
		}
	}
	ellipsis(nick, 20);
}

static const char *get_host(const online_user_info_t *ip)
{
	const char *host;
	if (HAS_PERM(PERM_OCHAT)) {
		host = ip->host;
	} else {
		host = mask_host(ip->host);
	}
	return host;
}

const char *get_status_color(int status, bool visible, bool web)
{
	if (!visible)
		return "\033[1;30m";
	else if (web)
		return "\033[36m";
	else if (status == ST_POSTING || status == ST_MARKET)
		return "\033[32m";
	else if (status == ST_FIVE || status == ST_BBSNET)
		return "\033[33m";
	else
		return "";
}

static void get_idle_str(char *buf, size_t size, fb_time_t refresh, int status)
{
	int idle = 0;
	if (refresh > 0 && status != ST_BBSNET)
		idle = (time(NULL) - refresh) / 60;

	if (!idle) {
		*buf = '\0';
	} else {
		if (idle > 999)
			strlcpy(buf, "999+", sizeof(buf));
		else
			snprintf(buf, size, "%d", idle);
	}
}

static tui_list_display_t online_users_display(tui_list_t *p, int i)
{
	online_users_t *up = p->data;
	online_user_info_t *ip = up->users + i;

	char nick[24];
	get_nick(nick, sizeof(nick), up, ip);

	const char *host = get_host(ip);

	int status = get_user_status(ip->sid);
	const char *color = get_status_color(status,
			!(ip->flag & SESSION_FLAG_INVISIBLE),
			ip->flag & SESSION_FLAG_WEB);

	int idle = get_idle_time(ip->sid);
	char idle_str[8];
	get_idle_str(idle_str, sizeof(idle_str), idle, status);

	char buf[128];
	snprintf(buf, sizeof(buf), " \033[m%4d%s  %-12.12s\033[m %-20.20s"
			"\033[m %-19.19s %s%-10.10s\033[37m %4s\033[m\n",
			i + 1, up->follow ? "\033[32m" : "", ip->name,
			nick, host, color, status_descr(status), idle_str);
	prints("%s", buf);
	return 0;
}

static int alter_nick(online_users_t *up)
{
	char buf[STRLEN];
	if (streq(currentuser.userid, "guest"))
		return DONOTHING;
	buf[0] = '\0';
	getdata(t_lines - 1, 0, "变换昵称(不是临时变换)为: ", buf, NAMELEN,
			DOECHO, NA);
	if (buf[0] != '\0') {
		set_safe_record();
		strlcpy(currentuser.username, buf, sizeof(currentuser.username));
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
		up->uptime = 0;
		return PARTUPDATE;
	}
	return MINIUPDATE;
}

static int kick_out(online_users_t *up, online_user_info_t *ip)
{
	char buf[STRLEN];
	if (!HAS_PERM(PERM_USER) && !streq(ip->name, currentuser.userid))
		return DONOTHING;
	if (streq(currentuser.userid, "guest"))
		return DONOTHING;
	if (ip->sid == session.id)
		strlcpy(buf, "您要把【自己】踢出去吗", sizeof(buf));
	else
		snprintf(buf, sizeof(buf), "你要把 %s 踢出站外吗", ip->name);
	if (!askyn(buf, false, true))
		return MINIUPDATE;

	if (bbs_kill(ip->sid, ip->pid, SIGHUP) == 0) {
		snprintf(buf, sizeof(buf), "%s 已被踢出站外", ip->name);
		up->uptime = 0;
		return PARTUPDATE;
	} else {
		snprintf(buf, sizeof(buf), "%s 无法踢出站外", ip->name);
		return MINIUPDATE;
	}
}

static tui_list_query_t online_users_query(tui_list_t *p)
{
	online_users_t *up = p->data;
	online_user_info_t *ip = up->users + p->cur;
	p->in_query = true;

	if (!ip)
		return DONOTHING;

	t_query(ip->name);
	move(t_lines - 1, 0);
	prints("\033[0;1;37;44m聊天[\033[1;32mt\033[37m] 寄信[\033[1;32mm\033[37m] "
			"送讯息[\033[1;32ms\033[37m] 加,减朋友[\033[1;32mo\033[37m,\033[1;32md\033[37m] "
			"选择使用者[\033[1;32m↑\033[37m,\033[1;32m↓\033[37m] "
			"求救[\033[1;32mh\033[37m]");
	refresh();
	return DONOTHING;
}

static tui_list_handler_t online_users_handler(tui_list_t *p, int ch)
{
	online_users_t *up = p->data;
	online_user_info_t *ip = up->users + p->cur;
	p->valid = false;

	char buf[STRLEN];
	switch (ch) {
		case 'h': case 'H':
			show_help("help/userlisthelp");
			return FULLUPDATE;
		case 'm': case 'M':
			if (!HAS_PERM(PERM_MAIL))
				return DONOTHING;
			m_send(ip->name);
			return FULLUPDATE;
#if 0
		case 's': case 'S':
			if (!strcmp(currentuser.userid, "guest") || !HAS_PERM(PERM_TALK))
				return DONOTHING;
			if (!canmsg(uin)) {
				snprintf(buf, sizeof(buf), "%s 已关闭讯息呼叫器", uin->userid);
				presskeyfor(buf, t_lines - 1);
				return MINIUPDATE;
			}
			do_sendmsg(uin, NULL, 0, uin->pid);
			return FULLUPDATE;
#endif
		case 'o': case 'O':
			if (streq(currentuser.userid, "guest"))
				return DONOTHING;
			snprintf(buf, sizeof(buf), "确定关注 %s 吗?", ip->name);
			if (!askyn(buf, false, true))
				return MINIUPDATE;
			if (follow(session.uid, ip->name, NULL)) {
				snprintf(buf, sizeof(buf), "成功关注 %s", ip->name);
				presskeyfor(buf, t_lines - 1);
			}
			return MINIUPDATE;
		case 'd': case 'D':
			if (streq(currentuser.userid, "guest"))
				return DONOTHING;
			snprintf(buf, sizeof(buf), "确定不再关注 %s 吗?", ip->name);
			if (!askyn(buf, false, true))
				return MINIUPDATE;
			{
				user_id_t uid = get_user_id(ip->name);
				if (uid > 0 && unfollow(session.uid, uid)) {
					snprintf(buf, sizeof(buf), "已取消关注 %s", ip->name);
					presskeyfor(buf, t_lines - 1);
					return PARTUPDATE;
				}
			}		
			return MINIUPDATE;
	}
	if (p->in_query)
		return DONOTHING;

	switch (ch) {
		case 'Y':
			if (HAS_PERM(PERM_CLOAK)) {
				x_cloak();
				up->uptime = 0;
				return PARTUPDATE;
			}
			return DONOTHING;
		case 'C': case 'c':
			return alter_nick(up);
		case 'k': case 'K':
			return kick_out(up, ip);
		case 'f': case 'F':
			up->follow = !up->follow;
			if (up->follow)
				set_user_status(ST_FRIEND);
			else
				set_user_status(ST_LUSERS);
			up->uptime = 0;
			return FULLUPDATE;
		case 'W': case 'w':
			if (streq(currentuser.userid, "guest"))
				return DONOTHING;
			up->show_note = !up->show_note;
			return PARTUPDATE;
#if 0
		case KEY_TAB:	
			if (HAS_PERM(PERM_OCHAT)) {
				if (++(up->sort) > USRSORT_STATUS)
					up->sort = USRSORT_USERID;
				up->uptime = 0;
				return FULLUPDATE;
			}
			return DONOTHING;
#endif
		case '\r': case '\n': case KEY_RIGHT:
			online_users_query(p);
			return DONOTHING;
		default:
			return DONOTHING;
	}
}

static int online_users(online_users_t *p)
{
	tui_list_t t = {
		.data = p,
		.loader = online_users_load,
		.title = online_users_title,
		.display = online_users_display,
		.handler = online_users_handler,
		.query = online_users_query,
	};

	tui_list(&t);

	online_users_free(p);
	return 0;
}

int show_online_users(void)
{
	online_users_t ou = { .follow = false };
	set_user_status(ST_LUSERS);
	return online_users(&ou);
}

int show_online_followings(void)
{
	online_users_t ou = { .follow = true };
	set_user_status(ST_FRIEND);
	return online_users(&ou);
}

int show_users_in_board(void)
{
	online_users_t ou = { .follow = false, .bid = currbp->id };
	set_user_status(ST_LUSERS);
	return online_users(&ou);
}

int show_followings_in_board(void)
{
	online_users_t ou = { .follow = true, .bid = currbp->id };
	set_user_status(ST_FRIEND);
	return online_users(&ou);
}
