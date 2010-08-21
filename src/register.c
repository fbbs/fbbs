#include "bbs.h"
#include "register.h"

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

int getnewuserid(void)
{
	struct userec user;
	memset(&user, 0, sizeof(user));
	strlcpy(user.userid, "new", sizeof(user.userid));

	int i = searchnewuser();

	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "uid %d from %s", i, fromhost);
	log_usies("APPLY", genbuf, &currentuser);

	if (i <= 0 || i > MAXUSERS)
		return i;

	substitut_record(PASSFILE, &user, sizeof(user), i);
	return i;
}

/**
 *
 */
static const char *invalid_userid(const char *userid)
{
	switch (check_userid(userid)) {
		case BBS_EREG_NONALPHA:
			return "ÕÊºÅ±ØĞëÈ«ÎªÓ¢ÎÄ×ÖÄ¸!\n";
		case BBS_EREG_SHORT:
			return "ÕÊºÅÖÁÉÙĞèÓĞÁ½¸öÓ¢ÎÄ×ÖÄ¸!\n";
		case BBS_EREG_BADNAME:
			return "±§Ç¸, Äú²»ÄÜÊ¹ÓÃÕâ¸ö×Ö×÷ÎªÕÊºÅ¡£\n";
		default:
			return NULL;
	}
}

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

