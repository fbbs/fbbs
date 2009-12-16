#include "libweb.h"

extern bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp);

int web_quotation(const char *str, size_t size, const char *owner, bool ismail)
{
	printf("【 在 %s 的%s中提到: 】\n", owner, ismail ? "来信" : "大作");
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

static void get_post_body(char **begin, char **end)
{
	char *ptr = *begin, *e = *end;
	// skip header.
	int n = 3;
	while (ptr != e && n >= 0) {
		if (*ptr == '\n')
			--n;
		++ptr;
	}
	*begin = ptr;

	ptr = e - 2; // skip last '\n'
	while (ptr >= *begin && *ptr != '\n')
		--ptr;
	if (ptr < *begin)
		return;
	if (!strncmp(ptr + 1, "\033[m\033[1;36m※ 修改", 17)) {
		--ptr;
		while (ptr >= *begin && *ptr != '\n')
			--ptr;
		*end = (ptr >= *begin) ? ptr : *begin;
	} else {
		*end = ptr;
	}
}

static int do_bbspst(bool isedit)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_EPST;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	unsigned long fid = 0;
	mmap_t m;
	struct fileheader fh;
	char *f = getparm("f");
	bool reply = !(*f == '\0');
	if (isedit && !reply)
		return BBS_EINVAL;
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (!bbscon_search(bp, fid, 0, &fh))
			return BBS_ENOFILE;
		if (!isedit && fh.accessed[0] & FILE_NOREPLY)
			return BBS_EPST;
		if (isedit && !chkBM(bp, &currentuser)
				&& strcmp(fh.owner, currentuser.userid))
			return BBS_EACCES;
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);
		m.oflag = O_RDONLY;
		if (mmap_open(file, &m) < 0)
			return BBS_ENOFILE;
	}
	
	xml_header("bbspst");
	char path[HOMELEN];
	snprintf(path, sizeof(path), BBSHOME"/upload/%s", bp->filename);
	printf("<bbspst ");
	print_session();
	printf(" brd='%s' bid='%d' edit='%d' att='%d'>", bp->filename, bid,
			isedit, dashd(path));
	if (reply) {
		printf("<t>");
		ansi_filter(fh.title, fh.title);
		xml_fputs2(fh.title, check_gbk(fh.title) - fh.title, stdout);
		printf("</t><po f='%u'>", fid);
		if (isedit) {
			char *begin = m.ptr, *end = (char *)(m.ptr) + m.size;
			get_post_body(&begin, &end);
			if (end > begin)
				xml_fputs2(begin, end - begin, stdout);
		} else {
			web_quotation(m.ptr, m.size, fh.owner, false);
		}
		mmap_close(&m);
		fputs("</po>", stdout);
	}
	printf("</bbspst>");
	return 0;
}

int bbspst_main(void)
{
	return do_bbspst(false);
}

int bbsedit_main(void)
{
	return do_bbspst(true);
}
