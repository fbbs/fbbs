#ifndef FB_CONVERT_H
#define FB_CONVERT_H

#include <stdbool.h>
#include <stdio.h>

#define GBK_BUFFER(buf, cchars)  char gbk_##buf[cchars * 2 + 1]
#define UTF8_BUFFER(buf, cchars)  char utf8_##buf[cchars * 4 + 1]
#define GBK_UTF8_BUFFER(buf, cchars)  char gbk_##buf[cchars * 2 + 1], utf8_##buf[cchars * 4 + 1]

typedef enum {
	CONVERT_NONE = 0,
	CONVERT_U2G = 1,
	CONVERT_G2U = 2,
} convert_type_e;

typedef int (*convert_handler_t)(const char *buf, size_t len, void *arg);

extern bool convert_open(convert_type_e type);
extern int convert(convert_type_e type, const char *from, size_t len, char *buf, size_t size, convert_handler_t handler, void *arg);
extern void convert_close(void);
extern int convert_to_file(convert_type_e type, const char *from, size_t len, FILE *fp);

#define CONVERT_ALL  ((size_t) -1)
#define convert_u2g(orig, buf) \
	convert(CONVERT_U2G, orig, CONVERT_ALL, buf, sizeof(buf), NULL, NULL)
#define convert_g2u(orig, buf) \
	convert(CONVERT_G2U, orig, CONVERT_ALL, buf, sizeof(buf), NULL, NULL)

#endif // FB_CONVERT_H
