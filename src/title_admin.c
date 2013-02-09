#include "bbs.h"
#include "fbbs/convert.h"
#include "fbbs/prop.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/time.h"
#include "fbbs/title.h"
#include "fbbs/tui_list.h"
#include "fbbs/user.h"

enum { ADDITIONAL_LEN = 64 };

#define append(s)  strappend(&tail, &remain, s)

static tui_list_loader_t title_list_loader(tui_list_t *p)
{
	title_list_t *l = p->data;
	if (l->list)
		title_list_data_free(l);

	char query[sizeof(TITLE_LIST_QUERY_BASE) + ADDITIONAL_LEN] =
			TITLE_LIST_QUERY_BASE;
	char *tail = query + sizeof(TITLE_LIST_QUERY_BASE) - 1;
	size_t remain = ADDITIONAL_LEN;

	switch (l->type) {
		case TITLE_LIST_PENDING:
			append("WHERE NOT t.approved");
			break;
		case TITLE_LIST_APPROVED:
			append("WHERE r.price > 0 AND t.approved");
			break;
		case TITLE_LIST_GRANTED:
			append("WHERE r.price = 0");
			break;
		case TITLE_LIST_ALL:
			break;
	}
	db_res_t *res = db_query(query);
	l->list = res;
	return (p->all = db_res_rows(res));
}

static tui_list_title_t title_list_title(tui_list_t *p)
{
	title_list_t *l = p->data;
	const char *t;
	switch (l->type) {
		case TITLE_LIST_PENDING:
			//% t = "待批准";
			t = "\xb4\xfd\xc5\xfa\xd7\xbc";
			break;
		case TITLE_LIST_APPROVED:
			//% t = "已批准";
			t = "\xd2\xd1\xc5\xfa\xd7\xbc";
			break;
		case TITLE_LIST_GRANTED:
			//% t = "已授予";
			t = "\xd2\xd1\xca\xda\xd3\xe8";
			break;
		default:
			//% t = "全部";
			t = "\xc8\xab\xb2\xbf";
			break;
	}

	//% prints("\033[1;33;44m[自定义身份管理][%s]\033[K\033[m\n", t);
	prints("\033[1;33;44m[\xd7\xd4\xb6\xa8\xd2\xe5\xc9\xed\xb7\xdd\xb9\xdc\xc0\xed][%s]\033[K\033[m\n", t);
	//% prints(" 批准[\033[1;32m.\033[m] 驳回[\033[1;32md\033[m]"
	prints(" \xc5\xfa\xd7\xbc[\033[1;32m.\033[m] \xb2\xb5\xbb\xd8[\033[1;32md\033[m]"
			//% " 切换[\033[1;32ms\033[m] 授予[\033[1;32mg\033[m]\n");
			" \xc7\xd0\xbb\xbb[\033[1;32ms\033[m] \xca\xda\xd3\xe8[\033[1;32mg\033[m]\n");
	//% prints("\033[1;44m编号 用户名       批准人       自定义身份"
	prints("\033[1;44m\xb1\xe0\xba\xc5 \xd3\xc3\xbb\xa7\xc3\xfb       \xc5\xfa\xd7\xbc\xc8\xcb       \xd7\xd4\xb6\xa8\xd2\xe5\xc9\xed\xb7\xdd"
			//% "                     购买日期 过期时间\033[m\n");
			"                     \xb9\xba\xc2\xf2\xc8\xd5\xc6\xda \xb9\xfd\xc6\xda\xca\xb1\xbc\xe4\033[m\n");
}

static tui_list_display_t title_list_display(tui_list_t *p, int n)
{
	db_res_t *l = ((title_list_t *)p->data)->list;

	GBK_BUFFER(title, TITLE_CCHARS);
	convert_u2g(title_list_get_title(l, n), gbk_title);

	const char *granter = title_list_get_approved(l, n) ?
			title_list_get_granter(l, n) : "-";

	char a[12], e[12];
	prints("%4d %-12s %-12s %-30s %s %s\n", n + 1,
			title_list_get_name(l, n), granter, gbk_title,
			fb_strftime(a, sizeof(a), "%Y%m%d", title_list_get_order_time(l, n)),
			fb_strftime(e, sizeof(e), "%Y%m%d", title_list_get_expire(l, n)));
	return 0;
}

