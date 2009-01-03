// For logging.

#include <bbs.h>

void report(const char *s, const char *userid)
{
#ifdef USE_METALOG
	syslog (LOG_LOCAL6 | LOG_INFO, "%-12.12s %s", userid, s);
	return;
#else
	do_report("trace", s);
#endif
}
