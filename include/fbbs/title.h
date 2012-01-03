#ifndef FB_TITLE_H
#define FB_TITLE_H

#include "fbbs/dbi.h"

enum {
	TITLE_CCHARS = 15
};

typedef enum title_list_type_t {
	TITLE_LIST_PENDING,
	TITLE_LIST_APPROVED,
	TITLE_LIST_GRANTED,
	TITLE_LIST_ALL,
} title_list_type_t;

typedef struct title_list_t {
	title_list_type_t type;
	db_res_t *list;
} title_list_t;

#define TITLE_LIST_QUERY_BASE \
	"SELECT t.id, u1.name, u2.name, t.title, r.order_time, r.expire, t.approved, r.price" \
	" FROM titles t JOIN all_users u1 ON t.user_id = u1.id" \
	" JOIN all_users u2 ON t.granter = u2.id" \
	" JOIN prop_records r ON t.record_id = r.id "

#define title_list_get_id(list, i)  db_get_user_id(list, i, 0)
#define title_list_get_name(list, i)  db_get_value(list, i, 1)
#define title_list_get_granter(list, i)  db_get_value(list, i, 2)
#define title_list_get_title(list, i)  db_get_value(list, i, 3)
#define title_list_get_order_time(list, i)  db_get_time(list, i, 4)
#define title_list_get_expire(list, i)  db_get_time(list, i, 5)
#define title_list_get_approved(list, i)  db_get_bool(list, i, 6)
#define title_list_get_price(list, i)  db_get_integer(list, i, 7)

#define title_list_data_free(data)  db_clear(((title_list_t *)data)->list)

extern bool title_check_existence(user_id_t uid);
extern bool title_submit_request(int type, user_id_t uid, const char *title, user_id_t granter);
extern void title_approve(int id);
extern void title_disapprove(int id);

#endif // FB_TITLE_H
