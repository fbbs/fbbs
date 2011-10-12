#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include "fbbs/string.h"

/**
 * Convert string to lower case.
 * @param dst result string.
 * @param src the string to convert.
 * @return the converted string.
 * @note 'dst' should have enough space to hold 'src'.
 */
char *strtolower(char *dst, const char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = tolower(*src++);
	*dst = '\0';
	return ret;	
}

/**
 * Convert string to upper case.
 * @param dst result string.
 * @param src the string to convert.
 * @return the converted string.
 * @note 'dst' should have enough space to hold 'src'.
 */
char *strtoupper(char *dst, const char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = toupper(*src++);
	*dst = '\0';
	return ret;	
}

// Compare string 's1' against 's2' and differences in case are ignored.
// No more than 'n' characters are compared.
// This function supports zh_CN.GBK.
int strncasecmp_gbk(const char *s1, const char *s2, int n)
{
	register int c1, c2, l = 0;

	while (*s1 && *s2 && l < n) {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
		if (c1 != c2)
			return (c1 - c2);
		++l;
		if (c1 & 0x80) {
			if (*s1++ == *s2++)
				++l;
			else
				return (*--s1 - *--s2);
		}
	}
	if (l == n)
		return 0;
	else
		return -1;
}

// Search string 'haystack' for a substring 'needle'
// and differences in case are ignored.
// This function supports zh_CN.GBK.
char *strcasestr_gbk(const char *haystack, const char *needle) {
	int i, nlength, hlength;

	if (haystack == NULL || needle == NULL)
		return NULL;
	nlength = strlen(needle);
	hlength = strlen(haystack);
	if (nlength > hlength)
		return NULL;
	if (hlength <= 0)
		return NULL;
	if (nlength <= 0)
		return (char *)haystack;
	for (i = 0; i <= (hlength - nlength); i++) {
		if (strncasecmp_gbk(haystack + i, needle, nlength) == 0)
			return (char *)(haystack + i);
		if (haystack[i] & 0x80)
			i++;
	}
	return NULL;
}

// Eliminate ANSI escape codes from 'src' and store it in 'dst'.
// 'src' and 'dst' can be the same.
char *ansi_filter(char *dst, const char *src)
{
	char *ret = dst;
	int flag = 0;

	if (dst == NULL || src == NULL)
		return NULL;
	for (; *src != '\0'; src++) {
		if (*src == '\033')
			flag = 1;
		else if (flag == 0)
			*dst++ = *src;
		else if (isalpha(*src))
			flag = 0;
	}
	*dst = '\0';
	return ret;
}

// Truncates 'str' to 'len' chars ended with ".."  or "...".
// Do nothing if 'str' is less than or equal to 'len' chars.
int ellipsis(char *str, int len)
{
	int i = 0, inGBK = 0;
	char *ptr = str;
	if (len < 0 || str == NULL)
		return 0;
	if (len < 3) {
		str[len] = '\0';
		return 1;
	}
	i = len - 3;
	while (*ptr != '\0' && i) {
		if (inGBK) {
			inGBK = 0;
		}
		else if (*ptr & 0x80)
			inGBK = 1;
		++ptr;
		--i;
	}
	i = 3;
	while(*ptr++ != '\0' && --i)
		;
	if(*ptr != '\0' && !i){
		str[len] = '\0';
		*--ptr = '.';
		*--ptr = '.';
		if(!inGBK && *--ptr & 0x80)
			*ptr = '.';
	}
	return 1;
}

// Removes trailing chars whose ASCII code is less than 0x20.
char *rtrim(char *str){
	if (str == NULL)
		return NULL;
	size_t len = strlen(str);
	unsigned char *ustr = (unsigned char *)str;
	unsigned char *ptr = ustr + len;
	while (*ptr <= 0x20 && ptr >= ustr) {
		--ptr;
	}
	*++ptr = '\0';
	return str;
}

