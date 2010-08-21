#include "libBBS.h"
#include "bbs.h"
#include "fbbs/register.h"

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

int check_userid(const char *userid)
{
	if (!strisalpha(userid))
		return BBS_EREG_NONALPHA;
	if (strlen(userid) < 2)
		return BBS_EREG_SHORT;
	if (bad_user_id(userid))
		return BBS_EREG_BADNAME;
	return 0;
}

/**
 * Read from /dev/urandom.
 * @param buf The buffer.
 * @param size Bytes to read.
 * @return 0 if OK, -1 on error.
 */
static int read_urandom(char *buf, size_t size)
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;
	if (read(fd, buf, size) < size)
		return -1;
	close(fd);
	return 0;
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
		buf[i] = panel[buf[i] % sizeof(panel)];
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
	sethomefile(file, user->userid, ".regpass");
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	fprintf(fp, "%s\n%s\n", password, mail);
	fclose(fp);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s -f %s.bbs@%s %s", MTA, user->userid,
			BBSHOST, mail);
	FILE *fout = popen(buf, "w");
	if (!fout)
		return -1;
	fprintf(fout, "Reply-To: SYSOP.bbs@%s\n"
			"From: SYSOP.bbs@%s\n"
			"To: %s\n"
			"Subject: %s@%s mail check.\n"
			"X-Purpose: %s registration mail.\n\n",
			BBSHOST, BBSHOST, mail, user->userid, BBSID, BBSNAME);
	fprintf(fout, "[中文]\n"
			"BBS 位址         : %s (%s)\n"
			"您注册的 BBS ID  : %s\n"
			"申请日期         : %s"
			"认证码           : %s (请注意大小写)\n",
			BBSHOST, BBSIP, user->userid, ctime(&user->firstlogin), password);
	fprintf(fout, "[English]\n"
			"BBS LOCATION     : %s (%s)\n"
			"YOUR BBS USER ID : %s\n"
			"APPLICATION DATE : %s"
			"VALID CODE       : %s (case sensitive)\n",
			BBSHOST, BBSIP, user->userid, ctime(&user->firstlogin), password);
	fclose(fout);
	return 0;
}
