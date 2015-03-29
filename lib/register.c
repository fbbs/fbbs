#include <fnmatch.h>
#include "bbs.h"
#include "fbbs/dbi.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/register.h"
#include "fbbs/string.h"
#include "fbbs/util.h"

#define REGISTER_LIST "unregistered"

enum {
	MIN_USER_NAME_LEN = 2,
	MIN_PASSWORD_LEN = 4,
	FILE_MATCH_CONTINUE = -1,
};

/**
 * 检查目前是否关闭注册
 * @return 目前关闭注册返回true, 否则false
 */
bool register_closed(void)
{
	return dashf("NOREGISTER");
}

typedef int (*file_match_callback_t)(const char *pattern, const char *string);

static bool file_match(const char *file, const char *string, char *buf,
		size_t size, file_match_callback_t callback, bool _default)
{
	FILE *fp = fopen(file, "r");
	if (fp) {
		while (fgets(buf, size, fp)) {
			char *ptr = strtok(buf, "# \t\n\r");
			if (ptr) {
				int ret = callback(ptr, string);
				if (ret != FILE_MATCH_CONTINUE) {
					fclose(fp);
					return ret;
				}
			}
		}
		fclose(fp);
	}
	return _default;
}

static int user_name_allowed_callback(const char *pattern, const char *uname)
{
	if (fnmatch(pattern, uname, FNM_CASEFOLD) == 0)
		return false;
	return FILE_MATCH_CONTINUE;
}

/**
 * 检查用户名是否在禁止注册列表中
 * @param uname 用户名
 * @return 用户名被禁止返回true, 否则false
 */
static bool user_name_allowed(const char *uname)
{
	char buf[STRLEN];
	return file_match(".badname", uname, buf, sizeof(buf),
			user_name_allowed_callback, true);
}

/**
 * 测试字符串是否全为英文字符
 * @param str 字符串
 * @return 全为英文字符返回true，否则false
 */
static bool string_is_alpha(const char *str)
{
	for (const char *s = str; *s; ++s)
		if (!isalpha(*s))
			return false;
	return true;
}

/**
 * 检查用户名是否合法
 * @param uname 用户名
 * @return 若用户名合法返回NULL, 否则返回错误信息
 */
const char *register_invalid_user_name(const char *uname)
{
	if (!string_is_alpha(uname))
		//% "帐号必须全为英文字母\n"
		return "\xd5\xca\xba\xc5\xb1\xd8\xd0\xeb\xc8\xab\xce\xaa"
				"\xd3\xa2\xce\xc4\xd7\xd6\xc4\xb8\n";
	if (strlen(uname) > IDLEN || strlen(uname) < MIN_USER_NAME_LEN)
		//% "帐号长度应为2~12个字符\n"
		return "\xd5\xca\xba\xc5\xb3\xa4\xb6\xc8\xd3\xa6\xce\xaa""2~12"
				"\xb8\xf6\xd7\xd6\xb7\xfb\n";
	if (!user_name_allowed(uname))
		//% "抱歉, 您不能使用这个字作为帐号\n"
		return "\xb1\xa7\xc7\xb8, \xc4\xfa\xb2\xbb\xc4\xdc\xca\xb9\xd3\xc3"
				"\xd5\xe2\xb8\xf6\xd7\xd6\xd7\xf7\xce\xaa\xd5\xca\xba\xc5\n";
	return NULL;
}

/**
 * 检查密码是否合法
 * @param password 密码
 * @param uname 用户名
 * @return 若密码合法返回NULL, 否则返回错误信息
 */
const char *register_invalid_password(const char *password, const char *uname)
{
	if (strlen(password) < MIN_PASSWORD_LEN || streq(password, uname))
		//% "密码太短或与使用者代号相同, 请重新输入\n"
		return "\xc3\xdc\xc2\xeb\xcc\xab\xb6\xcc\xbb\xf2\xd3\xeb"
				"\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5\xcf\xe0\xcd\xac,"
				" \xc7\xeb\xd6\xd8\xd0\xc2\xca\xe4\xc8\xeb\n";
	return NULL;
}

/**
 * Generate random password.
 * @param[out] buf The buffer.
 * @param[in] size The buffer size.
 */
static void generate_random_password(char *buf, size_t size)
{
	const char panel[]=
			"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	read_urandom(buf, size - 1);

	for (int i = 0; i < size; ++i) {
		buf[i] = panel[buf[i] % (sizeof(panel) - 1)];
	}

	buf[size - 1] = '\0';
}

/**
 * 发送注册验证信
 * @param user 用户
 * @param email 电子邮件地址
 * @return 0 if OK, -1 on error.
 */
