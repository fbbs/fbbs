#include "bbs.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/uinfo.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15

extern time_t login_start_time;
extern char fromhost[60];

//将ptr指向的字符串中字符值为0xFF的转换成空格
void filter_ff(char *ptr) {
	while (*ptr) {
		if (*(unsigned char *)ptr == 0xff)
			*ptr = ' ';
		ptr++;
	}
	return;
}

void disply_userinfo(const struct userec *u)
{
	int num, exp;
	time_t now;

	move(2, 0);
	screen_clrtobot();
	now = time(0);
	set_safe_record();
	//% prints("您的代号     : %-14s", u->userid);
	prints("\xc4\xfa\xb5\xc4\xb4\xfa\xba\xc5     : %-14s", u->userid);
	//% prints("昵称 : %-20s", u->username);
	prints("\xea\xc7\xb3\xc6 : %-20s", u->username);
	//% prints("     性别 : %s\n", (u->gender == 'M' ? "男" : "女"));
	prints("     \xd0\xd4\xb1\xf0 : %s\n", (u->gender == 'M' ? "\xc4\xd0" : "\xc5\xae"));
	//% prints("出生日期     : %d年%d月%d日", u->birthyear + 1900, u->birthmonth,
	prints("\xb3\xf6\xc9\xfa\xc8\xd5\xc6\xda     : %d\xc4\xea%d\xd4\xc2%d\xc8\xd5", u->birthyear + 1900, u->birthmonth,
			u->birthday);
	//% prints(" (累计生活天数 : %d)\n", days_elapsed(u->birthyear + 1900, 
	prints(" (\xc0\xdb\xbc\xc6\xc9\xfa\xbb\xee\xcc\xec\xca\xfd : %d)\n", days_elapsed(u->birthyear + 1900, 
			u->birthmonth, u->birthday, now));

#ifndef ENABLE_FDQUAN
	db_res_t *res = db_query("SELECT e.addr FROM alive_users u"
			" JOIN emails e ON u.email = e.id"
			" WHERE lower(u.name) = lower(%s) ", u->userid);
	if (res && db_res_rows(res) == 1)
		//% prints("电子邮件信箱 : %s\n", db_get_value(res, 0, 0));
		prints("\xb5\xe7\xd7\xd3\xd3\xca\xbc\xfe\xd0\xc5\xcf\xe4 : %s\n", db_get_value(res, 0, 0));
	db_clear(res);
#endif

	//% prints("最近光临机器 : %-22s\n", u->lasthost);
	prints("\xd7\xee\xbd\xfc\xb9\xe2\xc1\xd9\xbb\xfa\xc6\xf7 : %-22s\n", u->lasthost);
	//% prints("帐号建立日期 : %s[距今 %d 天]\n",
	prints("\xd5\xca\xba\xc5\xbd\xa8\xc1\xa2\xc8\xd5\xc6\xda : %s[\xbe\xe0\xbd\xf1 %d \xcc\xec]\n",
			format_time(u->firstlogin, TIME_FORMAT_ZH),
			(now - (u->firstlogin)) / 86400);
	format_time(u->lastlogin, NA);
	//% prints("最近光临日期 : %s[距今 %d 天]\n",
	prints("\xd7\xee\xbd\xfc\xb9\xe2\xc1\xd9\xc8\xd5\xc6\xda : %s[\xbe\xe0\xbd\xf1 %d \xcc\xec]\n",
			format_time(u->lastlogin, TIME_FORMAT_ZH),
			(now-(u->lastlogin)) / 86400);
#ifdef ALLOWGAME
	//% prints("文章数目     : %-20d 奖章数目 : %d\n",u->numposts,u->nummedals);
	prints("\xce\xc4\xd5\xc2\xca\xfd\xc4\xbf     : %-20d \xbd\xb1\xd5\xc2\xca\xfd\xc4\xbf : %d\n",u->numposts,u->nummedals);
	//% prints("私人信箱     : %d 封\n", u->nummails);
	prints("\xcb\xbd\xc8\xcb\xd0\xc5\xcf\xe4     : %d \xb7\xe2\n", u->nummails);
	//% prints("您的银行存款 : %d元  贷款 : %d元 (%s)\n",
	prints("\xc4\xfa\xb5\xc4\xd2\xf8\xd0\xd0\xb4\xe6\xbf\xee : %d\xd4\xaa  \xb4\xfb\xbf\xee : %d\xd4\xaa (%s)\n",
			u->money,u->bet,cmoney(u->money-u->bet));
#else
	//% prints("文章数目     : %-20d \n", u->numposts);
	prints("\xce\xc4\xd5\xc2\xca\xfd\xc4\xbf     : %-20d \n", u->numposts);
	//% prints("私人信箱     : %d 封 \n", u->nummails);
	prints("\xcb\xbd\xc8\xcb\xd0\xc5\xcf\xe4     : %d \xb7\xe2 \n", u->nummails);
#endif
	//% prints("上站次数     : %d 次      ", u->numlogins);
	prints("\xc9\xcf\xd5\xbe\xb4\xce\xca\xfd     : %d \xb4\xce      ", u->numlogins);
	//% prints("上站总时数   : %d 小时 %d 分钟\n", u->stay/3600, (u->stay/60)%60);
	prints("\xc9\xcf\xd5\xbe\xd7\xdc\xca\xb1\xca\xfd   : %d \xd0\xa1\xca\xb1 %d \xb7\xd6\xd6\xd3\n", u->stay/3600, (u->stay/60)%60);
	exp = countexp(u);
	//modified by iamfat 2002.07.25
#ifdef SHOWEXP
	//% prints("经验值       : %d  (%-10s)    ", exp, cexpstr(exp));
	prints("\xbe\xad\xd1\xe9\xd6\xb5       : %d  (%-10s)    ", exp, cexpstr(exp));
#else
	//% prints("经验值       : [%-10s]     ", cexpstr(exp));
	prints("\xbe\xad\xd1\xe9\xd6\xb5       : [%-10s]     ", cexpstr(exp));
#endif
	exp = countperf(u);
#ifdef SHOWPERF
	//% prints("表现值 : %d  (%s)\n", exp, cperf(exp));
	prints("\xb1\xed\xcf\xd6\xd6\xb5 : %d  (%s)\n", exp, cperf(exp));
#else
	//% prints("表现值  : [%s]\n", cperf(exp));
	prints("\xb1\xed\xcf\xd6\xd6\xb5  : [%s]\n", cperf(exp));
#endif
	strcpy(genbuf, "ltmprbBOCAMURS#@XLEast0123456789\0");
	for (num = 0; num < strlen(genbuf) ; num++)
		if (!(u->userlevel & (1 << num))) //相应权限为空,则置'-'
			genbuf[num] = '-';
	//% prints("使用者权限   : %s\n", genbuf);
	prints("\xca\xb9\xd3\xc3\xd5\xdf\xc8\xa8\xcf\xde   : %s\n", genbuf);
	prints("\n");
	if (u->userlevel & PERM_SYSOPS) {
		//% prints("  您是本站的站长, 感谢您的辛勤劳动.\n");
		prints("  \xc4\xfa\xca\xc7\xb1\xbe\xd5\xbe\xb5\xc4\xd5\xbe\xb3\xa4, \xb8\xd0\xd0\xbb\xc4\xfa\xb5\xc4\xd0\xc1\xc7\xda\xc0\xcd\xb6\xaf.\n");
	} else if (u->userlevel & PERM_BOARDS) {
		//% prints("  您是本站的版主, 感谢您的付出.\n");
		prints("  \xc4\xfa\xca\xc7\xb1\xbe\xd5\xbe\xb5\xc4\xb0\xe6\xd6\xf7, \xb8\xd0\xd0\xbb\xc4\xfa\xb5\xc4\xb8\xb6\xb3\xf6.\n");
	} else if (u->userlevel & PERM_REGISTER) {
		//% prints("  您的注册程序已经完成, 欢迎加入本站.\n");
		prints("  \xc4\xfa\xb5\xc4\xd7\xa2\xb2\xe1\xb3\xcc\xd0\xf2\xd2\xd1\xbe\xad\xcd\xea\xb3\xc9, \xbb\xb6\xd3\xad\xbc\xd3\xc8\xeb\xb1\xbe\xd5\xbe.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		//% prints("  新手上路, 请阅读 Announce 讨论区.\n");
		prints("  \xd0\xc2\xca\xd6\xc9\xcf\xc2\xb7, \xc7\xeb\xd4\xc4\xb6\xc1 Announce \xcc\xd6\xc2\xdb\xc7\xf8.\n");
	} else {
		//% prints("  注册尚未成功, 请参考本站进站画面说明.\n");
		prints("  \xd7\xa2\xb2\xe1\xc9\xd0\xce\xb4\xb3\xc9\xb9\xa6, \xc7\xeb\xb2\xce\xbf\xbc\xb1\xbe\xd5\xbe\xbd\xf8\xd5\xbe\xbb\xad\xc3\xe6\xcb\xb5\xc3\xf7.\n");
	}
}

//	改变用户记录,u为以前的记录,newinfo为新记录,后两个参数均为指针
//		i为所显示的行
void uinfo_change1(int i, struct userec *u, struct userec *newinfo) {
	char buf[STRLEN], genbuf[128];

	//% sprintf(genbuf, "上线次数 [%d]: ", u->numlogins);
	sprintf(genbuf, "\xc9\xcf\xcf\xdf\xb4\xce\xca\xfd [%d]: ", u->numlogins);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) > 0)
		newinfo->numlogins = atoi(buf);

	//% sprintf(genbuf, "发表文章数 [%d]: ", u->numposts);
	sprintf(genbuf, "\xb7\xa2\xb1\xed\xce\xc4\xd5\xc2\xca\xfd [%d]: ", u->numposts);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) >0)
		newinfo->numposts = atoi(buf);

	//% sprintf(genbuf, "登陆总时间 [%d]: ", u->stay);
	sprintf(genbuf, "\xb5\xc7\xc2\xbd\xd7\xdc\xca\xb1\xbc\xe4 [%d]: ", u->stay);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) > 0)
		newinfo->stay = atoi(buf);
	sprintf(genbuf, "firstlogin [%"PRIdFBT"]: ", u->firstlogin);
	getdata(i++, 0, genbuf, buf, 15, DOECHO, YEA);
	if (atoi(buf) >0)
		newinfo->firstlogin = atoi(buf);
	//add end          				      	      	
