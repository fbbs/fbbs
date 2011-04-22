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

static int _user_data_add(db_conn_t *c, const char *name, int uid,
		int field, int delta)
{
	const char *fields[] = { "logins", "posts", "stay" };
	if (field < 0 || field >= NELEMS(fields))
		return -1;

	char query[128];
	int bytes = snprintf(query, sizeof(query),
		"UPDATE users SET %s = %s + %d WHERE %s", fields[field],
		fields[field], delta, name ? "lower(name) = $1" : "id = $1");
	if (bytes >= sizeof(query))
		return -1;

	const char *param = name;
	char buf[16];
	if (!name) {
		param = buf;
		snprintf(buf, sizeof(buf), "%d", uid);
	}

	db_param_t params[1] = { PARAM_TEXT(param) };
	db_res_t *res = db_exec_params(c, query, 1, params, true);

	int ret = 0;
	if (db_res_status(res) != DBRES_COMMAND_OK)
		ret = -1;
	db_clear(res);
	return ret;
}

int user_data_add_by_name(db_conn_t *c, const char *name, int field, int delta)
{
	return _user_data_add(c, name, 0, field, delta);
}

int user_data_add(db_conn_t *c, int uid, int field, int delta)
{
	return _user_data_add(c, NULL, uid, field, delta);
}
