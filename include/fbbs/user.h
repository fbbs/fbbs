#ifndef FB_USER_H
#define FB_USER_H

#include <inttypes.h>
#include <stdint.h>

#define PRIdUID  PRId32
#define DBIdUID  "d"
typedef int32_t user_id_t;
#define db_get_user_id(res, row, col)  db_get_integer(res, row, col)

#define has_permission(p, x)  ((x) ? p & (x) : 1)

enum {
	USER_FIELD_LOGINS = 0,
	USER_FIELD_POSTS,
	USER_FIELD_STAY,
};

extern void remove_user_id_cache(const char *uname);
extern user_id_t get_user_id(const char *name);
extern int get_user_count(void);
extern int user_data_add_by_name(const char *name, int field, int delta);
extern int user_data_add(int uid, int field, int delta);

extern int calc_user_stay(bool is_login, bool is_dup, time_t login, time_t logout);

extern int set_my_last_post_time(fb_time_t t);
extern fb_time_t get_my_last_post_time(void);

#endif // FB_USER_H