// Removes both leading and trailing chars
// whose ASCII code is less than 0x20.
char *trim(char *str)
{
	if (str == NULL)
		return NULL;
	size_t len = strlen(str);
	unsigned char *ustr = (unsigned char *)str;
	unsigned char *right = ustr + len;
	while (*right <= 0x20 && right >= ustr) {
		--right;
	}
	*++right = '\0';
	unsigned char *left = ustr;
	while (*left <= 0x20 && *left != '\0')
		++left;
	if (left != ustr)
		memmove(ustr, left, right - left + 1);
	return str;
}

// OpenBSD: strlcpy.c,v 1.11
// Copy 'src' to string 'dst' of size 'siz'.
// At most siz-1 characters will be copied.
// Always NUL terminates (unless siz == 0).
// Returns strlen(src); if retval >= siz, truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	// Copy as many bytes as will fit
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	// Not enough room in dst, add NUL and traverse rest of src
	if (n == 0) {
		if (siz != 0)
			*d = '\0';  // NUL-terminate dst
		while (*s++)
			;
	}

	return(s - src - 1);  //count does not include NUL
}


void strtourl(char *url, const char *str)
{
	char c, h;

	while ((c = *str) != '\0') {
		if (isprint(c) && c != ' ' && c!= '%') {
			*url++ = c;
		} else {
			*url++ = '%';
			// hexadecimal representation
			h = c / 16;
			*url++ = h > 9 ? 'A' - 10 + h : '0' + h;
			h = c % 16;
			*url++ = h > 9 ? 'A' - 10 + h : '0' + h;
		}
		++str;
	}
	*url = '\0';
}

void strappend(char **dst, size_t *size, const char *src)
{
	size_t len = strlcpy(*dst, src, *size);
	if (len >= *size)
		len = *size;
	*dst += len;
	*size -= len;
}

/**
 * Remove non-printable characters.
 * @param[in, out] str The string to be filtered.
 */
void printable_filter(char *str)
{
	if (!str)
		return;

	char *dst = str;

	while (*str != '\0') {
		if (isprint2(*str))
			*dst++ = *str;
		str++;
	}
	*dst = '\0';
}

enum {
	GBK_FIRST = 0,
	GBK_SECOND,
};

int valid_gbk(unsigned char *str, int len, int replace)
{
	int count = 0;
	int state = GBK_FIRST;
	unsigned char *s;
	for (s = str; len > 0; --len, ++s) {
		switch (state) {
			case GBK_FIRST:
				if (*s == 0x80 || *s == 0xff) {
					*s = replace;
					break;
				} else if (*s & 0x80) {
					state = GBK_SECOND;
				}
				break;
			case GBK_SECOND:
				if (*s < 0x40 || *s == 0x7f) {
					*--s = replace;
					++len;
					++count;
				}
				state = GBK_FIRST;
				break;
		}
	}
	if (state == GBK_SECOND)
		*(s - 1) = replace;
	return count;
}

const char *check_gbk(const char *title)
{
	bool gbk = false;
	while (*title != '\0') {
		if (!gbk && *title & 0x80)
			gbk = true;
		else
			gbk = false;
		title++;
	}
	return (gbk ? title - 1 : title);
}

/** Check continuation byte (10xxxxxx) */
#define is_cont(c) ((c & 0xc0) == 0x80)

/**
 * Get next wchar from a UTF-8 multibyte sequence.
 * @param[in, out] str The UTF-8 multibyte string. On valid input, *str will
 * be incremented accordingly.
 * @param[in, out] leftp Bytes left in the string. Could be left NULL for a
 * NUL-terminated string. On valid input, *leftp will be decremented accordingly.
 * @return The wchar on success, 0 on string end, WEOF on invalid sequence.
 */
