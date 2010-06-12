#ifndef FB_USER_H
#define FB_USER_H

enum {
	EXT_ID_LEN = 16,
	NICK_LEN = 40,
	NAME_LEN = 32,
	EMAIL_LEN = 40,
	PASS_LEN = 14,
};

typedef struct user_t {
	seq_t uid;
	uint_t perm;
	uint_t logins;
	uint_t posts;
	uint_t stay;
	uint_t medals;
	int money;

	char gender;
	uchar_t birthyear;
	uchar_t birthmonth;
	uchar_t birthday;
	
	int utmpkey;
	int prefs;
	fb_time_t reg;
	fb_time_t login;
	fb_time_t logout;

	char passwd[PASS_LEN];
	char name[EXT_ID_LEN];
	char nick[NICK_LEN];
	char real[NAME_LEN];
	char email[EMAIL_LEN];
} user_t;

#endif // FB_USER_H

