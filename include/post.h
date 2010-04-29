#ifndef FB_POST_H
#define FB_POST_H

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
} post_request_t;

extern int do_post_article(const post_request_t *pr);

#endif // FB_POST_H

