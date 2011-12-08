#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
#include "fbbs/shop.h"
#include "fbbs/string.h"
#include "fbbs/title.h"
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

static int tui_title_buy(int type, int price)
{
	if (title_check_existence(session.uid)) {
		presskeyfor("您已购买了自定义身份，请至我的商品中查看", t_lines - 1);
		return MINIUPDATE;
	}

	GBK_UTF8_BUFFER(title, TITLE_CCHARS);
	getdata(t_lines - 1, 0, "请输入自定义身份: ", gbk_title,
			sizeof(gbk_title), YEA, YEA);
	convert_g2u(gbk_title, utf8_title);
	if (validate_utf8_input(utf8_title, TITLE_CCHARS) <= 0)
		return MINIUPDATE;

	if (title_submit_request(type, utf8_title)) {
		presskeyfor("购买成功。请您耐心等待审核。", t_lines - 1);
	} else {
		presskeyfor("购买失败: 光华币余额不足或系统错误", t_lines - 1);
	}
	return MINIUPDATE;
}

static tui_list_handler_t tui_shop_handler(tui_list_t *p, int key)
{
	int type = shopping_list_get_id(p->data, p->cur);
	int price = shopping_list_get_price(p->data, p->cur);
	switch (type) {
		case SHOP_TITLE_90DAYS:
		case SHOP_TITLE_180DAYS:
		case SHOP_TITLE_365DAYS:
			return tui_title_buy(type, price);
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
		.query = NULL,
	};

	tui_list(&t);

	shopping_list_free(t.data);
	return 0;
}
