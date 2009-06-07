#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Convert string 'src' to lowercase and store it in 'dst'.
// Caller should ensure the capacity of 'dst' is no less than 'src'.
char *strtolower(char *dst, const char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = tolower(*src++);
	*dst = '\0';
	return ret;	
}

// Convert string 'src' to uppercase and store it in 'dst'.
// Caller should ensure the capacity of 'dst' is no less than 'src'.
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
int strncasecmp_gbk(const char *s1, const char *s2, int n) {
	register int c1, c2, l = 0;

	while (*s1 && *s2 && l < n) {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
		if (c1 != c2)
			return (c1 - c2);
		++l;
		if (c1 & 0x80) {
			if(*s1 == *s2)
				++l;
			else
				return (*s1 - *s2);
		}
	}
	if (l==n)
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
