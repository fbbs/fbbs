#ifndef FB_REGISTER_H
#define FB_REGISTER_H

#include <stdbool.h>

enum {
	BBS_EREG_NONALPHA = -1,
	BBS_EREG_SHORT = -2,
	BBS_EREG_BADNAME = -3,
};

bool is_no_register(void);
int check_userid(const char *userid);

#endif // FB_REGISTER_H
