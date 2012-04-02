#ifndef FB_POST_H
#define FB_POST_H

#include "fbbs/board.h"
#include "fbbs/convert.h"

#define ANONYMOUS_ACCOUNT "Anonymous"
#define ANONYMOUS_NICK    "我是匿名天使"
#define ANONYMOUS_SOURCE  "匿名天使的家"

typedef int64_t post_id_t;
#define PRIdPID  PRId64
#define DBIdPID  "l"
#define db_get_post_id(res, row, col)  db_get_bigint(res, row, col)

enum {
	POST_FLAG_DIGEST = 0x1,
	POST_FLAG_MARKED = 0x2,
	POST_FLAG_LOCKED = 0x4,
	POST_FLAG_IMPORT = 0x8,
	POST_FLAG_STICKY = 0x10,

	POST_TITLE_CCHARS = 33,
};

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
	const struct fileheader *o_fp;
	bool noreply;
	bool mmark;
	bool anony;
	convert_t *cp;
} post_request_t;

extern unsigned int do_post_article(const post_request_t *pr);

#endif // FB_POST_H

