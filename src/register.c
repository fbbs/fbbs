#include "bbs.h"
#include "fbbs/register.h"

enum {
	MAX_NEW_TRIES = 9,
	MAX_SET_PASSWD_TRIES = 7,
	MIN_PASSWD_LENGTH = 4
};

//modified by money 2002.11.15
extern char fromhost[60];
extern time_t login_start_time;

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif

/**
 *
 */
static void fill_new_userec(struct userec *user, const char *userid,
		const char *passwd, bool usegbk)
{
	memset(user, 0, sizeof(*user));
	strlcpy(user->userid, userid, sizeof(user->userid));
	strlcpy(user->passwd, genpasswd(passwd), ENCPASSLEN);

	user->gender = 'X';
#ifdef ALLOWGAME
	user->money = 1000;
#endif
	user->userdefine = ~0;
	if (!strcmp(userid, "guest")) {
		user->userlevel = 0;
		user->userdefine &= ~(DEF_FRIENDCALL | DEF_ALLMSG | DEF_FRIENDMSG);
	} else {
		user->userlevel = PERM_LOGIN;
		user->flags[0] = PAGER_FLAG;
	}
	user->userdefine &= ~(DEF_NOLOGINSEND);

	if (!usegbk)
		user->userdefine &= ~DEF_USEGB;

	user->flags[1] = 0;
	user->firstlogin = user->lastlogin = time(NULL);
}

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

/**
 * Telnet register interface.
 */
void new_register(void)
{
	char userid[IDLEN + 1], passwd[PASSLEN], passbuf[PASSLEN], log[STRLEN];
	const char *errmsg;

	if (is_no_register()) {
		ansimore("NOREGISTER", NA);
		pressreturn();
		return;
	}

	ansimore("etc/register", NA);
#ifndef FDQUAN
	if (!askyn("您是否同意本站Announce版精华区x-3目录所列站规?", false, false))
		return;
#endif

	int tried = 0;
	prints("\n");
	while (1) {
		if (++tried >= MAX_NEW_TRIES) {
			outs("\n拜拜，按太多下  <Enter> 了...\n");
			refresh();
			return;
		}

		getdata(0, 0, "请输入帐号名称 (Enter User ID, \"0\" to abort): ",
				userid, sizeof(userid), DOECHO, YEA);
		if (userid[0] == '0')
			return;
		errmsg = invalid_userid(userid);
		if (errmsg != NULL) {
			outs(errmsg);
			continue;
		}

		char path[HOMELEN];
		sethomepath(path, userid);
		if (dosearchuser(userid, &currentuser, &usernum) || dashd(path)) {
			outs("此帐号已经有人使用\n");
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
		getdata(0, 0, "请输入上图所包含的英文字母: ", attempt, sizeof(attempt),
				DOECHO, YEA);
		unlink(link);

		get_captcha_answer(pos, answer, sizeof(answer));
		if (strcasecmp(answer, attempt) != 0) {
			outs("验证码输入错误...\n");
			continue;
		} else {
			break;
		}
#endif // REG_CAPTCHA
	}

	for (tried = 0; tried <= MAX_SET_PASSWD_TRIES; ++tried) {
		passbuf[0] = '\0';
		getdata(0, 0, "请设定您的密码 (Setup Password): ", passbuf,
				sizeof(passbuf), NOECHO, YEA);
		errmsg = invalid_password(passbuf, userid);
		if (errmsg) {
			outs(errmsg);
			continue;
		}
		strlcpy(passwd, passbuf, PASSLEN);
		getdata(0, 0, "请再输入一次您的密码 (Confirm Password): ", passbuf,
				PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, passwd, PASSLEN) != 0) {
			prints("密码输入错误, 请重新输入密码\n");
			continue;
		}
		passwd[8] = '\0';
		break;
	}
	if (tried > MAX_SET_PASSWD_TRIES)
		return;

	struct userec newuser;
#ifdef ALLOWSWITCHCODE
	fill_new_userec(&newuser, userid, passwd, !convcode);
#else
	fill_new_userec(&newuser, userid, passwd, true);
#endif

	if (create_user(&newuser) < 0) {
		outs("Failed to create user.\n");
		return;
	}

	snprintf(log, sizeof(log), "new account from %s", fromhost);
	report(log, currentuser.userid);

	prints("请重新登录 %s 并填写注册信息\n", newuser.userid);
	pressanykey();
	return;
}

