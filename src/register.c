#include "bbs.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/register.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/ucache.h"

enum {
	MAX_NEW_TRIES = 9,
	MAX_SET_PASSWD_TRIES = 7,
	MIN_PASSWD_LENGTH = 4
};

//modified by money 2002.11.15
extern char fromhost[60];

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif

#ifdef REG_CAPTCHA
/**
 *
 *
 */
static int gen_captcha_link(char *link, size_t size, int *n)
{
	char target[HOMELEN];
	int r = urandom_pos_int() % NUM_CAPTCHAS;
	snprintf(target, sizeof(target), CAPTCHA_DIR"/%d.gif", r + 1);
	while (1) {
		*n = urandom_pos_int();
		snprintf(link, size, CAPTCHA_OUT"/%d.gif", *n);
		if (symlink(target, link) == 0)
			return r;
		if (errno != EEXIST)
			return -1;
	}
}

/**
 *
 *
 */
static int get_captcha_answer(int pos, char *answer, size_t size)
{
	FILE *fp = fopen(CAPTCHA_INDEX, "r");
	if (!fp)
		return -1;
	fseek(fp, pos * (CAPTCHA_LEN + 1), SEEK_SET);
	fread(answer, size, 1, fp);
	strtok(answer, " ");
	fclose(fp);
	return 0;
}
#endif // REG_CAPTCHA

/**
 * Telnet register interface.
 */
