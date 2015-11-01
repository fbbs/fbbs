#include <ctype.h>
#include <gcrypt.h>
#include <stdlib.h>
#include <string.h>
#include "libweb.h"
#include "fbbs/pool.h"
#include "fbbs/string.h"
#include "fbbs/web.h"
#include "fbbs/json.h"

typedef struct {
	const char *from;
	web_param_pair_t params[WEB_PARAM_MAX];
	int count;
	int flag;
	web_request_method_e method;
} web_request_t;

typedef struct web_response_t {
	json_value_e type;
	json_object_t *object;
} web_response_t;

struct web_ctx_t {
	pool_t *p;
	web_request_t req;
	gcry_md_hd_t sha1;
	bool inited;
	bool remove_cookies;
	web_response_t resp;
};

static struct web_ctx_t ctx = { .inited = false };

/**
 * 获取HTTP请求的环境变量.
 * @param[in] key 键值
 * @return 如果找到相应键值, 返回其内容, 否则返回空字符串.
 */
static const char *_get_server_env(const char *key)
{
	const char *s = getenv(key);
	return s ? s : "";
}

/**
 * 将十六进制字符转换成十进制.
 * @param[in] c 十六进制字符
 * @return 相应的十进制数字, 出错返回0.
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
 * 解码URL字符串.
 * "+"转成空格, "%xx"转成相应字符
 * @param[in,out] s 要解析的字符串
 * @return 转换后的字符串
 */
static char *_url_decode(char *s)
{
	char *f = s, *t = s;
	for (; *f; ++f, ++t) {
		if (*f == '+') {
			*t = ' ';
		} else if (*f == '%') {
			if (f[1] && f[2]) {
				*t = _hex2dec(f[1]) * 16 + _hex2dec(f[2]);
				f += 2;
				continue;
			} else {
				*t = '\0';
				return s;
			}
		} else {
			*t = *f;
		}
	}
	*t = '\0';
	return s;
}

/**
 * 解析形如"key=value"的字符串
 * @param[out] req HTTP请求结构
 * @param[in] str 字符串
 * @param[in] len 字符串长度
 * @return 成功返回0, 否则-1
 */
static int _parse_param(web_request_t *req, const char *str, size_t len)
{
	if (!len)
		return 0;

	char *ptr = pool_alloc(ctx.p, len + 1);
	if (!ptr)
		return -1;

	strlcpy(ptr, str, len + 1);

	char *key = ptr, *val = strchr(ptr, '=');
	if (val) {
		*val++ = '\0';
		_url_decode(val);
	} else {
		val = "";
	}

	req->params[req->count].key = trim(key);
	req->params[req->count].val = val;
	++req->count;
	return 0;
}

/**
 * 解析形如"k1=v1&k2=v2"(或其他分隔符)的字符串
 * @param[out] req HTTP请求结构
 * @param[in] str 字符串
 * @param delim 键值对之间的分隔符
 * @return 成功返回0, 否则-1
 */
static int _parse_params(web_request_t *req, const char *str, int delim)
{
	const char *ptr = strchr(str, delim), *last = str;
	while (ptr) {
		if (_parse_param(req, last, ptr - last) != 0)
			return -1;
		last = ptr + 1;
		ptr = strchr(last, delim);
	}
	if (_parse_param(req, last, strlen(last)) < 0)
		return -1;
	return 0;
}

/**
 * 解析HTTP请求中的GET参数和cookie
 * @param[out] req HTTP请求结构
 * @return 成功返回0, 否则-1
 */
static int _parse_http_req(web_request_t *req)
{
	if (_parse_params(req, _get_server_env("QUERY_STRING"), '&') < 0)
		return -1;
	if (_parse_params(req, _get_server_env("HTTP_COOKIE"), ';') < 0)
		return -1;
	return 0;
}

/**
 * 获取请求参数
 * @param[in] key 参数名
 * @return 对应的值, 找不到则返回空字符串
 */
const char *web_get_param(const char *key)
{
	web_request_t *r = &ctx.req;
	for (int i = 0; i < r->count; ++i) {
		if (streq(r->params[i].key, key))
			return r->params[i].val;
	}
	return "";
}

long web_get_param_long(const char *key)
{
	return strtol(web_get_param(key), NULL, 10);
}

const web_param_pair_t *web_get_param_pair(int idx)
{
	if (idx >= 0 && idx < ctx.req.count)
		return ctx.req.params + idx;
	return NULL;
}

typedef struct {
	const char *param;
	int flag;
} option_pairs_t;

static bool ends_with(const char *s, const char *pattern)
{
	size_t len_s = strlen(s), len_p = strlen(pattern);

	if (len_s >= len_p)
		return !memcmp(s + len_s - len_p, pattern, len_p);

	return false;
}

static void get_global_options(void)
{
	option_pairs_t pairs[] = {
		{ "api", WEB_REQUEST_API },
		{ "new", WEB_REQUEST_PARSED },
		{ "mob", WEB_REQUEST_MOBILE },
		{ "utf8", WEB_REQUEST_UTF8 },
	};

	ctx.req.flag = 0;
	for (int i = 0; i < ARRAY_SIZE(pairs); ++i) {
		if (*web_get_param(pairs[i].param) == '1')
			ctx.req.flag |= pairs[i].flag;
	}

	const char *name = getenv("SCRIPT_NAME");
	if (name) {
		if (ends_with(name, ".xml"))
			ctx.req.flag |= WEB_REQUEST_XML | WEB_REQUEST_API;
		if (ends_with(name, ".json"))
			ctx.req.flag |= WEB_REQUEST_JSON | WEB_REQUEST_API;
	}
	if (ctx.req.flag & WEB_REQUEST_API)
		ctx.req.flag |= WEB_REQUEST_UTF8;

	const char *xhr_header = getenv("HTTP_X_REQUESTED_WITH");
	if (xhr_header && streq(xhr_header, "XMLHttpRequest"))
		ctx.req.flag |= WEB_REQUEST_XHR;
}

