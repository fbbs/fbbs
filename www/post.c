#include <stdlib.h>
#include "fbbs/web.h"

int bbs_post(web_ctx_t *ctx)
{
	const char *idstr = get_param(ctx->r, "id");
	long id = strtol(idstr, NULL, 10);

	char file[128];
	snprintf(file, sizeof(file), "posts/%ld/%ld", id / 10000, id);

	const char *query = "SELECT board_id, gid FROM posts WHERE id = $1";
	db_param_t params[] = { PARAM_TEXT(idstr) };
	db_res_t *res = db_exec_params(ctx->d, query, 1, params, true);
	if (db_res_status(res) != DBRES_TUPLES_OK || db_num_rows(res) <= 0) {
		db_clear(res);
		return -1;
	}

	xml_header(NULL);
	printf("<bbs_post bid='%d' gid='%ld'>", 
			db_get_integer(res, 0, 0),
			db_get_bigint(res, 0, 1));

	printf("<post id='%ld'>", id);
	xml_print_post(file, true);
	printf("</post>");

	printf("</bbs_post>");

	db_clear(res);
	return 0;
}