void new_register(void)
{
	char userid[IDLEN + 1], passwd[PASSLEN], passbuf[PASSLEN], log[STRLEN];
	const char *errmsg;

	if (register_closed()) {
		ansimore("NOREGISTER", NA);
		pressreturn();
		return;
	}

	ansimore("etc/register", NA);
#ifndef FDQUAN
	//% if (!askyn("您是否同意本站Announce版精华区x-3目录所列站规?", false, false))
	if (!askyn("\xc4\xfa\xca\xc7\xb7\xf1\xcd\xac\xd2\xe2\xb1\xbe\xd5\xbeAnnounce\xb0\xe6\xbe\xab\xbb\xaa\xc7\xf8x-3\xc4\xbf\xc2\xbc\xcb\xf9\xc1\xd0\xd5\xbe\xb9\xe6?", false, false))
		return;
#endif

	int tried = 0;
	prints("\n");
	while (1) {
		if (++tried >= MAX_NEW_TRIES) {
			//% outs("\n拜拜，按太多下  <Enter> 了...\n");
			outs("\n\xb0\xdd\xb0\xdd\xa3\xac\xb0\xb4\xcc\xab\xb6\xe0\xcf\xc2  <Enter> \xc1\xcb...\n");
			refresh();
			return;
		}

		//% getdata(0, 0, "请输入帐号名称 (Enter User ID, \"0\" to abort): ",
		getdata(0, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd5\xca\xba\xc5\xc3\xfb\xb3\xc6 (Enter User ID, \"0\" to abort): ",
				userid, sizeof(userid), DOECHO, YEA);
		if (userid[0] == '0')
			return;
		errmsg = register_invalid_user_name(userid);
		if (errmsg != NULL) {
			outs(errmsg);
			continue;
		}

		char path[HOMELEN];
		sethomepath(path, userid);
		if (dosearchuser(userid, &currentuser, &usernum) || dashd(path)) {
			//% outs("此帐号已经有人使用\n");
			outs("\xb4\xcb\xd5\xca\xba\xc5\xd2\xd1\xbe\xad\xd3\xd0\xc8\xcb\xca\xb9\xd3\xc3\n");
			continue;
		}
#ifndef REG_CAPTCHA
		break;
#else
		char link[STRLEN], attempt[CAPTCHA_LEN + 1], answer[CAPTCHA_LEN + 1];
		int lnum;
		int pos = gen_captcha_link(link, sizeof(link), &lnum);
		if (pos < 0)
			return;

		prints("http://"BBSHOST"/captcha/%d.gif\n", lnum);
		//% getdata(0, 0, "请输入上图所包含的英文字母: ", attempt, sizeof(attempt),
		getdata(0, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc9\xcf\xcd\xbc\xcb\xf9\xb0\xfc\xba\xac\xb5\xc4\xd3\xa2\xce\xc4\xd7\xd6\xc4\xb8: ", attempt, sizeof(attempt),
				DOECHO, YEA);
		unlink(link);

		get_captcha_answer(pos, answer, sizeof(answer));
		if (strcasecmp(answer, attempt) != 0) {
			//% outs("验证码输入错误...\n");
			outs("\xd1\xe9\xd6\xa4\xc2\xeb\xca\xe4\xc8\xeb\xb4\xed\xce\xf3...\n");
			continue;
		} else {
			break;
		}
#endif // REG_CAPTCHA
	}

	for (tried = 0; tried <= MAX_SET_PASSWD_TRIES; ++tried) {
		passbuf[0] = '\0';
		//% getdata(0, 0, "请设定您的密码 (Setup Password): ", passbuf,
		getdata(0, 0, "\xc7\xeb\xc9\xe8\xb6\xa8\xc4\xfa\xb5\xc4\xc3\xdc\xc2\xeb (Setup Password): ", passbuf,
				sizeof(passbuf), NOECHO, YEA);
		errmsg = register_invalid_password(passbuf, userid);
		if (errmsg) {
			outs(errmsg);
			continue;
		}
		strlcpy(passwd, passbuf, PASSLEN);
		//% getdata(0, 0, "请再输入一次您的密码 (Confirm Password): ", passbuf,
		getdata(0, 0, "\xc7\xeb\xd4\xd9\xca\xe4\xc8\xeb\xd2\xbb\xb4\xce\xc4\xfa\xb5\xc4\xc3\xdc\xc2\xeb (Confirm Password): ", passbuf,
				PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, passwd, PASSLEN) != 0) {
			//% prints("密码输入错误, 请重新输入密码\n");
			prints("\xc3\xdc\xc2\xeb\xca\xe4\xc8\xeb\xb4\xed\xce\xf3, \xc7\xeb\xd6\xd8\xd0\xc2\xca\xe4\xc8\xeb\xc3\xdc\xc2\xeb\n");
			continue;
		}
		passwd[8] = '\0';
		break;
	}
	if (tried > MAX_SET_PASSWD_TRIES)
		return;

	struct userec user;
#ifdef ALLOWSWITCHCODE
	init_userec(&user, userid, passwd, !convcode);
#else
	init_userec(&user, userid, passwd, true);
#endif
	strlcpy(user.lasthost, fromhost, sizeof(user.lasthost));

	if (create_user(&user) < 0) {
		outs("Failed to create user.\n");
		return;
	}

	snprintf(log, sizeof(log), "new account from %s", fromhost);
	report(log, currentuser.userid);

	//% prints("请重新登录 %s 并填写注册信息\n", user.userid);
	prints("\xc7\xeb\xd6\xd8\xd0\xc2\xb5\xc7\xc2\xbc %s \xb2\xa2\xcc\xee\xd0\xb4\xd7\xa2\xb2\xe1\xd0\xc5\xcf\xa2\n", user.userid);
	pressanykey();
	return;
}

int check_register_ok(void) {
	char fname[STRLEN];

	sethomefile(fname, currentuser.userid, "register");
	if (dashf(fname)) {
		move(21, 0);
		//% prints("恭贺您!! 您已顺利完成本站的使用者注册手续,\n");
		prints("\xb9\xa7\xba\xd8\xc4\xfa!! \xc4\xfa\xd2\xd1\xcb\xb3\xc0\xfb\xcd\xea\xb3\xc9\xb1\xbe\xd5\xbe\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xd7\xa2\xb2\xe1\xca\xd6\xd0\xf8,\n");
		//% prints("从现在起您将拥有一般使用者的权利与义务...\n");
		prints("\xb4\xd3\xcf\xd6\xd4\xda\xc6\xf0\xc4\xfa\xbd\xab\xd3\xb5\xd3\xd0\xd2\xbb\xb0\xe3\xca\xb9\xd3\xc3\xd5\xdf\xb5\xc4\xc8\xa8\xc0\xfb\xd3\xeb\xd2\xe5\xce\xf1...\n");
		pressanykey();
		return 1;
	}
	return 0;
}

