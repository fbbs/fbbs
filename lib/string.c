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

char datestring[30]; //For compatibility, should be eliminated gradually.

// Convert time to string in specified format.
// mode: 0 - "2001年02月03日04:05:06 星期六"
//       1 - "02/03/01 04:05:06"
//       2 - "02.03 04:05"
//       4 - "02/03/01 04:05:06 Sat"
int getdatestring(time_t time, int mode)
{
	struct tm t;
	char weeknum[7][3] = {"天", "一", "二", "三", "四", "五", "六"};
	char engweek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

	localtime_r(&time, &t);
	switch (mode) {
		case 0:
			sprintf(datestring, "%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
					t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
					t.tm_hour, t.tm_min, t.tm_sec, weeknum[t.tm_wday]);
			break;
		case 1:
			sprintf(datestring, "%02d/%02d/%02d %02d:%02d:%02d",
					t.tm_mon + 1, t.tm_mday, t.tm_year - 100,
					t.tm_hour, t.tm_min, t.tm_sec);
			break;
		case 2:
			sprintf(datestring, "%02d.%02d %02d:%02d",
					t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
			break;
		case 4:
		default:
			sprintf(datestring, "%02d/%02d/%02d %02d:%02d:%02d %3s",
					t.tm_mon + 1, t.tm_mday, t.tm_year - 100,
					t.tm_hour, t.tm_min, t.tm_sec, engweek[t.tm_wday]);
			break;
	}
	return (t.tm_sec % 10);
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
char *trim(char *str){
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
	if(*left <= 0x20){
		while(*++left <= 0x20)
			;
		memmove(ustr, left, right - left + 1);
	}
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

