#include "libweb.h"

enum {
	NEWMAIL_EXPIRE = 30,
};

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
	printf("<bbsmail start='%d' total='%d' page='%d' ", start, total, TLINES);
	print_session();
	printf(">");
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
	printf("</bbsmail>");
	return 0;
}

int print_new_mail(void *buf, int count, void *args)
{
	struct fileheader *fp = buf;
	time_t limit = *(time_t *)args;
	time_t date = getfiletime(fp);
	if (date < limit)
		return QUIT;
	if (!(fp->accessed[0] & FILE_READ)) {
		printf("<mail from='%s' date='%s' name='%s' n='%d'>", fp->owner, 
				getdatestring(date, DATE_XML), fp->filename, count);
		xml_fputs(fp->title, stdout);
		printf("</mail>");
	}
	return 0;
}

int bbsnewmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	xml_header("bbsnewmail");
	printf("<bbsnewmail ");
	print_session();
	printf(">");
	char file[HOMELEN];
	setmdir(file, currentuser.userid);
	time_t limit = time(NULL) - 24 * 60 * 60 * NEWMAIL_EXPIRE;
	apply_record(file, print_new_mail, sizeof(struct fileheader), &limit,
			false, true, true);
	printf("</bbsnewmail>");
	return 0;
}
