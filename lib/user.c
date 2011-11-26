#include "fbbs/fbbs.h"

/**
 * Get user id by name.
 * @param name The user name.
 * @return user id on success, 0 if not exist, -1 on error.
 */
user_id_t get_user_id(const char *name)
{
	db_res_t *res = db_exec_query(env.d, true,
			"SELECT id FROM users WHERE lower(name) = lower(%s)", name);
	if (!res)
		return -1;

	user_id_t ret = 0;
	if (db_num_rows(res) > 0)
		ret = db_get_user_id(res, 0, 0);
	db_clear(res);
	return ret;
}

int get_user_count(db_conn_t *c)
{
	int count = -1;
	// TODO: need to optimize later
	db_res_t *r = db_exec(c, "SELECT count(*) FROM users");
	if (db_res_status(r) == DBRES_TUPLES_OK)
		count = db_get_integer(r, 0, 0);
	db_clear(r);
	return count;
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
		fields[field], delta, name ? "lower(name) = lower(%s)" : "id = %d");
	if (bytes >= sizeof(query))
		return -1;

	db_res_t *res;
	if (name)
		res = db_exec_query(c, true, query, name);
	else
		res = db_exec_query(c, true, query, uid);
	if (!res)
		return -1;
	return 0;
}

int user_data_add_by_name(db_conn_t *c, const char *name, int field, int delta)
{
	return _user_data_add(c, name, 0, field, delta);
}

int user_data_add(db_conn_t *c, int uid, int field, int delta)
{
	return _user_data_add(c, NULL, uid, field, delta);
}

int calc_user_stay(bool is_login, bool is_dup, time_t login, time_t logout)
{
	time_t now = time(NULL);
	time_t last = logout > login ? logout : login;
	int stay = now - last;
	if (stay < 0 || (is_login && !is_dup))
		stay = 0;
	return stay;	
}
