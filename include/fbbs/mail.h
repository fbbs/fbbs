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

int chkmail(void);
int doforward(const char *direct, const struct fileheader *fh, bool uuencode);
int do_send(const char *userid, const char *title);
int check_query_mail(const char *qry_mail_dir);
int mail_forward(int ent, struct fileheader *fileinfo, const char *direct);
int mail_u_forward(int ent, struct fileheader *fileinfo, const char *direct);
int m_read(void);
int sharedmail_file(const char *tmpfile, const char *userid, const char *title);
int mail_mark(int ent, struct fileheader *fileinfo, char *direct);
int tui_forward(const char *file, const char *gbk_title, bool uuencode);

#endif // FB_MAIL_H
