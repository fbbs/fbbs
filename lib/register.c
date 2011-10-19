#include "libBBS.h"
#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/register.h"
#include "fbbs/string.h"
#include "fbbs/util.h"

#define REGISTER_LIST "unregistered"

enum {
	MIN_ID_LEN = 2,
	MIN_PASSWORD_LEN = 4,
};

bool is_no_register(void)
{
	return dashf("NOREGISTER");
}

/* Add by Amigo. 2001.02.13. Called by ex_strcmp. */
/* Compares at most n characters of s2 to s1 from the tail. */
static int ci_strnbcmp(register char *s1, register char *s2, int n)
{
	char *s1_tail, *s2_tail;
	char c1, c2;

	s1_tail = s1 + strlen(s1) - 1;
	s2_tail = s2 + strlen(s2) - 1;

	while ( (s1_tail >= s1 ) && (s2_tail >= s2 ) && n --) {
		c1 = *s1_tail --;
		c2 = *s2_tail --;
		if (c1 >= 'a' && c1 <= 'z')
			c1 += 'A' - 'a';
		if (c2 >= 'a' && c2 <= 'z')
			c2 += 'A' - 'a';
		if (c1 != c2)
			return (c1 - c2);
	}
	if ( ++n)
		if ( (s1_tail < s1 ) && (s2_tail < s2 ))
			return 0;
		else if (s1_tail < s1)
			return -1;
		else
			return 1;
	else
		return 0;
}

/* Add by Amigo. 2001.02.13. Called by bad_user_id. */
/* Compares userid to restrictid. */
/* Restrictid support * match in three style: prefix*, *suffix, prefix*suffix. */
/* Prefix and suffix can't contain *. */
/* Modified by Amigo 2001.03.13. Add buffer strUID for userid. Replace all userid with strUID. */
static int ex_strcmp(const char *restrictid, const char *userid)
{
	/* Modified by Amigo 2001.03.13. Add definition for strUID. */
	char strBuf[STRLEN ], strUID[STRLEN ], *ptr;
	int intLength;

	/* Added by Amigo 2001.03.13. Add copy lower case userid to strUID. */
	intLength = 0;
	while ( *userid)
		if ( *userid >= 'A' && *userid <= 'Z')
			strUID[ intLength ++ ] = (*userid ++) - 'A' + 'a';
		else
			strUID[ intLength ++ ] = *userid ++;
	strUID[ intLength ] = '\0';

	intLength = 0;
	/* Modified by Amigo 2001.03.13. Copy lower case restrictid to strBuf. */
	while ( *restrictid)
		if ( *restrictid >= 'A' && *restrictid <= 'Z')
			strBuf[ intLength ++ ] = (*restrictid ++) - 'A' + 'a';
		else
			strBuf[ intLength ++ ] = *restrictid ++;
	strBuf[ intLength ] = '\0';

	if (strBuf[ 0 ] == '*' && strBuf[ intLength - 1 ] == '*') {
		strBuf[ intLength - 1 ] = '\0';
		if (strstr(strUID, strBuf + 1) != NULL)
			return 0;
		else
			return 1;
	} else if (strBuf[ intLength - 1 ] == '*') {
		strBuf[ intLength - 1 ] = '\0';
		return strncasecmp(strBuf, strUID, intLength - 1);
	} else if (strBuf[ 0 ] == '*') {
		return ci_strnbcmp(strBuf + 1, strUID, intLength - 1);
	} else if ( (ptr = strstr(strBuf, "*") ) != NULL) {
		return (strncasecmp(strBuf, strUID, ptr - strBuf) || ci_strnbcmp(
				strBuf, strUID, intLength - 1 - (ptr - strBuf )) );
	}
	return 1;
}
/*
 Commented by Erebus 2004-11-08 called by getnewuserid(),new_register()
 configure ".badname" to restrict user id
 */
