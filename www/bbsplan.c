#include "libweb.h"

static int edit_user_file(const char *file, const char *desc, const char *submit)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char buf[HOMELEN];
	sethomefile(buf, currentuser.userid, file);
	parse_post_data();
	char *text = getparm("text");
	if (*text != '\0') {
		int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			return BBS_EINTNL;
		flock(fd, LOCK_EX);
		safer_write(fd, text, strlen(text));
		flock(fd, LOCK_UN);
		close(fd);
		xml_header("bbseufile");
		printf("<bbseufile ");
		print_session();
		printf(" desc='%s'></bbseufile>", desc);
	} else {
		xml_header("bbseufile");
		printf("<bbseufile ");
		print_session();
		printf(" desc='%s' submit='%s'>", desc, submit);
		xml_printfile(buf, stdout);
		printf("</bbseufile>");
	}
	return 0;
}

int bbsplan_main(void)
{
	return edit_user_file("plans", "±à¼­ËµÃ÷µµ", "plan");
}

int bbssig_main(void)
{
	return edit_user_file("signatures", "±à¼­Ç©Ãûµµ", "sig");
}

