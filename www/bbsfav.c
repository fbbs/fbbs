#include "libweb.h"

int bbsfav_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	// Read '.goodbrd'
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return BBS_ENOFILE;
	struct goodbrdheader *iter, *end;
	int num = m.size / sizeof(struct goodbrdheader);
	if (num > GOOD_BRC_NUM)
		num = GOOD_BRC_NUM;
	end = (struct goodbrdheader *)m.ptr + num;

	// Print all favorite boards.
	xml_header("bbsfav");
	printf("<root><bbsfav p='%s' u='%s'>", get_permission(),
			currentuser.userid);
	for (iter = m.ptr; iter != end; ++iter) {
		if (!gbrd_is_custom_dir(iter)) {
			printf("<brd bid='%d' brd='%s'>", iter->pos + 1, iter->filename);
			xml_fputs(iter->title + 11, stdout);
			printf("</brd>");
		}
	}
	printf("</bbsfav></root>");
	return 0;
}
