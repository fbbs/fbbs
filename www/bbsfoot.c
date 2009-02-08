#include "libweb.h"

// Returns 1 if last mail is unread, 0 otherwise.
static int check_unread_mail(const struct userec *user)
{
	char buf[HOMELEN];
	setmfile(buf, user->userid, DOT_DIR);
	struct fileheader *fh = NULL;
	// Get count of all mails.
	size_t count = get_num_records(buf, sizeof(fh));
	// Get last mail header in .DIR.
	get_record(buf, fh, sizeof(fh), count);
	if (fh != NULL)
		return !(fh->accessed[0] & FILE_READ);
	return 0;
}

int bbsfoot_main(void)
{
	xml_header("bbsfoot");
	printf("<bbsfoot>\n");
	if (loginok) {
		printf("<user>%s</user>\n<gender>%c</gender>\n"
				"<month>%d</month>\n<day>%d</day>\n"
				"<mail>%d</mail>\n<bind>%d</bind>\n",
				currentuser.userid, currentuser.gender,
				currentuser.birthmonth, currentuser.birthday,
				check_unread_mail(&currentuser),
				HAS_PERM(PERM_BINDMAIL));
		int level, repeat;
		level = iconexp(countexp(&currentuser), &repeat);
		printf("<level>%d</level>\n<repeat>%d</repeat>\n", level, repeat);
		int dt;
#ifdef SPARC
		dt = abs(time(NULL) - *(int*)(u_info->from + 30)) / 60;
#else
		dt = abs(time(NULL) - *(int*)(u_info->from + 32)) / 60;
#endif
		printf("<hour>%d</hour>\n<min>%d</min>\n", dt / 60, dt % 60);
	}
	printf("<online>%d</online>\n", count_online());
	printf("</bbsfoot>\n");
	return 0;
}
