#ifndef FB_FBBS_H
#define FB_FBBS_H

#include "fbbs/cfg.h"
#include "fbbs/dbi.h"
#include "fbbs/pool.h"

typedef struct bbs_env_t {
	config_t *c;
	db_conn_t *d;
	pool_t *p;
} bbs_env_t;

extern bbs_env_t env;

#endif // FB_FBBS_H
