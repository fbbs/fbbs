#include <dlfcn.h>
#include "bbs.h"
#include "mmap.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

//-------------------------------------------------------------------
int Announce(), Personal(), Info(), Goodbye();
int board_select(), Welcome();
int msg_more(), x_lockscreen();
int Conditions(), x_cloak(), show_online_users(), x_info(), x_vote();
int x_results(), ent_bnet(), a_edits(), x_edits();
int x_userdefine();
int m_new(), m_read(), m_send(), g_send();
int ov_send(), s_msg(), mailall(), offline();
int giveUpBBS();
extern int fill_reg_form(void);

int ent_bnet2();

#ifdef ALLOWGAME
int ent_winmine();
#endif

#ifdef INTERNET_EMAIL
int m_internet();
#endif

int show_online_followings(), t_list(), t_monitor();
int x_cloak();
int AddPCorpus();
extern int tui_following_list(void);
extern int tui_black_list(void);
extern int tui_query(void);

#ifndef WITHOUT_ADMIN_TOOLS
int m_vote();
#endif

int wall();

extern int tui_props(void);
extern int tui_my_props(void);

extern int tui_ordain_bm(const char *);
extern int tui_retire_bm(const char *);
extern int tui_new_board(const char *);
extern int tui_edit_board(const char *);

extern int post_list_reply(void);
extern int post_list_mention(void);
//-----------------------------------------

extern void active_board_show(void);

typedef uint16_t menu_offset_t;
typedef int32_t menu_value_t;

typedef int (*menu_function_t)();

typedef struct {
	const char *string;
	menu_function_t func;
} internal_func_t;

#define DECLARE_INTERNAL(f)  { .string = #f, .func = f }

enum {
	MENU_LEVEL_MAX = 5,
	MENU_CONST_RECORD_SIZE = sizeof(menu_value_t),
	MENU_GROUP_RECORD_SIZE = sizeof(menu_offset_t) * 2 + 1,
};

typedef struct {
	uchar_t line;
	uchar_t col;
	menu_offset_t cmd;
	menu_offset_t descr;
	menu_offset_t perm;
} menu_item_t;

typedef struct {
	const char *string_base;
	menu_offset_t size;
	menu_offset_t const_count;
	menu_offset_t group_count;
	const char *buf;
} menu_t;

static menu_t _menu = { .buf = NULL };

#define UNSERIALIZE_TYPE(type, func) \
static type func(const char *src) \
{ \
	union { \
		char s[sizeof(type)]; \
		type t; \
	} u; \
	memcpy(u.s, src, sizeof(type)); \
	return u.t; \
}

UNSERIALIZE_TYPE(menu_offset_t, menu_read_offset)
UNSERIALIZE_TYPE(menu_value_t, menu_read_value)

static bool valid_offset(int offset)
{
	return offset >= 0 && offset <= UINT16_MAX;
}

static const char *group_base(void)
{
	return _menu.buf + _menu.const_count * MENU_CONST_RECORD_SIZE;
}

static const char *item_base(void)
{
	return group_base() + _menu.group_count * MENU_GROUP_RECORD_SIZE;
}

static const char *menu_end(void)
{
	return _menu.buf + _menu.size;
}

/**
 * 读取 menu.img 文件
 * @param file menu.img 文件路径
 * @return 成功返回 true, 出错返回 false
 *
 * menu.img 结构示意 
 * 常量定义总数: menu_offset_t
 * 菜单组总数: menu_offset_t
 * -------------------------- _menu.buf
 * [常量]
 *  值: menu_value_t
 *  ...
 *
 * [菜单组]
 *  名称: menu_offset_t
 *  起始菜单项: menu_offset_t
 *  组内菜单数: uchar_t
 *  ...
 *
 * [菜单项]
 *  菜单项: menu_item_t
 *  ...
 *
 * -------------------------- _menu.string_base
 * [字符串]
 *  C 风格字符串
 * -------------------------- _menu.buf + _menu.size
 */
