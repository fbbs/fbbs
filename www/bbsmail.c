#include "libweb.h"

int bbsmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	int start = strtol(getparm("start"), NULL, 10);
	char buf[HOMELEN];
	setmdir(buf, currentuser.userid);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;
	int total = m.size / sizeof(struct fileheader);
	if (start <= 0)
		start = total - TLINES + 1;
	if (start < 1)
		start = 1;
	struct fileheader *fh = (struct fileheader *)m.ptr + start - 1;
	struct fileheader *end = (struct fileheader *)m.ptr + total;
	xml_header("bbsmail");
	printf("<root><bbsmail p='%s' u='%s' start='%d' total='%d' page='%d'>",
			get_permission(), currentuser.userid, start, total, TLINES);
	for (int i = 0; i < TLINES && fh != end; ++i) {
		int mark = ' ';
		if (fh->accessed[0] & MAIL_REPLY)
			mark = 'r';
		if (fh->accessed[0] & FILE_MARKED) {
			if (mark == 'r')
				mark = 'b';
			else
				mark = 'm';
		}
		if (!(fh->accessed[0] & FILE_READ)) {
			if (mark == ' ')
				mark = '+';
			else
				mark = toupper(mark);
		}
		printf("<mail m='%c' from='%s' date='%s' name='%s'>", mark, fh->owner,
				getdatestring(getfiletime(fh), DATE_XML), fh->filename);
		xml_fputs(fh->title, stdout);
		printf("</mail>");
		fh++;
	}
	mmap_close(&m);
	printf("</bbsmail></root>");
	return 0;
}
