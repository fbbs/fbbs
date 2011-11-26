#include "fbbs/fbbs.h"

bbs_session_t session;

int get_online_count(db_conn_t *c)
{
	int ret = -1;
	db_res_t *res = db_exec(c, "SELECT count(*) FROM sessions");
	if (db_res_status(res) == DBRES_TUPLES_OK)
		ret = db_get_integer(res, 0, 0);
	db_clear(res);
	return ret;
}
