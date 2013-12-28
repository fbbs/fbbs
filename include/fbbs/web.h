#ifndef FB_WEB_H
#define FB_WEB_H

#include <fcgi_stdio.h>

#include "fbbs/xml.h"

#define COOKIE_KEY  "utmpkey"
#define COOKIE_USER  "utmpuser"

enum {
	MAX_PARAMETERS = 32,
	MAX_CONTENT_LENGTH = 1 * 1024 * 1024,

	PARSE_NOSIG = 0x1,
	PARSE_NOQUOTEIMG = 0x2,
	PARSE_NOSIGIMG = 0x4,
};

typedef enum {
	WEB_REQUEST_API = 1,
	WEB_REQUEST_PARSED = 1 << 1,
	WEB_REQUEST_MOBILE = 1 << 2,
	WEB_REQUEST_UTF8 = 1 << 3,
	WEB_REQUEST_XML = 1 << 4,
	WEB_REQUEST_JSON = 1 << 5,
	WEB_REQUEST_XHR = 1 << 6,
} web_request_type_e;

typedef enum {
	WEB_REQUEST_METHOD_GET,
	WEB_REQUEST_METHOD_POST,
	WEB_REQUEST_METHOD_PUT,
	WEB_REQUEST_METHOD_DELETE,
} web_request_method_e;

typedef enum {
	HTTP_OK = 200,
	HTTP_FOUND = 302,
	HTTP_BAD_REQUEST = 400,
	HTTP_UNAUTHORIZED = 401,
	HTTP_FORBIDDEN = 403,
	HTTP_NOT_FOUND = 404,
	HTTP_INTERNAL_SERVER_ERROR = 500,
	HTTP_SERVICE_UNAVAILABLE = 503,
} http_status_code_e;

typedef enum {
	ERROR_NONE = 1,
	ERROR_INCORRECT_PASSWORD,
	ERROR_USER_SUSPENDED,
	ERROR_BAD_REQUEST,
	ERROR_INTERNAL,
	ERROR_LOGIN_REQUIRED,
	ERROR_BOARD_NOT_FOUND,
} error_code_e;

enum {
	RESPONSE_DEFAULT = 0,
	RESPONSE_HTML = 1,
	RESPONSE_XML = 2,
	RESPONSE_JSON = 3,
};

typedef struct pair_t {
	char *key;
	char *val;
} pair_t;

extern bool web_ctx_init(void);
extern void web_ctx_destroy(void);

extern const char *web_get_param(const char *name);
extern const pair_t *web_get_param_pair(int idx);
extern int parse_post_data(void);

extern bool _web_request_type(web_request_type_e type);
#define web_request_type(type)  _web_request_type(WEB_REQUEST_##type)
extern web_request_method_e web_request_method(void);

extern void html_header(void);
extern void xml_header(const char *xslfile);

extern void xml_print(const char *s);
extern int xml_print_post(const char *str, size_t size, int option);

extern const unsigned char *calc_digest(const void *s, size_t size);

extern void *palloc(size_t size);
extern char *pstrdup(const char *s);

extern void set_response_type(int type);
extern xml_node_t *set_response_root(const char *name, int type, int encoding);
extern void respond(int code);

extern http_status_code_e error_msg(int code);

#endif // FB_WEB_H