static int grant_title(user_id_t uid, const char *title)
{
	if (validate_utf8_input(title, TITLE_CCHARS) < 0)
		return -1;

	return title_submit_request(PROP_TITLE_FREE, uid, title, session.uid);
}

static int tui_grant_title(tui_list_t *p)
{
	char name[IDLEN + 1];
	//% getdata(t_lines - 1, 0, "请输入用户名: ", name, sizeof(name), YEA, YEA);
	getdata(t_lines - 1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd3\xc3\xbb\xa7\xc3\xfb: ", name, sizeof(name), YEA, YEA);
	if (!*name)
		return MINIUPDATE;

	user_id_t uid = get_user_id(name);
	if (uid <= 0)
		return MINIUPDATE;

	GBK_UTF8_BUFFER(title, TITLE_CCHARS);
	//% getdata(t_lines - 1, 0, "请输入身份: ", gbk_title, sizeof(gbk_title),
	getdata(t_lines - 1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc9\xed\xb7\xdd: ", gbk_title, sizeof(gbk_title),
			YEA, YEA);
	if (!*gbk_title)
		return MINIUPDATE;

	convert_g2u(gbk_title, utf8_title);
	if (grant_title(uid, utf8_title)) {
		title_list_t *l = p->data;
		if (l->type == TITLE_LIST_GRANTED || l->type == TITLE_LIST_ALL) {
			p->valid = false;
			return PARTUPDATE;
		}
	}
	return MINIUPDATE;
}

static int tui_remove_title(tui_list_t *p)
{
	title_list_t *l = p->data;
	db_res_t *r = l->list;

	const char *prompt;
	if (title_list_get_price(r, p->cur) > 0) {
		//% prompt = "该用户只能获得一小部分退款，确定吗?";
		prompt = "\xb8\xc3\xd3\xc3\xbb\xa7\xd6\xbb\xc4\xdc\xbb\xf1\xb5\xc3\xd2\xbb\xd0\xa1\xb2\xbf\xb7\xd6\xcd\xcb\xbf\xee\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0?";
	} else {
		//% prompt = "确定收回这个免费身份?";
		prompt = "\xc8\xb7\xb6\xa8\xca\xd5\xbb\xd8\xd5\xe2\xb8\xf6\xc3\xe2\xb7\xd1\xc9\xed\xb7\xdd?";
	}
	
	if (askyn(prompt, NA, YEA)) {
		title_remove(title_list_get_record_id(r, p->cur));
		p->valid = false;
	}
	return MINIUPDATE;
}

static tui_list_handler_t title_list_handler(tui_list_t *p, int key)
{
	title_list_t *l = p->data;
	db_res_t *r = l->list;

	bool valid = p->cur < p->all;

	switch (key) {
		case 's':
			if (++l->type > TITLE_LIST_ALL)
				l->type = TITLE_LIST_PENDING;
			p->valid = false;
			return FULLUPDATE;
		case '.':
			if (l->type != TITLE_LIST_PENDING || !valid)
				return DONOTHING;
			title_approve(title_list_get_id(r, p->cur));
			p->valid = false;
			break;
		case 'd':
			if (!valid)
				return DONOTHING;
			if (l->type == TITLE_LIST_PENDING) {
				//% if (!askyn("确定驳回?", NA, YEA))
				if (!askyn("\xc8\xb7\xb6\xa8\xb2\xb5\xbb\xd8?", NA, YEA))
					return MINIUPDATE;
				title_remove(title_list_get_record_id(r, p->cur));
				p->valid = false;
			} else {
				return tui_remove_title(p);
			}
			break;
		case 'g':
			if (valid)
				return tui_grant_title(p);
			break;
		default:
			return READ_AGAIN;
	}
	return DONOTHING;
}

int tui_title_list(void)
{
	title_list_t l = {
		.type = TITLE_LIST_PENDING,
		.list = NULL,
	};
	tui_list_t t = {
		.data = &l,
		.loader = title_list_loader,
		.title = title_list_title,
		.display = title_list_display,
		.handler = title_list_handler,
		.query = NULL,
	};

	set_user_status(ST_ADMIN);
	tui_list(&t);

	title_list_data_free(t.data);
	return 0;
}