bool _web_request_type(web_request_type_e type)
{
	return ctx.req.flag & type;
}

static web_request_method_e parse_request_method(void)
{
	const char *method = _get_server_env("REQUEST_METHOD");
	if (streq(method, "GET"))
		return WEB_REQUEST_METHOD_GET;
	else if (streq(method, "POST"))
		return WEB_REQUEST_METHOD_POST;
	else if (streq(method, "PUT"))
		return WEB_REQUEST_METHOD_PUT;
	else if (streq(method, "DELETE"))
		return WEB_REQUEST_METHOD_DELETE;
	return WEB_REQUEST_METHOD_GET;
}

/**
 * 解析HTTP请求
 * 解析GET参数和cookie.
 * @return 成功true, 否则false
 */
static bool parse_web_request(void)
{
	ctx.req.count = 0;
	if (_parse_http_req(&ctx.req) != 0)
		return false;

	ctx.req.from = _get_server_env("REMOTE_ADDR");
	ctx.req.method = parse_request_method();

	get_global_options();

	return true;
}

bool _web_request_method(web_request_method_e method)
{
	return ctx.req.method == method;
}

/**
 * Parse parameters submitted by POST method.
 * @return 0 on success, -1 on error.
 */
int web_parse_post_data(void)
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

/**
 * 初始化web环境
 * @return 成功true, 否则false
 */
bool web_ctx_init(void)
{
	if (!ctx.inited) {
		if (initialize_gcrypt() == 0)
			ctx.inited = true;
		else
			return false;
	}

	ctx.remove_cookies = false;
	ctx.p = pool_create(0);
	return parse_web_request();
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
	const char *charset = web_request_type(UTF8) ? "utf-8" : CHARSET;
	const char *xsl = xslfile ? xslfile : "bbs";
	printf("Content-type: text/xml; charset=%s\n\n"
			"<?xml version=\"1.0\" encoding=\"%s\"?>\n"
			"<?xml-stylesheet type=\"text/xsl\" href=\"../xsl/%s.xsl?v20150923\"?>\n",
			charset, charset, xsl);
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

const unsigned char *web_calc_digest(const void *s, size_t size)
{
	gcry_md_reset(ctx.sha1);
	gcry_md_write(ctx.sha1, s, size);
	gcry_md_final(ctx.sha1);
	return gcry_md_read(ctx.sha1, 0);
}

void *web_palloc(size_t size)
{
	return pool_alloc(ctx.p, size);
}

char *web_pstrdup(const char *s)
{
	return pool_strdup(ctx.p, s, 0);
}

void web_set_response(json_object_t *object, json_value_e type)
{
	ctx.resp.object = object;
	ctx.resp.type = type;
}

void web_remove_cookies(void)
{
	ctx.remove_cookies = true;
}

static void remove_cookie(const char *key)
{
	printf("Set-cookie: %s=;Expires=Fri, 19-Apr-1996 11:11:11 GMT\n", key);
}

struct error_msg_t {
	web_error_code_e code;
	web_status_code_e status;
	const char *msg;
};

static const web_status_code_e errors[] = {
	[WEB_ERROR_NONE] = WEB_STATUS_OK,
	[WEB_ERROR_INCORRECT_PASSWORD] = WEB_STATUS_FORBIDDEN,
	[WEB_ERROR_USER_SUSPENDED] = WEB_STATUS_FORBIDDEN,
	[WEB_ERROR_BAD_REQUEST] = WEB_STATUS_BAD_REQUEST,
	[WEB_ERROR_INTERNAL] = WEB_STATUS_INTERNAL_SERVER_ERROR,
	[WEB_ERROR_LOGIN_REQUIRED] = WEB_STATUS_FORBIDDEN,
	[WEB_ERROR_BOARD_NOT_FOUND] = WEB_STATUS_NOT_FOUND,
	[WEB_ERROR_METHOD_NOT_ALLOWED] = WEB_STATUS_METHOD_NOT_ALLOWED,
};

static web_status_code_e error_msg(web_error_code_e code)
{
	json_object_t *object = json_object_new();
	web_set_response(object, JSON_OBJECT);
	json_object_integer(object, "code", code + 10000);

	int status = WEB_STATUS_OK;
	if (code > WEB_OK && code < ARRAY_SIZE(errors))
		status = errors[code];
	return status;
}

void web_respond(web_error_code_e code)
{
	web_status_code_e status = WEB_STATUS_OK;
	if (code != WEB_OK)
		status = error_msg(code);

	printf("Content-type: application/json; charset=utf-8\n"
			"Status: %d\n", (int) status);
	if (ctx.remove_cookies) {
		remove_cookie(WEB_COOKIE_USER);
		remove_cookie(WEB_COOKIE_KEY);
	}
	putchar('\n');

	json_dump(ctx.resp.object, ctx.resp.type);
	FCGI_Finish();
}
