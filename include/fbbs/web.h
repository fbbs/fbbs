#ifndef FB_WEB_H
#define FB_WEB_H

#include "fbbs/cfg.h"
#include "fbbs/convert.h"
#include "fbbs/dbi.h"
#include "fbbs/pool.h"
#include <fcgi_stdio.h>

enum {
	MAX_PARAMETERS = 32,
	MAX_CONTENT_LENGTH = 1 * 1024 * 1024,

	PARSE_NOSIG = 0x1,
	PARSE_NOQUOTEIMG = 0x2,
	PARSE_NOSIGIMG = 0x4,

	REQUEST_API = 0x1,
	REQUEST_PARSED = 0x2,
	REQUEST_MOBILE = 0x4,
	REQUEST_UTF8 = 0x8,
};

typedef struct pair_t {
	char *key;
	char *val;
} pair_t;

typedef struct http_req_t {
	pool_t *p;
	char *from;
	pair_t params[MAX_PARAMETERS];
	int count;
	int flag;
} http_req_t;

typedef struct web_ctx_t {
	config_t *c;
	pool_t *p;
	http_req_t *r;
	convert_t *u2g;
	convert_t *g2u;
} web_ctx_t;

extern http_req_t *get_request(pool_t *p);
extern const char *get_param(http_req_t *r, const char *name);
extern int parse_post_data(http_req_t *r);

extern void html_header(void);
extern void xml_header(const char *xslfile);

extern void xml_print(const char *s);
extern int xml_print_post(const char *file, int option);

#endif // FB_WEB_H
