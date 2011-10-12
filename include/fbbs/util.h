#ifndef FB_UTIL_H
#define FB_UTIL_H

#include <endian.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#ifndef be64toh
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  include <byteswap.h>
#  define be64toh(x) bswap_64(x)
# else
#  define be64toh(x) (x)
# endif
#endif

#ifndef htobe64
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  include <byteswap.h>
#  define htobe64(x) bswap_64(x)
# else
#  define htobe64(x) (x)
# endif
#endif

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

#define FB_ULONG_MAX  UINT64_MAX
#define FB_UINT_MAX   UINT32_MAX
#define FB_USHORT_MAX UINT16_MAX
#define FB_UCHAR_MAX  UINT8_MAX

typedef uint64_t ulong_t;
typedef uint32_t uint_t;
typedef uint16_t ushort_t;
typedef uint8_t uchar_t;

typedef void (*sighandler_t)(int);

extern void start_daemon(void);

extern sighandler_t fb_signal(int signum, sighandler_t handler);

extern void *fb_malloc(size_t size);

extern int read_urandom(void *buf, size_t size);
extern int urandom_pos_int(void);

#endif // FB_UTIL_H
