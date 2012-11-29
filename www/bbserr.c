#include "libweb.h"

/**
 * Print HTTP error message with status code set.
 * @param status HTTP status code
 * @param prompt description of the error
 * @return 0
 */
static int http_fatal2(int status, const char *prompt)
{
	printf("Content-type: text/html; charset=%s\nStatus: %d\n\n",
			CHARSET, status);
	//% printf("<html><head><title>发生错误</title></head><body><div>%s</div>"
	printf("<html><head><title>\xb7\xa2\xc9\xfa\xb4\xed\xce\xf3</title></head><body><div>%s</div>"
			//% "<a href=javascript:history.go(-1)>快速返回</a></body></html>",
			"<a href=javascript:history.go(-1)>\xbf\xec\xcb\xd9\xb7\xb5\xbb\xd8</a></body></html>",
			prompt);
	FCGI_Finish();
	return 0;
}

/**
 * Print HTTP error message with status OK (200).
 * @param prompt description of the error
 * @return 0
 */
static int http_fatal(const char *prompt)
{
	return http_fatal2(HTTP_OK, prompt);
}

/**
 * Print error information.
 * @param err bbs errno
 * @return 0
 */
int check_bbserr(int err)
{
	if (err >= 0)
		return 0;
	switch (err) {
		case BBS_EINVAL:
			//% return http_fatal2(HTTP_BAD_REQUEST, "参数错误");
			return http_fatal2(HTTP_BAD_REQUEST, "\xb2\xce\xca\xfd\xb4\xed\xce\xf3");
		case BBS_ELGNREQ:
			//% return http_fatal("请先<a href='login'>登录</a>");
			return http_fatal("\xc7\xeb\xcf\xc8<a href='login'>\xb5\xc7\xc2\xbc</a>");
		case BBS_EACCES:
			//% return http_fatal("权限不足");
			return http_fatal("\xc8\xa8\xcf\xde\xb2\xbb\xd7\xe3");
		case BBS_EPST:
			//% return http_fatal("此文不可回复，或您没有发文权限");
			return http_fatal("\xb4\xcb\xce\xc4\xb2\xbb\xbf\xc9\xbb\xd8\xb8\xb4\xa3\xac\xbb\xf2\xc4\xfa\xc3\xbb\xd3\xd0\xb7\xa2\xce\xc4\xc8\xa8\xcf\xde");
		case BBS_ENOFILE:
			//% return http_fatal("找不到指定的文件");
			return http_fatal("\xd5\xd2\xb2\xbb\xb5\xbd\xd6\xb8\xb6\xa8\xb5\xc4\xce\xc4\xbc\xfe");
		case BBS_ENODIR:
			//% return http_fatal("找不到指定的目录");
			return http_fatal("\xd5\xd2\xb2\xbb\xb5\xbd\xd6\xb8\xb6\xa8\xb5\xc4\xc4\xbf\xc2\xbc");
		case BBS_ENOBRD:
			//% return http_fatal("找不到指定的版面，或权限不足");
			return http_fatal("\xd5\xd2\xb2\xbb\xb5\xbd\xd6\xb8\xb6\xa8\xb5\xc4\xb0\xe6\xc3\xe6\xa3\xac\xbb\xf2\xc8\xa8\xcf\xde\xb2\xbb\xd7\xe3");
		case BBS_ENOUSR:
			//% return http_fatal("找不到指定的用户");
			return http_fatal("\xd5\xd2\xb2\xbb\xb5\xbd\xd6\xb8\xb6\xa8\xb5\xc4\xd3\xc3\xbb\xa7");
		case BBS_ENOURL:
			//% return http_fatal("找不到指定的网址");
			return http_fatal("\xd5\xd2\xb2\xbb\xb5\xbd\xd6\xb8\xb6\xa8\xb5\xc4\xcd\xf8\xd6\xb7");
		case BBS_EDUPLGN:
			//% return http_fatal("您在本机已经登录了一个帐号，请先退出");
			return http_fatal("\xc4\xfa\xd4\xda\xb1\xbe\xbb\xfa\xd2\xd1\xbe\xad\xb5\xc7\xc2\xbc\xc1\xcb\xd2\xbb\xb8\xf6\xd5\xca\xba\xc5\xa3\xac\xc7\xeb\xcf\xc8\xcd\xcb\xb3\xf6");
		case BBS_EWPSWD:
			//% return http_fatal("用户名和密码不匹配");
			return http_fatal("\xd3\xc3\xbb\xa7\xc3\xfb\xba\xcd\xc3\xdc\xc2\xeb\xb2\xbb\xc6\xa5\xc5\xe4");
		case BBS_EBLKLST:
			//% return http_fatal("您在对方的黑名单中");
			return http_fatal("\xc4\xfa\xd4\xda\xb6\xd4\xb7\xbd\xb5\xc4\xba\xda\xc3\xfb\xb5\xa5\xd6\xd0");
		case BBS_ELGNQE:
			//% return http_fatal("您不能登录更多帐号了");
			return http_fatal("\xc4\xfa\xb2\xbb\xc4\xdc\xb5\xc7\xc2\xbc\xb8\xfc\xb6\xe0\xd5\xca\xba\xc5\xc1\xcb");
		case BBS_EBRDQE:
			//% return http_fatal("收藏夹版面数已达上限");
			return http_fatal("\xca\xd5\xb2\xd8\xbc\xd0\xb0\xe6\xc3\xe6\xca\xfd\xd2\xd1\xb4\xef\xc9\xcf\xcf\xde");
		case BBS_EATTQE:
			//% return http_fatal("版面附件区容量已满");
			return http_fatal("\xb0\xe6\xc3\xe6\xb8\xbd\xbc\xfe\xc7\xf8\xc8\xdd\xc1\xbf\xd2\xd1\xc2\xfa");
		case BBS_EMAILQE:
			//% return http_fatal("信箱已满");
			return http_fatal("\xd0\xc5\xcf\xe4\xd2\xd1\xc2\xfa");
		case BBS_EFRNDQE:
			//% return http_fatal("您的好友数已达上限");
			return http_fatal("\xc4\xfa\xb5\xc4\xba\xc3\xd3\xd1\xca\xfd\xd2\xd1\xb4\xef\xc9\xcf\xcf\xde");
		case BBS_EFBIG:
			//% return http_fatal("文件大小超过限制");
			return http_fatal("\xce\xc4\xbc\xfe\xb4\xf3\xd0\xa1\xb3\xac\xb9\xfd\xcf\xde\xd6\xc6");
		case BBS_ELFREQ:
			//% return http_fatal("登录过于频繁");
			return http_fatal("\xb5\xc7\xc2\xbc\xb9\xfd\xd3\xda\xc6\xb5\xb7\xb1");
		case BBS_EPFREQ:
			//% return http_fatal("发文过于频繁");
			return http_fatal("\xb7\xa2\xce\xc4\xb9\xfd\xd3\xda\xc6\xb5\xb7\xb1");
		case BBS_E2MANY:
			//% return http_fatal("在线用户数已达上限");
			return http_fatal("\xd4\xda\xcf\xdf\xd3\xc3\xbb\xa7\xca\xfd\xd2\xd1\xb4\xef\xc9\xcf\xcf\xde");
		case BBS_EINTNL:
			//% return http_fatal2(HTTP_INTERNAL_SERVER_ERROR, "内部错误");
			return http_fatal2(HTTP_INTERNAL_SERVER_ERROR, "\xc4\xda\xb2\xbf\xb4\xed\xce\xf3");
		case BBS_ERMQE:
			//% return http_fatal("对方信箱已满");
			return http_fatal("\xb6\xd4\xb7\xbd\xd0\xc5\xcf\xe4\xd2\xd1\xc2\xfa");
		default:
			//% return http_fatal("未知错误");
			return http_fatal("\xce\xb4\xd6\xaa\xb4\xed\xce\xf3");
	}
}
