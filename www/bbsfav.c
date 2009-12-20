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
	printf("<bbsfav ");
	print_session();
	printf(">");
	for (iter = m.ptr; iter != end; ++iter) {
		if (!gbrd_is_custom_dir(iter)) {
			struct boardheader *bp = bcache + iter->pos;
			printf("<brd bid='%d' brd='%s'>", iter->pos + 1, bp->filename);
			xml_fputs(get_board_desc(bp), stdout);
			printf("</brd>");
		}
	}
	mmap_close(&m);
	printf("</bbsfav>");
	return 0;
}
