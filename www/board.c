#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

int web_fav(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	xml_header(NULL);
	printf("<bbsfav>");
	print_session();

	db_res_t *res = db_query("SELECT b.id, b.name, b.descr FROM boards b"
			" JOIN fav_boards f ON b.id = f.board WHERE f.user_id = %d",
			session_uid());
	if (res) {
		for (int i = 0; i < db_res_rows(res); ++i) {
			int bid = db_get_integer(res, i, 0);
			const char *name = db_get_value(res, i, 1);

			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			convert_u2g(db_get_value(res, i, 2), gbk_descr);

			printf("<brd bid='%d' brd='%s'>", bid, name);
			xml_fputs(gbk_descr);
			printf("</brd>");
		}
	}
	db_clear(res);

	printf("</bbsfav>");
	return 0;
}

int web_brdadd(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	int bid = strtol(get_param("bid"), NULL, 10);
	int ok = fav_board_add(session_uid(), NULL, bid,
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
				if (has_read_perm(&board)) {
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

void board_to_node(const board_t *bp, xml_node_t *node)
{
	xml_attr_boolean(node, "dir", bp->flag & BOARD_DIR_FLAG);
	xml_attr_string(node, "name", bp->name, false);
	xml_attr_string(node, "categ", bp->categ, false);
	xml_attr_string(node, "descr", bp->descr, false);

	xml_node_t *b = xml_new_child(node, "bms", XML_NODE_CHILD_ARRAY);
	if (*bp->bms) {
		char bms[sizeof(bp->bms)];
		strlcpy(bms, bp->bms, sizeof(bms));
		for (const char *s = strtok(bms, " "); s; s = strtok(NULL, " ")) {
			xml_node_t *t = xml_new_child(b, "name",
					XML_NODE_ANONYMOUS_JSON | XML_NODE_PLAIN_JSON);
			xml_attr_string(t, NULL, s, true);
		}
	}
}

int api_board_all(void)
{
	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	if (!res)
		return error_msg(ERROR_INTERNAL);

	xml_node_t *root = set_response_root("bbs-board-all",
			XML_NODE_ANONYMOUS_JSON, XML_ENCODING_UTF8);
	xml_node_t *boards = xml_new_node("boards", XML_NODE_CHILD_ARRAY);
	xml_add_child(root, boards);

	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		board_t board;
		res_to_board(res, i, &board);
		if (!has_read_perm(&board))
			continue;

		xml_node_t *node = xml_new_node("board", XML_NODE_ANONYMOUS_JSON);
		board_to_node(&board, node);
		xml_add_child(boards, node);
	}
	db_clear(res);
	return HTTP_OK;
}

static xml_node_t *attach_group(xml_node_t *groups, db_res_t *res, int id)
{
	xml_node_t *group = xml_new_child(groups, "group",
			XML_NODE_ANONYMOUS_JSON);
	xml_node_t *boards = xml_new_child(group, "boards", XML_NODE_CHILD_ARRAY);
	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		int folder = db_get_integer(res, i, 2);
		if (folder == id) {
			xml_node_t *board = xml_new_child(boards, "board",
					XML_NODE_ANONYMOUS_JSON);
			int bid = db_get_integer(res, i, 0);
			xml_attr_integer(board, "id", bid);
			const char *name = db_get_value(res, i, 1);
			xml_attr_string(board, "name", name, false);
			xml_attr_boolean(board, "unread",
					brc_board_unread(currentuser.userid, name, bid));
		}
	}
	return group;
}

/*
{
	groups: [
		{
			name: OPTIONAL TEXT,
			descr: OPTIONAL TEXT,
			boards: [
				{ id: INTEGER, name: TEXT, unread: OPTIONAL BOOLEAN },
				...
			]
		},
		...
	]
}
*/
int api_board_fav(void)
{
	if (!session_id())
		return error_msg(ERROR_LOGIN_REQUIRED);

	xml_node_t *root = set_response_root("bbs-board-fav",
			XML_NODE_ANONYMOUS_JSON, XML_ENCODING_UTF8);
	xml_node_t *groups = xml_new_node("groups", XML_NODE_CHILD_ARRAY);
	xml_add_child(root, groups);

	query_t *q = query_new(0);
	query_select(q, "board, name, folder");
	query_from(q, "fav_boards");
	query_where(q, "user_id = %"DBIdUID, session_uid());
	db_res_t *boards = query_exec(q);

	q = query_new(0);
	query_select(q, "id, name, descr");
	query_from(q, "fav_board_folders");
	query_where(q, "user_id = %"DBIdUID, session_uid());
	db_res_t *folders = query_exec(q);

	if (folders && boards) {
		attach_group(groups, boards, FAV_BOARD_ROOT_FOLDER);
		for (int i = db_res_rows(folders) - 1; i >= 0; --i) {
			int id = db_get_integer(folders, i, 0);
			xml_node_t *group = attach_group(groups, boards, id);
			xml_attr_string(group, "name", db_get_value(folders, i, 1), true);
			xml_attr_string(group, "descr", db_get_value(folders, i, 2), true);
		}
	}

	db_clear(folders);
	db_clear(boards);
	return HTTP_OK;
}

static int show_sector(int sid, db_res_t *res, int last)
{
	for (int i = last + 1; i < db_res_rows(res); ++i) {
		int sector = db_get_integer(res, i, 2);
		if (sector == sid) {
			last = i;

			const char *utf8_descr = db_get_value(res, i, 1);
			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			if (!request_type(REQUEST_UTF8)) {
				convert_u2g(utf8_descr, gbk_descr);
			}
			printf("<brd name='%s' desc='%s'/>",
					db_get_value(res, i, 0),
					request_type(REQUEST_UTF8) ? utf8_descr : gbk_descr);
		}
	}
	return last;
}

int bbssec_main(void)
{
	db_res_t *r1 = db_query("SELECT id, name, descr, short_descr"
			" FROM board_sectors ORDER BY name ASC");
	if (!r1)
		return BBS_EINVAL;

	db_res_t *res = db_query("SELECT b.name, b.descr, b.sector"
			" FROM boards b JOIN board_sectors s ON b.sector = s.id"
			" WHERE b.flag & %d <> 0", BOARD_RECOMMEND_FLAG);
	if (!res) {
		db_clear(r1);
		return BBS_EINVAL;
	}

	xml_header(NULL);
	printf("<bbssec>");
	print_session();

	int last = -1;
	for (int i = 0; i < db_res_rows(r1); ++i) {
		printf("<sec id='%s' desc='%s [%s]'>", db_get_value(r1, i, 1),
				db_get_value(r1, i, 2), db_get_value(r1, i, 3));
		int sid = db_get_integer(r1, i, 0);
		last = show_sector(sid, res, last);
		printf("</sec>");
	}

	db_clear(res);
	db_clear(r1);

	printf("</bbssec>");
	return 0;
}

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
		if (!has_read_perm(&board))
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
		if (!request_type(REQUEST_UTF8))
			board_to_gbk(&board);
		printf("<brd dir='%d' title='%s' cate='%.6s' desc='%s' bm='%s' "
				"read='%d' count='%d' />",
				(board.flag & BOARD_DIR_FLAG) ? 1 : 0, board.name,
				board.categ, board.descr, board.bms,
				brc_board_unread(currentuser.userid, board.name, board.id),
				filenum(board.name));
	}
}

