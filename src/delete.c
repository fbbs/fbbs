#include <signal.h>
#include "bbs.h"
#include "fbbs/helper.h"
#include "fbbs/session.h"
#include "fbbs/terminal.h"
#include "fbbs/uinfo.h"

void mail_info(char *lastword);

static void kill_other_sessions(void)
{
	basic_session_info_t *res = get_my_sessions();
	if (res) {
		for (int i = 0; i < basic_session_info_count(res); ++i) {
			session_id_t sid = basic_session_info_sid(res, i);
			if (sid != session_id())
				bbs_kill(sid, basic_session_info_pid(res, i), SIGHUP);
		}
	}
	basic_session_info_clear(res);
}

//自杀,详情后叙
int offline() {
	int i;
	char buf[STRLEN], lastword[640];

	set_user_status(ST_OFFLINE);
	clear();
	/*2003.04.22 modified by stephen to deny the user who is under punishing to suicide*/
	if (!HAS_PERM(PERM_POST)|| !HAS_PERM(PERM_MAIL)
			|| !HAS_PERM(PERM_TALK)) {
		move(3, 0);
		//% prints("您被封禁权限, 不能随便自杀!!!\n");
		prints("\xc4\xfa\xb1\xbb\xb7\xe2\xbd\xfb\xc8\xa8\xcf\xde, \xb2\xbb\xc4\xdc\xcb\xe6\xb1\xe3\xd7\xd4\xc9\xb1!!!\n");
		pressreturn();
		clear();
		return 0;

	}
	if (HAS_PERM(PERM_SYSOPS) || HAS_PERM(PERM_BOARDS)
			|| HAS_PERM(PERM_ADMINMENU) || HAS_PERM(PERM_SEEULEVELS)) {
		move(3, 0);
		//% prints("您有重任在身, 不能随便自杀啦!!\n");
		prints("\xc4\xfa\xd3\xd0\xd6\xd8\xc8\xce\xd4\xda\xc9\xed, \xb2\xbb\xc4\xdc\xcb\xe6\xb1\xe3\xd7\xd4\xc9\xb1\xc0\xb2!!\n");
		pressreturn();
		clear();
		return 0;
	}
	/*2003.04.22 stephen modify end*/
	if (currentuser.stay < 86400) {
		move(1, 0);
		//% prints("\n\n对不起, 您还未够资格执行此命令!!\n");
		prints("\n\n\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xbb\xb9\xce\xb4\xb9\xbb\xd7\xca\xb8\xf1\xd6\xb4\xd0\xd0\xb4\xcb\xc3\xfc\xc1\xee!!\n");
		//% prints("请 mail 给 SYSOP 说明自杀原因, 谢谢。\n");
		prints("\xc7\xeb mail \xb8\xf8 SYSOP \xcb\xb5\xc3\xf7\xd7\xd4\xc9\xb1\xd4\xad\xd2\xf2, \xd0\xbb\xd0\xbb\xa1\xa3\n");
		pressreturn();
		clear();
		return 0;
	}
	//% getdata(1, 0, "请输入您的密码: ", buf, PASSLEN, NOECHO, YEA);
	getdata(1, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xc3\xdc\xc2\xeb: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !passwd_check(currentuser.userid, buf)) {
		//% prints("\n\n很抱歉, 您输入的密码不正确。\n");
		prints("\n\n\xba\xdc\xb1\xa7\xc7\xb8, \xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xc3\xdc\xc2\xeb\xb2\xbb\xd5\xfd\xc8\xb7\xa1\xa3\n");
		pressreturn();
		clear();
		return 0;
	}
	clear();
	//   move(1, 0);
	//   prints("[1;5;31m警告[0;1;31m： 自杀後, 您将无法再用此帐号进入本站！！");
	move(3, 0);
	//   prints("[1;32m本站站务没有义务为您恢复帐号。好难过喔 :( .....[m");
	//   move(5,0);
	i = 0;
	//% if (askyn("真是舍不得你，你走之前有什么话想说么", NA, NA)==YEA) {
	if (askyn("\xd5\xe6\xca\xc7\xc9\xe1\xb2\xbb\xb5\xc3\xc4\xe3\xa3\xac\xc4\xe3\xd7\xdf\xd6\xae\xc7\xb0\xd3\xd0\xca\xb2\xc3\xb4\xbb\xb0\xcf\xeb\xcb\xb5\xc3\xb4", NA, NA)==YEA) {
		strcpy(lastword, ">\n> ");
		buf[0] = '\0';
		for (i = 0; i< 8; i++) {
			getdata(i+6, 0, ": ", buf, 77, DOECHO, YEA);
			if (buf[0] == '\0')
				break;
			strcat(lastword, buf);
			strcat(lastword, "\n> ");
		}
		if (i == 0)
			lastword[0] = '\0';
		else
			strcat(lastword, "\n\n");
		move(i + 8, 0);
		if (i == 0)
			//% prints("哎，你还是什么都不愿意说，是不是还有心思未了？");
			prints("\xb0\xa5\xa3\xac\xc4\xe3\xbb\xb9\xca\xc7\xca\xb2\xc3\xb4\xb6\xbc\xb2\xbb\xd4\xb8\xd2\xe2\xcb\xb5\xa3\xac\xca\xc7\xb2\xbb\xca\xc7\xbb\xb9\xd3\xd0\xd0\xc4\xcb\xbc\xce\xb4\xc1\xcb\xa3\xbf");
		else if (i <= 4)
			//% prints("看着你憔悴的脸，我心都碎了 ... ");
			prints("\xbf\xb4\xd7\xc5\xc4\xe3\xe3\xbe\xe3\xb2\xb5\xc4\xc1\xb3\xa3\xac\xce\xd2\xd0\xc4\xb6\xbc\xcb\xe9\xc1\xcb ... ");
		else
			//% prints("我会记得你的，朋友，我也知道你的离开也是没有办法的事，好走了");
			prints("\xce\xd2\xbb\xe1\xbc\xc7\xb5\xc3\xc4\xe3\xb5\xc4\xa3\xac\xc5\xf3\xd3\xd1\xa3\xac\xce\xd2\xd2\xb2\xd6\xaa\xb5\xc0\xc4\xe3\xb5\xc4\xc0\xeb\xbf\xaa\xd2\xb2\xca\xc7\xc3\xbb\xd3\xd0\xb0\xec\xb7\xa8\xb5\xc4\xca\xc2\xa3\xac\xba\xc3\xd7\xdf\xc1\xcb");
	} else {
		strcpy(lastword, "> ......\n\n");
	}
	move(i + 10, 0);
	//% if (askyn("你确定要离开这个大家庭", NA, NA) == 1) {
	if (askyn("\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xc0\xeb\xbf\xaa\xd5\xe2\xb8\xf6\xb4\xf3\xbc\xd2\xcd\xa5", NA, NA) == 1) {
		clear();
		kill_other_sessions();
		currentuser.userlevel = 0;
		substitut_record(PASSFILE, &currentuser, sizeof(struct userec),
				usernum);
		mail_info(lastword);
		set_user_status(ST_OFFLINE);
		bbs_kill(session_id(), session_pid(), SIGHUP);
		exit(0);
	}
	return 0;
}

