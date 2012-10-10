#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/list.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct post_list_position_t {
	post_list_type_e type;
	user_id_t uid;
	post_id_t min_pid;
	post_id_t min_tid;
	post_id_t cur_pid;
	post_id_t cur_tid;
	UTF8_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
	SLIST_FIELD(post_list_position_t) next;
} post_list_position_t;

SLIST_HEAD(post_list_position_list_t, post_list_position_t);

static bool match(const post_list_position_t *p, const post_filter_t *fp)
{
	return !(p->type != fp->type
			|| (p->type == POST_LIST_AUTHOR && p->uid != fp->uid)
			|| (p->type == POST_LIST_KEYWORD
				&& streq(p->utf8_keyword, fp->utf8_keyword)));
}

static void filter_to_position_record(const post_filter_t *fp,
		post_list_position_t *p)
{
	p->type = fp->type;
	p->uid = fp->uid;
	p->min_pid = p->min_tid = p->cur_pid = p->cur_tid = 0;
	memcpy(p->utf8_keyword, fp->utf8_keyword, sizeof(p->utf8_keyword));
}

static post_list_position_t *get_post_list_position(const post_filter_t *fp)
{
	static struct post_list_position_list_t *list = NULL;
	if (!list) {
		list = malloc(sizeof(*list));
		SLIST_INIT_HEAD(list);
	}

	SLIST_FOREACH(post_list_position_t, p, list, next) {
		if (match(p, fp))
			return p;
	}

	post_list_position_t *p = malloc(sizeof(*p));
	filter_to_position_record(fp, p);
	SLIST_INSERT_HEAD(list, p, next);
	return p;
}

typedef struct {
	post_filter_t filter;
	post_list_position_t *pos;

	bool relocate;
	bool reload;
	bool sreload;
	int last_query_rows;

	post_info_t **index;
	post_info_t *posts;
	post_info_t *sposts;
	int icount;
	int count;
	int scount;
} post_list_t;

static bool is_asc(slide_list_base_e base)
{
	return (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_NEXT);
}

static void res_to_array(db_res_t *r, post_list_t *l, slide_list_base_e base,
		int size)
{
	int rows = db_res_rows(r);
	if (rows < 1)
		return;

	bool asc = is_asc(base);

	if (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_BOTTOMUP
			|| rows >= size) {
		rows = rows > size ? size : rows;
		for (int i = 0; i < rows; ++i)
			res_to_post_info(r, asc ? i : rows - i - 1, l->posts + i);
		l->count = rows;
	} else {
		int extra = l->count + rows - size;
		int left = l->count - (extra > 0 ? extra : 0);
		if (asc) {
			if (extra > 0) {
				memmove(l->posts, l->posts + extra,
						sizeof(*l->posts) * (l->count - extra));
			}
			for (int i = 0; i < rows; ++i)
				res_to_post_info(r, i, l->posts + left + i);
		} else {
			memmove(l->posts + rows, l->posts,
					sizeof(*l->posts) * (l->count - left));
			for (int i = 0; i < rows; ++i)
				res_to_post_info(r, rows - i - 1, l->posts + i);
		}
		l->count = left + rows;
	}
}

static void load_sticky_posts(post_list_t *l)
{
	if (l->filter.type == POST_LIST_NORMAL && !l->sposts && l->sreload) {
		l->scount = _load_sticky_posts(l->filter.bid, &l->sposts);
		l->sreload = false;
	}
}

static void index_posts(post_list_t *l, int limit, slide_list_base_e base)
{
	if (base != SLIDE_LIST_CURRENT) {
		int remain = limit, start = 0;
		if (base == SLIDE_LIST_BOTTOMUP)
			start = l->scount;
		if (base == SLIDE_LIST_NEXT)
			start = l->count - l->last_query_rows;
		if (start < 0)
			start = 0;

		l->icount = 0;
		for (int i = start; i < l->count && i < limit; ++i) {
			l->index[l->icount++] = l->posts + i;
			--remain;
		}

		if (l->sposts) {
			for (int i = 0; i < remain && i < l->scount; ++i) {
				l->index[l->icount++] = l->sposts + i;
			}
		}
	}
}

static void adjust_filter(post_list_t *l, slide_list_base_e base)
{
	if (is_asc(base)) {
		if (base == SLIDE_LIST_TOPDOWN)
			l->filter.min = 0;
		else if (l->count)
			l->filter.min = l->posts[l->count - 1].id + 1;
		l->filter.max = 0;
	} else {
		l->filter.min = 0;
		if (base == SLIDE_LIST_BOTTOMUP)
			l->filter.max = 0;
		else if (l->count)
			l->filter.max = l->posts[0].id - 1;
	}
}

static void _load_posts(slide_list_t *p, slide_list_base_e base,
		post_filter_t *fp, int page, int limit)
{
	query_builder_t *b = build_post_query(fp, is_asc(base), limit);
	db_res_t *res = b->query(b);
	query_builder_free(b);

	post_list_t *l = p->data;
	res_to_array(res, l, base, page);
	l->last_query_rows = db_res_rows(res);
	db_clear(res);
}

static void load_posts(slide_list_t *p, int page)
{
	post_list_t *l = p->data;
	adjust_filter(l, p->base);

	_load_posts(p, p->base, &l->filter, page, page);
}

static void reverse_load_posts(slide_list_t *p, int page, int limit)
{
	post_list_t *l = p->data;
	slide_list_base_e base = (p->base == SLIDE_LIST_NEXT ?
			SLIDE_LIST_PREV : SLIDE_LIST_NEXT);

	post_filter_t filter = l->filter;
	if (base == SLIDE_LIST_NEXT) {
		filter.min = l->filter.max + 1;
		filter.max = 0;
	} else {
		filter.min = 0;
		filter.max = l->filter.min - 1;
	}

	_load_posts(p, base, &filter, page, limit);
}

static slide_list_loader_t post_list_loader(slide_list_t *p)
{
	post_list_t *l = p->data;

	if (l->reload) {
		p->base = SLIDE_LIST_NEXT;
		if (l->pos->min_pid)
			l->filter.min = l->pos->min_pid;
		else
			l->filter.min = brc_last_read() + 1;
	}
	if (p->base == SLIDE_LIST_CURRENT)
		return 0;

	int page = t_lines - 4;
	if (!l->index) {
		l->index = malloc(sizeof(*l->index) * page);
		l->posts = malloc(sizeof(*l->posts) * page);
		l->sposts = NULL;
		l->icount = l->count = l->scount = 0;
	}

	load_posts(p, page);

	if ((p->base == SLIDE_LIST_NEXT && l->last_query_rows < page)
			|| p->base == SLIDE_LIST_BOTTOMUP) {
		load_sticky_posts(l);
	}

	if (l->reload && l->last_query_rows + l->scount < page
			&& (p->base == SLIDE_LIST_PREV || p->base == SLIDE_LIST_NEXT)) {
		reverse_load_posts(p, page, page - l->last_query_rows - l->scount);
	}

	if (l->last_query_rows) {
		if (p->update != FULLUPDATE)
			p->update = PARTUPDATE;
	} else {
		p->cur = p->base == SLIDE_LIST_PREV ? 0 : page - 1;
	}

	index_posts(l, page, p->base);
	l->reload = false;
	return 0;
}

static basic_session_info_t *get_sessions_by_name(const char *uname)
{
	return db_query("SELECT "BASIC_SESSION_INFO_FIELDS
			" FROM sessions s JOIN alive_users u ON s.user_id = u.id"
			" WHERE s.active AND lower(u.name) = lower(%s)", uname);
}

static int bm_color(const char *uname)
{
	basic_session_info_t *s = get_sessions_by_name(uname);
	int visible = 0, color = 33;

	if (s) {
		for (int i = 0; i < basic_session_info_count(s); ++i) {
			if (basic_session_info_visible(s, i))
				++visible;
		}

		if (visible)
			color = 32;
		else if (HAS_PERM(PERM_SEECLOAK) && basic_session_info_count(s) > 0)
			color = 36;
	}
	basic_session_info_clear(s);
	return color;
}

