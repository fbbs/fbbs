#include "bbs.h"
#include "fbbs/fbbs.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

#define POST_LIST_FIELDS  \
	"id, reid, tid, owner, uname, stamp, digest, marked," \
	" locked, imported, replies, comments, score, title"

typedef enum {
	POST_LIST_NORMAL = 1,
	POST_LIST_THREAD,
	POST_LIST_MARKED,
	POST_LIST_DIGEST,
	POST_LIST_AUTHOR,
	POST_LIST_KEYWORD,
	POST_LIST_TRASH,
	POST_LIST_JUNK,
} post_list_type_e;

enum {
	POST_LIST_KEYWORD_LEN = 19,
};

typedef struct {
	int replies;
	int comments;
	int score;
	int flag;
	user_id_t uid;
	post_id_t id;
	post_id_t reid;
	post_id_t tid;
	fb_time_t stamp;
	char owner[IDLEN + 1];
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
} post_info_t;

typedef struct {
	post_info_t *sposts;
	post_info_t *posts;
	int scount;
	int count;
	post_list_type_e type;
	post_id_t pid;
	user_id_t uid;
	UTF8_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
} post_list_t;

static void res_to_post_info(db_res_t *r, int i, post_info_t *p)
{
	p->id = db_get_post_id(r, i, 0);
	p->reid = db_get_post_id(r, i, 1);
	p->tid = db_get_post_id(r, i, 2);
	p->uid = db_get_is_null(r, i, 3) ? 0 : db_get_user_id(r, i, 3);
	strlcpy(p->owner, db_get_value(r, i, 4), sizeof(p->owner));
	p->stamp = db_get_time(r, i, 5);
	p->flag = (db_get_bool(r, i, 6) ? POST_FLAG_DIGEST : 0)
			| (db_get_bool(r, i, 7) ? POST_FLAG_MARKED : 0)
			| (db_get_bool(r, i, 8) ? POST_FLAG_LOCKED : 0)
			| (db_get_bool(r, i, 9) ? POST_FLAG_IMPORT : 0);
	p->replies = db_get_integer(r, i, 10);
	p->comments = db_get_integer(r, i, 11);
	p->score = db_get_integer(r, i, 12);
	strlcpy(p->utf8_title, db_get_value(r, i, 13), sizeof(p->utf8_title));
}

static size_t post_table_name(char *table, size_t size, post_list_type_e type)
{
	const char *t;
	switch (type) {
		case POST_LIST_TRASH:
		case POST_LIST_JUNK:
			t = "posts_deleted";
			break;
		default:
			t = "posts";
			break;
	}
	return strlcpy(table, t, size);
}

static const char *post_filter(post_list_type_e type)
{
	switch (type) {
		case POST_LIST_MARKED:
			return "marked";
		case POST_LIST_DIGEST:
			return "digest";
		case POST_LIST_AUTHOR:
			return "p.owner = %%"DBIdPID;
		case POST_LIST_KEYWORD:
			return "p.title LIKE %%s";
		case POST_LIST_TRASH:
			return "bm_visible";
		case POST_LIST_JUNK:
			return "NOT bm_visible";
		default:
			return "TRUE";
	}
}

static int build_query(char *query, size_t size, post_list_type_e type,
		bool asc, int limit)
{
	char table[16];
	post_table_name(table, sizeof(table), type);

	return snprintf(query, size, "SELECT " POST_LIST_FIELDS
			" FROM %s WHERE id %c %%"DBIdPID" AND %s ORDER BY id %s LIMIT %d",
			table, asc ? '>' : '<', post_filter(type),
			asc ? "ASC" : "DESC", limit);
}

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
				return l->pid;
			return is_asc(base) ? l->posts[l->count - 1].id : l->posts->id;
	}
}

static db_res_t *exec_query(const char *query, post_list_type_e type,
		post_id_t pid, user_id_t uid, const char *keyword)
{
	switch (type) {
		case POST_LIST_AUTHOR:
			return db_query(query, pid, uid);
		case POST_LIST_KEYWORD:
			return db_query(query, pid, keyword);
		default:
			return db_query(query, pid);
	}
}

static void res_to_array(db_res_t *r, post_list_t *l, slide_list_base_e base,
		int page)
{
	if (!l->posts) {
		l->posts = malloc(sizeof(*l->posts) * page);
		l->count = 0;
	}

	int rows = db_res_rows(r);
	if (rows < 1) {
		return;
	}

	bool asc = is_asc(base);

	if (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_BOTTOMUP
			|| rows >= page) {
		rows = rows > page ? page : rows;
		for (int i = 0; i < rows; ++i)
			res_to_post_info(r, asc ? i : rows - i - 1, l->posts + i);
		l->count = rows;
	} else {
		int extra = l->count + rows - page;
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
	if (!l->sposts)
		l->sposts = malloc(sizeof(*l->sposts) * MAX_NOTICE);

	db_res_t *r = db_query("SELECT " POST_LIST_FIELDS " FROM posts"
			" WHERE sticky ORDER BY id DESC");
	if (r) {
		l->scount = db_res_rows(r);
		for (int i = 0; i < l->scount; ++i) {
			res_to_post_info(r, i, l->sposts + i);
			l->sposts[i].flag &= POST_FLAG_STICKY;
		}
		db_clear(r);
	}

	return;
}

static slide_list_loader_t post_list_loader(slide_list_t *p)
{
	post_list_t *l = p->data;
	if (p->base == SLIDE_LIST_CURRENT)
		return 0;

	int page = t_lines - 4;

	bool asc = is_asc(p->base);
	post_id_t pid = pid_base(l, p->base);

	char query[512];
	build_query(query, sizeof(query), l->type, asc, page);

	db_res_t *res = exec_query(query, l->type, pid, l->uid, l->utf8_keyword);
	res_to_array(res, l, p->base, page);

	if ((p->base == SLIDE_LIST_NEXT && db_res_rows(res) < page)
			|| p->base == SLIDE_LIST_BOTTOMUP) {
		load_sticky_posts(l);
	}

	db_clear(res);
	p->update = PARTUPDATE;
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

	if (!l->posts)
		return 0;
	for (int i = 0; i < l->count; ++i) {
		post_list_display_entry(l->posts + i);
	}

	if (!l->sposts)
		return 0;
	int remain = t_lines - 4 - l->count;
	for (int i = 0; i < remain && i < l->scount; ++i) {
		post_list_display_entry(l->sposts + i);
	}

	return 0;
}

static slide_list_handler_t post_list_handler(slide_list_t *p, int ch)
{
	return 0;
}

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, user_id_t uid, const char *keyword)
{
	post_list_t p = {
		.sposts = NULL, .scount = 0, .posts = NULL, .count = 0,
		.type = type, .pid = pid, .uid = uid,
	};
	if (keyword)
		strlcpy(p.utf8_keyword, keyword, sizeof(p.utf8_keyword));

	slide_list_t s = {
		.base = base,
		.data = &p,
		.loader = post_list_loader,
		.title = post_list_title,
		.display = post_list_display,
		.handler = post_list_handler,
	};

	slide_list(&s);

	free(p.sposts);
	free(p.posts);
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
