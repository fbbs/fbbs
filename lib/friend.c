#include "fbbs/fbbs.h"
#include "fbbs/friend.h"
#include "fbbs/dbi.h"
#include "fbbs/string.h"

/**
 * Follow a person.
 * @param follower The follower id.
 * @param followed The name of followed person.
 * @param notes Notes.
 * @return Affected rows.
 */
int follow(user_id_t follower, const char *followed, const char *notes)
{
	if (validate_utf8_input(notes, FOLLOW_NOTE_CCHARS) < 0)
		return 0;

	db_res_t *res = db_exec_cmd(env.d, "INSERT INTO follows"
			" (user_id, follower, notes)"
			" SELECT id, %"PRIdUID", %s FROM users"
			" WHERE lower(name) = lower(%s)", follower, notes, followed);
	if (res) {
		int ret = db_cmd_rows(res);
		db_clear(res);
		return ret;
	}
	return 0;
}

int unfollow(user_id_t follower, user_id_t followed)
{
	db_res_t *res = db_exec_cmd(env.d,
			"DELETE FROM follows WHERE user_id = %"PRIdUID
			" AND follower = %"PRIdUID,
			followed, follower);
	if (res) {
		int ret = db_cmd_rows(res);
		db_clear(res);
		return ret;
	}
	return 0;
}

void edit_followed_note(user_id_t follower, user_id_t followed, const char *notes)
{
	if (validate_utf8_input(notes, FOLLOW_NOTE_CCHARS) < 0)
		return;

	db_res_t *res = db_exec_cmd(env.d, "UPDATE follows SET notes = %s"
			"WHERE user_id = %"PRIdUID" AND follower = %"PRIdUID,
			notes, followed, follower);
	db_clear(res);
}

following_list_t *following_list_load(user_id_t uid)
{
	return (following_list_t *)db_exec_query(env.d, true,
			FOLLOWING_LIST_LOAD_QUERY, uid);
}