enum {
	BM_NAME_LIST_LEN = 56,
};

static int show_board_managers(const board_t *bp)
{
	prints("\033[33m");

	if (!bp->bms[0]) {
		prints("诚征版主中");
		return 10;
	}

	char bms[sizeof(bp->bms)];
	strlcpy(bms, bp->bms, sizeof(bms));

	prints("版主:");
	int width = 5;
	for (const char *s = strtok(bms, " "); s; s = strtok(NULL, " ")) {
		++width;
		prints(" ");

		int len = strlen(s);
		if (width + len > BM_NAME_LIST_LEN) {
			prints("...");
			return width + 3;
		}

		width += len;
		int color = bm_color(s);
		prints("\033[%dm%s", color, s);
	}

	prints("\033[36m");
	return width;
}

static void repeat(int c, int repeat)
{
	for (int i = 0; i < repeat; ++i)
		outc(c);
}

static void align_center(const char *s, int remain)
{
	int len = strlen(s);
	if (len > remain) {
		prints("%s", s);
	} else {
		int spaces = (remain - len) / 2;
		repeat(' ', spaces);
		prints("%s", s);
		spaces = remain - len - spaces;
		repeat(' ', spaces);
	}
}

static void show_prompt(const board_t *bp, const char *prompt, int remain)
{
	int blen = strlen(bp->name) + 2;
	int plen = prompt ? strlen(prompt) : strlen(bp->descr);

	if (blen + plen > remain) {
		if (prompt) {
			align_center(prompt, remain);
		} else {
			repeat(' ', remain - blen);
			prints("[%s]", bp->name);
		}
		return;
	} else {
		align_center(prompt ? prompt : bp->descr, remain - blen);
		prints("\033[33m[%s]", bp->name);
	}
}

static const char *mode_description(post_list_type_e type)
{
	const char *s;
	switch (type) {
		case POST_LIST_NORMAL:
			if (DEFINE(DEF_THESIS))
				s = "主题";
			else
				s =  "一般";
			break;
		case POST_LIST_THREAD:
			s = "主题";
//			s = "原作";
			break;
		case POST_LIST_MARKED:
			s = "MARK";
			break;
		case POST_LIST_DIGEST:
			s = "文摘";
			break;
		case POST_LIST_AUTHOR:
			s = "精确";
			break;
		case POST_LIST_KEYWORD:
			s = "标题关键字";
			break;
		case POST_LIST_TRASH:
			s = "垃圾箱";
			break;
		case POST_LIST_JUNK:
			s = "站务垃圾箱";
			break;
//		case ATTACH_MODE:
//			readmode = "附件区";
//			break;
		default:
			s = "未定义";
	}
	return s;
}

static slide_list_title_t post_list_title(slide_list_t *p)
{
	board_t board;
	get_board(currboard, &board);
	board_to_gbk(&board);

	prints("\033[1;44m");
	int width = show_board_managers(&board);

	const char *prompt = NULL;
	if (chkmail())
		prompt = "[您有信件，按 M 看新信]";
	else if ((board.flag & BOARD_VOTE_FLAG))
		prompt = "※投票中,按 v 进入投票※";
	show_prompt(&board, prompt, 78 - width);

	move(1, 0);
	prints("\033[m 离开[\033[1;32m←\033[m,\033[1;32me\033[m] "
		"选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] "
		"阅读[\033[1;32m→\033[m,\033[1;32mRtn\033[m] "
		"发文章[\033[1;32mCtrl-P\033[m] 砍信[\033[1;32md\033[m] "
		"备忘录[\033[1;32mTAB\033[m] 求助[\033[1;32mh\033[m]\n");

	post_list_t *l = p->data;
	const char *mode = mode_description(l->filter.type);

	prints("\033[1;37;44m  编号    %-12s %6s %-25s 在线:%-4d"
				"    [%4s模式] \033[m\n", "刊 登 者", "日  期", " 标  题",
				count_onboard(board.id), mode);
	clrtobot();
}

char *getshortdate(time_t time) {
	static char str[10];
	struct tm *tm;

	tm = localtime(&time);
	sprintf(str, "%02d.%02d.%02d", tm->tm_year - 100, tm->tm_mon + 1,
			tm->tm_mday);
	return str;
}

#ifdef ENABLE_FDQUAN
#define ENABLE_COLOR_ONLINE_STATUS
#endif

#ifdef ENABLE_COLOR_ONLINE_STATUS
const char *get_board_online_color(const char *uname, int bid)
{
	if (streq(uname, currentuser.userid))
		return session.visible ? "1;37" : "1;36";

	const char *color = "";
	bool online = false;
	int invisible = 0;

	basic_session_info_t *s = get_sessions_by_name(uname);
	if (s) {
		for (int i = 0; i < basic_session_info_count(s); ++i) {
			if (!basic_session_info_visible(s, i)) {
				if (HAS_PERM(PERM_SEECLOAK))
					++invisible;
				else
					continue;
			}

			online = true;
			if (get_current_board(basic_session_info_sid(s, i)) == bid) {
				if (basic_session_info_visible(s, i))
					color = "1;37";
				else
					color = "1;36";
				basic_session_info_clear(s);
				return color;
			}
		}
	}

	if (!online)
		color = "1;30";
	else if (invisible > 0 && invisible == basic_session_info_count(s))
		color = "36";

	basic_session_info_clear(s);
	return color;
}
#else
#define get_board_online_color(n, i)  ""
#endif

static const char *get_post_date(fb_time_t stamp)
{
#ifdef FDQUAN
	if (time(NULL) - stamp < 24 * 60 * 60)
		return fb_ctime(&stamp) + 10;
#endif
	return fb_ctime(&stamp) + 4;
}

static void post_list_display_entry(post_list_type_e type, post_info_t *p)
{
	char id_str[24];
	if (p->flag & POST_FLAG_STICKY)
		strlcpy(id_str, " \033[1;31m[∞]\033[m ", sizeof(id_str));
	else
		pid_to_base32(p->id, id_str, 7);

	int mark = get_post_mark(p);
	const char *mark_prefix = "", *mark_suffix = "";
	if ((p->flag & POST_FLAG_IMPORT) && am_curr_bm()) {
		mark_prefix = (type == ' ') ? "\033[42m" : "\033[32m";
		mark_suffix = "\033[m";
	}
#if 0
	if (digestmode == ATTACH_MODE) {
		filetime = ent->timeDeleted;
	} else {
		filetime = atoi(ent->filename + 2);
	}
#endif

	const char *date = get_post_date(p->stamp);

	const char *idcolor = get_board_online_color(p->owner, currbp->id);

	char color[10] = "";
#ifdef COLOR_POST_DATE
	struct tm *mytm = fb_localtime(&p->stamp);
	snprintf(color, sizeof(color), "\033[1;%dm", 30 + mytm->tm_wday + 1);
#endif

//		sprintf(title, "%s %s", (digestmode == 2) ? ((ent->accessed[1]
//				& FILE_LASTONE) ? "└" : "├") : "Re:", ent->title + 4);
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (strneq(p->utf8_title, "Re: ", 4)) {
		convert_u2g(p->utf8_title, gbk_title);
	} else {
		GBK_BUFFER(title2, POST_TITLE_CCHARS);
		convert_u2g(p->utf8_title, gbk_title2);
		snprintf(gbk_title, sizeof(gbk_title), "◆ %s", gbk_title2);
	}

	if (is_deleted(type)) {
		;
//		ellipsis(title, 38 - strlen(ent->szEraser));
	} else {
		ellipsis(gbk_title, 49);
	}

//	if (digestmode == ATTACH_MODE) {
//		sprintf(buf, " %5d %c %-12.12s %s%6.6s\033[m %s", num, type,
//				ent->owner, color, date, title);
	char buf[128];
	snprintf(buf, sizeof(buf),
			" %s %s%c%s \033[%sm%-12.12s %s%6.6s %s\033[m%s\n",
			id_str, mark_prefix, mark, mark_suffix,
			idcolor, p->owner, color, date,
			(p->flag & POST_FLAG_LOCKED) ? "\033[1;33mx" : " ",
			gbk_title);
	outs(buf);
}

