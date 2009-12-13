#include "libweb.h"

int bbsmailcon_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char file[40];
	strlcpy(file, getparm("f"), sizeof(file));
	if (!valid_mailname(file))
		return BBS_EINVAL;
	char buf[HOMELEN];
	mmap_t m;
	m.oflag = O_RDWR;
	// deal with index
	setmdir(buf, currentuser.userid);
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;
	struct fileheader *fh = bbsmail_search(m.ptr, m.size, file);
	if (fh == NULL) {
		mmap_close(&m);
		return BBS_ENOFILE;
	}
	if (!(fh->accessed[0] & FILE_READ)) {
		fh->accessed[0] |= FILE_READ;
	}
	xml_header("bbsmailcon");
	printf("<bbsmailcon %s", get_session_str());
	struct fileheader *prev = fh - 1;
	if (prev >= (struct fileheader *)m.ptr)
		printf(" prev='%s'", prev->filename);
	struct fileheader *next = fh + 1;
	if (next < (struct fileheader *)m.ptr + m.size / sizeof(*next))
		printf(" next='%s'", next->filename);
	printf("><t>");
	xml_fputs(fh->title, stdout);
	printf("</t>");
	mmap_close(&m);

	// show mail content.
	if (file[0] == 's') // shared mail
		strlcpy(buf, file, sizeof(buf));
	else
		setmfile(buf, currentuser.userid, file);
	m.oflag = O_RDONLY;
	if (mmap_open(buf, &m) < 0)
		return BBS_ENOFILE;
	printf("<mail f='%s' n='%s'>", file, getparm("n"));
	xml_fputs((char *)m.ptr, stdout);
	fputs("</mail>\n", stdout);
	mmap_close(&m);
	printf("</bbsmailcon>");
	return 0;
}

