#ifndef FB_USER_H
#define FB_USER_H

#include <inttypes.h>
#include <stdint.h>
#include "bbs.h"
#include "fbbs/time.h"

#define PRIdUID  PRId32
#define DBIdUID  "d"
typedef int32_t user_id_t;
#define db_get_user_id(res, row, col)  db_get_integer(res, row, col)
#define parcel_write_user_id(parcel, id)  parcel_write_varint(parcel, id)
#define parcel_read_user_id(parcel)  parcel_read_varint(parcel)

#define has_permission(p, x)  ((x) ? p & (x) : 1)

enum {
	USER_FIELD_LOGINS = 0,
	USER_FIELD_POSTS,
	USER_FIELD_STAY,
};

typedef enum {
	USER_POSITION_ADMIN,
	USER_POSITION_BM_BEGIN,
	USER_POSITION_BM,
	USER_POSITION_BM_END,
	USER_POSITION_CUSTOM,
} user_position_e;

typedef void (*user_position_callback_t)(const char *, void *arg, user_position_e);
extern void user_position(const struct userec *user, const char *title, user_position_callback_t callback, void *arg);
extern void user_position_string(const struct userec *user, const char *title, char *buf, size_t size);

extern void remove_user_id_cache(const char *uname);
extern user_id_t get_user_id(const char *name);
extern int get_user_count(void);
extern int user_data_add_by_name(const char *name, int field, int delta);
extern int user_data_add(int uid, int field, int delta);

extern int calc_user_stay(bool is_login, bool is_dup, time_t login, time_t logout);

extern int set_my_last_post_time(fb_time_t t);
extern fb_time_t get_my_last_post_time(void);

#endif // FB_USER_H
