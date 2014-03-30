#include "bbs.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

enum {
	TUI_LIST_START = 3,
};

static void tui_list_init(tui_list_t *p)
{
	p->lines = screen_lines() - 4;
	p->all = p->cur = p->begin = 0;
	p->update = FULLUPDATE;
	p->valid = false;
	p->in_query = false;
}

static int tui_list_display_loop(tui_list_t *p)
{
	int end = p->begin + p->lines;
	if (end > p->all)
		end = p->all;
	for (int i = p->begin; i < end; ++i) {
		if (p->display(p, i) == -1)
			return -1;
	}
	return 0;
}

static void adjust_window(tui_list_t *tl)
{
	if (tl->cur < tl->begin || tl->cur >= tl->begin + tl->lines) {
		tl->begin = (tl->cur / tl->lines) * tl->lines;
		if (tl->update != FULLUPDATE)
			tl->update = PARTUPDATE;
	}
}

int tui_list_seek(tui_list_t *tl, int operation, bool invalidate, bool loop)
{
	switch (operation) {
		case KEY_PGUP:
			tl->begin -= tl->lines - 1;
			if (tl->begin < 0)
				tl->begin = 0;
			tl->cur = tl->begin;
			if (invalidate)
				tl->valid = false;
			return PARTUPDATE;
		case KEY_UP:
			if (--tl->cur >= tl->begin)
				return DONOTHING;
			if (invalidate)
				tl->valid = false;
			if (tl->cur >= 0) {
				tl->begin -= tl->lines - 1;
			} else {
				if (loop) {
					tl->cur = tl->all - 1;
					if (tl->cur < 0)
						tl->cur = 0;
					tl->begin = tl->all - tl->lines + 1;
				} else {
					tl->cur = 0;
					return DONOTHING;
				}
			}
			if (tl->begin < 0)
				tl->begin = 0;
			return PARTUPDATE;
		case KEY_PGDN:
			if (tl->begin + tl->lines - 1 >= tl->all) {
				tl->cur = tl->all - 1;
				return DONOTHING;
			}
			tl->cur = tl->begin += tl->lines - 1;
			if (invalidate)
				tl->valid = false;
			return PARTUPDATE;
		case KEY_DOWN:
			if (++tl->cur >= tl->all) {
				if (loop) {
					tl->begin = tl->cur = 0;
					if (invalidate)
						tl->valid = false;
					return PARTUPDATE;
				} else {
					--tl->cur;
					return DONOTHING;
				}
			} else if (tl->cur >= tl->begin + tl->lines - 1) {
				tl->begin = tl->cur;
				if (invalidate)
					tl->valid = false;
				return PARTUPDATE;
			} else {
				return DONOTHING;
			}
	}
	return DONOTHING;
}

int tui_list(tui_list_t *p)
{
	p->jump = 0;
	bool end = false;

	tui_list_init(p);
	
	while (!end) {
		adjust_window(p);
		if (!p->in_query && !p->valid) {
			if (p->loader(p) < 0)
				break;
			p->valid = true;
			if (p->update != FULLUPDATE)
				p->update = PARTUPDATE;
		}

		if (p->cur >= p->all)
			p->cur = p->all - 1;
		if (p->cur < 0)
			p->cur = 0;
		adjust_window(p);

		if (p->update != DONOTHING) {
			if (p->update == FULLUPDATE) {
				screen_clear();
				p->title(p);
				p->update = PARTUPDATE;
			}
			if (p->update == PARTUPDATE) {
				move(TUI_LIST_START, 0);
				screen_clrtobot();
				tui_list_display_loop(p);
			}
			tui_update_status_line();
			p->update = DONOTHING;
		}

		if (!p->in_query) {
			move(TUI_LIST_START + p->cur - p->begin, 0);
			outs(">");
		}

		int ch = terminal_getchar();

		if (!p->in_query) {
			move(TUI_LIST_START + p->cur - p->begin, 0);
			outs(" ");
		}

		int ret = p->handler(p, ch);
		if (ret < 0)
			break;
		else if (ret != READ_AGAIN) {
			p->update = ret;
			continue;
		}

		switch (ch) {
			case 'q': case 'e': case KEY_LEFT: case EOF:
				if (p->in_query) {
					p->in_query = false;
					p->update = FULLUPDATE;
				} else {
					end = true;
				}
				break;
			case 'b': case Ctrl('B'): case KEY_PGUP:
				if (p->cur == 0)
					p->cur = p->all - 1;
				else
					p->cur -= p->lines;
				break;
			case 'N': case Ctrl('F'): case KEY_PGDN: case ' ':
				if (p->cur == p->all - 1)
					p->cur = 0;
				else
					p->cur += p->lines;
				break;
			case 'k': case KEY_UP:
				if (--p->cur < 0)
					p->cur = p->all - 1;
				if (p->in_query && p->query)
					p->query(p);
				break;
			case 'j': case KEY_DOWN:
				++p->cur;
				if (p->cur >= p->all)
					p->cur = 0;
				if (p->in_query && p->query)
					p->query(p);
				break;
			case '$': case KEY_END:
				p->cur = p->all - 1;
				break;
			case KEY_HOME:
				p->cur = 0;
				break;
			case '\n': case '\r':
				if (p->jump > 0) {
					p->cur = p->jump - 1;
					p->jump = 0;
					break;
				}
				// fall through
			default:
				if (ch >= '0' && ch <= '9') {
					p->jump = p->jump * 10 + (ch - '0');
					ch = '\0';
				} else {
					p->jump = 0;
					ret = p->handler(p, ch);
					if (ret < 0)
						end = true;
					else
						p->update = ret;
				}
				break;
		}
	}
	return 0;
}