static slide_list_display_t post_list_display(slide_list_t *p)
{
	post_list_t *l = p->data;
	for (int i = 0; i < l->icount; ++i) {
		post_list_display_entry(l->filter.type, l->index[i]);
	}
	return 0;
}

static int toggle_post_lock(post_info_t *ip)
{
	bool locked = ip->flag & POST_FLAG_LOCKED;
	if (am_curr_bm() || (session.id == ip->uid && !locked)) {
		post_filter_t filter = {
			.bid = ip->bid, .min = ip->id, .max = ip->id
		};
		if (set_post_flag(&filter, "locked", !locked, false)) {
			set_post_flag_local(ip, POST_FLAG_LOCKED, !locked);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int toggle_post_stickiness(post_info_t *ip, post_list_t *l)
{
	if (is_deleted(l->filter.type))
		return DONOTHING;

	bool sticky = ip->flag & POST_FLAG_STICKY;
	if (am_curr_bm() && sticky_post_unchecked(ip->bid, ip->id, !sticky)) {
		l->sreload = l->reload = true;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int toggle_post_flag(post_info_t *ip, post_flag_e flag,
		const char *field)
{
	bool set = ip->flag & flag;
	if (am_curr_bm()) {
		post_filter_t filter = {
			.bid = ip->bid, .min = ip->id, .max = ip->id
		};
		if (set_post_flag(&filter, field, !set, false)) {
			set_post_flag_local(ip, flag, !set);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int post_list_with_filter(const post_filter_t *filter);

static int post_list_deleted(int bid, post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !am_curr_bm())
		return DONOTHING;

	post_filter_t filter = { .type = POST_LIST_TRASH, .bid = bid };
	post_list_with_filter(&filter);
	return FULLUPDATE;
}

static int post_list_admin_deleted(int bid, post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !HAS_PERM(PERM_OBOARDS))
		return DONOTHING;

	post_filter_t filter = { .type = POST_LIST_JUNK, .bid = bid };
	post_list_with_filter(&filter);
	return FULLUPDATE;
}

static int tui_post_list_selected(post_list_t *l, post_info_t *ip)
{
	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	char ans[3];
	getdata(t_lines - 1, 0, "切换模式到: 1)文摘 2)同主题 3)被 m 文章 4)原作"
			" 5)同作者 6)标题关键字 [1]: ", ans, sizeof(ans), DOECHO, YEA);

	int c = ans[0];
	if (!c)
		c = '1';
	c -= '1';

	const post_list_type_e types[] = {
		POST_LIST_DIGEST, POST_LIST_THREAD, POST_LIST_MARKED,
		POST_LIST_TOPIC, POST_LIST_AUTHOR, POST_LIST_KEYWORD,
	};
	if (c < 0 || c >= NELEMS(types))
		return MINIUPDATE;

	post_filter_t filter = { .bid = l->filter.bid, .type = types[c] };
	switch (filter.type) {
		case POST_LIST_DIGEST:
			filter.flag &= POST_FLAG_DIGEST;
			break;
		case POST_LIST_MARKED:
			filter.flag &= POST_FLAG_MARKED;
		case POST_LIST_AUTHOR: {
				char uname[IDLEN + 1];
				strlcpy(uname, ip->owner, sizeof(uname));
				getdata(t_lines - 1, 0, "您想查找哪位网友的文章? ", uname,
						sizeof(uname), DOECHO, YEA);
				user_id_t uid = get_user_id(uname);
				if (!uid)
					return MINIUPDATE;
				filter.uid = uid;
			}
			break;
		case POST_LIST_KEYWORD: {
				GBK_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
				getdata(t_lines - 1, 0, "您想查找的文章标题关键字: ",
						gbk_keyword, sizeof(gbk_keyword), DOECHO, YEA);
				convert_g2u(gbk_keyword, filter.utf8_keyword);
				if (!filter.utf8_keyword[0])
					return MINIUPDATE;
			}
			break;
		default:
			break;
	}

	post_list_with_filter(&filter);
	l->reload = true;
	return FULLUPDATE;
}

static bool match_filter(post_filter_t *filter, post_info_t *p)
{
	bool match = true;
	if (filter->uid)
		match &= p->uid == filter->uid;
	if (filter->min)
		match &= p->id >= filter->min;
	if (filter->max)
		match &= p->id <= filter->max;
	if (filter->tid)
		match &= p->id == filter->tid;
	return match;
}

static int relocate_to_filter(slide_list_t *p, post_filter_t *filter,
		bool upward)
{
	post_list_t *l = p->data;

	int found = -1;
	if (upward) {
		for (int i = p->cur - 1; i >= 0; --i) {
			if (match_filter(filter, l->index[i])) {
				found = i;
				break;
			}
		}
	} else {
		for (int i = p->cur + 1; i < l->icount; ++i) {
			if (match_filter(filter, l->index[i])) {
				found = i;
				break;
			}
		}
	}

	if (found >= 0) {
		p->cur = found;
		return MINIUPDATE;
	} else {
		// TODO
		return PARTUPDATE;
	}
}

static int relocate_to_author(slide_list_t *p, user_id_t uid, bool upward)
{
	post_list_t *l = p->data;
	post_filter_t filter = { .bid = l->filter.bid, .uid = uid };
	return relocate_to_filter(p, &filter, upward);
}

static int tui_search_author(slide_list_t *p, bool upward)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	char prompt[80];
	snprintf(prompt, sizeof(prompt), "向%s搜索作者 [%s]: ",
			upward ? "上" : "下", ip->owner);
	char ans[IDLEN + 1];
	getdata(t_lines - 1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);

	user_id_t uid = ip->uid;
	if (*ans && !streq(ans, ip->owner))
		uid = get_user_id(ans);

	return relocate_to_author(p, uid, upward);
}

static int tui_search_title(slide_list_t *p, bool upward)
{
	post_list_t *l = p->data;

	char prompt[80];
	static GBK_BUFFER(title, POST_TITLE_CCHARS) = "";
	snprintf(prompt, sizeof(prompt), "向%s搜索标题[%s]: ",
			upward ? "上" : "下", gbk_title);

	GBK_BUFFER(ans, POST_TITLE_CCHARS);
	getdata(t_lines - 1, 0, prompt, gbk_ans, sizeof(gbk_ans), DOECHO, YEA);

	if (*gbk_ans != '\0')
		strlcpy(gbk_title, gbk_ans, sizeof(gbk_title));

	if (!*gbk_title != '\0')
		return MINIUPDATE;

	post_filter_t filter = { .type = l->filter.type, .bid = l->filter.bid };
	convert_g2u(gbk_title, filter.utf8_keyword);
	return relocate_to_filter(p, &filter, upward);
}

static int jump_to_thread_first(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];
	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .max = ip->tid
	};
	return relocate_to_filter(p, &filter, true);
}

static int jump_to_thread_prev(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	if (ip->id == ip->tid)
		return DONOTHING;

	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .max = ip->id - 1,
	};
	return relocate_to_filter(p, &filter, true);
}

static int jump_to_thread_next(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];
	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .min = ip->id + 1,
	};
	return relocate_to_filter(p, &filter, false);
}

static int read_posts(post_list_t *l, post_info_t *ip, bool thread, bool user);

static int jump_to_thread_first_unread(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .min = brc_first_unread()
	};

	const int limit = 40;
	bool end = false;
	while (!end) {
		query_builder_t *b = build_post_query(&filter, true, limit);
		db_res_t *res = b->query(b);
		query_builder_free(b);

		int rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			post_id_t id = db_get_post_id(res, i, 0);
			if (brc_unread(id)) {
				post_info_t info;
				res_to_post_info(res, i, &info);
				read_posts(l, &info, true, false);
				db_clear(res);
				return FULLUPDATE;
			}
		}
		if (rows < limit)
			end = true;
		if (rows > 0)
			filter.min = db_get_post_id(res, rows - 1, 0) + 1;
		db_clear(res);
	}
	return PARTUPDATE;
}

