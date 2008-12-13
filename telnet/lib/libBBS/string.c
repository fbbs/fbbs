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
