#define _GNU_SOURCE
#include "bbs.h"
#include "fbbs/pcache.h"

/** @defgroup plist_cache Post List Cache */
/** @{ */

void plist_cache_clear(plist_cache_t *c)
{
	c->begin = c->end = c->count = 0;
	c->top = c->bottom = 0;
	c->sticky = false;
}

void plist_cache_init(plist_cache_t *c, int page, int scapacity)
{
	if (!c->posts) {
		c->scount = -1;
		c->page = page;
		c->capacity = page * 3;
		c->scapacity = scapacity;
		c->posts = malloc(sizeof(*c->posts) * (c->capacity + c->scapacity));
		plist_cache_clear(c);
	}
}

post_info_t *plist_cache_get(const plist_cache_t *c, int pos)
{
	if (pos >= 0) {
		if (c->begin + pos < c->end)
			return c->posts + c->begin + pos;
		pos -= c->end - c->begin;
		if (c->sticky && pos < c->scount)
			return c->posts + c->capacity + pos;
	}
	return NULL;
}

static post_info_t *plist_cache_get_non_sticky(const plist_cache_t *c,
		int pos)
{
	while (1) {
		post_info_t *p = plist_cache_get(c, pos);
		if (!p || !(p->flag & POST_FLAG_STICKY))
			return p;
		if (--pos < 0)
			break;
	}
	return NULL;
}

bool plist_cache_is_top(const plist_cache_t *c, int pos)
{
	if (!c->top)
		return false;
	if (pos < 0)
		pos = 0;
	post_info_t *p = plist_cache_get(c, pos);
	return p && p->id <= c->top;
}

bool plist_cache_is_bottom(const plist_cache_t *c, int pos)
{
	if (!c->bottom)
		return false;
	post_info_t *p = NULL;
	if (pos < 0)
		pos = c->page - 1;
	p = plist_cache_get_non_sticky(c, pos);
	return p && p->id >= c->bottom;
}

int plist_cache_max_visible(const plist_cache_t *c)
{
	int max = c->end - c->begin;
	if (c->sticky && plist_cache_is_bottom(c, -1) && c->end == c->count)
		max += c->scount;
	return max > c->page ? c->page : max;
}

static void adjust_window(plist_cache_t *c, int delta)
{
	c->begin += delta;
	c->end += delta;
	if (c->begin < 0)
		c->begin = 0;
	if (c->end > c->count)
		c->end = c->count;
	if (c->end - c->begin < c->page) {
		if (delta > 0) {
			if (c->sticky && c->bottom
					&& c->posts[c->end - 1].id >= c->bottom) {
				if (c->end - c->begin < c->page - c->scount
						|| c->begin >= c->end)
					c->begin = c->end + c->scount - c->page;
			} else {
				c->begin = c->end - c->page;
			}
			if (c->begin < 0)
				c->begin = 0;
		} else {
			c->end = c->begin + c->page;
			if (c->end > c->count)
				c->end = c->count;
		}
	}
}

static bool cached_top(const plist_cache_t *c)
{
	return c->top && c->posts[0].id <= c->top;
}

static bool cached_bottom(const plist_cache_t *c)
{
	return c->bottom && c->posts[c->count - 1].id >= c->bottom;
}

static bool already_cached(plist_cache_t *c, slide_list_base_e base)
{
	if (base == SLIDE_LIST_TOPDOWN && cached_top(c))
		adjust_window(c, -c->capacity);
	else if (base == SLIDE_LIST_BOTTOMUP && cached_bottom(c))
		adjust_window(c, c->capacity);
	else if (base == SLIDE_LIST_PREV && (c->begin >= c->page || cached_top(c)))
		adjust_window(c, -c->page);
	else if (base == SLIDE_LIST_NEXT &&
			(c->count - c->end >= c->page || cached_bottom(c)))
		adjust_window(c, c->page);
	else
		return false;
	return true;
}

static bool is_asc(slide_list_base_e base)
{
	return (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_NEXT);
}

static void set_window(plist_cache_t *c, slide_list_base_e base)
{
	if (base == SLIDE_LIST_TOPDOWN) {
		adjust_window(c, -c->capacity);
	} else if (base == SLIDE_LIST_BOTTOMUP) {
		adjust_window(c, c->capacity + c->scapacity);
	} else {
		adjust_window(c, is_asc(base) ? c->page : -c->page);
	}
}