static int bad_user_id(const char *userid)
{
	FILE *fp;
	char buf[STRLEN];
	const char *ptr = userid;
	char ch;

	while ( (ch = *ptr++) != '\0') {
		if ( !isalnum(ch) && ch != '_')
			return 1;
	}
	if ( (fp = fopen(".badname", "r")) != NULL) {
		while (fgets(buf, STRLEN, fp) != NULL) {
			ptr = strtok(buf, " \n\t\r");
			/* Modified by Amigo. 2001.02.13.8. * match support added. */
			/* Original: if( ptr != NULL && *ptr != '#' && strcasecmp( ptr, userid ) == 0 ) {*/
			if (ptr != NULL && *ptr != '#' && (strcasecmp(ptr, userid) == 0
					|| ex_strcmp(ptr, userid) == 0 )) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

/*2003.06.02 stephen modify end*/

/**
 *
 */
static bool strisalpha(const char *str)
{
	for (const char *s = str; *s != '\0'; s++)
		if (!isalpha(*s))
			return false;
	return true;
}

/**
 *
 */
const char *invalid_userid(const char *userid)
{
	if (!strisalpha(userid))
		return "帐号必须全为英文字母\n";
	if (strlen(userid) < MIN_ID_LEN || strlen(userid) > IDLEN)
		return "帐号长度应为2~12个字符\n";
	if (bad_user_id(userid))
		return "抱歉, 您不能使用这个字作为帐号\n";
	return NULL;
}

const char *invalid_password(const char *password, const char *userid)
{
	if (strlen(password) < MIN_PASSWORD_LEN || !strcmp(password, userid))
		return "密码太短或与使用者代号相同, 请重新输入\n";
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
 * Send registration activation mail.
 * @param user The user.
 * @param mail The email address.
 * @return 0 if OK, -1 on error.
 */
int send_regmail(const struct userec *user, const char *mail)
{
	char password[RNDPASSLEN + 1];
	generate_random_password(password, sizeof(password));

	char file[HOMELEN];
	sethomefile(file, user->userid, REG_CODE_FILE);
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	fprintf(fp, "%s\n%s\n", password, mail);
	fclose(fp);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s %s", MTA, mail);
	FILE *fout = popen(buf, "w");
	if (!fout)
		return -1;
	fprintf(fout, "To: %s\n"
			"Subject: [%s]邮件验证(Registration Email)\n"
			"X-Purpose: %s registration mail.\n\n",
			mail, BBSNAME, BBSNAME);
	fprintf(fout, "感谢您注册 %s BBS站\n"
			"Thank you for your registration.\n\n"
			"请访问以下链接激活账号:\n"
			"Please use the following link to activate your account:\n"
			"http://%s/bbs/activate?user=%s&code=%s\n\n"
			"您也可以使用telnet登录输入下列验证码激活账号:\n"
			"You can also enter the code that follows on telnet login\n"
			"验证码(Validation code)    : %s (请注意大小写/Case sensitive)\n\n",
			BBSNAME, BBSHOST, user->userid, password, password);
	fprintf(fout, "附加信息(Additional Info)\n"
			"本站地址(Site address)     : %s (%s)\n"
			"您注册的账号(Your account) : %s\n"
			"申请日期(Application date) : %s\n\n",
			BBSHOST, BBSIP, user->userid, ctime((time_t *)&user->firstlogin));
	fprintf(fout, "本信件由系统自动发送，请不要回复。\n"
			"Note: this is an automated email. Please don't reply.\n"
			"如果您从未注册以上账号，请忽略此信。\n"
			"If you have never registered this account, please ignore the mail.\n");
	fclose(fout);
	return 0;
}

bool activate_email(const char *userid, const char *attempt)
{
	char file[HOMELEN];
	sethomefile(file, userid, REG_CODE_FILE);
	FILE *fp = fopen(file, "r");
	if (!fp)
		return false;

	char code[RNDPASSLEN + 1], email[EMAIL_LEN];
	fscanf(fp, "%s", code);
	fscanf(fp, "%s", email);
	fclose(fp);

	if (strcmp(code, attempt) != 0)
		return false;

	int num = getuserec(userid, &currentuser);
	if (!num)
		return false;

	currentuser.userlevel |= (PERM_DEFAULT | PERM_BINDMAIL);
	strlcpy(currentuser.email, email, sizeof(currentuser.email));
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
	while (fread(&reg, sizeof(reg), 1, fp)) {
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
	fb_flock(fileno(fp), LOCK_EX);

	int found = 0;
	reginfo_t tmp;
	while (fread(&tmp, sizeof(tmp), 1, fp)) {
		if (strcasecmp(reg->userid, tmp.userid) == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		fwrite(reg, sizeof(*reg), 1, fp);
	}

	fb_flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0 - found;
}

bool is_banned_email(const char *mail)
{
	char tmp[128];
	FILE *fp = fopen(".bad_email", "r");
	if (fp) {
		while (fgets(tmp, sizeof(tmp), fp)) {
			strtok(tmp, "# \t\r\n");
			if (strcasecmp(tmp, mail) == 0) {
				fclose(fp);
				return true;
			}
		}
		fclose(fp);
	}
	return false;
}

bool domain_allowed(const char *mail)
{
	return strstr(mail, "@fudan.edu.cn") || strstr(mail, "@alu.fudan.edu.cn");
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

int save_register_file(const reginfo_t *reg)
{
	char file[HOMELEN];
	sethomefile(file, reg->userid, "register");

	FILE *fp = fopen(file, "a");
	if (fp) {
		fprintf(fp, "注册时间     : %s\n", getdatestring(reg->regdate, DATE_EN));
		fprintf(fp, "申请帐号     : %s\n", reg->userid);
		fprintf(fp, "真实姓名     : %s\n", reg->realname);
		fprintf(fp, "联络电话     : %s\n", reg->phone);
#ifndef FDQUAN
		fprintf(fp, "电子邮件     : %s\n", reg->email);
#endif
		fclose(fp);
	}

	return 0;
}