int register_send_email(const struct userec *user, const char *email)
{
	char password[RNDPASSLEN + 1];
	generate_random_password(password, sizeof(password));

	char file[HOMELEN];
	sethomefile(file, user->userid, REG_CODE_FILE);
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	fprintf(fp, "%s\n%s\n", password, email);
	fclose(fp);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s %s", MTA, email);
	FILE *fout = popen(buf, "w");
	if (!fout)
		return -1;
	fprintf(fout, "To: %s\n"
			//% "Subject: [%s]邮件验证(Registration Email)\n"
			"Subject: [%s]\xd3\xca\xbc\xfe\xd1\xe9\xd6\xa4(Registration Email)\n"
			"X-Purpose: %s registration mail.\n\n",
			email, BBSNAME, BBSNAME);
	//% fprintf(fout, "感谢您注册 %s BBS站\n"
	fprintf(fout, "\xb8\xd0\xd0\xbb\xc4\xfa\xd7\xa2\xb2\xe1 %s BBS\xd5\xbe\n"
			"Thank you for your registration.\n\n"
			//% "请访问以下链接激活账号:\n"
			"\xc7\xeb\xb7\xc3\xce\xca\xd2\xd4\xcf\xc2\xc1\xb4\xbd\xd3\xbc\xa4\xbb\xee\xd5\xcb\xba\xc5:\n"
			"Please use the following link to activate your account:\n"
			"http://%s/bbs/activate?user=%s&code=%s\n\n"
			//% "您也可以使用telnet登录输入下列验证码激活账号:\n"
			"\xc4\xfa\xd2\xb2\xbf\xc9\xd2\xd4\xca\xb9\xd3\xc3telnet\xb5\xc7\xc2\xbc\xca\xe4\xc8\xeb\xcf\xc2\xc1\xd0\xd1\xe9\xd6\xa4\xc2\xeb\xbc\xa4\xbb\xee\xd5\xcb\xba\xc5:\n"
			"You can also enter the code that follows on telnet login\n"
			//% "验证码(Validation code)    : %s (请注意大小写/Case sensitive)\n\n",
			"\xd1\xe9\xd6\xa4\xc2\xeb(Validation code)    : %s (\xc7\xeb\xd7\xa2\xd2\xe2\xb4\xf3\xd0\xa1\xd0\xb4/Case sensitive)\n\n",
			BBSNAME, BBSHOST, user->userid, password, password);
	time_t firstlogin = user->firstlogin;
	//% "附加信息(Additional Info)\n"
	fprintf(fout, "\xb8\xbd\xbc\xd3\xd0\xc5\xcf\xa2(Additional Info)\n"
			//% "本站地址(Site address)     : %s (%s)\n"
			"\xb1\xbe\xd5\xbe\xb5\xd8\xd6\xb7(Site address)     : %s (%s)\n"
			//% "您注册的账号(Your account) : %s\n"
			"\xc4\xfa\xd7\xa2\xb2\xe1\xb5\xc4\xd5\xcb\xba\xc5(Your account) : %s\n"
			//% "申请日期(Application date) : %s\n\n",
			"\xc9\xea\xc7\xeb\xc8\xd5\xc6\xda(Application date) : %s\n\n",
			BBSHOST, BBSIP, user->userid, ctime(&firstlogin));
	//% "本信件由系统自动发送，请不要回复。\n"
	fprintf(fout, "\xb1\xbe\xd0\xc5\xbc\xfe\xd3\xc9\xcf\xb5\xcd\xb3\xd7\xd4\xb6\xaf\xb7\xa2\xcb\xcd\xa3\xac\xc7\xeb\xb2\xbb\xd2\xaa\xbb\xd8\xb8\xb4\xa1\xa3\n"
			"Note: this is an automated email. Please don't reply.\n"
			//% "如果您从未注册以上账号，请忽略此信。\n"
			"\xc8\xe7\xb9\xfb\xc4\xfa\xb4\xd3\xce\xb4\xd7\xa2\xb2\xe1\xd2\xd4\xc9\xcf\xd5\xcb\xba\xc5\xa3\xac\xc7\xeb\xba\xf6\xc2\xd4\xb4\xcb\xd0\xc5\xa1\xa3\n"
			"If you have never registered this account, please ignore the mail.\n");
	pclose(fout);
	return 0;
}

static int insert_email(const char *email)
{
	db_res_t *res = db_cmd("INSERT INTO emails (addr) VALUES (%s)", email);
	db_clear(res);

	int id = 0;
	res = db_query("SELECT id FROM emails WHERE addr = %s", email);
	if (res && db_res_rows(res) == 1)
		id = db_get_integer(res, 0, 0);
	db_clear(res);
	return id;
}

static bool activate_email(const char *uname, const char *email)
{
	int id = insert_email(email);
	if (!id)
		return false;

	db_res_t *res = db_cmd("UPDATE users SET email = %d"
			" WHERE alive AND lower(name) = lower(%s)", id, uname);
	if (!res)
		return false;

	db_clear(res);
	return true;
}

/**
 * 激活注册邮箱
 * @param uname 用户名
 * @param attempt 激活码
 * @return 激活成功与否
 */
