#ifndef FB_REGISTER_H
#define FB_REGISTER_H

#include <stdbool.h>

#define REG_CODE_FILE ".regpass"

#ifndef FDQUAN
#define REG_CAPTCHA
#endif

enum {
	BBS_EREG_NONALPHA = -1,
	BBS_EREG_SHORT = -2,
	BBS_EREG_BADNAME = -3,
};

#ifdef REG_CAPTCHA
#define CAPTCHA_DIR    BBSHOME"/captcha"
#define CAPTCHA_OUT    BBSHOME"/captcha2"
#define CAPTCHA_INDEX  CAPTCHA_DIR"/index"
enum {
	NUM_CAPTCHAS = 1000,
	CAPTCHA_LEN = 7,
};
#endif // REG_CAPTCHA

typedef struct reginfo {
	char userid[IDLEN + 1];
	char realname[NAMELEN];
	char dept[STRLEN];
	char addr[STRLEN];
	char phone[STRLEN];
	char email[STRLEN - 12];
	char assoc[STRLEN];
	time_t regdate;
} reginfo_t;

extern bool is_no_register(void);
extern int check_userid(const char *userid);
extern int send_regmail(const struct userec *user, const char *mail);
extern bool activate_email(const char *userid, const char *attempt);
extern bool is_reg_pending(const char *userid);
extern int append_reg_list(const reginfo_t *reg);

#endif // FB_REGISTER_H
