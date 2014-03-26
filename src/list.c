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