int check_register_ok(void) {
	char fname[STRLEN];

	sethomefile(fname, currentuser.userid, "register");
	if (dashf(fname)) {
		move(21, 0);
		prints("恭贺您!! 您已顺利完成本站的使用者注册手续,\n");
		prints("从现在起您将拥有一般使用者的权利与义务...\n");
		pressanykey();
		return 1;
	}
	return 0;
}

void tui_check_reg_mail(void)
{
	char email[EMAIL_LEN] = "您的邮箱", file[HOMELEN], buf[RNDPASSLEN + 1];
	int i = 0;

	sethomefile(file, currentuser.userid, REG_CODE_FILE);
	if (!dashf(file)) {
		move(1, 0);
		outs("    请输入您的复旦邮箱(username@fudan.edu.cn)\n");
		outs("    \033[1;32m本站采用复旦邮箱绑定认证，将发送认证码至您的复旦邮箱\033[m");
		do {
			getdata(3, 0, "    E-Mail:> ", email, sizeof(email), DOECHO, YEA);
			if (!valid_addr(email) || !domain_allowed(email) ||
					is_banned_email(email)) {
				prints("    对不起, 该email地址无效, 请重新输入 \n");
				continue;
			} else
				break;
		} while (1);

		send_regmail(&currentuser, email);
	}

	move(4, 0);
	clrtoeol();
	move(5, 0);
	prints(" \033[1;33m   认证码已发送到 %s ，请查收\033[m\n", email);

	move(7, 0);
	if (askyn("    现在输入认证码么？", true, false)) {
		move(9, 0);
		outs("请输入注册确认信里, \"认证码\"来做为身份确认\n");
		prints("一共是 %d 个字符, 大小写是有差别的, 请注意.\n", RNDPASSLEN);
		outs("请注意, 请输入最新一封认证信中所包含的乱数密码！\n\n"
				"\033[1;31m提示：注册码输错 3次后系统将要求您重填需绑定的邮箱。\033[m\n");

		for (i = 0; i < 3; i++) {
			move(15, 0);
			prints("您还有 %d 次机会\n", 3 - i);
			getdata(16, 0, "请输入认证码: ", buf, sizeof(buf), DOECHO, YEA);
			if (activate_email(currentuser.userid, buf))
				break;
		}
	}

	if (i == 3) {
		unlink(buf);
		prints("认证失败! 请重填邮箱。\n");
		sethomefile(buf, currentuser.userid, ".regextra");
		if (dashf(buf))
			unlink(buf);
		pressanykey();
	} else {
		prints("认证成功!\n");
		sethomefile(buf, currentuser.userid, ".regextra");
		if (dashf(buf)) {
			prints("我们将暂时保留您的正常使用权限,如果核对您输入的个人信息有误将停止您的发文权限,\n");
			prints("因此请确保您输入的是个人真实信息.\n");
		}
		if (!HAS_PERM(PERM_REGISTER)) {
			prints("请继续填写注册单。\n");
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
			prints("请输入个人信息. 如果输入错误,可以重新输入.\n");
			/*default value is 0*/
			do {
				getdata(2, 0, "输入以前的学号: ", schmate.school_num,
						SCHOOLNUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.school_num)); //如果有输入非数字,重新输入!下同
			do {
				getdata(4, 0, "输入邮箱(外部邮箱亦可): ", schmate.email, STRLEN,
						DOECHO, YEA);
			} while (!valid_addr(schmate.email));
			do {
				getdata(6, 0, "输入身份证号码: ", schmate.identity_card_num,
						IDCARDLEN+1, DOECHO, YEA);
			} while (!isNumStrPlusX(schmate.identity_card_num)
					|| strlen(schmate.identity_card_num) !=IDCARDLEN);

			do {
				getdata(8, 0, "输入毕业证书编号: ", schmate.diploma_num,
						DIPLOMANUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.diploma_num));

			do {
				getdata(10, 0, "输入手机或固定电话号码: ", schmate.mobile_num,
						MOBILENUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.mobile_num));

			strcpy(buf, "");
			getdata(11, 0, "以上信息输入正确并进行邮箱绑定认证[Y/n]", buf, 2, DOECHO, YEA);
		} while (buf[0] =='n' || buf[0] == 'N');
		sprintf(buf, "%s, %s, %s, %s, %s\n", schmate.school_num,
				schmate.email, schmate.identity_card_num,
				schmate.diploma_num, schmate.mobile_num);
		sethomefile(bufe, currentuser.userid, ".regextra");
		file_append(bufe, buf);
		do_report(".SCHOOLMATE", buf);
		send_regmail(urec, schmate.email);
	}
	clear();
	tui_check_reg_mail();
}