static int load_posts_from_db(plist_cache_t *c, post_filter_t *filter,
		slide_list_base_e base, int limit)
{
	bool asc = is_asc(base);

	query_t *q = build_post_query(filter, asc, limit);
	db_res_t *res = query_exec(q);

	int rows = db_res_rows(res);
	int extra = rows + c->count - c->capacity;
	if (extra < 0)
		extra = 0;
	if (asc) {
		memmove(c->posts, c->posts + extra,
				sizeof(*c->posts) * (c->count - extra));
		c->begin -= extra;
		c->end -= extra;
		for (int i = 0; i < rows; ++i) {
			res_to_post_info(res, i, filter->archive,
					c->posts + c->count - extra + i);
		}
	} else {
		memmove(c->posts + extra, c->posts,
				sizeof(*c->posts) * (c->count - extra));
		c->begin += extra;
		c->end += extra;
		for (int i = 0; i < rows; ++i) {
			res_to_post_info(res, i, filter->archive,
					c->posts + rows - i - 1);
		}
	}
	c->count += rows - extra;
	if (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_BOTTOMUP
			|| (rows < limit && c->count >= 0)) {
		if (base == SLIDE_LIST_NEXT || base == SLIDE_LIST_BOTTOMUP)
			c->bottom = c->posts[c->count - 1].id;
		else
			c->top = c->posts[0].id;
	}

	set_window(c, base);
	return rows;
}

void plist_cache_set_sticky(plist_cache_t *c, const post_filter_t *fp)
{
	c->sticky = (fp->type == POST_LIST_NORMAL && !fp->archive);
}

extern void clear_filter(post_filter_t *filter);

int plist_cache_load(plist_cache_t *c, post_filter_t *filter,
		slide_list_base_e base, bool force)
{
	if (!force && already_cached(c, base))
		return 0;
	if (force)
		plist_cache_clear(c);

	int limit = 0;
	if (base == SLIDE_LIST_TOPDOWN || base == SLIDE_LIST_BOTTOMUP) {
		limit = c->capacity;
		plist_cache_clear(c);
	} else if (base == SLIDE_LIST_NEXT) {
		limit = c->capacity - (c->count - c->begin);
		if (c->count > 0) {
			filter->min = c->posts[c->count - 1].id + 1;
			if (filter->type == POST_LIST_THREAD)
				filter->tid = c->posts[c->count - 1].tid;
		}
	} else if (base == SLIDE_LIST_PREV) {
		limit = c->capacity - c->end;
		if (c->count > 0) {
			filter->max = c->posts[0].id - 1;
			if (filter->type == POST_LIST_THREAD)
				filter->tid = c->posts[0].tid;
		}
	}

	int rows = load_posts_from_db(c, filter, base, limit);
	if (rows < c->page && base == SLIDE_LIST_NEXT) {
		plist_cache_clear(c);
		clear_filter(filter);
		rows = load_posts_from_db(c, filter, SLIDE_LIST_BOTTOMUP, limit);
	}
	return rows;
}

void plist_cache_load_sticky(plist_cache_t *c, post_filter_t *filter,
		bool force)
{
	post_info_t *p = c->posts + c->capacity;
	if (force || (c->sticky && c->scount < 0))
		c->scount = load_sticky_posts(filter->bid, &p);
}

static bool match_filter(post_filter_t *filter, post_info_t *p)
{
	bool match = true;
	if (filter->uid)
		match &= p->uid == filter->uid;
	if (filter->min)
		match &= p->id >= filter->min;
	if (filter->max)
		match &= p->id <= filter->max;
	if (filter->tid)
		match &= p->tid == filter->tid;
	if (*filter->utf8_keyword)
		match &= (bool)strcasestr(p->utf8_title, filter->utf8_keyword);
	if (filter->flag)
		match &= (p->flag & filter->flag) == filter->flag;
	return match;
}

static int relocate_cursor(plist_cache_t *c, int cursor)
{
	if (cursor >= 0 && cursor < c->count) {
		while (cursor < c->begin)
			adjust_window(c, -c->page);
		while (cursor >= c->end)
			adjust_window(c, c->page);
	}
	return cursor - c->begin;
}

int plist_cache_relocate(plist_cache_t *c, int current, post_filter_t *filter,
		bool upward)
{
	int found = -1;

	int delta = upward ? -1 : 1;
	for (int i = c->begin + current + delta;
			i >= 0 && i < c->count;
			i += delta) {
		if (match_filter(filter, c->posts + i)) {
			found = i;
			break;
		}
	}

	if (found >= 0)
		return relocate_cursor(c, found);
	return found;
}

void plist_cache_free(plist_cache_t *c)
{
	free(c->posts);
}

/** @} */


