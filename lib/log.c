#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"

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
	if (user == NULL) {
		syslog(LOG_LOCAL4 | LOG_INFO, "%s %s", mode, mesg);
	} else {
		syslog(LOG_LOCAL4 | LOG_INFO, "%s %-12s %s", mode, user->userid, mesg);
	}
	return;
}

void log_attempt(const char *name, const char *addr, const char *type)
{
	char file[STRLEN], buf[256];

	snprintf(buf, sizeof(buf), "%-12.12s  %-30s %s %s\n", name,
			format_time(fb_time(), TIME_FORMAT_ZH), addr, type);
	file_append(BADLOGINFILE, buf);
	sethomefile(file, name, BADLOGINFILE);
	file_append(file, buf);
}
