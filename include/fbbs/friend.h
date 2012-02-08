#ifndef FB_FRIEND_H
#define FB_FRIEND_H

#include "fbbs/dbi.h"

enum {
	FOLLOW_NOTE_CCHARS = 15,
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

#endif // FB_FRIEND_H