void mail_info(char *lastword) {
	FILE *fn;
	char filename[STRLEN];

	fb_time_t now = time(0);
	//% sprintf(filename, "%s 于 %s 登记自杀", currentuser.userid, format_time(now, TIME_FORMAT_ZH));
	sprintf(filename, "%s \xd3\xda %s \xb5\xc7\xbc\xc7\xd7\xd4\xc9\xb1", currentuser.userid, format_time(now, TIME_FORMAT_ZH));
	securityreport(filename, 1, 3);
	sprintf(filename, "tmp/suicide.%s", currentuser.userid);
	if ((fn = fopen(filename, "w")) != NULL) {
		//% fprintf(fn, "大家好,\n\n");
		fprintf(fn, "\xb4\xf3\xbc\xd2\xba\xc3,\n\n");
		//% fprintf(fn, "我是 %s (%s)。我己经决定在 15 天后离开这里了。\n\n",
		fprintf(fn, "\xce\xd2\xca\xc7 %s (%s)\xa1\xa3\xce\xd2\xbc\xba\xbe\xad\xbe\xf6\xb6\xa8\xd4\xda 15 \xcc\xec\xba\xf3\xc0\xeb\xbf\xaa\xd5\xe2\xc0\xef\xc1\xcb\xa1\xa3\n\n",
				currentuser.userid, currentuser.username);
		//% fprintf(fn, "自 %14.14s 至今，我已经来此 %d 次了，在这总计 %d 分钟的网络生命中，\n",
		fprintf(fn, "\xd7\xd4 %14.14s \xd6\xc1\xbd\xf1\xa3\xac\xce\xd2\xd2\xd1\xbe\xad\xc0\xb4\xb4\xcb %d \xb4\xce\xc1\xcb\xa3\xac\xd4\xda\xd5\xe2\xd7\xdc\xbc\xc6 %d \xb7\xd6\xd6\xd3\xb5\xc4\xcd\xf8\xc2\xe7\xc9\xfa\xc3\xfc\xd6\xd0\xa3\xac\n",
				format_time(currentuser.firstlogin, TIME_FORMAT_ZH), currentuser.numlogins, currentuser.stay/60);
		//% fprintf(fn, "我又如何会轻易舍弃呢？但是我得走了...  点点滴滴－－尽在我心中！\n\n");
		fprintf(fn, "\xce\xd2\xd3\xd6\xc8\xe7\xba\xce\xbb\xe1\xc7\xe1\xd2\xd7\xc9\xe1\xc6\xfa\xc4\xd8\xa3\xbf\xb5\xab\xca\xc7\xce\xd2\xb5\xc3\xd7\xdf\xc1\xcb...  \xb5\xe3\xb5\xe3\xb5\xce\xb5\xce\xa3\xad\xa3\xad\xbe\xa1\xd4\xda\xce\xd2\xd0\xc4\xd6\xd0\xa3\xa1\n\n");
		fprintf(fn, "%s", lastword);
		//% fprintf(fn, "朋友们，请把 %s 从你们的好友名单中拿掉吧。因为我己经决定离开这里了!\n\n",
		fprintf(fn, "\xc5\xf3\xd3\xd1\xc3\xc7\xa3\xac\xc7\xeb\xb0\xd1 %s \xb4\xd3\xc4\xe3\xc3\xc7\xb5\xc4\xba\xc3\xd3\xd1\xc3\xfb\xb5\xa5\xd6\xd0\xc4\xc3\xb5\xf4\xb0\xc9\xa1\xa3\xd2\xf2\xce\xaa\xce\xd2\xbc\xba\xbe\xad\xbe\xf6\xb6\xa8\xc0\xeb\xbf\xaa\xd5\xe2\xc0\xef\xc1\xcb!\n\n",
				currentuser.userid);
		//% fprintf(fn, "或许有朝一日我会回来的。 珍重!! 再见!!\n\n\n");
		fprintf(fn, "\xbb\xf2\xd0\xed\xd3\xd0\xb3\xaf\xd2\xbb\xc8\xd5\xce\xd2\xbb\xe1\xbb\xd8\xc0\xb4\xb5\xc4\xa1\xa3 \xd5\xe4\xd6\xd8!! \xd4\xd9\xbc\xfb!!\n\n\n");
		//% fprintf(fn, "%s 于 %s 留.\n\n", currentuser.userid, format_time(now, TIME_FORMAT_ZH));
		fprintf(fn, "%s \xd3\xda %s \xc1\xf4.\n\n", currentuser.userid, format_time(now, TIME_FORMAT_ZH));
		fclose(fn);
		{
			char sc_title[128];
			//% sprintf(sc_title, "%s的自杀留言...", currentuser.userid);
			sprintf(sc_title, "%s\xb5\xc4\xd7\xd4\xc9\xb1\xc1\xf4\xd1\xd4...", currentuser.userid);
			Postfile(filename, "GoneWithTheWind", sc_title, 2);
			unlink(filename);
		}
	}
}

