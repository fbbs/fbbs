#include "BBSLIB.inc"

int main() {
   	FILE *fp;
	char path[80], userid[80], exp[80];
	init_all();
   	if(!loginok) http_fatal("您尚未登录，请先登录");
   	sprintf(path, "home/%c/%s/friends", toupper(currentuser.userid[0]), currentuser.userid);
   	printf("<b><font style='font-size: 18pt'>添加好友</font> · %s [使用者: %s]</b>\n", BBSNAME, currentuser.userid);
	printpretable_lite();
	strsncpy(userid, getparm("userid"), 13);
	strsncpy(exp, getparm("exp"), 32);
	loadfriend(currentuser.userid);
	if(userid[0]==0 || exp[0]==0) {
		if(userid[0]) printf("<font color=red>请输入好友说明</font>");
		printf("<form action=bbsfadd>\n");
		printf("请输入欲加入的好友帐号: <input type=text name=userid value='%s'><br>\n",
			userid);
		printf("请输入对这个好友的说明: <input type=text name=exp>\n", 
			exp);
		printf("<br><br><input type=submit value=确定></form>\n");
		http_quit();
	}
	if(!getuser(userid)) http_fatal("错误的使用者帐号");
	if(friendnum>=199) http_fatal("您的好友名单已达到上限, 不能添加新的好友");
   	if(isfriend(userid)) http_fatal("此人已经在您的好友名单里了");
	strcpy(fff[friendnum].id, getuser(userid)->userid);
	strcpy(fff[friendnum].exp, exp);
	friendnum++;
   	fp=fopen(path, "w");
   	fwrite(fff, sizeof(struct override), friendnum, fp);
   	fclose(fp);
   	printf("[%s]已加入您的好友名单.<br><br>\n <a href=bbsfall>返回好友名单</a>", userid);
	http_quit();
}
