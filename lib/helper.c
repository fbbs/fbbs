#include "bbs.h"
#include "mmap.h"
#include "fbbs/cfg.h"
#include "fbbs/convert.h"
#include "fbbs/fbbs.h"
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
		fb_flock(fileno(fpr), LOCK_EX);
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
		fb_flock(fileno(fpr), LOCK_UN);
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
	fb_flock(fileno(fpr), LOCK_EX);
	snprintf(fnew, sizeof(fnew), "%s.%d", file, getpid());
	if ((fpw = fopen(fnew, "w")) == NULL) {
		fb_flock(fileno(fpr), LOCK_UN);
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
	fb_flock(fileno(fpr), LOCK_UN);
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

/**
 * Attach a signature.
 * @param fp Output file.
 * @param user The user.
 * @param sig number of signature, 1-based.
 */
void add_signature(FILE *fp, const char *user, int sig)
{
	fputs("\n--\n", fp);
	if (sig <= 0)
		return;

	char file[HOMELEN];
	sethomefile(file, user, "signatures");
	FILE *fin = fopen(file, "r");
	if (!fin)
		return;

	char buf[256];
	for (int i = 0; i < (sig - 1) * MAXSIGLINES; ++i) {
		if (!fgets(buf, sizeof(buf), fin)) {
			fclose(fin);
			return;
		}
	}

	int blank = 0;
	for (int i = 0; i < MAXSIGLINES; i++) {
		if (!fgets(buf, sizeof(buf), fin))
			break;
		if (buf[0] == '\n' || streq(buf, "\r\n")) {
			++blank;
		} else {
			while (blank-- > 0)
				fputs("\n", fp);
			blank = 0;
			if (!strstr(buf, ":·"BBSNAME" "BBSHOST"·[FROM:"))
				fputs(buf, fp);
		}
	}
	fclose(fin);
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

char *valid_title(char *title)
{
	char *end = (char *)check_gbk(title);
	*end = '\0';
	valid_gbk((unsigned char *)title, strlen(title), '?');
	return title;
}

void initialize_convert_env(void)
{
	env.u2g = pool_alloc(env.p, sizeof(*env.u2g));
	env.g2u = pool_alloc(env.p, sizeof(*env.g2u));
	if (convert_open(env.u2g, "GBK", "UTF-8") < 0
			|| convert_open(env.g2u, "UTF-8", "GBK") < 0)
		exit(EXIT_FAILURE);
}

void initialize_db(void)
{
	atexit(db_finish);
	if (!db_connect(config_get(env.c, "host"), config_get(env.c, "port"),
			config_get(env.c, "dbname"), config_get(env.c, "user"),
			config_get(env.c, "password"))) {
		exit(EXIT_FAILURE);
	}
}

static void mdb_disconnect(void)
{
	mdb_finish(env.m->c);
}

void initialize_mdb(void)
{
	atexit(mdb_disconnect);
	env.m = pool_alloc(env.p, sizeof(*env.m));
	if (mdb_connect_unix(config_get(env.c, "mdb")) < 0)
		exit(EXIT_FAILURE);
}

void initialize_environment(int flags)
{
	env.p = pool_create(0);
	env.c = config_load(env.p, DEFAULT_CFG_FILE);

	if (flags & INIT_CONV)
		initialize_convert_env();
	if (flags & INIT_DB)
		initialize_db();
	if (flags & INIT_MDB)
		initialize_mdb();
}