bool menu_load(const char *file)
{
	const int header_size = 2 * sizeof(menu_offset_t);

	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) != 0)
		return false;

	if (m.size <= header_size || m.size > UINT16_MAX) {
		mmap_close(&m);
		return false;
	}

	_menu.size = m.size - header_size;
	_menu.buf = malloc(_menu.size);
	if (!_menu.buf) {
		mmap_close(&m);
		return false;
	}

	memcpy(&_menu.const_count, m.ptr, header_size);
	memcpy((char *) _menu.buf, (char *) m.ptr + header_size, _menu.size);
	mmap_close(&m);

	const char *end = menu_end();
	const char *group = group_base();
	if (group >= end)
		return false;

	int items = 0;
	for (int i = 0; i < _menu.group_count; ++i) {
		items += (uchar_t) group[(i + 1) * MENU_GROUP_RECORD_SIZE - 1];
	}
	_menu.string_base = item_base() + items * sizeof(menu_item_t);
	return _menu.string_base < end;
}

static const char *menu_get_string(menu_offset_t offset)
{
	const char *s = _menu.string_base + offset;
	return s >= menu_end() ? NULL : s;
}

static menu_value_t menu_get_const(menu_offset_t offset)
{
	if (offset < _menu.const_count)
		return menu_read_value(_menu.buf + MENU_CONST_RECORD_SIZE * offset);
	return 0;
}

static int find_group(const char *group_name)
{
	const char *base = group_base();
	for (int i = 0; i < _menu.group_count; ++i) {
		const char *ptr = base + i * MENU_GROUP_RECORD_SIZE;
		const char *str = menu_get_string(menu_read_offset(ptr));
		if (str && streq(str, group_name))
			return i;
	}
	return -1;
}

static void print_background(const char *ptr, int line, int col)
{
	move(line, col);
	screen_clrtobot();

	int ch;
	while ((ch = *ptr++) != '\0') {
		if (ch == 1) {
			if (*ptr != '\0' && ptr[1] != '\0') {
				ch = *ptr++;
				int n = *ptr++;
				tui_repeat_char(ch, n);
			}
		} else {
			screen_putc(ch);
		}
	}
}

static bool read_item(int offset, menu_item_t *item)
{
	const char *ptr = item_base() + sizeof(*item) * offset;
	if (ptr + sizeof(*item) < menu_end()) {
		memcpy(item, ptr, sizeof(*item));
		return true;
	}
	return false;
}

typedef int (*menu_callback_t)(const menu_item_t *item, void *arg, int offset);

static void foreach_item(int begin, int end, menu_callback_t cb, void *arg)
{
	int line = 0, col = 0;
	for (int i = begin; i < end; ++i) {
		menu_item_t item;
		if (!read_item(i, &item))
			continue;

		menu_value_t perm = menu_get_const(item.perm);
		if (!HAS_PERM(perm))
			continue;

		if (item.line == 0) {
			item.line = ++line;
			item.col = col;
		} else {
			line = item.line;
			col = item.col;
		}

		if (cb(&item, arg, i) < 0)
			return;
	}
}

typedef struct {
	int now;
	bool jump_to_mail_menu;
} print_item_arg_t;

static int print_item(const menu_item_t *item, void *a, int offset)
{
	print_item_arg_t *arg = a;

	const char *cmd = menu_get_string(item->cmd);
	if (!cmd)
		return 0;

	const char *descr = menu_get_string(item->descr);
	if (streq(cmd, "title")) {
		// TODO
	} else if (streq(cmd, "screen")) {
		if (descr)
			print_background(descr, item->line, item->col);
	} else {
		if (descr) {
			screen_replace(item->line, item->col + 2, descr);
			if (!arg->now)
				arg->now = offset;
		}
	}

	if (arg->jump_to_mail_menu && descr && *descr == 'M')
		arg->now = offset;
	return 0;
}

static int draw_group(int first_item, int items, bool top)
{
	if (!valid_offset(first_item) && !valid_offset(items))
		return first_item;
	screen_clear();

	print_item_arg_t arg = {
		.now = 0,
		.jump_to_mail_menu = false,
	};
	if (top) {
		user_id_t user_id = session_uid();
		arg.jump_to_mail_menu = chkmail()
				|| post_reply_get_count(user_id)
				|| post_mention_get_count(user_id);
	}

	foreach_item(first_item, first_item + items, print_item, &arg);
	return arg.now ? arg.now : first_item;
}

typedef struct {
	int *line;
	int *col;
} get_coordination_arg_t;

