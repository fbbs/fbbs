#include "libweb.h"

extern bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp);

/**
 * Forward post.
 * @return 0 on success, bbserrno on error.
 */
// fwd?bid=[bid]&f=[fid]&u=[recipient]
int bbsfwd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	parse_post_data();
	char *reci = getparm("u");
	if (*reci == '\0') {
		xml_header("bbs");
		printf("<bbsfwd bid='%s' f='%s' ", getparm("bid"), getparm("f"));
		print_session();
		printf("></bbsfwd>");
	} else {
		if (!HAS_PERM(PERM_MAIL))
			return BBS_EACCES;
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
		char file[HOMELEN];
		setbfile(file, bp->filename, fh.filename);
		char title[STRLEN];
		snprintf(title, sizeof(title), "[转寄]%s", fh.title);
		int ret = mail_file(file, reci, title);
		if (ret)
			return ret;
		http_header();
		printf("</head><body><p>文章转寄成功</p>"
				"<a href='javascript:history.go(-2)'>返回</a></body></html>");
	}
	return 0;
}
