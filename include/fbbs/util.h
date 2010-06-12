#ifndef FB_UTIL_H
#define FB_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

typedef int64_t fb_time_t;
#define PRIdFBT PRId64

typedef uint32_t uint_t;
typedef uint16_t ushort_t;
typedef uint8_t uchar_t;

typedef uint32_t seq_t;
typedef uint32_t varchar_t;

typedef void (*sighandler_t)(int);

extern void start_daemon(void);

extern sighandler_t fb_signal(int signum, sighandler_t handler);

extern void *fb_malloc(size_t size);

#endif // FB_UTIL_H
