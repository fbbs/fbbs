#ifndef FB_CACHE_H
#define FB_CACHE_H

#include "fbbs/bbs.h"
#include "fbbs/hash.h"
#include "fbbs/mmap.h"
#include "fbbs/pass.h"
#include "fbbs/user.h"

#define CACHE_SERVER BBSHOME"/tmp/cache-server"
#define CACHE_CLIENT BBSHOME"/tmp/cache-client"

enum {
	PASSWORD_QUERY = 0,
};

typedef struct ucache_t {
	user_t *begin;
	user_t *end;
	mmap_t m;
	hash_t name_hash;
	hash_t uid_hash;
} ucache_t;

typedef struct password_query_t {
	int type;
	char username[EXT_ID_LEN];
	char passwd[MAX_PASSWORD_LENGTH];
} password_query_t;

typedef struct password_result_t {
	int type;
	char match;
} password_result_t;

#endif // FB_CACHE_H
