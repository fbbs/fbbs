#include <bbs.h>

// Returns the path of 'filename' under the home directory of 'userid'.
char *sethomefile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, "home/%c/%s/%s", toupper(userid[0]), userid, filename);
	return buf;
}

// Returns the path of board 'boardname'.
char *setbpath(char *buf, const char *boardname)
{
	strcpy(buf, "boards/");
	strcat(buf, boardname);
	return buf;
}

// Returns the path of DOT_DIR file under the directory of 'boardname'.
char *setwbdir(char *buf, const char *boardname)
{
	sprintf (buf, "boards/%s/" DOT_DIR, boardname);
	return buf;
}

// Returns the path of 'filename' under the directory of 'boardname'.
char *setbfile(char *buf, const char *boardname, const char *filename)
{
	sprintf(buf, "boards/%s/%s", boardname, filename);
	return buf;
}

// Returns the path of 'filename' under the mail directory of 'userid'.
char *setmfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, "mail/%c/%s/%s", toupper(userid[0]), userid, filename);
	return buf;
}

// Returns the path of '.DIR' under the mail directory of 'userid'.
char *setmdir(char *buf, const char *userid)
{
	sprintf(buf, "mail/%c/%s/" DOT_DIR, toupper(userid[0]), userid);
	return buf;
}

sigjmp_buf bus_jump;
void sigbus(int signo)
{
	siglongjmp(bus_jump, 1);
}

// Sends signal 'sig' to 'user'.
// Returns 0 on success (the same as kill does), -1 on error.
// If the 'user' is web user, does not send signal and returns -1.
int bbskill(const struct user_info *user, int sig)
{
	if (user == NULL)
		return -1;

	if (user->pid > 0) {
		if (user->mode != WWW) {
			return kill(user->pid, sig);
		} else {
			// Since web users have no forked processes,
			// do not send signals to pid.
			// Implementation TBD
			return 0;
		}
	}
	// Sending signals to multiple processes is not allowed.
	return -1;
}

// Search 'uid' in id-host pairs in "etc/special.ini"(case insensitive)
// and then modify 'host' accordingly.
// 'len' should be the length of 'host'.
void SpecialID(const char *uid, char *host, int len)
{
	FILE *fp;
	char line[STRLEN];
	char *special;

	if ((fp = fopen("etc/special.ini", "r")) != NULL) {
		while (fgets(line, sizeof(line), fp)) {
			special = strtok(line, " \r\n\t");
			if (special && !strcasecmp(uid, special)) {
				special = strtok(NULL, " \r\n\t");
				if (special)
					strlcpy(host, special, len);
				break;
			}
		}
		fclose(fp);
	}
}

// Convert time to string in specified format.
// mode: DATE_ZH -     "2001年02月03日04:05:06 星期六"
//       DATE_EN -     "02/03/01 04:05:06"
//       DATE_SHORT -  "02.03 04:05"
//       DATE_ENWEEK - "02/03/01 04:05:06 Sat"
//       DATE_XML -    "2001-02-03T04:05:06"
char *getdatestring(time_t time, enum DATE_FORMAT mode)
{
	static char str[30] = {'\0'};
	struct tm *t;
	char weeknum[7][3] = {"天", "一", "二", "三", "四", "五", "六"};
	char engweek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

	// No multi-thread
	t = localtime(&time);
	switch (mode) {
		case DATE_ZH:
			// using sprintf here is safe
			sprintf(str, "%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec, weeknum[t->tm_wday]);
			break;
		case DATE_EN:
			sprintf(str, "%02d/%02d/%02d %02d:%02d:%02d",
					t->tm_mon + 1, t->tm_mday, t->tm_year - 100,
					t->tm_hour, t->tm_min, t->tm_sec);
			break;
		case DATE_SHORT:
			sprintf(str, "%02d.%02d %02d:%02d",
					t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
			break;
		case DATE_ENWEEK:
			sprintf(str, "%02d/%02d/%02d %02d:%02d:%02d %3s",
					t->tm_mon + 1, t->tm_mday, t->tm_year - 100,
					t->tm_hour, t->tm_min, t->tm_sec, engweek[t->tm_wday]);
			break;
		case DATE_XML:
		default:
			sprintf(str, "%4d-%02d-%02dT%02d:%02d:%02d",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec);
			break;
	}
	return str;
}

bool seek_in_file(const char *filename, const char *seekstr)
{
	FILE* fp;
	char buf[STRLEN];
	char *namep;
	if ((fp = fopen(filename, "r")) == NULL)
		return false;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return true;
		}
	}
	fclose(fp);
	return false;
}
