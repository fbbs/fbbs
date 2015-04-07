#include "libweb.h"
#include "fbbs/mdbi.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

enum {
	ENTRY_LEN = IPLEN + SESSION_KEY_LEN + 32,
};

static void make_entry(user_id_t user_id, const char *session_key,
		const char *ip_addr, char *entry, size_t size)
{
	snprintf(entry, size, "%"PRIdUID"-%s-%s", user_id, session_key, ip_addr);
}

static session_id_t session_get_web_cache(user_id_t user_id,
		const char *session_key, const char *token, const char *ip_addr)
{
	char entry[ENTRY_LEN];
	make_entry(user_id, session_key, ip_addr, entry, sizeof(entry));

	session_id_t session_id = 0;
	mdb_res_t *res = mdb_res("HGET", SESSION_WEB_HASH_KEY" %s", entry);
	if (res) {
		const char *s = mdb_string(res);
		if (s) {
			session_id = strtol(s, NULL, 10);
			if (token) {
				bool success = false;
				const char *colon = strchr(s, ':');
				if (colon)
					success = streq(token, colon + 1);
				if (!success)
					session_id = 0;
			}
		}
		mdb_clear(res);
	}
	return session_id;
}

void session_set_web_cache(user_id_t user_id, const char *session_key,
		const char *token, session_id_t session_id, const char *ip_addr)
{
	char entry[ENTRY_LEN];
	make_entry(user_id, session_key, ip_addr, entry, sizeof(entry));
	char value[SESSION_TOKEN_LEN + 32];
	snprintf(value, sizeof(value), "%"PRIdSID":%s", session_id,
			token ? token : "");
	mdb_cmd("HSET", SESSION_WEB_HASH_KEY" %s %s", entry, value);
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

static bool _get_session(const char *user_name, const char *key,
		const char *token)
{
	user_id_t user_id = get_user_id(user_name);
	if (user_id > 0) {
		session_id_t session_id =
				session_get_web_cache(user_id, key, token, fromhost);
		if (session_id > 0) {
			session_set_id(session_id);
			session_set_uid(user_id);
			return true;
		}

		db_res_t *res = db_query("SELECT id, active FROM sessions"
				" WHERE user_id = %"DBIdUID" AND session_key = %s AND web",
				user_id, key);
		if (res && db_res_rows(res) == 1) {
			session_id = db_get_session_id(res, 0, 0);
			bool active = db_get_bool(res, 0, 1);
			if (active || activate_session(session_id, user_name)) {
				session_set_id(session_id);
				session_set_uid(user_id);
				session_set_web_cache(user_id, key, token, session_id,
						fromhost);
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
	const char *token = NULL;
	if (web_request_type(API) && web_request_method(GET))
		token = web_get_param("token");

	bool ok = _get_session(uname, key, token);
	if (ok)
		getuserec(uname, &currentuser);

	return ok;
}
