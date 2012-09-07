#include "libweb.h"
#include "fbbs/brc.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
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

static int check_web_login_quota(const char *uname, bool force)
{
	db_res_t *res = db_query("SELECT s.id"
			" FROM sessions s JOIN users u ON s.user_id = u.id"
			" WHERE s.active AND s.web AND lower(u.name) = lower(%s)", uname);
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

static char *digest_to_hex(const uchar_t *digest, char *buf, size_t size)
{
	const char *str = "0123456789abcdef";
	for (int i = 0; i < size / 2; ++i) {
		buf[i * 2] = str[(digest[i] & 0xf0) >> 4];
		buf[i * 2 + 1] = str[digest[i] & 0x0f];
	}
	buf[size - 1] = '\0';
	return buf;
}

static char *generate_session_key(char *buf, size_t size, session_id_t sid)
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

	const uchar_t *digest = calc_digest(&s, sizeof(s));
	return digest_to_hex(digest, buf, size);
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

static const char *get_login_referer(void)
{
	const char *next = get_param("next");
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
	const char *referer = get_param("ref");
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

int do_web_login(const char *uname, const char *pw)
{
	struct userec user;
	if (getuserec(uname, &user) == 0)
		return error_msg(ERROR_INCORRECT_PASSWORD);
	session.uid = get_user_id(uname);

	if (pw && !passwd_check(uname, pw)) {
		log_attempt(user.userid, fromhost, "web");
		return error_msg(ERROR_INCORRECT_PASSWORD);
	}

	int sessions = check_web_login_quota(uname, false);
	if (!HAS_PERM2(PERM_SYSOPS, &user) && sessions >= WEB_ACTIVE_LOGIN_QUOTA)
		return BBS_ELGNQE;

	if (!HAS_PERM2(PERM_LOGIN, &user))
		return error_msg(ERROR_USER_SUSPENDED);

	time_t now = time(NULL);
	if (now - user.lastlogin >= 20 * 60 || user.numlogins < 100)
		user.numlogins++;

#ifdef CHECK_FREQUENTLOGIN
	time_t last = user.lastlogin;
	if (!HAS_PERM(PERM_SYSOPS) && abs(last - now) < 10) {
		report("Too Frequent", user.userid);
		return BBS_ELFREQ;
	}
#endif

	update_user_stay(&user, true, sessions >= 1);

	grant_permission(&user);

	strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));
	save_user_data(&user);
	currentuser = user;

	log_usies("ENTER", fromhost, &user);
	return 0;
}

extern void set_web_session_cache(user_id_t uid, const char *key,
		session_id_t sid);

int web_login(void)
{
	if (request_type(REQUEST_API)) {
		if (session.id)
			return error_msg(ERROR_NONE);
		else
			return error_msg(ERROR_INCORRECT_PASSWORD);
	}

	if (session.id)
		return login_redirect(NULL, 0);

	if (parse_post_data() < 0)
		return error_msg(ERROR_BAD_REQUEST);

	const char *uname = get_param("id");
	if (*uname == '\0' || strcaseeq(uname, "guest"))
		return redirect_homepage();

	char pw[PASSLEN];
	strlcpy(pw, get_param("pw"), sizeof(pw));

	int ret = do_web_login(uname, pw);
	if (ret == 0) {
		bool persistent = *get_param("persistent");
		int max_age = persistent ? COOKIE_PERSISTENT_PERIOD : 0;

		char key[SESSION_KEY_LEN + 1];
		session.id = session_new_id();
		generate_session_key(key, sizeof(key), session.id);
		session.id = session_new(key, session.id, session.uid, fromhost,
				SESSION_WEB, SESSION_PLAIN, max_age);
		if (session.id)
			set_web_session_cache(session.uid, key, session.id);

		return login_redirect(key, max_age);
	}
	return ret;
}

int web_logout(void)
{
	if (session.id) {
		update_user_stay(&currentuser, false, false);
		save_user_data(&currentuser);

		expire_cookie(COOKIE_KEY);
		expire_cookie(COOKIE_USER);
	}
	redirect_homepage();
	return 0;
}
