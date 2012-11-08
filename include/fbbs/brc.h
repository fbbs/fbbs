#ifndef FB_BRC_H
#define FB_BRC_H

#include <stdint.h>

extern void brc_update(const char *userid, const char *board);
extern int brc_initial(const char* userid, const char *board);

extern void brc_addlist_legacy(const char *filename);
extern bool brc_unread_legacy(const char *filename);

extern void brc_mark_as_read(int64_t id);
extern bool brc_unread(int64_t id);
extern int brc_first_unread(void);
extern int brc_last_read(void);
extern void brc_clear(int64_t id);
extern void brc_clear_all(void);

extern void brc_zapbuf(int *zbuf);
extern int brc_fcgi_init(const char *user, const char *board);
extern bool brc_board_unread(const char *user, const char *bname, int bid);

#endif // FB_BRC_H
