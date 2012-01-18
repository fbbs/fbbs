#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

int web_fav(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	xml_header(NULL);
	printf("<bbsfav>");
	print_session();

	db_res_t *res = db_exec_query(env.d, true, "SELECT b.id, b.name, b.descr"
			" FROM boards b JOIN fav_boards f WHERE b.id = f.board");
	if (res) {
		for (int i = 0; i < db_res_rows(res); ++i) {
			int bid = db_get_integer(res, i, 0);
			const char *name = db_get_value(res, i, 1);

			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			convert_u2g(db_get_value(res, i, 2), gbk_descr);

			printf("<brd bid='%d' brd='%s'", bid, name);
			xml_fputs(gbk_descr, stdout);
			printf("</brd>");
		}
	}
	db_clear(res);

	printf("</bbsfav>");
	return 0;
}

int web_brdadd(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	int bid = strtol(get_param("bid"), NULL, 10);
	int ok = fav_board_add(session.uid, bid, FAV_BOARD_ROOT_FOLDER);
	if (ok) {
		xml_header(NULL);
		printf("<bbsbrdadd>");
		print_session();

		board_t board;
		get_board_by_bid(bid, &board);
		printf("<brd>%s</brd><bid>%d</bid></bbsbrdadd>", board.name, board.id);
		return 0;
	}
	return BBS_EBRDQE;
}
