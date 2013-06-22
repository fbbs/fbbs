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
	if (!session.id)
		return BBS_ELGNREQ;

	xml_header(NULL);
	printf("<bbsfav>");
	print_session();

	db_res_t *res = db_query("SELECT b.id, b.name, b.descr FROM boards b"
			" JOIN fav_boards f ON b.id = f.board WHERE f.user_id = %d",
			session.uid);
	if (res) {
		for (int i = 0; i < db_res_rows(res); ++i) {
			int bid = db_get_integer(res, i, 0);
			const char *name = db_get_value(res, i, 1);

			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			convert_u2g(db_get_value(res, i, 2), gbk_descr);

			printf("<brd bid='%d' brd='%s'>", bid, name);
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

static void board_to_node(const board_t *bp, xml_node_t *node)
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
		if (!has_read_perm(&currentuser, &board))
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
	if (!session.id)
		return error_msg(ERROR_LOGIN_REQUIRED);

	xml_node_t *root = set_response_root("bbs-board-fav",
			XML_NODE_ANONYMOUS_JSON, XML_ENCODING_UTF8);
	xml_node_t *groups = xml_new_node("groups", XML_NODE_CHILD_ARRAY);
	xml_add_child(root, groups);

	query_t *q = query_new(0);
	query_select(q, "board, name, folder");
	query_from(q, "fav_boards");
	query_where(q, "user_id = %"DBIdUID, session.uid);
	db_res_t *boards = query_exec(q);

	q = query_new(0);
	query_select(q, "id, name, descr");
	query_from(q, "fav_board_folders");
	query_where(q, "user_id = %"DBIdUID, session.uid);
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

static int get_board_by_param(board_t *bp)
{
	int bid = strtol(get_param("bid"), NULL, 10);
	if (bid > 0 && get_board_by_bid(bid, bp) > 0)
		return bp->id;

	const char *bname = get_param("board");
	if (bname && *bname)
		return get_board(bname, bp);
	return 0;
}

static xml_node_t *create_post_node(const post_info_t *p)
{
	xml_node_t *post = xml_new_node("post", XML_NODE_ANONYMOUS_JSON);
	xml_attr_bigint(post, "id", p->id);
	xml_attr_bigint(post, "reid", p->reid);
	xml_attr_bigint(post, "tid", p->tid);
	xml_attr_string(post, "owner", p->owner, false);
	xml_attr_string(post, "stamp", format_time(p->stamp, TIME_FORMAT_XML), false);
	xml_attr_string(post, "title", p->utf8_title, true);
	char mark[2] = "\0";
	mark[0] = get_post_mark(p);
	xml_attr_string(post, "mark", mark, false);
	return post;
}

static int print_toc(xml_node_t *posts, db_res_t *res, int archive, bool asc)
{
	int rows = db_res_rows(res);
	for (int i = asc ? rows - 1 : 0;
			asc ? i >= 0 : i < rows;
			i += asc ? -1 : 1) {
		post_info_t p;
		res_to_post_info(res, i, archive, &p);
		xml_node_t *post = create_post_node(&p);
		xml_add_child(posts, post);
	}
	return rows;
}

static void print_toc_sticky(xml_node_t *root, int bid)
{
	xml_node_t *stickies = xml_new_child(root, "stickies",
			XML_NODE_CHILD_ARRAY);
	db_res_t *res = db_query("SELECT " POST_LIST_FIELDS " FROM posts.recent"
			" WHERE board = %d AND sticky ORDER BY id DESC", bid);
	print_toc(stickies, res, 0, false);
	db_clear(res);
}

enum {
	API_BOARD_TOC_LIMIT_DEFAULT = 20,
	API_BOARD_TOC_LIMIT_MAX = 50,
};

int api_board_toc(void)
{
	board_t board;
	if (get_board_by_param(&board) <= 0)
		return error_msg(ERROR_BOARD_NOT_FOUND);
	brc_initialize(currentuser.userid, board.name);

	bool asc = streq(get_param("page"), "next");
	post_id_t start = strtoll(get_param("start"), NULL, 10);

	int limit = strtol(get_param("limit"), NULL, 10);
	if (limit > API_BOARD_TOC_LIMIT_MAX)
		limit = API_BOARD_TOC_LIMIT_MAX;
	if (limit < 0)
		return error_msg(ERROR_BAD_REQUEST);
	if (!limit)
		limit = API_BOARD_TOC_LIMIT_DEFAULT;

	int flag = *get_param("flag");

	xml_node_t *root = set_response_root("bbs-board-toc",
			XML_NODE_ANONYMOUS_JSON, XML_ENCODING_UTF8);
	xml_node_t *posts = xml_new_child(root, "posts", XML_NODE_CHILD_ARRAY);

	post_filter_t filter = {
		.bid = board.id, .flag = flag,
		.min = asc ? start : 0, .max = asc ? 0 : start,
	};
	query_t *q = build_post_query(&filter, asc, limit);
	db_res_t *res = query_exec(q);
	print_toc(posts, res, filter.archive, asc);
	db_clear(res);

	if (!start && !asc && !flag && board.id)
		print_toc_sticky(root, board.id);

	xml_node_t *bnode = xml_new_child(root, "board", 0);
	board_to_node(&board, bnode);

	return HTTP_OK;
}
