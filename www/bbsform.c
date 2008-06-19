#include "BBSLIB.inc"

int main() {
  	FILE *fp;
	int type;
	init_all();
	printf("<b>填写注册单 · %s</b><br>\n", BBSNAME);
	printpretable_lite();
	type=atoi(getparm("type"));
  	if(!loginok) http_fatal("您尚未登录, 请重新登录。");
	check_if_ok();
	if(type==1) {
		check_submit_form();
		http_quit();
	}
  	printf("您好, %s, 注册单通过后即可获得注册用户的权限, 下面各项务必请认真填写<br><hr>\n", currentuser.userid);
  	printf("<form method=post action=bbsform?type=1>\n");
  	printf("真实姓名: <input name=realname type=text maxlength=8 size=8 value='%s'><br>\n", 
		nohtml(currentuser.realname));
  	printf("学校系级: <input name=dept type=text maxlength=32 size=32 value='%s'>(或工作单位)<br>\n", 
		nohtml(currentuser.reginfo));
  	printf("居住地址: <input name=address type=text maxlength=32 size=32 value='%s'><br>\n", 
		nohtml(currentuser.address));
  	printf("联络电话: <input name=phone type=text maxlength=32 size=32>(没有可写'无')<br><hr><br>\n");
  	printf("<input type=submit> <input type=reset>");
	http_quit();
}

int check_if_ok() {
  	if(user_perm(&currentuser, PERM_REGISTER)) http_fatal("您已经通过本站的身份认证, 无需再次填写注册单.");
  	if(has_fill_form()) http_fatal("目前站长尚未处理您的注册单，请耐心等待.");
}

int check_submit_form() {
	FILE *fp;
  	fp=fopen("new_register", "a");
	if(fp==0) http_fatal("注册文件错误，请通知SYSOP");
  	fprintf(fp, "usernum: %d, %s\n", getusernum(currentuser.userid)+1, Ctime(time(0)));
  	fprintf(fp, "userid: %s\n", currentuser.userid);
  	fprintf(fp, "realname: %s\n", getparm("realname"));
  	fprintf(fp, "dept: %s\n", getparm("dept"));
  	fprintf(fp, "addr: %s\n", getparm("address"));
  	fprintf(fp, "phone: %s\n", getparm("phone"));
  	fprintf(fp, "----\n" );
  	fclose(fp);
  	printf("您的注册单已成功提交. 站长检验过后会给您发信, 请留意您的信箱.");
}
