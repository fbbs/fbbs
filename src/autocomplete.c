#include <string.h>
#include <strings.h>
#include "bbs.h"
#include "fbbs/autocomplete.h"
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

typedef struct ac_name_list {
	const char *name;
	struct ac_name_list *next;
} ac_name_list;

struct ac_list {
	pool_t *pool;
	ac_name_list *head;
	ac_name_list *tail;
	ac_name_list **col;
	ac_name_list *seek;
	const char *match;
	int extra;
	int xbase;
	int ybase;
};

ac_list *ac_list_new(void)
{
	pool_t *p = pool_create(0);
	ac_list *acl = pool_alloc(p, sizeof(*acl));
	acl->pool = p;
	acl->head = NULL;
	acl->tail = NULL;
	acl->col = NULL;
	acl->seek = NULL;
	return acl;
}

void ac_list_add(ac_list *acl, const char *name)
{
	ac_name_list *l = pool_alloc(acl->pool, sizeof(*l));
	l->name = pool_strdup(acl->pool, name, 0);
	l->next = NULL;

	if (acl->tail)
		acl->tail->next = l;
	acl->tail = l;

	if (!acl->head)
		acl->head = l;
}

void ac_list_free(ac_list *acl)
{
	if (acl && acl->pool)
		pool_destroy(acl->pool);
}

static ac_name_list *next_match(ac_name_list *l, const char *prefix)
{
	size_t len = strlen(prefix);
	for (; l; l = l->next) {
		if (strncaseeq(prefix, l->name, len)) {
			return l;
		}
	}
	return NULL;
}

static const char *best_match(ac_name_list *l, const char *prefix)
{
	ac_name_list *first = next_match(l, prefix);
	for (l = first; l; l = l->next) {
		if (strcaseeq(prefix, l->name))
			return l->name;
	}
	return first ? first->name : NULL;
}

static int _autocomplete(ac_list *acl, char *buf, size_t size)
{
	int rows = t_lines - acl->ybase - 2, extra = 0;
	const char *base = NULL;

	if (!acl->seek || acl->seek == acl->head) {
		ac_name_list *l1 = next_match(acl->head, buf);
		if (!l1)
			return 0;

		ac_name_list *l2 = next_match(l1->next, buf);
		if (!l2) {
			extra = strlen(l1->name) - strlen(buf);
			strlcpy(buf, l1->name, size);
			return extra;
		}

		base = l1->name;
		extra = strlen(base) - strlen(buf);

		if (!acl->col) {
			acl->col = pool_alloc(acl->pool, sizeof(*acl->col) * rows);
		}
	}

	move(acl->ybase + 1, 0);
	clrtobot();
	printdash(" 列表 ");

	const int columns = 80;
	int xbase = 0, width = 0, count = 0;
	if (!acl->seek)
		acl->seek = acl->head;
	size_t len = strlen(buf);

	for (ac_name_list *l = acl->seek; l; l = l->next) {
		if (strncaseeq(l->name, buf, len)) {
			acl->col[count++] = l;

			int w = strlen(l->name);
			if (w > width)
				width = w;
			if (extra) {
				if (extra > w - len)
					extra = w - len;
				for (int i = len; i < len + extra; ++i) {
					if (base[i] != l->name[i]) {
						extra = i - len;
						break;
					}
				}
			}
		}

		if (count == rows - 1 || !l->next) {
			if (xbase + width > columns) {
				acl->seek = *acl->col;
				move(t_lines - 1, 0);
				prints("\033[1m -- 还有 --\033[m");
				break;
			} else {
				acl->seek = NULL;
				for (int i = 0; i < count; ++i) {
					move(acl->ybase + i + 2, xbase);
					outs(acl->col[i]->name);
				}
				move(t_lines - 1, 0);
				clrtoeol();
				xbase += width + 1;
				width = 0;
				count = 0;
			}
		}
	}

	if (len + extra >= size - 1)
		extra = size - 1 - len;
	if (extra > 0)
		strlcpy(buf + len, (*acl->col)->name + len, extra + 1);
	return extra;
}

void autocomplete(ac_list *acl, const char *prompt, char *buf, size_t size)
{
	if (prompt) {
		prints("%s", prompt);
		clrtoeol();
	}

	getyx(&acl->ybase, &acl->xbase);

	buf[0] = '\0';
	size_t len = 0;
	int ch;
	while ((ch = igetkey())) {
		switch (ch) {
			case '\n':
			case '\r':
				{
					if (!*buf)
						return;
					const char *match = best_match(acl->head, buf);
					if (match)
						strlcpy(buf, match, size);
				}
				return;
			case ' ':
			case KEY_TAB:
				len += _autocomplete(acl, buf, size);
				move(acl->ybase, acl->xbase);
				outs(buf);
				break;
			case Ctrl('H'):
			case KEY_BACKSPACE:
				if (len) {
					buf[--len] = '\0';
					move(acl->ybase, acl->xbase + len);
					outc(' ');
					move(acl->ybase, acl->xbase + len);
					acl->seek = NULL;
				}
				break;
			default:
				if (len < size - 1) {
					buf[len++] = ch;
					buf[len] = '\0';
					if (next_match(acl->head, buf)) {
						outc(ch);
					} else {
						buf[--len] = '\0';
						move(acl->ybase, acl->xbase + len);
						outc(' ');
						move(acl->ybase, acl->xbase + len);
					}
				}
				acl->seek = NULL;
				break;
		}
	}
}
