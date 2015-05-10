#include "fbbs/friend.h"
#include "fbbs/dbi.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/user.h"

/**
 * Follow a person.
 * @param follower The follower id.
 * @param followed The name of followed person.
 * @param notes Notes.
 * @return Affected rows.
 */
int follow(user_id_t follower, const char *followed, const char *notes)
{
	if (notes) {
		if (string_validate_utf8(notes, FOLLOW_NOTE_CCHARS, false) < 0)
			return 0;
	} else {
		notes = "";
	}

	user_id_t uid = get_user_id(followed);
	if (!uid)
		return 0;

	db_res_t *res = db_cmd("INSERT INTO follows (user_id, follower, notes)"
			" VALUES (%"DBIdUID", %"DBIdUID", %s)", uid, follower, notes);
	if (res) {
		int ret = db_cmd_rows(res);
		db_clear(res);
		return ret;
	}
	return 0;
}

int unfollow(user_id_t follower, user_id_t followed)
{
	db_res_t *res = db_cmd("DELETE FROM follows WHERE user_id = %"DBIdUID
			" AND follower = %"DBIdUID,
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
	if (string_validate_utf8(notes, FOLLOW_NOTE_CCHARS, false) < 0)
		return;

	db_res_t *res = db_cmd("UPDATE follows SET notes = %s"
			"WHERE user_id = %"DBIdUID" AND follower = %"DBIdUID,
			notes, followed, follower);
	db_clear(res);
}

bool am_followed_by(const char *uname)
{
	db_res_t *res = db_query("SELECT f.followed"
			" FROM follows f JOIN users u ON f.follower = u.id"
			" WHERE user_id = %"DBIdUID" AND lower(u.name) = lower(%s)",
			session_get_user_id(), uname);
	int rows = res ? db_res_rows(res) : 0;
	db_clear(res);
	return rows;
}

following_list_t *following_list_load(user_id_t uid)
{
	return (following_list_t *)db_query(FOLLOWING_LIST_LOAD_QUERY, uid);
}

black_list_t *black_list_load(user_id_t uid)
{
	return (black_list_t *)db_query(BLACK_LIST_LOAD_QUERY, uid);
}

bool black_list_add(user_id_t uid, const char *blocked, const char *notes)
{
	if (string_validate_utf8(notes, BLACK_LIST_NOTE_CCHARS, false) < 0)
		return false;

	user_id_t block_id = get_user_id(blocked);
	if (block_id <= 0 || block_id == uid)
		return false;
	
	db_res_t *res = db_cmd("INSERT INTO blacklists"
			" (user_id, blocked, notes, stamp)"
			" VALUES (%d, %d, %s, current_timestamp)", uid, block_id, notes);
	db_clear(res);
	return res;
}

bool black_list_rm(user_id_t uid, user_id_t blocked)
{
	db_res_t *res = db_cmd("DELETE FROM blacklists"
			" WHERE user_id = %"DBIdUID" AND blocked = %"DBIdUID,
			uid, blocked);
	db_clear(res);
	return res;
}

bool black_list_edit(user_id_t uid, user_id_t blocked, const char *notes)
{
	if (string_validate_utf8(notes, BLACK_LIST_NOTE_CCHARS, false) < 0)
		return false;

	db_res_t *res = db_cmd("UPDATE blacklists SET notes = %s"
			" WHERE user_id = %"DBIdUID" AND blocked = %"DBIdUID,
			notes, uid, blocked);
	db_clear(res);
	return res;
}

bool is_blocked(const char *uname)
{
	db_res_t *res = db_query("SELECT b.blocked FROM blacklists b"
			" JOIN alive_users u ON b.user_id = u.id"
			" WHERE b.blocked = %"DBIdUID" AND lower(u.name) = lower(%s)",
			session_get_user_id(), uname);
	int rows = res ? db_res_rows(res) : 0;
	db_clear(res);
	return rows;
}

/**
 * 获取关注和拉黑某人的用户列表
 * @param[in] uid 要查询的用户ID
 * @param[out] followers 关注该用户的用户列表
 * @param[out] blacklisters 拉黑该用户的用户列表
 */
void friend_load_followers_and_blacklisters(user_id_t uid,
		friend_uid_list_t **followers, friend_uid_list_t **blacklisters)
{
	if (!followers || !blacklisters)
		return;

	query_t *q = query_new(0);
	query_select(q, "follower");
	query_from(q, "follows");
	query_where(q, "user_id = %"DBIdUID, uid);
	*followers = query_exec(q);

	q = query_new(0);
	query_select(q, "user_id");
	query_from(q, "blacklists");
	query_where(q, "blocked = %"DBIdUID, uid);
	*blacklisters = query_exec(q);
}

/**
 * 检查指定的用户ID是否在列表中
 * @param[in] list 用户ID列表
 * @param[in] uid 用户ID
 * @return 在列表中返回true, 否则false
 */
bool friend_uid_list_contains(const friend_uid_list_t *list, user_id_t uid)
{
	if (!list)
		return false;
	for (int i = db_res_rows(list) - 1; i >= 0; --i) {
		if (db_get_user_id(list, i, 0) == uid)
			return true;
	}
	return false;
}
