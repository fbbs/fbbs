#include "libweb.h"
#include "fbbs/mdbi.h"
#include "fbbs/session.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

static session_id_t get_web_session_cache(user_id_t uid, const char *key)
{
	return mdb_integer(0, "HGET", WEB_SESSION_HASH_KEY" %"PRIdUID":%s",
			uid, key);
}

void set_web_session_cache(user_id_t uid, const char *key, session_id_t sid)
{
	mdb_res_t *r = mdb_cmd("HSET",
			WEB_SESSION_HASH_KEY" %"PRIdUID":%s %"PRIdSID, uid, key, sid);
	mdb_clear(r);
}

extern int do_web_login(const char *uname, const char *pw);

static bool activate_session(session_id_t sid, const char *uname)
{
	db_res_t *res = db_cmd("UPDATE sessions SET active = TRUE, stamp = %t"
			" WHERE id = %"DBIdSID, sid, time(NULL));
	db_clear(res);

	if (res)
		return !do_web_login(uname, NULL);

	return false;
}

static bool _get_session(const char *uname, const char *key)
{
	session.id = session.uid = 0;

	user_id_t uid = get_user_id(uname);
	if (uid > 0) {
		session_id_t sid = get_web_session_cache(uid, key);
		if (sid > 0) {
			session.uid = uid;
			return (session.id = sid);
		}

		db_res_t *res = db_query("SELECT id, active FROM sessions"
				" WHERE user_id = %"DBIdUID" AND session_key = %s AND web",
				uid, key);
		if (res && db_res_rows(res) == 1) {
			if (db_get_bool(res, 0, 1)
					|| activate_session(session.id, uname)) {
				session.id = db_get_session_id(res, 0, 0);
				session.uid = uid;
				set_web_session_cache(uid, key, session.id);
			}
		}
		db_clear(res);
	}
	return session.id;
}

bool get_session(void)
{
	const char *uname = get_param(COOKIE_USER);
	const char *key = get_param(COOKIE_KEY);

	memset(&currentuser, 0, sizeof(currentuser));

	bool ok = _get_session(uname, key);
	if (ok)
		getuserec(uname, &currentuser);

	return ok;
}