static int get_coordination_callback(const menu_item_t *item, void *a,
		int offset)
{
	get_coordination_arg_t *arg = a;
	*arg->line = item->line;
	*arg->col = item->col;
	return 0;
}

static void get_coordination(int first_item, int now, int *line, int *col)
{
	*line = *col = 0;
	get_coordination_arg_t arg = {
		.line = line,
		.col = col,
	};
	foreach_item(first_item, now + 1, get_coordination_callback, &arg);
}

static int internal_function_compare(const void *key, const void *arg)
{
	const internal_func_t *ptr = arg;
	return strcmp(key, ptr->string);
}

static void function_eval(const char *cmd, const char *descr)
{
	static const internal_func_t internal[] = {
		DECLARE_INTERNAL(AddPCorpus),
		DECLARE_INTERNAL(Announce),
		DECLARE_INTERNAL(Conditions),
		DECLARE_INTERNAL(Goodbye),
		DECLARE_INTERNAL(Info),
		DECLARE_INTERNAL(Personal),
		DECLARE_INTERNAL(Welcome),
		DECLARE_INTERNAL(board_select),
		DECLARE_INTERNAL(ent_bnet),
		DECLARE_INTERNAL(ent_bnet2),
		DECLARE_INTERNAL(ent_winmine),
		DECLARE_INTERNAL(fill_reg_form),
		DECLARE_INTERNAL(giveUpBBS),
		DECLARE_INTERNAL(g_send),
		DECLARE_INTERNAL(mailall),
		DECLARE_INTERNAL(m_internet),
		DECLARE_INTERNAL(m_new),
		DECLARE_INTERNAL(m_read),
		DECLARE_INTERNAL(m_send),
		DECLARE_INTERNAL(msg_more),
		DECLARE_INTERNAL(m_vote),
		DECLARE_INTERNAL(offline),
		DECLARE_INTERNAL(ov_send),
		DECLARE_INTERNAL(post_list_mention),
		DECLARE_INTERNAL(post_list_reply),
		DECLARE_INTERNAL(shownotepad),
		DECLARE_INTERNAL(show_online_followings),
		DECLARE_INTERNAL(show_online_users),
		DECLARE_INTERNAL(s_msg),
		DECLARE_INTERNAL(tui_all_boards),
		DECLARE_INTERNAL(tui_black_list),
		DECLARE_INTERNAL(tui_favorite_boards),
		DECLARE_INTERNAL(tui_following_list),
		DECLARE_INTERNAL(tui_my_props),
		DECLARE_INTERNAL(tui_props),
		DECLARE_INTERNAL(tui_query),
		DECLARE_INTERNAL(tui_read_sector),
		DECLARE_INTERNAL(tui_unread_boards),
		DECLARE_INTERNAL(x_cloak),
		DECLARE_INTERNAL(x_edits),
		DECLARE_INTERNAL(x_info),
		DECLARE_INTERNAL(x_lockscreen),
		DECLARE_INTERNAL(x_results),
		DECLARE_INTERNAL(x_userdefine),
		DECLARE_INTERNAL(x_vote),
	};

	internal_func_t *ptr = bsearch(cmd, internal, ARRAY_SIZE(internal),
			sizeof(internal[0]), internal_function_compare);
	if (ptr) {
		ptr->func(descr);
	}
}

static void menu_execute(const char *cmd, const char *descr)
{
	if (cmd && *cmd == '@') {
		if (strchr(cmd, ':')) {
			char buf[128];
			strlcpy(buf, cmd, sizeof(buf));

			char *ptr = strchr(buf, ':');
			if (!ptr)
				return;
			*ptr = '\0';
			++ptr;

			void *hdll = dlopen(buf + 1, RTLD_LAZY);
			if (hdll) {
				menu_function_t func;
				*(void **) &func = dlsym(hdll, ptr);
				if (func)
					func();
				dlclose(hdll);
			}
		} else {
			function_eval(cmd + 1, descr);
		}
	}
}

typedef struct {
	int line;
	int col;
	int sign;
	int found;
} sibling_item_arg_t;

static int sibling_item_callback(const menu_item_t *item, void *a,
		int offset)
{
	sibling_item_arg_t *arg = a;
	if (arg->sign * (item->col - arg->col) < 0) {
		arg->found = offset;
		return -1;
	}
	return 0;
}

