#include "BBSLIB.inc"

int main() {
	char filename[80];
	init_all();
	printf("<b>消息寄回邮箱 · %s </b><br>\n",BBSNAME);
	printpretable_lite();

	if(!loginok) http_fatal("匆匆过客不能处理消息，请先登录");
	/* modified by roly ? bug??*/
	//sprintf(filename, "home/%c/%s/msgfile", toupper(currentuser.userid[0]), currentuser.userid);
	sprintf(filename, "home/%c/%s/msgfile.me", toupper(currentuser.userid[0]), currentuser.userid);
	/* modify end */
	post_mail(currentuser.userid, "所有消息备份", filename, currentuser.userid, currentuser.username, fromhost, -1);
	unlink(filename);
	printf("消息备份已经寄回您的信箱");
	printposttable_lite();
	printf("<a href='javascript:history.go(-2)'>返回</a>");
	http_quit();
}