static wchar_t next_wchar(const char **str, size_t *leftp)
{
	unsigned char *s = (unsigned char *)*str;
	wchar_t wc;
	size_t incr = 0, left = leftp ? *leftp : 9;
	if (left && *s) {
		if (*s < 0x80) {
			wc = *s;
			incr = 1;
		} else if ((*s & 0xe0) == 0xc0 && left > 1 && is_cont(s[1])) {
			wc = ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
			incr = 2;
		} else if ((*s & 0xf0) == 0xe0 && left > 2
				&& is_cont(s[1]) && is_cont(s[2]) ) {
			wc = ((s[0] & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
			incr = 3;
		} else if ((*s & 0xf8) == 0xf0 && left > 3
				&& is_cont(s[1]) && is_cont(s[2]) && is_cont(s[3])) {
			wc = ((s[0] & 0x07) << 18) | ((s[1] & 0x3f) << 12) |
					((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
			incr = 4;
		}
	} else {
		return 0;
	}
	if (incr) {
		*str += incr;
		if (leftp)
			*leftp -= incr;
		return wc;
	}
	return WEOF;
}

/**
 * Validate a UTF-8 string, while checking its width and length.
 * @param[in] str The UTF-8 string to be validated.
 * @param[in] max_chinese_chars maximum column width and string length,
 * assuming each chinese character occupies 2 columns and max 4 bytes.
 * @return If the input a) is valid UTF-8 sequence b) does not exceed any of
 * the limits, and c) only contain characters with positive column width,
 * its total column width is returned, -1 otherwise.
 */
int validate_utf8_input(const char *str, size_t max_chinese_chars)
{
	const char *s = str;
	size_t width = 0;
	while (1) {
		wchar_t wc = next_wchar(&s, NULL);
		if (wc == WEOF)
			return -1;
		if (!wc) {
			if (s - str > max_chinese_chars * 4 || width > max_chinese_chars * 2)
				return -1;
			return width;
		}
		int w = wcwidth(wc);
		if (w <= 0)
			return -1;
		width += wcwidth(wc);
	}
}

static uint_t pstring_round_size(uint_t size)
{
	if (size == FB_UINT_MAX)
		return FB_UINT_MAX;
	uint_t s = PSTRING_DEFAULT_LEN + 1;
	while (s <= size)
		s *= 2;
	return s;
}

pstring_t *pstring_sized_new(pool_t *p, uint_t size)
{
	size = pstring_round_size(size);
	pstring_t *s = pool_alloc(p, sizeof(*s));
	s->str = pool_alloc(p, size);
	s->len = 0;
	s->size = size;
	return s;
}

pstring_t *pstring_new(pool_t *p)
{
	return pstring_sized_new(p, PSTRING_DEFAULT_LEN);
}

static pstring_t *_pstring_realloc(pool_t *p, pstring_t *s, uint_t size)
{
	s->size = size;
	char *str = s->str;
	s->str = pool_alloc(p, size);
	memcpy(s->str, str, s->len + 1);
	return s;
}

static pstring_t *pstring_realloc(pool_t *p, pstring_t *s)
{
	if (s->size == FB_UINT_MAX)
		return NULL;
	return _pstring_realloc(p, s, s->size * 2);
}

pstring_t *pstring_append_c(pool_t *p, pstring_t *s, int c)
{
	if (s->len == s->size - 1)
		s = pstring_realloc(p, s);
	s->str[s->len++] = c;
	s->str[s->len] = '\0';
	return s;
}

pstring_t *pstring_append_printf(pool_t *p, pstring_t *s, const char *format, ...)
{
	va_list ap, ap2;
	va_start(ap, format);
	va_copy(ap2, ap);

	uint_t remain = s->size - s->len;
	uint_t chars = vsnprintf(s->str + s->len, remain, format, ap);
	if (chars >= remain) {
		_pstring_realloc(p, s, pstring_round_size(s->len + chars + 1));
		vsnprintf(s->str + s->len, s->size - s->len, format, ap2);
	}
	s->len += chars;

	va_end(ap2);
	va_end(ap);
	return s;
}
