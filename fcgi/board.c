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
	if (!session_get_id())
		return BBS_ELGNREQ;

	xml_header(NULL);
	printf("<bbsfav>");
	print_session();

	db_res_t *res = db_query("SELECT b.id, b.name, b.descr FROM boards b"
			" JOIN fav_boards f ON b.id = f.board WHERE f.user_id = %d",
			session_get_user_id());
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
	if (!session_get_id())
		return BBS_ELGNREQ;

	int bid = strtol(web_get_param("bid"), NULL, 10);
	int ok = fav_board_add(session_get_user_id(), NULL, bid,
			FAV_BOARD_ROOT_FOLDER, &currentuser);
	if (ok) {
		xml_header(NULL);
		printf("<bbsbrdadd>");
		print_session();

		board_t board;
		get_board_by_bid(bid, &board);
		printf("<brd>%s</brd><bid>%d</bid></bbsbrdadd>", board.name, board.id);
		session_set_board(board.id);
		return 0;
	}
	return BBS_EBRDQE;
}

int web_sel(void)
{
	xml_header("bbssel");
	printf("<bbssel>");
	print_session();

	const char *brd = web_get_param("brd");
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

void board_to_object(const board_t *bp, json_object_t *o)
{
	json_object_bool(o, "dir", bp->flag & BOARD_FLAG_DIR);
	json_object_string(o, "name", bp->name);
	json_object_string(o, "categ", bp->categ);
	json_object_string(o, "descr", bp->descr);

	json_array_t *array = json_array_new();
	if (*bp->bms) {
		char bms[sizeof(bp->bms)];
		strlcpy(bms, bp->bms, sizeof(bms));
		for (const char *s = strtok(bms, " "); s; s = strtok(NULL, " ")) {
			json_array_string(array, s);
		}
	}
	json_object_append(o, "bms", array, JSON_ARRAY);
}

/*
[
	{
		dir: BOOL,
		name: TEXT,
		categ: TEXT,
		descr: TEXT,
		bms: [ TEXT, ... ]
	},
	...
]
 */
int api_board_all(void)
{
	db_res_t *res = db_query(BOARD_SELECT_QUERY_BASE);
	if (!res)
		return WEB_ERROR_INTERNAL;

	json_array_t *array = json_array_new();
	web_set_response(array, JSON_ARRAY);

	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		board_t board;
		res_to_board(res, i, &board);
		if (!has_read_perm(&board))
			continue;

		json_object_t *object = json_object_new();
		board_to_object(&board, object);
		json_array_append(array, object, JSON_OBJECT);
	}
	db_clear(res);
	return WEB_OK;
}

static json_object_t *attach_group(json_array_t *a, db_res_t *res, int id)
{
	json_object_t *object = json_object_new();
	json_array_append(a, object, JSON_OBJECT);

	json_array_t *array = json_array_new();
	json_object_append(object, "boards", array, JSON_ARRAY);

	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		int folder = db_get_integer(res, i, 2);
		if (folder == id) {
			json_object_t *o = json_object_new();
			json_array_append(array, o, JSON_OBJECT);

			int board_id = db_get_integer(res, i, 0);
			json_object_integer(o, "id", board_id);
			const char *name = db_get_value(res, i, 1);
			json_object_string(o, "name", name);
			json_object_bool(o, "unread",
					brc_board_unread(currentuser.userid, name, board_id));
		}
	}
	return object;
}

/*
[
	{
		name: OPTIONAL TEXT,
		descr: OPTIONAL TEXT,
		boards: [
			{ id: INTEGER, name: TEXT, unread: OPTIONAL BOOL },
			...
		]
	},
	...
]
*/
int api_board_fav(void)
{
	if (!session_get_id())
		return WEB_ERROR_LOGIN_REQUIRED;

	json_array_t *array = json_array_new();
	web_set_response(array, JSON_ARRAY);

	query_t *q = query_new(0);
	query_select(q, "board, name, folder");
	query_from(q, "fav_boards");
	query_where(q, "user_id = %"DBIdUID, session_get_user_id());
	db_res_t *boards = query_exec(q);

	q = query_new(0);
	query_select(q, "id, name, descr");
	query_from(q, "fav_board_folders");
	query_where(q, "user_id = %"DBIdUID, session_get_user_id());
	db_res_t *folders = query_exec(q);

	if (folders && boards) {
		attach_group(array, boards, FAV_BOARD_ROOT_FOLDER);
		for (int i = db_res_rows(folders) - 1; i >= 0; --i) {
			int id = db_get_integer(folders, i, 0);
			json_object_t *o = attach_group(array, boards, id);
			json_object_string(o, "name", db_get_value(folders, i, 1));
			json_object_string(o, "descr", db_get_value(folders, i, 2));
		}
	}

	db_clear(folders);
	db_clear(boards);
	return WEB_OK;
}

