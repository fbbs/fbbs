#ifndef FB_TITLE_H
#define FB_TITLE_H

#include "dbi.h"

enum {
	TITLE_CCHARS = 15
};

extern bool title_check_existence(user_id_t uid);
extern bool title_submit_request(int type, const char *title);

#endif // FB_TITLE_H
