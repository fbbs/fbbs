#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/friend.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

static tui_list_loader_t following_list_loader(tui_list_t *p)
{
	if (p->data)
		following_list_free(p->data);
	p->data = following_list_load(session_uid());
	return (p->all = following_list_rows(p->data));
}

static tui_list_title_t following_list_title(tui_list_t *p)
{
	//% const char *middle = chkmail() ? "[您有信件]" : BBSNAME;
	const char *middle = chkmail() ? "[\xc4\xfa\xd3\xd0\xd0\xc5\xbc\xfe]" : BBSNAME;
	//% showtitle("[编辑关注名单]", middle);
	showtitle("[\xb1\xe0\xbc\xad\xb9\xd8\xd7\xa2\xc3\xfb\xb5\xa5]", middle);
	//% prints(" [\033[1;32m←\033[m,\033[1;32me\033[m] 离开"
	prints(" [\033[1;32m\xa1\xfb\033[m,\033[1;32me\033[m] \xc0\xeb\xbf\xaa"
			//% " [\033[1;32mh\033[m] 求助"
			" [\033[1;32mh\033[m] \xc7\xf3\xd6\xfa"
			//% " [\033[1;32m→\033[m,\033[1;32mRtn\033[m] 说明档"
			" [\033[1;32m\xa1\xfa\033[m,\033[1;32mRtn\033[m] \xcb\xb5\xc3\xf7\xb5\xb5"
			//% " [\033[1;32m↑\033[m,\033[1;32m↓\033[m] 选择"
			" [\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd\033[m] \xd1\xa1\xd4\xf1"
			//% " [\033[1;32ma\033[m] 加关注"
			" [\033[1;32ma\033[m] \xbc\xd3\xb9\xd8\xd7\xa2"
			//% " [\033[1;32md\033[m] 取消关注\n"
			" [\033[1;32md\033[m] \xc8\xa1\xcf\xfb\xb9\xd8\xd7\xa2\n"
			//% "\033[1;44m 编号  关注的人     备注\033[K\033[m\n");
			"\033[1;44m \xb1\xe0\xba\xc5  \xb9\xd8\xd7\xa2\xb5\xc4\xc8\xcb     \xb1\xb8\xd7\xa2\033[K\033[m\n");
}

static tui_list_display_t following_list_display(tui_list_t *p, int i)
{
	following_list_t *list = p->data;

	char gbk_note[FOLLOW_NOTE_CCHARS * 2 + 1];
	convert_u2g(following_list_get_notes(list, i), gbk_note);

	prints(" %4d  %-12s %s\n", i + 1, following_list_get_name(list, i),
			gbk_note);
	return 0;
}

static int tui_follow(void)
{
	char buf[IDLEN + 1];
	//% "请输入要关注的人: "
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xb9\xd8\xd7\xa2"
			"\xb5\xc4\xc8\xcb: ", buf, IDLEN, DOECHO, YEA);
	if (!*buf)
		return 0;
	GBK_UTF8_BUFFER(note, FOLLOW_NOTE_CCHARS);
	//% "请输入备注: "
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xb1\xb8\xd7\xa2: ",
			gbk_note, sizeof(gbk_note), DOECHO, YEA);
	convert_g2u(gbk_note, utf8_note);
	return follow(session_uid(), buf, utf8_note);
}

static int tui_unfollow(user_id_t uid)
{
	move(-1, 0);
	//% return askyn("确定取消关注?", false, true) ? unfollow(session_uid(), uid) : 0;
	return askyn("\xc8\xb7\xb6\xa8\xc8\xa1\xcf\xfb\xb9\xd8\xd7\xa2?", false, true) ? unfollow(session_uid(), uid) : 0;
}

static int tui_edit_followed_note(user_id_t followed, const char *orig)
{
	char note[FOLLOW_NOTE_CCHARS * 2 + 1], utf8_note[FOLLOW_NOTE_CCHARS * 4 + 1];
	//% getdata(-1, 0, "请输入备注: ", note, sizeof(note), DOECHO, YEA);
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xb1\xb8\xd7\xa2: ", note, sizeof(note), DOECHO, YEA);
	convert_g2u(note, utf8_note);

	if (!*utf8_note || streq(orig, utf8_note))
		return DONOTHING;

	edit_followed_note(session_uid(), followed, utf8_note);
	return FULLUPDATE;
}

