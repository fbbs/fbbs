#include "libweb.h"

int bbspreupload_main(void)
{
	char *board = getparm("board");
	if (!loginok)
		http_fatal("匆匆过客无权限上传文件");
	struct boardheader *bp = getbcache(board);
	if (bp == NULL || !haspostperm(&currentuser, bp))
		http_fatal("错误的讨论区或无权上传文件至本讨论区");

	xml_header("bbspreupload");
	printf("<bbspreupload><board>%s</board><user>%s</user><max>%d</max>"
			"</bbspreupload>", board, currentuser.userid, maxlen(board));
	return 0;
}
