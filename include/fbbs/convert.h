#ifndef FB_CONVERT_H
#define FB_CONVERT_H

#include <iconv.h>

enum {
	CONVERT_BUFSIZE = 1024
};

typedef struct convert_t {
	iconv_t cd;
	char buf[CONVERT_BUFSIZE];
} convert_t;

typedef int (*convert_handler_t)(char *buf, size_t len, void *arg);

extern int convert_open(convert_t *cp, const char *to, const char *from);
extern void convert_reset(convert_t *cp);
extern int convert(convert_t *cp, const char *from, size_t len,
		char *buf, size_t size, convert_handler_t handler, void *arg);
extern int convert_close(convert_t *cp);

#endif // FB_CONVERT_H
