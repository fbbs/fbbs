#ifndef FB_SHOP_H
#define FB_SHOP_H

#include "fbbs/dbi.h"

enum {
	SHOPPING_ITEM_CCHARS = 15,
	SHOPPING_CATEGORY_CCHARS = 15,
	SHOP_TITLE_90DAYS = 1,
	SHOP_TITLE_180DAYS,
	SHOP_TITLE_365DAYS,
};

typedef db_res_t shopping_list_t;

#define SHOPPING_LIST_SELECT_ALL \
		"SELECT i.id, c.name, i.name, i.price FROM shopping_items i" \
		" JOIN shopping_categories c ON i.category = c.id" \
		" ORDER BY i.id ASC"
extern shopping_list_t *shopping_list_load(void);
#define shopping_list_load() \
	(shopping_list_t *)db_exec_query(env.d, true, SHOPPING_LIST_SELECT_ALL)
#define shopping_list_free(p)  db_clear(p)

#define shopping_list_num_rows(list)  db_res_rows(list)
#define shopping_list_get_id(l, i)  db_get_integer(l, i, 0)
#define shopping_list_get_name(l, i)  db_get_value(l, i, 1)
#define shopping_list_get_category_name(l, i)  db_get_value(l, i, 2)
#define shopping_list_get_price(l, i)  db_get_integer(l, i, 3)

#endif // FB_SHOP_H
