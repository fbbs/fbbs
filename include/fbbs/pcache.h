#ifndef FB_PCACHE_H
#define FB_PCACHE_H

#include "fbbs/post.h"
#include "fbbs/tui_list.h"

/** @addtogroup plist_cache */
/** @{ */
typedef struct {
	post_info_t *posts;  ///< Array to hold posts in.
	int begin;  ///< Index of first visible post.
	int end;  ///< Off-the-end index of last visible post.
	int count;  ///< Number of posts in the cache.
	int scount;  ///< Number of sticky posts in the cache.
	int capacity;  ///< Maximum number of posts.
	int scapacity;  ///< Maximum number of sticky posts.
	int page;  ///< Count of posts in a page.
	post_id_t top;  ///< The oldest post id.
	post_id_t bottom;  ///< The newest post id.
	bool sticky;  ///< True if sticky posts support is on.
} plist_cache_t;
/** @} */

extern void plist_cache_clear(plist_cache_t *c);
extern void plist_cache_init(plist_cache_t *c, int page, int scapacity);
extern post_info_t *plist_cache_get(const plist_cache_t *c, int pos);
extern int plist_cache_max_visible(const plist_cache_t *c);
extern bool plist_cache_is_top(const plist_cache_t *c, int pos);
extern bool plist_cache_is_bottom(const plist_cache_t *c, int pos);
extern void plist_cache_set_sticky(plist_cache_t *c, const post_filter_t *fp);
extern int plist_cache_load(plist_cache_t *c, post_filter_t *filter, slide_list_base_e base, bool force);
extern void plist_cache_load_sticky(plist_cache_t *c, post_filter_t *filter, bool force);
extern int plist_cache_relocate(plist_cache_t *c, int current, post_filter_t *filter, bool upward);
extern void plist_cache_free(plist_cache_t *c);

#endif
