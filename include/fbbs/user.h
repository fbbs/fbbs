#ifndef FB_USER_H
#define FB_USER_H

#include "fbbs/dbi.h"

#define has_permission(p, x)  ((x) ? p & (x) : 1)

enum {
	USER_FIELD_LOGINS = 0,
	USER_FIELD_POSTS,
	USER_FIELD_STAY,
};

extern user_id_t get_user_id(const char *name);
extern int get_user_count(void);
extern int user_data_add_by_name(const char *name, int field, int delta);
extern int user_data_add(int uid, int field, int delta);

extern int calc_user_stay(bool is_login, bool is_dup, time_t login, time_t logout);

extern int set_last_post_time(fb_time_t t);
extern fb_time_t get_last_post_time(void);
extern int set_doc_mode(int mode);
extern int get_doc_mode(void);

#endif // FB_USER_H
