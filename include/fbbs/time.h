#ifndef FB_TIME_H
#define FB_TIME_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>


typedef uint32_t fb_time_t;
#define PRIdFBT PRId32
#define parcel_write_fb_time(parcel, val)  parcel_write_varuint64(parcel, val)
#define parcel_read_fb_time(parcel)  parcel_read_varuint64(parcel)

typedef enum {
	TIME_FORMAT_ZH,      ///< "2001年02月03日04:05:06 星期六"
	TIME_FORMAT_UTF8_ZH, ///< "2001年02月03日04:05:06 星期六"
	TIME_FORMAT_EN,      ///< "02/03/01 04:05:06"
	TIME_FORMAT_SHORT,   ///< "02.03 04:05"
	TIME_FORMAT_XML,     ///< "2001-02-03T04:05:06"
	TIME_FORMAT_RSS,     ///< "Sat,03 Feb 2001 04:05:06 +0800"
} time_format_e;

#define fb_time() ((fb_time_t) time(NULL))
extern struct tm *fb_localtime(const fb_time_t *t);
extern const char *fb_ctime(const fb_time_t *t);

extern char *format_time(fb_time_t time, time_format_e fmt);
extern char *fb_strftime(char *buf, size_t size, const char *fmt, fb_time_t t);

extern bool valid_date(int year, int month, int day);

#endif // FB_TIME_H
