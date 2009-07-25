#include "libweb.h"

int bbsanc_main()
{
	char *path = getparm("path");
	if (strstr(path, "bbslist") || strstr(path, ".Search")
			|| strstr(path, ".Names") || strstr(path, "..")
			|| strstr(path, "SYSHome"))
		http_fatal("文件不存在");
	char *board = getbfroma(path);
	struct boardheader *bp = NULL;
	if (*board != '\0') {
		bp = getbcache(board);
		if (!hasreadperm(&currentuser, bp))
			http_fatal("文件不存在或无权访问");
	}

	char fname[512];
	sprintf(fname, "0Announce%s", path);
	void *ptr;
	size_t size;
	int fd = mmap_open(fname, MMAP_RDONLY, &ptr, &size);
	if (fd < 0)
		http_fatal2(HTTP_STATUS_INTERNAL_ERROR, "文章打开失败");
	xml_header("bbsanc");
	fputs("<bbsanc><content>", stdout);
	xml_fputs((char *)ptr, stdout);
	mmap_close(ptr, size, fd);
	puts("</content>");
	if (bp != NULL)
		printf("<board>%s</board>", bp->filename);
	puts("</bbsanc>");
	return 0;
}
