#include "bbs.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/tui_list.h"

#define POST_LIST_FIELDS  \
	"p.id, p.reid, p.tid, u.id, u.name, p.stamp, p.digest, p.marked," \
	" p.locked, p.imported, p.sticky, p.replies, p.comments, p.score, p.title"

typedef enum {
	POST_LIST_NORMAL = 1,
	POST_LIST_THREAD,
	POST_LIST_MARKED,
	POST_LIST_DIGEST,
	POST_LIST_AUTHOR,
	POST_LIST_KEYWORD,
} post_list_type_e;

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
	slide_list_base_e base;
	post_id_t pid;
} post_list_t;

static void res_to_post_info(db_res_t *r, int i, post_info_t *p)
{
	p->id = db_get_post_id(r, i, 0);
	p->reid = db_get_post_id(r, i, 1);
	p->tid = db_get_post_id(r, i, 2);
	p->uid = db_get_user_id(r, i, 3);
	strlcpy(p->owner, db_get_value(r, i, 4), sizeof(p->owner));
	p->stamp = db_get_time(r, i, 5);
	p->flag = (db_get_bool(r, i, 6) ? POST_FLAG_DIGEST : 0)
			| (db_get_bool(r, i, 7) ? POST_FLAG_MARKED : 0)
			| (db_get_bool(r, i, 8) ? POST_FLAG_LOCKED : 0)
			| (db_get_bool(r, i, 9) ? POST_FLAG_IMPORT : 0)
			| (db_get_bool(r, i, 10) ? POST_FLAG_STICKY : 0);
	p->replies = db_get_integer(r, i, 11);
	p->comments = db_get_integer(r, i, 12);
	p->score = db_get_integer(r, i, 13);
	strlcpy(p->utf8_title, db_get_value(r, i, 14), sizeof(p->utf8_title));
}

static slide_list_loader_t post_list_loader(slide_list_t *p, slide_list_base_e base)
{
	return 0;
}

static slide_list_title_t post_list_title(slide_list_t *p)
{
	return;
}

static slide_list_display_t post_list_display(slide_list_t *p, int i)
{
	return 0;
}

static slide_list_handler_t post_list_handler(slide_list_t *p, int ch)
{
	return 0;
}

static int post_list(int bid, post_list_type_e type, post_id_t pid,
		slide_list_base_e base, const char *owner, const char *keyword)
{
	return 0;
}

int post_list_normal_range(int bid, post_id_t pid, slide_list_base_e base)
{
	return post_list(bid, POST_LIST_NORMAL, pid, base, NULL, NULL);
}

int post_list_normal(int bid)
{
	return 0;
}
