#ifndef FB_UCACHE_H
#define FB_UCACHE_H

enum {
	UCACHE_EINTNL = -1,
	UCACHE_EEXIST = -2,
	UCACHE_EFULL = -3,
};

extern int create_user(const struct userec *user);

#endif // FB_UCACHE_H
