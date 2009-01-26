#include "libweb.h"

int main() {
	int type;
  	char pw1[20], pw2[20], pw3[20];
	init_all();
	if(!loginok) http_fatal("您尚未登录, 请先登录");
	type=atoi(getparm("type"));
	if(type==0) {
		printf("<b>%s -- 修改密码 [用户: %s]</b>\n", BBSNAME, currentuser.userid);
		printpretable_lite();
		printf("<form action=bbspwd?type=1 method=post>\n");
		printf("您的旧密码: <input maxlength=12 size=12 type=password name=pw1><br>\n");
		printf("您的新密码: <input maxlength=12 size=12 type=password name=pw2><br>\n");
		printf("再输入一次: <input maxlength=12 size=12 type=password name=pw3><br><br>\n");
		printf("<input type=submit value=确定修改>\n");
		printposttable_lite();
		http_quit();
	}
  	strlcpy(pw1, getparm("pw1"), 13);
  	strlcpy(pw2, getparm("pw2"), 13);
  	strlcpy(pw3, getparm("pw3"), 13);
  	if(strcmp(pw2, pw3)) http_fatal("两次密码不相同");
  	if(strlen(pw2)<2) http_fatal("新密码太短");
  	if(!checkpasswd(currentuser.passwd, pw1)) http_fatal("密码不正确");
  	strcpy(currentuser.passwd, crypt(pw2, pw2));
  	save_user_data(&currentuser);
  	printf("[%s] 密码修改成功.", currentuser.userid);
}

