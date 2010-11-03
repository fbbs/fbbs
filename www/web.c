#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/web.h"

/**
 * Get an environment variable.
 * The function searches environment list where FastCGI stores parameters
 * for a string that matches the string pointed by 'key'. The strings are
 * of the form key=value.
 * @param key The key.
 * @return a pointer to the value in the environment, or empty string if
 * there is no match
 */
static const char *_get_server_env(const char *key)
{
	char *s = getenv(key);
	if (s)
		return s;
	return "";
}

/**
 * Get decimal value of a hex char 'c'
 * @param c The hexadecimal character.
 * @return Converted decimal value, 0 on error.
 */
static int _hex2dec(int c)
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
 * @param s The string to be decoded.
 * @return The converted string.
 */
static char *_url_decode(char *s)
{
	int m, n;
	for (m = 0, n = 0; s[m]; ++m, ++n) {
		if (s[m] == '+') {
			s[n] = ' ';
		} else if (s[m] == '%') {
			if (s[m + 1] && s[m + 2]) {
				s[n] = _hex2dec(s[m + 1]) * 16 + _hex2dec(s[m + 2]);
				m += 2;
				continue;
			} else {
				s[n] = '\0';
				return s;
			}
		} else {
			s[n] = s[m];
		}
	}
	s[n] = '\0';
	return s;
}

static int _parse_param(http_req_t *r, const char *begin, size_t len)
{
	if (len == 0)
		return 0;

	char *s = pool_alloc(r->p, len + 1);
	if (!s)
		return -1;

	strlcpy(s, begin, len + 1);
	s[len] = '\0';

	char *key = s, *val = strchr(s, '=');
	if (val) {
		*val++ = '\0';
		_url_decode(val);
	} else {
		val = "";
	}

	r->params[r->count].key = key;
	r->params[r->count].val = val;
	++r->count;
	return 0;
}

static int _parse_params(http_req_t *r, const char *key, int delim)
{
	const char *env = _get_server_env(key);
	const char *ptr = strchr(env, delim), *last = env;
	while (ptr) {
		if (_parse_param(r, last, ptr - last) < 0)
			return -1;
		last = ptr + 1;
		ptr = strchr(last, delim);
	}
	if (_parse_param(r, last, strlen(last)) < 0)
		return -1;
	return 0;
}

/**
 * Parse GET parameters and cookies.
 * @param r The http request.
 * @return 0 on success, -1 on error.
 */
static int _parse_http_req(http_req_t *r)
{
	if (_parse_params(r, "QUERY_STRING", '&') < 0)
		return -1;
	if (_parse_params(r, "HTTP_COOKIE", ';') < 0)
		return -1;
	return 0;
}

/**
 * Get an http request.
 * The GET request and cookies are parsed into key=value pairs.
 * @param p A memory pool to use.
 */
http_req_t *get_request(pool_t *p)
{
	http_req_t *r = pool_alloc(p, sizeof(*r));
	if (!r)
		return NULL;

	r->p = p;
	r->count = 0;
	if (_parse_http_req(r) < 0)
		return NULL;
	return r;
}

/**
 * Get a parameter value.
 * @param r The http request.
 * @param key The name of the parameter.
 * @return the value corresponding to 'key', an empty string if not found.
 */
const char *get_param(http_req_t *r, const char *key)
{
	for (int i = 0; i < r->count; ++i) {
		if (streq(r->params[i].key, key))
			return r->params[i].val;
	}
	return "";
}

/**
 * Parse parameters submitted by POST method.
 * @return 0 on success, -1 on error.
 */
int parse_post_data(http_req_t *r)
{
	unsigned long size = strtoul(_get_server_env("CONTENT_LENGTH"), NULL, 10);
	if (size == 0)
		return 0;
	else if (size > MAX_CONTENT_LENGTH)
		size = MAX_CONTENT_LENGTH;

	char *buf = pool_alloc(r->p, size + 1);
	if (!buf)
		return -1;
	if (fread(buf, size, 1, stdin) != 1)
		return -1;

	buf[size] = '\0';
	_parse_params(r, buf, '&');
	return 0;
}

/**
 * Print HTML response header.
 */
void html_header(void)
{
	printf("Content-type: text/html; charset="CHARSET"\n\n"
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" "
			"\"http://www.w3.org/TR/html4/strict.dtd\"><html><head>");
}

/**
 * Print XML response header.
 * @param xslfile The name of the XSLT file.
 */
void xml_header(const char *xslfile)
{
	const char *xsl = xslfile ? xslfile : "bbs";
	printf("Content-type: text/xml; charset="CHARSET"\n\n"
			"<?xml version=\"1.0\" encoding=\""CHARSET"\"?>\n"
			"<?xml-stylesheet type=\"text/xsl\" href=\"../xsl/%s.xsl\"?>\n",
			xsl);
}