static int jump_to_thread_last(slide_list_t *p)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	post_filter_t filter = {
		.bid = l->filter.bid, .tid = ip->tid, .min = ip->id + 1,
	};

	query_builder_t *b = build_post_query(&filter, false, 1);
	db_res_t *res = b->query(b);
	if (res && db_res_rows(res) > 0) {
		post_info_t info;
		res_to_post_info(res, 0, &info);
		read_posts(l, &info, true, false);
		db_clear(res);
		return FULLUPDATE;
	}
	return DONOTHING;
}

static int skip_post(slide_list_t *p, post_id_t pid)
{
	brc_mark_as_read(pid);
	if (++p->cur >= t_lines - 4) {
		p->base = SLIDE_LIST_NEXT;
		p->cur = 0;
	}
	return DONOTHING;
}

static int tui_delete_single_post(post_list_t *p, post_info_t *ip)
{
	if (ip->uid == session.uid || am_curr_bm()) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (delete_posts(&f, true, false, true)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int tui_undelete_single_post(post_list_t *p, post_info_t *ip)
{
	if (is_deleted(p->filter.type)) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (undelete_posts(&f, p->filter.type == POST_LIST_TRASH)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int show_post_info(const post_info_t *ip)
{
	clear();
	move(0, 0);
	prints("%s的详细信息:\n\n", "版面文章");

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);
	prints("标题: %s\n", gbk_title);
	prints("作者: %s\n", ip->owner);
	prints("时间: %s\n", getdatestring(ip->stamp, DATE_ZH));
	//	prints("大    小:     %d 字节\n", filestat.st_size);

	char buf[PID_BUF_LEN];
	prints("id:   %s\n", pid_to_base32(ip->id, buf, sizeof(buf)));
	prints("tid:  %s\n", pid_to_base32(ip->tid, buf, sizeof(buf)));
	prints("reid: %s\n", pid_to_base32(ip->reid, buf, sizeof(buf)));

	char link[STRLEN];
	snprintf(link, sizeof(link),
			"http://%s/bbs/con?new=1&bid=%d&f=%"PRIdPID"%s\n",
			BBSHOST, currbp->id, ip->id,
			(ip->flag & POST_FLAG_STICKY) ? "&s=1" : "");
	prints("\n%s", link);

	pressanykey();
	return FULLUPDATE;
}

static bool dump_content(const post_info_t *ip, char *file, size_t size)
{
	post_filter_t filter = {
		.min = ip->id, .max = ip->id, .bid = ip->bid,
		.type = post_list_type(ip),
	};
	db_res_t *res = query_post_by_pid(&filter, "content");
	if (!res || db_res_rows(res) < 1)
		return false;

	int ret = dump_content_to_gbk_file(db_get_value(res, 0, 0),
			db_get_length(res, 0, 0), file, size);

	db_clear(res);
	return ret == 0;
}

extern int tui_cross_post_legacy(const char *file, const char *title);

static int tui_cross_post(const post_info_t *ip)
{
	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	tui_cross_post_legacy(file, gbk_title);

	unlink(file);
	return FULLUPDATE;
}

static int forward_post(const post_info_t *ip, bool uuencode)
{
	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	tui_forward(file, gbk_title, uuencode);

	unlink(file);
	return FULLUPDATE;
}

static int reply_with_mail(const post_info_t *ip)
{
	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	post_reply(ip->owner, gbk_title, file);

	unlink(file);
	return FULLUPDATE;
}

static int tui_edit_post_title(post_info_t *ip)
{
	if (ip->uid != session.uid && !am_curr_bm())
		return DONOTHING;

	GBK_UTF8_BUFFER(title, POST_TITLE_CCHARS);

	ansi_filter(utf8_title, ip->utf8_title);
	convert_u2g(utf8_title, gbk_title);

	getdata(t_lines - 1, 0, "新文章标题: ", gbk_title, sizeof(gbk_title),
			DOECHO, NA);

	check_title(gbk_title, sizeof(gbk_title));
	convert_g2u(gbk_title, utf8_title);

	if (!*utf8_title || streq(utf8_title, ip->utf8_title))
		return MINIUPDATE;

	if (alter_title(ip, utf8_title)) {
		strlcpy(ip->utf8_title, utf8_title, sizeof(ip->utf8_title));
		return PARTUPDATE;
	}
	return MINIUPDATE;
}

static int tui_edit_post_content(post_info_t *ip)
{
	if (ip->uid != session.uid && !am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file)))
		return DONOTHING;

	set_user_status(ST_EDIT);

	clear();
	if (vedit(file, NA, NA) != -1) {
		char *content = convert_file_to_utf8_content(file);
		if (content) {
			if (alter_content(ip, content)) {
				char buf[STRLEN];
				snprintf(buf, sizeof(buf), "edited post #%"PRIdPID, ip->id);
				report(buf, currentuser.userid);
			}
			free(content);
		}
	}

	unlink(file);
	return FULLUPDATE;
}

extern int show_board_notes(const char *bname, int command);
extern int noreply;

static int tui_new_post(int bid, post_info_t *ip)
{
	time_t now = time(NULL);
	if (now - get_last_post_time() < 3) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("您太辛苦了，先喝杯咖啡歇会儿，3 秒钟后再发表文章。\n");
		pressreturn();
		return MINIUPDATE;
	}

	board_t board;
	if (!get_board_by_bid(bid, &board) ||
			!has_post_perm(&currentuser, &board)) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("此讨论区是唯读的, 或是您尚无权限在此发表文章。");
		pressreturn();
		return FULLUPDATE;
	}

	set_user_status(ST_POSTING);

	clear();
	show_board_notes(board.name, 1);

	struct postheader header;
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (ip) {
		header.reply = true;
		if (strncaseeq(ip->utf8_title, "Re: ", 4)) {
			convert_u2g(ip->utf8_title, header.title);
		} else {
			convert_u2g(ip->utf8_title, gbk_title);
			snprintf(header.title, sizeof(header.title), "Re: %s", gbk_title);
		}
	}

	strlcpy(header.ds, board.name, sizeof(header.ds));
	header.postboard = true;
	header.prefix[0] = '\0';

	if (post_header(&header) == YEA) {
		if (!header.reply && header.prefix[0]) {
#ifdef FDQUAN
			if (board.flag & BOARD_PREFIX_FLAG) {
				snprintf(gbk_title, sizeof(gbk_title),
						"\033[1;33m[%s]\033[m%s", header.prefix, header.title);
			} else {
				snprintf(gbk_title, sizeof(gbk_title),
						"\033[1m[%s]\033[m%s", header.prefix, header.title);
			}
#else
			snprintf(gbk_title, sizeof(gbk_title),
					"[%s]%s", header.prefix, header.title);
#endif
		} else {
			ansi_filter(header.title, header.title);
//			strlcpy(postfile.title, header.title, sizeof(postfile.title));
		}
	} else {
		return FULLUPDATE;
	}

	now = time(NULL);
	set_last_post_time(now);

	in_mail = NA;

	char file[HOMELEN];
	snprintf(file, sizeof(file), "tmp/editbuf.%d", getpid());
	if (ip) {
		char orig[HOMELEN];
		dump_content(ip, orig, sizeof(orig));
		do_quote(orig, file, header.include_mode);
		unlink(orig);
	}

	if (vedit(file, YEA, YEA) == -1) {
		unlink(file);
		clear();
		return FULLUPDATE;
	}

	// header.chk_anony
	// save_title
	// valid_title()

	if (header.mail_owner) {
		if (header.anonymous) {
			prints("对不起，您不能在匿名版使用寄信给原作者功能。");
		} else {
			if (!is_blocked(ip->owner)
					&& !mail_file(file, ip->owner, gbk_title)) {
				prints("信件已成功地寄给原作者 %s", ip->owner);
			}
			else {
				prints("信件邮寄失败，%s 无法收信。", ip->owner);
			}
		}
		pressanykey();
	}

	post_request_t req = {
		.autopost = false,
		.crosspost = false,
		.uname = currentuser.userid,
		.nick = currentuser.username,
		.user = &currentuser,
		.board = &board,
		.title = gbk_title,
		.content = NULL,
		.gbk_file = file,
		.sig = 0,
		.ip = NULL,
		.reid = ip ? ip->id : 0,
		.tid = ip ? ip->tid : 0,
		.locked = header.locked,
		.marked = false,
		.anony = header.anonymous,
		.cp = NULL,
	};

	post_id_t pid = publish_post(&req);
	if (pid) {
		brc_mark_as_read(pid);

		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "posted '%s' on %s",
				gbk_title, board.name);
		report(buf, currentuser.userid);
	}

	if (!is_junk_board(&board) && !header.anonymous) {
		set_safe_record();
		currentuser.numposts++;
		substitut_record(PASSFILE, &currentuser, sizeof (currentuser),
				usernum);
	}
	bm_log(currentuser.userid, currboard, BMLOG_POST, 1);
	return FULLUPDATE;
}

