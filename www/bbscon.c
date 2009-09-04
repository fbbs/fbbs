#include "libweb.h"

int bbscon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	unsigned int fid = strtoul(getparm("f"), NULL, 10);
	char *action = getparm("a");

	struct fileheader fh;
	if (!bbscon_search(bp, fid, *action, &fh))
		return BBS_ENOFILE;
	fid = fh.id;

	xml_header("bbscon");
	printf("<root><bbscon p='%s' u='%s' link='con' bid='%d'>",
			get_permission(), currentuser.userid, bid);
	printf("<po fid='%u'", fid);
	if (fh.reid != fh.id)
		printf(" reid='%u' gid='%u'>", fh.reid, fh.gid);
	else
		printf(">");
	char file[HOMELEN];
	setbfile(file, bp->filename, fh.filename);
	xml_printfile(file, stdout);
	printf("</po></bbscon></root>");
	return 0;
}

int bbsgcon_main(void)
{
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;
	char *f = getparm("f");
	if (strstr(f, "..") || strstr(f, "/") || strncmp(f, "G.", 2))
		return BBS_EINVAL;
	xml_header("bbscon");
	printf("<root><bbscon p='%s' u='%s' link='gcon' bid='%d'><po>",
			get_permission(), currentuser.userid, bid);
	char file[HOMELEN];
	setbfile(file, bp->filename, f);
	xml_printfile(file, stdout);
	printf("</po></bbscon></root>", bid);
	return 0;
}
