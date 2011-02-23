#include "libweb.h"
#include "fbbs/web.h"

int bbsclear_main(web_ctx_t *ctx)
{
	if (!loginok)
		return BBS_ELGNREQ;
	const char *board = get_param(ctx->r, "board");
	struct boardheader *bp = getbcache(board);
	if (bp == NULL || !hasreadperm(&currentuser, bp))
		return BBS_ENOBRD;
	const char *start = get_param(ctx->r, "start");
	brc_fcgi_init(currentuser.userid, board);
	brc_clear(NA, NULL, YEA);
	brc_update(currentuser.userid, board);
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "doc?board=%s&start=%s", board, start);
	http_header();
	refreshto(0, buf);
	printf("</head></html>");
	return 0;
}
