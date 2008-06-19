#include "BBSLIB.inc"

int main() {
	char path[80];
	init_all();
	printf("<b>删除消息 · %s </b><br>\n",BBSNAME);
	printpretable_lite();
	if(!loginok) http_fatal("匆匆过客不能处理讯息, 请先登录");
	sethomefile(path, currentuser.userid, "msgfile.me");
	unlink(path);
	printf("已删除所有讯息备份");
	printposttable_lite();
	http_quit();
}
