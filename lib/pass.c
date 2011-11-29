#include "bbs.h"

#ifdef HAVE_CRYPT_H
# include <crypt.h>
#else
# include <unistd.h>
#endif

#include "fbbs/dbi.h"
#include "fbbs/fbbs.h"
#include "fbbs/string.h"

#ifndef MD5
#ifndef DES
/* nor DES, MD5, fatal error!! */
#error "(pass.c) you've not define DES nor MD5, fatal error!!"
#endif
#endif

// 0 ... 63 => ascii - 64
static unsigned char itoa64[] =
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

// Uses 7 least significant bits of 'v' to find counterpart in 'itoa64'.
// Concatenates result to string 's' and right shifts 'v' for 6 bits.
// Then loops for 'n' times.
static void to64(char *s, long v, int n) {
	while (--n >= 0) {
		*s++ = itoa64[v & 0x3f];
		v >>= 6;
	}
}

// Encrypts 'pw' using libcrypt and returns result.
char *genpasswd(const char *pw)
{
	char salt[10];
	struct timeval tv;

	if (pw == NULL || pw[0] == '\0')
		return "";

	srand(time(NULL) % getpid());
	gettimeofday(&tv, NULL);

#ifdef MD5			/* use MD5 salt */
	strncpy(&salt[0], "$1$", 3);
	to64(&salt[3], random(), 3);
	to64(&salt[6], tv.tv_usec, 3);
	salt[8] = '\0';
#endif
#ifdef DES			/* use DES salt */
	to64(&salt[0], random(), 3);
	to64(&salt[3], tv.tv_usec, 3);
	to64(&salt[6], tv.tv_sec, 2);
	salt[8] = '\0';
#endif
	// When using DES, only the first 8 chars in 'pw' are significant.
	return crypt(pw, salt);
}

/**
 * Check if \a pw_try matches \a pw_crypted.
 * @param pw_crypted The crypted password.
 * @param pw_try The password in plain text.
 * @return True if they match, false otherwise.
 */
bool passwd_match(const char *pw_crypted, const char *pw_try)
{
	return streq(crypt(pw_try, pw_crypted), pw_crypted);
}

/**
 * Check if \a pw_try could be the password of user \a name.
 * @param name The username.
 * @param pw_try The password in plain text.
 * @return True if they match, false otherwise.
 */
bool passwd_check(const char *name, const char *pw_try)
{
	db_res_t *res = db_exec_query(env.d, true,
			"SELECT passwd FROM users WHERE lower(name) = lower(%s)", name);
	if (!res)
		return false;
	if (db_res_rows(res) < 1) {
		db_clear(res);
		return false;
	}

	bool match = passwd_match(db_get_value(res, 0, 0), pw_try);
	db_clear(res);
	return match;
}

/**
 * Set one's password.
 * @param name The user name.
 * @param pw The password in plain text.
 * @return 0 on sucess, -1 on error.
 */
int passwd_set(const char *name, const char *pw)
{
	db_res_t *res = db_exec_cmd(env.d, "UPDATE all_users"
			" SET passwd = %s WHERE lower(name) = lower(%s) AND alive = 1",
			genpasswd(pw), name);
	int ret = res ? 0 : -1;
	db_clear(res);
	return ret;
}
