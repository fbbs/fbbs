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
	char *action = getparm("a");

	struct fileheader fh;
	if (!bbscon_search(bp, fid, *action, &fh))
		http_fatal2(HTTP_STATUS_NOTFOUND, "没有找到指定的文章");
	fid = fh.id;

	xml_header("bbscon");
	printf("<bbscon><post>");
	char file[HOMELEN];
	setbfile(file, bp->filename, fh.filename);
	xml_printfile(file, stdout);
	puts("</post>");
	if (fh.reid != fh.id)
		printf("<reid>%u</reid>\n<gid>%u</gid>\n", fh.reid, fh.gid);
	printf("<bid>%d</bid><f>%u</f><link>con</link></bbscon>", bid, fid);
	return 0;
}

int bbsgcon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal("您选择的是一个目录");
	char *f = getparm("f");
	if (strstr(f, "..") || strstr(f, "/") || strncmp(f, "G.", 2))
		http_fatal("错误的文件名");
	xml_header("bbscon");
	printf("<bbscon><post>");
	char file[HOMELEN];
	setbfile(file, bp->filename, f);
	xml_printfile(file, stdout);
	printf("</post><bid>%d</bid><link>gcon</link></bbscon>", bid);
	return 0;
}
