#include "fbbs/dbi.h"

/**
 * Get user id by name.
 * @param c The database connection.
 * @param name The user name.
 * @return user id on success, 0 if not exist, -1 on error.
 */
int get_user_id(db_conn_t *c, const char *name)
{
	db_param_t param[1] = { PARAM_TEXT(name) };
	db_res_t *res = db_exec_params(c,
			"SELECT id FROM users WHERE lower(name) = $1",
			1, param, true);
	if (db_res_status(res) != DBRES_TUPLES_OK) {
		db_clear(res);
		return -1;
	}

	int ret = 0;
	if (db_num_rows(res) > 0)
		ret = db_get_integer(res, 0, 0);
	db_clear(res);
	return ret;
}
