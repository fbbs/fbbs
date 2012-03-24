#ifndef FB_MSG_H
#define FB_MSG_H

extern int get_msg(const char *uid, char *msg, int line);
extern int s_msg(void);
extern int msg_author(int ent, const struct fileheader *fileinfo, char *direct);
extern int broadcast_msg(const char *msg);
extern int logout_msg(const char *msg);
extern int login_msg(void);
extern void msg_reply(int ch);
extern void msg_handler(int signum);

#endif // FB_MSG_H
