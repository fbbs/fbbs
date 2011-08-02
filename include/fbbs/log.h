#ifndef FB_LOG_H
#define FB_LOG_H

#define fb_log(domain, format, ...) \
	syslog(LOG_LOCAL6 | LOG_INFO, "%s "format, domain, __VA_ARGS__)

#endif // FB_LOG_H
