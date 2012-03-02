/*
 *  Fdubbs Project
 *  See COPYRIGHT for more details.
 */
#include <math.h>

#include "libweb.h"
#include "mmap.h"
#include "fbbs/fbbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/status.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

char seccode[SECNUM][6]={
#ifdef FDQUAN
	"0ab","cd","ij","kl","mn","op","qr","st","uv"
#else
	"0oOhH", "1pP", "2qQ", "3rRhH", "4sS", "5tT", "6uU", "7vV", "8wW", "9xX", "ayY", "bzZ"
#endif
};

char secname[SECNUM][2][20] = {
#ifdef FDQUAN
	{"BBS 系统", "[站内]"},
	{"复旦大学", "[本校]"},
	{"光华公司", "[勤助]"},	
	{"社团组织", "[团体]"},
	{"竞赛活动", "[临时]"},
	{"谈天聊地", "[感性]"},
	{"游戏休闲", "[休闲]"},
	{"个人风采", "[个人]"},
	{"积分论坛", "[团体]"}
#else
	{"BBS 系统", "[站内]"},
	{"复旦大学", "[本校]"},
	{"院系风采", "[校园]"},
	{"电脑技术", "[电脑]"},
	{"休闲娱乐", "[休闲]"},
	{"文学艺术", "[文艺]"},
	{"体育健身", "[运动]"},
	{"感性空间", "[感性]"},
	{"新闻信息", "[新闻]"},
	{"学科学术", "[学科]"},
	{"影视音乐", "[影音]"},
	{"交易专区", "[交易]"}
#endif
};

int loginok = 0;
struct userec currentuser;
struct user_info *u_info;

/**
 * Get an environment variable.
 * The function searches environment list where FastCGI stores parameters
 * for a string that matches the string pointed by s. The strings are of 
 * the form name=value.
 * @param s the name
 * @return a pointer to the value in the environment, or empty string if
 *         there is no match
 */
const char *getsenv(const char *s)
{
	char *t = getenv(s);
	if (t!= NULL)
		return t;
	return "";
}

/**
 * Get referrer of the request.
 * @return the path related to the site root, an empty string on error.
 */
const char *get_referer(void)
{
	const char *r = getsenv("HTTP_REFERER");
	int i = 3;
	if (r != NULL) {
		// http://host/path... let's find the third slash
		while (i != 0 && *r != '\0') {
			if (*r == '/')
				i--;
			r++;
		}
	}
	if (i == 0)
		return --r;
	return "";
}

/**
 * Print XML escaped string.

 * @param s string to print
 * @param size maximum output bytes. If 0, print until NUL is encountered.
 * @param stream output stream
 * @note '<' => "&lt;" '&' => "&amp;" '>' => "&gt;" '\\033' => ">1b"
 *       '\\r' => ""
 *       ESC('\\033') is not allowed in XML 1.0. We have to use a workaround 
 *       for compatibility with ANSI escape sequences.
 */
void xml_fputs2(const char *s, size_t size, FILE *stream)
{
	char *c = (char *)s; // To fit FastCGI prototypes..
	char *last = c;
	char *end = c + size;
	char *subst;
	if (size == 0)
		end = NULL;
	while (c != end && *c != '\0') {
		switch (*c) {
			case '<':
				subst = "&lt;";
				break;
			case '>':
				subst = "&gt;";
				break;
			case '&':
				subst = "&amp;";
				break;
			case '\033':
				subst = ">1b";
				break;
			case '\r':
				subst = "";
				break;
			default:
				subst = NULL;
				break;
		}
		if (subst != NULL) {
			fwrite(last, sizeof(char), c - last, stream);
			while (*subst != '\0')
				fputc(*subst++, stream);
			last = ++c;
		} else {
			++c;
		}
	}
	fwrite(last, sizeof(char), c - last, stream);
}

/**
 * Print XML escaped string.
 * @param s string to print (should be NUL terminated)
 * @param stream output stream
 * @see xml_fputs2
 */
void xml_fputs(const char *s, FILE *stream)
{
	xml_fputs2(s, 0, stream);
}

/**
 * Print a file with XML escaped.
 * @param file filename to print
 * @param stream output stream
 * @see xml_fputs2
 */
