#include "libweb.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/log.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/ucache.h"
#include "fbbs/uinfo.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

#define LOGIN_HOMEPAGE  "top10"

enum {
	WEB_ACTIVE_LOGIN_QUOTA = 4,
	COOKIE_PERSISTENT_PERIOD = 2 * 7 * 24 * 60 * 60,
};

static int check_web_login_quota(user_id_t user_id, bool force)
{
	db_res_t *res = db_query("SELECT id FROM sessions"
			" WHERE active AND web AND user_id = %"DBIdUID, user_id);
	if (!res)
		return INT_MAX;

	int count = db_res_rows(res);
	if (force) {
		// TODO: terminate extra web sessions
		;
	}
	db_clear(res);
	return count;
}

static void digest_to_hex(const uchar_t *digest, char *buf, size_t size)
{
	const char *str = "0123456789abcdef";
	for (int i = 0; i < size / 2; ++i) {
		buf[i * 2] = str[(digest[i] & 0xf0) >> 4];
		buf[i * 2 + 1] = str[digest[i] & 0x0f];
	}
	buf[size - 1] = '\0';
}

static void generate_session_key(char *key, size_t ksize,
		char *token, size_t tsize, session_id_t sid)
{
	struct {
		time_t sec;
		int usec;
		int random;
		session_id_t sid;
	} s;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	s.sec = tv.tv_sec;
	s.usec = tv.tv_usec;

	s.random = rand();

	s.sid = sid;

	const uchar_t *digest = web_calc_digest(&s, sizeof(s));
	char buf[SESSION_KEY_LEN + SESSION_TOKEN_LEN + 1];
	digest_to_hex(digest, buf, sizeof(buf));

	strlcpy(key, buf, ksize);
	strlcpy(token, buf + SESSION_KEY_LEN, tsize);
}

static void grant_permission(struct userec *user)
{
	if (!HAS_PERM2(PERM_REGISTER, user) && HAS_PERM2(PERM_BINDMAIL, user)) {
		char file[HOMELEN];
		sethomefile(file, user->userid, "register");
		if (dashf(file)) {
			user->userlevel |= PERM_DEFAULT;
		}
	}
}

#if 0
static const char *get_login_referer(void)
{
	const char *next = web_get_param("next");
	if (*next != '\0' && !strchr(next, '.'))
		return next;
	const char *referer = get_referer();
	const char *ref;
	if (!strcmp(referer, "/") || !strcmp(referer, "/index.htm")
			|| strstr(referer, "login"))
		ref = LOGIN_HOMEPAGE;
	else
		ref = referer;
	return ref;
}
#endif

static int redirect_homepage(void)
{
	printf("Location: ..\n\n");
	return 0;
}

static void set_cookie(const char *name, const char *value, int max_age)
{
	printf("Set-cookie: %s=%s", name, value);
	if (max_age > 0) {
		printf(";Max-Age=%d", max_age);
	} else if (max_age < 0) {
		printf(";Expires=Fri, 19-Apr-1996 11:11:11 GMT");
	}
	printf("\n");
}

static void expire_cookie(const char *name)
{
	set_cookie(name, "", -1);
}

static int login_redirect(const char *key, int max_age)
{
	const char *referer = web_get_param("ref");
	if (*referer == '\0')
		referer = LOGIN_HOMEPAGE;

	printf("Content-type: text/html; charset=%s\n", CHARSET);
	if (key) {
		set_cookie(COOKIE_KEY, key, max_age);
		set_cookie(COOKIE_USER, currentuser.userid, max_age);
	}
	printf("Location: %s\n\n", referer);
	return 0;
}

int do_web_login(const char *uname, const char *pw, bool api)
{
	struct userec user;
	if (!getuserec(uname, &user))
		return api ? WEB_ERROR_INCORRECT_PASSWORD : BBS_EWPSWD;
	user_id_t user_id = get_user_id(uname);
	session_set_uid(user_id);

	if (pw && !passwd_check(uname, pw)) {
		log_attempt(user.userid, fromhost, "web");
		return api ? WEB_ERROR_INCORRECT_PASSWORD : BBS_EWPSWD;
	}

	int sessions = check_web_login_quota(user_id, false);
	if (!HAS_PERM2(PERM_SYSOPS, &user) && sessions >= WEB_ACTIVE_LOGIN_QUOTA)
		return BBS_ELGNQE;

	if (!HAS_PERM2(PERM_LOGIN, &user))
		return api ? WEB_ERROR_USER_SUSPENDED : BBS_EACCES;

	fb_time_t now = fb_time();
	if (now - user.lastlogin >= 20 * 60 || user.numlogins < 100)
		user.numlogins++;

	update_user_stay(&user, true, sessions >= 1);

	grant_permission(&user);

	strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));
	save_user_data(&user);
	currentuser = user;

	log_usies("ENTER", fromhost, &user);
	return 0;
}

extern void session_set_web_cache(user_id_t user_id, const char *session_key,
		const char *token, session_id_t session_id, const char *ip_addr);

static int _web_login(bool persistent, bool redirect)
{
	int max_age = persistent ? COOKIE_PERSISTENT_PERIOD : 0;
	char key[SESSION_KEY_LEN + 1], token[SESSION_TOKEN_LEN + 1];
	session_new_id();
	generate_session_key(key, sizeof(key), token, sizeof(token), session_id());
	session_new(key, token, session_id(), session_uid(), currentuser.userid,
			fromhost, SESSION_WEB, SESSION_PLAIN, true, max_age);
	if (session_id())
		session_set_web_cache(session_uid(), key, NULL, session_id(), fromhost);
	return redirect ? login_redirect(key, max_age) : 0;
}

static int api_login(void)
{
	if (web_request_method(GET)) {
		if (session_id())
			return WEB_ERROR_NONE;
		else
			return WEB_ERROR_LOGIN_REQUIRED;
	} else if (web_request_method(POST)) {
		if (web_parse_post_data() < 0)
			return WEB_ERROR_BAD_REQUEST;

		const char *uname = web_get_param("id");
		char pw[PASSLEN];
		strlcpy(pw, web_get_param("pw"), sizeof(pw));

		int ret = do_web_login(uname, pw, true);
		if (ret == 0)
			_web_login(true, false);
		return ret;
	} else {
		return WEB_ERROR_METHOD_NOT_ALLOWED;
	}
}

int web_login(void)
{
	if (web_request_type(API))
		return api_login();

	if (session_id())
		return login_redirect(NULL, 0);

	if (web_parse_post_data() < 0)
		return BBS_EINVAL;

	const char *uname = web_get_param("id");
	if (*uname == '\0' || strcaseeq(uname, "guest"))
		return redirect_homepage();

	char pw[PASSLEN];
	strlcpy(pw, web_get_param("pw"), sizeof(pw));

	int ret = do_web_login(uname, pw, false);
	if (ret == 0) {
		bool persistent = *web_get_param("persistent");
		_web_login(persistent, true);
	}
	return ret;
}

int web_logout(void)
{
	if (session_id()) {
		update_user_stay(&currentuser, false, false);
		save_user_data(&currentuser);

		expire_cookie(COOKIE_KEY);
		expire_cookie(COOKIE_USER);
	}
	redirect_homepage();
	return 0;
}
