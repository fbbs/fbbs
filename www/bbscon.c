#include "libweb.h"

bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp)
{
	if (bp == NULL || fp == NULL)
		return false;
	char dir[HOMELEN];
	setbfile(dir, bp->filename, DOT_DIR);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(dir, &m) < 0)
		return false;
	struct fileheader *begin = m.ptr, *end;
	end = begin + (m.size / sizeof(*begin));
	const struct fileheader *f = dir_bsearch(begin, end, fid);
	if (f != end && f->id == fid) {
		unsigned int gid = f->gid;
		switch (action) {
			case 'p':  // previous post
				--f;
				break;
			case 'n':  // next post
				++f;
				break;
			case 'b':  // previous post of same thread
				while (--f >= begin && f->gid != gid)
					; // null statement
				break;
			case 'a':  // next post of same thread
				while (++f < end && f->gid != gid)
					; // null statement
				break;
			default:
				break;
		}
		if (f >= begin && f < end)
			*fp = *f;
		else
			f = NULL;
	}
	mmap_close(&m);
	return (f != NULL);
}

int bbscon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	unsigned int fid = strtoul(getparm("f"), NULL, 10);
	char *action = getparm("a");

	struct fileheader fh;
	if (!bbscon_search(bp, fid, *action, &fh))
		return BBS_ENOFILE;
	fid = fh.id;

	xml_header("bbscon");
	printf("<bbscon p='%s' u='%s' link='con' bid='%d'>",
			get_permission(), currentuser.userid, bid);
	printf("<po fid='%u'", fid);
	if (fh.reid != fh.id)
		printf(" reid='%u' gid='%u'>", fh.reid, fh.gid);
	else
		printf(">");
	char file[HOMELEN];
	setbfile(file, bp->filename, fh.filename);
	xml_printfile(file, stdout);
	printf("</po></bbscon>");
	return 0;
}

int bbsgcon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	char *f = getparm("f");
	if (strstr(f, "..") || strstr(f, "/") || strncmp(f, "G.", 2))
		return BBS_EINVAL;
	xml_header("bbscon");
	printf("<bbscon p='%s' u='%s' link='gcon' bid='%d'><po>",
			get_permission(), currentuser.userid, bid);
	char file[HOMELEN];
	setbfile(file, bp->filename, f);
	xml_printfile(file, stdout);
	printf("</po></bbscon>", bid);
	return 0;
}
