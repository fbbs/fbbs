#include "libweb.h"

int bbsanc_main()
{
	char *path = getparm("path");
	if (strstr(path, "bbslist") || strstr(path, ".Search")
			|| strstr(path, ".Names") || strstr(path, "..")
			|| strstr(path, "SYSHome"))
		return BBS_EINVAL;
	char *board = getbfroma(path);
	struct boardheader *bp = NULL;
	if (*board != '\0') {
		bp = getbcache(board);
		if (!hasreadperm(&currentuser, bp))
			return BBS_ENOFILE;
	}

	char fname[512];
	sprintf(fname, "0Announce%s", path);
	xml_header("bbsanc");
	printf("<bbsanc p='%s' u='%s'", get_permission(), currentuser.userid);
	if (bp != NULL)
		printf(" brd='%s'", bp->filename);
	printf(">");
	xml_printfile(fname, stdout);
	printf("</bbsanc>");
	return 0;
}
