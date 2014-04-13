#ifndef FB_TUI_LIST_H
#define FB_TUI_LIST_H

#include <stdbool.h>
#include "fbbs/list.h"
#include "fbbs/user.h"
#include "fbbs/vector.h"

#define TUI_LIST_HELP(func, key)  func "[\033[1;32m" key "\033[m]"
#define TUI_LIST_HELP2(func, k1, k2)  func "[\033[1;32m" k1 "\033[m,\033[1;32m" k2 "\033[m]"

typedef int tui_list_loader_t;
typedef void tui_list_title_t;
typedef int tui_list_display_t;
typedef int tui_list_handler_t;
typedef int tui_list_query_t;

typedef struct tui_list_t {
	int all;       ///< Number of entries.
	int cur;       ///< Current entry.
	int jump;      ///< The entry number to jump to.
	int begin;     ///< Starting entry of the page.
	int update;    ///< UI update status.
	int valid;     ///< True if data are not out-dated.
	int lines;     ///< Number of available lines for displaying content.
	bool in_query; ///< True if in query mode.
	void *data;    ///< Data.
	int (*loader)(struct tui_list_t *);   ///< Data loader.
	void (*title)(struct tui_list_t *);   ///< Function that shows title.
	int (*display)(struct tui_list_t *, int);  ///< Display function.
	int (*handler)(struct tui_list_t *, int);  ///< Key handler.
	int (*query)(struct tui_list_t *); ///< Query handler.
} tui_list_t;

extern int tui_list_seek(tui_list_t *tl, int operation, bool invalidate, bool loop);
extern int tui_list(tui_list_t *p);

typedef int (*tui_list_recent_loader_t)(user_id_t user_id, int64_t id, void *buf, size_t size);

typedef struct {
	user_id_t user_id;
	tui_list_recent_loader_t loader;
	vector_size_t len;
	void (*title)(struct tui_list_t *);
	int (*display)(struct tui_list_t *, int);
	int (*handler)(struct tui_list_t *, int);
	int (*query)(struct tui_list_t *);
	int (*deleter)(user_id_t, void *);
	void (*finalizer)(user_id_t, void *);
} tui_list_recent_t;

extern int tui_list_recent(tui_list_recent_t *tlr);
extern void *tui_list_recent_get_data(tui_list_t *tl, int n);
extern int tui_list_recent_delete(tui_list_t *tl);

#endif // FB_TUI_LIST_H
