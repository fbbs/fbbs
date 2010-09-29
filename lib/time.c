#include <stdio.h>
#include "fbbs/time.h"

/**
 * Convert time to string in specified format.
 * @param time time to convert.
 * @param mode specified format.
 * @return converted string.
 */
char *getdatestring(time_t time, enum DATE_FORMAT mode)
{
	static char str[32] = {'\0'};
	struct tm *t;
	char weeknum[7][3] = {"天", "一", "二", "三", "四", "五", "六"};
	char engweek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

	// No multi-thread
	t = localtime(&time);
	switch (mode) {
		case DATE_ZH:
			snprintf(str, sizeof(str),
					"%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec, weeknum[t->tm_wday]);
			break;
		case DATE_EN:
			snprintf(str, sizeof(str), "%02d/%02d/%02d %02d:%02d:%02d",
					t->tm_mon + 1, t->tm_mday, t->tm_year - 100,
					t->tm_hour, t->tm_min, t->tm_sec);
			break;
		case DATE_SHORT:
			snprintf(str, sizeof(str), "%02d.%02d %02d:%02d",
					t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
			break;
		case DATE_ENWEEK:
			snprintf(str, sizeof(str), "%02d/%02d/%02d %02d:%02d:%02d %3s",
					t->tm_mon + 1, t->tm_mday, t->tm_year - 100,
					t->tm_hour, t->tm_min, t->tm_sec, engweek[t->tm_wday]);
			break;
		case DATE_RSS:
			strftime(str, sizeof(str), "%a,%d %b %Y %H:%M:%S %z", t);
			break;
		case DATE_XML:
		default:
			snprintf(str, sizeof(str), "%4d-%02d-%02dT%02d:%02d:%02d",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec);
			break;
	}
	return str;
}


