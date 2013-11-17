#include "libweb.h"
#include "fbbs/mail.h"
#include "fbbs/register.h"
#include "fbbs/string.h"
#include "fbbs/uinfo.h"
#include "fbbs/web.h"

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
	const char *domain;
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
	if (register_closed() ||
			(t->tm_hour >= WEB_NOREG_START && t->tm_hour < WEB_NOREG_END))
		//% return "当前时段恕不开放web注册，请稍后再试";
		return "\xb5\xb1\xc7\xb0\xca\xb1\xb6\xce\xcb\xa1\xb2\xbb\xbf\xaa\xb7\xc5web\xd7\xa2\xb2\xe1\xa3\xac\xc7\xeb\xc9\xd4\xba\xf3\xd4\xd9\xca\xd4";

	if (strcmp(r->agree, "agree") != 0)
		//% return "您必须同意站规";
		return "\xc4\xfa\xb1\xd8\xd0\xeb\xcd\xac\xd2\xe2\xd5\xbe\xb9\xe6";

	const char *error = register_invalid_user_name(r->id);
	if (error)
		return error;

	if (strcmp(r->pw, r->pw2) != 0)
		//% return "两次密码不匹配";
		return "\xc1\xbd\xb4\xce\xc3\xdc\xc2\xeb\xb2\xbb\xc6\xa5\xc5\xe4";

	error = register_invalid_password(r->pw, r->id);
	if (error)
		return error;

	struct userec user;
	init_userec(&user, r->id, r->pw, true);
	strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));

#ifndef FDQUAN
	char email[sizeof(user.email)];
	snprintf(email, sizeof(email), "%s@%s", r->mail, r->domain);
	if (!valid_addr(email) || !domain_allowed(email)
			|| !register_email_allowed(email)) {
		//% return "电子邮件地址无效，或不在允许范围";
		return "\xb5\xe7\xd7\xd3\xd3\xca\xbc\xfe\xb5\xd8\xd6\xb7\xce\xde\xd0\xa7\xa3\xac\xbb\xf2\xb2\xbb\xd4\xda\xd4\xca\xd0\xed\xb7\xb6\xce\xa7";
	}
	user.email[0] = '\0';
#endif // FDQUAN

	user.gender = r->gender[0];
	strlcpy(user.username, r->nick, sizeof(user.username));
	string_remove_non_printable_gbk(user.username);
	user.birthyear = r->year > 1900 ? r->year - 1900 : r->year;
	user.birthmonth = r->month;
	user.birthday = r->day;
	if (check_user_profile(&user) != 0
			|| strlen(r->name) < 4 || strlen(r->tel) < 8)
		//% return "请详细填写个人资料";
		return "\xc7\xeb\xcf\xea\xcf\xb8\xcc\xee\xd0\xb4\xb8\xf6\xc8\xcb\xd7\xca\xc1\xcf";

	if (create_user(&user) != 0)
		//% return "用户已存在，或出现其他内部错误";
		return "\xd3\xc3\xbb\xa7\xd2\xd1\xb4\xe6\xd4\xda\xa3\xac\xbb\xf2\xb3\xf6\xcf\xd6\xc6\xe4\xcb\xfb\xc4\xda\xb2\xbf\xb4\xed\xce\xf3";

	reginfo_t reg;
	memset(&reg, 0, sizeof(reg));
	strlcpy(reg.userid, r->id, sizeof(reg.userid));
	strlcpy(reg.realname, r->name, sizeof(reg.realname));
	strlcpy(reg.phone, r->tel, sizeof(reg.phone));
#ifndef FDQUAN
	strlcpy(reg.email, email, sizeof(reg.email));
#endif
	reg.regdate = now;

	char file[HOMELEN];
	snprintf(file, sizeof(file), "home/%c/%s",
			toupper(user.userid[0]), user.userid);
	if (mkdir(file, 0755) != 0)
		//% return "内部错误";
		return "\xc4\xda\xb2\xbf\xb4\xed\xce\xf3";

	//TODO: should be put in fcgi_activate
	if (register_save(&reg) != 0)
		//% return "提交注册资料失败";
		return "\xcc\xe1\xbd\xbb\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf\xca\xa7\xb0\xdc";

#ifndef FDQUAN
	if (register_send_email(&user, email) != 0)
		//% return "发送注册信失败";
		return "\xb7\xa2\xcb\xcd\xd7\xa2\xb2\xe1\xd0\xc5\xca\xa7\xb0\xdc";
#endif // FDQUAN

	return NULL;
}

int fcgi_reg(void)
{
	parse_post_data();
	reg_req_t request = {
		.id = get_param("id"),
		.pw = get_param("pw"),
		.pw2 = get_param("pw2"),
		.mail = get_param("mail"),
		.domain = get_param("domain"),
		.nick = get_param("nick"),
		.gender = get_param("gender"),
		.name = get_param("name"),
		.tel = get_param("tel"),
		.agree = get_param("agree"),
		.year = strtol(get_param("byear"), NULL, 10),
		.month = strtol(get_param("bmon"), NULL, 10),
		.day = strtol(get_param("bday"), NULL, 10)
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
	const char *code = get_param("code");
	const char *user = get_param("user");
	xml_header(NULL);
	printf("<bbsactivate success='%d'>", register_activate_email(user, code));
	print_session();
	printf("</bbsactivate>");
	return 0;
}

int fcgi_exist(void)
{
	const char *user = get_param("user");
	xml_header(NULL);
	printf("<bbsexist>%d</bbsexist>", searchuser(user) != 0);
	return 0;
}