/*add end*/

void check_register_info() {
	struct userec *urec = &currentuser;
	FILE *fout;
	char buf[192], buf2[STRLEN];
	if (!(urec->userlevel & PERM_LOGIN)) {
		urec->userlevel = 0;
		return;
	}
#ifdef NEWCOMERREPORT
	if (urec->numlogins == 1) {
		clear();
		sprintf(buf, "tmp/newcomer.%s", currentuser.userid);
		if ((fout = fopen(buf, "w")) != NULL) {
			fprintf(fout, "大家好,\n\n");
			fprintf(fout, "我是 %s (%s), 来自 %s\n",
					currentuser.userid, urec->username, fromhost);
			fprintf(fout, "今天%s初来此站报到, 请大家多多指教。\n",
					(urec->gender == 'M') ? "小弟" : "小女子");
			move(2, 0);
			prints("非常欢迎 %s 光临本站，希望您能在本站找到属于自己的一片天空！\n\n", currentuser.userid);
			prints("请您作个简短的个人简介, 向本站其他使用者打个招呼\n");
			prints("(简介最多三行, 写完可直接按 <Enter> 跳离)....");
			getdata(6, 0, ":", buf2, 75, DOECHO, YEA);
			if (buf2[0] != '\0') {
				fprintf(fout, "\n\n自我介绍:\n\n");
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
			sprintf(buf2, "新手上路: %s", urec->username);
			Postfile(buf, "newcomers", buf2, 2);
			unlink(buf);
		}
		pressanykey();
	}
#endif
#ifndef FDQUAN
	//检查邮箱
	while (!HAS_PERM(PERM_BINDMAIL)) {
		clear();
		if (HAS_PERM(PERM_REGISTER)) {
			while (askyn("是否绑定复旦邮箱", NA, NA)== NA)
			//add  by eefree.06.7.20
			{
				if (askyn("是否填写校友信息", NA, NA) == NA) {
					clear();
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

	clear();
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

static void getfield(int line, char *info, char *desc, char *buf, int len)
{
	move(line, 0);
	prints("  原先设定: %-40.40s \033[1;32m(%s)\033[m",
			(buf[0] == '\0') ? "(未设定)" : buf, info);
	char prompt[STRLEN];
	snprintf(prompt, sizeof(prompt), "  %s: ", desc);
	getdata(line + 1, 0, prompt, buf, len, DOECHO, YEA);
	printable_filter(buf);
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

	modify_user_mode(NEW);

	clear();
	move(2, 0);
	clrtobot();
	if (currentuser.userlevel & PERM_REGISTER) {
		prints("您已经完成本站的使用者注册手续, 欢迎加入本站的行列.");
		pressreturn();
		return 0;
	}

	if (is_reg_pending(currentuser.userid)) {
		prints("站长尚未处理您的注册申请单, 您先到处看看吧.");
		pressreturn();
		return 0;
	}

	memset(&reg, 0, sizeof(reg));
	strlcpy(reg.userid, currentuser.userid, sizeof(reg.userid));
	strlcpy(reg.email, currentuser.email, sizeof(reg.email));
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s 您好, 请据实填写以下的资料:\n", currentuser.userid);
		do {
			getfield(6, "请用中文", "真实姓名",
					reg.realname, sizeof(reg.realname));
		} while (strlen(reg.realname) < 4);

		do {
			getfield(8, "学校系级或所在单位", "学校系级",
					reg.dept, sizeof(reg.dept));
		} while (strlen(reg.dept) < 6);

		do {
			getfield(10, "包括寝室或门牌号码", "目前住址",
					reg.addr, sizeof(reg.addr));
		} while (strlen(reg.addr) < 10);

		do {
			getfield(12, "包括可联络时间", "联络电话",
					reg.phone, sizeof(reg.phone));
		} while (strlen(reg.phone) < 8);

		getfield(14, "校友会或毕业学校", "校 友 会",
				reg.assoc, sizeof(reg.assoc));

		char ans[3];
		getdata(t_lines - 1, 0,
				"以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [Y]: ",
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
