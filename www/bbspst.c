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

	if (!loginok)
		return BBS_ELGNREQ;
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_EPST;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	unsigned int fid = 0;
	mmap_t m;
	struct fileheader fh;
	char *f = getparm("f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (!bbscon_search(bp, fid, 0, &fh))
			return BBS_ENOFILE;
		if (fh.accessed[0] & FILE_NOREPLY)
			return BBS_EPST;
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);
		m.oflag = O_RDONLY;
		if (mmap_open(file, &m) < 0)
			return BBS_ENOFILE;
	}
	
	xml_header("bbspst");
	printf("<root><bbspst p='%s' u='%s' brd='%s' bid='%d'>", get_permission(),
			currentuser.userid, bp->filename, bid);
	if (reply) {
		printf("<t>");
		ansi_filter(fh.title, fh.title);
		xml_fputs(fh.title, stdout);
		printf("</t><po f='%u'>", fid);
		web_quotation(m.ptr, m.size, &fh);
		mmap_close(&m);
		fputs("</po>", stdout);
	}
	printf("</bbspst></root>");
	return 0;
}

