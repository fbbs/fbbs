#ifndef FB_POOL_H
#define FB_POOL_H

#include "fbbs/util.h"

enum {
	DEFAULT_POOL_SIZE = 16 * 1024,
};

typedef struct pool_large_t {
	void *ptr;
	struct pool_large_t *next;
} pool_large_t;

typedef struct pool_block_t {
	uchar_t *last;
	uchar_t *end;
	struct pool_block_t *next;
} pool_block_t;

typedef struct pool_t {
	struct pool_block_t *head;
	pool_large_t *large;
} pool_t;

extern pool_t *pool_create(size_t size);
extern void pool_clear(pool_t *p);
extern void pool_destroy(pool_t *p);
extern void *pool_alloc(pool_t *p, size_t size);

#endif // FB_POOL_H
