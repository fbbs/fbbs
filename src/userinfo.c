#include "bbs.h"
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
extern char *cexpstr();

//½«ptrÖ¸ÏòµÄ×Ö·û´®ÖĞ×Ö·ûÖµÎª0xFFµÄ×ª»»³É¿Õ¸ñ
void filter_ff(char *ptr) {
	while (*ptr) {
		if (*(unsigned char *)ptr == 0xff)
			*ptr = ' ';
		ptr++;
	}
	return;
}

//	ÓÃÓÚ	Éè¶¨¸öÈË×ÊÁÏ  Ñ¡µ¥Ê±ÏÔÊ¾µÄĞÅÏ¢,¼´ÏÔÊ¾¸öÈË×ÊÁÏ
void disply_userinfo(struct userec *u) {
	int num, exp;
	time_t now;

	move(2, 0);
	clrtobot();
	now = time(0);
	set_safe_record();
	prints("ÄúµÄ´úºÅ     : %-14s", u->userid);
	prints("êÇ³Æ : %-20s", u->username);
	prints("     ĞÔ±ğ : %s\n", (u->gender == 'M' ? "ÄĞ" : "Å®"));
	prints("³öÉúÈÕÆÚ     : %dÄê%dÔÂ%dÈÕ", u->birthyear + 1900, u->birthmonth,
			u->birthday);
	prints(" (ÀÛ¼ÆÉú»îÌìÊı : %d)\n", days_elapsed(u->birthyear + 1900, 
			u->birthmonth, u->birthday, now));
	prints("µç×ÓÓÊ¼şĞÅÏä : %s\n", u->email);
	prints("×î½ü¹âÁÙ»úÆ÷ : %-22s\n", u->lasthost);
	prints("ÕÊºÅ½¨Á¢ÈÕÆÚ : %s[¾à½ñ %d Ìì]\n",
			getdatestring(u->firstlogin, DATE_ZH),
			(now - (u->firstlogin)) / 86400);
	getdatestring(u->lastlogin, NA);
	prints("×î½ü¹âÁÙÈÕÆÚ : %s[¾à½ñ %d Ìì]\n",
			getdatestring(u->lastlogin, DATE_ZH),
			(now-(u->lastlogin)) / 86400);
#ifdef ALLOWGAME
	prints("ÎÄÕÂÊıÄ¿     : %-20d ½±ÕÂÊıÄ¿ : %d\n",u->numposts,u->nummedals);
	prints("Ë½ÈËĞÅÏä     : %d ·â\n", u->nummails);
	prints("ÄúµÄÒøĞĞ´æ¿î : %dÔª  ´û¿î : %dÔª (%s)\n",
			u->money,u->bet,cmoney(u->money-u->bet));
#else
	prints("ÎÄÕÂÊıÄ¿     : %-20d \n", u->numposts);
	prints("Ë½ÈËĞÅÏä     : %d ·â \n", u->nummails);
#endif
	prints("ÉÏÕ¾´ÎÊı     : %d ´Î      ", u->numlogins);
	prints("ÉÏÕ¾×ÜÊ±Êı   : %d Ğ¡Ê± %d ·ÖÖÓ\n", u->stay/3600, (u->stay/60)%60);
	exp = countexp(u);
	//modified by iamfat 2002.07.25
#ifdef SHOWEXP
	prints("¾­ÑéÖµ       : %d  (%-10s)    ", exp, cexpstr(exp));
#else
	prints("¾­ÑéÖµ       : [%-10s]     ", cexpstr(exp));
#endif
	exp = countperf(u);
#ifdef SHOWPERF
	prints("±íÏÖÖµ : %d  (%s)\n", exp, cperf(exp));
#else
	prints("±íÏÖÖµ  : [%s]\n", cperf(exp));
#endif
	strcpy(genbuf, "ltmprbBOCAMURS#@XLEast0123456789\0");
	for (num = 0; num < strlen(genbuf) ; num++)
		if (!(u->userlevel & (1 << num))) //ÏàÓ¦È¨ÏŞÎª¿Õ,ÔòÖÃ'-'
			genbuf[num] = '-';
	prints("Ê¹ÓÃÕßÈ¨ÏŞ   : %s\n", genbuf);
	prints("\n");
	if (u->userlevel & PERM_SYSOPS) {
		prints("  ÄúÊÇ±¾Õ¾µÄÕ¾³¤, ¸ĞĞ»ÄúµÄĞÁÇÚÀÍ¶¯.\n");
	} else if (u->userlevel & PERM_BOARDS) {
		prints("  ÄúÊÇ±¾Õ¾µÄ°æÖ÷, ¸ĞĞ»ÄúµÄ¸¶³ö.\n");
	} else if (u->userlevel & PERM_REGISTER) {
		prints("  ÄúµÄ×¢²á³ÌĞòÒÑ¾­Íê³É, »¶Ó­¼ÓÈë±¾Õ¾.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		prints("  ĞÂÊÖÉÏÂ·, ÇëÔÄ¶Á Announce ÌÖÂÛÇø.\n");
	} else {
		prints("  ×¢²áÉĞÎ´³É¹¦, Çë²Î¿¼±¾Õ¾½øÕ¾»­ÃæËµÃ÷.\n");
	}
}

//	¸Ä±äÓÃ»§¼ÇÂ¼,uÎªÒÔÇ°µÄ¼ÇÂ¼,newinfoÎªĞÂ¼ÇÂ¼,ºóÁ½¸ö²ÎÊı¾ùÎªÖ¸Õë
//		iÎªËùÏÔÊ¾µÄĞĞ
void uinfo_change1(int i, struct userec *u, struct userec *newinfo) {
	char buf[STRLEN], genbuf[128];

	if (currentuser.userlevel & PERM_SYSOPS) {
		char temp[30];
		temp[0] = 0;
		FILE *fp;
		sethomefile(genbuf, u->userid, ".volunteer");
		if ((fp = fopen(genbuf, "r")) != NULL) {
			fgets(temp, 30, fp);
			fclose(fp);
			sprintf(genbuf, "ÊäÈëÉí·İ(Êä¿Õ¸ñÈ¡ÏûÉí·İ)£º[%s]", temp);
		} else
			sprintf(genbuf, "ÊäÈëÉí·İ£º");
		getdata(i++, 0, genbuf, buf, 30, DOECHO, YEA);
		if (buf[0]) {
			sethomefile(genbuf, u->userid, ".volunteer");
			if ((fp = fopen(genbuf, "w")) != NULL) {
				if (buf[0] != ' ') {
					fputs(buf, fp);
					fclose(fp);
				} else
					unlink(genbuf);
			}
		}
	}

	sprintf(genbuf, "µç×ÓĞÅÏä [%s]: ", u->email);
	getdata(i++, 0, genbuf, buf, STRLEN - 1, DOECHO, YEA);
	if (buf[0]) {
		strlcpy(newinfo->email, buf, STRLEN-12);
	}

	sprintf(genbuf, "ÉÏÏß´ÎÊı [%d]: ", u->numlogins);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) > 0)
		newinfo->numlogins = atoi(buf);

	sprintf(genbuf, "·¢±íÎÄÕÂÊı [%d]: ", u->numposts);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) >0)
		newinfo->numposts = atoi(buf);

	sprintf(genbuf, "µÇÂ½×ÜÊ±¼ä [%d]: ", u->stay);
	getdata(i++, 0, genbuf, buf, 10, DOECHO, YEA);
	if (atoi(buf) > 0)
		newinfo->stay = atoi(buf);
	sprintf(genbuf, "firstlogin [%d]: ", u->firstlogin);
	getdata(i++, 0, genbuf, buf, 15, DOECHO, YEA);
	if (atoi(buf) >0)
		newinfo->firstlogin = atoi(buf);
	//add end          				      	      	