static tui_list_query_t following_list_query(tui_list_t *p)
{
	p->in_query = true;
	if (t_query(following_list_get_name(p->data, p->cur)) == -1)
		return FULLUPDATE;
	screen_move_clear(-1);
	//% prints("\033[0;1;44;31m\033[33m 寄信 m │ 结束 Q,← │上一位 ↑│"
	prints("\033[0;1;44;31m\033[33m \xbc\xc4\xd0\xc5 m \xa9\xa6 \xbd\xe1\xca\xf8 Q,\xa1\xfb \xa9\xa6\xc9\xcf\xd2\xbb\xce\xbb \xa1\xfc\xa9\xa6"
			//% "下一位 <Space>,↓                            \033[m");
			"\xcf\xc2\xd2\xbb\xce\xbb <Space>,\xa1\xfd                            \033[m");
	refresh();
	return DONOTHING;
}

static tui_list_handler_t following_list_handler(tui_list_t *p, int key)
{
	switch (key) {
		case 'a':
			if (tui_follow() > 0)
				p->valid = false;
			else
				return MINIUPDATE;
			break;
		case 'd':
			if (tui_unfollow(following_list_get_id(p->data, p->cur)) > 0)
				p->valid = false;
			else
				return MINIUPDATE;
			break;
		case 'm':
			m_send(following_list_get_name(p->data, p->cur));
			break;
		case 'E':
			if (tui_edit_followed_note(following_list_get_id(p->data, p->cur),
						following_list_get_notes(p->data, p->cur)) == DONOTHING ) {
				return MINIUPDATE;
			} else {
				p->valid = false;
			}
			break;
		case KEY_RIGHT:
		case '\r':
		case '\n':
			following_list_query(p);
			return DONOTHING;
		case 'h':
			show_help("help/friendshelp");
			return FULLUPDATE;
		default:
			return READ_AGAIN;
	}
	return DONOTHING;
}

int tui_following_list(void)
{
	tui_list_t t = {
		.data = NULL,
		.loader = following_list_loader,
		.title = following_list_title,
		.display = following_list_display,
		.handler = following_list_handler,
		.query = following_list_query
	};

	set_user_status(ST_GMENU);
	tui_list(&t);

	following_list_free(t.data);
	return 0;
}

static tui_list_loader_t black_list_loader(tui_list_t *p)
{
	if (p->data)
		black_list_free(p->data);
	p->data = black_list_load(session_uid());
	return (p->all = black_list_rows(p->data));
}

static tui_list_title_t black_list_title(tui_list_t *p)
{
	//% const char *middle = chkmail() ? "[您有信件]" : BBSNAME;
	const char *middle = chkmail() ? "[\xc4\xfa\xd3\xd0\xd0\xc5\xbc\xfe]" : BBSNAME;
	//% showtitle("[编辑黑名单]", middle);
	showtitle("[\xb1\xe0\xbc\xad\xba\xda\xc3\xfb\xb5\xa5]", middle);
	//% prints(" [\033[1;32m←\033[m,\033[1;32me\033[m] 离开"
	prints(" [\033[1;32m\xa1\xfb\033[m,\033[1;32me\033[m] \xc0\xeb\xbf\xaa"
			//% " [\033[1;32mh\033[m] 求助"
			" [\033[1;32mh\033[m] \xc7\xf3\xd6\xfa"
			//% " [\033[1;32m→\033[m,\033[1;32mRtn\033[m] 说明档"
			" [\033[1;32m\xa1\xfa\033[m,\033[1;32mRtn\033[m] \xcb\xb5\xc3\xf7\xb5\xb5"
			//% " [\033[1;32m↑\033[m,\033[1;32m↓\033[m] 选择"
			" [\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd\033[m] \xd1\xa1\xd4\xf1"
			//% " [\033[1;32ma\033[m] 添加"
			" [\033[1;32ma\033[m] \xcc\xed\xbc\xd3"
			//% " [\033[1;32md\033[m] 解除\n"
			" [\033[1;32md\033[m] \xbd\xe2\xb3\xfd\n"
			//% "\033[1;44m 编号  用户     备注\033[K\033[m\n");
			"\033[1;44m \xb1\xe0\xba\xc5  \xd3\xc3\xbb\xa7     \xb1\xb8\xd7\xa2\033[K\033[m\n");
}

