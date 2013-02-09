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

extern int tui_list(tui_list_t *p);

typedef enum {
	SLIDE_LIST_CURRENT = 0,
	SLIDE_LIST_TOPDOWN,
	SLIDE_LIST_PREV,
	SLIDE_LIST_NEXT,
	SLIDE_LIST_BOTTOMUP,
} slide_list_base_e;

typedef int slide_list_loader_t;
typedef void slide_list_title_t;
typedef int slide_list_display_t;
typedef int slide_list_handler_t;

typedef struct slide_list_t {
	int cur;
	int max;
	int update;
	slide_list_base_e base;
	bool in_query;
	void *data;
	int (*loader)(struct slide_list_t *);
	void (*title)(struct slide_list_t *);
	int (*display)(struct slide_list_t *);
	int (*handler)(struct slide_list_t *, int);
} slide_list_t;

extern int slide_list(slide_list_t *p);

enum {
	TUI_LIST_POS_KEY_LEN = 16,
};

typedef struct tui_list_pos_t {
	char key[TUI_LIST_POS_KEY_LEN];
	int top;
	int cursor;
	SLIST_FIELD(tui_list_pos_t) next;
} tui_list_pos_t;

SLIST_HEAD(tui_list_pos_list_t, tui_list_pos_t);

#endif // FB_TUI_LIST_H
