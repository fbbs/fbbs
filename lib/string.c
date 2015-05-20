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
static int strncasecmp_gbk(const char *s1, const char *s2, int n)
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
char *string_remove_ansi_control_code(char *dst, const char *src)
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
	while(*++ptr != '\0' && --i)
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
void string_remove_non_printable_gbk(char *str)
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
wchar_t next_wchar(const char **str, size_t *leftp)
{
	const uchar_t *s = (const uchar_t *) *str;
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

size_t string_check_tail(char *begin, char *end)
{
	if (!end)
		end = begin + strlen(begin);

	for (const char *p = end - 1; p >= begin; --p) {
		if (!is_cont(*p)) {
			if (next_wchar(&p, NULL) == WEOF) {
				*(char *) p = '\0';
				return end - p;
			}
			return 0;
		}
	}
	return 0;
}

size_t string_cp(char *dst, const char *src, size_t siz)
{
	size_t size = strlcpy(dst, src, siz);
	if (size >= siz)
		size -= string_check_tail(dst, dst + siz - 1);
	return size;
}

size_t string_copy_allow_null(char *dst, const char *src, size_t size)
{
	if (!src) {
		*dst = '\0';
		return 0;
	}
	return strlcpy(dst, src, size);
}

struct interval {
  int first;
  int last;
};

/* auxiliary function for binary search in interval table */
static bool bisearch(wchar_t ucs, const struct interval *table, int max)
{
	int min = 0;
	int mid;

	if (ucs < table[0].first || ucs > table[max].last)
		return false;
	while (max >= min) {
		mid = (min + max) / 2;
		if (ucs > table[mid].last)
			min = mid + 1;
		else if (ucs < table[mid].first)
			max = mid - 1;
		else
			return true;
	}

	return false;
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
int fb_wcwidth(wchar_t ch)
{
	/*
	 * Sorted list of non-overlapping intervals of non-spacing characters,
	 * generated by
	 *   "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c".
	 */
	static const struct interval combining[] = {
		{ 0x0300, 0x036F }, { 0x0483, 0x0489 }, { 0x0591, 0x05BD },
		{ 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 }, { 0x05C4, 0x05C5 },
		{ 0x05C7, 0x05C7 }, { 0x0600, 0x0605 }, { 0x0610, 0x061A },
		{ 0x061C, 0x061C }, { 0x064B, 0x065F }, { 0x0670, 0x0670 },
		{ 0x06D6, 0x06DD }, { 0x06DF, 0x06E4 }, { 0x06E7, 0x06E8 },
		{ 0x06EA, 0x06ED }, { 0x070F, 0x070F }, { 0x0711, 0x0711 },
		{ 0x0730, 0x074A }, { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 },
		{ 0x0816, 0x0819 }, { 0x081B, 0x0823 }, { 0x0825, 0x0827 },
		{ 0x0829, 0x082D }, { 0x0859, 0x085B }, { 0x08E4, 0x0902 },
		{ 0x093A, 0x093A }, { 0x093C, 0x093C }, { 0x0941, 0x0948 },
		{ 0x094D, 0x094D }, { 0x0951, 0x0957 }, { 0x0962, 0x0963 },
		{ 0x0981, 0x0981 }, { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 },
		{ 0x09CD, 0x09CD }, { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 },
		{ 0x0A3C, 0x0A3C }, { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 },
		{ 0x0A4B, 0x0A4D }, { 0x0A51, 0x0A51 }, { 0x0A70, 0x0A71 },
		{ 0x0A75, 0x0A75 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
		{ 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
		{ 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 },	{ 0x0B3C, 0x0B3C },
		{ 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B44 }, { 0x0B4D, 0x0B4D },
		{ 0x0B56, 0x0B56 }, { 0x0B62, 0x0B63 }, { 0x0B82, 0x0B82 },
		{ 0x0BC0, 0x0BC0 }, { 0x0BCD, 0x0BCD }, { 0x0C00, 0x0C00 },
		{ 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 }, { 0x0C4A, 0x0C4D },
		{ 0x0C55, 0x0C56 }, { 0x0C62, 0x0C63 }, { 0x0C81, 0x0C81 },
		{ 0x0CBC, 0x0CBC }, { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 },
		{ 0x0CCC, 0x0CCD }, { 0x0CE2, 0x0CE3 }, { 0x0D01, 0x0D01 },
		{ 0x0D41, 0x0D44 }, { 0x0D4D, 0x0D4D }, { 0x0D62, 0x0D63 },
		{ 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
		{ 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
		{ 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
		{ 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
		{ 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
		{ 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F8D, 0x0F97 },
		{ 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
		{ 0x1032, 0x1037 }, { 0x1039, 0x103A }, { 0x103D, 0x103E },
		{ 0x1058, 0x1059 }, { 0x105E, 0x1060 }, { 0x1071, 0x1074 },
		{ 0x1082, 0x1082 }, { 0x1085, 0x1086 }, { 0x108D, 0x108D },
		{ 0x109D, 0x109D }, { 0x1160, 0x11FF }, { 0x135D, 0x135F },
		{ 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
		{ 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
		{ 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
		{ 0x180B, 0x180E }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
		{ 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
		{ 0x1A17, 0x1A18 }, { 0x1A1B, 0x1A1B }, { 0x1A56, 0x1A56 },
		{ 0x1A58, 0x1A5E }, { 0x1A60, 0x1A60 }, { 0x1A62, 0x1A62 },
		{ 0x1A65, 0x1A6C }, { 0x1A73, 0x1A7C }, { 0x1A7F, 0x1A7F },
		{ 0x1AB0, 0x1ABE }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
		{ 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
		{ 0x1B6B, 0x1B73 }, { 0x1B80, 0x1B81 }, { 0x1BA2, 0x1BA5 },
		{ 0x1BA8, 0x1BA9 }, { 0x1BAB, 0x1BAD }, { 0x1BE6, 0x1BE6 },
		{ 0x1BE8, 0x1BE9 }, { 0x1BED, 0x1BED }, { 0x1BEF, 0x1BF1 },
		{ 0x1C2C, 0x1C33 }, { 0x1C36, 0x1C37 }, { 0x1CD0, 0x1CD2 },
		{ 0x1CD4, 0x1CE0 }, { 0x1CE2, 0x1CE8 }, { 0x1CED, 0x1CED },
		{ 0x1CF4, 0x1CF4 }, { 0x1CF8, 0x1CF9 }, { 0x1DC0, 0x1DF5 },
		{ 0x1DFC, 0x1DFF }, { 0x200B, 0x200F }, { 0x202A, 0x202E },
		{ 0x2060, 0x2064 }, { 0x2066, 0x206F }, { 0x20D0, 0x20F0 },
		{ 0x2CEF, 0x2CF1 }, { 0x2D7F, 0x2D7F }, { 0x2DE0, 0x2DFF },
		{ 0x302A, 0x302D }, { 0x3099, 0x309A }, { 0xA66F, 0xA672 },
		{ 0xA674, 0xA67D }, { 0xA69F, 0xA69F }, { 0xA6F0, 0xA6F1 },
		{ 0xA802, 0xA802 }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
		{ 0xA825, 0xA826 }, { 0xA8C4, 0xA8C4 }, { 0xA8E0, 0xA8F1 },
		{ 0xA926, 0xA92D }, { 0xA947, 0xA951 }, { 0xA980, 0xA982 },
		{ 0xA9B3, 0xA9B3 }, { 0xA9B6, 0xA9B9 }, { 0xA9BC, 0xA9BC },
		{ 0xA9E5, 0xA9E5 }, { 0xAA29, 0xAA2E }, { 0xAA31, 0xAA32 },
		{ 0xAA35, 0xAA36 }, { 0xAA43, 0xAA43 }, { 0xAA4C, 0xAA4C },
		{ 0xAA7C, 0xAA7C }, { 0xAAB0, 0xAAB0 }, { 0xAAB2, 0xAAB4 },
		{ 0xAAB7, 0xAAB8 }, { 0xAABE, 0xAABF }, { 0xAAC1, 0xAAC1 },
		{ 0xAAEC, 0xAAED }, { 0xAAF6, 0xAAF6 }, { 0xABE5, 0xABE5 },
		{ 0xABE8, 0xABE8 }, { 0xABED, 0xABED }, { 0xFB1E, 0xFB1E },
		{ 0xFE00, 0xFE0F }, { 0xFE20, 0xFE2D }, { 0xFEFF, 0xFEFF },
		{ 0xFFF9, 0xFFFB }, { 0x101FD, 0x101FD }, { 0x102E0, 0x102E0 },
		{ 0x10376, 0x1037A }, { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 },
		{ 0x10A0C, 0x10A0F }, { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F },
		{ 0x10AE5, 0x10AE6 }, { 0x11001, 0x11001 }, { 0x11038, 0x11046 },
		{ 0x1107F, 0x11081 }, { 0x110B3, 0x110B6 }, { 0x110B9, 0x110BA },
		{ 0x110BD, 0x110BD }, { 0x11100, 0x11102 }, { 0x11127, 0x1112B },
		{ 0x1112D, 0x11134 }, { 0x11173, 0x11173 }, { 0x11180, 0x11181 },
		{ 0x111B6, 0x111BE }, { 0x1122F, 0x11231 }, { 0x11234, 0x11234 },
		{ 0x11236, 0x11237 }, { 0x112DF, 0x112DF }, { 0x112E3, 0x112EA },
		{ 0x11301, 0x11301 }, { 0x1133C, 0x1133C }, { 0x11340, 0x11340 },
		{ 0x11366, 0x1136C }, { 0x11370, 0x11374 }, { 0x114B3, 0x114B8 },
		{ 0x114BA, 0x114BA }, { 0x114BF, 0x114C0 }, { 0x114C2, 0x114C3 },
		{ 0x115B2, 0x115B5 }, { 0x115BC, 0x115BD }, { 0x115BF, 0x115C0 },
		{ 0x11633, 0x1163A }, { 0x1163D, 0x1163D }, { 0x1163F, 0x11640 },
		{ 0x116AB, 0x116AB }, { 0x116AD, 0x116AD }, { 0x116B0, 0x116B5 },
		{ 0x116B7, 0x116B7 }, { 0x16AF0, 0x16AF4 }, { 0x16B30, 0x16B36 },
		{ 0x16F8F, 0x16F92 }, { 0x1BC9D, 0x1BC9E }, { 0x1BCA0, 0x1BCA3 },
		{ 0x1D167, 0x1D169 }, { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B },
		{ 0x1D1AA, 0x1D1AD }, { 0x1D242, 0x1D244 }, { 0x1E8D0, 0x1E8D6 },
		{ 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F }, { 0xE0100, 0xE01EF },
	};

	static const struct interval full_width[] = {
		{ 0x1100, 0x115F }, { 0x2329, 0x232A }, { 0x2E80, 0x2E99 },
		{ 0x2E9B, 0x2EF3 }, { 0x2F00, 0x2FD5 }, { 0x2FF0, 0x2FFB },
		{ 0x3000, 0x303E }, { 0x3041, 0x3096 }, { 0x3099, 0x30FF },
		{ 0x3105, 0x312D }, { 0x3131, 0x318E }, { 0x3190, 0x31BA },
		{ 0x31C0, 0x31E3 }, { 0x31F0, 0x321E }, { 0x3220, 0x3247 },
		{ 0x3250, 0x32FE }, { 0x3300, 0x4DBF }, { 0x4E00, 0xA48C },
		{ 0xA490, 0xA4C6 }, { 0xA960, 0xA97C }, { 0xAC00, 0xD7A3 },
		{ 0xF900, 0xFAFF }, { 0xFE10, 0xFE19 }, { 0xFE30, 0xFE52 },
		{ 0xFE54, 0xFE66 }, { 0xFE68, 0xFE6B }, { 0xFF01, 0xFF60 },
		{ 0xFFE0, 0xFFE6 }, { 0x1B000, 0x1B001 }, { 0x1F200, 0x1F202 },
		{ 0x1F210, 0x1F23A }, { 0x1F240, 0x1F248 }, { 0x1F250, 0x1F251 },
		{ 0x20000, 0x2FFFD }, { 0x30000, 0x3FFFD }
	};

	/* test for 8-bit control characters */
	if (ch == 0)
		return 0;
	if (ch < 32 || (ch >= 0x7f && ch < 0xa0))
		return -1;

	/* binary search in table of non-spacing characters */
	if (bisearch(ch, combining, ARRAY_SIZE(combining) - 1))
		return 0;

	if (bisearch(ch, full_width, ARRAY_SIZE(full_width) - 1))
		return 2;

	return 1;
}

/**
 * 将宽字符串转换成UTF-8编码的字符串
 * @param dest 存放结果的缓冲区
 * @param src 要转换的宽字符串
 * @param n 目标缓冲区的长度
 * @return 已被转换的UTF-8字符串长度
 */
size_t fb_wcstombs(char *dest, const wchar_t *src, size_t n)
{
	uchar_t *p = (uchar_t *) dest, *end = p + n;
	wchar_t wc;
	while ((wc = *src++)) {
		if (wc < 0x80) {
			if (p + 1 >= end)
				goto r;
			p[0] = wc;
			p += 1;
		} else if (wc < 0x800) {
			if (p + 2 >= end)
				goto r;
			p[0] = 0xc0 | (wc >> 6);
			p[1] = 0x80 | (wc & 0x3f);
			p += 2;
		} else if (wc < 0x10000) {
			if (p + 3 >= end)
				goto r;
			p[0] = 0xe0 | (wc >> 12);
			p[1] = 0x80 | ((wc >> 6) & 0x3f);
			p[2] = 0x80 | (wc & 0x3f);
			p += 3;
		} else if (wc < 0x200000) {
			if (p + 4 >= end)
				goto r;
			p[0] = 0xf0 | (wc >> 18);
			p[1] = 0x80 | ((wc >> 12) & 0x3f);
			p[2] = 0x80 | ((wc >> 6) & 0x3f);
			p[3] = 0x80 | (wc & 0x3f);
			p += 4;
		} else {
			return (size_t) -1;
		}
	}
r:	*p = '\0';
	return p - (uchar_t *) dest;
}

/**
 * 在UTF-8字符串中寻找上一个字符的起始位置
 * @param begin 字符串头指针
 * @param ptr 字符串当前位置的指针
 * @return 上一个字符的起始位置
 */
const char *string_previous_utf8_start(const char *begin, const char *ptr)
{
	while (--ptr >= begin) {
		uchar_t c = *(const uchar_t *) ptr;
		if (c < 0x80 || c >= 0xc0) {
			return ptr;
		}
	}
	return ptr;
}

/**
 * 在UTF-8字符串中寻找下一个字符的起始位置
 * @param ptr 字符串当前位置的指针
 * @param end 字符串的尾端指针
 * @return 下一个字符的起始位置
 */
const char *string_next_utf8_start(const char *ptr, const char *end)
{
	while (++ptr < end) {
		uchar_t c = *(const uchar_t *) ptr;
		if (c < 0x80 || c >= 0xc0) {
			return ptr;
		}
	}
	return ptr;
}

void string_remove_non_printable(char *str)
{
	if (!str)
		return;

	char *dst = str;
	for (; *str; ) {
		char *src = str;
		const char **ptr = (const char **) &str;
		wchar_t wc = next_wchar(ptr, NULL);
		if (!wc) {
			*dst = '\0';
		} else if (wc == WEOF) {
			++str;
		} else if (fb_wcwidth(wc) > 0) {
			if (dst != src)
				memcpy(dst, src, str - src);
			dst += str - src;
		}
	}
	*dst = '\0';
}

/**
 * 检查UTF-8字符串的宽度和长度
 * @param[in] str 字符串
 * @param[in] length 最大允许长度
 * @param[in] max_width 最大允许宽度
 * @param[in] allow_zero_width 允许零宽度字符, 允许换行符
 * @param[in] allow_esc 允许ESC(27)
 * @return 如果字符串符合长度和宽度要求, 不含非法UTF-8字符或零宽度字符,
 *         则返回其长度, 否则返回-1.
 */
int string_validate_width_and_length(const char *str, size_t length,
		size_t max_width, bool allow_zero_width, bool allow_esc)
{
	const char *s = str;
	size_t width = 0;
	while (1) {
		wchar_t wc = next_wchar(&s, NULL);
		if (wc == WEOF)
			return -1;
		if (!wc) {
			if (s - str > length || width > max_width)
				return -1;
			return s - str;
		}
		int w = fb_wcwidth(wc);

		if (!(w > 0 || (allow_esc && wc == L'\033')
				|| (allow_zero_width && (w == 0 || wc == L'\n')))) {
			return -1;
		}
		width += w;
	}
}

/**
 * 检查UTF-8字符串
 * @param[in] str 要检测的字符串
 * @param[in] max_chinese_chars 最多的汉字字符数，每个汉字记为长度4，宽度2。
 * @param[in] allow_zero_width_or_esc 允许零宽度字符, 允许换行符, 允许ESC(27)
 * @return @see ::string_validate_width_and_length
 */
int string_validate_utf8(const char *str, size_t max_chinese_chars,
		bool allow_zero_width_or_esc)
{
	return string_validate_width_and_length(str, max_chinese_chars * 4,
			max_chinese_chars * 2, allow_zero_width_or_esc,
			allow_zero_width_or_esc);
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
	return s;
}

pstring_t *pstring_append_string(pool_t *p, pstring_t *s, const char *str)
{
	size_t len = strlen(str);
	if (s->len + len >= s->size)
		pstring_realloc(p, s);
	memmove(s->str + s->len, str, len);
	s->len += len;
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
	if (s->len != 0 && !isspace(s->str[s->len - 1]))
		pstring_append_c(p, s, ' ');
	return s;
}

const char *pstring(const pstring_t *s)
{
	s->str[s->len] = '\0';
	return s->str;
}

/**
 * Find line end from string.
 * @param begin The line beginning.
 * @param end Off-the-end pointer of the string.
 * @return Off-the-end pointer of the line.
 */
const char *get_line_end(const char *begin, const char *end)
{
	const char *s = begin;
	while (s != end && *s != '\n') {
		++s;
	}
	return (s == end ? s : s + 1);
}
