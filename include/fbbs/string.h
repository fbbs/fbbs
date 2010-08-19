#ifndef FB_STRING_H
#define FB_STRING_H

#include <stddef.h>
#include <stdbool.h>

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

static inline bool isprint2(char c)
{
	return (((c & 0x80) && c != 0xFF) || isprint(c));
}

#endif // FB_STRING_H