void tui_check_reg_mail(void)
{
	//% char email[EMAIL_LEN] = "您的邮箱", file[HOMELEN], buf[RNDPASSLEN + 1];
	char email[EMAIL_LEN] = "\xc4\xfa\xb5\xc4\xd3\xca\xcf\xe4", file[HOMELEN], buf[RNDPASSLEN + 1];
	int i = 0;

	sethomefile(file, currentuser.userid, REG_CODE_FILE);
	if (!dashf(file)) {
		move(1, 0);
		//% outs("    请输入您的复旦邮箱(username@fudan.edu.cn/alu.fudan.edu.cn)\n"
		outs("    \xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xb8\xb4\xb5\xa9\xd3\xca\xcf\xe4(username@fudan.edu.cn/alu.fudan.edu.cn)\n"
				//% "    \033[1;32m本站采用复旦邮箱绑定认证，将发送认证码至您的复旦邮箱\033[m");
				"    \033[1;32m\xb1\xbe\xd5\xbe\xb2\xc9\xd3\xc3\xb8\xb4\xb5\xa9\xd3\xca\xcf\xe4\xb0\xf3\xb6\xa8\xc8\xcf\xd6\xa4\xa3\xac\xbd\xab\xb7\xa2\xcb\xcd\xc8\xcf\xd6\xa4\xc2\xeb\xd6\xc1\xc4\xfa\xb5\xc4\xb8\xb4\xb5\xa9\xd3\xca\xcf\xe4\033[m");
		do {
			getdata(3, 0, "    E-Mail:> ", email, sizeof(email), DOECHO, YEA);
			if (!valid_addr(email) || !register_domain_allowed(email)
					|| !register_email_allowed(email)) {
				//% prints("    对不起, 该email地址无效, 请重新输入 \n");
				prints("    \xb6\xd4\xb2\xbb\xc6\xf0, \xb8\xc3""email\xb5\xd8\xd6\xb7\xce\xde\xd0\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xca\xe4\xc8\xeb \n");
				continue;
			} else
				break;
		} while (1);

		register_send_email(&currentuser, email);
	}

	move(4, 0);
	clrtoeol();
	move(5, 0);
	//% prints(" \033[1;33m   认证码已发送到 %s ，请查收\033[m\n", email);
	prints(" \033[1;33m   \xc8\xcf\xd6\xa4\xc2\xeb\xd2\xd1\xb7\xa2\xcb\xcd\xb5\xbd %s \xa3\xac\xc7\xeb\xb2\xe9\xca\xd5\033[m\n", email);

	move(7, 0);
	//% if (askyn("    现在输入认证码么？", true, false)) {
	if (askyn("    \xcf\xd6\xd4\xda\xca\xe4\xc8\xeb\xc8\xcf\xd6\xa4\xc2\xeb\xc3\xb4\xa3\xbf", true, false)) {
		move(9, 0);
		//% outs("请输入注册确认信里, \"认证码\"来做为身份确认\n");
		outs("\xc7\xeb\xca\xe4\xc8\xeb\xd7\xa2\xb2\xe1\xc8\xb7\xc8\xcf\xd0\xc5\xc0\xef, \"\xc8\xcf\xd6\xa4\xc2\xeb\"\xc0\xb4\xd7\xf6\xce\xaa\xc9\xed\xb7\xdd\xc8\xb7\xc8\xcf\n");
		//% prints("一共是 %d 个字符, 大小写是有差别的, 请注意.\n", RNDPASSLEN);
		prints("\xd2\xbb\xb9\xb2\xca\xc7 %d \xb8\xf6\xd7\xd6\xb7\xfb, \xb4\xf3\xd0\xa1\xd0\xb4\xca\xc7\xd3\xd0\xb2\xee\xb1\xf0\xb5\xc4, \xc7\xeb\xd7\xa2\xd2\xe2.\n", RNDPASSLEN);
		//% outs("请注意, 请输入最新一封认证信中所包含的乱数密码！\n\n"
		outs("\xc7\xeb\xd7\xa2\xd2\xe2, \xc7\xeb\xca\xe4\xc8\xeb\xd7\xee\xd0\xc2\xd2\xbb\xb7\xe2\xc8\xcf\xd6\xa4\xd0\xc5\xd6\xd0\xcb\xf9\xb0\xfc\xba\xac\xb5\xc4\xc2\xd2\xca\xfd\xc3\xdc\xc2\xeb\xa3\xa1\n\n"
				//% "\033[1;31m提示：注册码输错 3次后系统将要求您重填需绑定的邮箱。\033[m\n");
				"\033[1;31m\xcc\xe1\xca\xbe\xa3\xba\xd7\xa2\xb2\xe1\xc2\xeb\xca\xe4\xb4\xed 3\xb4\xce\xba\xf3\xcf\xb5\xcd\xb3\xbd\xab\xd2\xaa\xc7\xf3\xc4\xfa\xd6\xd8\xcc\xee\xd0\xe8\xb0\xf3\xb6\xa8\xb5\xc4\xd3\xca\xcf\xe4\xa1\xa3\033[m\n");

		for (i = 0; i < 3; i++) {
			move(15, 0);
			//% prints("您还有 %d 次机会\n", 3 - i);
			prints("\xc4\xfa\xbb\xb9\xd3\xd0 %d \xb4\xce\xbb\xfa\xbb\xe1\n", 3 - i);
			//% getdata(16, 0, "请输入认证码: ", buf, sizeof(buf), DOECHO, YEA);
			getdata(16, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc8\xcf\xd6\xa4\xc2\xeb: ", buf, sizeof(buf), DOECHO, YEA);
			if (register_activate_email(currentuser.userid, buf))
				break;
		}
	}

	if (i == 3) {
		unlink(file);
		//% prints("认证失败! 请重填邮箱。\n");
		prints("\xc8\xcf\xd6\xa4\xca\xa7\xb0\xdc! \xc7\xeb\xd6\xd8\xcc\xee\xd3\xca\xcf\xe4\xa1\xa3\n");
		sethomefile(file, currentuser.userid, ".regextra");
		if (dashf(file))
			unlink(file);
		pressanykey();
	} else {
		//% prints("认证成功!\n");
		prints("\xc8\xcf\xd6\xa4\xb3\xc9\xb9\xa6!\n");
		sethomefile(file, currentuser.userid, ".regextra");
		if (dashf(file)) {
			//% prints("我们将暂时保留您的正常使用权限,如果核对您输入的个人信息有误将停止您的发文权限,\n");
			prints("\xce\xd2\xc3\xc7\xbd\xab\xd4\xdd\xca\xb1\xb1\xa3\xc1\xf4\xc4\xfa\xb5\xc4\xd5\xfd\xb3\xa3\xca\xb9\xd3\xc3\xc8\xa8\xcf\xde,\xc8\xe7\xb9\xfb\xba\xcb\xb6\xd4\xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xb8\xf6\xc8\xcb\xd0\xc5\xcf\xa2\xd3\xd0\xce\xf3\xbd\xab\xcd\xa3\xd6\xb9\xc4\xfa\xb5\xc4\xb7\xa2\xce\xc4\xc8\xa8\xcf\xde,\n");
			//% prints("因此请确保您输入的是个人真实信息.\n");
			prints("\xd2\xf2\xb4\xcb\xc7\xeb\xc8\xb7\xb1\xa3\xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xca\xc7\xb8\xf6\xc8\xcb\xd5\xe6\xca\xb5\xd0\xc5\xcf\xa2.\n");
		}
		if (!HAS_PERM(PERM_REGISTER)) {
			//% prints("请继续填写注册单。\n");
			prints("\xc7\xeb\xbc\xcc\xd0\xf8\xcc\xee\xd0\xb4\xd7\xa2\xb2\xe1\xb5\xa5\xa1\xa3\n");
		}
		pressanykey();
	}
}

/*add by Ashinmarch*/
int isNumStr(char *buf) {
	if (*buf =='\0'|| !(*buf))
		return 0;
	int i;
	for (i = 0; i < strlen(buf); i++) {
		if (!(buf[i]>='0' && buf[i]<='9'))
			return 0;
	}
	return 1;
}
int isNumStrPlusX(char *buf) {
	if (*buf =='\0'|| !(*buf))
		return 0;
	int i;
	for (i = 0; i < strlen(buf); i++) {
		if (!(buf[i]>='0' && buf[i]<='9') && !(buf[i] == 'X'))
			return 0;
	}
	return 1;
}
void check_reg_extra() {
	struct schoolmate_info schmate;
	struct userec *urec = &currentuser;
	char buf[192], bufe[192];
	sethomefile(buf, currentuser.userid, ".regextra");

	if (!dashf(buf)) {
		do {
			memset(&schmate, 0, sizeof(schmate));
			strcpy(schmate.userid, currentuser.userid);
			move(1, 0);
			//% prints("请输入个人信息. 如果输入错误,可以重新输入.\n");
			prints("\xc7\xeb\xca\xe4\xc8\xeb\xb8\xf6\xc8\xcb\xd0\xc5\xcf\xa2. \xc8\xe7\xb9\xfb\xca\xe4\xc8\xeb\xb4\xed\xce\xf3,\xbf\xc9\xd2\xd4\xd6\xd8\xd0\xc2\xca\xe4\xc8\xeb.\n");
			/*default value is 0*/
			do {
				//% getdata(2, 0, "输入以前的学号: ", schmate.school_num,
				getdata(2, 0, "\xca\xe4\xc8\xeb\xd2\xd4\xc7\xb0\xb5\xc4\xd1\xa7\xba\xc5: ", schmate.school_num,
						SCHOOLNUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.school_num)); //如果有输入非数字,重新输入!下同
			do {
				//% getdata(4, 0, "输入邮箱(外部邮箱亦可): ", schmate.email, STRLEN,
				getdata(4, 0, "\xca\xe4\xc8\xeb\xd3\xca\xcf\xe4(\xcd\xe2\xb2\xbf\xd3\xca\xcf\xe4\xd2\xe0\xbf\xc9): ", schmate.email, STRLEN,
						DOECHO, YEA);
			} while (!valid_addr(schmate.email));
			do {
				//% getdata(6, 0, "输入身份证号码: ", schmate.identity_card_num,
				getdata(6, 0, "\xca\xe4\xc8\xeb\xc9\xed\xb7\xdd\xd6\xa4\xba\xc5\xc2\xeb: ", schmate.identity_card_num,
						IDCARDLEN+1, DOECHO, YEA);
			} while (!isNumStrPlusX(schmate.identity_card_num)
					|| strlen(schmate.identity_card_num) !=IDCARDLEN);

			do {
				//% getdata(8, 0, "输入毕业证书编号: ", schmate.diploma_num,
				getdata(8, 0, "\xca\xe4\xc8\xeb\xb1\xcf\xd2\xb5\xd6\xa4\xca\xe9\xb1\xe0\xba\xc5: ", schmate.diploma_num,
						DIPLOMANUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.diploma_num));

			do {
				//% getdata(10, 0, "输入手机或固定电话号码: ", schmate.mobile_num,
				getdata(10, 0, "\xca\xe4\xc8\xeb\xca\xd6\xbb\xfa\xbb\xf2\xb9\xcc\xb6\xa8\xb5\xe7\xbb\xb0\xba\xc5\xc2\xeb: ", schmate.mobile_num,
						MOBILENUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.mobile_num));

			strcpy(buf, "");
			//% getdata(11, 0, "以上信息输入正确并进行邮箱绑定认证[Y/n]", buf, 2, DOECHO, YEA);
			getdata(11, 0, "\xd2\xd4\xc9\xcf\xd0\xc5\xcf\xa2\xca\xe4\xc8\xeb\xd5\xfd\xc8\xb7\xb2\xa2\xbd\xf8\xd0\xd0\xd3\xca\xcf\xe4\xb0\xf3\xb6\xa8\xc8\xcf\xd6\xa4[Y/n]", buf, 2, DOECHO, YEA);
		} while (buf[0] =='n' || buf[0] == 'N');
		sprintf(buf, "%s, %s, %s, %s, %s\n", schmate.school_num,
				schmate.email, schmate.identity_card_num,
				schmate.diploma_num, schmate.mobile_num);
		sethomefile(bufe, currentuser.userid, ".regextra");
		file_append(bufe, buf);
		do_report(".SCHOOLMATE", buf);
		register_send_email(urec, schmate.email);
	}
	screen_clear();
	tui_check_reg_mail();
}

static void getfield(int line, char *info, char *desc, char *buf, int len)
{
	move(line, 0);
	//% prints("  原先设定: %s \033[1;32m(%s)\033[m",
	prints("  \xd4\xad\xcf\xc8\xc9\xe8\xb6\xa8: %s \033[1;32m(%s)\033[m",
			//% (buf[0] == '\0') ? "(未设定)" : buf, info);
			(buf[0] == '\0') ? "(\xce\xb4\xc9\xe8\xb6\xa8)" : buf, info);
	char prompt[STRLEN];
	snprintf(prompt, sizeof(prompt), "  %s: ", desc);
	getdata(line + 1, 0, prompt, buf, len, DOECHO, YEA);
	string_remove_non_printable_gbk(buf);
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

int fill_reg_form(void)
{
	reginfo_t reg;

	if (!strcmp("guest", currentuser.userid))
		return 0;

	set_user_status(ST_NEW);

	screen_clear();
	move(2, 0);
	screen_clrtobot();
	if (currentuser.userlevel & PERM_REGISTER) {
		//% prints("您已经完成本站的使用者注册手续, 欢迎加入本站的行列.");
		prints("\xc4\xfa\xd2\xd1\xbe\xad\xcd\xea\xb3\xc9\xb1\xbe\xd5\xbe\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xd7\xa2\xb2\xe1\xca\xd6\xd0\xf8, \xbb\xb6\xd3\xad\xbc\xd3\xc8\xeb\xb1\xbe\xd5\xbe\xb5\xc4\xd0\xd0\xc1\xd0.");
		pressreturn();
		return 0;
	}

	if (is_reg_pending(currentuser.userid)) {
		//% prints("站长尚未处理您的注册申请单, 您先到处看看吧.");
		prints("\xd5\xbe\xb3\xa4\xc9\xd0\xce\xb4\xb4\xa6\xc0\xed\xc4\xfa\xb5\xc4\xd7\xa2\xb2\xe1\xc9\xea\xc7\xeb\xb5\xa5, \xc4\xfa\xcf\xc8\xb5\xbd\xb4\xa6\xbf\xb4\xbf\xb4\xb0\xc9.");
		pressreturn();
		return 0;
	}

	memset(&reg, 0, sizeof(reg));
	strlcpy(reg.userid, currentuser.userid, sizeof(reg.userid));
	strlcpy(reg.email, currentuser.email, sizeof(reg.email));
	while (1) {
		move(3, 0);
		clrtoeol();
		//% prints("%s 您好, 请据实填写以下的资料:\n", currentuser.userid);
		prints("%s \xc4\xfa\xba\xc3, \xc7\xeb\xbe\xdd\xca\xb5\xcc\xee\xd0\xb4\xd2\xd4\xcf\xc2\xb5\xc4\xd7\xca\xc1\xcf:\n", currentuser.userid);
		do {
			//% getfield(6, "请用中文", "真实姓名",
			getfield(6, "\xc7\xeb\xd3\xc3\xd6\xd0\xce\xc4", "\xd5\xe6\xca\xb5\xd0\xd5\xc3\xfb",
					reg.realname, sizeof(reg.realname));
		} while (strlen(reg.realname) < 4);

		do {
			//% getfield(8, "学校系级或所在单位", "学校系级",
			getfield(8, "\xd1\xa7\xd0\xa3\xcf\xb5\xbc\xb6\xbb\xf2\xcb\xf9\xd4\xda\xb5\xa5\xce\xbb", "\xd1\xa7\xd0\xa3\xcf\xb5\xbc\xb6",
					reg.dept, sizeof(reg.dept));
		} while (strlen(reg.dept) < 6);

		do {
			//% getfield(10, "包括寝室或门牌号码", "目前住址",
			getfield(10, "\xb0\xfc\xc0\xa8\xc7\xde\xca\xd2\xbb\xf2\xc3\xc5\xc5\xc6\xba\xc5\xc2\xeb", "\xc4\xbf\xc7\xb0\xd7\xa1\xd6\xb7",
					reg.addr, sizeof(reg.addr));
		} while (strlen(reg.addr) < 10);

		do {
			//% getfield(12, "包括可联络时间", "联络电话",
			getfield(12, "\xb0\xfc\xc0\xa8\xbf\xc9\xc1\xaa\xc2\xe7\xca\xb1\xbc\xe4", "\xc1\xaa\xc2\xe7\xb5\xe7\xbb\xb0",
					reg.phone, sizeof(reg.phone));
		} while (strlen(reg.phone) < 8);

		//% getfield(14, "校友会或毕业学校", "校 友 会",
		getfield(14, "\xd0\xa3\xd3\xd1\xbb\xe1\xbb\xf2\xb1\xcf\xd2\xb5\xd1\xa7\xd0\xa3", "\xd0\xa3 \xd3\xd1 \xbb\xe1",
				reg.assoc, sizeof(reg.assoc));

		char ans[3];
		getdata(-1, 0,
				//% "以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [Y]: ",
				"\xd2\xd4\xc9\xcf\xd7\xca\xc1\xcf\xca\xc7\xb7\xf1\xd5\xfd\xc8\xb7, \xb0\xb4 Q \xb7\xc5\xc6\xfa\xd7\xa2\xb2\xe1 (Y/N/Quit)? [Y]: ",
				ans, sizeof(ans), DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return 0;
		if (ans[0] != 'N' && ans[0] != 'n')
			break;
	}

	reg.regdate = time(NULL);
	append_reg_list(&reg);
	return 0;
}

void check_register_info(void)
{
	struct userec *urec = &currentuser;
	FILE *fout;
	char buf[192], buf2[STRLEN];
	if (!(urec->userlevel & PERM_LOGIN)) {
		urec->userlevel = 0;
		return;
	}
#ifdef NEWCOMERREPORT
	if (urec->numlogins == 1) {
		screen_clear();
		sprintf(buf, "tmp/newcomer.%s", currentuser.userid);
		if ((fout = fopen(buf, "w")) != NULL) {
			//% fprintf(fout, "大家好,\n\n");
			fprintf(fout, "\xb4\xf3\xbc\xd2\xba\xc3,\n\n");
			//% fprintf(fout, "我是 %s (%s), 来自 %s\n",
			fprintf(fout, "\xce\xd2\xca\xc7 %s (%s), \xc0\xb4\xd7\xd4 %s\n",
					currentuser.userid, urec->username, fromhost);
			//% fprintf(fout, "今天%s初来此站报到, 请大家多多指教。\n",
			fprintf(fout, "\xbd\xf1\xcc\xec%s\xb3\xf5\xc0\xb4\xb4\xcb\xd5\xbe\xb1\xa8\xb5\xbd, \xc7\xeb\xb4\xf3\xbc\xd2\xb6\xe0\xb6\xe0\xd6\xb8\xbd\xcc\xa1\xa3\n",
					//% (urec->gender == 'M') ? "小弟" : "小女子");
					(urec->gender == 'M') ? "\xd0\xa1\xb5\xdc" : "\xd0\xa1\xc5\xae\xd7\xd3");
			move(2, 0);
			//% prints("非常欢迎 %s 光临本站，希望您能在本站找到属于自己的一片天空！\n\n", currentuser.userid);
			prints("\xb7\xc7\xb3\xa3\xbb\xb6\xd3\xad %s \xb9\xe2\xc1\xd9\xb1\xbe\xd5\xbe\xa3\xac\xcf\xa3\xcd\xfb\xc4\xfa\xc4\xdc\xd4\xda\xb1\xbe\xd5\xbe\xd5\xd2\xb5\xbd\xca\xf4\xd3\xda\xd7\xd4\xbc\xba\xb5\xc4\xd2\xbb\xc6\xac\xcc\xec\xbf\xd5\xa3\xa1\n\n", currentuser.userid);
			//% prints("请您作个简短的个人简介, 向本站其他使用者打个招呼\n");
			prints("\xc7\xeb\xc4\xfa\xd7\xf7\xb8\xf6\xbc\xf2\xb6\xcc\xb5\xc4\xb8\xf6\xc8\xcb\xbc\xf2\xbd\xe9, \xcf\xf2\xb1\xbe\xd5\xbe\xc6\xe4\xcb\xfb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xf2\xb8\xf6\xd5\xd0\xba\xf4\n");
			//% prints("(简介最多三行, 写完可直接按 <Enter> 跳离)....");
			prints("(\xbc\xf2\xbd\xe9\xd7\xee\xb6\xe0\xc8\xfd\xd0\xd0, \xd0\xb4\xcd\xea\xbf\xc9\xd6\xb1\xbd\xd3\xb0\xb4 <Enter> \xcc\xf8\xc0\xeb)....");
			getdata(6, 0, ":", buf2, 75, DOECHO, YEA);
			if (buf2[0] != '\0') {
				//% fprintf(fout, "\n\n自我介绍:\n\n");
				fprintf(fout, "\n\n\xd7\xd4\xce\xd2\xbd\xe9\xc9\xdc:\n\n");
				fprintf(fout, "%s\n", buf2);
				getdata(7, 0, ":", buf2, 75, DOECHO, YEA);
				if (buf2[0] != '\0') {
					fprintf(fout, "%s\n", buf2);
					getdata(8, 0, ":", buf2, 75, DOECHO, YEA);
					if (buf2[0] != '\0') {
						fprintf(fout, "%s\n", buf2);
					}
				}
			}
			fclose(fout);
			//% sprintf(buf2, "新手上路: %s", urec->username);
			sprintf(buf2, "\xd0\xc2\xca\xd6\xc9\xcf\xc2\xb7: %s", urec->username);
			Postfile(buf, "newcomers", buf2, 2);
			unlink(buf);
		}
		pressanykey();
	}
#endif
#ifndef FDQUAN
	//检查邮箱
	while (!HAS_PERM(PERM_BINDMAIL)) {
		screen_clear();
		if (HAS_PERM(PERM_REGISTER)) {
			//% while (askyn("是否绑定复旦邮箱", NA, NA)== NA)
			while (askyn("\xca\xc7\xb7\xf1\xb0\xf3\xb6\xa8\xb8\xb4\xb5\xa9\xd3\xca\xcf\xe4", NA, NA)== NA)
			//add  by eefree.06.7.20
			{
				//% if (askyn("是否填写校友信息", NA, NA) == NA) {
				if (askyn("\xca\xc7\xb7\xf1\xcc\xee\xd0\xb4\xd0\xa3\xd3\xd1\xd0\xc5\xcf\xa2", NA, NA) == NA) {
					screen_clear();
					continue;
				}
				check_reg_extra();
				return;
			}
			//add end.
		}
		tui_check_reg_mail();
	}
#endif

	screen_clear();
	if (HAS_PERM(PERM_REGISTER))
		return;
#ifndef AUTOGETPERM

	if (check_register_ok()) {
#endif
		set_safe_record();
		urec->userlevel |= PERM_DEFAULT;
		substitut_record(PASSFILE, urec, sizeof(struct userec), usernum);
		return;
#ifndef AUTOGETPERM

	}
#endif

	if (!chkmail())
		fill_reg_form();
}
