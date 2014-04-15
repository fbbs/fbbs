#ifndef FB_BRC_H
#define FB_BRC_H

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t brc_item_t;

extern void brc_sync(const char *user_name);
extern int brc_init(const char *user_name, const char *board_name);
extern void brc_reset(void);

extern bool brc_mark_as_read(brc_item_t item);
extern bool brc_unread(brc_item_t item);
extern brc_item_t brc_last_read(void);
extern void brc_clear(brc_item_t item);
extern void brc_clear_all();

extern void brc_zapbuf(int *zbuf);
extern bool brc_board_unread(const char *user_name, const char *board_name, int board_id);

#endif // FB_BRC_H