int xml_printfile(const char *file, FILE *stream)
{
	if (file == NULL || stream == NULL)
		return -1;
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return -1;
	if (m.size > 0)
		xml_fputs2((char *)m.ptr, m.size, stream);
	mmap_close(&m);
	return 0;
}

static char *digest_to_hex(const uchar_t *digest, char *buf, size_t size)
{
	const char *str = "0123456789abcdef";
	for (int i = 0; i < size / 2; ++i) {
		buf[i * 2] = str[(digest[i] & 0xf0) >> 4];
		buf[i * 2 + 1] = str[digest[i] & 0x0f];
	}
	buf[size - 1] = '\0';
	return buf;
}

static char *generate_session_key(char *buf, size_t size, session_id_t sid)
{
	struct {
		time_t sec;
		int usec;
		int random;
		session_id_t sid;
	} s;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	s.sec = tv.tv_sec;
	s.usec = tv.tv_usec;

	s.random = rand();

	s.sid = sid;

	gcry_md_reset(ctx.sha1);
	gcry_md_write(ctx.sha1, &s, sizeof(s));
	gcry_md_final(ctx.sha1);

	const uchar_t *digest = gcry_md_read(ctx.sha1, 0);
	return digest_to_hex(digest, buf, size);
}

/**
 * Load user information from cookie.
 * If everything is OK, initializes @a x and @a y.
 * Otherwise, @a x is cleared and @a y is set to NULL.
 * @param[in,out] x pointer to user infomation struct.
 * @param[in,out] y pointer to pointer to user online cache.
 * @param[in] mode user mode.
 * @return 1 on valid user login, 0 on error.
 */
 // TODO: no lock?
static int user_init(struct userec *x, struct user_info **y, int mode)
{
	memset(x, 0, sizeof(*x));

	// Get information from cookie.
	const char *id = get_param("utmpuserid");
	int i = strtol(get_param("utmpnum"), NULL, 10);
	int key = strtol(get_param("utmpkey"), NULL, 10);

	// Boundary check.
	if (i <= 0 || i > MAXACTIVE) {
		*y = NULL;
		return 0;
	}
	// Get user_info from utmp.
	(*y) = &(utmpshm->uinfo[i - 1]);

	// Verify cookie and user status.
	if (strcmp((*y)->from, fromhost)
			|| (*y)->utmpkey != key
			|| (*y)->active == 0
			|| (*y)->userid[0] == '\0'
			|| !is_web_user((*y)->mode)) {
		*y = NULL;
		return 0;
	}

	// If not normal user.
	if (!strcasecmp((*y)->userid, "new")
			|| !strcasecmp((*y)->userid, "guest")) {
		*y = NULL;
		return 0;
	}

	// Get userec from ucache.
	getuserbyuid(x, (*y)->uid);
	if (strcmp(x->userid, id)) {
		memset(x, 0, sizeof(*x));
		*y = NULL;
		return 0;
	}

	// Refresh idle time, set user mode.
	(*y)->idle_time = time(NULL);
	if (get_web_mode(ST_IDLE) != mode)
		(*y)->mode = mode;

	session.uid = get_user_id(id);

	return 1;
}

/**
 * Initialization inside a FastCGI loop.
 * @param mode user mode.
 * @return 0
 */
// TODO: return value?
int fcgi_init_loop(int mode)
{
	loginok = user_init(&currentuser, &u_info, mode);
	return 0;
}

/**
 * Print HTML response header.
 */
void http_header(void)
{
	printf("Content-type: text/html; charset=%s\n\n", CHARSET);
	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" "
			"\"http://www.w3.org/TR/html4/strict.dtd\"><html><head>");
}

/**
 * HTML code for redirecting/refreshing.
 * @param second seconds before reloading
 * @param url URL to load
 */
void refreshto(int second, const char *url)
{
	printf("<meta http-equiv='Refresh' content='%d; url=%s' />\n", second, url);
}

// Convert exp to icons.
int iconexp(int exp, int *repeat)
{
	int i = 0, j;

	if (exp < 0)
		j = -1;
	else {
		exp = sqrt(exp / 5);
		i = exp / 10;
		i = i > 5 ? 5 : i;
		j = exp - i * 10;
		j = j > 9 ? 9 : j;
	}
	*repeat = ++j;
	return i;
}

