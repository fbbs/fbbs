#include "libweb.h"

int bbsnot_main(void)
{
	struct boardheader *bp = getbcache(getparm("board"));
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	char fname[HOMELEN];
	snprintf(fname, sizeof(fname), "vote/%s/notes", bp->filename);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(fname, &m) < 0)
		return BBS_ENOFILE;
	xml_header("bbsnot");
	printf("<bbsnot p='%s' u='%s' brd='%s'>", get_permission(),
			currentuser.userid, bp->filename);
	xml_fputs((char *)m.ptr, stdout);
	mmap_close(&m);
	printf("</bbsnot>");
	return 0;
}
