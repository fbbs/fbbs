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

void log_usies(const char *mode, const char *mesg, const struct userec *user)
{
	char *fmt;

	fmt = user->userid[0] ? "%s %-12s %s" : "%s %s%s";
	syslog(LOG_LOCAL4 | LOG_INFO, fmt, mode, user->userid, mesg);
	return;
}

