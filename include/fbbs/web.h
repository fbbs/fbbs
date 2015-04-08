#ifndef FB_WEB_H
#define FB_WEB_H

#include <fcgi_stdio.h>

#include "fbbs/json.h"

#define WEB_COOKIE_KEY  "utmpkey"
#define WEB_COOKIE_USER  "utmpuser"

enum {
	WEB_PARAM_MAX = 32,
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
	WEB_STATUS_OK = 200,
	WEB_STATUS_FOUND = 302,
	WEB_STATUS_BAD_REQUEST = 400,
	WEB_STATUS_UNAUTHORIZED = 401,
	WEB_STATUS_FORBIDDEN = 403,
	WEB_STATUS_NOT_FOUND = 404,
	WEB_STATUS_METHOD_NOT_ALLOWED = 405,
	WEB_STATUS_INTERNAL_SERVER_ERROR = 500,
	WEB_STATUS_SERVICE_UNAVAILABLE = 503,
} web_status_code_e;

typedef enum {
	WEB_OK = 1,  ///< 没有错误, 自行构建输出
	WEB_ERROR_NONE, ///< 没有错误, 采用默认输出
	WEB_ERROR_INCORRECT_PASSWORD,
	WEB_ERROR_USER_SUSPENDED,
	WEB_ERROR_BAD_REQUEST,
	WEB_ERROR_INTERNAL,
	WEB_ERROR_LOGIN_REQUIRED,
	WEB_ERROR_BOARD_NOT_FOUND,
	WEB_ERROR_METHOD_NOT_ALLOWED,
} web_error_code_e;

typedef struct {
	char *key;
	char *val;
} web_param_pair_t;

extern bool web_ctx_init(void);
extern void web_ctx_destroy(void);

extern const char *web_get_param(const char *name);
extern const web_param_pair_t *web_get_param_pair(int idx);
extern int web_parse_post_data(void);

extern bool _web_request_type(web_request_type_e type);
#define web_request_type(type)  _web_request_type(WEB_REQUEST_##type)
extern bool _web_request_method(web_request_method_e method);
#define web_request_method(method)  _web_request_method(WEB_REQUEST_METHOD_##method)

extern void html_header(void);
extern void xml_header(const char *xslfile);

extern void xml_print(const char *s);
extern int xml_print_post(const char *str, size_t size, int option);

extern const unsigned char *web_calc_digest(const void *s, size_t size);

extern void *web_palloc(size_t size);
extern char *web_pstrdup(const char *s);

extern void web_set_response(json_object_t *object, json_value_e type);
extern void web_respond(web_error_code_e code);

#endif // FB_WEB_H
