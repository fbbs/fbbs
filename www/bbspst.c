#include "libweb.h"

static int web_quotation(const char *str, size_t size, const struct fileheader *fh)
{
	printf("【 在 %s 的大作中提到: 】\n", fh->owner);
	int lines = 0;
	const char *start = str;
	const char *end = str + size;
	for (const char *ptr = start; ptr != end; start = ++ptr) {
		while (ptr != end && *ptr != '\n') {
			++ptr;
		}
		if (ptr == end) {
			xml_fputs2(start, ptr - start, stdout);
			break;
		} else {
			if (ptr == start)
				continue;
			if (!strncmp(start, ": 【", 4) || !strncmp(start, ": : ", 4))
				continue;
			if (!strncmp(start, "--\n", 3))
				break;
			if (lines++ < 3)
				continue;			
			if (lines >= 10) {
				fputs(": .................（以下省略）", stdout);
				break;
			}
			fwrite(": ", sizeof(char), 2, stdout);
			xml_fputs2(start, ptr - start + 1, stdout);
		}
	}
	return lines;	
}

int bbspst_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
#ifdef aasdffdsa
	if (!loginok)
		http_fatal(HTTP_STATUS_OK, "匆匆过客不能发表文章，请先登录");
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	if (bp->flag & BOARD_DIR_FLAG)
		http_fatal(HTTP_STATUS_OK, "您选择的是一个目录");
	// TODO: Check if the user is banned of posting
#endif
	unsigned int fid;
	void *ptr;
	size_t size;
	int fd;
	struct fileheader fh;
	char *f = getparm("f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (!bbscon_search(bp, fid, &fh))
			http_fatal(HTTP_STATUS_OK, "错误的文章");
		if (fh.accessed[0] & FILE_NOREPLY)
			http_fatal(HTTP_STATUS_OK, "该文章具有不可回复属性");
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);
		if (!safe_mmapfile(file, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, &fd))
			http_fatal(HTTP_STATUS_INTERNAL_ERROR, "文章打开失败");
	}
	
	xml_header("bbspst");
	printf("<bbspst>\n<board>%s</board>\n<user>%s</user>\n",
			bp->filename, currentuser.userid);
	if (reply) {
		printf("<title>%s</title>\n", fh.title);
		fputs("<post>", stdout);
		web_quotation(ptr, size, &fh);
		end_mmapfile(ptr, size, fd);
		fputs("</post>\n", stdout);
	}
	fputs("</bbspst>", stdout);
	
	return 0;
}

