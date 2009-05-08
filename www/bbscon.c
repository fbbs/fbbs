#include "libweb.h"

int bbscon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal("您选择的是一个目录");
	unsigned int fid = strtoul(getparm("f"), NULL, 10);
	struct fileheader fh;
	if (!bbscon_search(bp, fid, &fh))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的文章");

	char file[HOMELEN];
	setbfile(file, bp->filename, fh.filename);
	void *ptr;
	size_t size;
	int fd;
	if (!safe_mmapfile(file, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, &fd))
		http_fatal2(HTTP_STATUS_INTERNAL_ERROR, "文章打开失败");
	xml_header("bbscon");
	fputs("<bbscon>\n<post>", stdout);
	xml_fputs((char *)ptr, stdout);
	fputs("</post>\n", stdout);
	end_mmapfile(ptr, size, fd);
	printf("<bid>%d</bid>\n<f>%u</f>\n</bbscon>", bid, fid);
	return 0;
}
