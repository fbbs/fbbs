#include "libweb.h"
#include "fbbs/mdbi.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

#define make_entry(user_id, session_key, ip_addr) \
	char entry[IPLEN + SESSION_KEY_LEN + 32]; \
	snprintf(entry, sizeof(entry), "%"PRIdUID"-%s-%s", \
			user_id, session_key, ip_addr)

static session_id_t session_get_web_cache(user_id_t user_id,
		const char *session_key, const char *ip_addr)
{
	make_entry(user_id, session_key, ip_addr);
	return mdb_integer(0, "HGET", SESSION_WEB_HASH_KEY" %s", entry);
}

void session_set_web_cache(user_id_t user_id, const char *session_key,
		session_id_t session_id, const char *ip_addr)
{
	make_entry(user_id, session_key, ip_addr);
	mdb_cmd("HSET", SESSION_WEB_HASH_KEY" %s %"PRIdSID, entry, session_id);
}

extern int do_web_login(const char *uname, const char *pw, bool api);

static bool activate_session(session_id_t sid, const char *uname)
{
	db_res_t *res = db_cmd("UPDATE sessions SET active = TRUE, stamp = %t"
			" WHERE id = %"DBIdSID, fb_time(), sid);
	db_clear(res);

	if (res)
		return !do_web_login(uname, NULL, false);

	return false;
}

static bool _get_session(const char *uname, const char *key)
{
	user_id_t uid = get_user_id(uname);
	if (uid > 0) {
		session_id_t sid = session_get_web_cache(uid, key, fromhost);
		if (sid > 0) {
			session_set_id(sid);
			session_set_uid(uid);
			return true;
		}

		db_res_t *res = db_query("SELECT id, active FROM sessions"
				" WHERE user_id = %"DBIdUID" AND session_key = %s AND web",
				uid, key);
		if (res && db_res_rows(res) == 1) {
			sid = db_get_session_id(res, 0, 0);
			bool active = db_get_bool(res, 0, 1);
			if (active || activate_session(sid, uname)) {
				session_set_id(sid);
				session_set_uid(uid);
				session_set_web_cache(uid, key, session_id(), fromhost);
			}
		}
		db_clear(res);
	}
	return session_id();
}

bool session_validate(void)
{
	memset(&currentuser, 0, sizeof(currentuser));
	session_clear();

	const char *uname = web_get_param(COOKIE_USER);
	const char *key = web_get_param(COOKIE_KEY);
	if (web_request_type(API) && web_request_method(GET)) {
		const char *token = web_get_param("token");
		if (!streq(token, key))
			return false;
	}

	bool ok = _get_session(uname, key);
	if (ok)
		getuserec(uname, &currentuser);

	return ok;
}
