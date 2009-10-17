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
	printf("<html><head><title>发生错误</title></head><body><div>%s</div>"
			"<a href=javascript:history.go(-1)>快速返回</a></body></html>",
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
	return http_fatal2(HTTP_STATUS_OK, prompt);
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
			return http_fatal2(HTTP_STATUS_BADREQUEST, "参数错误");
		case BBS_ELGNREQ:
			return http_fatal("请先登录");
		case BBS_EACCES:
			return http_fatal("权限不足");
		case BBS_EPST:
			return http_fatal("此文不可回复，或您没有发文权限");
		case BBS_ENOFILE:
			return http_fatal("找不到指定的文件");
		case BBS_ENODIR:
			return http_fatal("找不到指定的目录");
		case BBS_ENOBRD:
			return http_fatal("找不到指定的版面，或权限不足");
		case BBS_ENOUSR:
			return http_fatal("找不到指定的用户");
		case BBS_ENOURL:
			return http_fatal("找不到指定的网址");
		case BBS_EDUPLGN:
			return http_fatal("您在本机已经登录了一个帐号，请先退出");
		case BBS_EWPSWD:
			return http_fatal("用户名和密码不匹配");
		case BBS_EBLKLST:
			return http_fatal("您在对方的黑名单中");
		case BBS_ELGNQE:
			return http_fatal("您不能登录更多帐号了");
		case BBS_EBRDQE:
			return http_fatal("收藏夹版面数已达上限");
		case BBS_EATTQE:
			return http_fatal("版面附件区容量已满");
		case BBS_EMAILQE:
			return http_fatal("信箱已满");
		case BBS_EFRNDQE:
			return http_fatal("您的好友数已达上限");
		case BBS_EFBIG:
			return http_fatal("文件大小超过限制");
		case BBS_ELFREQ:
			return http_fatal("登录过于频繁");
		case BBS_EPFREQ:
			return http_fatal("发文过于频繁");
		case BBS_E2MANY:
			return http_fatal("在线用户数已达上限");
		case BBS_EINTNL:
			return http_fatal2(HTTP_STATUS_INTERNAL_ERROR, "内部错误");
		case BBS_ERMQE:
			return http_fatal("对方信箱已满");
		default:
			return http_fatal("未知错误");
	}
}
