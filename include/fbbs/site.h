#ifndef FB_SITE_H
#define FB_SITE_H

#define TESTING

#define BBSHOME "/home/bbs"

enum {
	BBSUID = 9999,
	BBSGID = 9999,
};

#ifdef TESTING
enum {
    MAX_USERS = 25000,
    MAX_BOARDS = 1000,
    MAX_ACTIVE = 1000,
};
#endif

#endif // FB_SITE_H

