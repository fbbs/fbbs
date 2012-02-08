#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
#include "fbbs/friend.h"
#include "fbbs/status.h"
#include "fbbs/string.h"
#include "fbbs/tui_list.h"

static tui_list_loader_t following_list_loader(tui_list_t *p)
{
	if (p->data)
		following_list_free(p->data);
	p->data = following_list_load(session.uid);
	p->eod = true;
	return (p->all = following_list_rows(p->data));
}

static tui_list_title_t following_list_title(tui_list_t *p)
{
	const char *middle = chkmail() ? "[您有信件]" : BBSNAME;
	showtitle("[编辑关注名单]", middle);
	prints(" [\033[1;32m←\033[m,\033[1;32me\033[m] 离开"
			" [\033[1;32mh\033[m] 求助"
			" [\033[1;32m→\033[m,\033[1;32mRtn\033[m] 说明档"
			" [\033[1;32m↑\033[m,\033[1;32m↓\033[m] 选择"
			" [\033[1;32ma\033[m] 加关注"
			" [\033[1;32md\033[m] 取消关注\n"
			"\033[1;44m 编号  关注的人     友 备注\033[K\033[m\n");
}

static tui_list_display_t following_list_display(tui_list_t *p, int i)
{
	following_list_t *list = p->data;

	char gbk_note[FOLLOW_NOTE_CCHARS * 2 + 1];
	convert_u2g(following_list_get_notes(list, i), gbk_note);

	prints(" %4d  %-12s %s %s\n", i + 1, following_list_get_name(list, i),
			following_list_get_is_friend(list, i) ? "√" : "  ", gbk_note);
	return 0;
}

static int tui_follow(void)
{
	char buf[IDLEN + 1];
	getdata(t_lines - 1, 0, "请输入要关注的人: ", buf, IDLEN, DOECHO, YEA);
	if (!*buf)
		return 0;
	char note[FOLLOW_NOTE_CCHARS * 2 + 1], utf8_note[FOLLOW_NOTE_CCHARS * 4 + 1];
	getdata(t_lines - 1, 0, "请输入备注: ", note, sizeof(note), DOECHO, YEA);
	convert_g2u(note, utf8_note);
	return follow(session.uid, buf, utf8_note);
}

static int tui_unfollow(user_id_t uid)
{
	move(t_lines - 1, 0);
	return askyn("确定取消关注?", false, true) ? unfollow(session.uid, uid) : 0;
}

static int tui_edit_followed_note(user_id_t followed, const char *orig)
{
	char note[FOLLOW_NOTE_CCHARS * 2 + 1], utf8_note[FOLLOW_NOTE_CCHARS * 4 + 1];
	getdata(t_lines - 1, 0, "请输入备注: ", note, sizeof(note), DOECHO, YEA);
	convert_g2u(note, utf8_note);

	if (!*utf8_note || streq(orig, utf8_note))
		return DONOTHING;

	edit_followed_note(session.uid, followed, utf8_note);
	return FULLUPDATE;
}

static tui_list_query_t following_list_query(tui_list_t *p)
{
	p->in_query = true;
	if (t_query(following_list_get_name(p->data, p->cur)) == -1)
		return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints("\033[0;1;44;31m\033[33m 寄信 m │ 结束 Q,← │上一位 ↑│"
			"下一位 <Space>,↓                            \033[m");
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
			break;
		default:
			return DONOTHING;
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