void new_register(void)
{
	char userid[IDLEN + 1], passwd[PASSLEN], passbuf[PASSLEN], log[STRLEN];

	if (is_no_register()) {
		ansimore("NOREGISTER", NA);
		pressreturn();
		return;
	}

	ansimore("etc/register", NA);

#ifndef FDQUAN
	if (!askyn("ÄúÊÇ·ñÍ¬Òâ±¾Õ¾Announce°æ¾«»ªÇøx-3Ä¿Â¼ËùÁĞÕ¾¹æ?", false, false))
		return 0;
#endif

	int tried = 0;
	prints("\n");
	while (1) {
		if (++tried >= 9) {
			prints("\n°İ°İ£¬°´Ì«¶àÏÂ  <Enter> ÁË...\n");
			refresh();
			return;
		}

		getdata(0, 0, "ÇëÊäÈëÕÊºÅÃû³Æ (Enter User ID, \"0\" to abort): ",
				userid, sizeof(userid), DOECHO, YEA);
		if (userid[0] == '0')
			return;
		const char *errmsg = invalid_userid(userid);
		if (errmsg != NULL) {
			outs(errmsg);
			continue;
		}

		char path[HOMELEN];
		sethomepath(path, userid);
		if (dosearchuser(userid, &currentuser, &usernum) || dashd(path))
			prints("´ËÕÊºÅÒÑ¾­ÓĞÈËÊ¹ÓÃ\n");
		else
			break;
	}

	for (tried = 0; tried <= MAX_SET_PASSWD_TRIES; ++tried) {
		passbuf[0] = '\0';
		getdata(0, 0, "ÇëÉè¶¨ÄúµÄÃÜÂë (Setup Password): ", passbuf,
				sizeof(passbuf), NOECHO, YEA);
		if (strlen(passbuf) < 4 || !strcmp(passbuf, userid)) {
			prints("ÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÇëÖØĞÂÊäÈë\n");
			continue;
		}
		strlcpy(passwd, passbuf, PASSLEN);
		getdata(0, 0, "ÇëÔÙÊäÈëÒ»´ÎÄúµÄÃÜÂë (Reconfirm Password): ", passbuf,
				PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, passwd, PASSLEN) != 0) {
			prints("ÃÜÂëÊäÈë´íÎó, ÇëÖØĞÂÊäÈëÃÜÂë.\n");
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

	/* added by roly */
	sprintf(genbuf, "/bin/rm -fr %s/mail/%c/%s", BBSHOME,
			toupper(newuser.userid[0]), newuser.userid) ;
	system(genbuf);
	sprintf(genbuf, "/bin/rm -fr %s/home/%c/%s", BBSHOME,
			toupper(newuser.userid[0]), newuser.userid) ;
	system(genbuf);
	/* add end */

	int allocid = getnewuserid();
	if (allocid > MAXUSERS || allocid <= 0) {
		prints("No space for new users on the system!\n\r");
		return;
	}
	setuserid(allocid, newuser.userid);
	if (substitut_record(PASSFILE, &newuser, sizeof(newuser), allocid) == -1) {
		prints("too much, good bye!\n");
		return;
	}
	if (!dosearchuser(newuser.userid, &currentuser, &usernum)) {
		prints("User failed to create\n");
		return;
	}

	snprintf(log, sizeof(log), "new account from %s", fromhost);
	report(log, currentuser.userid);

	prints("ÇëÖØĞÂµÇÂ¼ %s ²¢ÌîĞ´×¢²áĞÅÏ¢\n", newuser.userid);
	pressanykey();
	return;
}

int invalid_email(char *addr) {
	FILE *fp;
	char temp[STRLEN], tmp2[STRLEN];

	if (strlen(addr)<3)
		return 1;

	strtolower(tmp2, addr);
	if (strstr(tmp2, "bbs") != NULL)
		return 1;

	if ((fp = fopen(".bad_email", "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			strtolower(genbuf, temp);
			if (strstr(tmp2, genbuf)!=NULL||strstr(genbuf, tmp2) != NULL) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

int check_register_ok(void) {
	char fname[STRLEN];

	sethomefile(fname, currentuser.userid, "register");
	if (dashf(fname)) {
		move(21, 0);
		prints("¹§ºØÄú!! ÄúÒÑË³ÀûÍê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø,\n");
		prints("´ÓÏÖÔÚÆğÄú½«ÓµÓĞÒ»°ãÊ¹ÓÃÕßµÄÈ¨ÀûÓëÒåÎñ...\n");
		pressanykey();
		return 1;
	}
	return 0;
}

void check_reg_mail() {
	struct userec *urec = &currentuser;
	char buf[192], code[STRLEN], email[STRLEN]="ÄúµÄÓÊÏä";
	FILE *fout;
	int i;
	sethomefile(buf, urec->userid, ".regpass");
	if (!dashf(buf)) {
		move(1, 0);
		prints("    ÇëÊäÈëÄúµÄ¸´µ©ÓÊÏä(username@fudan.edu.cn)\n");
		prints("    [1;32m±¾Õ¾²ÉÓÃ¸´µ©ÓÊÏä°ó¶¨ÈÏÖ¤£¬½«·¢ËÍÈÏÖ¤ÂëÖÁÄúµÄ¸´µ©ÓÊÏä[m");
		do {
			getdata(3, 0, "    E-Mail:> ", email, STRLEN-12, DOECHO, YEA);
			if (invalidaddr(email) ||(strstr(email, "@fudan.edu.cn")
					== NULL) || invalid_email(email) == 1) {
				prints("    ¶Ô²»Æğ, ¸ÃemailµØÖ·ÎŞĞ§, ÇëÖØĞÂÊäÈë \n");
				continue;
			} else
				break;
		} while (1);
		send_regmail(urec, email);
	}
	move(4, 0);
	clrtoeol();
	move(5, 0);
	prints(" [1;33m   ÈÏÖ¤ÂëÒÑ·¢ËÍµ½ %s £¬Çë²éÊÕ[m\n", email);

	getdata(7, 0, "    ÏÖÔÚÊäÈëÈÏÖ¤ÂëÃ´£¿[Y/n] ", buf, 2, DOECHO, YEA);
	if (buf[0] != 'n' && buf[0] != 'N') {
		move(9, 0);
		prints("ÇëÊäÈë×¢²áÈ·ÈÏĞÅÀï, \"ÈÏÖ¤Âë\"À´×öÎªÉí·İÈ·ÈÏ\n");
		prints("Ò»¹²ÊÇ %d ¸ö×Ö·û, ´óĞ¡Ğ´ÊÇÓĞ²î±ğµÄ, Çë×¢Òâ.\n", RNDPASSLEN);
		prints("Çë×¢Òâ, ÇëÊäÈë×îĞÂÒ»·âÈÏÖ¤ĞÅÖĞËù°üº¬µÄÂÒÊıÃÜÂë£¡\n");
		prints("\n[1;31mÌáÊ¾£º×¢²áÂëÊä´í 3´ÎºóÏµÍ³½«ÒªÇóÄúÖØÌîĞè°ó¶¨µÄÓÊÏä¡£[m\n");

		sethomefile(buf, currentuser.userid, ".regpass");
		if ((fout = fopen(buf, "r")) != NULL) {
			//ÊäÈÏÖ¤Âë
			fscanf(fout, "%s", code);
			fscanf(fout, "%s", email);
			fclose(fout);
			//3´Î»ú»á
			for (i = 0; i < 3; i++) {
				move(15, 0);
				prints("Äú»¹ÓĞ %d ´Î»ú»á\n", 3 - i);
				getdata(16, 0, "ÇëÊäÈëÈÏÖ¤Âë: ", genbuf, (RNDPASSLEN+1), DOECHO,
						YEA);

				if (strcmp(genbuf, "") != 0) {
					if (strcmp(genbuf, code) != 0)
						continue;
					else
						break;
				}
			}
		} else
			i = 3;

		unlink(buf);
		if (i == 3) {
			prints("ÈÏÖ¤ÂëÈÏÖ¤Ê§°Ü!ÇëÖØÌîÓÊÏä¡£\n");
			//add by eefree 06.8.16
			sethomefile(buf, currentuser.userid, ".regextra");
			if (dashf(buf))
				unlink(buf);
			//add end
			pressanykey();
		} else {
			set_safe_record();
			urec->userlevel |= PERM_BINDMAIL;
			strcpy(urec->email, email);
			substitut_record(PASSFILE, urec, sizeof(struct userec),
					usernum);
			prints("ÈÏÖ¤ÂëÈÏÖ¤³É¹¦!\n");
			//add by eefree 06.8.10
			sethomefile(buf, currentuser.userid, ".regextra");
			if (dashf(buf)) {
				prints("ÎÒÃÇ½«ÔİÊ±±£ÁôÄúµÄÕı³£Ê¹ÓÃÈ¨ÏŞ,Èç¹ûºË¶ÔÄúÊäÈëµÄ¸öÈËĞÅÏ¢ÓĞÎó½«Í£Ö¹ÄúµÄ·¢ÎÄÈ¨ÏŞ,\n");
				prints("Òò´ËÇëÈ·±£ÄúÊäÈëµÄÊÇ¸öÈËÕæÊµĞÅÏ¢.\n");
			}
			//add end
			if (!HAS_PERM(PERM_REGISTER)) {
				prints("Çë¼ÌĞøÌîĞ´×¢²áµ¥¡£\n");
			}
			pressanykey();
		}
	} else {
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
			prints("ÇëÊäÈë¸öÈËĞÅÏ¢. Èç¹ûÊäÈë´íÎó,¿ÉÒÔÖØĞÂÊäÈë.\n");
			/*default value is 0*/
			do {
				getdata(2, 0, "ÊäÈëÒÔÇ°µÄÑ§ºÅ: ", schmate.school_num,
						SCHOOLNUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.school_num)); //Èç¹ûÓĞÊäÈë·ÇÊı×Ö,ÖØĞÂÊäÈë!ÏÂÍ¬
			do {
				getdata(4, 0, "ÊäÈëÓÊÏä(Íâ²¿ÓÊÏäÒà¿É): ", schmate.email, STRLEN,
						DOECHO, YEA);
			} while (invalidaddr(schmate.email));
			do {
				getdata(6, 0, "ÊäÈëÉí·İÖ¤ºÅÂë: ", schmate.identity_card_num,
						IDCARDLEN+1, DOECHO, YEA);
			} while (!isNumStrPlusX(schmate.identity_card_num)
					|| strlen(schmate.identity_card_num) !=IDCARDLEN);

			do {
				getdata(8, 0, "ÊäÈë±ÏÒµÖ¤Êé±àºÅ: ", schmate.diploma_num,
						DIPLOMANUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.diploma_num));

			do {
				getdata(10, 0, "ÊäÈëÊÖ»ú»ò¹Ì¶¨µç»°ºÅÂë: ", schmate.mobile_num,
						MOBILENUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.mobile_num));

			strcpy(buf, "");
			getdata(11, 0, "ÒÔÉÏĞÅÏ¢ÊäÈëÕıÈ·²¢½øĞĞÓÊÏä°ó¶¨ÈÏÖ¤[Y/n]", buf, 2, DOECHO, YEA);
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
	check_reg_mail();
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
			fprintf(fout, "´ó¼ÒºÃ,\n\n");
			fprintf(fout, "ÎÒÊÇ %s (%s), À´×Ô %s\n",
					currentuser.userid, urec->username, fromhost);
			fprintf(fout, "½ñÌì%s³õÀ´´ËÕ¾±¨µ½, Çë´ó¼Ò¶à¶àÖ¸½Ì¡£\n",
					(urec->gender == 'M') ? "Ğ¡µÜ" : "Ğ¡Å®×Ó");
			move(2, 0);
			prints("·Ç³£»¶Ó­ %s ¹âÁÙ±¾Õ¾£¬Ï£ÍûÄúÄÜÔÚ±¾Õ¾ÕÒµ½ÊôÓÚ×Ô¼ºµÄÒ»Æ¬Ìì¿Õ£¡\n\n", currentuser.userid);
			prints("ÇëÄú×÷¸ö¼ò¶ÌµÄ¸öÈË¼ò½é, Ïò±¾Õ¾ÆäËûÊ¹ÓÃÕß´ò¸öÕĞºô\n");
			prints("(¼ò½é×î¶àÈıĞĞ, Ğ´Íê¿ÉÖ±½Ó°´ <Enter> ÌøÀë)....");
			getdata(6, 0, ":", buf2, 75, DOECHO, YEA);
			if (buf2[0] != '\0') {
				fprintf(fout, "\n\n×ÔÎÒ½éÉÜ:\n\n");
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
			sprintf(buf2, "ĞÂÊÖÉÏÂ·: %s", urec->username);
			Postfile(buf, "newcomers", buf2, 2);
			unlink(buf);
		}
		pressanykey();
	}
#endif
#ifdef PASSAFTERTHREEDAYS
	if (urec->lastlogin - urec->firstlogin < 3 * 86400) {
		if (!HAS_PERM(PERM_SYSOP)) {
			set_safe_record();
			urec->userlevel &= ~(PERM_DEFAULT);
			urec->userlevel |= PERM_LOGIN;
			substitut_record(PASSFILE, urec, sizeof(struct userec), usernum);
			ansimore("etc/newregister", YEA);
			return;
		}
	}
#endif
#ifndef FDQUAN
	//¼ì²éÓÊÏä
	while (!HAS_PERM(PERM_BINDMAIL)) {
		clear();
		if (HAS_PERM(PERM_REGISTER)) {
			while (askyn("ÊÇ·ñ°ó¶¨¸´µ©ÓÊÏä", NA, NA)== NA)
			//add  by eefree.06.7.20
			{
				if (askyn("ÊÇ·ñÌîĞ´Ğ£ÓÑĞÅÏ¢", NA, NA) == NA) {
					clear();
					continue;
				}
				check_reg_extra();
				return;
			}
			//add end.
		}
		check_reg_mail();
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
		x_fillform();
}

//deardrago 2000.09.27  over
