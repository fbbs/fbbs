#include "libweb.h"

bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp);
int post_article(const struct userec *user, const struct boardheader *bp, 
		const char *title, const char *content, bool cross, const char *ip, 
		const struct fileheader *o_fp);

int bbsccc_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	unsigned int fid = strtoul(getparm("f"), NULL, 10);
	struct fileheader fh;
	if (!bbscon_search(bp, fid, 0, &fh))
		return BBS_ENOFILE;

	char *target = getparm("t");
	if (*target != '\0') {
		struct boardheader *bp2 = getbcache(target);
		if (bp2 == NULL)
			return BBS_ENOBRD;
		if (bp2->flag & BOARD_DIR_FLAG || bp2 == bp)
			return BBS_EINVAL;
		if (!haspostperm(&currentuser, bp2))
			return BBS_EPST;
		mmap_t m;
		m.oflag = O_RDONLY;
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);

		char title[sizeof(fh.title)];
		if (strncmp(fh.title, "[зЊди]", sizeof("[зЊди]") - 1) == 0)
			strlcpy(title, fh.title, sizeof(title));
		else
			snprintf(title, sizeof(title), "[зЊди]%s", fh.title);
		if (mmap_open(file, &m) < 0)
			return BBS_EINTNL;
		int ret = post_article(&currentuser, bp2, title, m.ptr, true,
				fromhost, NULL);
		mmap_close(&m);
		if (ret < 0)
			return BBS_EINTNL;
		xml_header("bbsccc");
		printf("<bbsccc %s t='%d' b='%d'/>", get_session_str(),
				bp2 - bcache + 1, bp - bcache + 1);
	} else {
		xml_header("bbsccc");
		printf("<bbsccc %s owner='%s' brd='%s' bid='%d' fid='%u'>", 
				get_session_str(), fh.owner, bp->filename,
				bp - bcache + 1, fid);
		xml_fputs(fh.title, stdout);
		printf("</bbsccc>");
	}
	return 0;
}
