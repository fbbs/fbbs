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
char *getsenv(const char *s)
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
	if (size > MAX_CONTENT_LENGTH)
		size = MAX_CONTENT_LENGTH;
	if (size <= 0)
		return BBS_EINVAL;
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

void xml_fputs(const char *s, FILE *stream)
{
	xml_fputs2(s, 0, stream);
}

void xml_fputs2(const char *s, size_t size, FILE *stream)
{
	// To fit FastCGI prototypes..
	char *c = (char *)s;
	char *last = c;
	char *end = c + size;
	if (size == 0)
		end = NULL;
	while (c != end && *c != '\0') {
		switch (*c) {
			case '<':
				fwrite(last, sizeof(char), c - last, stream);
				fwrite("&lt;", sizeof(char), 4, stream);
				last = ++c;
				break;
			case '>':
				fwrite(last, sizeof(char), c - last, stream);
				fwrite("&gt;", sizeof(char), 4, stream);
				last = ++c;
				break;
			case '&':
				fwrite(last, sizeof(char), c - last, stream);
				fwrite("&amp;", sizeof(char), 5, stream);
				last = ++c;
				break;
			case '\033':
				fwrite(last, sizeof(char), c - last, stream);
				fwrite(">1b", sizeof(char), 3, stream);
				last = ++c;
				break;
			default:
				++c;
				break;
		}
	}
	fwrite(last, sizeof(char), c - last, stream);
}

int xml_printfile(const char *file, FILE *stream)
{
	if (file == NULL || stream == NULL)
		return -1;
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(file, &m) < 0)
		return -1;
	xml_fputs((char *)m.ptr, stream);
	mmap_close(&m);
	return 0;
}


static int http_init(void)
{
	int my_style;

	param_init();

#ifdef SQUID
	char *fromtmp;
	fromtmp = strrchr(getsenv("HTTP_X_FORWARDED_FOR"), ',');
	if (fromtmp == NULL) {
		strlcpy(fromhost, getsenv("HTTP_X_FORWARDED_FOR"), sizeof(fromhost));
	} else {
		while ((*fromtmp < '0')&&(*fromtmp != '\0'))
			fromtmp++;
		strlcpy(fromhost, fromtmp, sizeof(fromhost));
	}
#else
	strlcpy(fromhost, getsenv("REMOTE_ADDR"), sizeof(fromhost));
#endif

	return my_style;
}

// Gets shared memory. Returns 0 if OK, exits on error.
int shm_init(void)
{
	if (resolve_ucache() == -1)
		exit(1);
	resolve_utmp();
	if (resolve_boards() < 0)
		exit(1);
	if (utmpshm == NULL || brdshm == NULL)
		exit(1);
	return 0;
}

// Load user information from cookie.
// If everything is OK, initializes *'x', 'y' and returns 1,
// on error, set *'x' to 0, 'y' to NULL and returns 0.
int user_init(struct userec *x, struct user_info **y)
{
	char id[IDLEN + 1];
	int i, key;

	memset(x, 0, sizeof(*x));
	// Get information from cookie.
	strlcpy(id, getparm("utmpuserid"), sizeof(id));
	i = strtol(getparm("utmpnum"), NULL, 10);
	key = strtol(getparm("utmpkey"), NULL, 10);

	// Boundary check.
	if (i <= 0 || i > MAXACTIVE) {
		return 0;
	}
	// Get user_info from utmp.
	(*y) = &(utmpshm->uinfo[i - 1]);

	// Verify cookie and user status.
	if (strncmp((*y)->from, fromhost, 16)
			|| (*y)->utmpkey != key
			|| (*y)->active == 0
			|| (*y)->userid[0] == '\0'
			|| (*y)->mode != WWW) {
		*y = NULL;
		return 0;
	}

	// If not normal user.
	if (!strcasecmp((*y)->userid, "new")
			|| !strcasecmp((*y)->userid, "guest")) {
		*y = NULL;
		return 0;
	}

	// Refresh idle time.
	(*y)->idle_time = time(NULL);

	// Get userec from ucache.
	getuserbyuid(x, (*y)->uid);
	if (strcmp(x->userid, id)) {
		memset(x, 0, sizeof(*x));
		return 0;
	}

	return 1;
}

