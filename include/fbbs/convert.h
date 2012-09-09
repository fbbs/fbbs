#ifndef FB_CONVERT_H
#define FB_CONVERT_H

#include <stdio.h>

#define GBK_BUFFER(buf, cchars)  char gbk_##buf[cchars * 2 + 1]
#define UTF8_BUFFER(buf, cchars)  char utf8_##buf[cchars * 4 + 1]
#define GBK_UTF8_BUFFER(buf, cchars)  char gbk_##buf[cchars * 2 + 1], utf8_##buf[cchars * 4 + 1]

typedef struct convert_t convert_t;

extern convert_t *env_u2g;
extern convert_t *env_g2u;

typedef int (*convert_handler_t)(const char *buf, size_t len, void *arg);

extern int convert_open(convert_t *cp, const char *to, const char *from);
extern void convert_reset(convert_t *cp);
extern int convert(convert_t *cp, const char *from, size_t len,
		char *buf, size_t size, convert_handler_t handler, void *arg);
extern int convert_close(convert_t *cp);

extern int convert_to_file(convert_t *cp, const char *from, size_t len, FILE *fp);

#define CONVERT_ALL  ((size_t) -1)
#define convert_u2g(orig, buf) \
	convert(env_u2g, orig, CONVERT_ALL, buf, sizeof(buf), NULL, NULL)
#define convert_g2u(orig, buf) \
	convert(env_g2u, orig, CONVERT_ALL, buf, sizeof(buf), NULL, NULL)

#endif // FB_CONVERT_H