#ifdef ALLOWGAME
	//% sprintf(genbuf, "银行存款 [%d]: ", u->money);
	sprintf(genbuf, "\xd2\xf8\xd0\xd0\xb4\xe6\xbf\xee [%d]: ", u->money);
	getdata(i++, 0, genbuf, buf, 8, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->money = atoi(buf);

	//% sprintf(genbuf, "银行贷款 [%d]: ", u->bet);
	sprintf(genbuf, "\xd2\xf8\xd0\xd0\xb4\xfb\xbf\xee [%d]: ", u->bet);
	getdata(i++, 0, genbuf, buf, 8, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->bet = atoi(buf);

	//% sprintf(genbuf, "奖章数 [%d]: ", u->nummedals);
	sprintf(genbuf, "\xbd\xb1\xd5\xc2\xca\xfd [%d]: ", u->nummedals);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->nummedals = atoi(buf);
#endif
}

void tui_check_uinfo(struct userec *u)
{
	char ans[5];
	bool finish = false;

	while (!finish) {
		switch (check_user_profile(u)) {
			case UINFO_ENICK:
				//% getdata(2, 0, "请输入您的昵称 (Enter nickname): ",
				getdata(2, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xea\xc7\xb3\xc6 (Enter nickname): ",
						u->username, NAMELEN, DOECHO, YEA);
				string_remove_non_printable_gbk(u->username);
				break;
			case UINFO_EGENDER:
				//% getdata(3, 0, "请输入您的性别: M.男 F.女 [M]: ",
				getdata(3, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xd0\xd4\xb1\xf0: M.\xc4\xd0 F.\xc5\xae [M]: ",
						ans, 2, DOECHO, YEA);
				if (ans[0] != 'F' && ans[0] != 'f')
					u->gender = 'M';
				else
					u->gender = 'F';
				break;
			case UINFO_EBIRTH:
				//% getdata(4, 0, "请输入您的生日年份(四位数): ",
				getdata(4, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xc9\xfa\xc8\xd5\xc4\xea\xb7\xdd(\xcb\xc4\xce\xbb\xca\xfd): ",
						ans, 5, DOECHO, YEA);
				u->birthyear = strtol(ans, NULL, 10) - 1900;
				//% getdata(5, 0, "请输入您的生日月份: ", ans, 3, DOECHO, YEA);
				getdata(5, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xc9\xfa\xc8\xd5\xd4\xc2\xb7\xdd: ", ans, 3, DOECHO, YEA);
				u->birthmonth = strtol(ans, NULL, 10);
				//% getdata(6, 0, "请输入您的出生日: ", ans, 3, DOECHO, YEA);
				getdata(6, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xb3\xf6\xc9\xfa\xc8\xd5: ", ans, 3, DOECHO, YEA);
				u->birthday = strtol(ans, NULL, 10);
				break;
			default:
				finish = true;
				break;
		}
	}
}

//	查询u所指向的用户的资料信息
void uinfo_query(struct userec *u, int real, int unum)
{
	struct userec newinfo;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	int i, fail = 0;
	int r = 0; //add by money 2003.10.14 for test 闰年
	memcpy(&newinfo, u, sizeof(currentuser));
	//% getdata(-1, 0, real ? "请选择 (0)结束 (1)修改资料 (2)设定密码 ==> [0]"
	getdata(-1, 0, real ? "\xc7\xeb\xd1\xa1\xd4\xf1 (0)\xbd\xe1\xca\xf8 (1)\xd0\xde\xb8\xc4\xd7\xca\xc1\xcf (2)\xc9\xe8\xb6\xa8\xc3\xdc\xc2\xeb ==> [0]"
			//% : "请选择 (0)结束 (1)修改资料 (2)设定密码 (3) 选签名档 ==> [0]", ans, 2,
			: "\xc7\xeb\xd1\xa1\xd4\xf1 (0)\xbd\xe1\xca\xf8 (1)\xd0\xde\xb8\xc4\xd7\xca\xc1\xcf (2)\xc9\xe8\xb6\xa8\xc3\xdc\xc2\xeb (3) \xd1\xa1\xc7\xa9\xc3\xfb\xb5\xb5 ==> [0]", ans, 2,
			DOECHO, YEA);
	screen_clear();

	//added by roly 02.03.07
	if (real && !HAS_PERM(PERM_SPECIAL0))
		return;
	//add end

	screen_flush();

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		//% prints("使用者代号: %s\n", u->userid);
		prints("\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5: %s\n", u->userid);
	switch (ans[0]) {
		case '1':
			move(1, 0);
			//% prints("请逐项修改,直接按 <ENTER> 代表使用 [] 内的资料。\n");
			prints("\xc7\xeb\xd6\xf0\xcf\xee\xd0\xde\xb8\xc4,\xd6\xb1\xbd\xd3\xb0\xb4 <ENTER> \xb4\xfa\xb1\xed\xca\xb9\xd3\xc3 [] \xc4\xda\xb5\xc4\xd7\xca\xc1\xcf\xa1\xa3\n");
			//% sprintf(genbuf, "昵称 [%s]: ", u->username);
			sprintf(genbuf, "\xea\xc7\xb3\xc6 [%s]: ", u->username);
			getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
			if (buf[0]) {
				strlcpy(newinfo.username, buf, NAMELEN);
				/* added by money 2003.10.29 for filter 0xff in nick */
				filter_ff(newinfo.username);
				/* added end */
			}
			//% sprintf(genbuf, "出生年 [%d]: ", u->birthyear + 1900);
			sprintf(genbuf, "\xb3\xf6\xc9\xfa\xc4\xea [%d]: ", u->birthyear + 1900);
			getdata(i++, 0, genbuf, buf, 5, DOECHO, YEA);
			if (buf[0] && atoi(buf) > 1920 && atoi(buf) < 1998)
				newinfo.birthyear = atoi(buf) - 1900;

			//% sprintf(genbuf, "出生月 [%d]: ", u->birthmonth);
			sprintf(genbuf, "\xb3\xf6\xc9\xfa\xd4\xc2 [%d]: ", u->birthmonth);
			getdata(i++, 0, genbuf, buf, 3, DOECHO, YEA);
			if (buf[0] && atoi(buf) >= 1 && atoi(buf) <= 12)
				newinfo.birthmonth = atoi(buf);

			//% sprintf(genbuf, "出生日 [%d]: ", u->birthday);
			sprintf(genbuf, "\xb3\xf6\xc9\xfa\xc8\xd5 [%d]: ", u->birthday);
			getdata(i++, 0, genbuf, buf, 3, DOECHO, YEA);
			if (buf[0] && atoi(buf) >= 1 && atoi(buf) <= 31)
				newinfo.birthday = atoi(buf);

			/* add by money 2003.10.24 for 2.28/29 test */
			if (newinfo.birthmonth == 2) {
				if (((newinfo.birthyear+1900) % 4) == 0) {
					if (((newinfo.birthyear+1900) % 100) != 0)
						r = 1;
					else if (((newinfo.birthyear+1900) % 400) == 0)
						r = 1;
				}
				if (r) {
					if (newinfo.birthday > 29)
						newinfo.birthday = 29;
				} else {
					if (newinfo.birthday > 28)
						newinfo.birthday = 28;
				}
			}

			if ((newinfo.birthmonth<7)&&(!(newinfo.birthmonth%2))
					&&(newinfo.birthday>30))
				newinfo.birthday = 30;
			if ((newinfo.birthmonth>8)&&(newinfo.birthmonth%2)
					&&(newinfo.birthday>30))
				newinfo.birthday = 30;
			/* add end */

			//% sprintf(genbuf, "性别(M.男)(F.女) [%c]: ", u->gender);
			sprintf(genbuf, "\xd0\xd4\xb1\xf0(M.\xc4\xd0)(F.\xc5\xae) [%c]: ", u->gender);
			getdata(i++, 0, genbuf, buf, 2, DOECHO, YEA);
			if (buf[0]) {
				if (strchr("MmFf", buf[0]))
					newinfo.gender = toupper(buf[0]);
			}

			if (real)
				uinfo_change1(i, u, &newinfo);
			break;
		case '2':
			if (!real) {
				//% getdata(i++, 0, "请输入原密码: ", buf, PASSLEN, NOECHO, YEA);
				getdata(i++, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd4\xad\xc3\xdc\xc2\xeb: ", buf, PASSLEN, NOECHO, YEA);
				if (*buf == '\0' || !passwd_check(u->userid, buf)) {
					//% prints("\n\n很抱歉, 您输入的密码不正确。\n");
					prints("\n\n\xba\xdc\xb1\xa7\xc7\xb8, \xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xc3\xdc\xc2\xeb\xb2\xbb\xd5\xfd\xc8\xb7\xa1\xa3\n");
					fail++;
					break;
				}
			}
			while (1) {
				//% getdata(i++, 0, "请设定新密码: ", buf, PASSLEN, NOECHO, YEA);
				getdata(i++, 0, "\xc7\xeb\xc9\xe8\xb6\xa8\xd0\xc2\xc3\xdc\xc2\xeb: ", buf, PASSLEN, NOECHO, YEA);
				if (buf[0] == '\0') {
					//% prints("\n\n密码设定取消, 继续使用旧密码\n");
					prints("\n\n\xc3\xdc\xc2\xeb\xc9\xe8\xb6\xa8\xc8\xa1\xcf\xfb, \xbc\xcc\xd0\xf8\xca\xb9\xd3\xc3\xbe\xc9\xc3\xdc\xc2\xeb\n");
					fail++;
					break;
				}
				if (strlen(buf) < 4 || !strcmp(buf, u->userid)) {
					//% prints("\n\n密码太短或与使用者代号相同, 密码设定取消, 继续使用旧密码\n");
					prints("\n\n\xc3\xdc\xc2\xeb\xcc\xab\xb6\xcc\xbb\xf2\xd3\xeb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5\xcf\xe0\xcd\xac, \xc3\xdc\xc2\xeb\xc9\xe8\xb6\xa8\xc8\xa1\xcf\xfb, \xbc\xcc\xd0\xf8\xca\xb9\xd3\xc3\xbe\xc9\xc3\xdc\xc2\xeb\n");
					fail++;
					break;
				}
				strlcpy(genbuf, buf, PASSLEN);
				//% getdata(i++, 0, "请重新输入新密码: ", buf, PASSLEN, NOECHO, YEA);
				getdata(i++, 0, "\xc7\xeb\xd6\xd8\xd0\xc2\xca\xe4\xc8\xeb\xd0\xc2\xc3\xdc\xc2\xeb: ", buf, PASSLEN, NOECHO, YEA);
				if (strncmp(buf, genbuf, PASSLEN)) {
					//% prints("\n\n新密码确认失败, 无法设定新密码。\n");
					prints("\n\n\xd0\xc2\xc3\xdc\xc2\xeb\xc8\xb7\xc8\xcf\xca\xa7\xb0\xdc, \xce\xde\xb7\xa8\xc9\xe8\xb6\xa8\xd0\xc2\xc3\xdc\xc2\xeb\xa1\xa3\n");
					fail++;
					break;
				}
				buf[8] = '\0';
				break;
			}
			break;
		case '3':
			if (!real) {
				//% sprintf(genbuf, "目前使用签名档 [%d]: ", u->signature);
				sprintf(genbuf, "\xc4\xbf\xc7\xb0\xca\xb9\xd3\xc3\xc7\xa9\xc3\xfb\xb5\xb5 [%d]: ", u->signature);
				getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
				if (atoi(buf) >= 0)
					newinfo.signature = atoi(buf);
			}
			break;
		default:
			screen_clear();
			return;
	}
	if (fail != 0) {
		pressreturn();
		screen_clear();
		return;
	}
	//% if (askyn("确定要改变吗", NA, YEA) == YEA) {
	if (askyn("\xc8\xb7\xb6\xa8\xd2\xaa\xb8\xc4\xb1\xe4\xc2\xf0", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			//% sprintf(secu, "修改 %s 的基本资料或密码。", u->userid);
			sprintf(secu, "\xd0\xde\xb8\xc4 %s \xb5\xc4\xbb\xf9\xb1\xbe\xd7\xca\xc1\xcf\xbb\xf2\xc3\xdc\xc2\xeb\xa1\xa3", u->userid);
			securityreport(secu, 0, 0);
		}
		if (strcmp(u->userid, newinfo.userid)) {
			sprintf(src, "mail/%c/%s", toupper(u->userid[0]), u->userid);
			sprintf(dst, "mail/%c/%s", toupper(newinfo.userid[0]),
					newinfo.userid);
			rename(src, dst);
			sethomepath(src, u->userid);
			sethomepath(dst, newinfo.userid);
			rename(src, dst);
			sethomefile(src, u->userid, "register");
			unlink(src);
			sethomefile(src, u->userid, "register.old");
			unlink(src);
			setuserid(unum, newinfo.userid);
		}
		if (ans[0] != '2') {
			memcpy(u, &newinfo, (size_t)sizeof(currentuser));
			substitut_record(PASSFILE, &newinfo, sizeof(newinfo), unum);
		} else {
			passwd_set(u->userid, buf);
		}
	}
	screen_clear();
	return;
}

//与Information相关联.在comm_list.c里,用于显示和设定个人资料
void x_info() {
	if (!strcmp("guest", currentuser.userid))
		return;
	set_user_status(ST_GMENU);
	disply_userinfo(&currentuser);
	uinfo_query(&currentuser, 0, usernum);
}


