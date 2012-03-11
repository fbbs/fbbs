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
extern int get_user_count(db_conn_t *c);
extern int user_data_add_by_name(db_conn_t *c, const char *name,
		int field, int delta);
extern int user_data_add(db_conn_t *c, int uid, int field, int delta);

extern int get_online_count(db_conn_t *c);

extern int calc_user_stay(bool is_login, bool is_dup, time_t login, time_t logout);

extern int set_last_post_time(user_id_t uid, fb_time_t t);
extern fb_time_t get_last_post_time(user_id_t uid);

#endif // FB_USER_H