void xml_header(const char *xslfile)
{
	printf("Content-type: application/xml; charset=%s\n\n", CHARSET);
	printf("<?xml version=\"1.0\" encoding=\"%s\"?>\n", CHARSET);
	printf("<?xml-stylesheet type=\"text/xsl\" href=\"/xsl/%s.xsl\"?>\n", xslfile);
}

void http_header(void)
{
	printf("Content-type: text/html; charset=%s\n\n", CHARSET);
	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" "
			"\"http://www.w3.org/TR/html4/strict.dtd\"><html><head>");
}

void refreshto(int second, const char *url)
{
	printf("<meta http-equiv='Refresh' content='%d; url=%s' />\n", second, url);
}

void setcookie(const char *a, const char *b)
{
	printf("<script>document.cookie='%s=%s'</script>\n", a, b);
}

// similar to 'date_to_fname()'.
// Creates a new file in 'dir' with prefix 'pfx'.
// Returns filename(in 'fname') and stream on success, NULL on error.
static FILE *get_fname(const char *dir, const char *pfx, char *fname, size_t size)
{
	if (dir == NULL || pfx == NULL)
		return NULL;
	const char c[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA";
	int t = (int)time(NULL);
	int count = snprintf(fname, size, "%s%s%d. ", dir, pfx, (int)time(NULL));
	if (count < 0 || count >= size)
		return NULL;
	int fd;
	for (int i = sizeof(c) - 2; i >= 0; ++i) {
		fname[count - 1] = c[i];
		if ((fd = open(fname, O_CREAT | O_WRONLY | O_EXCL, 0644)) > 0)
			return fdopen(fd, "w");
	}
	return NULL;
}

// Post an article with 'title', 'content' on board 'bp' by 'user' from 'ip'
// as a reply to 'o_fp'. If 'o_fp' == NULL then it starts a new thread.
// Returns 0 on success, -1 on error.
int post_article(const struct userec *user, const struct boardheader *bp,
		const char *title, const char *content, 
		const char *ip, const struct fileheader *o_fp)
{
	if (user == NULL || bp == NULL || title == NULL 
			|| content == NULL || ip == NULL)
		return -1;

	char fname[HOMELEN];
	char dir[HOMELEN];
	int idx = snprintf(dir, sizeof(dir), "boards/%s/", bp->filename);
	const char *pfx = "M.";
	FILE *fptr;
	if ((fptr = get_fname(dir, pfx, fname, sizeof(fname))) == NULL)
		return -1;
	fprintf(fptr, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			user->userid, user->username, bp->filename, title, BBSNAME,
			getdatestring(time(NULL), DATE_ZH));
	fputs(content, fptr);
	fprintf(fptr, "\n--\n");
	// TODO: signature
	fprintf(fptr, "\033[m\033[1;%2dm※ 来源:·"BBSNAME" "BBSHOST
			"·HTTP [FROM: %-.20s]\033[m\n", 31 + rand() % 7, ip);
	fclose(fptr);

	struct fileheader fh;
	memset(&fh, 0, sizeof(fh));	
	strlcpy(fh.filename, fname + idx, sizeof(fh.filename));
	strlcpy(fh.owner, user->userid, sizeof(fh.owner));
	strlcpy(fh.title, title, sizeof(fh.title));
	fh.id = get_nextid2(bp);
	if (o_fp != NULL) { //reply
		fh.reid = o_fp->id;
		fh.gid = o_fp->gid;
	} else {
		fh.reid = fh.id;
		fh.gid = fh.id;
	}
	setwbdir(dir, bp->filename);
	append_record(dir, &fh, sizeof(fh));
	updatelastpost(bp->filename);
	return 0;
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

int fcgi_init_all(void)
{
	srand(time(NULL) * 2 + getpid());
	chdir(BBSHOME);
	seteuid(BBSUID);
	if(geteuid() != BBSUID)
		return BBS_EINTNL;
	shm_init();

	return 0;
}

int fcgi_init_loop(void)
{
	int my_style = http_init();
	loginok = user_init(&currentuser, &u_info);

	return my_style;
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

int set_my_cookie(void)
{
	FILE *fp;
	char path[256], buf[256], buf1[256], buf2[256];
	int my_t_lines=20, my_link_mode=0, my_def_mode=0, my_style=0;
	sprintf(path, "home/%c/%s/.mywww", toupper(currentuser.userid[0]), currentuser.userid);
	fp = fopen(path, "r");
	if(fp != NULL) {
		while(1) {
			if(fgets(buf, 80, fp)==0)
				break;
			if(sscanf(buf, "%80s %80s", buf1, buf2)!=2)
				continue;
			if(!strcmp(buf1, "t_lines"))
				my_t_lines=atoi(buf2);
			if(!strcmp(buf1, "link_mode"))
				my_link_mode=atoi(buf2);
			if(!strcmp(buf1, "def_mode"))
				my_def_mode=atoi(buf2);
			if(!strcmp(buf1, "mystyle"))
				my_style=atoi(buf2);
		}
		fclose(fp);
		printf("<my_t_lines>%d</my_t_lines>\n<my_link_mode>%d</my_link_mode>\n"
				"<my_def_mode>%d</my_def_mode>\n<my_style>%d</my_style>\n",
				my_t_lines, my_link_mode, my_def_mode, my_style);
	}
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

bool bbscon_search(const struct boardheader *bp, unsigned int fid,
		int action, struct fileheader *fp)
{
	if (bp == NULL || fp == NULL)
		return false;
	char dir[HOMELEN];
	setbfile(dir, bp->filename, DOT_DIR);
	mmap_t m;
	m.oflag = O_RDONLY;
	if (mmap_open(dir, &m) < 0)
		return false;
	struct fileheader *begin = m.ptr, *end;
	end = begin + (m.size / sizeof(*begin));
	const struct fileheader *f = dir_bsearch(begin, end, fid);
	if (f != end && f->id == fid) {
		unsigned int gid = f->gid;
		switch (action) {
			case 'p':  // previous post
				--f;
				break;
			case 'n':  // next post
				++f;
				break;
			case 'b':  // previous post of same thread
				while (--f >= begin && f->gid != gid)
					; // null statement
				break;
			case 'a':  // next post of same thread
				while (++f < end && f->gid != gid)
					; // null statement
				break;
			default:
				break;
		}
		if (f >= begin && f < end)
			*fp = *f;
		else
			f = NULL;
	}
	mmap_close(&m);
	return (f != NULL);
}

// TODO: put into memory
int maxlen(const char *board)
{
	char path[HOMELEN];
	int	limit = UPLOAD_MAX;
	snprintf(path, sizeof(path), BBSHOME"/upload/%s/.maxlen", board);
	FILE *fp = fopen(path, "r");
	if (fp != NULL) {
		fscanf(fp, "%d", &limit);
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

char *get_permission(void)
{
	static char c[5];
	c[0] = loginok ? 'l' : ' ';
	c[1] = HAS_PERM(PERM_TALK) ? 't' : ' ';
	c[2] = HAS_PERM(PERM_CLOAK) ? '#': ' ';
	c[3] = HAS_PERM(PERM_OBOARDS) && HAS_PERM(PERM_SPECIAL0) ? 'f' : ' ';
	c[4] = '\0';
	return c;
}

const char *get_referer(void)
{
	char *r = getsenv("HTTP_REFERER");
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
