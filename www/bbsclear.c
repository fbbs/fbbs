#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/session.h"
#include "fbbs/web.h"

int bbsclear_main(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board(get_param("board"), &board)
			|| !has_read_perm(&currentuser, &board))
		return BBS_ENOBRD;

	const char *start = get_param("start");
	brc_fcgi_init(currentuser.userid, board.name);
	brc_clear_all(board.id);
	brc_update(currentuser.userid, board.name);
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "doc?board=%s&start=%s", board.name, start);
	http_header();
	refreshto(0, buf);
	printf("</head></html>");
	return 0;
}
