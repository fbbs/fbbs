#include "libweb.h"
#include "fbbs/fbbs.h"
#include "fbbs/prop.h"

static void show_prop_detail(prop_list_t *p, int i)
{
	GBK_BUFFER(name, PROP_ITEM_CCHARS);
	GBK_BUFFER(categ, PROP_CATEGORY_CCHARS);
	convert_u2g(prop_list_get_name(p, i), gbk_name);
	convert_u2g(prop_list_get_category_name(p, i), gbk_categ);
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
	for (int i = 0; i < prop_list_num_rows(p); ++i) {
		show_prop_detail(p, i);
	}
	printf("</bbsprop>");

	prop_list_free(p);
	return 0;
}

static void show_my_prop_detail(my_props_t *p, int i)
{
	GBK_BUFFER(name, PROP_ITEM_CCHARS);
	GBK_BUFFER(categ, PROP_CATEGORY_CCHARS);
	convert_u2g(my_prop_get_item_name(p, i), gbk_name);
	convert_u2g(my_prop_get_categ_name(p, i), gbk_categ);

	printf("<prop record='%d' item='%d' name='%s' categ='%s' price='%d'"
			" order='%s'", my_prop_get_record_id(p, i),
			my_prop_get_item_id(p, i), gbk_name, gbk_categ,
			TO_YUAN_INT(my_prop_get_price(p, i)),
			getdatestring(my_prop_get_order_time(p, i), DATE_XML));
	printf(" expire='%s'/>", getdatestring(my_prop_get_expire(p, i), DATE_XML));
}

int web_my_props(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	my_props_t *p = my_props_load(session.uid);
	if (!p)
		return BBS_EINTNL;

	xml_header(NULL);
	printf("<bbsmyprop>");
	for (int i = 0; i < my_props_count(p); ++i) {
		show_my_prop_detail(p, i);
	}
	printf("</bbsmyprop>");
	
	my_props_free(p);
	return 0;
}