static int tui_save_post(const post_info_t *ip)
{
	if (!am_curr_bm())
		return DONOTHING;

	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file)))
		return DONOTHING;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(ip->utf8_title, gbk_title);

	a_Save(gbk_title, file, false, true);

	return MINIUPDATE;
}

static int tui_import_post(const post_info_t *ip)
{
	if (!HAS_PERM(PERM_BOARDS))
		return DONOTHING;

	if (DEFINE(DEF_MULTANNPATH)
			&& set_ann_path(NULL, NULL, ANNPATH_GETMODE) == 0)
		return FULLUPDATE;

	char file[HOMELEN];
	if (dump_content(ip, file, sizeof(file))) {
		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(ip->utf8_title, gbk_title);

		a_Import(gbk_title, file, NA);
	}

	if (DEFINE(DEF_MULTANNPATH))
		return FULLUPDATE;
	return DONOTHING;
}

static bool tui_input_range(post_id_t *min, post_id_t *max)
{
	char num[8];
	getdata(t_lines - 1, 0, "首篇文章编号: ", num, sizeof(num), DOECHO, YEA);
	*min = base32_to_pid(num);
	if (!*min) {
		move(t_lines - 1, 50);
		prints("错误编号...");
		egetch();
		return false;
	}

	getdata(t_lines - 1, 25, "末篇文章编号: ", num, sizeof(num), DOECHO, YEA);
	*max = base32_to_pid(num);
	if (*max < *min) {
		move(t_lines - 1, 50);
		prints("错误区间...");
		egetch();
		return false;
	}
	return true;
}

static int tui_delete_posts_in_range(slide_list_t *p)
{
	if (!am_curr_bm())
		return DONOTHING;

	post_list_t *l = p->data;
	if (l->filter.type != POST_LIST_NORMAL)
		return DONOTHING;

	post_id_t min = 0, max = 0;
	if (!tui_input_range(&min, &max))
		return MINIUPDATE;

	move(t_lines - 1, 50);
	if (askyn("确定删除", NA, NA)) {
		post_filter_t filter = {
			.bid = l->filter.bid, .min = min, .max = max
		};
		if (delete_posts(&filter, false, !HAS_PERM(PERM_OBOARDS), false)) {
			bm_log(currentuser.userid, currboard, BMLOG_DELETE, 1);
			l->reload = true;
		}
		return PARTUPDATE;
	}
	move(t_lines - 1, 50);
	clrtoeol();
	prints("放弃删除...");
	egetch();
	return MINIUPDATE;
}

static bool count_posts_in_range(post_id_t min, post_id_t max, bool asc,
		int sort, int least, char *file, size_t size)
{
	query_builder_t *b = query_builder_new(0);
	b->append(b, "SELECT");
	b->append(b, "uname, COUNT(*) AS a, SUM(marked::integer) AS m");
	b->append(b, ", SUM(digest::integer) AS g, SUM(water::integer) AS w, ");
	b->append(b, "SUM((NOT marked AND NOT digest AND NOT water)::integer) as n");
	b->sappend(b, "FROM", "posts");
	b->sappend(b, "WHERE", "id >= %"DBIdPID, min);
	b->sappend(b, "AND", "id <= %"DBIdPID, max);
	b->sappend(b, "GROUP BY", "uname");

	const char *field[] = { "a", "m", "g", "w", "n" };
	b->sappend(b, "ORDER BY", field[sort]);
	b->append(b, asc ? "ASC" : "DESC");

	db_res_t *res = b->query(b);
	query_builder_free(b);

	if (res) {
		snprintf(file, sizeof(file), "tmp/count.%d", getpid());
		FILE *fp = fopen(file, "w");
		if (fp) {
			fprintf(fp, "版    面: \033[1;33m%s\033[m\n", currboard);

			int count = 0;
			for (int i = db_res_rows(res) - 1; i >=0; --i) {
				count += db_get_bigint(res, i, 1);
			}
			char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
			fprintf(fp, "有效篇数: \033[1;33m%d\033[m 篇"
					" [\033[1;33m%s-%s\033[m]\n", count,
					pid_to_base32(min, min_str, sizeof(min_str)),
					pid_to_base32(max, max_str, sizeof(max_str)));
			
			const char *descr[] = {
				"总数", "被m的", "被g的", "被w的", "无标记"
			};
			fprintf(fp, "排序方式: \033[1;33m按%s%s\033[m\n", descr[sort],
					asc ? "升序" : "降序");
			fprintf(fp, "文章数下限: \033[1;33m%d\033[m\n\n", least);
			fprintf(fp, "\033[1;44;37m 使用者代号  │总  数│ 被M的│ 被G的"
					"│ 被w的│无标记 \033[m\n");

			for (int i = 0; i < db_res_rows(res); ++i) {
				int all = db_get_bigint(res, i, 1);
				if (all > least) {
					int m = db_get_bigint(res, i, 2);
					int g = db_get_bigint(res, i, 3);
					int w = db_get_bigint(res, i, 4);
					int n = db_get_bigint(res, i, 5);
					fprintf(fp, " %-12.12s  %6d  %6d  %6d  %6d  %6d \n",
							db_get_value(res, i, 0), all, m, g, w, n);
				}
			}
			fclose(fp);
			db_clear(res);
			return true;
		}
		db_clear(res);
	}
	return false;
}

static int tui_count_posts_in_range(post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !am_curr_bm())
		return DONOTHING;

	post_id_t min = 0, max = 0;
	if (!tui_input_range(&min, &max))
		return MINIUPDATE;

	char num[8];
	getdata(t_lines - 1, 0, "排序方式 (0)降序 (1)升序 ? : ", num, 2, DOECHO, YEA);
	bool asc = (num[0] == '1');

	getdata(t_lines - 1, 0, "排序选项 (0)总数 (1)被m (2)被g (3)被w (4)无标记 ? : ",
			num, 2, DOECHO, YEA);
	int sort = strtol(num, NULL, 10);
	if (sort < 0 || sort > 4)
		sort = 0;

	getdata(t_lines - 1, 0, "文章数下限(默认0): ", num, 6, DOECHO, YEA);
	int least = strtol(num, NULL, 10);

	char file[HOMELEN];
	if (!count_posts_in_range(min, max, asc, sort, least, file, sizeof(file)))
		return MINIUPDATE;

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
	snprintf(gbk_title, sizeof(gbk_title), "[%s]统计文章数(%s-%s)",
			currboard, pid_to_base32(min, min_str, sizeof(min_str)),
			pid_to_base32(max, max_str, sizeof(max_str)));
	mail_file(file, currentuser.userid, gbk_title);
	unlink(file);

	return MINIUPDATE;
}