extern const char *get_post_list_type_string(void);

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
				|| !has_read_perm(&parent))
			return BBS_ENOBRD;
	}

	xml_header(NULL);
	printf("<bbsboa link='%sdoc' ", get_post_list_type_string());

	if (*sname) {
		char path[HOMELEN];
		sprintf(path, "%s/info/egroup%d/icon.jpg", BBSHOME,
				(int) strtol(sname, NULL, 16));
		if (dashf(path))
			printf(" icon='%s'", path);
		
		const char *utf8_sector = db_get_value(res, 0, 1);
		if (request_type(REQUEST_UTF8)) {
			printf(" title='%s'>", utf8_sector);
		} else {
			GBK_BUFFER(sector, BOARD_SECTOR_NAME_CCHARS);
			convert_u2g(utf8_sector, gbk_sector);
			printf(" title='%s'>", gbk_sector);
		}
		sid = db_get_integer(res, 0, 0);
		db_clear(res);
	} else {
		if (request_type(REQUEST_UTF8)) {
			printf(" dir= '1' title='%s'>", parent.descr);
		} else {
			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			convert_u2g(parent.descr, gbk_descr);
			printf(" dir= '1' title='%s'>", gbk_descr);
		}
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

int bbsclear_main(void)
{
	if (!session_id())
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board(get_param("board"), &board)
			|| !has_read_perm(&board))
		return BBS_ENOBRD;

	const char *start = get_param("start");
	brc_initialize(currentuser.userid, board.name);
	brc_clear_all();
	brc_update(currentuser.userid, board.name);
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "doc?board=%s&start=%s", board.name, start);
	http_header();
	refreshto(0, buf);
	printf("</head></html>");
	return 0;
}

int bbsnot_main(void)
{
	board_t board;
	if (!get_board(get_param("board"), &board)
			|| !has_read_perm(&board))
		return BBS_ENOBRD;

	if (board.flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	char fname[HOMELEN];
	snprintf(fname, sizeof(fname), "vote/%s/notes", board.name);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(fname, &m) < 0)
		return BBS_ENOFILE;
	xml_header(NULL);
	printf("<bbsnot brd='%s'>", board.name);
	xml_fputs2((char *) m.ptr, m.size);
	mmap_close(&m);
	print_session();
	printf("</bbsnot>");
	return 0;
}
