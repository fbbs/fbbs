#include "fbbs/dbi.h"
#include "fbbs/post.h"
#include "fbbs/time.h"
#include "fbbs/web.h"

enum {
	QUERY_LENGTH = 512,
};

bool allow_reply(uint_t flag)
{
	return !(flag & FILE_NOREPLY);
}

int get_post_mark(uint_t flag)
{
	int mark = ' ';
	if (flag & FILE_DIGEST)
		mark = 'G';
	if (flag & FILE_MARKED) {
		if (mark == ' ')
			mark = 'M';
		else
			mark = 'B';
	}
	if ((flag & FILE_WATER) && mark == ' ')
		mark = 'W';
	return mark;
}

int bbs_board(web_ctx_t *ctx)
{
	const char *bid = get_param(ctx->r, "bid");
	const char *action = get_param(ctx->r, "a");
	const char *pid = get_param(ctx->r, "pid");

	bool asc = (*action == 'n');
	bool act = (*action == 'n' || *action == 'p');

	char *query = pool_alloc(ctx->p, QUERY_LENGTH);
	int len = snprintf(query, QUERY_LENGTH,
			"SELECT u.name, p.flag, p.time, p.user_name, p.id, p.title "
			"FROM posts p LEFT JOIN users u ON p.user_id = u.id "
			"WHERE p.board_id = $1 AND itype = 0 %s "
			"ORDER BY p.id %s LIMIT 20",
			asc ? "AND p.id > $2" : (*action == 'p' ? "AND p.id < $2" : ""),
			asc ? "ASC" : "DESC");
	if (len >= QUERY_LENGTH)
		return -1;

	db_param_t param[] = { PARAM_TEXT(bid), PARAM_TEXT(pid) };
	db_res_t *res = db_exec_params(ctx->d, query, act ? 2 : 1, param, true);
	if (db_res_status(res) != DBRES_TUPLES_OK) {
		db_clear(res);
		return -1;
	}

	xml_header(NULL);
	printf("<bbs_board>");

	int rows = db_num_rows(res);
	long begin = 0, end = 0;
	if (rows > 0) {
		int e = asc ? rows : -1;
		int step = asc ? 1 : -1;
		for (int i = asc ? 0 : rows - 1; i != e; i += step) {
			uint_t flag = db_get_integer(res, i, 1);
			printf("<po %sm='%c' o='%s' t='%s' id='%ld'>",
					allow_reply(flag) ? "" : "nore='1' ",
					get_post_mark(flag),
					db_get_value(res, i, 3),
					date2str(db_get_time(res, i, 2), DATE_XML),
					db_get_bigint(res, i, 4));
			xml_print(db_get_value(res, i, 5));
			printf("</po>\n");
		}
		begin = db_get_bigint(res, asc ? 0 : rows - 1, 4);
		end = db_get_bigint(res, asc ? rows - 1 : 0, 4);
	}
	db_clear(res);

	const char *q = "SELECT id, name, description FROM boards WHERE id = $1";
	res = db_exec_params(ctx->d, q, 1, param, true);
	if (db_res_status(res) != DBRES_TUPLES_OK) {
		db_clear(res);
		return -1;
	}
	printf("<brd bid='%d' name='%s' desc='%s' begin='%ld' end='%ld'/>",
			db_get_integer(res, 0, 0),
			db_get_value(res, 0, 1),
			db_get_value(res, 0, 2),
			begin, end);

	printf("</bbs_board>");

	db_clear(res);
	return 0;
}
