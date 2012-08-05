#ifndef FB_POOL_H
#define FB_POOL_H

#include "fbbs/util.h"

typedef struct pool_t pool_t;

extern pool_t *pool_create(size_t size);
extern void pool_clear(pool_t *p);
extern void pool_destroy(pool_t *p);
extern void *pool_alloc(pool_t *p, size_t size);
extern char *pool_strdup(pool_t *p, const char *str, size_t size);

#endif // FB_POOL_H
