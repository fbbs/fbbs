#ifndef FB_STRING_H
#define FB_STRING_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "fbbs/pool.h"
#include "fbbs/util.h"

#define streq(a, b)          (!strcmp(a, b))
#define strneq(a, b, n)      (!strncmp(a, b, n))
#define strcaseeq(a, b)      (!strcasecmp(a, b))
#define strncaseeq(a, b, n)  (!strncasecmp(a, b, n))

enum {
	PSTRING_DEFAULT_LEN = 7,
};

extern char *strtolower(char *dst, const char *src);
extern char *strtoupper(char *dst, const char *src);
extern char *strcasestr_gbk(const char *haystack, const char *needle);
extern char *ansi_filter(char *dst, const char *src);
extern int ellipsis(char *str, int len);
extern char *rtrim(char *str);
extern char *trim(char *str);
extern size_t strlcpy(char *dst, const char *src, size_t siz);
extern void strtourl(char *url, const char *str);
extern void strappend(char **dst, size_t *size, const char *src);
extern void printable_filter(char *str);
extern int valid_gbk(unsigned char *str, int len, int replace);
extern const char *check_gbk(const char *title);
extern int validate_utf8_input(const char *str, size_t max_chinese_chars);

static inline bool isprint2(int ch)
{
	unsigned char c = ch;
	return (((c & 0x80) && c != 0xFF) || isprint(c));
}

typedef struct pstring_t pstring_t;

extern pstring_t *pstring_new(pool_t *p);
extern pstring_t *pstring_sized_new(pool_t *p, uint_t size);
extern pstring_t *pstring_append_c(pool_t *p, pstring_t *s, int c);
extern pstring_t *pstring_append_printf(pool_t *p, pstring_t *s, const char *format, ...);
extern pstring_t *pstring_append_space(pool_t *p, pstring_t *s);
extern const char *pstring(const pstring_t *s);

#endif // FB_STRING_H
