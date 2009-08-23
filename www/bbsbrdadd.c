#include "libweb.h"

int bbsbrdadd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDWR;
	if (mmap_open(file, &m) < 0)
		return BBS_EINTNL;
	int size = m.size / sizeof(struct goodbrdheader);
	struct goodbrdheader *iter = m.ptr;
	struct goodbrdheader *end = iter + size;
	while (iter != end && iter->pos != bid - 1)
		++iter;
	if (iter == end) { // Not exist
		if (size >= GOOD_BRC_NUM) {
			mmap_close(&m);
			return BBS_EBRDQE;
		}
		if (mmap_truncate(&m, (size + 1) * sizeof(struct goodbrdheader)) < 0) {
			mmap_close(&m);
			return BBS_EINTNL;
		}
		iter = ((struct goodbrdheader *)m.ptr) + size;
		iter->id = size + 1;
		iter->pid = 0;
		iter->pos = bid - 1;
		memcpy(iter->title, bp->title, sizeof(iter->title));
		memcpy(iter->filename, bp->filename, sizeof(iter->filename));
	}
	mmap_close(&m);
	xml_header("bbsmybrd");
	printf("<bbsbrdadd><brd>%s</brd><bid>%d</bid></bbsbrdadd>",
			bp->filename, bid);
	return 0;
}
