#ifndef FB_POST_H
#define FB_POST_H

#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/record.h"

#define ANONYMOUS_ACCOUNT "Anonymous"
//% #define ANONYMOUS_NICK    "我是匿名天使"
#define ANONYMOUS_NICK    "\xce\xd2\xca\xc7\xc4\xe4\xc3\xfb\xcc\xec\xca\xb9"
//% #define ANONYMOUS_SOURCE  "匿名天使的家"
#define ANONYMOUS_SOURCE  "\xc4\xe4\xc3\xfb\xcc\xec\xca\xb9\xb5\xc4\xbc\xd2"

#define POST_REPLIES_FILE  "replies"

typedef int64_t post_id_t;
#define PRIdPID  PRId64
#define DBIdPID  "l"
#define db_get_post_id(res, line, col)  db_get_bigint(res, line, col)
#define parcel_write_post_id(parcel, id)  parcel_write_varint64(parcel, id)
#define parcel_read_post_id(parcel)  parcel_read_varint64(parcel)

/** 文章ID序列 @mdb_string */
#define POST_ID_KEY  "post_id_seq"
#define BOARD_POST_COUNT_KEY "board_post_count"

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
	POST_CONTENT_CCHARS = 128 * 1024,
};

typedef enum {
	QUOTE_NOTHING = 'N',
	QUOTE_AUTO = 'R',
	QUOTE_LONG = 'Y',
	QUOTE_SOURCE = 'S',
	QUOTE_ALL = 'A',
	QUOTE_PACK = 'P',
	QUOTE_PACK_COMPACT = 'C',
} post_quote_e;

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
	POST_LIST_FORUM,
	POST_LIST_REPLIES,
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
	fb_time_t estamp;
	int bid;
	char owner[IDLEN + 1];
	char ename[IDLEN + 1];
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
} post_info_t;

typedef struct {
	const char *uname;
	const char *nick;
	const struct userec *user;
	const board_t *board;
	const char *title;
	const char *content;
	size_t length;
	const char *gbk_file;
	const char *ip;
	convert_t *cp;
	post_id_t reid;
	post_id_t tid;
	user_id_t uid_replied;
	int sig;
	bool locked;
	bool marked;
	bool anony;
	bool web;
	bool autopost;
	bool crosspost;
	bool hide_uid;
} post_request_t;

enum {
	POST_LIST_KEYWORD_LEN = 19,
};

typedef struct { // @frontend
	post_list_type_e type;
	int bid;
	int flag;
	user_id_t uid;
	post_id_t min;
	post_id_t max;
	post_id_t tid;
	int offset_min;
	int offset_max;
	bool archive;
	UTF8_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
} post_filter_t;

// 32 bytes
typedef struct {
	post_id_t id;
	uint32_t reid_delta;
	uint32_t tid_delta;
	user_id_t uid;
	int flag;
	fb_time_t stamp;
	fb_time_t cstamp;
} post_index_board_t;

typedef struct {
	post_id_t id;
	uint32_t reid_delta;
	uint32_t tid_delta;
	user_id_t uid;
	int flag;
	fb_time_t stamp;
	fb_time_t cstamp;
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
	fb_time_t stamp;
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
	record_t record;
} post_index_record_t;

typedef struct {
	post_id_t tid;
	uint_t count;
	fb_time_t last;
	int bid;
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
	char owner[16];
	char bname[BOARD_NAME_LEN + 1];
} topic_stat_t;

extern int post_index_cmp(const void *p1, const void *p2);
extern int post_index_board_open_file(const char *file, record_perm_e rdonly, record_t *rec);
extern int post_index_board_open(int bid, record_perm_e rdonly, record_t *rec);
extern int post_index_board_open_sticky(int bid, record_perm_e rdonly, record_t *rec);
extern int post_index_board_to_info(post_index_record_t *pir, const post_index_board_t *pib, post_info_t *pi, int count);
extern int post_index_board_read(record_t *rec, int base, post_index_record_t *pir, post_info_t *buf, int size, post_list_type_e type);
extern int post_index_board_delete(const post_filter_t *filter, bool junk, bool bm_visible, bool force);
extern int post_index_board_undelete(const post_filter_t *filter, bool bm_visible);
extern bool match_filter(const post_index_board_t *pib, post_index_record_t *pir, const post_filter_t *filter, int offset);