/*2003.04.22 added by stephen to add retire function
 **can give up these permisions: 1.login 2.chat 3.mail 4.post
 **use lookupuser as temp userec struct 
 */
//	戒网
int giveUpBBS() {
	char buf[STRLEN], genbuf[STRLEN];
	FILE *fn;
	char ans[3], day[10];
	int i, j, k, lcount, tcount;
	int id;

	lookupuser = currentuser;

	id = getuser(currentuser.userid);

	set_user_status(ST_GIVEUPBBS);
	if (!HAS_PERM(PERM_REGISTER)) {
		clear();
		move(11, 28);
		//% prints("[1m[33m你有还没有注册通过，不能戒网！[m");
		prints("[1m[33m\xc4\xe3\xd3\xd0\xbb\xb9\xc3\xbb\xd3\xd0\xd7\xa2\xb2\xe1\xcd\xa8\xb9\xfd\xa3\xac\xb2\xbb\xc4\xdc\xbd\xe4\xcd\xf8\xa3\xa1[m");
		pressanykey();
		return 0;
	}

	if (HAS_PERM(PERM_SYSOPS) || HAS_PERM(PERM_BOARDS)
			|| HAS_PERM(PERM_OBOARDS) || HAS_PERM(PERM_ANNOUNCE)) {
		clear();
		move(11, 28);
		//% prints("[1m[33m你有重任在身，不能戒网！[m");
		prints("[1m[33m\xc4\xe3\xd3\xd0\xd6\xd8\xc8\xce\xd4\xda\xc9\xed\xa3\xac\xb2\xbb\xc4\xdc\xbd\xe4\xcd\xf8\xa3\xa1[m");
		pressanykey();
		return 0;
	}

	lcount = 0;
	tcount = 0;

	memset(buf, 0, STRLEN);
	memset(ans, 0, 3);
	memset(day, 0, 10);

	sethomefile(genbuf, lookupuser.userid, "giveupBBS");
	fn = fopen(genbuf, "rt");
	if (fn) {
		clear();
		move(1, 0);
		//% prints("你现在的戒网情况：\n\n");
		prints("\xc4\xe3\xcf\xd6\xd4\xda\xb5\xc4\xbd\xe4\xcd\xf8\xc7\xe9\xbf\xf6\xa3\xba\n\n");
		while (!feof(fn)) {
			if (fscanf(fn, "%d %d", &i, &j) <= 0)
				break;
			switch (i) {
				case 1:
					//% prints("上站权限");
					prints("\xc9\xcf\xd5\xbe\xc8\xa8\xcf\xde");
					break;
				case 2:
					//% prints("发表权限");
					prints("\xb7\xa2\xb1\xed\xc8\xa8\xcf\xde");
					break;
				case 3:
					//% prints("聊天权限");
					prints("\xc1\xc4\xcc\xec\xc8\xa8\xcf\xde");
					break;
				case 4:
					//% prints("发信权限");
					prints("\xb7\xa2\xd0\xc5\xc8\xa8\xcf\xde");
					break;
			}
			//% sprintf(buf, "        还有%ld天\n", j - time(0) / 3600 / 24);
			sprintf(buf, "        \xbb\xb9\xd3\xd0%ld\xcc\xec\n", j - time(0) / 3600 / 24);
			outs(buf);
			lcount++;
		}
		fclose(fn);
		memset(buf, 0, STRLEN);
		pressanykey();
	}

	clear();
	move(1, 0);
	//% prints("请选择戒网种类:");
	prints("\xc7\xeb\xd1\xa1\xd4\xf1\xbd\xe4\xcd\xf8\xd6\xd6\xc0\xe0:");
	move(3, 0);
	//% prints("(0) - 结束");
	prints("(0) - \xbd\xe1\xca\xf8");
	move(4, 0);
	//% prints("(1) - 上站权限");
	prints("(1) - \xc9\xcf\xd5\xbe\xc8\xa8\xcf\xde");
	move(5, 0);
	//% prints("(2) - 发表权限");
	prints("(2) - \xb7\xa2\xb1\xed\xc8\xa8\xcf\xde");
	move(6, 0);
	//% prints("(3) - 聊天权限");
	prints("(3) - \xc1\xc4\xcc\xec\xc8\xa8\xcf\xde");
	move(7, 0);
	//% prints("(4) - 发信权限");
	prints("(4) - \xb7\xa2\xd0\xc5\xc8\xa8\xcf\xde");

	//% getdata(10, 0, "请选择 [0]", ans, 2, DOECHO, NA);
	getdata(10, 0, "\xc7\xeb\xd1\xa1\xd4\xf1 [0]", ans, 2, DOECHO, NA);
	if (ans[0] < '1' || ans[0] > '4') {
		return 0;
	}
	k = 1;
	switch (ans[0]) {
		case '1':
			k = k && (lookupuser.userlevel & PERM_LOGIN);
			break;
		case '2':
			k = k && (lookupuser.userlevel & PERM_POST);
			break;
		case '3':
			k = k && (lookupuser.userlevel & PERM_TALK);
			break;
		case '4':
			k = k && (lookupuser.userlevel & PERM_MAIL);
			break;
	}

	if (!k) {
		//% prints("\n\n你已经没有了该权限");
		prints("\n\n\xc4\xe3\xd2\xd1\xbe\xad\xc3\xbb\xd3\xd0\xc1\xcb\xb8\xc3\xc8\xa8\xcf\xde");
		pressanykey();
		return 0;
	}

	//% getdata(11, 0, "请输入戒网天数 [0]", day, 4, DOECHO, NA);
	getdata(11, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xbd\xe4\xcd\xf8\xcc\xec\xca\xfd [0]", day, 4, DOECHO, NA);
	i = 0;
	while (day[i]) {
		if (!isdigit(day[i]))
			return 0;
		i++;
	}
	j = atoi(day);
	if (j <= 0)
		return 0;

	if (compute_user_value(&lookupuser) <= j) {
		//% prints("\n\n对不起，天数不可以大于生命力...");
		prints("\n\n\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xcc\xec\xca\xfd\xb2\xbb\xbf\xc9\xd2\xd4\xb4\xf3\xd3\xda\xc9\xfa\xc3\xfc\xc1\xa6...");
		pressanykey();
		return 0;
	}

	j = time(0) / 3600 / 24 + j;

	move(13, 0);

	//% if (askyn("你确定要戒网吗？", NA, NA) == 1) {
	if (askyn("\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xbd\xe4\xcd\xf8\xc2\xf0\xa3\xbf", NA, NA) == 1) {
		//% getdata(15, 0, "请输入密码: ", buf, 39, NOECHO, NA);
		getdata(15, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc3\xdc\xc2\xeb: ", buf, 39, NOECHO, NA);
		if (*buf == '\0' || !passwd_check(lookupuser.userid, buf)) {
			//% prints("\n\n很抱歉, 您输入的密码不正确。\n");
			prints("\n\n\xba\xdc\xb1\xa7\xc7\xb8, \xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xc3\xdc\xc2\xeb\xb2\xbb\xd5\xfd\xc8\xb7\xa1\xa3\n");
			pressanykey();
			return 0;
		}

		sethomefile(genbuf, lookupuser.userid, "giveupBBS");
		fn = fopen(genbuf, "at");
		if (!fn) {
			//% prints("\n\n由于系统问题，现在你不能戒网");
			prints("\n\n\xd3\xc9\xd3\xda\xcf\xb5\xcd\xb3\xce\xca\xcc\xe2\xa3\xac\xcf\xd6\xd4\xda\xc4\xe3\xb2\xbb\xc4\xdc\xbd\xe4\xcd\xf8");
			pressanykey();
			return 0;
		}
		fprintf(fn, "%d %d\n", ans[0] - 48, j);
		fclose(fn);

		char *str = format_time(fb_time(), TIME_FORMAT_ZH);
		switch (ans[0]) {
			case '1':
				lookupuser.userlevel &= ~PERM_LOGIN;
				//% sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
				sprintf(buf, "%s \xd3\xda %14.14s \xbd\xe4 %s\xc8\xa8\xcf\xde %d \xcc\xec\xa1\xa3",
						//% lookupuser.userid, str, "上站", atoi(day));
						lookupuser.userid, str, "\xc9\xcf\xd5\xbe", atoi(day));
				break;
			case '2':
				lookupuser.userlevel &= ~PERM_POST;
				//% sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
				sprintf(buf, "%s \xd3\xda %14.14s \xbd\xe4 %s\xc8\xa8\xcf\xde %d \xcc\xec\xa1\xa3",
						//% lookupuser.userid, str, "发文", atoi(day));
						lookupuser.userid, str, "\xb7\xa2\xce\xc4", atoi(day));
				break;
			case '3':
				lookupuser.userlevel &= ~PERM_TALK;
				//% sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
				sprintf(buf, "%s \xd3\xda %14.14s \xbd\xe4 %s\xc8\xa8\xcf\xde %d \xcc\xec\xa1\xa3",
						//% lookupuser.userid, str, "聊天", atoi(day));
						lookupuser.userid, str, "\xc1\xc4\xcc\xec", atoi(day));
				break;
			case '4':
				lookupuser.userlevel &= ~PERM_MAIL;
				//% sprintf(buf, "%s 于 %14.14s 戒 %s权限 %d 天。",
				sprintf(buf, "%s \xd3\xda %14.14s \xbd\xe4 %s\xc8\xa8\xcf\xde %d \xcc\xec\xa1\xa3",
						//% lookupuser.userid, str, "信件", atoi(day));
						lookupuser.userid, str, "\xd0\xc5\xbc\xfe", atoi(day));
				break;
		}
		lcount++;
		securityreport(buf, 1, 3);

		if (lookupuser.userlevel & PERM_LOGIN)
			tcount++;
		if (lookupuser.userlevel & PERM_POST)
			tcount++;
		if (lookupuser.userlevel & PERM_TALK)
			tcount++;
		if (lookupuser.userlevel & PERM_MAIL)
			tcount++;

		if (lcount + tcount == 4)
			lookupuser.flags[0] |= GIVEUPBBS_FLAG;
		else
			lookupuser.flags[0] &= ~GIVEUPBBS_FLAG;

		//% prints("\n\n你已经开始戒网了");
		prints("\n\n\xc4\xe3\xd2\xd1\xbe\xad\xbf\xaa\xca\xbc\xbd\xe4\xcd\xf8\xc1\xcb");

		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);

		memset(buf, 0, STRLEN);
		memset(day, 0, 10);

		pressanykey();
		if (ans[0] == '1')
			abort_bbs(0);

		memset(ans, 0, 3);
	}
	return 0;
}
/*2003.04.22 stephen add end*/
