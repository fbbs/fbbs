#ifndef FB_MAIL_H
#define FB_MAIL_H

#include <stdbool.h>

extern int getmailboxsize(unsigned int userlevel);
extern int getmailboxhold(unsigned int userlevel);
extern int getmailsize(const char *user);
extern int getmailnum(const char *userid);
extern int do_mail_file(const char *recv, const char *title,
		const char *header, const char *text, int len, const char *source);
extern int mail_file(const char *file, const char *recv, const char *title);
extern bool valid_addr(const char *addr);

#endif // FB_MAIL_H
