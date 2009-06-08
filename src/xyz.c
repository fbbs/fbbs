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
 $Id: xyz.c 325 2006-10-27 15:38:46Z danielfree $
 */

#define EXTERN
#include "bbs.h"
int use_define = 0;
int child_pid = 0;
extern int iscolor;
extern int enabledbchar;

#ifdef ALLOWSWITCHCODE
extern int switch_code ();
extern int convcode;
#endif

extern struct UCACHE *uidshm;
#define TH_LOW	10
#define TH_HIGH	15

/* Function : if host can use bbsnet return 0, else return 1
 * Writer:    roly
 * Data:      02.02.20
 */
//"etc/bbsnetip"
//      ÅĞ¶ÏfromhostÕâ¸öµØÖ·¿É·ñÊ¹ÓÃbbsnet,Ëü´ÓbbsnetipÖĞ¶ÁÈ¡²»¿É·ÃÎÊÁĞ±í 
//      ÆäÖĞ,ÒÔ#¿ªÍ·µÄ±íÊ¾×¢ÊÍ,ºöÂÔ
//               ÒÔ!¿ªÍ·µÄ±íÊ¾²»¿ÉÊ¹ÓÃ
//               ·ñÔò,·µ»Ø´óÓÚÁãµÄÖµ
int canbbsnet(char *bbsnetip, char *fromhost) {
	FILE *fp;
	char buf[STRLEN], ptr2[STRLEN], *ptr, *ch;
	int canflag;
	ptr = fromhost;

	if (!strcasecmp(fromhost, "127.0.0.1"))
		return 1;
	if (!strcasecmp(fromhost, "localhost"))
		return 1;

	if ((fp = fopen(bbsnetip, "r")) != NULL) {
		strtolower(ptr2, fromhost);
		while (fgets(buf, STRLEN, fp) != NULL) {
			ptr = strtok(buf, " \n\t\r");
			if (ptr != NULL && *ptr != '#') {
				canflag = (*ptr == '!') ? 0 : 1;
				if (!canflag)
					ptr++;
				ch = ptr;
				while (*ch != '\0') {
					if (*ch == '*')
						break;
					ch++;
				}
				*ch = '\0';
				if (!strncmp(ptr2, ptr, strlen(ptr))) {
					fclose(fp);
					return canflag;
				} //if !strncmp                  
			} //if ptr!=NULL
		} //while
		fclose(fp);
	}
	return 0;
}

//¸ü¸ÄÓÃ»§ Ä£Ê½×´Ì¬ÖÁmode
int modify_user_mode(int mode) {
	uinfo.mode = mode;
	update_ulist(&uinfo, utmpent);
	return 0; //sdjfielsdfje
}

//      ¶ÔÓÚÈ¨ÏŞ¶¨ÒåÖµ,ÅĞ¶ÏÆäµÚiÎ»ÊÇ·ñÎªÕæ,²¢¸ù¾İuse_defineµÄÖµÀ´
//      µ÷ÕûÆä¶ÔÓ¦Î»µÄÈ¨ÏŞÏÔÊ¾×Ö·û´®
//      ×îºóÔÚÓÉiÖ¸Ê¾µÄÎ»ÖÃ´¦ÏÔÊ¾,¸üĞÂ
int showperminfo(int pbits, int i) {
	char buf[STRLEN];
	sprintf(buf, "%c. %-30s %2s", 'A' + i,
			(use_define) ? user_definestr[i] : permstrings[i], ((pbits
					>> i) & 1 ? "ÊÇ" : "¡Á"));
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	prints(buf);
	refresh();
	return YEA;
}

//      ¸ü¸ÄÓÃ»§µÄÈ¨ÏŞÉè¶¨
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) ()) {
	int lastperm = numbers - 1;
	int i, done = NA;
	char choice[3], buf[80];
	move(4, 0);
	prints("Çë°´ÏÂÄúÒªµÄ´úÂëÀ´Éè¶¨%s£¬°´ Enter ½áÊø.\n", prompt);
	move(6, 0);
	clrtobot();
	for (i = 0; i <= lastperm; i++) {
		(*showfunc)(pbits, i, NA);
	}
	while (!done) {
		sprintf(buf, "Ñ¡Ôñ(ENTER ½áÊø%s): ",
				((strcmp(prompt, "È¨ÏŞ") != 0)) ? "" : "£¬0 Í£È¨");
		getdata(t_lines - 1, 0, buf, choice, 2, DOECHO, YEA);
		*choice = toupper(*choice);
		/*		if (*choice == '0')
		 return (0);
		 else modified by kit,rem 0Í£È¨* remed all by Amigo 2002.04.03*/
		if (*choice == '\n' || *choice == '\0')
		done = YEA;
		else if (*choice < 'A' || *choice> 'A' + lastperm)
		bell ();
		else {
			i = *choice - 'A';
			pbits ^= (1 << i);
			if ((*showfunc) (pbits, i, YEA) == NA) {
				pbits ^= (1 << i);
			} //if
		} //else
	}
	//while !done
	return (pbits);
}

