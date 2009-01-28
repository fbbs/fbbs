#include "bbs.h"
#include <sys/param.h>
#include <sys/resource.h>
#include <pwd.h>
#include <unistd.h>

#ifndef MD5
#ifndef DES
/* nor DES, MD5, fatal error!! */
#error "(pass.c) you've not define DES nor MD5, fatal error!!"
#endif
#endif

static unsigned char itoa64[] = /* 0 ... 63 => ascii - 64 */
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

//	取v的各6位,并按取值结果在itoa64数组中找到
static void to64(char *s, long v, int n) {
	while (--n >= 0) {
		*s++ = itoa64[v & 0x3f];
		v >>= 6;
	}
}

// 对pw进行加密并返回密文
char *genpasswd(const char *pw)
{
	char salt[10];
	static char pwbuf[PASSLEN];
	struct timeval tv;
	if (strlen(pw) == 0)
		return "";

	srand(time(0) % getpid());
	gettimeofday(&tv, 0);

#ifdef MD5			/* use MD5 salt */

	strlcpy(&salt[0], "$1$", 3);
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

	strcpy(pwbuf, pw);
	return crypt(pwbuf, salt);//返回pwbuf用salt进行DES加密的结果
}

int checkpasswd(const char *passwd, const char *test) //检查密码
// test 为用户输入密码字符串
{ // passwd 为salt值
	char *pw;
	static char pwbuf[PASSLEN];

	strlcpy(pwbuf, test, PASSLEN);
	pw = crypt(pwbuf, passwd);
	return (!strcmp(pw, passwd));
}
