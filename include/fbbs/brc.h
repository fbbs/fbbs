#ifndef FB_BRC_H
#define FB_BRC_H

#include <stdint.h>

extern void brc_update(const char *userid, const char *board);
extern int brc_initial(const char* userid, const char *board);
extern void brc_addlist(const char *filename);

extern bool brc_unread_legacy(const char *filename);
extern bool brc_unread(int64_t id);

extern int brc_clear(int ent, const char *direct, int clearall);
extern void brc_zapbuf(int *zbuf);
extern int brc_fcgi_init(const char *user, const char *board);
extern bool brc_board_unread(const char *user, const char *bname, int bid);

#endif // FB_BRC_H