#if 0
		struct stat filestat;
		int i, len;
		char tmp[1024];

		sprintf(genbuf, "upload/%s/%s", currboard, fileinfo->filename);
		if (stat(genbuf, &filestat) < 0) {
			clear();
			move(10, 30);
			prints("对不起，%s 不存在！\n", genbuf);
			pressanykey();
			clear();
			return FULLUPDATE;
		}

		clear();
		prints("文件详细信息\n\n");
		prints("版    名:     %s\n", currboard);
		prints("序    号:     第 %d 篇\n", ent);
		prints("文 件 名:     %s\n", fileinfo->filename);
		prints("上 传 者:     %s\n", fileinfo->owner);
		prints("上传日期:     %s\n", getdatestring(fileinfo->timeDeleted, DATE_ZH));
		prints("文件大小:     %d 字节\n", filestat.st_size);
		prints("文件说明:     %s\n", fileinfo->title);
		prints("URL 地址:\n");
		sprintf(tmp, "http://%s/upload/%s/%s", BBSHOST, currboard,
				fileinfo->filename);
		strtourl(genbuf, tmp);
		len = strlen(genbuf);
		clrtoeol();
		for (i = 0; i < len; i += 78) {
			strlcpy(tmp, genbuf + i, 78);
			tmp[78] = '\n';
			tmp[79] = '\0';
			outs(tmp);
		}
		if (!(ch == KEY_UP || ch == KEY_PGUP))
			ch = egetch();
		switch (ch) {
			case 'N':
			case 'Q':
			case 'n':
			case 'q':
			case KEY_LEFT:
				break;
			case ' ':
			case 'j':
			case KEY_RIGHT:
				if (DEFINE(DEF_THESIS)) {
					sread(0, 0, fileinfo);
					break;
				} else
					return READ_NEXT;
			case KEY_DOWN:
			case KEY_PGDN:
				return READ_NEXT;
			case KEY_UP:
			case KEY_PGUP:
				return READ_PREV;
			default:
				break;
		}
		return FULLUPDATE;
#endif

#if 0
	switch (ch) {
		case '*':
			show_file_info(ent, fileinfo, direct);
			break;
		case ' ':
		case 'j':
		case KEY_RIGHT:
			if (DEFINE(DEF_THESIS)) {
				sread(0, 0, fileinfo);
				break;
			} else
				return READ_NEXT;
		case KEY_DOWN:
		case KEY_PGDN:
			return READ_NEXT;
		case KEY_UP:
		case KEY_PGUP:
			return READ_PREV;
		case 'Y':
		case 'R':
		case 'y':
		case 'r': {
			board_t board;
			get_board(currboard, &board);
			noreply = (fileinfo->accessed[0] & FILE_NOREPLY)
					|| (board.flag & BOARD_NOREPLY_FLAG);
			local_article = true;
			if (!noreply || am_curr_bm()) {
				do_reply(fileinfo);
			} else {
				clear();
				prints("\n\n    对不起, 该文章有不可 RE 属性, 您不能回复(RE) 这篇文章.    ");
				pressreturn();
				clear();
			}
		}
			break;
		case Ctrl('R'):
			post_reply(ent, fileinfo, direct);
			break;
		case 'g':
			digest_post(ent, fileinfo, direct);
			break;
		case Ctrl('U'):
			sread(1, 1, fileinfo);
			break;
		case Ctrl('N'):
			locate_the_post(fileinfo, fileinfo->title, 5, 0, 1);
			sread(1, 0, fileinfo);
			break;
		case Ctrl('S'):
		case 'p':
			sread(0, 0, fileinfo);
			break;
		case Ctrl('A'):
			clear();
			show_author(0, fileinfo, '\0');
			return READ_NEXT;
			break;
		case 'S':
			if (!HAS_PERM(PERM_TALK))
				break;
			clear();
			s_msg();
			break;
		default:
			break;
	}
#endif

static int load_full_post(const post_filter_t *fp, post_info_full_t *ip,
		bool upward)
{
	query_builder_t *b = query_builder_new(0);
	b->sappend(b, "SELECT", POST_LIST_FIELDS_FULL);
	b->sappend(b, "FROM", post_table_name(fp));
	build_post_filter(b, fp);
	b->sappend(b, "ORDER BY", post_table_index(fp));
	b->append(b, upward ? "DESC" : "ASC");
	b->append(b, "LIMIT 1");

	db_res_t *res = b->query(b);
	query_builder_free(b);

	int rows = db_res_rows(res);
	if (rows > 0)
		res_to_post_info_full(res, 0, ip);
	else
		db_clear(res);
	return rows;
}

static int read_posts(post_list_t *l, post_info_t *ip, bool thread, bool user)
{
	post_info_full_t info = { .p = *ip };
	post_filter_t filter = l->filter;

	char file[HOMELEN];
	if (!dump_content(ip, file, sizeof(file)))
		return DONOTHING;

	bool end = false, upward = false;
	while (!end) {
		brc_mark_as_read(info.p.id);

		int ch = ansimore(file, false);

		move(t_lines - 1, 0);
		clrtoeol();
		prints("\033[0;1;44;31m[阅读文章]  \033[33m回信 R │ 结束 Q,← │上一封 ↑"
				"│下一封 <Space>,↓│主题阅读 ^s或p \033[m");
		refresh();

		if (!(ch == KEY_UP || ch == KEY_PGUP))
			ch = egetch();
		switch (ch) {
			case 'N': case 'Q': case 'n': case 'q': case KEY_LEFT:
				end = true;
				break;
			case 'Y': case 'R': case 'y': case 'r':
				// TODO
				tui_new_post(info.p.bid, &info.p);
				break;
			case ' ': case '\n': case KEY_DOWN: case KEY_RIGHT:
				upward = false;
				break;
			case KEY_UP: case 'u': case 'U':
				upward = true;
				break;
			case Ctrl('A'):
				t_query(info.p.owner);
				break;
			case Ctrl('R'):
				reply_with_mail(&info.p);
				break;
			case 'g':
				toggle_post_flag(&info.p, POST_FLAG_DIGEST, "digest");
				break;
			default:
				break;
		}

		unlink(file);
		free_post_info_full(&info);

		if (!end) {
			if (upward) {
				filter.min = 0;
				filter.max = info.p.id - 1;
			} else {
				filter.min = info.p.id + 1;
				filter.max = 0;
			}
			if (load_full_post(&filter, &info, upward)) {
				dump_content_to_gbk_file(info.content, info.length,
						file, sizeof(file));
			} else {
				end = true;
			}
		}
	}
	return FULLUPDATE;
}

static void construct_prompt(char *s, size_t size, const char **options,
		size_t len)
{
	char *p = s;
	strappend(&p, &size, "区段:");
	for (int i = 0; i < len; ++i) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d)%s", i + 1, options[i]);
		strappend(&p, &size, buf);
	}
	strappend(&p, &size, "[0]:");
}

#if 0
		clear();
		prints("\n\n您将进行区段转载。转载范围是：[%d -- %d]\n", num1, num2);
		prints("当前版面是：[ %s ] \n", currboard);
		board_complete(6, "请输入要转贴的讨论区名称: ", bname, sizeof(bname),
				AC_LIST_BOARDS_ONLY);
		if (!*bname)
			return FULLUPDATE;

		if (!strcmp(bname, currboard)&&session.status != ST_RMAIL) {
			prints("\n\n对不起，本文就在您要转载的版面上，所以无需转载。\n");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (askyn("确定要转载吗", NA, NA)==NA)
			return FULLUPDATE;
			case 4:
				break;
			case 5:
				break;
			default:
				break;
		}

	return DIRCHANGED;
}
#endif

extern int import_file(const char *title, const char *file, const char *path);

static int import_posts(post_filter_t *filter, const char *path)
{
	query_builder_t *b = query_builder_new(0);
	b->sappend(b, "UPDATE", post_table_name(filter));
	b->sappend(b, "SET", "imported = TRUE");
	build_post_filter(b, filter);
	b->sappend(b, "RETURNING", "title, content");

	db_res_t *res = b->query(b);
	query_builder_free(b);

	if (res) {
		int rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			GBK_BUFFER(title, POST_TITLE_CCHARS);
			convert_u2g(db_get_value(res, i, 0), gbk_title);

			char file[HOMELEN];
			dump_content_to_gbk_file(db_get_value(res, i, 1),
					db_get_length(res, i, 1), file, sizeof(file));

			import_file(gbk_title, file, path);
		}
		db_clear(res);
		return rows;
	}
	return 0;
}

