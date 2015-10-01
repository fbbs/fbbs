#include <signal.h>
#include "bbs.h"
#include "mmap.h"
#include "fbbs/cfg.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/uinfo.h"

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

int bbs_kill(session_id_t sid, int pid, int sig)
{
	int r = 0;
	if (pid > 0) {
		if ((r = kill(pid, sig)) == 0)
			return 0;
	}

	if (sig == SIGHUP) {
		if (sid <= 0)
			return -1;
		return session_destroy(sid);
	} else {
		return r;
	}
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

int add_to_file(const char *file, const char *str, size_t len, bool overwrite,
		bool (*equal)(const char *, size_t, const char *, size_t))
{
	char fnew[HOMELEN], buf[LINE_BUFSIZE];
	if (snprintf(fnew, sizeof(fnew), "%s.%d", file, getpid()) >= sizeof(fnew))
		return -1;
	FILE *fpr = fopen(file, "r");
	FILE *fpw = fopen(fnew, "w");
	if (!fpw) {
		if (fpr)
			fclose(fpr);
		return -1;
	}
	bool exist = false;
	if (fpr) {
		file_lock_all(fileno(fpr), FILE_WRLCK);
		while (fgets(buf, sizeof(buf), fpr)) {
			if (!exist && equal && (*equal)(buf, sizeof(buf), str, len)) {
				exist = true;
				if (overwrite)
					fputs(str, fpw);
				else
					break;
			} else {
				fputs(buf, fpw);
			}
		}
	}
	if (!exist)
		fputs(str, fpw);
	fclose(fpw);
	if (fpr) {
		file_lock_all(fileno(fpr), FILE_UNLCK);
		fclose(fpr);
	}
	if (!overwrite && exist) {
		unlink(fnew);
		return BBS_ELEXIST;
	} else {
		return rename(fnew, file);
	}
}


/**
 * 从文件中删除指定行.
 * @param[in] file 需要修改的文件.
 * @param[in] str 匹配行的开头, 后跟空格/换行.
 * @return 成功 0, 出错(包括无匹配) -1.
 */
int del_from_file(const char *file, const char *str)
{
	FILE *fpr, *fpw;
	bool deleted = false, empty = true;
	char buf[1024], fnew[HOMELEN];

	if ((fpr = fopen(file, "r")) == NULL)
		return -1;
	if (file_lock_all(fileno(fpr), FILE_RDLCK) != 0) {
		fclose(fpr);
		return -1;
	}
	snprintf(fnew, sizeof(fnew), "%s.%d", file, getpid());
	if ((fpw = fopen(fnew, "w")) == NULL) {
		(void) file_lock_all(fileno(fpr), FILE_UNLCK);
		fclose(fpr);
		return -1;
	}

	size_t len = strlen(str);
	while (fgets(buf, sizeof(buf), fpr) != NULL) {
		if (!deleted) {
			char c = buf[len];
			if ((c == '\0' || c == ' ' || c == '\n')
					&& !strncmp(buf, str, len)) {
				deleted = true;
				continue;
			}
		}
		fputs(buf, fpw);
	}
	empty = (ftell(fpw) <= 0);
	fclose(fpw);
	(void) file_lock_all(fileno(fpr), FILE_UNLCK);
	fclose(fpr);

	if (deleted) {
		if (empty)
			return unlink(file);
		else
			return rename(fnew, file);
	}
	unlink(fnew);
	return -1;
}

/**
 * Mask last section of an IP address.
 * @param host IP address to be masked.
 * @return masked IP address.
 */
const char *mask_host(const char *host)
{
	static char masked[IP_LEN];
	char *end = masked + sizeof(masked);	
	strlcpy(masked, host, sizeof(masked));
	char *last = strrchr(masked, '.'); // IPv4 address.
	if (last != NULL) {
		if (++last < end && *last >= '0' && *last <= '9') {
			*last = '*';
			if (++last < end)
				*last = '\0';
		}
	} else {
		last = strrchr(masked, ':'); // IPv6 address.
		if (last != NULL) {
			if (++last < end)
				*last = '*';
			if (++last < end)
				*last = '\0';
		}
	}
	masked[sizeof(masked) - 1] = '\0';
	return masked;
}

int valid_gbk_file(const char *file, int replace)
{
	mmap_t m = { .oflag = O_RDWR };
	if (mmap_open(file, &m) != 0)
		return -1;

	int count = valid_gbk(m.ptr, m.size, '?');

	mmap_close(&m);
	return count;
}

char *valid_title_gbk(char *title)
{
	char *end = (char *)check_gbk(title);
	*end = '\0';
	valid_gbk((unsigned char *)title, strlen(title), '?');
	return title;
}

void initialize_convert_env(void)
{
	if (!convert_open(CONVERT_U2G) || !convert_open(CONVERT_G2U))
		exit(EXIT_FAILURE);
	atexit(convert_close);
}

void initialize_db(void)
{
	atexit(db_finish);
	if (!db_connect(config_get("host"), config_get("port"),
			config_get("dbname"), config_get("user"),
			config_get("password"))) {
		exit(EXIT_FAILURE);
	}
}

void initialize_mdb(void)
{
	atexit(mdb_disconnect);

	const char *socket = config_get("mdb");
	if (socket && mdb_connect_unix(socket) == 0)
		return;

	const char *host = config_get("mdb_host");
	const char *port = config_get("mdb_port");
	if (host && port && mdb_connect(host, strtol(port, NULL, 10)) == 0)
		return;

	exit(EXIT_FAILURE);
}

void initialize_environment(int flags)
{
	config_load(DEFAULT_CFG_FILE);

	if (flags & INIT_MDB)
		initialize_mdb();
	if (flags & INIT_CONV)
		initialize_convert_env();
	if (flags & INIT_DB)
		initialize_db();
}
