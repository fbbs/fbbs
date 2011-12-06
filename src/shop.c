#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
#include "fbbs/shop.h"
#include "fbbs/tui_list.h"

static tui_list_loader_t tui_shop_loader(tui_list_t *p)
{
	if (p->data)
		shopping_list_free(p->data);
	p->data = shopping_list_load();
	p->eod = true;
	return (p->all = shopping_list_num_rows(p->data));
}

static tui_list_title_t tui_shop_title(tui_list_t *p)
{
	prints("\033[1;33;44m["SHORT_BBSNAME"商城]\033[K\033[m\n"
			" 购买[\033[1;32m→\033[m,\033[1;32mRtn\033[m]"
			" 选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m]"
			" 离开[\033[1;32m←\033[m,\033[1;32me\033[m]"
			" 求助[\033[1;32mh\033[m]\n"
			"\033[1;44m 编号    价格   类别 / 项目\033[K\033[m\n");
}

static tui_list_display_t tui_shop_display(tui_list_t *p, int n)
{
	shopping_list_t *l = p->data;

	GBK_BUFFER(item, SHOPPING_ITEM_CCHARS);
	GBK_BUFFER(categ, SHOPPING_CATEGORY_CCHARS);
	convert_u2g(shopping_list_get_name(l, n), gbk_item);
	convert_u2g(shopping_list_get_category_name(l, n), gbk_categ);

	prints(" %4d %7d  %s / %s\n", n + 1,
			TO_YUAN_INT(shopping_list_get_price(l, n)), gbk_item, gbk_categ);
	return 0;
}

static tui_list_handler_t tui_shop_handler(tui_list_t *p, int key)
{
	return DONOTHING;
}

static tui_list_query_t tui_shop_query(tui_list_t *p)
{
	switch (shopping_list_get_id(p->data, p->cur)) {
		case SHOP_TITLE_90DAYS:
		case SHOP_TITLE_180DAYS:
		case SHOP_TITLE_365DAYS:
		default:
			break;
	}
	return DONOTHING;
}

int tui_shop(void)
{
	tui_list_t t = {
		.data = NULL,
		.loader = tui_shop_loader,
		.title = tui_shop_title,
		.display = tui_shop_display,
		.handler = tui_shop_handler,
		.query = tui_shop_query,
	};

	tui_list(&t);

	shopping_list_free(t.data);
	return 0;
}
