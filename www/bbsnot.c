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
	void *ptr;
	size_t size;
	int fd = mmap_open(fname, MMAP_RDONLY, &ptr, &size);
	if (fd < 0)
		return BBS_ENOFILE;
	xml_header("bbsnot");
	printf("<bbsnot><content>");
	xml_fputs((char *)ptr, stdout);
	mmap_close(ptr, size, fd);
	printf("</content><board>%s</board></bbsnot>", bp->filename);
	return 0;
}
