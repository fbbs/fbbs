#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/helper.h"
#include "fbbs/money.h"
#include "fbbs/prop.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/title.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

static tui_list_loader_t tui_prop_loader(tui_list_t *p)
{
	if (p->data)
		prop_list_free(p->data);
	p->data = prop_list_load();
	return (p->all = prop_list_num_rows(p->data));
}

static tui_list_title_t tui_prop_title(tui_list_t *p)
{
	screen_printf("\033[1;33;44m[聚宝盆]\033[K\033[m\n"
			" 购买[\033[1;32m→\033[m,\033[1;32mRtn\033[m]"
			" 选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m]"
			" 离开[\033[1;32m←\033[m,\033[1;32me\033[m]"
			" 求助[\033[1;32mh\033[m]\n"
			"\033[1;44m 编号    价格  类别 / 项目\033[K\033[m\n");
}

static tui_list_display_t tui_prop_display(tui_list_t *p, int n)
{
	prop_list_t *l = p->data;
	screen_printf(" %4d %7d  %s / %s\n", n + 1,
			TO_YUAN_INT(prop_list_get_price(l, n)),
			prop_list_get_categ_name(l, n),
			prop_list_get_name(l, n));
	return 0;
}

static int tui_title_buy(int type, int price)
{
	if (title_check_existence(session_get_user_id())) {
		//% 您已购买了自定义身份，请至[藏经阁]中查看
		presskeyfor("\xc4\xfa\xd2\xd1\xb9\xba\xc2\xf2\xc1\xcb\xd7\xd4\xb6\xa8\xd2\xe5\xc9\xed\xb7\xdd\xa3\xac\xc7\xeb\xd6\xc1[\xb2\xd8\xbe\xad\xb8\xf3]\xd6\xd0\xb2\xe9\xbf\xb4", -1);
		return MINIUPDATE;
	}

	GBK_UTF8_BUFFER(title, TITLE_CCHARS);
	//% getdata(-1, 0, "请输入自定义身份: ", gbk_title,
	getdata(-1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd7\xd4\xb6\xa8\xd2\xe5\xc9\xed\xb7\xdd: ", gbk_title,
			sizeof(gbk_title), YEA, YEA);
	convert_g2u(gbk_title, utf8_title);
	if (string_validate_utf8(utf8_title, TITLE_CCHARS, false) <= 0)
		return MINIUPDATE;

	if (title_submit_request(type, session_get_user_id(), utf8_title, 0)) {
		//% 购买成功。请您耐心等待审核。
		presskeyfor("\xb9\xba\xc2\xf2\xb3\xc9\xb9\xa6\xa1\xa3\xc7\xeb\xc4\xfa\xc4\xcd\xd0\xc4\xb5\xc8\xb4\xfd\xc9\xf3\xba\xcb\xa1\xa3", -1);
	} else {
		//% 购买失败: 光华币余额不足或系统错误
		presskeyfor("\xb9\xba\xc2\xf2\xca\xa7\xb0\xdc: \xb9\xe2\xbb\xaa\xb1\xd2\xd3\xe0\xb6\xee\xb2\xbb\xd7\xe3\xbb\xf2\xcf\xb5\xcd\xb3\xb4\xed\xce\xf3", -1);
	}
	return MINIUPDATE;
}

static tui_list_handler_t tui_prop_handler(tui_list_t *p, int key)
{
	if (key != KEY_RIGHT && key != '\n')
		return READ_AGAIN;

	int type = prop_list_get_id(p->data, p->cur);
	int price = prop_list_get_price(p->data, p->cur);
	switch (type) {
		case PROP_TITLE_30DAYS:
		case PROP_TITLE_90DAYS:
		case PROP_TITLE_180DAYS:
		case PROP_TITLE_1YEAR:
			return tui_title_buy(type, price);
		default:
			return READ_AGAIN;
	}
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
	p->data = my_props_load(session_get_user_id());
	return (p->all = my_props_count(p->data));
}

static tui_list_title_t tui_my_props_title(tui_list_t *p)
{
	screen_printf("\033[1;33;44m[藏经阁]\033[K\033[m\n"
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

	screen_printf(" %4d %7d %s %s %s / %s\n", n + 1,
			TO_YUAN_INT(my_prop_get_price(r, n)), o, e,
			my_prop_get_categ_name(r, n),
			my_prop_get_item_name(r, n));
	return 0;
}

static int tui_my_title(int record)
{
	db_res_t *res = db_query("SELECT id, title, approved"
			" FROM titles WHERE record_id = %d", record);
	if (res && db_res_rows(res) > 0) {
		screen_move_clear(-1);
		screen_printf("自定义身份%s: %s", db_get_bool(res, 0, 2) ? "" : "[尚在审核]",
				db_get_value(res, 0, 1));
		egetch();
	}
	db_clear(res);
	return MINIUPDATE;
}

static int tui_remove_title(tui_list_t *p)
{
	my_props_t *r = p->data;

	if (my_prop_get_price(r, p->cur) > 0) {
		//% if (askyn("您将只能获得一小部分退款，确定吗?", NA, YEA)) {
		if (askyn("\xc4\xfa\xbd\xab\xd6\xbb\xc4\xdc\xbb\xf1\xb5\xc3\xd2\xbb\xd0\xa1\xb2\xbf\xb7\xd6\xcd\xcb\xbf\xee\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0?", NA, YEA)) {
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
	return READ_AGAIN;
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
