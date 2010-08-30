#ifndef FB_UTIL_H
#define FB_UTIL_H

#include <stddef.h>

extern int read_urandom(void *buf, size_t size);
extern int urandom_pos_int(void);

#endif // FB_UTIL_H
