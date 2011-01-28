#include "fbbs/dbi.h"

int get_user_id(db_conn_t *c, const char *name)
{
	db_param_t param[1] = { PARAM_TEXT(name) };
	db_res_t *res = db_exec_params(c, "SELECT id FROM users WHERE name = $1",
			1, param, true);
	if (db_res_status(res) != DBRES_TUPLES_OK)
		return -1;
	if (db_num_rows(res) > 0)
		return db_get_integer(res, 0, 0);
	return 0;
}

