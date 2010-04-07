#ifndef FB_LIST_H
#define FB_LIST_H

typedef struct choose_t {
	int all;       ///< Number of entries.
	int cur;       ///< Current entry.
	int start;     ///< Starting entry of the page.
	int update;    ///< UI update status.
	int valid;     ///< True if data are not out-dated.
	void *data;    ///< Data.
	void (*title)(struct choose_t *);   ///< Function that shows title.
	int (*display)(struct choose_t *);  ///< Display function.
	int (*handler)(struct choose_t *);  ///< Key handler.
} choose_t;

extern int choose2(choose_t *cp);

#endif // FB_LIST_H
