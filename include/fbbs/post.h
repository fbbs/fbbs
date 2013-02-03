#ifndef FB_POST_H
#define FB_POST_H

#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/dbi.h"
#include "fbbs/record.h"

#define ANONYMOUS_ACCOUNT "Anonymous"
//% #define ANONYMOUS_NICK    "我是匿名天使"
#define ANONYMOUS_NICK    "\xce\xd2\xca\xc7\xc4\xe4\xc3\xfb\xcc\xec\xca\xb9"
//% #define ANONYMOUS_SOURCE  "匿名天使的家"
#define ANONYMOUS_SOURCE  "\xc4\xe4\xc3\xfb\xcc\xec\xca\xb9\xb5\xc4\xbc\xd2"

typedef int64_t post_id_t;
#define PRIdPID  PRId64
#define DBIdPID  "l"
#define POST_ID_MAX  INT64_MAX
#define db_get_post_id(res, row, col)  db_get_bigint(res, row, col)
enum {
	PID_BUF_LEN = (sizeof(post_id_t) * 8 + 4) / 5 + 1,
};

#define POST_LIST_FIELDS  \
	"id,reid,tid,fake_id,board,owner,uname,stamp,digest,marked,water," \
	"locked,imported,replies,comments,score,title"

#define POST_LIST_FIELDS_FULL  POST_LIST_FIELDS ",content"

typedef enum {
	POST_FLAG_DIGEST = 0x1,
	POST_FLAG_MARKED = 0x2,
	POST_FLAG_LOCKED = 0x4,
	POST_FLAG_IMPORT = 0x8,
	POST_FLAG_STICKY = 0x10,
	POST_FLAG_WATER = 0x20,
	POST_FLAG_DELETED = 0x40,
	POST_FLAG_JUNK = 0x80,
	POST_FLAG_ATTACHMENT = 0x100,
	POST_FLAG_ARCHIVE = 0x200,
} post_flag_e;

enum {
	POST_TITLE_CCHARS = 24,
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
	POST_LIST_TOPIC,
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
	int fake_id;
	user_id_t uid;
	post_id_t id;
	post_id_t reid;
	post_id_t tid;
	fb_time_t stamp;
	fb_time_t estamp;
	int bid;
	char owner[IDLEN + 1];
	char ename[IDLEN + 1];
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
	const char *uname;
	const char *nick;
	const struct userec *user;
	const board_t *board;
	const char *title;
	const char *content;
	const char *gbk_file;
	int sig;
	const char *ip;
	post_id_t reid;
	post_id_t tid;
	bool locked;
	bool marked;
	bool anony;
	convert_t *cp;
} post_request_t;

enum {
	POST_LIST_KEYWORD_LEN = 19,
};

typedef struct {
	post_list_type_e type;
	int bid;
	int flag;
	user_id_t uid;
	post_id_t min;
	post_id_t max;
	post_id_t tid;
	int fake_id_min;
	int fake_id_max;
	bool archive;
	UTF8_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
} post_filter_t;

// 24 bytes
typedef struct {
	post_id_t id;
	uint32_t reid_delta;
	uint32_t tid_delta;
	user_id_t uid;
	int flag;
} post_index_board_t;

typedef struct {
	post_id_t id;
	uint32_t reid_delta;
	uint32_t tid_delta;
	user_id_t uid;
	int flag;
	fb_time_t estamp;
	char ename[16];
} post_index_trash_t;

typedef enum {
	POST_INDEX_TRASH = 1,
	POST_INDEX_JUNK = 0,
} post_index_trash_e;

// 128 bytes
typedef struct {
	post_id_t id;
	uint32_t reid_delta;
	uint32_t tid_delta;
	uint32_t stamp;
	user_id_t uid;
	int flag;
	int bid;
	uint16_t replies;
	uint16_t comments;
	uint16_t score;
	char owner[IDLEN + 1];
	char utf8_title[77];
} post_index_t;

typedef struct {
	post_id_t base;
	record_perm_e rdonly;
	mmap_t map;
} post_index_record_t;

extern int post_index_cmp(const void *p1, const void *p2);
extern int post_index_board_open_file(const char *file, record_perm_e rdonly, record_t *rec);
extern int post_index_board_open(int bid, record_perm_e rdonly, record_t *rec);
extern int post_index_board_open_sticky(int bid, record_perm_e rdonly, record_t *rec);
extern int post_index_board_read(record_t *rec, int base, post_index_record_t *pir, post_info_t *buf, int size);

extern int post_index_trash_cmp(const void *p1, const void *p2);
extern int post_index_trash_open(int bid, post_index_trash_e trash, record_t *rec);

extern void post_index_record_open(post_index_record_t *rec);
extern int post_index_record_read(post_index_record_t *rec, post_id_t id, post_index_t *buf);
extern int post_index_record_update(post_index_record_t *rec, post_id_t id, post_index_t *buf);
extern void post_index_record_close(post_index_record_t *rec);

extern const char *pid_to_base32(post_id_t pid, char *s, size_t size);
extern post_id_t base32_to_pid(const char *s);

extern post_id_t publish_post(const post_request_t *pr);

extern void quote_string(const char *str, size_t size, const char *output,
		int mode, bool mail, size_t (*filter)(const char *, size_t, FILE *));
extern void quote_file_(const char *orig, const char *output, int mode,
		bool mail, size_t (*filter)(const char *, size_t, FILE *));

extern int set_post_flag(post_filter_t *filter, const char *field, bool set, bool toggle);
extern bool sticky_post_unchecked(int bid, post_id_t pid, bool sticky);

extern void res_to_post_info(db_res_t *r, int i, bool archive, post_info_t *p);
void set_post_flag_local(post_info_t *ip, post_flag_e flag, bool set);
extern int load_sticky_posts(int bid, post_info_t **posts);
extern bool is_deleted(post_list_type_e type);
extern post_list_type_e post_list_type(const post_info_t *ip);
extern const char *post_table_name(const post_filter_t *filter);
extern const char *post_table_index(const post_filter_t *filter);
extern void build_post_filter(query_t *q, const post_filter_t *f, const bool *asc);
extern query_t *build_post_query(const post_filter_t *filter, bool asc, int limit);
extern void res_to_post_info_full(db_res_t *res, int row, bool archive, post_info_full_t *p);
extern void free_post_info_full(post_info_full_t *p);

extern int dump_content_to_gbk_file(const char *utf8_str, size_t length,
		char *file, size_t size);
extern char *convert_file_to_utf8_content(const char *file);

extern bool set_last_post_id(int bid, post_id_t pid);
extern post_id_t get_last_post_id(int bid);

extern int delete_posts(post_filter_t *filter, bool junk, bool bm_visible, bool force);
extern int undelete_posts(post_filter_t *filter);

extern db_res_t *query_post_by_pid(const post_filter_t *filter, const char *fields);
extern bool alter_title(const post_info_t *ip, const char *title);
extern bool alter_content(const post_info_t *ip, const char *content);

extern int get_post_mark(const post_info_t *p);

extern int get_last_fake_pid(int bid);
extern int incr_last_fake_pid(int bid, int delta);

#endif // FB_POST_H
