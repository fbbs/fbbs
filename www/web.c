#include <ctype.h>
#include <gcrypt.h>
#include <stdlib.h>
#include <string.h>
#include "libweb.h"
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/web.h"
#include "fbbs/xml.h"

typedef struct http_req_t {
	const char *from;
	pair_t params[MAX_PARAMETERS];
	int count;
	int flag;
} http_req_t;

typedef struct http_response_t {
	int type;
	xml_document_t *doc;
} http_response_t;

struct web_ctx_t {
	pool_t *p;
	http_req_t req;
	gcry_md_hd_t sha1;
	bool inited;
	http_response_t r;
};

static struct web_ctx_t ctx = { .inited = false };

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

/**
 * Parse a 'key=value' pair and put it into request struct.
 * @param r The http request.
 * @param begin The string.
 * @param len Length of the string.
 * @return 0 on success, -1 on error.
 */
static int _parse_param(http_req_t *r, const char *begin, size_t len)
{
	if (len == 0)
		return 0;

	char *s = pool_alloc(ctx.p, len + 1);
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

	r->params[r->count].key = trim(key);
	r->params[r->count].val = val;
	++r->count;
	return 0;
}

/**
 * Parse 'key=value' pairs and put them into request struct.
 * @param r The http request.
 * @param buf Value buffer.
 * @param delim Delimiter.
 * @return 0 on success, -1 on error.
 */
static int _parse_params(http_req_t *r, const char *buf, int delim)
{
	const char *ptr = strchr(buf, delim), *last = buf;
	while (ptr) {
		if (_parse_param(r, last, ptr - last) != 0)
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
	if (_parse_params(r, _get_server_env("QUERY_STRING"), '&') < 0)
		return -1;
	if (_parse_params(r, _get_server_env("HTTP_COOKIE"), ';') < 0)
		return -1;
	return 0;
}

/**
 * Get a parameter value.
 * @param r The http request.
 * @param key The name of the parameter.
 * @return the value corresponding to 'key', an empty string if not found.
 */
const char *get_param(const char *key)
{
	http_req_t *r = &ctx.req;
	for (int i = 0; i < r->count; ++i) {
		if (streq(r->params[i].key, key))
			return r->params[i].val;
	}
	return "";
}

const pair_t *get_param_pair(int idx)
{
	if (idx >= 0 && idx < ctx.req.count)
		return ctx.req.params + idx;
	return NULL;
}

struct _option_pairs {
	const char *param;
	int flag;
};

static bool ends_with(const char *s, const char *pattern)
{
	size_t len_s = strlen(s), len_p = strlen(pattern);

	if (len_s >= len_p)
		return !memcmp(s + len_s - len_p, pattern, len_p);

	return false;
}

static void get_global_options(void)
{
	struct _option_pairs pairs[] = {
		{ "api", REQUEST_API },
		{ "new", REQUEST_PARSED },
		{ "mob", REQUEST_MOBILE },
		{ "utf8", REQUEST_UTF8 }
	};

	ctx.req.flag = 0;
	for (int i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
		if (*get_param(pairs[i].param) == '1')
			ctx.req.flag |= pairs[i].flag;
	}

	const char *name = getenv("SCRIPT_NAME");
	if (name) {
		if (ends_with(name, ".xml"))
			ctx.req.flag |= REQUEST_XML;
		if (ends_with(name, ".json"))
			ctx.req.flag |= REQUEST_JSON;
	}
}

/**
 * Get an http request.
 * The GET request and cookies are parsed into key=value pairs.
 * @return True on success, false on error.
 */
static bool get_web_request(void)
{
	ctx.req.count = 0;
	if (_parse_http_req(&ctx.req) != 0)
		return false;

	ctx.req.from = _get_server_env("REMOTE_ADDR");

	get_global_options();

	return true;
}

bool request_type(int type)
{
	return ctx.req.flag & type;
}

/**
 * Parse parameters submitted by POST method.
 * @return 0 on success, -1 on error.
 */
int parse_post_data(void)
{
	unsigned long size = strtoul(_get_server_env("CONTENT_LENGTH"), NULL, 10);
	if (size == 0)
		return 0;
	else if (size > MAX_CONTENT_LENGTH)
		size = MAX_CONTENT_LENGTH;

	char *buf = pool_alloc(ctx.p, size + 1);
	if (!buf)
		return -1;
	if (fread(buf, size, 1, stdin) != 1)
		return -1;

	buf[size] = '\0';
	_parse_params(&ctx.req, buf, '&');
	return 0;
}

static int initialize_gcrypt(void)
{
	if (!gcry_check_version(GCRYPT_VERSION))
		return -1;

	if (gcry_control(GCRYCTL_DISABLE_SECMEM, 0) != 0)
		return -1;

	if (gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0) != 0)
		return -1;

	if (gcry_md_open(&ctx.sha1, GCRY_MD_SHA1, 0) != 0)
		return -1;

	return 0;
}

