/*
 Pirate Bulletin Board System
 Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
 Eagles Bulletin Board System
 Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
 Guy Vega, gtvega@seabass.st.usm.edu
 Dominic Tynes, dbtynes@seabass.st.usm.edu
 Firebird Bulletin Board System
 Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
 Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */
/*
 $Id: delete.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include "bbs.h"

void mail_info(char *lastword);

int cmpuids3(int unum, const struct user_info *urec)
{
	return ((unum == urec->uid) && (uinfo.pid != urec->pid));
}

//×ÔÉ±,ÏêÇéºóĞğ
int offline() {
	int i;
	char buf[STRLEN], lastword[640];

	modify_user_mode(OFFLINE);
	clear();
	/*2003.04.22 modified by stephen to deny the user who is under punishing to suicide*/
	if (!HAS_PERM(PERM_POST)|| !HAS_PERM(PERM_MAIL)
			|| !HAS_PERM(PERM_TALK)) {
		move(3, 0);
		prints("Äú±»·â½ûÈ¨ÏŞ, ²»ÄÜËæ±ã×ÔÉ±!!!\n");
		pressreturn();
		clear();
		return;

	}
	if (HAS_PERM(PERM_SYSOPS) || HAS_PERM(PERM_BOARDS)
			|| HAS_PERM(PERM_ADMINMENU) || HAS_PERM(PERM_SEEULEVELS)) {
		move(3, 0);
		prints("ÄúÓĞÖØÈÎÔÚÉí, ²»ÄÜËæ±ã×ÔÉ±À²!!\n");
		pressreturn();
		clear();
		return;
	}
	/*2003.04.22 stephen modify end*/
	if (currentuser.stay < 86400) {
		move(1, 0);
		prints("\n\n¶Ô²»Æğ, Äú»¹Î´¹»×Ê¸ñÖ´ĞĞ´ËÃüÁî!!\n");
		prints("Çë mail ¸ø SYSOP ËµÃ÷×ÔÉ±Ô­Òò, Ğ»Ğ»¡£\n");
		pressreturn();
		clear();
		return;
	}
	getdata(1, 0, "ÇëÊäÈëÄúµÄÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
		pressreturn();
		clear();
		return;
	}
	clear();
	//   move(1, 0);
	//   prints("[1;5;31m¾¯¸æ[0;1;31m£º ×ÔÉ±áá, Äú½«ÎŞ·¨ÔÙÓÃ´ËÕÊºÅ½øÈë±¾Õ¾£¡£¡");
	move(3, 0);
	//   prints("[1;32m±¾Õ¾Õ¾ÎñÃ»ÓĞÒåÎñÎªÄú»Ö¸´ÕÊºÅ¡£ºÃÄÑ¹ıà¸ :( .....[m");
	//   move(5,0);
	i = 0;
	if (askyn("ÕæÊÇÉá²»µÃÄã£¬Äã×ßÖ®Ç°ÓĞÊ²Ã´»°ÏëËµÃ´", NA, NA)==YEA) {
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
			prints("°¥£¬Äã»¹ÊÇÊ²Ã´¶¼²»Ô¸ÒâËµ£¬ÊÇ²»ÊÇ»¹ÓĞĞÄË¼Î´ÁË£¿");
		else if (i <= 4)
			prints("¿´×ÅÄãã¾ã²µÄÁ³£¬ÎÒĞÄ¶¼ËéÁË ... ");
		else
			prints("ÎÒ»á¼ÇµÃÄãµÄ£¬ÅóÓÑ£¬ÎÒÒ²ÖªµÀÄãµÄÀë¿ªÒ²ÊÇÃ»ÓĞ°ì·¨µÄÊÂ£¬ºÃ×ßÁË");
	} else {
		strcpy(lastword, "> ......\n\n");
	}
	move(i + 10, 0);
	if (askyn("ÄãÈ·¶¨ÒªÀë¿ªÕâ¸ö´ó¼ÒÍ¥", NA, NA) == 1) {
		clear();
		{
			struct user_info uin;
			if (search_ulist(&uin, cmpuids3, usernum)) {
				if (!uin.active || (uin.pid && bbskill(&uin, 0) == -1))
					;
				else if (uin.pid)
					bbskill(&uin, SIGHUP);
			}
		}
		currentuser.userlevel = 0;
		substitut_record(PASSFILE, &currentuser, sizeof(struct userec),
				usernum);
		mail_info(lastword);
		modify_user_mode(OFFLINE);
		do_kick_user(&uinfo);
		exit(0);
	}
}

