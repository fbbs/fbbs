#ifndef FB_PROP_H
#define FB_PROP_H

#include "fbbs/dbi.h"

enum {
	PROP_ITEM_CCHARS = 15,
	PROP_CATEGORY_CCHARS = 15,
	PROP_TITLE_FREE = 1,
	PROP_TITLE_90DAYS,
	PROP_TITLE_180DAYS,
	PROP_TITLE_365DAYS,
};

typedef db_res_t prop_list_t;

#define PROP_LIST_SELECT_ALL \
		"SELECT i.id, c.name, i.name, i.price FROM prop_items i" \
		" JOIN prop_categs c ON i.categ = c.id WHERE valid ORDER BY i.id ASC"
#define prop_list_load() \
	(prop_list_t *)db_exec_query(env.d, true, PROP_LIST_SELECT_ALL)
#define prop_list_free(p)  db_clear(p)

#define prop_list_num_rows(list)  db_res_rows(list)
#define prop_list_get_id(l, i)  db_get_integer(l, i, 0)
#define prop_list_get_name(l, i)  db_get_value(l, i, 1)
#define prop_list_get_category_name(l, i)  db_get_value(l, i, 2)
#define prop_list_get_price(l, i)  db_get_integer(l, i, 3)

typedef db_res_t my_props_t;
#define MY_PROPS_QUERY \
		"SELECT r.id, c.name, i.id, i.name, r.price, r.order_time, r.expire" \
		" FROM prop_records r JOIN prop_items i ON r.item = i.id" \
		" JOIN prop_categs c ON i.categ = c.id" \
		" WHERE r.expire > current_timestamp AND r.user_id = %"PRIdUID \
		" ORDER BY r.order_time ASC"
#define my_props_load(uid) \
	(my_props_t *)db_exec_query(env.d, true, MY_PROPS_QUERY, uid)
#define my_props_free(p)  db_clear(p)
#define my_props_count(p)  db_res_rows(p)

#define my_prop_get_record_id(p, i)  db_get_integer(p, i, 0)
#define my_prop_get_categ_name(p, i)  db_get_value(p, i, 1)
#define my_prop_get_item_id(p, i)  db_get_integer(p, i, 2)
#define my_prop_get_item_name(p, i)  db_get_value(p, i, 3)
#define my_prop_get_price(p, i)  db_get_integer(p, i, 4)
#define my_prop_get_order_time(p, i)  db_get_time(p, i, 5)
#define my_prop_get_expire(p, i)  db_get_time(p, i, 6)

#endif // FB_PROP_H
