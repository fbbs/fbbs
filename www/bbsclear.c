#include "libweb.h"

int bbsclear_main(void)
{
	if (!loginok)
		http_fatal("匆匆过客无法执行此项操作, 请先登录");
	const char *board = getparm("board");
	struct boardheader *bp = getbcache(board);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		http_fatal2(HTTP_STATUS_NOTFOUND, "错误的讨论区");
	const char *start = getparm("start");
	brc_initial(currentuser.userid, board);
	brc_clear(NA, NULL, YEA);
	brc_update(currentuser.userid, board);
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "bbsdoc?board=%s&start=%s", board, start);
	http_header();
	refreshto(0, buf);
	printf("</head></html>");
	return 0;
}
