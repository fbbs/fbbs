// Lists all boards.

#include "libweb.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/web.h"

int web_all_boards(void)
{
	xml_header(NULL);
	printf("<bbsall>");
	print_session();

	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	if (!res)
		return BBS_EINTNL;
	for (int i = 0; i < db_res_rows(res); ++i) {
		board_t board;
		res_to_board(res, i, &board);
		if (!has_read_perm(&currentuser, &board))
			continue;
		board_to_gbk(&board);
		printf("<brd dir='%d' title='%s' cate='%s' desc='%s' bm='%s' />",
				(board.flag & BOARD_DIR_FLAG) ? 1 : 0, board.name, board.categ,
				board.descr, board.bms);
	}
	db_clear(res);

	printf("</bbsall>");
	return 0;
}

// TODO: unify with telnet
static int filenum(char *board)
{
	char file[HOMELEN];
	struct stat st;
	sprintf(file, "boards/%s/.DIR", board);
	if (stat(file, &st) < 0)
		return 0;
	return st.st_size / sizeof(struct fileheader);
}

static void show_board(db_res_t *res)
{
	for (int i = 0; i < db_res_rows(res); ++i) {
		board_t board;
		res_to_board(res, i, &board);
		board_to_gbk(&board);
		printf("<brd dir='%d' title='%s' cate='%.6s' desc='%s' bm='%s' "
				"read='%d' count='%d' />",
				(board.flag & BOARD_DIR_FLAG) ? 1 : 0, board.name,
				board.categ, board.descr, board.bms,
				brc_board_unread(currentuser.userid, board.name, board.id),
				filenum(board.name));
	}
}

int web_sector(void)
{
	int sid = 0;
	board_t parent;
	db_res_t *res = NULL;

	const char *sname = get_param("s");
	if (*sname) {
		res = db_query("SELECT id, descr"
				" FROM board_sectors WHERE name = %s", sname);
		if (!res || db_res_rows(res) < 1) {
			db_clear(res);
			return BBS_EINVAL;
		}
	} else {
		const char *pname = get_param("board");
		if (*pname)
			get_board(pname, &parent);
		else
			get_board_by_bid(strtol(get_param("bid"), NULL, 10), &parent);
		if (!parent.id || !(parent.flag & BOARD_DIR_FLAG)
				|| !has_read_perm(&currentuser, &parent))
			return BBS_ENOBRD;
	}

	xml_header(NULL);
	printf("<bbsboa link='%sdoc' ", get_doc_mode_str());

	if (*sname) {
		char path[HOMELEN];
		sprintf(path, "%s/info/egroup%d/icon.jpg", BBSHOME,
				(int) strtol(sname, NULL, 16));
		if (dashf(path))
			printf(" icon='%s'", path);
		
		GBK_BUFFER(sector, BOARD_SECTOR_NAME_CCHARS);
		convert_u2g(db_get_value(res, 0, 1), gbk_sector);
		printf(" title='%s'>", gbk_sector);

		sid = db_get_integer(res, 0, 0);
		db_clear(res);
	} else {
		GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
		convert_u2g(parent.descr, gbk_descr);
		printf(" dir= '1' title='%s'>", gbk_descr);
	}

	if (sid)
		res = db_query(BOARD_SELECT_QUERY_BASE "WHERE b.sector = %d", sid);
	else
		res = db_query(BOARD_SELECT_QUERY_BASE "WHERE b.parent = %d", parent.id);

	if (res && db_res_rows(res) > 0)
		show_board(res);
	db_clear(res);

	print_session();
	printf("</bbsboa>");
	return 0;
}

