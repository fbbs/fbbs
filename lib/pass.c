#include "bbs.h"
#include <crypt.h>

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

// Checks if encrypted 'pw_try' matches 'pw_crypted'.
// Returns 1 if match, 0 otherwise.
int checkpasswd(const char *pw_crypted, const char *pw_try)
{
	return !strcmp(crypt(pw_try, pw_crypted), pw_crypted);
}
