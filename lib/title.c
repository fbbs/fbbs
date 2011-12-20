#include "fbbs/fbbs.h"
#include "fbbs/shop.h"
#include "fbbs/title.h"

bool title_check_existence(user_id_t uid)
{
	db_res_t *res = db_query("SELECT id FROM titles"
			" WHERE paid > 0 AND user_id = %"PRIdUID, uid);
	int rows = db_res_rows(res);
	db_clear(res);
	return rows;
}

bool title_submit_request(int type, const char *title)
{
	fb_time_t now = time(NULL);
	fb_time_t expire = now;
	switch (type) {
		case SHOP_TITLE_90DAYS:
			expire += 97 * 86400;
			break;
		case SHOP_TITLE_180DAYS:
			expire += 187 * 86400;
			break;
		case SHOP_TITLE_365DAYS:
			expire += 372 * 86400;
			break;
	}

	db_res_t *res = db_cmd("INSERT INTO titles"
			" (user_id, granter, title, add_time, expire, paid)"
			" VALUES (%"PRIdUID", %"PRIdUID", %s, %t, %t, "
			" (SELECT price FROM shopping_items WHERE id = %d))",
			session.uid, session.uid, title, now, expire, type);
	bool ok = res;
	db_clear(res);
	return ok;
}

void title_approve(int id)
{
	db_res_t *res = db_cmd("UPDATE titles SET approved = true"
			" WHERE id = %d", id);
	db_clear(res);
}

void title_disapprove(int id)
{
	db_res_t *res = db_cmd("DELETE FROM titles WHERE id = %d", id);
	db_clear(res);
}
