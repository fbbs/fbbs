#ifndef FB_POST_H
#define FB_POST_H

#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/dbi.h"

#define ANONYMOUS_ACCOUNT "Anonymous"
#define ANONYMOUS_NICK    "我是匿名天使"
#define ANONYMOUS_SOURCE  "匿名天使的家"

typedef int64_t post_id_t;
#define PRIdPID  PRId64
#define DBIdPID  "l"
#define POST_ID_MAX  INT64_MAX
#define db_get_post_id(res, row, col)  db_get_bigint(res, row, col)

#define POST_LIST_FIELDS  \
	"id, reid, tid, owner, uname, stamp, digest, marked," \
	" locked, imported, replies, comments, score, title"

#define POST_LIST_FIELDS_FULL  POST_LIST_FIELDS ", content"

typedef enum {
	POST_FLAG_DIGEST = 0x1,
	POST_FLAG_MARKED = 0x2,
	POST_FLAG_LOCKED = 0x4,
	POST_FLAG_IMPORT = 0x8,
	POST_FLAG_STICKY = 0x10,
} post_flag_e;

enum {
	POST_TITLE_CCHARS = 33,
};

enum {
	QUOTE_NOTHING = 'N',
	QUOTE_AUTO = 'R',
	QUOTE_LONG = 'Y',
	QUOTE_SOURCE = 'S',
	QUOTE_ALL = 'A',
};

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
	post_info_t p;
	db_res_t *res;
	const char *content;
	size_t length;
} post_info_full_t;

typedef struct {
	bool autopost;
	bool crosspost;
	const char *userid;
	const char *nick;
	const struct userec *user;
	const board_t *board;
	const char *title;
	const char *content;
	int sig;
	const char *ip;
	const post_info_t *o_fp;
	bool noreply;
	bool mmark;
	bool anony;
	convert_t *cp;
} post_request_t;

enum {
	POST_LIST_KEYWORD_LEN = 19,
};

typedef struct {
	int bid;
	post_list_type_e type;
	post_id_t pid;
	user_id_t uid;
	UTF8_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
} post_list_filter_t;

extern post_id_t publish_post(const post_request_t *pr);

extern void quote_string(const char *str, size_t size, const char *output,
		int mode, bool mail, size_t (*filter)(const char *, size_t, FILE *));
extern void quote_file_(const char *orig, const char *output, int mode,
		bool mail, size_t (*filter)(const char *, size_t, FILE *));

extern bool set_post_flag_unchecked(int bid, post_id_t pid, const char *field, bool on);
extern bool sticky_post_unchecked(int bid, post_id_t pid, bool sticky);

extern void res_to_post_info(db_res_t *r, int i, post_info_t *p);
void set_post_flag(post_info_t *ip, post_flag_e flag, bool set);
extern int _load_sticky_posts(post_list_filter_t *filter, post_info_t **posts);
extern int build_post_query(char *query, size_t size, post_list_type_e type, bool asc, int limit);

extern void res_to_post_info_full(db_res_t *res, int row, post_info_full_t *p);
extern void free_post_info_full(post_info_full_t *p);

extern int dump_content_to_gbk_file(const char *utf8_str, size_t length,
		char *file, size_t size);
#endif // FB_POST_H