#ifdef ALLOWGAME
	sprintf(genbuf, "ÒøĞĞ´æ¿î [%d]: ", u->money);
	getdata(i++, 0, genbuf, buf, 8, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->money = atoi(buf);

	sprintf(genbuf, "ÒøĞĞ´û¿î [%d]: ", u->bet);
	getdata(i++, 0, genbuf, buf, 8, DOECHO, YEA);
	if (atoi(buf)> 0)
	newinfo->bet = atoi(buf);

	sprintf(genbuf, "½±ÕÂÊı [%d]: ", u->nummedals);
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
				getdata(2, 0, "ÇëÊäÈëÄúµÄêÇ³Æ (Enter nickname): ",
						u->username, NAMELEN, DOECHO, YEA);
				strlcpy(uinfo.username, u->username, sizeof(uinfo.username));
				printable_filter(uinfo.username);
				update_ulist(&uinfo, utmpent);
				break;
			case UINFO_EGENDER:
				getdata(3, 0, "ÇëÊäÈëÄúµÄĞÔ±ğ: M.ÄĞ F.Å® [M]: ",
						ans, 2, DOECHO, YEA);
				if (ans[0] != 'F' && ans[0] != 'f')
					u->gender = 'M';
				else
					u->gender = 'F';
				break;
			case UINFO_EBIRTH:
				getdata(4, 0, "ÇëÊäÈëÄúµÄÉúÈÕÄê·İ(ËÄÎ»Êı): ",
						ans, 5, DOECHO, YEA);
				u->birthyear = strtol(ans, NULL, 10);
				getdata(5, 0, "ÇëÊäÈëÄúµÄÉúÈÕÔÂ·İ: ", ans, 3, DOECHO, YEA);
				u->birthmonth = strtol(ans, NULL, 10);
				getdata(6, 0, "ÇëÊäÈëÄúµÄ³öÉúÈÕ: ", ans, 3, DOECHO, YEA);
				u->birthday = strtol(ans, NULL, 10);
				break;
			default:
				finish = true;
				break;
		}
	}
}

