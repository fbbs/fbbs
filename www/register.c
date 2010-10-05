#include "libweb.h"
#include "fbbs/mail.h"
#include "fbbs/register.h"
#include "fbbs/string.h"
#include "fbbs/uinfo.h"

// Since there is no captcha for web registration...
enum {
	WEB_NOREG_START = 5,
	WEB_NOREG_END = 7,
};

extern int create_user(const struct userec *user);

typedef struct reg_req_t {
	const char *id;
	const char *pw;
	const char *pw2;
	const char *mail;
	const char *nick;
	const char *gender;
	const char *name;
	const char *tel;
	const char *agree;
	int year;
	int month;
	int day;
} reg_req_t;

static const char *_reg(const reg_req_t *r)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	if (t->tm_hour >= WEB_NOREG_START && t->tm_hour < WEB_NOREG_END)
		return "当前时段恕不开放web注册，请稍后再试";

	if (strcmp(r->agree, "agree") != 0)
		return "您必须同意站规";

	const char *error = invalid_userid(r->id);
	if (error)
		return error;

	if (strcmp(r->pw, r->pw2) != 0)
		return "两次密码不匹配";

	error = invalid_password(r->pw, r->id);
	if (error)
		return error;

	struct userec user;
	init_userec(&user, r->id, r->pw, true);

#ifndef FDQUAN
	char email[sizeof(user.email)];
	snprintf(email, sizeof(email), "%s@fudan.edu.cn", r->mail);
	if (!valid_addr(email) || !domain_allowed(email)
			|| is_banned_email(email)) {
		return "电子邮件地址无效，或不在允许范围";
	}
	user.email[0] = '\0';
#endif // FDQUAN

	user.gender = r->gender[0];
	strlcpy(user.username, r->nick, sizeof(user.username));
	printable_filter(user.username);
	user.birthyear = r->year > 1900 ? r->year - 1900 : r->year;
	user.birthmonth = r->month;
	user.birthday = r->day;
	if (check_user_profile(&user) != 0
			|| strlen(r->name) < 4 || strlen(r->tel) < 8)
		return "请详细填写个人资料";

	if (create_user(&user) != 0)
		return "用户已存在，或出现其他内部错误";

	reginfo_t reg;
	memset(&reg, 0, sizeof(reg));
	strlcpy(reg.userid, r->id, sizeof(reg.userid));
	strlcpy(reg.realname, r->name, sizeof(reg.realname));
	strlcpy(reg.phone, r->tel, sizeof(reg.phone));
#ifndef FDQUAN
	strlcpy(reg.email, email, sizeof(reg.email));
#endif
	reg.regdate = now;
	if (append_reg_list(&reg) != 0)
		return "提交注册资料失败";

	char file[HOMELEN];
	snprintf(file, sizeof(file), "home/%c/%s",
			toupper(user.userid[0]), user.userid);
	if (mkdir(file, 0755) != 0)
		return "内部错误";

#ifndef FDQUAN
	if (send_regmail(&user, email) != 0)
		return "发送注册信失败";
#endif // FDQUAN

	return NULL;
}

int fcgi_reg(void)
{
	parse_post_data();
	reg_req_t request = {
		.id = getparm("id"),
		.pw = getparm("pw"),
		.pw2 = getparm("pw2"),
		.mail = getparm("mail"),
		.nick = getparm("nick"),
		.gender = getparm("gender"),
		.name = getparm("name"),
		.tel = getparm("tel"),
		.agree = getparm("agree"),
		.year = strtol(getparm("byear"), NULL, 10),
		.month = strtol(getparm("bmon"), NULL, 10),
		.day = strtol(getparm("bday"), NULL, 10)
	};

	const char *error = _reg(&request);

	xml_header(NULL);
	printf("<bbsreg error='%d'>", error ? 1 : 0);
	print_session();
	if (error)
		printf("%s", error);
	printf("</bbsreg>");
	return 0;
}

int fcgi_activate(void)
{
	const char *code = getparm("code");
	const char *user = getparm("user");
	xml_header(NULL);
	printf("<bbsactivate success='%d'>", activate_email(user, code));
	print_session();
	printf("</bbsactivate>");
	return 0;
}

int fcgi_exist(void)
{
	const char *user = getparm("user");
	xml_header(NULL);
	printf("<bbsexist>%d</bbsexist>", searchuser(user) != 0);
	return 0;
}
