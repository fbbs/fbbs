#include "libweb.h"
#include "fbbs/fbbs.h"
#include "fbbs/prop.h"
#include "fbbs/string.h"
#include "fbbs/title.h"

static void show_prop(prop_list_t *p, int i)
{
	GBK_BUFFER(name, PROP_ITEM_CCHARS);
	GBK_BUFFER(categ, PROP_CATEG_CCHARS);
	convert_u2g(prop_list_get_name(p, i), gbk_name);
	convert_u2g(prop_list_get_categ_name(p, i), gbk_categ);
	printf("<item id='%d' name='%s' categ='%s' price='%d'/>", 
			prop_list_get_id(p, i), gbk_name, gbk_categ,
			TO_YUAN_INT(prop_list_get_price(p, i)));
}

int web_props(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	prop_list_t *p = prop_list_load();
	if (!p)
		return BBS_EINTNL;

	xml_header(NULL);
	printf("<bbsprop>");
	print_session();
	for (int i = 0; i < prop_list_num_rows(p); ++i) {
		show_prop(p, i);
	}
	printf("</bbsprop>");

	prop_list_free(p);
	return 0;
}

static void show_my_prop(my_props_t *p, int i)
{
	GBK_BUFFER(name, PROP_ITEM_CCHARS);
	GBK_BUFFER(categ, PROP_CATEG_CCHARS);
	convert_u2g(my_prop_get_item_name(p, i), gbk_name);
	convert_u2g(my_prop_get_categ_name(p, i), gbk_categ);

	printf("<prop record='%d' item='%d' name='%s' categ='%s' price='%d'"
			" order='%s'", my_prop_get_record_id(p, i),
			my_prop_get_item_id(p, i), gbk_name, gbk_categ,
			TO_YUAN_INT(my_prop_get_price(p, i)),
			getdatestring(my_prop_get_order_time(p, i), DATE_XML));
	printf(" expire='%s'/>", getdatestring(my_prop_get_expire(p, i), DATE_XML));
}

static int show_title_detail(int record)
{
	db_res_t *res = db_query("SELECT title, approved FROM titles"
			" WHERE record_id = %d AND user_id = %"PRIdUID,
			record, session.uid);
	if (!res || db_res_rows(res) <= 0) {
		db_clear(res);
		return BBS_EINVAL;
	}

	GBK_BUFFER(title, TITLE_CCHARS);
	convert_u2g(db_get_value(res, 0, 0), gbk_title);

	xml_header(NULL);
	printf("<bbspropdetail>");
	print_session();
	printf("<prop>自定义身份%s: %s</prop></bbspropdetail>",
			db_get_bool(res, 0, 1) ? "" : "[尚在审核]", gbk_title);

	db_clear(res);
	return 0;
}

int web_my_props(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	int record = strtol(get_param("record"), NULL, 10);
	int item = strtol(get_param("item"), NULL, 10);

	if (record <= 0 || item <= 0) {
		my_props_t *p = my_props_load(session.uid);
		if (!p)
			return BBS_EINTNL;

		xml_header(NULL);
		printf("<bbsmyprop>");
		print_session();
		for (int i = 0; i < my_props_count(p); ++i) {
			show_my_prop(p, i);
		}
		printf("</bbsmyprop>");

		my_props_free(p);
	} else {
		switch (item) {
			case PROP_TITLE_FREE:
			case PROP_TITLE_30DAYS:
			case PROP_TITLE_90DAYS:
			case PROP_TITLE_180DAYS:
				return show_title_detail(record);
			default:
				return BBS_EINVAL;
		}
	}
	return 0;
}

static int buy_title(int item, const char *title)
{
	xml_header(NULL);
	printf("<bbsbuyprop>");
	print_session();
	if (title && *title) {
		UTF8_BUFFER(title, TITLE_CCHARS);
		convert_g2u(title, utf8_title);
		if (validate_utf8_input(utf8_title, TITLE_CCHARS) > 0
				&& title_submit_request(item, utf8_title)) {
			printf("<success/>");
		}
	} else {
		printf("<inputs item='%d'>"
				"<label for='title'>请输入自定义身份</label>"
				"<input name='title' type='text' width='%d'></input>"
				"</inputs>", item, TITLE_CCHARS);
	}
	printf("</bbsbuyprop>");
	return 0;
}

int web_buy_prop(void)
{
	int item = strtol(get_param("item"), NULL, 10);
	if (item <= 0)
		return BBS_EINVAL;

	parse_post_data();

	switch (item) {
		case PROP_TITLE_30DAYS:
		case PROP_TITLE_90DAYS:
		case PROP_TITLE_180DAYS:
			return buy_title(item, get_param("title"));
		default:
			return BBS_EINVAL;
	}
}