int save_user_data(struct userec *x) {
	int n;
	n = searchuser(x->userid) - 1;
	if(n < 0 || n >= MAXUSERS)
		return 0;
	memcpy( &(uidshm->passwd[n]), x, sizeof(struct userec) );
	return 1;
}

int user_perm(struct userec *x, int level) {
	return (level?x->userlevel & level:1);
}

// TODO: put into memory
int maxlen(const char *board)
{
	int max = 0;

	char path[HOMELEN];
	snprintf(path, sizeof(path), BBSHOME"/upload/%s", board);

	if (dashd(path)) {
		max = UPLOAD_MAX;
		snprintf(path, sizeof(path), BBSHOME"/upload/%s/.maxlen", board);

		FILE *fp = fopen(path, "r");
		if (fp) {
			if (fscanf(FCGI_ToFILE(fp), "%d", &max) <= 0)
				max = UPLOAD_MAX;
			fclose(fp);
		}
	}
	return max;
}

// Get file time according to its name 's'.
time_t getfiletime(const struct fileheader *f)
{
	if (f->filename[0] == 's') // sharedmail..
		return strtol(f->filename + strlen(f->owner) + 20, NULL, 10);
	return strtol(f->filename + 2, NULL, 10);
}

struct fileheader *bbsmail_search(const void *ptr, size_t size, const char *file)
{
	if (ptr == NULL || file == NULL)
		return NULL;
	int total = size / sizeof(struct fileheader);
	if (total < 1)
		return NULL;
	// linear search from the end.
	// TODO: unique id should be added to speed up search.
	struct fileheader *begin = (struct fileheader *)ptr;
	struct fileheader *last = begin + total - 1;
	for (struct fileheader *fh = last; fh >= begin; --fh) {
		if (!strcmp(fh->filename, file))
			return fh;
	}
	return NULL;
}

bool valid_mailname(const char *file)
{
	if (!strncmp(file, "sharedmail/", 11)) {
		if (strstr(file + 11, "..") || strchr(file + 11, '/'))
			return false;
	} else {
		if (strncmp(file, "M.", 2) || strstr(file, "..") || strchr(file, '/'))
			return false;
	}
	return true;
}

static char *get_permission(void)
{
	static char c[5];
	c[0] = loginok ? 'l' : ' ';
	c[1] = HAS_PERM(PERM_TALK) ? 't' : ' ';
	c[2] = HAS_PERM(PERM_CLOAK) ? '#': ' ';
	c[3] = HAS_PERM(PERM_OBOARDS) && HAS_PERM(PERM_SPECIAL0) ? 'f' : ' ';
	c[4] = '\0';
	return c;
}

const char *get_doc_mode_str(void)
{
	if (!loginok)
		return "";
	char flag = uidshm->passwd[u_info->uid - 1].flags[1] & 0xf;
	switch (flag) {
		case MODE_THREAD:
			return "t";
		case MODE_FORUM:
			return "f";
	}
	return "";
}

int get_user_flag(void)
{
	return currentuser.flags[1];
}

void set_user_flag(int flag)
{
	if (loginok)
		uidshm->passwd[u_info->uid - 1].flags[1] = flag;
}

void print_session(void)
{
	if (ctx.r->flag & REQUEST_API)
		return;
	bool mobile = ctx.r->flag & REQUEST_MOBILE;

	printf("<session m='%s'><p>%s</p><u>%s</u><f>", get_doc_mode_str(),
			get_permission(), currentuser.userid);

	db_res_t *res = db_query("SELECT b.id, b.name FROM boards b"
			" JOIN fav_boards f ON b.id = f.board WHERE f.user_id = %d",
			session.uid);
	if (res) {
		for (int i = 0; i < db_res_rows(res); ++i) {
			int bid = db_get_integer(res, i, 0);
			const char *name = db_get_value(res, i, 1);
			printf("<b bid='%d'", bid);
			if (mobile && !brc_board_unread(currentuser.userid, name, bid))
				printf(" r='1'");

			if (*name & 0x80) {
				GBK_BUFFER(name, BOARD_NAME_LEN / 2);
				convert_u2g(name, gbk_name);
				printf(">%s</b>", gbk_name);
			} else {
				printf(">%s</b>", name);
			}
		}
	}
	db_clear(res);

	printf("</f></session>");
}


