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

#define BOARD_POST_COUNT_KEY "board_post_count"

typedef enum {
	POST_FLAG_DIGEST = 1,
	POST_FLAG_MARKED = 1 << 1,
	POST_FLAG_LOCKED = 1 << 2,
	POST_FLAG_IMPORT = 1 << 3,
	POST_FLAG_STICKY = 1 << 4,
	POST_FLAG_WATER = 1 << 5,
	POST_FLAG_DELETED = 1 << 6,
	POST_FLAG_JUNK = 1 << 7,
	POST_FLAG_ATTACHMENT = 1 << 8,
	POST_FLAG_ARCHIVE = 1 << 9,
} post_flag_e;

enum {
	POST_TITLE_CCHARS = 23,
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
	int flag;
	user_id_t user_id;
	post_id_t id;
	post_id_t reply_id;
	post_id_t thread_id;
	fb_time_t stamp;
	fb_time_t delete_stamp;
	int board_id;
	char user_name[IDLEN + 1];
	char eraser_name[IDLEN + 1];
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

typedef enum {
	POST_TRASH = 1,
	POST_JUNK = 0,
} post_trash_e;

typedef struct {
	post_id_t id;
	post_id_t reply_id;
	post_id_t thread_id;
	user_id_t user_id;
	int board_id;
	int flag;
	char user_name[IDLEN + 1];
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
} post_record_t;

typedef struct {
	post_record_t basic;
	bool junk;
	bool bm_visible;
	user_id_t eraser_id;
	fb_time_t stamp;
	char eraser_name[IDLEN + 1];
} post_record_extended_t;

typedef struct {
	post_id_t tid;
	uint_t count;
	fb_time_t last;
	int bid;
	UTF8_BUFFER(title, POST_TITLE_CCHARS);
	char owner[16];
	char bname[BOARD_NAME_LEN + 1];
} topic_stat_t;

extern int post_record_cmp(const void *p1, const void *p2);
extern int post_record_open(int bid, record_perm_e rdonly, record_t *rec);
extern int post_record_open_sticky(int bid, record_perm_e rdonly, record_t *rec);
extern int post_record_open_trash(int bid, post_trash_e trash, record_t *rec);

extern void quote_string(const char *str, size_t size, FILE *output, post_quote_e mode, bool mail, bool utf8, size_t (*filter)(const char *, size_t, FILE *));
extern void quote_file_(const char *orig, const char *output, post_quote_e mode, bool mail, bool utf8, size_t (*filter)(const char *, size_t, FILE *));

extern bool is_deleted(post_list_type_e type);

extern int post_dump_gbk_file(const char *utf8_str, size_t length, char *file, size_t size);
extern char *post_convert_to_utf8(const char *file);

extern bool set_last_post_time(int bid, fb_time_t stamp);
extern fb_time_t get_last_post_time(int bid);

extern int post_mark_raw(fb_time_t stamp, int flag);
extern int post_mark(const post_info_t *p);

extern fb_time_t post_stamp_from_id(post_id_t id);
extern post_id_t post_id_from_stamp(fb_time_t stamp);

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

extern post_id_t post_new(const post_request_t *pr);

typedef struct { // @frontend
	post_filter_t *filter;
	bool junk;
	bool bm_visible;
	bool force;
	user_id_t user_id;
	const char *user_name;
} backend_request_post_delete_t;

typedef struct { // @backend
	int deleted;
} backend_response_post_delete_t;

extern int post_delete(const post_filter_t *filter, bool junk, bool bm_visible, bool force);

typedef struct { // @frontend
	post_filter_t *filter;
	bool bm_visible;
} backend_request_post_undelete_t;

typedef struct { // @backend
	int undeleted;
} backend_response_post_undelete_t;

extern int post_undelete(const post_filter_t *filter, bool bm_visible);

typedef struct { // @frontend
	post_filter_t *filter;
	post_flag_e flag;
	bool set;
	bool toggle;
} backend_request_post_set_flag_t;

typedef struct { // @backend
	int affected;
} backend_response_post_set_flag_t;

extern int post_set_flag(const post_filter_t *filter, post_flag_e flag, bool set, bool toggle);

typedef struct { // @frontend
	post_id_t post_id;
	const char *title;
} backend_request_post_alter_title_t;

typedef struct { // @backend
	bool ok;
} backend_response_post_alter_title_t;

extern bool post_alter_title(post_id_t post_id, const char *title);

extern void post_record_invalidity_change(int bid, int delta);
extern int post_record_read(record_t *rec, int base, post_info_t *buf, int size, post_list_type_e type);
extern void post_record_to_info(const post_record_t *pr, post_info_t *pi, int count);
extern bool post_match_filter(const post_record_t *pr, const post_filter_t *filter, int offset);

extern bool post_update_sticky_record(int board_id);

extern int post_sticky_count(int board_id);

extern char *post_content_get(post_id_t post_id);
extern bool post_content_set(post_id_t post_id, const char *str);

#endif // FB_POST_H
