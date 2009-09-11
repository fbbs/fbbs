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

struct stat *f_stat(char *file) {
	static struct stat buf;
	bzero(&buf, sizeof(buf));
	if(stat(file, &buf)==-1) bzero(&buf, sizeof(buf));
	return &buf;
}

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

int http_fatal(const char *prompt)
{
	printf("Content-type: text/html; charset=%s\n\n", CHARSET);
	printf("<html><head><title>发生错误</title></head><body><div>%s</div>"
			"<a href=javascript:history.go(-1)>快速返回</a></body></html>",
			prompt);
	FCGI_Finish();
	return 0;
}

int http_fatal2(enum HTTP_STATUS status, const char *prompt)
{
	printf("Content-type: text/html; charset=%s\nStatus: %d\n\n",
			CHARSET, status);
	printf("<html><head><title>发生错误</title></head><body><div>%s</div>"
			"<a href=javascript:history.go(-1)>快速返回</a></body></html>",
			prompt);
	FCGI_Finish();
	return 0;
}

void xml_fputs(const char *s, FILE *stream)
{
	const char *last = s;
	while (*s != '\0') {
		switch (*s) {
			case '<':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite("&lt;", sizeof(char), 4, stream);
				last = ++s;
				break;
			case '>':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite("&gt;", sizeof(char), 4, stream);
				last = ++s;
				break;
			case '&':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite("&amp;", sizeof(char), 5, stream);
				last = ++s;
				break;
			case '\033':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite(">1b", sizeof(char), 3, stream);
				last = ++s;
				break;
			default:
				++s;
				break;
		}
	}
	fwrite(last, sizeof(char), s - last, stream);
}

void xml_fputs2(const char *s, size_t size, FILE *stream)
{
	const char *last = s;
	const char *end = s + size;
	while (s != end) {
		switch (*s) {
			case '<':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite("&lt;", sizeof(char), 4, stream);
				last = ++s;
				break;
			case '>':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite("&gt;", sizeof(char), 4, stream);
				last = ++s;
				break;
			case '&':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite("&amp;", sizeof(char), 5, stream);
				last = ++s;
				break;
			case '\033':
				fwrite(last, sizeof(char), s - last, stream);
				fwrite(">1b", sizeof(char), 3, stream);
				last = ++s;
				break;
			default:
				++s;
				break;
		}
	}
	fwrite(last, sizeof(char), s - last, stream);
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

// Convert a hex char 'c' to a base 10 integer.
static int __to16(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0;
}

static int __unhcode(char *s)
{
	int m, n;
	for(m = 0, n = 0; s[m] != 0; m++, n++) {
		if (s[m] == '+') {
			s[n] = ' ';
			continue;
		}
		if (s[m] == '%') {
			s[n] = __to16(s[m+1]) * 16 +__to16(s[m+2]);
			m += 2;
			continue;
		}
		s[n]=s[m];
	}
	s[n] = 0;
	return 0;
}

char parm_name[256][80], *parm_val[256];
int parm_num = 0;

// Adds 'name' 'val' pairs to global arrays 'parm_name' and 'parm_val'.
// Increases 'parm_num' by 1.
static int parm_add(const char *name, const char *val)
{
	if (parm_num >= sizeof(parm_val) - 1)
		http_fatal2(HTTP_STATUS_BADREQUEST, "too many parms.");
	size_t len = strlen(val);
	parm_val[parm_num] = malloc(len + 1);
	if (parm_val[parm_num] == NULL)
		http_fatal2(HTTP_STATUS_SERVICE_UNAVAILABLE, "memory overflow2");
	strlcpy(parm_name[parm_num], name, sizeof(parm_name[0]));
	memcpy(parm_val[parm_num], val, len);
	parm_val[parm_num][len] = '\0';
	return ++parm_num;
}

// Frees 'parm_val'.
static int parm_free(void)
{
	int i;
	for (i = parm_num - 1; i >= 0; --i) {
		free(parm_val[i]);
	}
	return parm_num = 0;
}

// Searches 'parm_name' for 'name'.
// Returns corresponding 'parm_val' if found, otherwise "".
char *getparm(const char *name)
{
	int n;
	for(n = 0; n < parm_num; n++) 
		if(!strcasecmp(parm_name[n], name))
			return parm_val[n];
	return "";
}

// Uses delimeter 'delim' to split 'buf' into "key=value" pairs.
// Adds these pairs into global arrays.
static void parm_parse(char *buf, const char *delim)
{
	char *t2, *t3;
	t2 = strtok(buf, delim);
	while (t2 != NULL) {
		t3 = strchr(t2, '=');
		if (t3 != NULL) {
			*t3++ = '\0';
			__unhcode(t3);
			parm_add(trim(t2), t3);
		}
		t2 = strtok(NULL, delim);
	}
	return;
}

// Read parameters from HTTP header.
void http_parm_init(void)
{
	char *buf[1024];
	parm_free();
	// Do not parse contents via 'POST' method
	strlcpy(buf, getsenv("QUERY_STRING"), sizeof(buf));
	parm_parse(buf, "&");
	strlcpy(buf, getsenv("HTTP_COOKIE"), sizeof(buf));
	parm_parse(buf, ";");
}

void parse_post_data(void)
{
	char *buf;
	unsigned long size = strtoul(getsenv("CONTENT_LENGTH"), NULL, 10);
	if (size > POST_LENGTH_LIMIT)
		size = POST_LENGTH_LIMIT;
	if (size <= 0)
		return;
	buf = malloc(size + 1);
	if(buf == NULL)
		http_fatal2(HTTP_STATUS_SERVICE_UNAVAILABLE, "memory overflow");
	if (fread(buf, 1, size, stdin) != size) {
		free(buf);
		http_fatal2(HTTP_STATUS_BADREQUEST, "HTTP请求格式错误");
	}
	buf[size] = '\0';
	parm_parse(buf, "&");
	free(buf);
}

static int http_init(void)
{
	int my_style;

	http_parm_init();

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
		http_fatal2(HTTP_STATUS_INTERNAL_ERROR, "uid error.");
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
