/*
 *  Fdubbs Project
 *  See COPYRIGHT for more details.
 */

#include "libweb.h"

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
char fromhost[40]; // IPv6 addresses can be represented in 39 chars.
char param_name[MAX_PARAMS][PARAM_NAMELEN]; ///< Parameter names.
char *param_val[MAX_PARAMS]; ///< Parameter values.
int param_num = 0;  ///< Count of parsed parameters.

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
 * Get decimal value of a hex char 'c'
 * @param c hex char
 * @return converted decimal value, 0 on error
 */
static int hex2dec(int c)
{
	c = toupper(c);
	switch (c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'B': return 11;
		case 'C': return 12;
		case 'D': return 13;
		case 'E': return 14;
		case 'F': return 15;
		default:  return 0;
	}
}

/**
 * Decode an url string.
 * @param s string to decode.
 * @return 0 on success, -1 if an error occurs.
 */
static int url_decode(char *s)
{
	int m, n;
	for(m = 0, n = 0; s[m] != 0; m++, n++) {
		if (s[m] == '+') {
			s[n] = ' ';
			continue;
		}
		if (s[m] == '%') {
			if (s[m + 1] != '\0' && s[m + 2] != '\0') {
				s[n] = hex2dec(s[m + 1]) * 16 + hex2dec(s[m + 2]);
				m += 2;
				continue;
			} else {
				s[n] = '\0';
				return -1;
			}
		}
		s[n] = s[m];
	}
	s[n] = '\0';
	return 0;
}

/**
 * Add a name-value pair to global buffer.
 * @param name name of the pair
 * @param val value of the pair
 * @return 0 on success, -1 on error.
 */
static int param_add(const char *name, const char *val)
{
	if (param_num >= sizeof(param_val) - 1)
		return -1;
	size_t len = strlen(val);
	param_val[param_num] = malloc(len + 1);
	if (param_val[param_num] == NULL)
		return -1;
	strlcpy(param_name[param_num], name, sizeof(param_name[0]));
	memcpy(param_val[param_num], val, len + 1);
	++param_num;
	return 0;
}

/**
 * Free all memory for parameters.
 */
static void param_free(void)
{
	for (int i = param_num - 1; i >= 0; --i) {
		free(param_val[i]);
	}
	param_num = 0;
}

/**
 * Parse parameters.
 * The function splits buf into key-value pairs and stores them 
 * in a global buffer.
 * @param buf the string to parse
 * @param delim delimiter to split 'buf'
 * @return 0 on success, -1 on error.
 */
static int param_parse(char *buf, const char *delim)
{
	char *t2, *t3;
	t2 = strtok(buf, delim);
	while (t2 != NULL) {
		t3 = strchr(t2, '=');
		if (t3 != NULL) {
			*t3++ = '\0';
			url_decode(t3);
			if (param_add(trim(t2), t3) < 0)
				return -1;
		}
		t2 = strtok(NULL, delim);
	}
	return 0;
}

/**
 * Read parameters from HTTP header.
 * The function parses both query string and cookie in HTTP header and stores
 * the result in a global buffer.
 * @see parse_post_data
 */
static void param_init(void)
{
	char buf[1024];
	param_free();
	// Do not parse contents via 'POST' method
	strlcpy(buf, getsenv("QUERY_STRING"), sizeof(buf));
	param_parse(buf, "&");
	strlcpy(buf, getsenv("HTTP_COOKIE"), sizeof(buf));
	param_parse(buf, ";");
}

/**
 * Parse parameters submitted by POST method.
 * @return 0 on success, bbserrno on error.
 */
int parse_post_data(void)
{
	char *buf;
	unsigned long size = strtoul(getsenv("CONTENT_LENGTH"), NULL, 10);
	if (size == 0)
		return 0;
	else if (size < 0)
		return BBS_EINVAL;
	else if (size > MAX_CONTENT_LENGTH)
		size = MAX_CONTENT_LENGTH;
	buf = malloc(size + 1);
	if(buf == NULL)
		return BBS_EINTNL;
	if (fread(buf, 1, size, stdin) != size) {
		free(buf);
		return BBS_EINTNL;
	}
	buf[size] = '\0';
	param_parse(buf, "&");
	free(buf);
	return 0;
}

/**
 * Get a parameter value.
 * @param name the name of the parameter
 * @return the value corresponding to 'name', an empty string if not found.
 */
