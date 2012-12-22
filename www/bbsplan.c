#include "libweb.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/web.h"

static int edit_user_file(const char *file, const char *desc, const char *submit)
{
	if (!session.id)
		return BBS_ELGNREQ;
	char buf[HOMELEN];
	sethomefile(buf, currentuser.userid, file);
	parse_post_data();
	const char *text = get_param("text");
	if (*text != '\0') {
		int fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			return BBS_EINTNL;
		fb_flock(fd, LOCK_EX);
		file_write(fd, text, strlen(text));
		fb_flock(fd, LOCK_UN);
		close(fd);
		xml_header(NULL);
		printf("<bbseufile desc='%s'>", desc);
		print_session();
		printf("</bbseufile>");
	} else {
		xml_header(NULL);
		printf("<bbseufile desc='%s' submit='%s'><text>", desc, submit);
		xml_printfile(buf);
		printf("</text>");
		print_session();
		printf("</bbseufile>");
	}
	return 0;
}

int bbsplan_main(void)
{
	//% return edit_user_file("plans", "编辑说明档", "plan");
	return edit_user_file("plans", "\xb1\xe0\xbc\xad\xcb\xb5\xc3\xf7\xb5\xb5", "plan");
}

int bbssig_main(void)
{
	//% return edit_user_file("signatures", "编辑签名档", "sig");
	return edit_user_file("signatures", "\xb1\xe0\xbc\xad\xc7\xa9\xc3\xfb\xb5\xb5", "sig");
}

