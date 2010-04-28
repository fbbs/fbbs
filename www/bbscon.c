#include "libweb.h"

static int cmp_fid(void *arg, void *buf)
{
	struct fileheader *fp = buf;
	unsigned int *fid = arg;
	return (fp->id == *fid);
}

// Find post whose id = 'fid'.
// If 'fid' > any post's id, return 'end',
// otherwise, return the minimum one among all post whose id > 'fid'.
const struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid)
{
	const struct fileheader *mid;
	while (begin < end) {
		mid = begin + (end - begin) / 2;
		if (mid->id == fid) {
			return mid;
		}
		if (mid->id < fid) {
			begin = mid + 1;
		} else {
			end = mid;
		}
	}
	return begin;
}

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
	bool sticky = *getparm("s");

	struct fileheader fh;
	if (sticky) {
		char file[HOMELEN];
		setbfile(file, bp->filename, NOTICE_DIR);
		if (!search_record(file, &fh, sizeof(fh), cmp_fid, &fid))
			return BBS_ENOFILE;
	} else {
		if (!bbscon_search(bp, fid, *action, &fh))
			return BBS_ENOFILE;
	}
	fid = fh.id;

	xml_header("bbs");
	printf("<bbscon link='con' bid='%d' ", bid);
	print_session();
	printf(">");
	printf("<po fid='%u'", fid);
	if (sticky)
		printf(" sticky='1'");
	if (fh.reid != fh.id)
		printf(" reid='%u' gid='%u'>", fh.reid, fh.gid);
	else
		printf(">");
	char file[HOMELEN];
	setbfile(file, bp->filename, fh.filename);
	xml_printfile(file, stdout);
	printf("</po></bbscon>");

	brc_fcgi_init(currentuser.userid, bp->filename);
	brc_addlist(fh.filename);
	brc_update(currentuser.userid, bp->filename);
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
	xml_header("bbs");
	printf("<bbscon link='gcon' bid='%d' ", bid);
	print_session();
	printf("><po>");
	char file[HOMELEN];
	setbfile(file, bp->filename, f);
	xml_printfile(file, stdout);
	printf("</po></bbscon>", bid);
	brc_fcgi_init(currentuser.userid, bp->filename);
	brc_addlist(f);
	brc_update(currentuser.userid, bp->filename);
	return 0;
}
