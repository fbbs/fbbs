#include "libweb.h"

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
	xml_header("bbsinfo");
	if (*type != '\0') {
		printf("<bbsinfo p='%s' u='%s'>", get_permission(),
				currentuser.userid);
		printf("%s</bbsinfo>", check_info());
	} else {
		printf("<bbsinfo p='%s' u='%s' post='%d' login='%d' stay='%d' "
				"since='%s' host='%s' year='%d' month='%d' "
				"day='%d' gender='%c'", get_permission(), 
				currentuser.userid, currentuser.numposts,
				currentuser.numlogins, currentuser.stay / 60,
				getdatestring(currentuser.firstlogin, DATE_XML),
				currentuser.lasthost, currentuser.birthyear,
				currentuser.birthmonth, currentuser.birthday,
				currentuser.gender);
		printf(" last='%s'><nick>",
				getdatestring(currentuser.lastlogin, DATE_XML));
		xml_fputs(currentuser.username, stdout);
		printf("</nick></bbsinfo>");
	}
	return 0;
}
