#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Convert string src to lowercase and store it in dst.
// Caller should ensure the capacity of dst is no less than src.
char *strtolower(char *dst, char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = tolower(*src++);
	*dst = '\0';
	return ret;	
}

// Convert string src to uppercase and store it in dst.
// Caller should ensure the capacity of dst is no less than src.
char *strtoupper(char *dst, char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = toupper(*src++);
	*dst = '\0';
	return ret;	
}

// Eliminate ANSI escape codes from src and store it in dst.
// src and dst can be the same.
char *ansi_filter(char *dst, char *src)
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
    };
    *dst = '\0';
    return ret;
}

// 将一个整数时间值轮换成 年月日时分秒周日格式,并返回
char *Cdate(time_t *clock) {
	static char foo[23];
	struct tm *mytm = localtime(clock);

	strftime(foo, 23, "%D %T %a", mytm);
	return (foo);
}
