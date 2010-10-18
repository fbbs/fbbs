#include "libweb.h"
#include "fbbs/fileio.h"

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
		fb_flock(fd, LOCK_EX);
		safer_write(fd, text, strlen(text));
		fb_flock(fd, LOCK_UN);
		close(fd);
		xml_header(NULL);
		printf("<bbseufile desc='%s'>", desc);
		print_session();
		printf("</bbseufile>");
	} else {
		xml_header(NULL);
		printf("<bbseufile desc='%s' submit='%s'><text>", desc, submit);
		xml_printfile(buf, stdout);
		printf("</text>");
		print_session();
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

