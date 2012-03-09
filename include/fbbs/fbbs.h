#ifndef FB_FBBS_H
#define FB_FBBS_H

#include "fbbs/cfg.h"
#include "fbbs/convert.h"
#include "fbbs/dbi.h"
#include "fbbs/mdbi.h"
#include "fbbs/pool.h"
#include "fbbs/schema.h"
#include "fbbs/session.h"

#ifdef ENABLE_BANK
# define TO_CENTS(y)  (y * 100)
# define TO_YUAN(c)  (c / 100.0L)
# define TO_YUAN_INT(c)  ((int)(c / 100.0L))
# define PERCENT_RANK(r)  (((int)(r * 1000)) / 10.0)
#endif // ENABLE_BANK

typedef struct bbs_env_t {
	config_t *c;
	db_conn_t *d;
	mdb_conn_t *m;
	pool_t *p;
	convert_t *u2g;
	convert_t *g2u;
} bbs_env_t;

extern bbs_env_t env;
extern bbs_session_t session;

extern bool passwd_match(const char *pw_crypted, const char *pw_try);
extern bool passwd_check(const char *name, const char *pw_try);
extern int passwd_set(const char *name, const char *pw);

#endif // FB_FBBS_H