static int tui_import_posts(post_filter_t *filter)
{
	if (DEFINE(DEF_MULTANNPATH) && !!set_ann_path(NULL, NULL, ANNPATH_GETMODE))
		return FULLUPDATE;

	char annpath[256];
	sethomefile(annpath, currentuser.userid, ".announcepath");

	FILE *fp = fopen(annpath, "r");
	if (!fp) {
		presskeyfor("对不起, 您没有设定丝路. 请先用 f 设定丝路.",
				t_lines - 1);
		return MINIUPDATE;
	}

	fscanf(fp, "%s", annpath);
	fclose(fp);

	if (!dashd(annpath)) {
		presskeyfor("您设定的丝路已丢失, 请重新用 f 设定.", t_lines - 1);
		return MINIUPDATE;
	}

	import_posts(filter, annpath);
	return PARTUPDATE;
}

static int operate_posts_in_range(int choice, post_list_t *l, post_id_t min,
		post_id_t max)
{
	bool deleted = is_deleted(l->filter.type);
	post_filter_t filter = { .bid = l->filter.bid, .min = min, .max = max };
	int ret = PARTUPDATE;
	switch (choice) {
		case 0:
			set_post_flag(&filter, "marked", true, true);
			break;
		case 1:
			set_post_flag(&filter, "digest", true, true);
			break;
		case 2:
			set_post_flag(&filter, "locked", true, true);
			break;
		case 3:
			delete_posts(&filter, true, !HAS_PERM(PERM_OBOARDS), false);
			break;
		case 4:
			ret = tui_import_posts(&filter);
			break;
		case 5:
			set_post_flag(&filter, "water", true, true);
			break;
		case 6:
			break;
		default:
			if (deleted) {
				undelete_posts(&filter, l->filter.type == POST_LIST_TRASH);
			} else {
				filter.flag |= POST_FLAG_WATER;
				delete_posts(&filter, true, !HAS_PERM(PERM_OBOARDS), false);
			}
			break;
	}

	switch (choice) {
		case 3:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEDEL, 1);
			break;
		case 4:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEANN, 1);
			break;
		default:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEOTHER, 1);
			break;
	}
	l->reload = true;
	return ret;
}

static int tui_operate_posts_in_range(slide_list_t *p)
{
	post_list_t *l = p->data;

	if (!am_curr_bm())
		return DONOTHING;

	const char *option8 = is_deleted(l->filter.type) ? "恢复" : "删水文";
	const char *options[] = {
		"保留",  "文摘", "不可RE", "删除",
		"精华区", "水文", "转载", option8,
	};

	char prompt[120], ans[8];
	construct_prompt(prompt, sizeof(prompt), options, NELEMS(options));
	getdata(t_lines - 1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);

	int choice = *ans - '1';
	if (choice < 0 || choice >= NELEMS(options))
		return MINIUPDATE;

	post_id_t min, max;
	if (!tui_input_range(&min, &max))
		return MINIUPDATE;

	char min_str[PID_BUF_LEN], max_str[PID_BUF_LEN];
	snprintf(prompt, sizeof(prompt), "区段[%s]操作范围 [ %s -- %s ]，确定吗",
			options[choice], pid_to_base32(min, min_str, sizeof(min_str)),
			pid_to_base32(min, max_str, sizeof(max_str)));
	if (!askyn(prompt, NA, YEA))
		return MINIUPDATE;

	return operate_posts_in_range(choice, l, min, max);
}

static void operate_posts_in_batch(post_filter_t *fp, post_info_t *ip, int mode,
		int choice, bool first, post_id_t pid, bool quote, bool junk,
		const char *annpath, const char *utf8_keyword, const char *gbk_keyword)
{
	post_filter_t filter = { .type = fp->type, .bid = fp->bid };
	switch (mode) {
		case 1:
			filter.uid = ip->uid;
			break;
		case 2:
			strlcpy(filter.utf8_keyword, utf8_keyword,
					sizeof(filter.utf8_keyword));
			break;
		default:
			filter.tid = ip->tid;
			break;
	}
	if (!first)
		filter.min = ip->id;

	switch (choice) {
		case 0:
			delete_posts(&filter, junk, HAS_PERM(PERM_OBOARDS), false);
			break;
		case 1:
			set_post_flag(&filter, "marked", false, true);
			break;
		case 2:
			set_post_flag(&filter, "digest", false, true);
			break;
		case 3:
			import_posts(&filter, annpath);
			break;
		case 4:
			set_post_flag(&filter, "water", false, true);
			break;
		case 5:
			set_post_flag(&filter, "locked", false, true);
			break;
		case 6:
			// pack thread
			break;
		default:
			if (is_deleted(fp->type)) {
				undelete_posts(&filter, fp->type == POST_LIST_TRASH);
			} else {
				// merge thread
			}
			break;
	}

	switch (choice) {
		case 0:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEDEL, 1);
			break;
		case 3:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEANN, 1);
			break;
		case 6:
			bm_log(currentuser.userid, currboard, BMLOG_COMBINE, 1);
			break;
		default:
			bm_log(currentuser.userid, currboard, BMLOG_RANGEOTHER, 1);
			break;
	}
}

static int tui_operate_posts_in_batch(slide_list_t *p)
{
	if (!am_curr_bm())
		return DONOTHING;

	post_list_t *l = p->data;
	bool deleted = is_deleted(l->filter.type);

	const char *batch_modes[] = { "相同主题", "相同作者", "相关主题" };
	const char *option8 = deleted ? "恢复" : "合并";
	const char *options[] = {
		"删除", "保留", "文摘", "精华区", "水文", "不可RE", "合集", option8
	};

	char ans[16];
	move(t_lines - 1, 0);
	clrtoeol();
	ans[0] = '\0';
	getdata(t_lines - 1, 0, "执行: 1) 相同主题  2) 相同作者 3) 相关主题"
			" 0) 取消 [0]: ", ans, sizeof(ans), DOECHO, YEA);
	int mode = strtol(ans, NULL, 10) - 1;
	if (mode < 0 || mode >= NELEMS(batch_modes))
		return MINIUPDATE;

	char prompt[120];
	construct_prompt(prompt, sizeof(prompt), options, NELEMS(options));
	getdata(t_lines - 1, 0, prompt, ans, sizeof(ans), DOECHO, YEA);
	int choice = strtol(ans, NULL, 10) - 1;
	if (choice < 0 || choice >= NELEMS(options))
		return MINIUPDATE;

	char buf[STRLEN];
	move(t_lines - 1, 0);
	snprintf(buf, sizeof(buf), "确定要执行%s[%s]吗", batch_modes[mode],
			options[choice]);
	if (!askyn(buf, NA, NA))
		return MINIUPDATE;

	post_id_t pid = 0;
	bool quote = true;
	if (choice == 6) {
		move(t_lines-1, 0);
		quote = askyn("制作的合集需要引言吗？", YEA, YEA);
	} else if (choice == 7) {
		if (!deleted) {
			getdata(t_lines - 1, 0, "本主题加至版面第几篇后？", ans,
					sizeof(ans), DOECHO, YEA);
			pid = base32_to_pid(ans);
		}
	}

	GBK_UTF8_BUFFER(keyword, POST_TITLE_CCHARS);
	if (mode == 2) {
		getdata(t_lines - 1, 0, "请输入主题关键字: ", gbk_keyword,
				sizeof(gbk_keyword), DOECHO, YEA);
		if (gbk_keyword[0] == '\0')
			return MINIUPDATE;
		convert_g2u(gbk_keyword, utf8_keyword);
	}

	bool junk = true;
	if (choice == 0) {
		junk = askyn("是否小d", YEA, YEA);
	}

	bool first = false;
	move(t_lines - 1, 0);
	snprintf(buf, sizeof(buf), "是否从%s第一篇开始%s (Y)第一篇 (N)目前这一篇",
			(mode == 1) ? "该作者" : "此主题", options[choice]);
	first = askyn(buf, YEA, NA);

	char annpath[512];
	if (choice == 3) {
		if (DEFINE(DEF_MULTANNPATH) &&
				!set_ann_path(NULL, NULL, ANNPATH_GETMODE))
			return FULLUPDATE;

		sethomefile(annpath, currentuser.userid, ".announcepath");
		FILE *fp = fopen(annpath, "r");
		if (!fp) {
			presskeyfor("对不起, 您没有设定丝路. 请先用 f 设定丝路.", t_lines - 1);
			return MINIUPDATE;
		}
		fscanf(fp, "%s", annpath);
		fclose(fp);
		if (!dashd(annpath)) {
			presskeyfor("您设定的丝路已丢失, 请重新用 f 设定.",t_lines - 1);
			return MINIUPDATE;
		}
	}

	operate_posts_in_batch(&l->filter, l->index[p->cur], mode, choice, first,
			pid, quote, junk, annpath, utf8_keyword, gbk_keyword);
#if 0
	if (BMch == 7) {
		if (strneq(keyword, "Re: ", 4) || strneq(keyword, "RE: ", 4))
			snprintf(buf, sizeof(buf), "[合集]%s", keyword + 4);
		else
			snprintf(buf, sizeof(buf), "[合集]%s", keyword);

		ansi_filter(keyword, buf);

		sprintf(buf, "tmp/%s.combine", currentuser.userid);

		Postfile(buf, currboard, keyword, 2);
		unlink(buf);
	}
#endif
	return PARTUPDATE;
}

