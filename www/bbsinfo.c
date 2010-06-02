#include "libweb.h"
#include "record.h"

/**
 * Check user info validity.
 * @return empty string on success, error msg otherwise.
 */
static char *check_info(void)
{
	unsigned char *nick;
	nick = (unsigned char *)getparm("nick");
	unsigned char *t2 = nick;
	while (*t2 != '\0') {
		if (*t2 < 0x20 || *t2 == 0xFF)
			return "昵称太短或包含非法字符";
		t2++;
	}
	strlcpy(currentuser.username, (char *)nick, sizeof(currentuser.username));

	// TODO: more accurate birthday check.
	char *tmp = getparm("year");
	long num = strtol(tmp, NULL, 10);
	if (num < 1910 || num > 1998)
		return "错误的出生年份";
	else
		currentuser.birthyear = num - 1900;

	tmp = getparm("month");
	num = strtol(tmp, NULL, 10);
	if (num <= 0 || num > 12)
		return "错误的出生月份";
	else
		currentuser.birthmonth = num;

	tmp = getparm("day");
	num = strtol(tmp, NULL, 10);
	if (num <= 0 || num > 31)
		return "错误的出生日期";
	else
		currentuser.birthday = num;

	tmp = getparm("gender");
	if (*tmp == 'M')
		currentuser.gender = 'M';
	else
		currentuser.gender = 'F';

	save_user_data(&currentuser);
	return "";
}

int bbsinfo_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	parse_post_data();
	char *type = getparm("type");
	xml_header("bbs");
	if (*type != '\0') {
		printf("<bbsinfo ");
		print_session();
		printf(">%s</bbsinfo>", check_info());
	} else {
		printf("<bbsinfo post='%d' login='%d' stay='%d' "
				"since='%s' host='%s' year='%d' month='%d' "
				"day='%d' gender='%c' ", currentuser.numposts,
				currentuser.numlogins, currentuser.stay / 60,
				getdatestring(currentuser.firstlogin, DATE_XML),
				currentuser.lasthost, currentuser.birthyear,
				currentuser.birthmonth, currentuser.birthday,
				currentuser.gender);
		print_session();
		printf(" last='%s'><nick>",
				getdatestring(currentuser.lastlogin, DATE_XML));
		xml_fputs(currentuser.username, stdout);
		printf("</nick></bbsinfo>");
	}
	return 0;
}

static int set_password(const char *orig, const char *new1, const char *new2)
{
	if (!checkpasswd(currentuser.passwd, orig))
		return BBS_EWPSWD;
	if (strcmp(new1, new2))
		return BBS_EINVAL;
	if (strlen(new1) < 2)
		return BBS_EINVAL;
	strlcpy(currentuser.passwd, crypt(new1, new1), sizeof(currentuser.passwd));
	save_user_data(&currentuser);
	return 0;
}

int bbspwd_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	parse_post_data();
	xml_header("bbs");
	printf("<bbspwd ");
	print_session();
	char *pw1 = getparm("pw1");
	if (*pw1 == '\0') {
		printf(" i='i'></bbspwd>");
		return 0;
	}
	printf(">", stdout);
	char *pw2 = getparm("pw2");
	char *pw3 = getparm("pw3");
	switch (set_password(pw1, pw2, pw3)) {
		case BBS_EWPSWD:
			printf("密码错误");
			break;
		case BBS_EINVAL:
			printf("新密码不匹配 或 新密码太短");
			break;
		default:
			break;
	}
	printf("</bbspwd>");
	return 0;
}

typedef struct mail_count_t {
	time_t limit;
	int count;
} mail_count_t;

static int count_new_mail(void *buf, int count, void *args)
{
	struct fileheader *fp = buf;
	mail_count_t *cp = args;
	time_t date = getfiletime(fp);
	if (date < cp->limit)
		return QUIT;
	if (!(fp->accessed[0] & FILE_READ))
		cp->count++;
	return 0;
}

int bbsidle_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	printf("Content-type: text/xml; charset="CHARSET"\n\n"
			"<?xml version=\"1.0\" encoding=\""CHARSET"\"?>\n<bbsidle");
	char file[HOMELEN];
	setmdir(file, currentuser.userid);
	mail_count_t c;
	c.limit = time(NULL) - 24 * 60 * 60 * NEWMAIL_EXPIRE;
	c.count = 0;
	apply_record(file, count_new_mail, sizeof(struct fileheader), &c, false,
			true, true);
	printf(" mail='%d'></bbsidle>", c.count);
	return 0;
}
