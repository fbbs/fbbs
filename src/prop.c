#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
#include "fbbs/prop.h"
#include "fbbs/status.h"
#include "fbbs/string.h"
#include "fbbs/title.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

static tui_list_loader_t tui_prop_loader(tui_list_t *p)
{
	if (p->data)
		prop_list_free(p->data);
	p->data = prop_list_load();
	p->eod = true;
	return (p->all = prop_list_num_rows(p->data));
}

static tui_list_title_t tui_prop_title(tui_list_t *p)
{
	prints("\033[1;33;44m[聚宝盆]\033[K\033[m\n"
			" 购买[\033[1;32m→\033[m,\033[1;32mRtn\033[m]"
			" 选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m]"
			" 离开[\033[1;32m←\033[m,\033[1;32me\033[m]"
			" 求助[\033[1;32mh\033[m]\n"
			"\033[1;44m 编号    价格  类别 / 项目\033[K\033[m\n");
}

static tui_list_display_t tui_prop_display(tui_list_t *p, int n)
{
	prop_list_t *l = p->data;

	GBK_BUFFER(item, PROP_ITEM_CCHARS);
	GBK_BUFFER(categ, PROP_CATEG_CCHARS);
	convert_u2g(prop_list_get_name(l, n), gbk_item);
	convert_u2g(prop_list_get_categ_name(l, n), gbk_categ);

	prints(" %4d %7d  %s / %s\n", n + 1,
			TO_YUAN_INT(prop_list_get_price(l, n)), gbk_categ, gbk_item);
	return 0;
}

static int tui_title_buy(int type, int price)
{
	if (title_check_existence(session.uid)) {
		presskeyfor("您已购买了自定义身份，请至[藏经阁]中查看", t_lines - 1);
		return MINIUPDATE;
	}

	GBK_UTF8_BUFFER(title, TITLE_CCHARS);
	getdata(t_lines - 1, 0, "请输入自定义身份: ", gbk_title,
			sizeof(gbk_title), YEA, YEA);
	convert_g2u(gbk_title, utf8_title);
	if (validate_utf8_input(utf8_title, TITLE_CCHARS) <= 0)
		return MINIUPDATE;

	if (title_submit_request(type, session.uid, utf8_title, 0)) {
		presskeyfor("购买成功。请您耐心等待审核。", t_lines - 1);
	} else {
		presskeyfor("购买失败: 光华币余额不足或系统错误", t_lines - 1);
	}
	return MINIUPDATE;
}

static tui_list_handler_t tui_prop_handler(tui_list_t *p, int key)
{
	int type = prop_list_get_id(p->data, p->cur);
	int price = prop_list_get_price(p->data, p->cur);
	switch (type) {
		case PROP_TITLE_30DAYS:
		case PROP_TITLE_90DAYS:
		case PROP_TITLE_180DAYS:
		case PROP_TITLE_1YEAR:
			return tui_title_buy(type, price);
		default:
			break;
	}
	return DONOTHING;
}

int tui_props(void)
{
	tui_list_t t = {
		.data = NULL,
		.loader = tui_prop_loader,
		.title = tui_prop_title,
		.display = tui_prop_display,
		.handler = tui_prop_handler,
		.query = NULL,
	};

	set_user_status(ST_PROP);
	tui_list(&t);

	prop_list_free(t.data);
	return 0;
}

static tui_list_loader_t tui_my_props_loader(tui_list_t *p)
{
	if (p->data)
		my_props_free(p->data);
	p->eod = true;
	p->data = my_props_load(session.uid);
	return (p->all = my_props_count(p->data));
}

static tui_list_title_t tui_my_props_title(tui_list_t *p)
{
	prints("\033[1;33;44m[藏经阁]\033[K\033[m\n"
			" 查看详情 [\033[1;32mEnter\033[m,\033[1;32m→\033[m] "
			"返回 [\033[1;32m←\033[m,\033[1;32me\033[m]\n"
			"\033[1;44m  编号   价格  购买时间   过期时间   类别 / 项目\033[K\033[m\n");
}

static tui_list_display_t tui_my_props_display(tui_list_t *p, int n)
{
	my_props_t *r = p->data;

	char o[12], e[12];
	fb_strftime(o, sizeof(o), "%Y-%m-%d", my_prop_get_order_time(r, n));
	fb_strftime(e, sizeof(e), "%Y-%m-%d", my_prop_get_expire(r, n));

	GBK_BUFFER(item, PROP_ITEM_CCHARS);
	GBK_BUFFER(categ, PROP_CATEG_CCHARS);
	convert_u2g(my_prop_get_item_name(r, n), gbk_item);
	convert_u2g(my_prop_get_categ_name(r, n), gbk_categ);

	prints(" %4d %7d %s %s %s / %s\n", n + 1,
			TO_YUAN_INT(my_prop_get_price(r, n)), o, e, gbk_categ, gbk_item);
	return 0;
}

static int tui_my_title(int record)
{
	db_res_t *res = db_query("SELECT id, title, approved"
			" FROM titles WHERE record_id = %d", record);
	if (res && db_res_rows(res) > 0) {
		GBK_BUFFER(title, TITLE_CCHARS);
		convert_u2g(db_get_value(res, 0, 1), gbk_title);
		move(t_lines - 1, 0);
		clrtoeol();
		prints("自定义身份%s: %s", db_get_bool(res, 0, 2) ? "" : "[尚在审核]",
				gbk_title);
		egetch();
	}
	db_clear(res);
	return MINIUPDATE;
}

static int tui_remove_title(tui_list_t *p)
{
	my_props_t *r = p->data;

	if (my_prop_get_price(r, p->cur) > 0) {
		if (askyn("您将只能获得一小部分退款，确定吗?", NA, YEA)) {
			title_remove(my_prop_get_record_id(r, p->cur));
			p->valid = false;
		}
	}
	return MINIUPDATE;
}

static tui_list_handler_t tui_my_props_handler(tui_list_t *p, int key)
{
	if (p->cur >= p->all)
		return DONOTHING;

	my_props_t *r = p->data;
	int id = my_prop_get_item_id(r, p->cur);
	switch (key) {
		case '\n':
		case KEY_RIGHT:
			switch (id) {
				case PROP_TITLE_FREE:
				case PROP_TITLE_30DAYS:
				case PROP_TITLE_90DAYS:
				case PROP_TITLE_180DAYS:
				case PROP_TITLE_1YEAR:
					return tui_my_title(my_prop_get_record_id(r, p->cur));
			}
			return DONOTHING;
		case 'd':
			return tui_remove_title(p);
	}
	return DONOTHING;
}

int tui_my_props(void)
{
	tui_list_t t = {
		.data = NULL,
		.loader = tui_my_props_loader,
		.title = tui_my_props_title,
		.display = tui_my_props_display,
		.handler = tui_my_props_handler,
		.query = NULL,
	};

	set_user_status(ST_MY_PROP);
	tui_list(&t);

	my_props_free(t.data);
	return 0;
}
