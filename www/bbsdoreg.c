#include "libweb.h"

int is_bad_id(char *s) {
	FILE *fp;
	char buf[256], buf2[256];
	fp=fopen(".badname", "r");
	if(fp==0) return 0;
	while(1) {
		if(fgets(buf, 250, fp)==0) break;
		if(sscanf(buf, "%s", buf2)!=1) continue;
		if(strcasestr(s, buf2)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int badymd(int y, int m, int d) {
	int max[]={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if((y%4==0 && y%100!=0) || y%400==0) max[2]=29;
	if(y<10 || y>100 || m<1 || m>12) return 1;
	if(d<0 || d>max[m]) return 1;
	return 0;
}

int main() {
   	FILE *fp;
	struct userec x;
	int i, gender, xz;
	char buf[80], filename[80], pass1[80], pass2[80], dept[80], phone[80], salt[80], words[1024];
   	init_all();
 	bzero(&x, sizeof(x));
	xz=atoi(getparm("xz"));
  	strsncpy(x.userid, getparm("userid"), 13);
   	strsncpy(pass1, getparm("pass1"), 13);
   	strsncpy(pass2, getparm("pass2"), 13);
   	strsncpy(x.username, getparm("username"), 32);
   	strsncpy(x.realname, getparm("realname"), 32);
   	strsncpy(dept, getparm("dept"), 32);
   	strsncpy(x.address, getparm("address"), 32);
   	strsncpy(x.email, getparm("email"), 32);
   	strsncpy(phone, getparm("phone"), 32);
	strsncpy(words, getparm("words"), 1000);
	x.gender='M';
	if(atoi(getparm("gender"))) x.gender='F';
	x.birthyear=atoi(getparm("year"))-1900;
	x.birthmonth=atoi(getparm("month"));
	x.birthday=atoi(getparm("day"));
   	for(i=0; x.userid[i]; i++)
      		if(!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", toupper(x.userid[i]))) http_fatal("帐号只能由英文字母组成");
   	if(strlen(x.userid)<2) http_fatal("帐号长度太短(2-12字符)");
   	if(strlen(pass1)<4) http_fatal("密码太短(至少4字符)");
   	if(strcmp(pass1, pass2)) http_fatal("两次输入的密码不一致, 请确认密码");
   	if(strlen(x.username)<2) http_fatal("请输入昵称(昵称长度至少2个字符)");
   	if(strlen(x.realname)<4) http_fatal("请输入真实姓名(请用中文, 至少2个字)");
   	if(strlen(dept)<6) http_fatal("工作单位的名称长度至少要6个字符(或3个汉字)");
   	if(strlen(x.address)<6) http_fatal("通讯地址长度至少要6个字符(或3个汉字)");
   	if(badstr(x.passwd)||badstr(x.username)||badstr(x.realname)) http_fatal("您的注册单中含有非法字符");
	if(badstr(dept)||badstr(x.address)||badstr(x.email)||badstr(phone)) http_fatal("您的注册单中含有非法字符");
   	if(badymd(x.birthyear, x.birthmonth, x.birthday)) http_fatal("请输入您的出生年月");
 	if(is_bad_id(x.userid)) http_fatal("不雅帐号或禁止注册的id, 请重新选择");
   	if(getuser(x.userid)) http_fatal("此帐号已经有人使用,请重新选择。");
   	sprintf(salt, "%c%c", 65+rand()%26, 65+rand()%26);
   	strsncpy(x.passwd, crypt1(pass1, salt), 14);
   	strcpy(x.termtype, "vt100");
   	strcpy(x.lasthost, fromhost);
   	x.userlevel=PERM_BASIC;
   	x.firstlogin=time(0);
	x.lastlogin=time(0);
   	x.userdefine=-1;
   	x.flags[0]=CURSOR_FLAG | PAGER_FLAG;
	if(xz==1) currentuser.userdefine ^= DEF_COLOREDSEX;
	if(xz==2) currentuser.userdefine ^= DEF_S_HOROSCOPE;
	adduser(&x);
   	fp=fopen("new_register", "a");
	if(fp) {
      		fprintf(fp, "usernum: %d, %s\n", 	getusernum(x.userid)+1, Ctime(time(0)));
      		fprintf(fp, "userid: %s\n",    	x.userid);
      		fprintf(fp, "realname: %s\n",  	x.realname);
      		fprintf(fp, "dept: %s\n",    	dept);
      		fprintf(fp, "addr: %s\n",      	x.address);
      		fprintf(fp, "phone: %s\n",     	phone );
//      		fprintf(fp, "assoc:\n");
      		fprintf(fp, "----\n" );
      		fclose(fp);
   	}
   	f_append("trace.post", "G");
   	sprintf(filename, "home/%c/%s", toupper(x.userid[0]), x.userid);
   	mkdir(filename, 0755);
   	printf("<center>\n");
	printpretable();
	printf("<table><td><td><pre>\n");
	printf("亲爱的新使用者，您好！\n\n");
        printf("欢迎光临 本站, 您的新帐号已经成功被登记了。\n");
        printf("您目前拥有本站基本的权限, 包括阅读文章、环顾四方、接收私人\n");
	printf("信件、接收他人的消息、进入聊天室等等。当您通过本站的身份确\n");
	printf("认手续之后，您还会获得更多的权限。目前您的注册单已经被提交\n");
	printf("等待审阅。一般情况24小时以内就会有答复，请耐心等待。同时请\n");
	printf("留意您的站内信箱。\n");
	printf("如果您有任何疑问，可以去sysop(站长的工作室)版发文求助。\n\n</pre></table>");
   	printf("<br>您的基本资料如下:<br>\n");
   	printf("<table border=1 width=400>");
   	printf("<tr><td>帐号位置: <td>%d\n", getusernum(x.userid));
   	printf("<tr><td>使用者代号: <td>%s (%s)\n", x.userid, x.username);
   	printf("<tr><td>姓  名: <td>%s<br>\n", x.realname);
	printf("<tr><td>昵  称: <td>%s<br>\n", x.username);
   	printf("<tr><td>上站位置: <td>%s<br>\n", x.lasthost);
   	printf("<tr><td>电子邮件: <td>%s<br></table><br>\n", x.email);
	printposttable();
   	printf("<center><input type=button onclick='window.close()' value=关闭本窗口></center>\n");
   	newcomer(&x, words);
   	//sprintf(buf, "%s %-12s %d\n", Ctime(time(0))+4, x.userid, getusernum(x.userid));
   	//f_append("wwwreg.log", buf);
}

int badstr(unsigned char *s) {
  	int i;
	for(i=0; s[i]; i++)
    		if(s[i]!=9 &&(s[i]<32 || s[i]==255)) return 1;
  	return 0;
}

int newcomer(struct userec *x, char *words) {
  	FILE *fp;
  	char filename[80];
	sprintf(filename, "tmp/%d.tmp", getpid());
	fp=fopen(filename, "w");
	fprintf(fp, "大家好, \n\n");
	fprintf(fp, "我是 %s(%s), 来自 %s\n", x->userid, x->username, fromhost);
	fprintf(fp, "今天初来此地报到, 请大家多多指教.\n\n");
	fprintf(fp, "自我介绍:\n\n");
	fprintf(fp, "%s", words);
	fclose(fp);
	post_article("newcomers", "WWW新手上路", filename, x->userid, x->username, fromhost, -1, -1, -1);
	unlink(filename);
}

void
setuserid(num, userid)
int     num;
char   *userid;
{
	if (num > 0 && num <= MAXUSERS) {
		if (num > shm_ucache->number)
			shm_ucache->number = num;
		strncpy(shm_ucache->userid[num - 1], userid, IDLEN + 1);
              /* hash 填充 */
      if( strcmp(userid, "new") ){
              char a1,a2;
              int key;

              key = uhashkey (userid, &a1, &a2);

              if( shm_ucache->hash[a1][a2][key] == 0 ){
                      shm_ucache->hash[a1][a2][key] = num;
              }else{
                      int i;
                      for(i=shm_ucache->hash[a1][a2][key]; shm_ucache->next[i-1]; i=shm_ucache->next[i-1]);
                      shm_ucache->next[i-1] = num;
                      shm_ucache->prev[num-1] = i;
              }
      }
              /* end of hash 填充 */
	}
}

int
searchnewuser()
{
	int num, i;
	num = shm_ucache->number;
	for (i = 0; i < num; i++)
		if (shm_ucache->userid[i][0] == '\0')
			return i + 1;
	if (num < MAXUSERS)
		return (num + 1);
	return 0;
}

int adduser(struct userec *x) {
	int i;

	i = searchnewuser();
	if( i<= 0)
		return -1;
	setuserid(i, x->userid)
	save_user_data(x);
}