bool web_ctx_init(void)
{
	if (!ctx.inited) {
		if (initialize_gcrypt() == 0)
			ctx.inited = true;
		else
			return false;
	}

	ctx.p = pool_create(0);
	ctx.r.doc = xml_new_doc();
	ctx.r.type = RESPONSE_DEFAULT;

	return get_web_request();
}

void web_ctx_destroy(void)
{
	pool_destroy(ctx.p);
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
			"<?xml-stylesheet type=\"text/xsl\" href=\"../xsl/%s.xsl?v1416\"?>\n",
			xsl);
}

void xml_print(const char *s)
{
	char *c = (char *)s;
	char *last = c;
	char *subst;
	while (*c != '\0') {
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
			case '\r':
				subst = "";
				break;
			default:
				subst = NULL;
				break;
		}
		if (subst) {
			fwrite(last, 1, c - last, stdout);
			fputs(subst, stdout);
			last = ++c;
		} else {
			++c;
		}
	}
	fwrite(last, 1, c - last, stdout);
}

const unsigned char *calc_digest(const void *s, size_t size)
{
	gcry_md_reset(ctx.sha1);
	gcry_md_write(ctx.sha1, s, size);
	gcry_md_final(ctx.sha1);
	return gcry_md_read(ctx.sha1, 0);
}

void *palloc(size_t size)
{
	return pool_alloc(ctx.p, size);
}

char *pstrdup(const char *s)
{
	return pool_strdup(ctx.p, s, 0);
}

void set_response_type(int type)
{
	ctx.r.type = type;
}

void set_response_root(const char *name, int type, int encoding)
{
	xml_node_t *node = xml_new_node(name, type);
	xml_set_doc_root(ctx.r.doc, node);
	xml_set_encoding(ctx.r.doc, encoding);
}

static int response_type(void)
{
	int type = ctx.r.type;
	if (type == RESPONSE_DEFAULT) {
		if (request_type(REQUEST_JSON))
			return RESPONSE_JSON;
		return RESPONSE_XML;
	}
	return type;
}

static const char *content_type(int type)
{
	switch (type) {
		case RESPONSE_HTML:
			return "text/html";
		case RESPONSE_JSON:
			return "application/json";
		default:
			return "text/xml";
	}
}

void respond(int code)
{
	int type = response_type();

	printf("Content-type: %s;  charset=utf-8\n"
			"Status: %d\n\n", content_type(type), code);

	xml_dump(ctx.r.doc, type == RESPONSE_JSON ? XML_AS_JSON : XML_AS_XML);
	FCGI_Finish();
}

struct error_msg_t {
	int code;
	int http_status_code;
	const char *msg;
};

static const struct error_msg_t error_msgs[] = {
	{ ERROR_INCORRECT_PASSWORD, HTTP_UNAUTHORIZED, "incorrect username or password" },
	{ ERROR_USER_SUSPENDED, HTTP_FORBIDDEN, "permission denied" },
	{ ERROR_BAD_REQUEST, HTTP_BAD_REQUEST, "bad request" },
};

int error_msg(int code)
{
	xml_node_t *node = xml_new_node("bbs_error", XML_NODE_ANONYMOUS_JSON);
	xml_set_doc_root(ctx.r.doc, node);
	xml_set_encoding(ctx.r.doc, XML_ENCODING_UTF8);

	const struct error_msg_t *e = error_msgs;
	if (code >= 0 && code < NELEMS(error_msgs))
		e = error_msgs + code;

	xml_attr_string(node, "msg", e->msg, false);
	xml_attr_integer(node, "code", e->code + 10000);

	return e->http_status_code;
}
