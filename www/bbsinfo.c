#include "BBSLIB.inc"

int main() {
	int n, type;
  	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	/* add end */

	if(!loginok) http_fatal("您尚未登录");
	type=atoi(getparm("type"));
	printf("<b>用户个人资料 · %s </b><br>\n", BBSNAME);
	printpretable_lite();
	if(type!=0) {
		check_info();
		http_quit();
	}
 	printf("<form action=bbsinfo?type=1 method=post>");
  	printf("您的帐号: %s<br>\n", currentuser.userid);
  	printf("您的昵称: <input type=text name=nick value='%s' size=24 maxlength=30><br>\n",
		currentuser.username);
  	printf("发表大作: %d 篇<br>\n", currentuser.numposts);
  	printf("信件数量: %d 封<br>\n", currentuser.nummails);
  	printf("上站次数: %d 次<br>\n", currentuser.numlogins);
  	printf("上站时间: %d 分钟<br>\n", currentuser.stay/60);
  	printf("真实姓名: <input type=text name=realname value='%s' size=16 maxlength=16><br>\n",
	 	currentuser.realname);
  	printf("居住地址: <input type=text name=address value='%s' size=40 maxlength=40><br>\n",
 		currentuser.address);
//  	printf("帐号建立: %s<br>", Ctime(currentuser.firstlogin));
//  	printf("最近光临: %s<br>", Ctime(currentuser.lastlogin));
//modified by iamfat 2002.08.01
  	printf("帐号建立: %s<br>", cn_Ctime(currentuser.firstlogin));
  	printf("最近光临: %s<br>", cn_Ctime(currentuser.lastlogin));
  	printf("来源地址: %s<br>", currentuser.lasthost);
//  	printf("电子邮件: <input type=text name=email value='%s' size=32 maxlength=32><br>\n", 
//		currentuser.email);
  	printf("出生日期: <input type=text name=year value=%d size=4 maxlength=4>年", 
		currentuser.birthyear+1900);
  	printf("<input type=text name=month value=%d size=2 maxlength=2>月", 
		currentuser.birthmonth);
  	printf("<input type=text name=day value=%d size=2 maxlength=2>日<br>\n", 
		currentuser.birthday);
  	printf("用户性别: ");
    	printf("男<input type=radio value=M name=gender %s>", 
		currentuser.gender=='M' ? "checked" : "");
    	printf("女<input type=radio value=F name=gender %s><br>",
		currentuser.gender=='F' ? "checked" : "");
  	printf("<br><input type=submit value=确定>   <input type=reset value=复原>\n");
  	printf("</form>");
	printposttable_lite();
	http_quit();
}

int check_info() {
  	int m, n;
  	char buf[256];
    	strsncpy(buf, getparm("nick"), 30);
    	for(m=0; m<strlen(buf); m++) if(buf[m]<32 && buf[m]>0 || buf[m]==-1) buf[m]=' ';
    	if(strlen(buf)>1) {
		strcpy(currentuser.username, buf);
	} else {
		printf("警告: 昵称太短!<br>\n");
	}
    	strsncpy(buf, getparm("realname"), 9);
    	if(strlen(buf)>1) {
		strcpy(currentuser.realname, buf); 
	} else {
		printf("警告: 真实姓名太短!<br>\n");
	}
    	strsncpy(buf, getparm("address"), 40);
    	if(strlen(buf)>8) {
		strcpy(currentuser.address, buf);
	} else {
		printf("警告: 居住地址太短!<br>\n");
	}
    	strsncpy(buf, getparm("year"), 5);
    	if(atoi(buf)>1910 && atoi(buf)<1998) {
		currentuser.birthyear=atoi(buf)-1900;
	} else {
		printf("警告: 错误的出生年份!<br>\n");
	}
    	strsncpy(buf, getparm("month"), 3);
    	if(atoi(buf)>0 && atoi(buf)<=12) {
		currentuser.birthmonth=atoi(buf);
	} else {
		printf("警告: 错误的出生月份!<br>\n");
	}
    	strsncpy(buf, getparm("day"), 3);
    	if(atoi(buf)>0 && atoi(buf)<=31) {
		currentuser.birthday=atoi(buf);
	} else {
		printf("警告: 错误的出生日期!<br>\n");
	}
    	strsncpy(buf, getparm("gender"), 2);
    	if(!strcasecmp(buf, "F")) currentuser.gender='F';
    	if(!strcasecmp(buf, "M")) currentuser.gender='M';
    	save_user_data(&currentuser);
    	printf("[%s] 个人资料修改成功.", currentuser.userid);
}
