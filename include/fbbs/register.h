#ifndef FB_REGISTER_H
#define FB_REGISTER_H

#include <stdbool.h>

#define REG_CODE_FILE ".regpass"

#ifndef FDQUAN
#define REG_CAPTCHA
#endif

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

extern bool register_closed(void);
extern const char *register_invalid_user_name(const char *uname);
extern const char *register_invalid_password(const char *password, const char *uname);
extern int send_regmail(const struct userec *user, const char *mail);
extern bool activate_email(const char *userid, const char *attempt);
extern bool is_reg_pending(const char *userid);
extern int append_reg_list(const reginfo_t *reg);
extern bool is_banned_email(const char *mail);
extern bool domain_allowed(const char *mail);
extern void init_userec(struct userec *user, const char *userid,
		        const char *passwd, bool usegbk);
extern int save_register_file(const reginfo_t *reg);
#endif // FB_REGISTER_H
