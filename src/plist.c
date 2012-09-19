#include "bbs.h"
#include "fbbs/brc.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

typedef struct {
	post_list_filter_t filter;

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

static post_id_t pid_base(post_list_t *l, slide_list_base_e base)
{
	switch (base) {
		case SLIDE_LIST_TOPDOWN:
			return 0;
		case SLIDE_LIST_BOTTOMUP:
			return POST_ID_MAX;
		default:
			if (!l->posts || !l->count)
				return l->filter.pid;
			if (l->reload || !is_asc(base))
				return l->index[0]->id;
			else
				return l->posts[l->count - 1].id;
	}
}

static db_res_t *exec_query(const char *query, post_list_filter_t *f)
{
	switch (f->type) {
		case POST_LIST_AUTHOR:
			return db_query(query, f->bid, f->pid, f->uid);
		case POST_LIST_KEYWORD:
			return db_query(query, f->bid, f->pid, f->utf8_keyword);
		default:
			return db_query(query, f->bid, f->pid);
	}
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
	if (!l->sposts && l->sreload) {
		l->scount = _load_sticky_posts(&l->filter, &l->sposts);
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
	l->filter.pid = pid_base(l, p->base);

	char query[256];
	build_post_query(query, sizeof(query), l->filter.type, asc, page);

	db_res_t *res = exec_query(query, &l->filter);
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

static int relocate_to_author(slide_list_t *p, user_id_t uid, bool upward)
{
	post_list_t *l = p->data;

	int found = -1;
	if (upward) {
		for (int i = p->cur - 1; i >= 0; --i) {
			if (l->index[i]->uid == uid) {
				found = i;
				break;
			}
		}
	} else {
		for (int i = p->cur + 1; i < l->icount; ++i) {
			if (l->index[i]->flag & POST_FLAG_STICKY)
				break;
			if (l->index[i]->uid == uid) {
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
	if (p->filter.type == POST_LIST_JUNK
			|| p->filter.type == POST_LIST_TRASH) {
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
			return post_list_deleted(l->filter.bid, l->filter.type);
		case 'J':
			return post_list_admin_deleted(l->filter.bid, l->filter.type);
		case 'a':
			return tui_search_author(p, false);
		case 'A':
			return tui_search_author(p, true);
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
		.filter = { .bid = bid, .type = type, .pid = pid, .uid = uid },
		.relocate = true, .reload = false, .last_query_rows = 0,
		.index = NULL, .posts = NULL, .sposts = NULL,
		.icount = 0, .count = 0, .scount = 0, .sreload = false,
	};
	if (keyword)
		strlcpy(p.filter.utf8_keyword, keyword, sizeof(p.filter.utf8_keyword));

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
