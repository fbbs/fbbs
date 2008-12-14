#define BBSFOOT
#include "libweb.h"

int main() {
	int dt=0, mail_total, mail_unread;
	char *id="guest";
	init_all();
	printf("<style type=text/css>\nA{color: #000000}\n</style>\n");
  	printf("<body bgcolor=#eeeeee marginwidth=0 marginheight=0>\n");
	if(!loginok)
		printf("<form action=bbslogin method=post target=_top>");
	else
	{	
		 id=currentuser.userid;
		 mail_total=mails(id, 0);
		mail_unread=mails(id, 1);
		if(mail_unread==1) {
			printf("<bgsound src=/mail.wav>\n");
		}
	}
	printf("<table border=0 cellspacing=0 padding=0 width=100%%><tr><td nowrap>");
	
if(!loginok)
{
	printf("用户<input name=id type=text maxlength=12 size=10 class='THINBLUE'>&nbsp;");
	printf("密码<input name=pw type=password maxlength=12 size=10 class='THINBLUE'>&nbsp;");
	printf("<input border=0 src=/images/login.gif type=image align=absmiddle>");
}
else
{
        printf("<img border=0 align=absmiddle src=/images/user_%c.gif><a href=bbsqry?userid=%s target=view><b>%s</b></a>&nbsp;", currentuser.gender=='F'?'F':'M', currentuser.userid, currentuser.userid);
        iconexp(countexp(&currentuser));
		 if (!HAS_PERM(PERM_BINDMAIL))
		 {
				 printf("<font color='#ff0000'>您尚未绑定邮箱,请尽快用telnet登陆本站填写邮箱进行绑定认证</font>");
		 }
#ifdef SPARC
		dt=abs(time(0) - *(int*)(u_info->from+30))/60;
#else
		dt=abs(time(0) - *(int*)(u_info->from+32))/60;
#endif
                u_info->idle_time=time(0);
}
	
	
	printf("</td><td nowrap width=100%% ALIGN=RIGHT>");
  	//printf("时间[%16.16s </a>] ", Ctime(time(0)));
	//modified by iamfat 2002.08.01
  	printf("<img border=0 src=/images/clock.gif align=absmiddle>[%s </a>] ", cn_Ctime(time(0)));
	printf("<img border=0 src=/images/users.gif align=absmiddle>[<a href=bbsusr target=view>%d</a>] ", count_online());
	if(loginok) {
		//mail_total=mails(id, 0);
		//mail_unread=mails(id, 1);
		if(mail_unread==0) {
			printf("<img border=0 src=/images/mailw.gif align=absmiddle>[<a href=bbsmail target=view>%d封</a>] ", mail_total);
		} else {
			printf("<img border=0 src=/images/mailw.gif align=absmiddle>[<a href=bbsmail target=view>%d(新信<font color=red>%d</font>)</a>] ", 
				mail_total, mail_unread);
		}
	}
	printf("<img border=0 src=/images/water.gif align=absmiddle>[%d小时%d分]", dt/60, dt%60);
  	printf("</td></tr></table>");
	if(!loginok)printf("</form>");
	printf("<script>setTimeout('self.location=self.location', 240000);</script>");
	printf("</body>");
}
	
int mails(char *id, int unread_only) {
        struct fileheader x;
        char path[80];
	int total=0, unread=0;
        FILE *fp;
	if(!loginok) return 0;
        sprintf(path, "mail/%c/%s/.DIR", toupper(id[0]), id);
        fp=fopen(path, "r");
        if(fp==0) return 0;
        while(fread(&x, sizeof(x), 1, fp)>0) {
                total++;
                if(!(x.accessed[0] & FILE_READ)) unread++;
        }
        fclose(fp);
	if(unread_only) return unread;
	return total;
}

