#ifndef FB_FRIEND_H
#define FB_FRIEND_H

#include "fbbs/dbi.h"

enum {
	FOLLOW_NOTE_CCHARS = 15,
	BLACK_LIST_NOTE_CCHARS = 15,
};

extern int follow(user_id_t follower, const char *followed, const char *notes);
extern int unfollow(user_id_t follower, user_id_t followed);
extern void edit_followed_note(user_id_t follower, user_id_t followed, const char *notes);

typedef db_res_t following_list_t;

extern following_list_t *following_list_load(user_id_t uid);
#define following_list_rows(list)  db_res_rows(list)
#define following_list_free(list)  db_clear(list)

#define FOLLOWING_LIST_LOAD_QUERY \
	"SELECT user_id, name, follow_time, is_friend, notes" \
	" FROM follows f JOIN users u ON f.user_id = u.id" \
	" WHERE follower = %d ORDER BY follow_time DESC"
#define following_list_get_id(list, i)  db_get_user_id(list, i, 0)
#define following_list_get_name(list, i)  db_get_value(list, i, 1)
#define following_list_get_is_friend(list, i)  db_get_bool(list, i, 3)
#define following_list_get_notes(list, i)  db_get_value(list, i, 4)

typedef db_res_t black_list_t;

extern black_list_t *black_list_load(user_id_t uid);
#define black_list_rows(list)  db_res_rows(list)
#define black_list_free(list)  db_clear(list)

#define BLACK_LIST_LOAD_QUERY \
	"SELECT blocked, name, notes FROM blacklists b JOIN users u" \
	" ON b.blocked = u.id WHERE b.user_id = %d ORDER BY stamp DESC"
#define black_list_get_id(l, i)  db_get_user_id(l, i, 0)
#define black_list_get_name(l, i)  db_get_value(l, i, 1)
#define black_list_get_notes(l, i)  db_get_value(l, i, 2)

extern bool black_list_add(user_id_t uid, const char *blocked, const char *notes);
extern bool black_list_rm(user_id_t uid, user_id_t blocked);
extern bool black_list_edit(user_id_t uid, user_id_t blocked, const char *notes);

#endif // FB_FRIEND_H
