#ifndef FB_FBBS_H
#define FB_FBBS_H

#include "fbbs/cfg.h"
#include "fbbs/convert.h"
#include "fbbs/dbi.h"
#include "fbbs/pool.h"
#include "fbbs/schema.h"

typedef struct bbs_env_t {
	config_t *c;
	db_conn_t *d;
	pool_t *p;
	convert_t *u2g;
	convert_t *g2u;
} bbs_env_t;

extern bbs_env_t env;

typedef struct bbs_session_t {
	user_id_t uid;
} bbs_session_t;

extern bbs_session_t session;

extern bool passwd_match(const char *pw_crypted, const char *pw_try);
extern bool passwd_check(const char *name, const char *pw_try);
extern int passwd_set(const char *name, const char *pw);

#endif // FB_FBBS_H