static int sibling_item(int first_item, int items, int line, int col,
		bool left)
{
	sibling_item_arg_t arg = {
		.line = line,
		.col = col,
		.sign = left ? 1 : -1,
		.found = first_item + items,
	};
	foreach_item(first_item, first_item + items, sibling_item_callback, &arg);
	return arg.found;
}

static int next_item(int begin, int end, int now, int delta)
{
	while (1) {
		if (now >= end && delta > 0)
			now = begin;
		if (now < begin && delta < 0)
			now = end - 1;

		menu_item_t item;
		if (!read_item(now, &item))
			break;

		menu_value_t perm = menu_get_const(item.perm);
		if (HAS_PERM(perm))
			break;
		else
			now += delta;
	}
	return now;
}

typedef struct {
	int found;
	int now;
	int ch;
} search_item_arg_t;

static int search_item_callback(const menu_item_t *item, void *a, int offset)
{
	search_item_arg_t *arg = a;
	const char *descr = menu_get_string(item->descr);
	if (descr && toupper(*descr) == arg->ch) {
		if (offset > arg->now) {
			arg->found = offset;
			return -1;
		} else if (arg->found == -1) {
			arg->found = offset;
		}
	}
	return 0;
}

static int search_item(int begin, int end, int now, int ch)
{
	search_item_arg_t arg = {
		.found = -1,
		.now = now,
		.ch = toupper(ch),
	};
	foreach_item(begin, end, search_item_callback, &arg);
	return arg.found >= begin && arg.found < end ? arg.found : now;
}

int menu_loop(const char *group_name)
{
	static char level = 0;
	if (++level > MENU_LEVEL_MAX)
		goto r;

	int offset = find_group(group_name);
	if (offset < 0 || offset >= _menu.group_count)
		goto r;

	const char *ptr = group_base() + offset * MENU_GROUP_RECORD_SIZE;
	menu_offset_t first_item =
			menu_read_offset(ptr + sizeof(menu_offset_t));
	int items = ptr[sizeof(menu_offset_t) * 2];

	menu_offset_t begin = first_item + 2, end = first_item + items;

	bool top = streq(group_name, "TOPMENU");
	int now = draw_group(first_item, items, top);
	bool refresh = true;

	while (1) {
		set_user_status(ST_MMENU);
		if (refresh) {
			active_board_show();
			refresh = false;
		}

		int line, col;
		get_coordination(first_item, now, &line, &col);

		move(line, col);
		screen_printf(">");

		int ch = egetch();

		move(line, col);
		screen_printf(" ");

		menu_item_t item;
		if (!read_item(now, &item))
			goto r;

		switch (ch) {
			case KEY_RIGHT: {
				int found = sibling_item(first_item, items, line, col, false);
				if (found >= begin && found < end) {
					now = found;
					break;
				}
				// fall through
			}
			case '\n': case '\r': {
				const char *cmd = menu_get_string(item.cmd);
				if (!cmd || streq(cmd, "!.."))
					goto r;
				if (*cmd == '!') {
					menu_loop(cmd + 1);
				} else {
					menu_execute(cmd, menu_get_string(item.descr));
				}
				draw_group(first_item, items, top);
				refresh = true;
				break;
			}
			case KEY_LEFT: {
				int found = sibling_item(first_item, items, line, col, false);
				if (found >= begin && found < end) {
					now = found;
					break;
				} else {
					goto r;
				}
			}
			case KEY_DOWN:
				now = next_item(begin, end, now + 1, 1);
				break;
			case KEY_UP:
				now = next_item(begin, end, now - 1, -1);
				break;
			case KEY_PGUP:
				now = next_item(begin, end, begin, 1);
				break;
			case KEY_PGDN:
				now = next_item(begin, end, end - 1, -1);
				break;
			case '!':
				goto r;
			case Ctrl('T'):
				if (tui_check_notice(NULL) == FULLUPDATE) {
					draw_group(first_item, items, top);
					active_board_show();
				} else {
					tui_update_status_line();
				}
				break;
			default:
				now = search_item(begin, end, now, ch);
				break;
		}
	}

r:	--level;
	return 0;
}