//	²éÑ¯uËùÖ¸ÏòµÄÓÃ»§µÄ×ÊÁÏĞÅÏ¢
int uinfo_query(struct userec *u, int real, int unum) {
	struct userec newinfo;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	int i, fail = 0;
	unsigned char *ptr; //add by money 2003.10.29 for filter '0xff' in nick
	int r = 0; //add by money 2003.10.14 for test ÈòÄê
	time_t now;
	struct tm *tmnow;
	memcpy(&newinfo, u, sizeof(currentuser));
	getdata(t_lines - 1, 0, real ? "ÇëÑ¡Ôñ (0)½áÊø (1)ĞŞ¸Ä×ÊÁÏ (2)Éè¶¨ÃÜÂë ==> [0]"
			: "ÇëÑ¡Ôñ (0)½áÊø (1)ĞŞ¸Ä×ÊÁÏ (2)Éè¶¨ÃÜÂë (3) Ñ¡Ç©Ãûµµ ==> [0]", ans, 2,
			DOECHO, YEA);
	clear();

	//added by roly 02.03.07
	if (real && !HAS_PERM(PERM_SPECIAL0))
		return;
	//add end

	refresh();
	now = time(0);
	tmnow = localtime(&now);

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		prints("Ê¹ÓÃÕß´úºÅ: %s\n", u->userid);
	switch (ans[0]) {
		case '1':
			move(1, 0);
			prints("ÇëÖğÏîĞŞ¸Ä,Ö±½Ó°´ <ENTER> ´ú±íÊ¹ÓÃ [] ÄÚµÄ×ÊÁÏ¡£\n");
			sprintf(genbuf, "êÇ³Æ [%s]: ", u->username);
			getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
			if (buf[0]) {
				strlcpy(newinfo.username, buf, NAMELEN);
				/* added by money 2003.10.29 for filter 0xff in nick */
				ptr = newinfo.username;
				filter_ff(ptr);
				/* added end */
			}
			sprintf(genbuf, "³öÉúÄê [%d]: ", u->birthyear + 1900);
			getdata(i++, 0, genbuf, buf, 5, DOECHO, YEA);
			if (buf[0] && atoi(buf) > 1920 && atoi(buf) < 1998)
				newinfo.birthyear = atoi(buf) - 1900;

			sprintf(genbuf, "³öÉúÔÂ [%d]: ", u->birthmonth);
			getdata(i++, 0, genbuf, buf, 3, DOECHO, YEA);
			if (buf[0] && atoi(buf) >= 1 && atoi(buf) <= 12)
				newinfo.birthmonth = atoi(buf);

			sprintf(genbuf, "³öÉúÈÕ [%d]: ", u->birthday);
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

			sprintf(genbuf, "ĞÔ±ğ(M.ÄĞ)(F.Å®) [%c]: ", u->gender);
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
				getdata(i++, 0, "ÇëÊäÈëÔ­ÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
				if (*buf == '\0' || !checkpasswd(u->passwd, buf)) {
					prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
					fail++;
					break;
				}
			}
			while (1) {
				getdata(i++, 0, "ÇëÉè¶¨ĞÂÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
				if (buf[0] == '\0') {
					prints("\n\nÃÜÂëÉè¶¨È¡Ïû, ¼ÌĞøÊ¹ÓÃ¾ÉÃÜÂë\n");
					fail++;
					break;
				}
				if (strlen(buf) < 4 || !strcmp(buf, u->userid)) {
					prints("\n\nÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÃÜÂëÉè¶¨È¡Ïû, ¼ÌĞøÊ¹ÓÃ¾ÉÃÜÂë\n");
					fail++;
					break;
				}
				strlcpy(genbuf, buf, PASSLEN);
				getdata(i++, 0, "ÇëÖØĞÂÊäÈëĞÂÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
				if (strncmp(buf, genbuf, PASSLEN)) {
					prints("\n\nĞÂÃÜÂëÈ·ÈÏÊ§°Ü, ÎŞ·¨Éè¶¨ĞÂÃÜÂë¡£\n");
					fail++;
					break;
				}
				buf[8] = '\0';
				strlcpy(newinfo.passwd, genpasswd(buf), ENCPASSLEN);
				break;
			}
			break;
		case '3':
			if (!real) {
				sprintf(genbuf, "Ä¿Ç°Ê¹ÓÃÇ©Ãûµµ [%d]: ", u->signature);
				getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
				if (atoi(buf) >= 0)
					newinfo.signature = atoi(buf);
			}
			break;
		default:
			clear();
			return 0;
	}
	if (fail != 0) {
		pressreturn();
		clear();
		return 0;
	}
	if (askyn("È·¶¨Òª¸Ä±äÂğ", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸Ä %s µÄ»ù±¾×ÊÁÏ»òÃÜÂë¡£", u->userid);
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
		if (!strcmp(u->userid, currentuser.userid)) {
			extern int WishNum;
			strlcpy(uinfo.username, newinfo.username, NAMELEN);
			WishNum = 9999;
		}
		memcpy(u, &newinfo, (size_t)sizeof(currentuser));
		substitut_record(PASSFILE, &newinfo, sizeof(newinfo), unum);
	}
	clear();
	return 0;
}

//ÓëInformationÏà¹ØÁª.ÔÚcomm_list.cÀï,ÓÃÓÚÏÔÊ¾ºÍÉè¶¨¸öÈË×ÊÁÏ
void x_info() {
	if (!strcmp("guest", currentuser.userid))
		return;
	modify_user_mode(GMENU);
	disply_userinfo(&currentuser);
	uinfo_query(&currentuser, 0, usernum);
}

//	¸ü¸ÄÓÃ»§×ÊÁÏÖĞÄ³ÓòËù¶ÔÓ¦Éè¶¨
void getfield(int line, char *info, char *desc, char *buf, int len) {
	char prompt[STRLEN];
	sprintf(genbuf, "  Ô­ÏÈÉè¶¨: %-40.40s [1;32m(%s)[m",
			(buf[0] == '\0') ? "(Î´Éè¶¨)" : buf, info);
	move(line, 0);
	prints("%s", genbuf);
	sprintf(prompt, "  %s: ", desc);
	getdata(line + 1, 0, prompt, genbuf, len, DOECHO, YEA);
	if (genbuf[0] != '\0')
		strlcpy(buf, genbuf, len);
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

#include "fbbs/register.h"
//	ÌîĞ´ÓÃ»§×ÊÁÏ
void x_fillform() {
	char ans[5], *mesg, *ptr;
	reginfo_t ri;
	FILE *fn;

	if (!strcmp("guest", currentuser.userid))
		return;
	modify_user_mode(NEW);
	clear();
	move(2, 0);
	clrtobot();
	if (currentuser.userlevel & PERM_REGISTER) {
		prints("ÄúÒÑ¾­Íê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø, »¶Ó­¼ÓÈë±¾Õ¾µÄĞĞÁĞ.");
		pressreturn();
		return;
	}
#ifdef PASSAFTERTHREEDAYS
	if (currentuser.lastlogin - currentuser.firstlogin < 3 * 86400) {
		prints("ÄúÊ×´ÎµÇÈë±¾Õ¾Î´ÂúÈıÌì(72¸öĞ¡Ê±)...\n");
		prints("ÇëÏÈËÄ´¦ÊìÏ¤Ò»ÏÂ£¬ÔÚÂúÈıÌìÒÔºóÔÙÌîĞ´×¢²áµ¥¡£");
		pressreturn();
		return;
	}
#endif      
	if ((fn = fopen("unregistered", "rb")) != NULL) {
		while (fread(&ri, sizeof(ri), 1, fn)) {
			if (!strcasecmp(ri.userid, currentuser.userid)) {
				fclose(fn);
				prints("Õ¾³¤ÉĞÎ´´¦ÀíÄúµÄ×¢²áÉêÇëµ¥, ÄúÏÈµ½´¦¿´¿´°É.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}

	memset(&ri, 0, sizeof(ri));
	strlcpy(ri.userid, currentuser.userid, IDLEN+1);
	strlcpy(ri.email, currentuser.email, STRLEN-12);
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s ÄúºÃ, Çë¾İÊµÌîĞ´ÒÔÏÂµÄ×ÊÁÏ:\n", currentuser.userid);
		do {
			getfield(6, "ÇëÓÃÖĞÎÄ", "ÕæÊµĞÕÃû", ri.realname, NAMELEN);
		} while (strlen(ri.realname)<4);

		do {
			getfield(8, "Ñ§Ğ£Ïµ¼¶»òËùÔÚµ¥Î»", "Ñ§Ğ£Ïµ¼¶", ri.dept, STRLEN);
		} while (strlen(ri.dept)< 6);

		do {
			getfield(10, "°üÀ¨ÇŞÊÒ»òÃÅÅÆºÅÂë", "Ä¿Ç°×¡Ö·", ri.addr, STRLEN);
		} while (strlen(ri.addr)<10);

		do {
			getfield(12, "°üÀ¨¿ÉÁªÂçÊ±¼ä", "ÁªÂçµç»°", ri.phone, STRLEN);
		} while (strlen(ri.phone)<8);

		getfield(14, "Ğ£ÓÑ»á»ò±ÏÒµÑ§Ğ£", "Ğ£ ÓÑ »á", ri.assoc, STRLEN);
		mesg = "ÒÔÉÏ×ÊÁÏÊÇ·ñÕıÈ·, °´ Q ·ÅÆú×¢²á (Y/N/Quit)? [Y]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return;
		if (ans[0] != 'N' && ans[0] != 'n')
			break;
	}
	ptr = ri.realname;
	filter_ff(ptr);
	ptr = ri.addr;
	filter_ff(ptr);
	ptr = ri.dept;
	filter_ff(ptr);
#ifndef FDQUAN
	strlcpy(currentuser.email, ri.email, STRLEN-12);
#endif	
	if ((fn = fopen("unregistered", "ab")) != NULL) {
		ri.regdate= time(NULL);
		fwrite(&ri, sizeof(ri), 1, fn);
		fclose(fn);
	}
	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);
}
// deardragon 2000.09.26 over