//±£´æÓÃ»§½üÆÚĞÅÏ¢
int getuinfo(FILE *fn) {
	int num;
	char buf[40];
	fprintf(fn, "\n\nËûµÄ´úºÅ     : %s\n", currentuser.userid);
	fprintf(fn, "ËûµÄêÇ³Æ     : %s\n", currentuser.username);
	fprintf(fn, "µç×ÓÓÊ¼şĞÅÏä : %s\n", currentuser.email);
	fprintf(fn, "ÕÊºÅ½¨Á¢ÈÕÆÚ : %s\n", getdatestring(currentuser.firstlogin, DATE_ZH));
	fprintf(fn, "×î½ü¹âÁÙÈÕÆÚ : %s\n", getdatestring(currentuser.lastlogin, DATE_ZH));
	fprintf(fn, "×î½ü¹âÁÙ»úÆ÷ : %s\n", currentuser.lasthost);
	fprintf(fn, "ÉÏÕ¾´ÎÊı     : %d ´Î\n", currentuser.numlogins);
	fprintf(fn, "ÎÄÕÂÊıÄ¿     : %d\n", currentuser.numposts);
	fprintf(fn, "ÉÏÕ¾×ÜÊ±Êı   : %d Ğ¡Ê± %d ·ÖÖÓ\n", currentuser.stay / 3600,
			(currentuser.stay / 60) % 60);
	strcpy(buf, "ltmprbBOCAMURS#@XLEast0123456789");
	for (num = 0; num < 30; num++)
		if (!(currentuser.userlevel & (1 << num)))
			buf[num] = '-';
	buf[num] = '\0';
	fprintf(fn, "Ê¹ÓÃÕßÈ¨ÏŞ   : %s\n\n", buf);
	return 0;
}

void mail_info(char *lastword) {
	FILE *fn;
	time_t now;
	char filename[STRLEN];

	now = time(0);
	sprintf(filename, "%s ÓÚ %s µÇ¼Ç×ÔÉ±", currentuser.userid, getdatestring(now, DATE_ZH));
	securityreport(filename, 1, 3);
	sprintf(filename, "tmp/suicide.%s", currentuser.userid);
	if ((fn = fopen(filename, "w")) != NULL) {
		fprintf(fn, "´ó¼ÒºÃ,\n\n");
		fprintf(fn, "ÎÒÊÇ %s (%s)¡£ÎÒ¼º¾­¾ö¶¨ÔÚ 15 ÌìºóÀë¿ªÕâÀïÁË¡£\n\n",
				currentuser.userid, currentuser.username);
		fprintf(fn, "×Ô %14.14s ÖÁ½ñ£¬ÎÒÒÑ¾­À´´Ë %d ´ÎÁË£¬ÔÚÕâ×Ü¼Æ %d ·ÖÖÓµÄÍøÂçÉúÃüÖĞ£¬\n",
				getdatestring(currentuser.firstlogin, DATE_ZH), currentuser.numlogins, currentuser.stay/60);
		fprintf(fn, "ÎÒÓÖÈçºÎ»áÇáÒ×ÉáÆúÄØ£¿µ«ÊÇÎÒµÃ×ßÁË...  µãµãµÎµÎ£­£­¾¡ÔÚÎÒĞÄÖĞ£¡\n\n");
		fprintf(fn, "%s", lastword);
		fprintf(fn, "ÅóÓÑÃÇ£¬Çë°Ñ %s ´ÓÄãÃÇµÄºÃÓÑÃûµ¥ÖĞÄÃµô°É¡£ÒòÎªÎÒ¼º¾­¾ö¶¨Àë¿ªÕâÀïÁË!\n\n",
				currentuser.userid);
		fprintf(fn, "»òĞíÓĞ³¯Ò»ÈÕÎÒ»á»ØÀ´µÄ¡£ ÕäÖØ!! ÔÙ¼û!!\n\n\n");
		fprintf(fn, "%s ÓÚ %s Áô.\n\n", currentuser.userid, getdatestring(now, DATE_ZH));
		fclose(fn);
		{
			char sc_title[128];
			sprintf(sc_title, "%sµÄ×ÔÉ±ÁôÑÔ...", currentuser.userid);
			Postfile(filename, "GoneWithTheWind", sc_title, 2);
			unlink(filename);
		}
	}
}

/*2003.04.22 added by stephen to add retire function
 **can give up these permisions: 1.login 2.chat 3.mail 4.post
 **use lookupuser as temp userec struct 
 */
