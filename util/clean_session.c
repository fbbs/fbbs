#include "func.h"
#include "site.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/time.h"
#include "fbbs/uinfo.h"

enum {
	MAX_WEB_SESSIONS = 10,
};

struct _session {
	session_id_t sid;
	user_id_t uid;
	bool active;
	bool web;
	int pid;
	fb_time_t stamp;
	fb_time_t expire;
	char key[SESSION_KEY_LEN + 1];
};

static int comparator(const void *v1, const void *v2)
{
	const struct _session *s1 = v1;
	const struct _session *s2 = v2;
	if (s1->uid == s2->uid)
		return s2->stamp - s1->stamp;
	return s1->uid - s2->uid;
}

static void res_to_session(db_res_t *res, struct _session *sessions, int count)
{
	struct _session *s = sessions;
	for (int i = 0; i < count; ++i) {
		s->sid = db_get_session_id(res, i, 0);
		s->active = db_get_value(res, i, 1);
		s->uid = db_get_user_id(res, i, 2);
		s->pid = db_get_integer(res, i, 3);
		s->web = db_get_bool(res, i, 4);
		s->stamp = db_get_time(res, i, 5);
		s->expire = db_get_time(res, i, 6);
		if (s->web)
			strlcpy(s->key, db_get_value(res, i, 7), sizeof(s->key));
		++s;
	}
	qsort(sessions, count, sizeof(*sessions), comparator);
}

static const struct _session *find_same_uid(const struct _session *begin,
		const struct _session *end)
{
	const struct _session *s = begin + 1;
	while (s < end) {
		if (s->uid != begin->uid)
			break;
		++s;
	}
	return s;
}

static bool get_user_name(user_id_t uid, char *name, size_t size)
{
	db_res_t *res = db_query("SELECT name FROM alive_users"
			" WHERE id = %"DBIdUID, uid);
	if (res && db_res_rows(res) == 1)
		strlcpy(name, db_get_value(res, 0, 0), size);
	else
		name[0] = '\0';
	db_clear(res);
	return name[0];
}

static void kill_session(const struct _session *s, bool is_dup)
{
	if (s->web || bbs_kill(s->sid, s->pid, SIGHUP) != 0) {
		char uname[IDLEN + 1];
		struct userec user;
		int legacy_uid;

		if (get_user_name(s->uid, uname, sizeof(uname))
				&& (legacy_uid = getuserec(uname, &user))) {
			update_user_stay(&user, false, is_dup);
			substitut_record(NULL, &user, sizeof(user), legacy_uid);
		}

		if (s->web && s->expire > time(NULL))
			session_inactivate(s->sid);
		else
			session_destroy(s->sid);

		if (s->web)
			remove_web_session_cache(s->uid, s->key);
	}
}

static bool check_timeout(const struct _session *s, int count)
{
	if (s->active) {
		fb_time_t refresh = get_idle_time(s->sid);
		fb_time_t now = time(NULL);
		if (refresh >= 0 && now - refresh > IDLE_TIMEOUT) {
			session_status_e status = get_user_status(s->sid);
			if (status != ST_BBSNET) {
				kill_session(s, count > 1);
				return true;
			}
		}
	}
	return false;
}

static void check_sessions(const struct _session *begin,
		const struct _session *end)
{
	int active = 0;
	for (const struct _session *s = begin; s < end; ++s) {
		if (s->active)
			++active;
	}

	int web = 0;
	for (const struct _session *s = begin; s < end; ++s) {
		if (check_timeout(s, active)) {
			--active;
		} else {
			if (!s->active && s->web) {
				if (++web > MAX_WEB_SESSIONS)
					kill_session(s, active--);
			}
		}
	}
}

int main(int argc, char **argv)
{
	initialize_environment(INIT_DB | INIT_MDB);

	if (resolve_ucache() != 0)
		return EXIT_FAILURE;

	db_res_t *res = db_query("SELECT id, active, user_id, pid, web, stamp,"
			" expire, session_key FROM sessions");

	int count = db_res_rows(res);
	struct _session *sessions = malloc(sizeof(*sessions) * count);
	res_to_session(res, sessions, count);

	const struct _session *begin = sessions, *send = sessions + count, *end;
	while (begin < send) {
		end = find_same_uid(begin, send);
		check_sessions(begin, end);
		begin = end;
	}

	free(sessions);
	db_clear(res);
	return EXIT_SUCCESS;
}
