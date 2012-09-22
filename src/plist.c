#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/mail.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct {
	post_list_type_e type;
	post_filter_t filter;

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
	if (l->type == POST_LIST_NORMAL && !l->sposts && l->sreload) {
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

static slide_list_loader_t post_list_loader(slide_list_t *p)
{
	post_list_t *l = p->data;

	if (l->reload)
		p->base = SLIDE_LIST_NEXT;
	if (p->base == SLIDE_LIST_CURRENT)
		return 0;

	int page = t_lines - 4;
	if (!l->index) {
		l->index = malloc(sizeof(*l->index) * page);
		l->posts = malloc(sizeof(*l->posts) * page);
		l->sposts = NULL;
		l->icount = l->count = l->scount = 0;
	}

	bool asc = is_asc(p->base);
	query_builder_t *b = build_post_query(l->type, &l->filter, asc, page);
	db_res_t *res = query_builder_query(b);
	query_builder_free(b);
	res_to_array(res, l, p->base, page);
	l->last_query_rows = db_res_rows(res);

	if ((p->base == SLIDE_LIST_NEXT && l->last_query_rows < page)
			|| p->base == SLIDE_LIST_BOTTOMUP) {
		load_sticky_posts(l);
	}
	db_clear(res);

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

static slide_list_title_t post_list_title(slide_list_t *p)
{
	return;
}

static void post_list_display_entry(post_info_t *p)
{
	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(p->utf8_title, gbk_title);
	prints("  %s %s\n", p->owner, gbk_title);
}

static slide_list_display_t post_list_display(slide_list_t *p)
{
	post_list_t *l = p->data;
	for (int i = 0; i < l->icount; ++i) {
		post_list_display_entry(l->index[i]);
	}
	return 0;
}

static int toggle_post_lock(int bid, post_info_t *ip)
{
	bool locked = ip->flag & POST_FLAG_LOCKED;
	if (am_curr_bm() || (session.id == ip->uid && !locked)) {
		if (set_post_flag_unchecked(bid, ip->id, "locked", !locked)) {
			set_post_flag(ip, POST_FLAG_LOCKED, !locked);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int toggle_post_stickiness(int bid, post_info_t *ip, post_list_t *l)
{
	bool sticky = ip->flag & POST_FLAG_STICKY;
	if (am_curr_bm() && sticky_post_unchecked(bid, ip->id, !sticky)) {
		l->sreload = l->reload = true;
		return PARTUPDATE;
	}
	return DONOTHING;
}

static int toggle_post_flag(int bid, post_info_t *ip, post_flag_e flag,
		const char *field)
{
	bool set = ip->flag & flag;
	if (am_curr_bm()) {
		if (set_post_flag_unchecked(bid, ip->id, field, !set)) {
			set_post_flag(ip, flag, !set);
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, user_id_t uid, const char *keyword);

static int post_list_deleted(int bid, post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !am_curr_bm())
		return DONOTHING;
	post_list(bid, POST_LIST_TRASH, 0, SLIDE_LIST_BOTTOMUP, 0, NULL);
	return FULLUPDATE;
}

static int post_list_admin_deleted(int bid, post_list_type_e type)
{
	if (type != POST_LIST_NORMAL || !HAS_PERM(PERM_OBOARDS))
		return DONOTHING;
	post_list(bid, POST_LIST_JUNK, 0, SLIDE_LIST_BOTTOMUP, 0, NULL);
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

static int tui_delete_single_post(post_list_t *p, post_info_t *ip)
{
	if (ip->uid == session.uid || am_curr_bm()) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (delete_posts(&f, true, false)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int tui_undelete_single_post(post_list_t *p, post_info_t *ip)
{
	if (p->type == POST_LIST_JUNK || p->type == POST_LIST_TRASH) {
		post_filter_t f = {
			.bid = p->filter.bid, .min = ip->id, .max = ip->id,
		};
		if (undelete_posts(&f, p->type == POST_LIST_TRASH)) {
			p->reload = true;
			return PARTUPDATE;
		}
	}
	return DONOTHING;
}

static int forward_post(post_list_type_e type, post_info_t *ip, bool uuencode)
{
	query_builder_t *b = query_builder_new(0);
	query_builder_append(b, "SELECT title, content FROM");
	query_builder_append(b, post_table_name(type));
	query_builder_append(b, "WHERE id = %"DBIdPID, ip->id);

	db_res_t *res = query_builder_query(b);
	query_builder_free(b);

	if (res && db_res_rows(res) == 1) {
		GBK_BUFFER(title, POST_TITLE_CCHARS);
		convert_u2g(db_get_value(res, 0, 0), gbk_title);

		char file[HOMELEN];
		dump_content_to_gbk_file(db_get_value(res, 0, 1),
				db_get_length(res, 0, 1), file, sizeof(file));

		db_clear(res);

		tui_forward(file, gbk_title, uuencode);
		return FULLUPDATE;
	} else {
		return DONOTHING;
	}
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
		case '_':
			return toggle_post_lock(l->filter.bid, ip);
		case '@':
			return show_online();
		case '#':
			return toggle_post_stickiness(l->filter.bid, ip, l);
		case 'm':
			return toggle_post_flag(l->filter.bid, ip,
					POST_FLAG_MARKED, "marked");
		case 'g':
			return toggle_post_flag(l->filter.bid, ip,
					POST_FLAG_DIGEST, "digest");
		case 'w':
			return toggle_post_flag(l->filter.bid, ip,
					POST_FLAG_WATER, "water");
		case '.':
			return post_list_deleted(l->filter.bid, l->type);
		case 'J':
			return post_list_admin_deleted(l->filter.bid, l->type);
		case 'a':
			return tui_search_author(p, false);
		case 'A':
			return tui_search_author(p, true);
		case '=':
			return jump_to_thread_first(p);
		case '[':
			return jump_to_thread_prev(p);
		case ']':
			return jump_to_thread_next(p);
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
		case 'F':
			return forward_post(l->type, ip, false);
		case 'U':
			return forward_post(l->type, ip, true);
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
		case 'h': case Ctrl('J'):
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
		default:
			return DONOTHING;
	}
}

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, user_id_t uid, const char *keyword)
{
	post_list_t p = {
		.type = type, .filter = { .bid = bid, .uid = uid },
		.relocate = true, .reload = false, .last_query_rows = 0,
		.index = NULL, .posts = NULL, .sposts = NULL,
		.icount = 0, .count = 0, .scount = 0, .sreload = false,
	};
	if (keyword)
		strlcpy(p.filter.utf8_keyword, keyword, sizeof(p.filter.utf8_keyword));
	if (is_asc(base)) {
		p.filter.min = pid;
		p.filter.max = 0;
	} else {
		p.filter.min = 0;
		p.filter.max = pid;
	}

	slide_list_t s = {
		.base = base,
		.data = &p,
		.update = FULLUPDATE,
		.loader = post_list_loader,
		.title = post_list_title,
		.display = post_list_display,
		.handler = post_list_handler,
	};

	slide_list(&s);

	free(p.index);
	free(p.posts);
	free(p.sposts);
	return 0;
}

int post_list_normal_range(int bid, post_id_t pid, slide_list_base_e base)
{
	return post_list(bid, POST_LIST_NORMAL, pid, base, 0, NULL);
}

int post_list_normal(int bid)
{
	return 0;
}
