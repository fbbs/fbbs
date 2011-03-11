#ifndef FB_POST_H
#define FB_POST_H

#include "fbbs/convert.h"

#define ANONYMOUS_ACCOUNT "Anonymous"
#define ANONYMOUS_NICK    "我是匿名天使"
#define ANONYMOUS_SOURCE  "匿名天使的家"

typedef struct {
	bool autopost;
	bool crosspost;
	const char *userid;
	const char *nick;
	const struct userec *user;
	const struct boardheader *bp;
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