typedef struct {
	bool all_loaded;
	user_id_t user_id;
	tui_list_recent_loader_t loader;
	int (*handler)(tui_list_t *, int);
	vector_t vector;
} tui_list_recent_helper_t;

static tui_list_loader_t tui_list_recent_loader(tui_list_t *tl)
{
	tui_list_recent_helper_t *helper = tl->data;
	if (helper->all_loaded)
		return 0;

	vector_t *v = &helper->vector;

	int64_t id = INT64_MAX;
	if (vector_size(v))
		id = *((int64_t *) vector_at(v, vector_size(v) - 1));

	void *buf = vector_grow(&helper->vector, tl->lines);
	if (!buf)
		return -1;

	int rows = helper->loader(helper->user_id, id, buf, tl->lines);
	if (rows < tl->lines) {
		vector_size_t diff = tl->lines - rows, size = vector_size(v);
		vector_erase_range(v, size - diff, size);
		helper->all_loaded = true;
	}
	tl->all = vector_size(v);
	tl->valid = true;
	return rows;
}

static void load_more(tui_list_t *tl)
{
	int all = tl->all;
	tui_list_recent_loader(tl);
	int diff = tl->all > all;
	if (diff > 0) {
		tl->begin += diff;
		tl->cur += diff;
	}
}

static tui_list_handler_t tui_list_recent_handler(tui_list_t *tl, int key)
{
	tui_list_recent_helper_t *helper = tl->data;
	if (helper->handler(tl, key) == READ_AGAIN) {
		switch (key) {
			case 'k': case KEY_UP:
				if (tl->cur == 0)
					load_more(tl);
				return READ_AGAIN;
			case 'b': case Ctrl('B'): case KEY_PGUP:
				if (tl->cur < tl->lines - 1)
					load_more(tl);
				return READ_AGAIN;
		}
	}
	return READ_AGAIN;
}

int tui_list_recent(tui_list_recent_t *tlr)
{
	int lines = screen_lines() - 4;

	tui_list_recent_helper_t helper = {
		.user_id = tlr->user_id,
		.loader = tlr->loader,
		.handler = tlr->handler,
	};
	if (!vector_init(&helper.vector, tlr->len, lines))
		return 0;

	tui_list_t tl = {
		.lines = lines,
		.data = &helper,
		.loader = tui_list_recent_loader,
		.title = tlr->title,
		.display = tlr->display,
		.handler = tui_list_recent_handler,
		.query = tlr->query,
	};
	tui_list(&tl);

	vector_free(&helper.vector);
	return 0;
}

void *tui_list_recent_get_data(tui_list_t *tl, int n)
{
	tui_list_recent_helper_t *helper = tl->data;
	vector_t *v = &helper->vector;
	return vector_at(v, tl->all - n - 1);
}
