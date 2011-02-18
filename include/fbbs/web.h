#ifndef FB_WEB_H
#define FB_WEB_H

#include "fbbs/cfg.h"
#include "fbbs/dbi.h"
#include "fbbs/pool.h"
#include <fcgi_stdio.h>

#define CHARSET "utf-8"

enum {
	MAX_PARAMETERS = 32,
	MAX_CONTENT_LENGTH = 1 * 1024 * 1024,
};

enum {
	PARSE_NOSIG = 0x1,
	PARSE_NOQUOTEIMG = 0x2,
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
} http_req_t;

typedef struct web_ctx_t {
	config_t *c;
	db_conn_t *d;
	pool_t *p;
	http_req_t *r;
} web_ctx_t;

extern http_req_t *get_request(pool_t *p);
extern const char *get_param(http_req_t *r, const char *name);
extern int parse_post_data(http_req_t *r);

extern void html_header(void);
extern void xml_header(const char *xslfile);

extern void print_session(void);

extern void xml_print(const char *s);
extern int xml_print_post(const char *file, int option);

#endif // FB_WEB_H
