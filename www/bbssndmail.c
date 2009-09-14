#include "libweb.h"

int bbssndmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	if (!HAS_PERM2(PERM_MAIL, &currentuser))
		return BBS_EACCES;
	if (parse_post_data() < 0)
		return BBS_EINVAL;
	// TODO: mail quota
	const char *recv = getparm("recv");
	if (*recv == '\0')
		return BBS_EINVAL;
	const char *title = getparm("title");
	if (*title == '\0')
		title = "没主题";
	const char *text = getparm("text");
	int len = strlen(text);
	char header[320];
	snprintf(header, sizeof(header), "寄信人: %s (%s)\n标  题: %s\n发信站: "
			BBSNAME" (%s)\n来  源: %s\n\n", currentuser.userid,
			currentuser.username, title, getdatestring(time(NULL), DATE_ZH),
			mask_host(fromhost));
	// TODO: signature, error code, backup?
	if (do_mail_file(recv, title, header, text, len, NULL) < 0)
		return BBS_EINVAL;
	const char *ref = getparm("ref");
	http_header();
	refreshto(1, ref);
	printf("</head>\n<body>发表成功，1秒钟后自动转到<a href='%s'>原页面</a>\n"
			"</body>\n</html>\n", ref);
	return 0;
}
