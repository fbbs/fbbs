#include "fbbs/fbbs.h"
#include "fbbs/prop.h"
#include "fbbs/title.h"

bool title_check_existence(user_id_t uid)
{
	db_res_t *res = db_query("SELECT t.id"
			" FROM titles t JOIN prop_records r ON t.record_id = r.id"
			" WHERE r.item <> %d AND t.user_id = %"PRIdUID,
			PROP_TITLE_FREE, uid);
	int rows = db_res_rows(res);
	db_clear(res);
	return rows;
}

bool title_submit_request(int type, user_id_t uid, const char *title, user_id_t granter)
{
	db_res_t *res = db_query("SELECT buy_title_request(%d, %d, %s, %d)",
			uid, type, title, granter);
	bool ok = res;
	db_clear(res);
	return ok;
}

void title_approve(int id)
{
	db_res_t *res = db_cmd("UPDATE titles"
			" SET approved = true, granter = %d WHERE id = %d",
			session.uid, id);
	db_clear(res);
}

void title_disapprove(int id)
{
	db_res_t *res = db_cmd("DELETE FROM titles WHERE id = %d", id);
	db_clear(res);
}
