#ifndef FB_SOCKET_H
#define FB_SOCKET_H

#define BBSD_SOCKET_BASE BBSHOME"/tmp/bbsd-socket"

extern int unix_dgram_connect(const char *basename, const char *server);
extern int unix_dgram_bind(const char *name, int qlen);

#endif // FB_SOCKET_H
