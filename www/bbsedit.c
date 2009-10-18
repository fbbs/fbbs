#include "libweb.h"

int bbsedit_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_EPST;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	char *f = getparm("f");
	if (*f == '\0')
		return BBS_EINVAL;
	unsigned long fid = strtoul(f, NULL, 10);
	struct fileheader fh;
	if (!bbscon_search(bp, fid, 0, &fh))
			return BBS_ENOFILE;
	if (!chkBM(bp, &currentuser) && strcmp(currentuser.userid, fh.owner))
		return BBS_EACCES;
	
}
