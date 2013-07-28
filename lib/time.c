#include <stdio.h>
#include "fbbs/time.h"

struct tm *fb_localtime(const fb_time_t *t)
{
	time_t tt = (time_t)*t;
	return localtime(&tt);
}

const char *fb_ctime(const fb_time_t *t)
{
	time_t tt = (time_t)*t;
	return ctime(&tt);
}

/**
 * 格式化时间.
 * @param time 时间.
 * @param fmt 格式.
 * @return 时间字符串.
 */
char *format_time(fb_time_t time, time_format_e fmt)
{
	//% "天" "一" "二" "三" "四" "五" "六"
	const char *weeknum[] = {
		"\xcc\xec", "\xd2\xbb", "\xb6\xfe", "\xc8\xfd",
		"\xcb\xc4", "\xce\xe5", "\xc1\xf9"
	};
	const char *utf8_weeknum[] = {
		"天", "一", "二", "三", "四", "五", "六"
	};
	static char str[32] = { '\0' };

	struct tm *t = fb_localtime(&time);
	switch (fmt) {
		case TIME_FORMAT_ZH:
			//% "%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
			snprintf(str, sizeof(str), "%4d\xc4\xea%02d\xd4\xc2%02d\xc8\xd5"
					"%02d:%02d:%02d \xd0\xc7\xc6\xda%2s", t->tm_year + 1900,
					t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
					t->tm_sec, weeknum[t->tm_wday]);
			break;
		case TIME_FORMAT_UTF8_ZH:
			snprintf(str, sizeof(str), "%4d年%02d月%02d日"
					"%02d:%02d:%02d 星期%s", t->tm_year + 1900,
					t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
					t->tm_sec, utf8_weeknum[t->tm_wday]);
			break;
		case TIME_FORMAT_EN:
			strftime(str, sizeof(str), "%m/%d/%Y %H:%M:%S", t);
			break;
		case TIME_FORMAT_SHORT:
			strftime(str, sizeof(str), "%m.%d %H:%M", t);
			break;
		case TIME_FORMAT_RSS:
			strftime(str, sizeof(str), "%a,%d %b %Y %H:%M:%S %z", t);
			break;
		case TIME_FORMAT_XML:
		default:
			strftime(str, sizeof(str), "%Y-%m-%dT%H:%M:%S", t);
			break;
	}
	return str;
}

/**
 * Validate date.
 * @param year Year. Not checked except for determining leap year.
 * @param month Month (1-12)
 * @param day Day (1-31)
 * @return True if valid, false otherwise.
 */
bool valid_date(int year, int month, int day)
{
	if (month < 1 || month > 12)
		return false;

	int days = 31;
	switch (month) {
		case 2:
			if (year % 4 == 0 && !(year % 100 == 0 && year % 400 != 0))
				days = 29;
			else
				days = 28;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			days = 30;
			break;
	}
	if (day < 1 || day > days)
		return false;

	return true;
}

char *fb_strftime(char *buf, size_t size, const char *fmt, fb_time_t t)
{
	strftime(buf, size, fmt, fb_localtime(&t));
	return buf;
}
