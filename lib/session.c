#include <sys/types.h>
#include <unistd.h>

#include "fbbs/fbbs.h"
#include "fbbs/mdbi.h"
#include "fbbs/session.h"

enum {
	IDLE_TIME_REFRESH_THRESHOLD = 5,
	ONLINE_FOLLOWS_COUNT_REFRESH_INTERVAL = 15,
	ONLINE_COUNT_REFRESH_INTERVAL = 15,
};

bbs_session_t session;

#define ONLINE_COUNT_CACHE_KEY  "c:online"

int online_count(void)
{
	int cached = mdb_get_integer(-1, "GET "ONLINE_COUNT_CACHE_KEY);
	if (cached >= 0)
		return cached;

	int online = 0;
	db_res_t *res = db_exec_query(env.d, true,
			"SELECT count(*) FROM sessions WHERE active");
	if (res && db_res_rows(res) > 0)
		online = db_get_integer(res, 0, 0);
	db_clear(res);

	mdb_res_t *r = mdb_cmd("SET "ONLINE_COUNT_CACHE_KEY" %d", online);
	mdb_clear(r);
	r = mdb_cmd("EXPIRE "ONLINE_COUNT_CACHE_KEY" %d",
			ONLINE_COUNT_REFRESH_INTERVAL);
	mdb_clear(r);

	return online;
}

void update_peak_online(int online)
{
	mdb_res_t *r = mdb_cmd("SET c:max_online %d", online);
	mdb_clear(r);
}

int get_peak_online(void)
{
	return mdb_get_integer(0, "GET c:max_online");
}

session_id_t session_new_id(void)
{
	db_res_t *res = db_exec_query(env.d, true,
			"SELECT nextval('sessions_id_seq')");
	if (!res)
		return 0;

	session_id_t sid = db_get_session_id(res, 0, 0);
	db_clear(res);
	return sid;
}

session_id_t session_new(const char *key, session_id_t sid, user_id_t uid,
		const char *ip_addr, bool is_web, bool is_secure)
{
	int pid = getpid();

	db_res_t *res = db_cmd("INSERT INTO sessions"
			" (id, session_key, user_id, pid, ip_addr, web, secure)"
			" VALUES (%"DBIdSID", %s, %"DBIdUID", %d, %s, %b, %b)",
			sid ? sid : session_new_id(),
			key, uid, pid, ip_addr, is_web, is_secure);
	if (!res)
		return 0;
	db_clear(res);

	set_idle_time(sid, time(NULL));
	return sid;
}

int session_destroy(session_id_t sid)
{
	db_res_t *res = db_cmd("DELETE from sessions WHERE id = %"DBIdSID, sid);
	db_clear(res);

	mdb_res_t *r = mdb_cmd("ZREM current_board %"PRIdSID, sid);
	mdb_clear(r);

	r = mdb_cmd("ZREM idle %"PRIdSID, sid);
	mdb_clear(r);

	return !res;
}

int set_idle_time(session_id_t sid, fb_time_t t)
{
	mdb_res_t *res = mdb_cmd("ZADD idle %"PRIdFBT" %"PRIdSID, t, sid);
	mdb_clear(res);
	return !res;
}

void cached_set_idle_time(void)
{
	time_t now = time(NULL);
	if (now > session.idle + IDLE_TIME_REFRESH_THRESHOLD)
		set_idle_time(session.id, now);
	session.idle = now;
}

fb_time_t get_idle_time(session_id_t sid)
{
	return (fb_time_t) mdb_get_integer(0, "ZSCORE idle %"PRIdSID, sid);
}

int set_current_board(int bid)
{
	mdb_res_t *res = mdb_cmd("ZADD current_board %d %"PRIdSID, bid, session.id);
	mdb_clear(res);
	return !res;
}

int get_current_board(session_id_t sid)
{
	return (int) mdb_get_integer(0, "ZSCORE current_board %"PRIdSID, sid);
}

int set_user_status(int status)
{
	session.status = status;
	mdb_res_t *res = mdb_cmd("HSET user_status %"PRIdSID" %d",
			session.id, status);
	mdb_clear(res);
	return !res;
}

int get_user_status(session_id_t sid)
{
	return (int) mdb_get_integer(0, "HGET user_status %"PRIdSID, sid);
}

int set_visibility(bool visible)
{
	db_res_t *res = db_cmd("UPDATE sessions SET visible = %b"
			" WHERE id = %"DBIdSID, visible, session.id);
	db_clear(res);
	return !res;
}

db_res_t *get_sessions_of_followings(void)
{
	return db_query("SELECT " ACTIVE_SESSION_FIELDS ", f.notes"
			" FROM sessions s JOIN follows f ON s.user_id = f.user_id"
			" JOIN users u ON s.user_id = u.id"
			" WHERE s.active AND f.follower = %"DBIdUID, session.uid);
}

db_res_t *get_active_sessions(void)
{
	return db_exec_query(env.d, true, ACTIVE_SESSION_QUERY);
}

basic_session_info_t *get_sessions(user_id_t uid)
{
	return db_query("SELECT " BASIC_SESSION_INFO_FIELDS " FROM sessions s"
			" WHERE active AND user_id = %"DBIdUID, uid);
}

basic_session_info_t *get_my_sessions(void)
{
	return get_sessions(session.uid);
}

static basic_session_info_t *basic_sessions_of_follows(void)
{
	return db_query("SELECT "BASIC_SESSION_INFO_FIELDS
			" FROM sessions s JOIN follows f ON s.user_id = f.user_id"
			" WHERE s.active AND f.follower = %"DBIdUID, session.uid);
}

int online_follows_count(bool visible_only)
{
	static time_t uptime = 0;
	static int count = 0;

	time_t now = time(NULL);
	if (now <= uptime + ONLINE_FOLLOWS_COUNT_REFRESH_INTERVAL)
		return count;
	uptime = now;

	basic_session_info_t *s = basic_sessions_of_follows();
	if (s) {
		if (!visible_only) {
			count = basic_session_info_count(s);
		} else {
			count = 0;
			for (int i = 0; i < basic_session_info_count(s); ++i) {
				if (basic_session_info_visible(s, i))
					++count;
			}
		}
	} else {
		count = 0;
	}
	basic_session_info_clear(s);
	return count;
}