//      pagerÓëmsgÉè¶¨
//
int x_userdefine() {
	int id;
	unsigned int newlevel;
	modify_user_mode(USERDEF);
	if (!(id = getuser(currentuser.userid))) {
		move(3, 0);
		prints("´íÎóµÄÊ¹ÓÃÕß ID...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	use_define = 1;
	newlevel = setperms(lookupuser.userdefine, "²ÎÊı", NUMDEFINES,
			showperminfo);
	move(2, 0);
	if (newlevel == lookupuser.userdefine)
		prints("²ÎÊıÃ»ÓĞĞŞ¸Ä...\n");
	else {
#ifdef ALLOWSWITCHCODE
		if ((!convcode && !(newlevel & DEF_USEGB))
				|| (convcode && (newlevel & DEF_USEGB)))
		switch_code ();
#endif
		lookupuser.userdefine = newlevel;
		currentuser.userdefine = newlevel;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		uinfo.pager |= FRIEND_PAGER;
		if (!(uinfo.pager & ALL_PAGER)) {
			if (!DEFINE(DEF_FRIENDCALL))
				uinfo.pager &= ~FRIEND_PAGER;
		}
		uinfo.pager &= ~ALLMSG_PAGER;
		uinfo.pager &= ~FRIENDMSG_PAGER;
		/* Following line added by Amigo 2002.04.03. For close logoff msg. */
		uinfo.pager &= ~LOGOFFMSG_PAGER;
		if (DEFINE(DEF_DELDBLCHAR))
			enabledbchar = 1;
		else
			enabledbchar = 0;
		uinfo.from[22] = DEFINE(DEF_NOTHIDEIP) ? 'S' : 'H';
		if (DEFINE(DEF_FRIENDMSG)) {
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		if (DEFINE(DEF_ALLMSG)) {
			uinfo.pager |= ALLMSG_PAGER;
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		/* Following 3 lines added by Amigo 2002.04.03. For close logoff msg. */
		if (DEFINE(DEF_LOGOFFMSG)) {
			uinfo.pager |= LOGOFFMSG_PAGER;
		}
		update_ulist(&uinfo, utmpent);
		prints("ĞÂµÄ²ÎÊıÉè¶¨Íê³É...\n\n");
	}
	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	pressreturn();
	clear();
	use_define = 0;
	return 0;
}

//¸ü¸ÄÒşÉíÊõÉèÖÃ,get_statusÀïËÆºõ´íÁË,ÕâÀïÒÔÇ°¸ü¸Äuinfo.invisibleµÄ
int x_cloak() {
	modify_user_mode(GMENU);
	report("toggle cloak", currentuser.userid);
	uinfo.invisible = (uinfo.invisible) ? NA : YEA;
	//add by infotech for get_status        04.11.29
	if (uinfo.invisible == YEA) {
		uidshm->passwd[uinfo.uid - 1].flags[0] |= CLOAK_FLAG;
	} else {
		uidshm->passwd[uinfo.uid - 1].flags[0] &= ~CLOAK_FLAG;
	}
	//end add
	update_ulist(&uinfo, utmpent);
	if (!uinfo.in_chat) {
		move(1, 0);
		clrtoeol();
		prints("ÒşÉíÊõ (cloak) ÒÑ¾­%sÁË!", (uinfo.invisible) ? "Æô¶¯" : "Í£Ö¹");
		pressreturn();
	}
	return 0;
}

//ĞŞ¸ÄÓÃ»§µÄµµ°¸
void x_edits() {
	int aborted;
	char ans[7], buf[STRLEN];
	int ch, num, confirm;
	extern int WishNum;
	static char *e_file[] = { "plans", "signatures", "notes", "logout",
			"GoodWish", NULL };
	static char *explain_file[] = { "¸öÈËËµÃ÷µµ", "Ç©Ãûµµ", "×Ô¼ºµÄ±¸ÍüÂ¼", "ÀëÕ¾µÄ»­Ãæ",
			"µ×²¿Á÷¶¯ĞÅÏ¢", NULL };
	modify_user_mode(GMENU);
	clear();
	move(1, 0);
	prints("±àĞŞ¸öÈËµµ°¸\n\n");
	for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
		prints("[[1;32m%d[m] %s\n", num + 1, explain_file[num]);
	}
	prints("[[1;32m%d[m] ¶¼²»Ïë¸Ä\n", num + 1);

	getdata(num + 5, 0, "ÄúÒª±àĞŞÄÄÒ»Ïî¸öÈËµµ°¸: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n'
			|| ans[0] == '\0')
		return;

	ch = ans[0] - '0' - 1;
	setuserfile(genbuf, e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("ÄúÈ·¶¨ÒªÉ¾³ıÕâ¸öµµ°¸", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
			pressreturn();
			clear();
			return;
		}
		unlink(genbuf);
		move(5, 0);
		prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
		sprintf(buf, "delete %s", explain_file[ch]);
		report(buf, currentuser.userid);
		pressreturn();
		if (ch == 4) {
			WishNum = 9999;
		}
		clear();
		return;
	}
	modify_user_mode(EDITUFILE);
	aborted = vedit(genbuf, NA, YEA);
	clear();
	if (!aborted) {
		prints("%s ¸üĞÂ¹ı\n", explain_file[ch]);
		sprintf(buf, "edit %s", explain_file[ch]);
		if (!strcmp(e_file[ch], "signatures")) {
			set_numofsig();
			prints("ÏµÍ³ÖØĞÂÉè¶¨ÒÔ¼°¶ÁÈëÄúµÄÇ©Ãûµµ...");
		}
		report(buf, currentuser.userid);
	} else {
		prints("%s È¡ÏûĞŞ¸Ä\n", explain_file[ch]);
	}
	pressreturn();
	if (ch == 4) {
		WishNum = 9999;
	}
}

//È¡µÃgenbufÖĞ±£´æµÄÓÃ»§ËùÔÚµÄ¼ÇÂ¼Î»ÖÃµ½*idÖĞ,ÎªÁã±íÊ¾²»´æÔÚ
int gettheuserid(int x, char *title, int *id) {
	move(x, 0);
	usercomplete(title, genbuf);
	if (*genbuf == '\0') {
		clear();
		return 0;
	}
	if (!(*id = getuser(genbuf))) {
		move(x + 3, 0);
		prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	return 1;
}

// µÃµ½ÓÃ»§ÊäÈëµÄÌÖÂÛÇøÃû,½«ÌÖÂÛÇøµÄÎ»ÖÃ±£´æÔÚposÖĞ·µ»Ø,
//              ·µ»ØÖµÎª1±íÊ¾³É¹¦,Îª0Ê§°Ü
int gettheboardname(int x, char *title, int *pos, struct boardheader *fh,
		char *bname, int mode) {
	extern int cmpbnames();
	move(x, 0);
	make_blist(mode);
	namecomplete(title, bname);
	if (*bname == '\0') {
		return 0;
	}
	*pos = search_record(BOARDS, fh, sizeof(struct boardheader),
			cmpbnames, bname);
	if (!(*pos)) {
		move(x + 3, 0);
		prints("²»ÕıÈ·µÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return 0;
	}
	return 1;
}

//ËøÆÁ,²¢½«ÓÅÏÈ¼¶ÉèÎª×îµÍ(19)
int x_lockscreen() {
	char buf[PASSLEN + 1];

	modify_user_mode(LOCKSCREEN);
	move(9, 0);
	clrtobot();
	buf[0] = '\0';
	move(9, 0);
	prints("[1;37m");
	prints("\n       _       _____   ___     _   _   ___     ___       __");
	prints("\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |");
	prints("\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |");
	prints("\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |");
	prints("\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|");
	prints("\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|\n");
	prints("\n\033[1;36mÆÁÄ»ÒÑÔÚ\033[33m %s\033[36m Ê±±»%sÔİÊ±Ëø×¡ÁË...\033[m",
			getdatestring(time(NULL), DATE_ZH), currentuser.userid);
	nice(19);
	while (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		move(18, 0);
		clrtobot();
		getdata(19, 0, "ÇëÊäÈëÄúµÄÃÜÂëÒÔ½âËø: ", buf, PASSLEN, NOECHO, YEA);
	}
	nice(0);
	return FULLUPDATE;
}

//      ¼ì²é¸ººÉ
int heavyload() {
#ifdef chkload
	register int load;
	register time_t uptime;

	if (time (0)> uptime) { //Ã¿¸ô45·ÖÖÓ¼ì²éÒ»´Î?
		load = chkload (load ? TH_LOW : TH_HIGH);
		uptime = time (0) + load + 45;
	}
	return load;
#else
	return 0;
#endif
}

//#define MY_DEBUG
//  Ö´ĞĞÃüÁîcmdfile,²ÎÊıÎªparam1
void exec_cmd(int umode, int pager, char *cmdfile, char *param1) {
	char buf[160];
	char *my_argv[18], *ptr;
	int save_pager, i;

	signal(SIGALRM, SIG_IGN);
	modify_user_mode(umode);
	clear();
	move(2, 0);
	if (num_useshell() > MAX_USESHELL) {
		prints("Ì«¶àÈËÊ¹ÓÃÍâ²¿³ÌÊ½ÁË£¬ÄúµÈÒ»ÏÂÔÙÓÃ°É...");
		pressanykey();
		return;
	}
	if (!HAS_PERM(PERM_SYSOPS) && heavyload()) {
		clear();
		prints("±§Ç¸£¬Ä¿Ç°ÏµÍ³¸ººÉ¹ıÖØ£¬´Ë¹¦ÄÜÔİÊ±²»ÄÜÖ´ĞĞ...");
		pressanykey();
		return;
	}
	if (!dashf(cmdfile)) {
		prints("ÎÄ¼ş [%s] ²»´æÔÚ£¡\n", cmdfile);
		pressreturn();
		return;
	}
	save_pager = uinfo.pager;
	if (pager == NA) {
		uinfo.pager = 0;
	}
	sprintf(buf, "%s %s %s %d", cmdfile, param1, currentuser.userid,
			getpid());
	report(buf, currentuser.userid);
	my_argv[0] = cmdfile;
	strcpy(buf, param1);
	if (buf[0] != '\0')
		ptr = strtok(buf, " \t");
	else
		ptr = NULL;
	for (i = 1; i < 18; i++) {
		if (ptr) {
			my_argv[i] = ptr;
			ptr = strtok(NULL, " \t");
		} else {
			my_argv[i] = NULL;
		}
	}
#ifdef MY_DEBUG
	for (i = 0; i < 18; i++) {
		if (my_argv[i] != NULL)
		prints ("my_argv[%d] = %s\n", i, my_argv[i]);
		else
		prints ("my_argv[%d] = (none)\n", i);
	}
	pressanykey ();
#else
	child_pid = fork();
	if (child_pid == -1) {
		prints("×ÊÔ´½ôÈ±£¬fork() Ê§°Ü£¬ÇëÉÔºóÔÙÊ¹ÓÃ");
		child_pid = 0;
		pressreturn();
		return;
	}
	if (child_pid == 0) {
		execv(cmdfile, my_argv);
		exit(0);
	} else {
		waitpid(child_pid, NULL, 0);
	}
#endif
	child_pid = 0;
	uinfo.pager = save_pager;
	clear();
}

//²éÑ¯Ê¹ÓÃÕß×ÊÁÏ
void x_showuser() {
	char buf[STRLEN];
	modify_user_mode(SYSINFO);
	clear();
	stand_title("±¾Õ¾Ê¹ÓÃÕß×ÊÁÏ²éÑ¯");
	ansimore("etc/showuser.msg", NA);
	getdata(20, 0, "Parameter: ", buf, 30, DOECHO, YEA);
	if ((buf[0] == '\0') || dashf("tmp/showuser.result"))
		return;
	securityreport("²éÑ¯Ê¹ÓÃÕß×ÊÁÏ", 0, 0);
	exec_cmd(SYSINFO, YEA, "bin/showuser", buf);
	sprintf(buf, "tmp/showuser.result");
	if (dashf(buf)) {
		mail_file(buf, currentuser.userid, "Ê¹ÓÃÕß×ÊÁÏ²éÑ¯½á¹û");
		unlink(buf);
	}
}

//added by iamfat 2002.09.04
//      ´©Ëó..
int ent_bnet2() {
	char buf[80];

	if (HAS_PERM(PERM_BLEVELS) || canbbsnet("etc/bbsnetip2", uinfo.from)) {
		sprintf(buf, "etc/bbsnet2.ini %s", currentuser.userid);
		exec_cmd(BBSNET, NA, "bin/bbsnet", buf);
	} else {
		clear();
		prints("±§Ç¸£¬ÄúËù³öµÄÎ»ÖÃÎŞ·¨Ê¹ÓÃ±¾´©Ëó¹¦ÄÜ...");
		pressanykey();
		return;
	}
	return 0;
}

//      ·µ»ØÖµÎŞÒâÒå
int ent_bnet() {
	/*   
	 char buf[80];
	 sprintf(buf,"etc/bbsnet.ini %s",currentuser.userid);
	 exec_cmd(BBSNET,NA,"bin/bbsnet",buf);
	 */
	char buf[80];

	if (HAS_PERM(PERM_BLEVELS) || canbbsnet("etc/bbsnetip", uinfo.from)) {
		sprintf(buf, "etc/bbsnet.ini %s", currentuser.userid);
		exec_cmd(BBSNET, NA, "bin/bbsnet", buf);
	} else {
		clear();
		prints("±§Ç¸£¬ÓÉÓÚÄúÊÇĞ£ÄÚÓÃ»§£¬ÄúÎŞ·¨Ê¹ÓÃ±¾´©Ëó¹¦ÄÜ...\n");
		prints("ÇëÖ±½ÓÁ¬Íù¸´µ©ÈªÕ¾£ºtelnet 10.8.225.9");
		pressanykey();
		return 0;
	}
	return 0;
}

//  ÅÅÀ×ÓÎÏ·
int ent_winmine() {
	char buf[80];
	sprintf(buf, "%s %s", currentuser.userid, currentuser.lasthost);
	exec_cmd(WINMINE, NA, "so/winmine", buf);
}

//      ¼ÍÄîÈÕÏà¹Ø
void fill_date() {
	time_t now, next;
	char buf[80], buf2[30], index[5], index_buf[5], *t = NULL;
	//char   *buf, *buf2, *index, index_buf[5], *t=NULL; commented by infotech.
	char h[3], m[3], s[3];
	int foo, foo2, i;
	struct tm *mytm;
	FILE *fp;
	now = time(0);
	if (resolve_boards() < 0)
		exit(1);

	if (now < brdshm->fresh_date && strlen(brdshm->date) != 0)
		return;

	mytm = localtime(&now);
	strftime(h, 3, "%H", mytm); //0-24  Ğ¡Ê±
	strftime(m, 3, "%M", mytm); //00-59 ·ÖÖÓ
	strftime(s, 3, "%S", mytm); //00-61 ÃëÊı,61ÎªÈòÃë

	next = (time_t) time(0)
			- ((atoi(h) * 3600) + (atoi(m) * 60) + atoi(s)) + 86400; /* Ëã³ö½ñÌì 0:0:00 µÄÊ±¼ä, È»ááÔÙÍùáá¼ÓÒ»Ìì */
	sprintf(genbuf, "¼ÍÄîÈÕ¸üĞÂ, ÏÂÒ»´Î¸üĞÂÊ±¼ä %s",
			getdatestring(next, DATE_ENWEEK));
	report(genbuf, currentuser.userid);

	fp = fopen(DEF_FILE, "r");

	if (fp == NULL)
		return;

	now = time(0);
	mytm = localtime(&now);
	strftime(index_buf, 5, "%m%d", mytm);

	strcpy(brdshm->date, DEF_VALUE);

	while (fgets(buf, 80, fp) != NULL) {
		if (buf[0] == ';' || buf[0] == '#' || buf[0] == ' ')
			continue;

		buf[35] = '\0';
		strlcpy(index, buf, 4);
		index[4] = '\0';
		strcpy(buf2, buf + 5);
		t = strchr(buf2, '\n');
		if (t) {
			*t = '\0';
		}

		if (index[0] == '\0' || buf2[0] == '\0')
			continue;

		if (strcmp(index, "0000") == 0 || strcmp(index_buf, index) == 0) {
			foo = strlen(buf2);
			foo2 = (30 - foo) / 2;
			strcpy(brdshm->date, "");
			for (i = 0; i < foo2; i++)
				strcat(brdshm->date, " ");
			strcat(brdshm->date, buf2);
			for (i = 0; i < 30 - (foo + foo2); i++)
				strcat(brdshm->date, " ");
		}
	}

	fclose(fp);
	brdshm->fresh_date = next;

	//free(buf);    by infotech.
	//free(buf2);
	//free(index);

	return;
}

//  ½ñÌìÊÇÉúÈÕ?
int is_birth(struct userec user) {
	struct tm *tm;
	time_t now;

	now = time(0);
	tm = localtime(&now);

	if (strcasecmp(user.userid, "guest") == 0)
		return NA;

	if (user.birthmonth == (tm->tm_mon + 1) && user.birthday
			== tm->tm_mday)
		return YEA;
	else
		return NA;
}

//      ·¢ËÍÁôÑÔ
int sendgoodwish(char *uid) {
	return sendGoodWish(NULL);
}

int sendGoodWish(char *userid) {
	FILE *fp;
	int tuid, i, count;
	time_t now;
	char buf[5][STRLEN], tmpbuf[STRLEN];
	char uid[IDLEN + 1], *ptr, *timestr;

	modify_user_mode(GOODWISH);
	clear();
	move(1, 0);
	prints("[0;1;32mÁôÑÔ±¾[m\nÄú¿ÉÒÔÔÚÕâÀï¸øÄúµÄÅóÓÑËÍÈ¥ÄúµÄ×£¸££¬");
	prints("\nÒ²¿ÉÒÔÎªÄú¸øËû/ËıÉÓÉÏÒ»¾äÇÄÇÄ»°¡£");
	move(5, 0);
	if (userid == NULL) {
		usercomplete("ÇëÊäÈëËûµÄ ID: ", uid);
		if (uid[0] == '\0') {
			clear();
			return 0;
		}
	} else {
		strcpy(uid, userid);
	}
	if (!(tuid = getuser(uid))) {
		move(7, 0);
		prints("[1mÄúÊäÈëµÄÊ¹ÓÃÕß´úºÅ( ID )²»´æÔÚ£¡[m\n");
		pressanykey();
		clear();
		return -1;
	}
	move(5, 0);
	clrtoeol();
	prints("[m¡¾¸ø [1m%s[m ÁôÑÔ¡¿", uid);
	move(6, 0);
	prints("ÄúµÄÁôÑÔ[Ö±½Ó°´ ENTER ½áÊøÁôÑÔ£¬×î¶à 5 ¾ä£¬Ã¿¾ä×î³¤ 50 ×Ö·û]:");
	for (count = 0; count < 5; count++) {
		getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
		if (tmpbuf[0] == '\0')
			break;;
		for (ptr = tmpbuf; *ptr == ' ' && *ptr != 0; ptr++)
			;
		if (*ptr == 0) {
			count--;
			continue;
		}
		for (i = strlen(ptr) - 1; i < 0; i--)
			if (ptr[i] != ' ')
				break;
		if (i < 0) {
			count--;
			continue;
		}
		ptr[i + 1] = 0;
		strcpy(buf[count], ptr);
	}
	if (count == 0)
		return 0;
	sprintf(genbuf, "ÄúÈ·¶¨Òª·¢ËÍÕâÌõÁôÑÔ¸ø [1m%s[m Âğ", uid);
	move(9 + count, 0);
	if (askyn(genbuf, YEA, NA) == NA) {
		clear();
		return 0;
	}
	sethomefile(genbuf, uid, "GoodWish");
	if ((fp = fopen(genbuf, "a")) == NULL) {
		prints("ÎŞ·¨¿ªÆô¸ÃÓÃ»§µÄµ×²¿Á÷¶¯ĞÅÏ¢ÎÄ¼ş£¬ÇëÍ¨ÖªÕ¾³¤...\n");
		pressanykey();
		return NA;
	}
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 5) = '\0';
	for (i = 0; i < count; i++) {
		fprintf(fp, "%s(%s)[%d/%d]£º%s\n", currentuser.userid, timestr, i
				+ 1, count, buf[i]);
	}
	fclose(fp);
	sethomefile(genbuf, uid, "HaveNewWish");
	if ((fp = fopen(genbuf, "w+")) != NULL) {
		fputs("Have New Wish", fp);
		fclose(fp);
	}
	move(11 + count, 0);
	prints("ÒÑ¾­°ïÄúËÍ³öÄúµÄÁôÑÔÁË¡£");
	sprintf(genbuf, "SendGoodWish to %s", uid);
	report(genbuf, currentuser.userid);
	pressanykey();
	clear();
	return 0;
}