bool register_activate_email(const char *uname, const char *attempt)
{
	char file[HOMELEN];
	sethomefile(file, uname, REG_CODE_FILE);
	FILE *fp = fopen(file, "r");
	if (!fp)
		return false;

	char code[RNDPASSLEN + 1], email[EMAIL_LEN + 1];
	bool s1 = fscanf(fp, "%10s", code) == 1;
	bool s2 = fscanf(fp, "%40s", email) == 1;
	fclose(fp);

	if (!s1 || !s2)
		return false;
	if (!streq(code, attempt))
		return false;

	if (!activate_email(uname, email))
		return false;

	int num = getuserec(uname, &currentuser);
	if (!num)
		return false;
	currentuser.userlevel |= (PERM_DEFAULT | PERM_BINDMAIL);
	substitut_record(NULL, &currentuser, sizeof(currentuser), num);

	unlink(file);
	return true;
}

bool is_reg_pending(const char *userid)
{
	FILE *fp = fopen(REGISTER_LIST, "r");
	if (!fp)
		return false;

	reginfo_t reg;
	while (fread(&reg, sizeof(reg), 1, fp) == 1) {
		if (strcasecmp(reg.userid, userid) == 0) {
			fclose(fp);
			return true;
		}
	}

	fclose(fp);
	return false;
}

/**
 * Append register info to pending list.
 * @param reg The register info.
 * @return 0 if OK, -1 on error or duplication.
 */
int append_reg_list(const reginfo_t *reg)
{
	FILE *fp = fopen(REGISTER_LIST, "r+");
	if (!fp)
		return -1;
	if (file_lock_all(fileno(fp), FILE_WRLCK) != 0) {
		fclose(fp);
		return -1;
	}

	int found = 0;
	reginfo_t tmp;
	while (fread(&tmp, sizeof(tmp), 1, fp) == 1) {
		if (strcasecmp(reg->userid, tmp.userid) == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		fwrite(reg, sizeof(*reg), 1, fp);
	}

	(void) file_lock_all(fileno(fp), FILE_UNLCK);
	fclose(fp);
	return 0 - found;
}

static int email_allowed_callback(const char *pattern, const char *email)
{
	if (strcaseeq(pattern, email))
		return false;
	return FILE_MATCH_CONTINUE;
}

bool register_email_allowed(const char *email)
{
	char buf[128];
	return file_match(".bad_email", email, buf, sizeof(buf),
			email_allowed_callback, true);
}

static int domain_allowed_callback(const char *pattern, const char *domain)
{
	bool allowed = true;
	if (*pattern == '-') {
		allowed = false;
		++pattern;
	}
	if (fnmatch(pattern, domain, 0) == 0)
		return allowed;
	return FILE_MATCH_CONTINUE;
}

bool register_domain_allowed(const char *email)
{
	const char *domain = strchr(email, '@');
	if (!domain)
		return false;

	++domain;
	if (streq(domain, "fudan.edu.cn") || streq(domain, "alu.fudan.edu.cn"))
		return true;

	char buf[80];
	return file_match(".domain", domain, buf, sizeof(buf),
			domain_allowed_callback, false);
}

/**
 * Initialize a basic user record.
 */
void init_userec(struct userec *user, const char *userid,
		const char *passwd, bool usegbk)
{
	memset(user, 0, sizeof(*user));
	strlcpy(user->userid, userid, sizeof(user->userid));
	strlcpy(user->passwd, genpasswd(passwd), ENCPASSLEN);

	user->gender = 'X';
#ifdef ALLOWGAME
	user->money = 1000;
#endif
	user->userdefine = ~0;
	if (!strcmp(userid, "guest")) {
		user->userlevel = 0;
		user->userdefine &= ~(DEF_FRIENDCALL | DEF_ALLMSG | DEF_FRIENDMSG);
	} else {
		user->userlevel = PERM_LOGIN;
		user->flags[0] = PAGER_FLAG;
	}
	user->userdefine &= ~(DEF_NOLOGINSEND);

	if (!usegbk)
		user->userdefine &= ~DEF_USEGB;

	user->flags[1] = 0;
	user->firstlogin = user->lastlogin = time(NULL);
}

int register_save(const reginfo_t *reg)
{
	char file[HOMELEN];
	sethomefile(file, reg->userid, "register");

	FILE *fp = fopen(file, "a");
	if (fp) {
		//% "注册时间     : %s\n"
		fprintf(fp, "\xd7\xa2\xb2\xe1\xca\xb1\xbc\xe4     : %s\n", format_time(reg->regdate, TIME_FORMAT_EN));
		//% "申请帐号     : %s\n"
		fprintf(fp, "\xc9\xea\xc7\xeb\xd5\xca\xba\xc5     : %s\n", reg->userid);
		//% "真实姓名     : %s\n"
		fprintf(fp, "\xd5\xe6\xca\xb5\xd0\xd5\xc3\xfb     : %s\n", reg->realname);
		//% "联络电话     : %s\n"
		fprintf(fp, "\xc1\xaa\xc2\xe7\xb5\xe7\xbb\xb0     : %s\n", reg->phone);
#ifndef FDQUAN
		//% "电子邮件     : %s\n"
		fprintf(fp, "\xb5\xe7\xd7\xd3\xd3\xca\xbc\xfe     : %s\n", reg->email);
#endif
		fclose(fp);
	}

	return 0;
}

