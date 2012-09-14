#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include "config.h"
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

#ifndef HAVE_STRLCPY
/**
 * Copy @a src to string @a dst of size @a siz.
 * At most siz-1 characters will be copied.
 * Always NUL terminates (unless siz == 0).
 * From OpenBSD: strlcpy.c,v 1.11.
 * @param dst The buffer to be copied into.
 * @param src The string to be copied.
 * @param siz The size of the buffer.
 * @return strlen(src); if retval >= siz, truncation occurred.
 */
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
#endif

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

struct interval {
  int first;
  int last;
};

/* auxiliary function for binary search in interval table */
static int bisearch(wchar_t ucs, const struct interval *table, int max)
{
	int min = 0;
	int mid;

	if (ucs < table[0].first || ucs > table[max].last)
		return 0;
	while (max >= min) {
		mid = (min + max) / 2;
		if (ucs > table[mid].last)
			min = mid + 1;
		else if (ucs < table[mid].first)
			max = mid - 1;
		else
			return 1;
	}

	return 0;
}

/*
 * The following function define the column width of an ISO 10646
 * character as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
 *
 *    - Other format characters (general category code Cf in the Unicode
 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
 *      have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      Full-width (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that ucs_char_t characters are encoded
 * in ISO 10646.
 */
static int fb_wcwidth(wchar_t ch)
{
	/*
	 * Sorted list of non-overlapping intervals of non-spacing characters,
	 * generated by
	 *   "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c".
	 */
	static const struct interval combining[] = {
		{ 0x0300, 0x0357 }, { 0x035D, 0x036F }, { 0x0483, 0x0486 },
		{ 0x0488, 0x0489 }, { 0x0591, 0x05A1 }, { 0x05A3, 0x05B9 },
		{ 0x05BB, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
		{ 0x05C4, 0x05C4 }, { 0x0600, 0x0603 }, { 0x0610, 0x0615 },
		{ 0x064B, 0x0658 }, { 0x0670, 0x0670 }, { 0x06D6, 0x06E4 },
		{ 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED }, { 0x070F, 0x070F },
		{ 0x0711, 0x0711 }, { 0x0730, 0x074A }, { 0x07A6, 0x07B0 },
		{ 0x0901, 0x0902 }, { 0x093C, 0x093C }, { 0x0941, 0x0948 },
		{ 0x094D, 0x094D }, { 0x0951, 0x0954 }, { 0x0962, 0x0963 },
		{ 0x0981, 0x0981 }, { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 },
		{ 0x09CD, 0x09CD }, { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 },
		{ 0x0A3C, 0x0A3C }, { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 },
		{ 0x0A4B, 0x0A4D }, { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 },
		{ 0x0ABC, 0x0ABC }, { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 },
		{ 0x0ACD, 0x0ACD }, { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 },
		{ 0x0B3C, 0x0B3C }, { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 },
		{ 0x0B4D, 0x0B4D }, { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 },
		{ 0x0BC0, 0x0BC0 }, { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 },
		{ 0x0C46, 0x0C48 }, { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 },
		{ 0x0CBC, 0x0CBC }, { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 },
		{ 0x0CCC, 0x0CCD }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
		{ 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
		{ 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
		{ 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
		{ 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
		{ 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
		{ 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
		{ 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
		{ 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
		{ 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x1712, 0x1714 },
		{ 0x1732, 0x1734 }, { 0x1752, 0x1753 }, { 0x1772, 0x1773 },
		{ 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD }, { 0x17C6, 0x17C6 },
		{ 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD }, { 0x180B, 0x180D },
		{ 0x18A9, 0x18A9 }, { 0x1920, 0x1922 }, { 0x1927, 0x1928 },
		{ 0x1932, 0x1932 }, { 0x1939, 0x193B }, { 0x200B, 0x200F },
		{ 0x202A, 0x202E }, { 0x2060, 0x2063 }, { 0x206A, 0x206F },
		{ 0x20D0, 0x20EA }, { 0x302A, 0x302F }, { 0x3099, 0x309A },
		{ 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F }, { 0xFE20, 0xFE23 },
		{ 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB }, { 0x1D167, 0x1D169 },
		{ 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B },
		{ 0x1D1AA, 0x1D1AD }, { 0xE0001, 0xE0001 },
		{ 0xE0020, 0xE007F }, { 0xE0100, 0xE01EF }
	};

	/* test for 8-bit control characters */
	if (ch == 0)
		return 0;
	if (ch < 32 || (ch >= 0x7f && ch < 0xa0))
		return -1;

	/* binary search in table of non-spacing characters */
	if (bisearch(ch, combining, sizeof(combining)
				/ sizeof(struct interval) - 1))
		return 0;

	/*
	 * If we arrive here, ch is neither a combining nor a C0/C1
	 * control character.
	 */

	return 1 +
		(ch >= 0x1100 &&
		 /* Hangul Jamo init. consonants */
		 (ch <= 0x115f ||
		  ch == 0x2329 || ch == 0x232a ||
		  /* CJK ... Yi */
		  (ch >= 0x2e80 && ch <= 0xa4cf &&
		   ch != 0x303f) ||
		  /* Hangul Syllables */
		  (ch >= 0xac00 && ch <= 0xd7a3) ||
		  /* CJK Compatibility Ideographs */
		  (ch >= 0xf900 && ch <= 0xfaff) ||
		  /* CJK Compatibility Forms */
		  (ch >= 0xfe30 && ch <= 0xfe6f) ||
		  /* Fullwidth Forms */
		  (ch >= 0xff00 && ch <= 0xff60) ||
		  (ch >= 0xffe0 && ch <= 0xffe6) ||
		  (ch >= 0x20000 && ch <= 0x2fffd) ||
		  (ch >= 0x30000 && ch <= 0x3fffd)));
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
		int w = fb_wcwidth(wc);
		if (w <= 0)
			return -1;
		width += w;
	}
}

struct pstring_t {
	char *str;
	uint_t len;
	uint_t size;
};

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
		pstring_realloc(p, s);
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

pstring_t *pstring_append_space(pool_t *p, pstring_t *s)
{
	if (s->len != 0 && s->str[s->len - 1] != ' ')
		pstring_append_c(p, s, ' ');
	return s;
}

const char *pstring(const pstring_t *s)
{
	return s->str;
}
