#ifndef FB_CACHE_H
#define FB_CACHE_H

#include "fbbs/hash.h"
#include "fbbs/mmap.h"
#include "fbbs/user.h"

typedef struct ucache_t {
	user_t *begin;
	user_t *end;
	mmap_t m;
	hash_t name_hash;
	hash_t uid_hash;
} ucache_t;

#endif // FB_CACHE_H
