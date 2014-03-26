#ifndef FB_TUI_LIST_H
#define FB_TUI_LIST_H

#include <stdbool.h>
#include "fbbs/list.h"

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

#endif // FB_TUI_LIST_H
