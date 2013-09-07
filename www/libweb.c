/*
 *  Fdubbs Project
 *  See COPYRIGHT for more details.
 */
#include <math.h>

#include "libweb.h"
#include "mmap.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/user.h"
#include "fbbs/web.h"

struct userec currentuser;

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
 * @note '<' => "&lt;" '&' => "&amp;" '>' => "&gt;" '\\033' => ">1b"
 *       '\\r' => ""
 *       ESC('\\033') is not allowed in XML 1.0. We have to use a workaround 
 *       for compatibility with ANSI escape sequences.
 */
void xml_fputs2(const char *s, size_t size)
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
			fwrite(last, sizeof(char), c - last, stdout);
			while (*subst != '\0')
				fputc(*subst++, stdout);
			last = ++c;
		} else {
			++c;
		}
	}
	fwrite(last, sizeof(char), c - last, stdout);
}

/**
 * Print XML escaped string.
 * @param s string to print (should be NUL terminated)
 * @see xml_fputs2
 */
void xml_fputs(const char *s)
{
	xml_fputs2(s, 0);
}

size_t xml_fputs3(const char *s, size_t size, FILE *stream)
{
	xml_fputs2(s, size);
	return 0;
}

/**
 * Print a file with XML escaped.
 * @param file filename to print
 * @see xml_fputs2
 */
int xml_printfile(const char *file)
{
	if (!file)
		return -1;

	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) < 0)
		return -1;
	if (m.size > 0)
		xml_fputs2((char *)m.ptr, m.size);
	mmap_close(&m);
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

int save_user_data(struct userec *x) {
	int n;
	n = searchuser(x->userid) - 1;
	if(n < 0 || n >= MAXUSERS)
		return 0;
	memcpy( &(uidshm->passwd[n]), x, sizeof(struct userec) );
	return 1;
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
	c[0] = session.id ? 'l' : ' ';
	c[1] = HAS_PERM(PERM_TALK) ? 't' : ' ';
	c[2] = HAS_PERM(PERM_CLOAK) ? '#': ' ';
	c[3] = HAS_PERM(PERM_OBOARDS) && HAS_PERM(PERM_SPECIAL0) ? 'f' : ' ';
	c[4] = '\0';
	return c;
}

int get_user_flag(void)
{
	return currentuser.flags[1];
}

void set_user_flag(int flag)
{
	if (session.id) {
		int n = searchuser(currentuser.userid) - 1;
		if (n < 0 || n >= MAXUSERS)
			return;
		uidshm->passwd[n].flags[1] = flag;
	}
}

extern const char *get_post_list_type_string(void);

void print_session(void)
{
	if (request_type(REQUEST_API))
		return;
	bool mobile = request_type(REQUEST_MOBILE);

	printf("<session m='%s'><p>%s</p><u>%s</u><f>",
			get_post_list_type_string(), get_permission(), currentuser.userid);

	query_t *q = query_new(0);
	query_select(q, "board, name");
	query_from(q, "fav_boards");
	query_where(q, "user_id = %"DBIdUID, session.uid);
	db_res_t *res = query_exec(q);

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