extern int tui_select_board(int);

static int switch_board(post_list_t *l)
{
	if (l->filter.type != POST_LIST_NORMAL || !l->filter.bid)
		return DONOTHING;

	int bid = tui_select_board(l->filter.bid);
	if (bid) {
		l->filter.bid = bid;
		l->relocate = l->reload = l->sreload = true;
		l->pos = get_post_list_position(&l->filter);
	}
	return FULLUPDATE;
}

extern int show_online(void);
extern int thesis_mode(void);
extern int deny_user(void);
extern int club_user(void);
extern int tui_send_msg(const char *);
extern int x_lockscreen(void);
extern int vote_results(const char *bname);
extern int b_vote(void);
extern int vote_maintain(const char *bname);
extern int b_notes_edit(void);
extern int b_notes_passwd(void);
extern int mainreadhelp(void);
extern int show_b_note(void);
extern int show_b_secnote(void);
extern int into_announce(void);

static slide_list_handler_t post_list_handler(slide_list_t *p, int ch)
{
	post_list_t *l = p->data;
	post_info_t *ip = l->index[p->cur];

	switch (ch) {
		case '\n': case '\r': case KEY_RIGHT: case 'r':
		case Ctrl('S'): case 'p':
			return read_posts(l, ip, false, false);
		case Ctrl('U'):
			return read_posts(l, ip, false, true);
		case '_':
			return toggle_post_lock(ip);
		case '@':
			return show_online();
		case '#':
			return toggle_post_stickiness(ip, l);
		case 'm':
			return toggle_post_flag(ip, POST_FLAG_MARKED, "marked");
		case 'g':
			return toggle_post_flag(ip, POST_FLAG_DIGEST, "digest");
		case 'w':
			return toggle_post_flag(ip, POST_FLAG_WATER, "water");
		case 'T':
			return tui_edit_post_title(ip);
		case 'E':
			return tui_edit_post_content(ip);
		case Ctrl('P'):
			return tui_new_post(l->filter.bid, NULL);
		case 'i':
			return tui_save_post(ip);
		case 'I':
			return tui_import_post(ip);
		case 'D':
			return tui_delete_posts_in_range(p);
		case 'L':
			return tui_operate_posts_in_range(p);
		case 'b':
			return tui_operate_posts_in_batch(p);
		case 'C':
			return tui_count_posts_in_range(l->filter.type);
		case '.':
			return post_list_deleted(l->filter.bid, l->filter.type);
		case 'J':
			return post_list_admin_deleted(l->filter.bid, l->filter.type);
		case Ctrl('G'): case Ctrl('T'): case '`':
			return tui_post_list_selected(l, ip);
		case 'a':
			return tui_search_author(p, false);
		case 'A':
			return tui_search_author(p, true);
		case '/':
			return tui_search_title(p, false);
		case '?':
			return tui_search_title(p, true);
		case '=':
			return jump_to_thread_first(p);
		case '[':
			return jump_to_thread_prev(p);
		case ']':
			return jump_to_thread_next(p);
		case 'n': case Ctrl('N'):
			return jump_to_thread_first_unread(p);
		case '\\':
			return jump_to_thread_last(p);
		case 'K':
			return skip_post(p, ip->id);
		case 'c':
			brc_clear(ip->id);
			return PARTUPDATE;
		case 'f':
			brc_clear_all();
			return PARTUPDATE;
		case 'd':
			return tui_delete_single_post(l, ip);
		case 'Y':
			return tui_undelete_single_post(l, ip);
		case '*':
			return show_post_info(ip);
		case Ctrl('C'):
			return tui_cross_post(ip);
		case 'F':
			return forward_post(ip, false);
		case 'U':
			return forward_post(ip, true);
		case Ctrl('R'):
			return reply_with_mail(ip);
		case 't':
			return thesis_mode();
		case '!':
			return Q_Goodbye();
		case 'S':
			s_msg();
			return FULLUPDATE;
		case 'o':
			show_online_followings();
			return FULLUPDATE;
		case 'Z':
			tui_send_msg(ip->owner);
			return FULLUPDATE;
		case '|':
			return x_lockscreen();
		case 'R':
			return vote_results(currboard);
		case 'v':
			return b_vote();
		case 'V':
			return vote_maintain(currboard);
		case KEY_TAB:
			return show_b_note();
		case 'z':
			return show_b_secnote();
		case 'W':
			return b_notes_edit();
		case Ctrl('W'):
			return b_notes_passwd();
		case 'h':
			return mainreadhelp();
		case Ctrl('A'):
			return t_query(ip->owner);
		case Ctrl('D'):
			return deny_user();
		case Ctrl('K'):
			return club_user();
		case 'x':
			if (into_announce() != DONOTHING)
				return FULLUPDATE;
		case 's':
			return switch_board(l);
		default:
			return DONOTHING;
	}
}

static void save_post_list_position(const slide_list_t *p,
		const post_list_t *l)
{
	post_info_t *min = l->icount ? l->index[0] : NULL;
	if (min && min->flag & POST_FLAG_STICKY)
		min = l->count ? l->posts + l->count - 1 : NULL;

	post_info_t *cur = NULL;
	for (int i = l->icount - 1; i >= 0; --i) {
		if (!(l->index[i]->flag & POST_FLAG_STICKY)) {
			cur = l->index[i];
			break;
		}
	}
	if (!cur)
		cur = min;

	l->pos->min_tid = min ? min->tid : 0;
	l->pos->cur_tid = cur ? cur->tid : 0;
	l->pos->min_pid = min ? min->id : 0;
	l->pos->cur_pid = cur ? cur->id : 0;
}

static int post_list_with_filter(const post_filter_t *filter)
{
	post_list_t p = {
		.relocate = true,
		.filter = *filter,
		.reload = true,
		.pos = get_post_list_position(filter),
		.sreload = filter->type == POST_LIST_NORMAL,
	};

	int status = session.status;
	set_user_status(ST_READING);

	slide_list_t s = {
		.base = SLIDE_LIST_CURRENT,
		.data = &p,
		.update = FULLUPDATE,
		.loader = post_list_loader,
		.title = post_list_title,
		.display = post_list_display,
		.handler = post_list_handler,
	};

	slide_list(&s);

	save_post_list_position(&s, &p);

	free(p.index);
	free(p.posts);
	free(p.sposts);

	set_user_status(status);
	return 0;
}

int post_list_normal(int bid)
{
	post_filter_t filter = { .type = POST_LIST_NORMAL, .bid = bid };
	return post_list_with_filter(&filter);
}