extern int post_index_trash_cmp(const void *p1, const void *p2);
extern int post_index_trash_open(int bid, post_index_trash_e trash, record_t *rec);

typedef int (*post_index_record_callback_t)(post_index_t *pi, void *args);

extern void post_index_record_open(post_index_record_t *pir);
extern int post_index_record_read(post_index_record_t *pir, post_id_t id, post_index_t *pi);
extern int post_index_record_update(post_index_record_t *pir, const post_index_t *pi);
extern int post_index_record_for_recent(post_index_record_callback_t cb, void *args);
extern void post_index_record_close(post_index_record_t *pir);
extern void post_index_record_get_title(post_index_record_t *pir, post_id_t id, char *buf, size_t size);
extern int post_index_record_lock(post_index_record_t *pir, record_lock_e lock, post_id_t id);

enum {
	POST_CONTENT_BUFLEN = 4096,
};

extern char *post_content_read(post_id_t id, char *buf, size_t size);
extern int post_content_write(post_id_t id, const char *str, size_t size);

typedef struct {
	post_id_t base;
	char *result;
	size_t size;
	int fd;
	char buf[POST_CONTENT_BUFLEN];
} post_content_record_t;

extern void post_content_record_open(post_content_record_t *pcr);
extern char *post_content_record_read(post_content_record_t *pcr, post_id_t id);
extern void post_content_record_close(post_content_record_t *pcr);

extern int post_remove_sticky(int bid, post_id_t id);
extern int post_add_sticky(int bid, const post_info_t *pi);
extern bool reorder_sticky_posts(int bid, post_id_t pid);

extern post_id_t publish_post(const post_request_t *pr);

extern void quote_string(const char *str, size_t size, FILE *output, post_quote_e mode, bool mail, bool utf8, size_t (*filter)(const char *, size_t, FILE *));
extern void quote_file_(const char *orig, const char *output, post_quote_e mode, bool mail, bool utf8, size_t (*filter)(const char *, size_t, FILE *));

extern int set_post_flag(record_t *rec, post_index_record_t *pir, post_filter_t *filter, post_flag_e flag, bool set, bool toggle);
extern int set_post_flag_one(record_t *rec, post_index_board_t *pib, int offset, post_flag_e flag, bool set, bool toggle);

extern bool is_deleted(post_list_type_e type);

extern int dump_content_to_gbk_file(const char *utf8_str, size_t length, char *file, size_t size);
extern char *convert_file_to_utf8_content(const char *file);

extern bool set_last_post_time(int bid, fb_time_t stamp);
extern fb_time_t get_last_post_time(int bid);

extern bool alter_title(post_index_record_t *pir, const post_info_t *pi);

extern int get_post_mark_raw(fb_time_t stamp, int flag);
extern int get_post_mark(const post_info_t *p);

extern int get_board_post_count(int bid);

typedef struct { // @frontend
	post_id_t reid;
	post_id_t tid;
	const char *title;
	const char *uname;
	const char *content;
	user_id_t uid;
	user_id_t uid_replied;
	int bid;
	bool marked;
	bool locked;
	bool hide_uid;
} backend_request_post_new_t;

typedef struct { // @backend
	post_id_t id;
} backend_response_post_new_t;

typedef struct { // @frontend
	post_filter_t *filter;
	bool junk;
	bool bm_visible;
	bool force;
	const char *ename;
} backend_request_post_delete_t;

typedef struct { // @backend
	int deleted;
} backend_response_post_delete_t;

typedef struct { // @frontend
	post_filter_t *filter;
	bool bm_visible;
} backend_request_post_undelete_t;

typedef struct { // @backend
	int undeleted;
} backend_response_post_undelete_t;

#endif // FB_POST_H
