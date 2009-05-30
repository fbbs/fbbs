#include "libweb.h"

int bbsnot_main(void)
{
	struct boardheader *bp = getbcache(getparm("board"));
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal("您选择的是一个目录");
	char fname[HOMELEN];
	snprintf(fname, sizeof(fname), "vote/%s/notes", bp->filename);
	void *ptr;
	size_t size;
	int fd;
	if (!safe_mmapfile(fname, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, &fd))
		http_fatal2(HTTP_STATUS_NOTFOUND, "本讨论区尚无进版画面");
	xml_header("bbsnot");
	printf("<bbsnot><content>");
	xml_fputs((char *)ptr, stdout);
	end_mmapfile(ptr, size, fd);
	printf("</content><board>%s</board></bbsnot>", bp->filename);
	return 0;
}
