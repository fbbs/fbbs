#ifndef FB_CONVERT_H
#define FB_CONVERT_H

#include <iconv.h>
#include <stdio.h>

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

extern int convert_to_file(convert_t *cp, const char *from, size_t len, FILE *fp);

#define convert_u2g(orig, buf)  convert(env.u2g, orig, 0, buf, sizeof(buf), NULL, NULL)
#define convert_g2u(orig, buf)  convert(env.g2u, orig, 0, buf, sizeof(buf), NULL, NULL)

#endif // FB_CONVERT_H
