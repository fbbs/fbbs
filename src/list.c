#include "bbs.h"
#include "record.h"
#include "fbbs/mail.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

enum {
	TUI_LIST_START = 3,
};

static void tui_list_init(tui_list_t *p)
{
	p->lines = t_lines - 4;
	p->all = p->cur = p->start = 0;
	p->update = FULLUPDATE;
	p->valid = false;
	p->in_query = false;
}

static int tui_list_display_loop(tui_list_t *p)
{
	int end = p->start + p->lines;
	if (end > p->all)
		end = p->all;
	for (int i = p->start; i < end; ++i) {
		if (p->display(p, i) == -1)
			return -1;
	}
	return 0;
}

int tui_list(tui_list_t *p)
{
	int number = 0;
	bool end = false;

	tui_list_init(p);
	
	while (!end) {
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

		if (p->cur < p->start || p->cur >= p->start + p->lines) {
			p->start = (p->cur / p->lines) * p->lines;
			if (p->update != FULLUPDATE)
				p->update = PARTUPDATE;
		}

		if (p->update != DONOTHING) {
			if (p->update == FULLUPDATE) {
				clear();
				p->title(p);
				p->update = PARTUPDATE;
			}
			if (p->update == PARTUPDATE) {
				move(TUI_LIST_START, 0);
				clrtobot();
				tui_list_display_loop(p);
			}
			update_endline();
			p->update = DONOTHING;
		}

		if (!p->in_query) {
			move(TUI_LIST_START + p->cur - p->start, 0);
			outs(">");
		}

		int ch = igetkey();

		if (!p->in_query) {
			move(TUI_LIST_START + p->cur - p->start, 0);
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
				if (number > 0) {
					p->cur = number - 1;
					number = 0;
					break;
				}
				// fall through
			default:
				if (ch >= '0' && ch <= '9') {
					number = number * 10 + (ch - '0');
					ch = '\0';
				} else {
					number = 0;
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

int slide_list(slide_list_t *p)
{
	bool end = false;

	while (!end) {
		if (p->loader(p) < 0)
			break;

		if (p->update != DONOTHING) {
			if (p->update == FULLUPDATE) {
				clear();
				p->title(p);
				p->update = PARTUPDATE;
			}
			if (p->update == PARTUPDATE) {
				move(TUI_LIST_START, 0);
				clrtobot();
				p->display(p);
			}
			update_endline();
			p->update = DONOTHING;
		}

		if (p->cur >= p->max)
			p->cur = p->max - 1;
		if (!p->in_query && p->cur >= 0) {
			move(TUI_LIST_START + p->cur, 0);
			outs(">");
		}

		int ch = igetkey();

		if (!p->in_query && p->cur >= 0) {
			move(TUI_LIST_START + p->cur, 0);
			outs(" ");
		}

		p->base = SLIDE_LIST_CURRENT;

		int ret = p->handler(p, ch);
		if (ret < 0)
			break;
		else if (ret != READ_AGAIN) {
			p->update = ret;
			continue;
		}

		switch (ch) {
			case 'q':
			case 'e':
			case KEY_LEFT:
			case EOF:
				if (p->in_query) {
					p->in_query = false;
					p->update = FULLUPDATE;
				} else {
					end = true;
				}
				break;
			case 'P':
			case Ctrl('B'):
			case KEY_PGUP:
				p->base = SLIDE_LIST_PREV;
				break;
			case 'N':
			case Ctrl('F'):
			case KEY_PGDN:
			case ' ':
				p->base = SLIDE_LIST_NEXT;
				break;
			case 'k':
			case KEY_UP:
				if (--p->cur < 0) {
					p->base = SLIDE_LIST_PREV;
					p->cur = t_lines - 5;
				}
				break;
			case 'j':
			case KEY_DOWN:
				if (++p->cur >= p->max) {
					p->base = SLIDE_LIST_NEXT;
					p->cur = 0;
				}
				break;
			case '$':
			case KEY_END:
				p->base = SLIDE_LIST_BOTTOMUP;
				p->cur = t_lines - 5;
				break;
			case KEY_HOME:
				p->base = SLIDE_LIST_TOPDOWN;
				p->cur = 0;
				break;
			default:
				break;
		}
	}
	return 0;
}