static int show_sector(int sid, db_res_t *res, int last)
{
	for (int i = last + 1; i < db_res_rows(res); ++i) {
		int sector = db_get_integer(res, i, 2);
		if (sector == sid) {
			last = i;

			const char *utf8_descr = db_get_value(res, i, 1);
			GBK_BUFFER(descr, BOARD_DESCR_CCHARS);
			if (!web_request_type(UTF8)) {
				convert_u2g(utf8_descr, gbk_descr);
			}
			printf("<brd name='%s' desc='%s'/>",
					db_get_value(res, i, 0),
					web_request_type(UTF8) ? utf8_descr : gbk_descr);
		}
	}
	return last;
}

int bbssec_main(void)
{
	db_res_t *r1 = db_query("SELECT id, name, descr, short_descr"
			" FROM board_sectors WHERE public ORDER BY name ASC");
	if (!r1)
		return BBS_EINVAL;

	db_res_t *res = db_query("SELECT b.name, b.descr, b.sector"
			" FROM boards b JOIN board_sectors s ON b.sector = s.id"
			" WHERE b.flag & %d <> 0", BOARD_FLAG_RECOMMEND);
	if (!res) {
		db_clear(r1);
		return BBS_EINVAL;
	}

	xml_header(NULL);
	printf("<bbssec>");
	print_session();

	int last = -1;
	for (int i = 0; i < db_res_rows(r1); ++i) {
		GBK_BUFFER(short_descr, 4);
		GBK_BUFFER(descr, 10);
		const char *short_descr = db_get_value(r1, i, 2),
			  *descr = db_get_value(r1, i, 3);
		if (!web_request_type(UTF8)) {
			convert_u2g(short_descr, gbk_short_descr);
			short_descr = gbk_short_descr;
			convert_u2g(descr, gbk_descr);
			descr = gbk_descr;
		}
		printf("<sec id='%s' desc='%s [%s]'>", db_get_value(r1, i, 1),
				short_descr, descr);
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
				(board.flag & BOARD_FLAG_DIR) ? 1 : 0, board.name, board.categ,
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
		if (!has_read_perm(&board))
			continue;
		if (!web_request_type(UTF8))
			board_to_gbk(&board);
		printf("<brd dir='%d' title='%s' cate='%.6s' desc='%s' bm='%s' "
				"read='%d' count='%d' />",
				(board.flag & BOARD_FLAG_DIR) ? 1 : 0, board.name,
				board.categ, board.descr, board.bms,
				brc_board_unread(currentuser.userid, board.name, board.id),
				filenum(board.name));
	}
}

extern const char *get_post_list_type_string(void);

int web_sector(void)
{
	int sid = 0;
	board_t parent = { .id = 0 };
	db_res_t *res = NULL;

	const char *sname = web_get_param("s");
	if (*sname) {
		res = db_query("SELECT id, descr"
				" FROM board_sectors WHERE name = %s", sname);
		if (!res || db_res_rows(res) < 1) {
			db_clear(res);
			return BBS_EINVAL;
		}
	} else {
		const char *pname = web_get_param("board");
		if (*pname)
			get_board(pname, &parent);
		else
			get_board_by_bid(strtol(web_get_param("bid"), NULL, 10), &parent);
		if (!parent.id || !(parent.flag & BOARD_FLAG_DIR)
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
		if (web_request_type(UTF8)) {
			printf(" title='%s'>", utf8_sector);
		} else {
			GBK_BUFFER(sector, BOARD_SECTOR_NAME_CCHARS);
			convert_u2g(utf8_sector, gbk_sector);
			printf(" title='%s'>", gbk_sector);
		}
		sid = db_get_integer(res, 0, 0);
		db_clear(res);
	} else {
		if (web_request_type(UTF8)) {
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
	if (!session_get_id())
		return BBS_ELGNREQ;

	board_t board;
	if (!get_board(web_get_param("board"), &board)
			|| !has_read_perm(&board))
		return BBS_ENOBRD;
	session_set_board(board.id);

	const char *start = web_get_param("start");
	brc_init(currentuser.userid, board.name);
	brc_clear_all();
	brc_sync(currentuser.userid);
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
	if (!get_board(web_get_param("board"), &board)
			|| !has_read_perm(&board))
		return BBS_ENOBRD;

	if (board.flag & BOARD_FLAG_DIR)
		return BBS_EINVAL;
	session_set_board(board.id);

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
