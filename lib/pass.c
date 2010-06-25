#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <crypt.h>
#include <sys/time.h>

#include "fbbs/site.h"

#ifndef MD5
#ifndef DES
/* nor DES, MD5, fatal error!! */
#error "(pass.c) you've not define DES nor MD5, fatal error!!"
#endif
#endif

/** Used in integer-string conversion. */
static const unsigned char itoa64[] =
		"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
 * Convert a long integer to string.
 * Every 6 bits of the integer is mapped to a char.
 * @param s The resulting string.
 * @param v The long integer.
 * @param n Number of converted characters.
 */
static void to64(char *s, long v, int n) {
	while (--n >= 0) {
		*s++ = itoa64[v & 0x3f];
		v >>= 6;
	}
}

/**
 * Encrypt string.
 * When using DES, only the first 8 chars in the string are significant.
 * @param pw The string to be encrypted.
 * @return The encrypted string.
 */
const char *generate_passwd(const char *pw)
{
	char salt[10];
	struct timeval tv;

	if (pw == NULL || pw[0] == '\0')
		return "";

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec % getpid());

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
 * Checks if the given plain text could be the original text
 * of encrypted string.
 * @param pw_crypted The encrypted string.
 * @param pw_try The plain text.
 * @return True if match, false otherwise.
 */
bool check_passwd(const char *pw_crypted, const char *pw_try)
{
	return !strcmp(crypt(pw_try, pw_crypted), pw_crypted);
}
