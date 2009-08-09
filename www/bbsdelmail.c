#include "libweb.h"

int bbsdelmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char file[40];
	strlcpy(file, getparm("f"), sizeof(file));
	if (!valid_mailname(file))
		return BBS_EINVAL;
	char buf[HOMELEN];
	mmap_t m;
	setmdir(buf, currentuser.userid);
	m.oflag = O_RDWR;
	if (mmap_open(buf, &m) < 0)
		return BBS_EINTNL;
	struct fileheader *fh = bbsmail_search(m.ptr, m.size, file);
	if (fh == NULL) {
		mmap_close(&m);
		return BBS_ENOFILE;
	}
	struct fileheader *end = 
			(struct fileheader *)m.ptr + m.size / sizeof(*fh);
	if (fh < end) {
		memmove(fh, fh + 1, (end - fh) * sizeof(*fh));
		mmap_truncate(&m, m.size - sizeof(*fh));
	}
	mmap_close(&m);
	if (file[0] != 's') {// not shared mail
		setmfile(buf, currentuser.userid, file);
		unlink(buf);
	}
	xml_header("bbsdelmail");
	printf("<bbsdelmail></bbsdelmail>\n");
	return 0;
}