char *getparm(const char *name)
{
	for(int n = 0; n < param_num; n++) 
		if(!strcasecmp(param_name[n], name))
			return param_val[n];
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

/**
 * Parse HTTP header and get client IP address.
 * @return 0
 */
static int http_init(void)
{
	param_init();
#ifdef SQUID
	char *from;
	from = strrchr(getsenv("HTTP_X_FORWARDED_FOR"), ',');
	if (from == NULL) {
		strlcpy(fromhost, getsenv("HTTP_X_FORWARDED_FOR"), sizeof(fromhost));
	} else {
		while ((*from < '0') && (*from != '\0'))
			from++;
		strlcpy(fromhost, from, sizeof(fromhost));
	}
#else
	strlcpy(fromhost, getsenv("REMOTE_ADDR"), sizeof(fromhost));
#endif
	return 0;
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
	char *id = getparm("utmpuserid");
	int i = strtol(getparm("utmpnum"), NULL, 10);
	int key = strtol(getparm("utmpkey"), NULL, 10);

	// Boundary check.
	if (i <= 0 || i > MAXACTIVE) {
		*y = NULL;
		return 0;
	}
	// Get user_info from utmp.
	(*y) = &(utmpshm->uinfo[i - 1]);

	// Verify cookie and user status.
	// TODO: magic number here. Lack IPv6 support.
	if (strncmp((*y)->from, fromhost, 16)
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
	(*y)->mode = mode;

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
	http_init();
	loginok = user_init(&currentuser, &u_info, mode);
	return 0;
}

/**
 * Print XML response header.
 * @param xslfile name of the XSLT file to use.
 */
void xml_header(const char *xslfile)
{
	printf("Content-type: application/xml; charset=%s\n\n", CHARSET);
	printf("<?xml version=\"1.0\" encoding=\"%s\"?>\n", CHARSET);
	printf("<?xml-stylesheet type=\"text/xsl\" href=\"../xsl/%s.xsl\"?>\n", xslfile);
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
	int i, j;

	if (exp < 0)
		j = -1;
	else {
		i = exp / 2000;
		i = i > 5 ? 5 : i;
		j = (exp - i * 2000) / 200;
		j = j > 9 ? 9 : j;
	}
	*repeat = ++j;
	return i;
}

int save_user_data(struct userec *x) {
	FILE *fp;
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

// TODO: better rewrite
char *getbfroma(const char *path)
{
	FILE *fp;
	static char buf1[256], buf2[256];
	memset(buf1, '\0', sizeof(buf1));
	memset(buf2, '\0', sizeof(buf2));
	int len;
	if (path == NULL || *path == '\0')
		return "";
	++path;
	fp = fopen("0Announce/.Search", "r");
	if (fp == NULL)
		return "";
	while (true) {
		if(fscanf(FCGI_ToFILE(fp), "%s %s", buf1, buf2) <= 0)
			break;
		if (*buf1 != '\0')
			buf1[strlen(buf1) - 1] = '\0';
		if (*buf1 == '*')
			continue;
		if(!strncmp(buf2, path, strlen(buf2)))
			return buf1;
	}
	fclose(fp);
	return "";
}

// Find post whose id = 'fid'.
// If 'fid' > any post's id, return 'end',
// otherwise, return the minimum one among all post whose id > 'fid'.
struct fileheader *dir_bsearch(const struct fileheader *begin, 
		const struct fileheader *end, unsigned int fid)
{
	const struct fileheader *mid;
	while (begin < end) {
		mid = begin + (end - begin) / 2;
		if (mid->id == fid) {
			return mid;
		}
		if (mid->id < fid) {
			begin = mid + 1;
		} else {
			end = mid;
		}
	}
	return begin;
}

// TODO: put into memory
int maxlen(const char *board)
{
	char path[HOMELEN];
	int	limit = UPLOAD_MAX;
	snprintf(path, sizeof(path), BBSHOME"/upload/%s/.maxlen", board);
	FILE *fp = fopen(path, "r");
	if (fp != NULL) {
		fscanf(FCGI_ToFILE(fp), "%d", &limit);
		fclose(fp);
	}
	return limit;
}

// Get file time according to its name 's'.
time_t getfiletime(const struct fileheader *f)
{
	return (time_t)strtol(f->filename + 2, NULL, 10);
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

void print_session(void)
{
	printf("s='%s;%s;", get_permission(), currentuser.userid);
	char file[HOMELEN];
	sethomefile(file, currentuser.userid, ".goodbrd");
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) == 0) {
		struct goodbrdheader *iter, *end;
		int num = m.size / sizeof(struct goodbrdheader);
		if (num > GOOD_BRC_NUM)
			num = GOOD_BRC_NUM;
		end = (struct goodbrdheader *)m.ptr + num;
		for (iter = m.ptr; iter != end; ++iter) {
			if (!gbrd_is_custom_dir(iter)) {
				struct boardheader *bp = bcache + iter->pos;
				printf("%s ", bp->filename);
			}
		}
	}
	printf("'");
}
