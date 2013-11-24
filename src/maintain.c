#include "bbs.h"
#include "record.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/terminal.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

char    cexplain[STRLEN];
char    lookgrp[30];
FILE   *cleanlog;

//保存用户近期信息
static int getuinfo(FILE *fn)
{
	int num;
	char buf[40];
	//% fprintf(fn, "\n\n他的代号     : %s\n", currentuser.userid);
	fprintf(fn, "\n\n\xcb\xfb\xb5\xc4\xb4\xfa\xba\xc5     : %s\n", currentuser.userid);
	//% fprintf(fn, "他的昵称     : %s\n", currentuser.username);
	fprintf(fn, "\xcb\xfb\xb5\xc4\xea\xc7\xb3\xc6     : %s\n", currentuser.username);
	//% fprintf(fn, "电子邮件信箱 : %s\n", currentuser.email);
	fprintf(fn, "\xb5\xe7\xd7\xd3\xd3\xca\xbc\xfe\xd0\xc5\xcf\xe4 : %s\n", currentuser.email);
	//% fprintf(fn, "帐号建立日期 : %s\n", format_time(currentuser.firstlogin, TIME_FORMAT_ZH));
	fprintf(fn, "\xd5\xca\xba\xc5\xbd\xa8\xc1\xa2\xc8\xd5\xc6\xda : %s\n", format_time(currentuser.firstlogin, TIME_FORMAT_ZH));
	//% fprintf(fn, "最近光临日期 : %s\n", format_time(currentuser.lastlogin, TIME_FORMAT_ZH));
	fprintf(fn, "\xd7\xee\xbd\xfc\xb9\xe2\xc1\xd9\xc8\xd5\xc6\xda : %s\n", format_time(currentuser.lastlogin, TIME_FORMAT_ZH));
	//% fprintf(fn, "最近光临机器 : %s\n", currentuser.lasthost);
	fprintf(fn, "\xd7\xee\xbd\xfc\xb9\xe2\xc1\xd9\xbb\xfa\xc6\xf7 : %s\n", currentuser.lasthost);
	//% fprintf(fn, "上站次数     : %d 次\n", currentuser.numlogins);
	fprintf(fn, "\xc9\xcf\xd5\xbe\xb4\xce\xca\xfd     : %d \xb4\xce\n", currentuser.numlogins);
	//% fprintf(fn, "文章数目     : %d\n", currentuser.numposts);
	fprintf(fn, "\xce\xc4\xd5\xc2\xca\xfd\xc4\xbf     : %d\n", currentuser.numposts);
	//% fprintf(fn, "上站总时数   : %d 小时 %d 分钟\n", currentuser.stay / 3600,
	fprintf(fn, "\xc9\xcf\xd5\xbe\xd7\xdc\xca\xb1\xca\xfd   : %d \xd0\xa1\xca\xb1 %d \xb7\xd6\xd6\xd3\n", currentuser.stay / 3600,
			(currentuser.stay / 60) % 60);
	strcpy(buf, "ltmprbBOCAMURS#@XLEast0123456789");
	for (num = 0; num < 30; num++)
		if (!(currentuser.userlevel & (1 << num)))
			buf[num] = '-';
	buf[num] = '\0';
	//% fprintf(fn, "使用者权限   : %s\n\n", buf);
	fprintf(fn, "\xca\xb9\xd3\xc3\xd5\xdf\xc8\xa8\xcf\xde   : %s\n\n", buf);
	return 0;
}

//	系统安全记录,自动发送到syssecurity版
//  mode == 0		syssecurity
//	mode == 1		boardsecurity
//  mode == 2		bmsecurity
//  mode == 3		usersecurity
void securityreport(char *str, int save, int mode)
{
	FILE*	se;
	char    fname[STRLEN];
	int     savemode;
	savemode = session_status();
	report(str, currentuser.userid);
	file_temp_name(fname, sizeof(fname));
	if ((se = fopen(fname, "w")) != NULL) {
		//% fprintf(se, "系统安全记录\n[1m原因：%s[m\n", str);
		fprintf(se, "\xcf\xb5\xcd\xb3\xb0\xb2\xc8\xab\xbc\xc7\xc2\xbc\n[1m\xd4\xad\xd2\xf2\xa3\xba%s[m\n", str);
		if (save){
			//% fprintf(se, "以下是个人资料:");
			fprintf(se, "\xd2\xd4\xcf\xc2\xca\xc7\xb8\xf6\xc8\xcb\xd7\xca\xc1\xcf:");
			getuinfo(se);
		}
		fclose(se);
		if (mode == 0){
			Postfile(fname, "syssecurity", str, 2);
		} else if (mode == 1){
			Postfile(fname, "boardsecurity", str, 2);
		} else if (mode == 2){
		    Postfile(fname, "bmsecurity", str, 2);
		} else if (mode == 3){
		    Postfile(fname, "usersecurity", str, 2);
		} else if (mode == 4){
		    Postfile(fname, "vote", str, 2);
		}
		unlink(fname);
		set_user_status(savemode);
	}
}

//	核对系统密码
int	check_systempasswd(void)
{
	FILE*	pass;
	char    passbuf[20], prepass[STRLEN];
	screen_clear();
	if ((pass = fopen("etc/.syspasswd", "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen(prepass) - 1] = '\0';
		//% getdata(1, 0, "请输入系统密码: ", passbuf, 19, NOECHO, YEA);
		getdata(1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!passwd_match(prepass, passbuf)) {
			move(2, 0);
			//% prints("错误的系统密码...");
			prints("\xb4\xed\xce\xf3\xb5\xc4\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb...");
			//% securityreport("系统密码输入错误...", 0, 0);
			securityreport("\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb\xca\xe4\xc8\xeb\xb4\xed\xce\xf3...", 0, 0);
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

//	自动发送到版面
//			title		标题
//			str			内容
//			uname		发送到的用户名,为null则不发送.
void autoreport(const char *board, const char *title, const char *str,
		const char *uname, int mode)
{
    report(title, currentuser.userid);

	if (uname)
		do_mail_file(uname, title, NULL, str, strlen(str), NULL);

	if (board)
		Poststring(str, board, title, mode);
}

char    curruser[IDLEN + 2];
extern int delmsgs[];
extern int delcnt;

void
domailclean(fhdrp)
struct fileheader *fhdrp;
{
	static int newcnt, savecnt, deleted, idc;
	char    buf[STRLEN];
	if (fhdrp == NULL) {
		fprintf(cleanlog, "new = %d, saved = %d, deleted = %d\n",
			newcnt, savecnt, deleted);
		newcnt = savecnt = deleted = idc = 0;
		if (delcnt) {
			sprintf(buf, "mail/%c/%s/%s", toupper(curruser[0]), curruser, DOT_DIR);
			while (delcnt--)
				delete_record(buf, sizeof(struct fileheader), delmsgs[delcnt],NULL,NULL);
		}
		delcnt = 0;
		return;
	}
	idc++;
	if (!(fhdrp->accessed[0] & FILE_READ))
		newcnt++;
	else if (fhdrp->accessed[0] & FILE_MARKED)
		savecnt++;
	else {
		deleted++;
		sprintf(buf, "mail/%c/%s/%s", toupper(curruser[0]), curruser, fhdrp->filename);
		unlink(buf);
		delmsgs[delcnt++] = idc;
	}
}
