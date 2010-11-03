#ifndef FB_WEB_H
#define FB_WEB_H

#include <fcgi_stdio.h>

#define CHARSET "utf-8"

enum {
	MAX_PARAMETERS = 32,
	MAX_CONTENT_LENGTH = 1 * 1024 * 1024,
};

typedef struct pair_t {
	char *key;
	char *val;
} pair_t;

typedef struct http_req_t {
	pool_t *p;
	pair_t params[MAX_PARAMETERS];
	int count;
} http_req_t;

extern http_req_t *get_request(pool_t *p);
extern const char *get_param(http_req_t *r, const char *name);
extern int parse_post_data(http_req_t *r);

extern void html_header(void);
extern void xml_header(const char *xslfile);

#endif // FB_WEB_H
