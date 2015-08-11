#include <stdarg.h>
#include <syslog.h>
#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/log.h"

void report(const char *s, const char *userid)
{
	syslog (LOG_LOCAL6 | LOG_INFO, "%-12.12s %s", userid, s);
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

int log_bm(log_bm_e type, int value)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "boards/%s/.bm.%s",
			currboard, currentuser.userid);

	int fd = open(file, O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		return 0;
	if (file_lock_all(fd, FILE_WRLCK) == -1) {
		file_close(fd);
		return 0;
	}

	int data[LOG_BM_LEN] = { 0 };
	file_read(fd, data, sizeof(data));
	if (type < LOG_BM_LEN)
		data[type] += value;

	lseek(fd, 0, SEEK_SET);
	file_write(fd, data, sizeof(data));

	(void) file_lock_all(fd, FILE_UNLCK);
	file_close(fd);
	return 0;
}

#define LOG_DEFINE(name, level) \
void log_internal_##name(const char *format, ...) \
{ \
	va_list ap; \
	va_start(ap, format); \
	vsyslog(level, format, ap); \
	va_end(ap); \
}

LOG_DEFINE(err, LOG_LOCAL7 | LOG_ERR)
LOG_DEFINE(warn, LOG_LOCAL7 | LOG_WARNING)
LOG_DEFINE(info, LOG_LOCAL7 | LOG_NOTICE)
LOG_DEFINE(dbg, LOG_LOCAL7 | LOG_INFO)
LOG_DEFINE(verbose, LOG_LOCAL7 | LOG_DEBUG)

LOG_DEFINE(info_legacy, LOG_LOCAL5 | LOG_INFO)
