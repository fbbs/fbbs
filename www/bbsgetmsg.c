#include <utime.h>
#include "libweb.h"

// TODO: msg mechanism should be redesigned since no web delegation to handle signal
static void add_msg(void)
{
	char buf[BBSMSG_RECORD_LENGTH];
	char path[HOMELEN], *id = currentuser.userid;
	sethomefile(path, id, "wwwmsg.flush");
	utime(path, NULL);
	sethomefile(path, id, "msgfile");
	int i = file_size(path) / BBSMSG_RECORD_LENGTH;
	if (get_record(path, buf, BBSMSG_RECORD_LENGTH, i) <= 0)
		return;
	sethomefile(path, id, "wwwmsg");
	append_record(path, buf, BBSMSG_RECORD_LENGTH);
}

int bbsgetmsg_main(void)
{
	add_msg();
	xml_header("bbsgetmsg");
	printf("<bbsgetmsg>");
	if (loginok) {
		struct stat st;
		char path[HOMELEN];
		sethomefile(path, currentuser.userid, "wwwmsg");
		if (stat(path, &st) == 0 && st.st_size > 0) {
			char buf[BBSMSG_RECORD_LENGTH + 1];
			get_record(path, buf, BBSMSG_RECORD_LENGTH, 1);
			del_record(buf, BBSMSG_RECORD_LENGTH, 0);
			char toid[IDLEN + 1];
			int topid;
			buf[BBSMSG_SPLIT_OFFSET] = '\0';
			strlcpy(toid, buf + BBSMSG_SENDER_OFFSET, sizeof(toid));
			sscanf(buf + BBSMSG_PID_OFFSET, "%d", &topid);
			printf("<id>%s</id><pid>%d</pid><msg>", toid, topid);
			// ignore different msg formats..
			xml_fputs(buf + BBSMSG_CONTENT_OFFSET, stdout);
			puts("</msg>");
		}
	}
	printf("</bbsgetmsg>");
	return 0;
}

