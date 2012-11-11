#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

int web_fav(void)
{
	if (!session.id)
		return BBS_ELGNREQ;

	xml_header(NULL);
	printf("<bbsfav>");
	print_session();

	db_res_t *res = db_query("SELECT b.id, b.name, b.descr"
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
	if (!session.id)
		return BBS_ELGNREQ;

	int bid = strtol(get_param("bid"), NULL, 10);
	int ok = fav_board_add(session.uid, NULL, bid,
			FAV_BOARD_ROOT_FOLDER, &currentuser);
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

int web_sel(void)
{
	xml_header("bbssel");
	printf("<bbssel>");
	print_session();

	const char *brd = get_param("brd");
	if (*brd != '\0') {
		char name[BOARD_NAME_LEN + 3];
		snprintf(name, sizeof(name), "%%%s%%", brd);

		db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE
				"WHERE lower(b.name) LIKE %s", name);
		if (res && db_res_rows(res) > 0) {
			board_t board;
			for (int i = 0; i < db_res_rows(res); ++i) {
				res_to_board(res, i, &board);
				if (has_read_perm(&currentuser, &board)) {
					board_to_gbk(&board);
					printf("<brd dir='%d' title='%s' desc='%s' />",
							is_board_dir(&board), board.name, board.descr);
				}
			}
		} else {
			printf("<notfound/>");
		}
		db_clear(res);
	}
	printf("</bbssel>");
	return 0;
}

int api_board_all(void)
{
	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	if (!res)
		return error_msg(ERROR_INTERNAL);

	xml_node_t *root = set_response_root("bbs-board-all",
			XML_NODE_ANONYMOUS_JSON | XML_NODE_CHILD_ARRAY,
			XML_ENCODING_UTF8);

	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		board_t board;
		res_to_board(res, i, &board);
		if (!has_read_perm(&currentuser, &board))
			continue;

		xml_node_t *node = xml_new_node("board", XML_NODE_ANONYMOUS_JSON);
		xml_attr_boolean(node, "dir", board.flag & BOARD_DIR_FLAG);
		xml_attr_string(node, "name", board.name, false);
		xml_attr_string(node, "categ", board.categ, false);
		xml_attr_string(node, "descr", board.descr, false);
		xml_attr_string(node, "bms", board.bms, false);

		xml_add_child(root, node);
	}
	db_clear(res);
	return HTTP_OK;
}