static tui_list_display_t black_list_display(tui_list_t *p, int i)
{
	black_list_t *l = p->data;
	
	GBK_BUFFER(note, BLACK_LIST_NOTE_CCHARS);
	*gbk_note = '\0';
	convert_u2g(black_list_get_notes(l, i), gbk_note);

	prints(" %4d  %-12s %s\n", i + 1, black_list_get_name(l, i), gbk_note);
	return 0;
}

static int tui_black_list_add(void)
{
	char buf[IDLEN + 1];
	//% getdata(-1, 0, "请输入要拉黑的人: ", buf, IDLEN, DOECHO, YEA);
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xc0\xad\xba\xda\xb5\xc4\xc8\xcb: ", buf, IDLEN, DOECHO, YEA);
	if (!*buf)
		return 0;
	GBK_UTF8_BUFFER(note, BLACK_LIST_NOTE_CCHARS);
	//% getdata(-1, 0, "请输入备注: ",
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xb1\xb8\xd7\xa2: ",
			gbk_note, sizeof(gbk_note), DOECHO, YEA);
	convert_g2u(gbk_note, utf8_note);
	return black_list_add(session_uid(), buf, utf8_note);
}

static int tui_black_list_edit(user_id_t blocked, const char *orig)
{
	GBK_UTF8_BUFFER(note, BLACK_LIST_NOTE_CCHARS);
	//% getdata(-1, 0, "请输入备注: ",
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xb1\xb8\xd7\xa2: ",
			gbk_note, sizeof(gbk_note), DOECHO, YEA);
	convert_g2u(gbk_note, utf8_note);

	if (*utf8_note && !streq(orig, utf8_note)
			&& black_list_edit(session_uid(), blocked, utf8_note) > 0)
		return FULLUPDATE;

	return MINIUPDATE;
}

static tui_list_query_t black_list_query(tui_list_t *p)
{
	p->in_query = true;
	if (t_query(black_list_get_name(p->data, p->cur)) == -1)
		return FULLUPDATE;
	screen_move_clear(-1);
	//% prints("\033[0;1;33;44m 结束 Q,← │上一位 ↑│下一位 <Space>,↓ "
	prints("\033[0;1;33;44m \xbd\xe1\xca\xf8 Q,\xa1\xfb \xa9\xa6\xc9\xcf\xd2\xbb\xce\xbb \xa1\xfc\xa9\xa6\xcf\xc2\xd2\xbb\xce\xbb <Space>,\xa1\xfd "
			"                           \033[m");
	refresh();
	return DONOTHING;
}

static tui_list_handler_t black_list_handler(tui_list_t *p, int key)
{
	switch (key) {
		case 'a':
			if (tui_black_list_add() > 0)
				p->valid = false;
			else
				return MINIUPDATE;
		case 'h':
			show_help("help/rejectshelp");
			return FULLUPDATE;
	}

	if (p->cur >= p->all)
		return READ_AGAIN;

	switch (key) {
		case 'd':
			if (black_list_rm(session_uid(),
						black_list_get_id(p->data, p->cur)) > 0)
				p->valid = false;
			break;
		case 'E':
			return tui_black_list_edit(black_list_get_id(p->data, p->cur),
					black_list_get_notes(p->data, p->cur));
			break;
		case KEY_RIGHT:
		case '\r':
		case '\n':
			black_list_query(p);
			return DONOTHING;
		default:
			return READ_AGAIN;
	}
	return DONOTHING;
}

int tui_black_list(void)
{
	tui_list_t t = {
		.data = NULL,
		.loader = black_list_loader,
		.title = black_list_title,
		.display = black_list_display,
		.handler = black_list_handler,
		.query = black_list_query,
	};

	set_user_status(ST_GMENU);
	tui_list(&t);

	black_list_free(t.data);
	return 0;
}