//	½äÍø
int giveUpBBS() {
	char buf[STRLEN], genbuf[STRLEN];
	FILE *fn;
	char ans[3], day[10];
	int i, j, k, lcount, tcount;
	int id;

	lookupuser = currentuser;

	id = getuser(currentuser.userid);

	modify_user_mode(GIVEUPBBS);
	if (!HAS_PERM(PERM_REGISTER)) {
		clear();
		move(11, 28);
		prints("[1m[33mÄãÓĞ»¹Ã»ÓĞ×¢²áÍ¨¹ı£¬²»ÄÜ½äÍø£¡[m");
		pressanykey();
		return;
	}

	if (HAS_PERM(PERM_SYSOPS) || HAS_PERM(PERM_BOARDS)
			|| HAS_PERM(PERM_OBOARDS) || HAS_PERM(PERM_ANNOUNCE)) {
		clear();
		move(11, 28);
		prints("[1m[33mÄãÓĞÖØÈÎÔÚÉí£¬²»ÄÜ½äÍø£¡[m");
		pressanykey();
		return;
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
		prints("ÄãÏÖÔÚµÄ½äÍøÇé¿ö£º\n\n");
		while (!feof(fn)) {
			if (fscanf(fn, "%d %d", &i, &j) <= 0)
				break;
			switch (i) {
				case 1:
					prints("ÉÏÕ¾È¨ÏŞ");
					break;
				case 2:
					prints("·¢±íÈ¨ÏŞ");
					break;
				case 3:
					prints("ÁÄÌìÈ¨ÏŞ");
					break;
				case 4:
					prints("·¢ĞÅÈ¨ÏŞ");
					break;
			}
			sprintf(buf, "        »¹ÓĞ%dÌì\n", j - time(0) / 3600 / 24);
			outs(buf);
			lcount++;
		}
		fclose(fn);
		memset(buf, 0, STRLEN);
		pressanykey();
	}

	clear();
	move(1, 0);
	prints("ÇëÑ¡Ôñ½äÍøÖÖÀà:");
	move(3, 0);
	prints("(0) - ½áÊø");
	move(4, 0);
	prints("(1) - ÉÏÕ¾È¨ÏŞ");
	move(5, 0);
	prints("(2) - ·¢±íÈ¨ÏŞ");
	move(6, 0);
	prints("(3) - ÁÄÌìÈ¨ÏŞ");
	move(7, 0);
	prints("(4) - ·¢ĞÅÈ¨ÏŞ");

	getdata(10, 0, "ÇëÑ¡Ôñ [0]", ans, 2, DOECHO, NULL);
	if (ans[0] < '1' || ans[0] > '4') {
		return;
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
		prints("\n\nÄãÒÑ¾­Ã»ÓĞÁË¸ÃÈ¨ÏŞ");
		pressanykey();
		return;
	}

	getdata(11, 0, "ÇëÊäÈë½äÍøÌìÊı [0]", day, 4, DOECHO, NULL);
	i = 0;
	while (day[i]) {
		if (!isdigit(day[i]))
			return;
		i++;
	}
	j = atoi(day);
	if (j <= 0)
		return;

	if (compute_user_value(&lookupuser) <= j) {
		prints("\n\n¶Ô²»Æğ£¬ÌìÊı²»¿ÉÒÔ´óÓÚÉúÃüÁ¦...");
		pressanykey();
		return;
	}

	j = time(0) / 3600 / 24 + j;

	move(13, 0);

	if (askyn("ÄãÈ·¶¨Òª½äÍøÂğ£¿", 0) == 1) {
		getdata(15, 0, "ÇëÊäÈëÃÜÂë: ", buf, 39, NOECHO, NULL);
		if (*buf == '\0' || !checkpasswd(lookupuser.passwd, buf)) {
			prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
			pressanykey();
			return;
		}

		sethomefile(genbuf, lookupuser.userid, "giveupBBS");
		fn = fopen(genbuf, "at");
		if (!fn) {
			prints("\n\nÓÉÓÚÏµÍ³ÎÊÌâ£¬ÏÖÔÚÄã²»ÄÜ½äÍø");
			pressanykey();
			return;
		}
		fprintf(fn, "%d %d\n", ans[0] - 48, j);
		fclose(fn);

		char *str = getdatestring(time(NULL), DATE_ZH);
		switch (ans[0]) {
			case '1':
				lookupuser.userlevel &= ~PERM_LOGIN;
				sprintf(buf, "%s ÓÚ %14.14s ½ä %sÈ¨ÏŞ %d Ìì¡£",
						lookupuser.userid, str, "ÉÏÕ¾", atoi(day));
				break;
			case '2':
				lookupuser.userlevel &= ~PERM_POST;
				sprintf(buf, "%s ÓÚ %14.14s ½ä %sÈ¨ÏŞ %d Ìì¡£",
						lookupuser.userid, str, "·¢ÎÄ", atoi(day));
				break;
			case '3':
				lookupuser.userlevel &= ~PERM_TALK;
				sprintf(buf, "%s ÓÚ %14.14s ½ä %sÈ¨ÏŞ %d Ìì¡£",
						lookupuser.userid, str, "ÁÄÌì", atoi(day));
				break;
			case '4':
				lookupuser.userlevel &= ~PERM_MAIL;
				sprintf(buf, "%s ÓÚ %14.14s ½ä %sÈ¨ÏŞ %d Ìì¡£",
						lookupuser.userid, str, "ĞÅ¼ş", atoi(day));
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

		prints("\n\nÄãÒÑ¾­¿ªÊ¼½äÍøÁË");

		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);

		memset(buf, 0, STRLEN);
		memset(day, 0, 10);

		pressanykey();
		if (ans[0] == '1')
			abort_bbs(0);

		memset(ans, 0, 3);
	}
}
/*2003.04.22 stephen add end*/
