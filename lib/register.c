#include "libBBS.h"
#include "register.h"
#include "bbs.h"

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

char *genrandpwd(int seed) {
	char panel[]=
			"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *result;
	int i, rnd;

	result = (char *) malloc(RNDPASSLEN + 1);
	srand((unsigned) (time(0) * seed));
	memset(result, 0, RNDPASSLEN + 1);
	for (i = 0; i < RNDPASSLEN; i++) {
		rnd = rand() % sizeof(panel);
		if (panel[rnd] == '\0') {
			i--;
			continue;
		}
		result[i] = panel[rnd];
	}
	sethomefile(genbuf, currentuser.userid, ".regpass");
	unlink(genbuf);
	file_append(genbuf, result);
	return ((char *) result);
}

void regmail_send(struct userec *trec, char* mail) {
	time_t code;
	FILE *fout, *dp;
	char buf[RNDPASSLEN + 1];
	sprintf(buf, "%s", (char *) genrandpwd((int) getpid()));
	sethomefile(genbuf, trec->userid, ".regpass");
	if ((dp = fopen(genbuf, "w")) == NULL)
		return;
	dp = fopen(genbuf, "w+");
	fprintf(dp, "%s\n", buf);
	fprintf(dp, "%s\n", mail);
	fclose(dp);

	code = time(0);
	sprintf(genbuf, "%s -f %s.bbs@%s %s", MTA, trec->userid, BBSHOST, mail);
	fout = popen(genbuf, "w");
	if (fout != NULL) {
		fprintf(fout, "Reply-To: SYSOP.bbs@%s\n", BBSHOST);
		fprintf(fout, "From: SYSOP.bbs@%s\n", BBSHOST);
		fprintf(fout, "To: %s\n", mail);
		fprintf(fout, "Subject: %s@%s mail check.\n", trec->userid, BBSID);
		fprintf(fout, "X-Purpose: %s registration mail.\n", BBSNAME);
		fprintf(fout, "\n");
		fprintf(fout, "[中文]\n");
		fprintf(fout, "BBS 位址         : %s (%s)\n", BBSHOST, BBSIP);
		fprintf(fout, "您注册的 BBS ID  : %s\n", trec->userid);
		fprintf(fout, "申请日期         : %s", ctime(&trec->firstlogin));
		fprintf(fout, "认证码           : %s (请注意大小写)\n", buf);
		fprintf(fout, "认证信发出日期   : %s\n", ctime(&code));

		fprintf(fout, "[English]\n");
		fprintf(fout, "BBS LOCATION     : %s (%s)\n", BBSHOST, BBSIP);
		fprintf(fout, "YOUR BBS USER ID : %s\n", trec->userid);
		fprintf(fout, "APPLICATION DATE : %s", ctime(&trec->firstlogin));
		fprintf(fout, "YOUR NICK NAME   : %s\n", trec->username);
		fprintf(fout, "VALID CODE       : %s (case sensitive)\n", buf);
		fprintf(fout, "THIS MAIL SENT ON: %s\n", ctime(&code));

		fprintf(fout, ".\n");
		fclose(fout);
	}

}
