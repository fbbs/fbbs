#ifndef FB_REGISTER_H
#define FB_REGISTER_H

#include <stdbool.h>

enum {
	BBS_EREG_NONALPHA, BBS_EREG_SHORT, BBS_EREG_BADNAME
};

bool is_no_register(void);
int check_userid(const char *userid);

#endif // FB_REGISTER_H
