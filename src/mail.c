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
 $Id: mail.c 369 2007-05-12 17:18:27Z danielfree $
 */

extern struct postheader header;
#include "bbs.h"

/*For read.c*/
int auth_search_down();
int auth_search_up();
int do_cross();
int edit_post();
int Import_post();
int Save_post();
int t_search_down();
int t_search_up();
int post_search_down();
int post_search_up();
int thread_up();
int thread_down();
/*int     deny_user();*/
int into_myAnnounce();
int show_user_notes();
int show_allmsgs();
int show_author();
int SR_first_new();
int SR_last();
int SR_first();
int SR_read();
int SR_author();
int Q_Goodbye();
int t_friends();
int s_msg();
int G_SENDMODE = NA;
int show_file_info();
int send_msg();
extern char quote_file[], quote_user[];
char currmaildir[STRLEN];
#define maxrecp 300

int chkmail() {
	static long lasttime = 0;
	static ismail = 0;
	struct fileheader fh;
	struct stat st;
	int fd, size;
	register int i, offset;
	register long numfiles;
	unsigned char ch;
	extern char currmaildir[STRLEN];
	if (!HAS_PERM(PERM_LOGIN)) {
		return 0;
	}
	size = sizeof(struct fileheader);
	offset = (int) ((char *) &(fh.accessed[0]) - (char *) &(fh));
	if ((fd = open(currmaildir, O_RDONLY)) < 0)
		return (ismail = 0);
	fstat(fd, &st);
	if (lasttime >= st.st_mtime) {
		close(fd);
		return ismail;
	}
	lasttime = st.st_mtime;
	numfiles = st.st_size;
	numfiles = numfiles / size;
	if (numfiles <= 0) {
		close(fd);
		return (ismail = 0);
	}
	lseek(fd, (off_t) (st.st_size - (size - offset)), SEEK_SET);
	for (i = 0; i < numfiles; i++) {
		read(fd, &ch, 1);
		if (!(ch & FILE_READ)) {
			close(fd);
			return (ismail = 1);
		}
		lseek(fd, (off_t) (-size - 1), SEEK_CUR);
	}
	close(fd);
	return (ismail = 0);
}

int
check_query_mail(qry_mail_dir)
char qry_mail_dir[STRLEN];
{
	struct fileheader fh;
	struct stat st;
	int fd, size;
	register int offset;
	register long numfiles;
	unsigned char ch;
	offset = (int) ((char *) &(fh.accessed[0]) - (char *) &(fh));
	if ((fd = open(qry_mail_dir, O_RDONLY)) < 0)
	return 0;
	fstat(fd, &st);
	numfiles = st.st_size;
	size = sizeof(struct fileheader);
	numfiles = numfiles / size;
	if (numfiles <= 0) {
		close(fd);
		return 0;
	}
	lseek(fd, (off_t) (st.st_size - (size - offset)), SEEK_SET);
	/*    for(i = 0 ; i < numfiles ; i++) {
	 read(fd,&ch,1) ;
	 if(!(ch & FILE_READ)) {
	 close(fd) ;
	 return YEA ;
	 }
	 lseek(fd,(off_t)(-size-1),SEEK_CUR);
	 }*/
	/*ÀëÏß²éÑ¯ĞÂĞÅÖ»Òª²éÑ¯×îááÒ»·âÊÇ·ñÎªĞÂĞÅ£¬ÆäËû²¢²»ÖØÒª*/
	/*Modify by SmallPig*/
	read(fd, &ch, 1);
	if (!(ch & FILE_READ)) {
		close(fd);
		return YEA;
	}
	close(fd);
	return NA;
}

int mailall() {
	char ans[4], fname[STRLEN], title[STRLEN];
	char doc[5][STRLEN], buf[STRLEN];
	int i;
	strcpy(title, "Ã»Ö÷Ìâ");
	modify_user_mode(SMAIL);
	clear();
	move(0, 0);
	sprintf(fname, "tmp/mailall.%s", currentuser.userid);
	prints("ÄãÒª¼Ä¸øËùÓĞµÄ£º\n");
	prints("(0) ·ÅÆú\n");
	strcpy(doc[0], "(1) ÉĞÎ´Í¨¹ıÉí·İÈ·ÈÏµÄÊ¹ÓÃÕß");
	strcpy(doc[1], "(2) ËùÓĞÍ¨¹ıÉí·İÈ·ÈÏµÄÊ¹ÓÃÕß");
	strcpy(doc[2], "(3) ËùÓĞµÄ°æÖ÷");
	strcpy(doc[3], "(4) ËùÓĞÏÖÈÎÕ¾Îñ");
	strcpy(doc[4], "(5) ÏÖÈÎÕ¾ÎñÒÔ¼°ÀëÈÎÕ¾Îñ");
	for (i = 0; i < 5; i++)
		prints("%s\n", doc[i]);
	getdata(8, 0, "ÇëÊäÈëÄ£Ê½ (0~5)? [0]: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' < 1 || ans[0] - '0' > 5) {
		return NA;
	}
	sprintf(buf, "ÊÇ·ñÈ·¶¨¼Ä¸ø%s ", doc[ans[0] - '0' - 1]);
	move(9, 0);
	if (askyn(buf, NA, NA) == NA)
		return NA;
	in_mail = YEA;
	header.reply_mode = NA;
	strcpy(header.title, "Ã»Ö÷Ìâ");
	strcpy(header.ds, doc[ans[0] - '0' - 1]);
	header.postboard = NA;
	i = post_header(&header);
	if (i == -1)
		return NA;
	if (i == YEA)
		sprintf(save_title, "[Type %c ¹«¸æ] %.60s", ans[0], header.title);
	setquotefile("");

	/***********Modified by Ashinmarch on 08.3.30 to improve Type 2 mailall*******************/
	/***********Type 2µÄÈºĞÅ¸ÄÎª¹²ÏíÎÄ¼şµÄĞÎÊ½£¬ Ä¿µÄ¼õÉÙÎÄ¼şµÄ¿½±´£¬·ÀÖ¹ËÀ»ú*****************/
	/***********Ïà¹Ø¸Ä¶¯ÎÄ¼ş£ºlist.c, bbs.c***************************************************/
	if (ans[0] - '0' == 2)
		sprintf(fname, "sharedmail/mailall.%s.%d", currentuser.userid,
				time(0));
	/**********Modified end**********/
	do_quote(quote_file, fname, header.include_mode);
	if (vedit(fname, YEA, YEA) == -1) {
		in_mail = NA;
		unlink(fname);
		clear();
		return -2;
	}
	move(t_lines - 1, 0);
	clrtoeol();
	prints("[5;1;32;44mÕıÔÚ¼Ä¼şÖĞ£¬ÇëÉÔºò.....                                                        [m");
	refresh();
	/****modify function: Add a parameter fname*****/
	mailtoall(ans[0] - '0', fname);
	/****end****/
	move(t_lines - 1);
	clrtoeol();
	/****type 2¹²ÏíÎÄ¼ş²»ĞèÒªÉ¾³ı****/
	if (ans[0] - '0' != 2)
		unlink(fname);
	in_mail = NA;
	return 0;
}

#ifdef INTERNET_EMAIL

void
m_internet()
{
	char receiver[68];
	modify_user_mode(SMAIL);
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		clear();
		move(4,0);
		prints("\n\n        ÄúÉĞÎ´Íê³É×¢²á£¬»òÕß·¢ËÍĞÅ¼şµÄÈ¨ÏŞ±»·â½û¡£");
		pressreturn();
		return;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return;
	}

	getdata(1, 0, "ÊÕĞÅÈËE-mail£º", receiver, 65, DOECHO, YEA);
	strtolower(genbuf, receiver);
	if (strstr(genbuf, ".bbs@"BBSHOST)
			|| strstr(genbuf, ".bbs@localhost")) {
		move(3, 0);
		prints("Õ¾ÄÚĞÅ¼ş, ÇëÓÃ (S)end Ö¸ÁîÀ´¼Ä\n");
		pressreturn();
	} else if (!invalidaddr(receiver)) {
		*quote_file = '\0';
		clear();
		do_send(receiver, NULL);
	} else {
		move(3, 0);
		prints("ÊÕĞÅÈË²»ÕıÈ·, ÇëÖØĞÂÑ¡È¡Ö¸Áî\n");
		pressreturn();
	}
	clear();
	refresh();
}
#endif

int
do_send(userid, title)
char *userid, *title;
{

	int lookupuserlevel; //added by roly 02.03.25
	struct fileheader newmessage;
	struct override fh;
	struct stat st;
	char filepath[STRLEN], fname[STRLEN], *ip;
	char save_title2[STRLEN];
	int fp, count, result;
	int internet_mail = 0;
	char tmp_fname[STRLEN];
	extern int cmpfnames();

	int maxmail;

	/* I hate go to , but I use it again for the noodle code :-) */
	if (strchr(userid, '@')) {
		internet_mail = YEA;
		sprintf(tmp_fname, "tmp/imail.%s.%05d", currentuser.userid, uinfo.pid);
		strcpy(filepath, tmp_fname);
		goto edit_mail_file;
	}
	/* end of kludge for internet mail */

	if (!getuser(userid))
	return -1;
	if (!(lookupuser.userlevel & PERM_READMAIL))
	return -3;

	sethomefile(filepath, userid, "rejects");
	if(search_record(filepath, &fh, sizeof(fh), cmpfnames, currentuser.userid))return -5;
	if(getmailboxsize(lookupuser.userlevel)*2<getmailsize(lookupuser.userid))
	return -4;

	/* added by roly 02.03.10*/

	lookupuserlevel=lookupuser.userlevel;
	maxmail = getmailboxhold(lookupuserlevel);
	if (getmailnum(lookupuser.userid)> maxmail*2)
	return -4;
	/* add end */
	sprintf(filepath, "mail/%c/%s", toupper(userid[0]), userid);
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0755) == -1)
		return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
		return -1;
	}
	memset(&newmessage, 0, sizeof(newmessage));
	sprintf(fname, "M.%d.A", time(NULL));
	sprintf(filepath, "mail/%c/%s/%s", toupper(userid[0]), userid, fname);
	ip = strrchr(fname, 'A');
	count = 0;
	while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		if (*ip == 'Z')
		ip++, *ip = 'A', *(ip + 1) = '\0';
		else
		(*ip)++;
		sprintf(filepath, "mail/%c/%s/%s", toupper(userid[0]), userid, fname);
		if (count++> MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fp);
	strcpy(newmessage.filename, fname);
	sprintf(genbuf, "%s", currentuser.userid);
	strlcpy(newmessage.owner, genbuf, STRLEN);
	sprintf(filepath, "mail/%c/%s/%s", toupper(userid[0]), userid, fname);

	edit_mail_file:
	if (title == NULL) {
		header.reply_mode = NA;
		strcpy(header.title, "Ã»Ö÷Ìâ");
	} else {
		header.reply_mode = YEA;
		strcpy(header.title, title);
	}
	header.postboard = NA;
	in_mail = YEA;

	setuserfile(genbuf, "signatures");
	ansimore2(genbuf, NA, 0, 18);
	strcpy(header.ds, userid);
	result = post_header(&header);
	if( result == -1 ) {
		clear();
		return -2;
	}
	if( result == YEA) {
		memcpy(newmessage.title, header.title, sizeof(header.title));
		strlcpy(save_title, newmessage.title, STRLEN);
		sprintf(save_title2, "{%.16s} %.60s", userid, newmessage.title);
		//		strncpy(save_filename, fname, 4096);
	}
	do_quote(quote_file, filepath, header.include_mode);

	if (internet_mail) {
#ifndef INTERNET_EMAIL
		prints("¶Ô²»Æğ£¬±¾Õ¾Ôİ²»Ìá¹© InterNet Mail ·şÎñ£¡");
		pressanykey();
#else
		int res;
		if (vedit(filepath, YEA, YEA) == -1) {
			unlink(filepath);
			clear();
			return -2;
		}
		clear();
		prints("ĞÅ¼ş¼´½«¼Ä¸ø %s \n", userid);
		prints("±êÌâÎª£º %s \n", header.title);
		if (askyn("È·¶¨Òª¼Ä³öÂğ", YEA, NA) == NA) {
			prints("\nĞÅ¼şÒÑÈ¡Ïû...\n");
			res = -2;
		} else {
			int filter=YEA;
#ifdef SENDMAIL_MIME_AUTOCONVERT
			int ans;
			ans = askyn("ÒÔ MIME ¸ñÊ½ËÍĞÅ", NA, NA);
			if (askyn("ÊÇ·ñ¹ıÂËANSI¿ØÖÆ·û",YEA,NA) == NA)
			filter = NA;
			if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", NA, NA) == YEA)
			mail_file(tmp_fname, currentuser.userid, save_title2);
			prints("ÇëÉÔºò, ĞÅ¼ş´«µİÖĞ...\n");
			refresh();
			res = bbs_sendmail(tmp_fname, header.title, userid, filter,ans);
#else

			if (askyn("ÊÇ·ñ¹ıÂËANSI¿ØÖÆ·û",YEA,NA) == NA)
			filter = NA;
			if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", NA, NA) == YEA)
			mail_file(tmp_fname, currentuser.userid, save_title2);
			prints("ÇëÉÔºò, ĞÅ¼ş´«µİÖĞ...\n");
			refresh();
			res = bbs_sendmail(tmp_fname, header.title, userid, filter);
#endif
		}
		unlink(tmp_fname);
		sprintf(genbuf, "mailed %s: %s", userid, header.title);
		report(genbuf, currentuser.userid);
		return res;
#endif
	} else {
		if (vedit(filepath, YEA, YEA) == -1) {
			unlink(filepath);
			clear();
			return -2;
		}

		//backup
		clear();
		if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", NA, NA) == YEA)
		mail_file(filepath, currentuser.userid, save_title2);
#if 0
		//-----add by yl to calculate the length of a mail -----
		sprintf(genbuf, "mail/%c/%s/%s", toupper(userid[0]), userid, newmessage.filename);
		if (stat(genbuf, &st) == -1)
		file_size = 0;
		else
		file_size=st.st_blksize*st.st_blocks;
		//memcpy(newmessage.filename+STRLEN-5,&file_size,4);
		sizeptr = (int*)(newmessage.filename+STRLEN-5);
		*sizeptr = file_size;
		//------------------------------------------------------
#endif

		sprintf(genbuf, "mail/%c/%s/%s", toupper(userid[0]), userid, DOT_DIR);
		if (append_record(genbuf, &newmessage, sizeof(newmessage)) == -1)
		return -1;
		sprintf(genbuf, "mailed %s: %s", userid, header.title);
		report(genbuf, currentuser.userid);
		return 0;
	}
}

int
m_send(userid)
char userid[];
{
	char uident[STRLEN];
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		clear();
		move(4,0);
		prints("\n\n        ÄúÉĞÎ´Íê³É×¢²á£¬»òÕß·¢ËÍĞÅ¼şµÄÈ¨ÏŞ±»·â½û¡£");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND
			&& uinfo.mode != GMENU) {
		move(1, 0);
		clrtoeol();
		modify_user_mode(SMAIL);
		usercomplete("ÊÕĞÅÈË£º ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		}
	} else
	strcpy(uident, userid);
	modify_user_mode(SMAIL);
	clear();
	*quote_file = '\0';
	switch (do_send(uident, NULL)) {
		case -1:
		prints("ÊÕĞÅÕß²»ÕıÈ·\n");
		break;
		case -2:
		prints("È¡Ïû\n");
		break;
		case -3:
		prints("[%s] ÎŞ·¨ÊÕĞÅ\n", uident);
		break;
		case -4:
		prints("[%s] ĞÅÏäÒÑÂú£¬ÎŞ·¨ÊÕĞÅ\n",uident);
		break;
		case -5:
		prints("[%s] ²»ÏëÊÕµ½ÄúµÄĞÅ¼ş\n",uident);
		break;
		default:
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
	}
	pressreturn();
	return FULLUPDATE;
}

int read_mail(struct fileheader *fptr) {
	/****ÅĞ¶ÏÊÇ·ñÎªsharedmail£¬Èç¹ûÊÇÔò´Ó¹²ÏíÎÄ¼ş¶ÁÈ¡****/
	if (fptr->filename[0] == 's')
		strcpy(genbuf, fptr->filename);
	else
		sprintf(genbuf, "mail/%c/%s/%s", toupper(currentuser.userid[0]),
				currentuser.userid, fptr->filename);
	ansimore(genbuf, NA);
	fptr->accessed[0] |= FILE_READ;
	return 0;
}

int mrd;

int delmsgs[1024];
int delcnt;

static int read_new_mail(void *fptrv, int index, void *arg)
{
	char done = NA, delete_it;
	char fname[256];
	if (fptrv == NULL) {
		delcnt = 0;
		return 0;
	}
	struct fileheader *fptr = (struct fileheader *)fptrv;
	//Modified by IAMFAT 2002-05-25
	if (fptr->accessed[0] & FILE_READ)
		return 0;
	mrd = 1;
	prints("¶ÁÈ¡ %s ¼ÄÀ´µÄ '%s' ?\n", fptr->owner, fptr->title);
	//prints("(Yes, or No): ");
	getdata(1, 0, "(Y)¶ÁÈ¡ (N)²»¶Á (Q)Àë¿ª [Y]: ", genbuf, 3, DOECHO, YEA);
	if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
		clear();
		return QUIT;
	}
	if (genbuf[0] != 'y' && genbuf[0] != 'Y' && genbuf[0] != '\0') {
		clear();
		return 0;
	}
	read_mail(fptr);
	strlcpy(fname, genbuf, sizeof(fname));

	//mrd = 1;
	if (substitute_record(currmaildir, fptr, sizeof(*fptr), index))
		return -1;
	delete_it = NA;
	while (!done) {
		move(t_lines - 1, 0);
		prints("(R)»ØĞÅ, (D)É¾³ı, (G)¼ÌĞø? [G]: ");
		switch (egetch()) {
			case 'R':
			case 'r':
				mail_reply(index, fptr, currmaildir);
				break;
			case 'D':
			case 'd':
				delete_it = YEA;
			default:
				done = YEA;
		}
		if (!done)
			ansimore(fname, NA); /* re-read */
	}
	if (delete_it) {
		clear();
		sprintf(genbuf, "É¾³ıĞÅ¼ş [%-.55s]", fptr->title);
		if (askyn(genbuf, NA, NA) == YEA) {
			sprintf(genbuf, "mail/%c/%s/%s",
					toupper(currentuser.userid[0]), currentuser.userid,
					fptr->filename);
			unlink(genbuf);
			delmsgs[delcnt++] = index;
		}
	}
	clear();
	return 0;
}

int m_new() {
	clear();
	mrd = 0;
	modify_user_mode(RMAIL);
	read_new_mail(NULL, 0, NULL);
	apply_record(currmaildir, read_new_mail, sizeof(struct fileheader), 0,
			1, 0, false);
	while (delcnt--)
		delete_record(currmaildir, sizeof(struct fileheader),
				delmsgs[delcnt], NULL, NULL);
	if (!mrd) {
		clear();
		move(10, 30);
		prints("ÄúÏÖÔÚÃ»ÓĞĞÂĞÅ¼ş!");
		pressanykey();
	}
	return -1;
}

extern char BoardName[];

/*
 void
 mailtitle()
 {
 showtitle("ĞÅ¼şÑ¡µ¥    ", BoardName);
 prints("Àë¿ª[[1;32m¡û[m,[1;32me[m]  Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m]  ÔÄ¶ÁĞÅ¼ş[[1;32m¡ú[m,[1;32mRtn[m]  »ØĞÅ[[1;32mR[m]  ¿³ĞÅ£¯Çå³ı¾ÉĞÅ[[1;32md[m,[1;32mD[m]  ÇóÖú[[1;32mh[m][m\n");
 prints("[1;44m ±àºÅ  %-12s %6s  %-50s[m\n", "·¢ĞÅÕß", "ÈÕ  ÆÚ", "±ê  Ìâ");
 clrtobot();
 }
 */
void mailtitle() {
	int total, used;
	total=getmailboxsize(currentuser.userlevel) ;
	used=getmailsize(currentuser.userid);
	showtitle("ĞÅ¼şÑ¡µ¥    ", BoardName);
	prints(" Àë¿ª[[1;32m¡û[m,[1;32me[m] Ñ¡Ôñ[[1;32m¡ü[m, [1;32m¡ı[m] ÔÄ¶ÁĞÅ¼ş[[1;32m¡ú[m,[1;32mRtn[m] »Ø ĞÅ[[1;32mR[m] ¿³ĞÅ£¯Çå³ı¾ÉĞÅ[[1;32md[m,[1;32mD[m] ÇóÖú[[1;32mh[m][m\n");
	//Modified by IAMFAT 2002-05-26
	//prints("[1;44m±àºÅ   ·¢ĞÅÕß       ÈÕ ÆÚ      ±êÌâ  ([33mÄúµÄĞÅÏäÈİÁ¿Îª[%4dK]£¬µ±Ç°ÒÑÓÃ[%4dK][37m) [m\n",total,used);
	prints(
			"[1;44m ±àºÅ   ·¢ĞÅÕß        ÈÕÆÚ   ±êÌâ    ([33mÄúµÄĞÅÏäÈİÁ¿Îª[%5dK]£¬µ±Ç°ÒÑÓÃ[%4dK][37m) [m\n",
			total, used);
	clrtobot() ;
}

// Check if user exceeds mail quota or max number of mails.
int check_maxmail(void)
{
	extern int mailXX;
	int maxmail, maxsize, mailsize;

	maxmail = getmailboxhold(currentuser.userlevel);

	set_safe_record();
	currentuser.nummails = get_num_records(currmaildir,
			sizeof(struct fileheader));
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	maxsize = getmailboxsize(currentuser.userlevel);
	mailsize = getmailsize(currentuser.userid);
	if (currentuser.nummails > maxmail || mailsize > maxsize) {
		mailXX = 1;
		clear();
		move(4, 0);
		if (currentuser.nummails > maxmail)
			prints("ÄúµÄË½ÈËĞÅ¼ş¸ß´ï %d ·â, ÄúµÄĞÅ¼şÉÏÏŞ: %d ·â\n",
				currentuser.nummails, maxmail);
		if (mailsize > maxsize)
			prints("ÄúµÄĞÅ¼şÈİÁ¿¸ß´ï %d K£¬ÄúµÄÈİÁ¿ÉÏÏŞ: %d K\n",
				mailsize, maxsize);
		prints("ÄúµÄË½ÈËĞÅ¼şÒÑ¾­³¬ÏŞ, ÇëÕûÀíĞÅÏä£¬"
			"·ñÔòÎŞ·¨Ê¹ÓÃ±¾Õ¾µÄËÍĞÅ¹¦ÄÜ¡£\n");
	} else
		mailXX = 0;

	return mailXX;
}


/* added end */
char * maildoent(int num, struct fileheader *ent) {
	static char buf[512];
	char b2[512];
	time_t filetime;
	char status;
	char color[10];
	char *date, *t;
	//Modified by IAMFAT 2002-05-27
	//extern char ReadPost[];
	//extern char ReplyPost[];
	extern char topic[];
	//End IAMFAT
	//Added by IAMFAT 2002-05-30
	char title[STRLEN];
	int reflag;
	//End IAMFAT
	char c1[8];
	char c2[8];
	int same = NA;
#ifdef COLOR_POST_DATE
	struct tm *mytm;
#endif
	/****ÅĞ¶ÏÊÇ·ñÊÇType2µÄ¹²ÏíÎÄ¼ş:ÎÄ¼şÃûsharedmail/mailall.$userid.$time****/
	if (ent->filename[0] == 's')
		filetime = atoi(ent->filename + strlen(ent->owner) + 20);
	else
		filetime = atoi(ent->filename + 2);
	if (filetime > 740000000) {
		date = ctime(&filetime) + 4;
	} else {
		date = "";
	}

#ifdef COLOR_POST_DATE
	mytm = localtime(&filetime);
	strftime(buf, 5, "%w", mytm);
	sprintf(color, "[1;%dm", 30 + atoi(buf) + 1);
#else
	strcpy(color, "");
#endif

	strcpy(c1, "[1;33m");
	strcpy(c2, "[1;36m");
	//Modified by IAMFAT 2002-05-27
	if (toupper(ent->title[0])=='R' && toupper(ent->title[1])=='E'
			&& ent->title[2]==':') {
		if (!strcmp(topic, ent->title+4))
			same = YEA;
		reflag=YEA;
	} else {
		if (!strcmp(topic, ent->title))
			same = YEA;
		reflag=NA;
	}
	/*
	 if (!strcmp(topic, ent->title) || !strcmp(topic, ent->title+4))
	 same = YEA;*/
	//End IAMFAT
	strlcpy(b2, ent->owner, STRLEN);
	if ((b2[strlen(b2) - 1] == '>') && strchr(b2, '<')) {
		t = strtok(b2, "<>");
		if (invalidaddr(t))
			t = strtok(NULL, "<>");
		if (t != NULL)
			strcpy(b2, t);
	}
	if ((t = strchr(b2, ' ')) != NULL)
		*t = '\0';
	if (ent->accessed[0] & FILE_READ) {
		if ( (ent->accessed[0] & FILE_MARKED ) && (ent->accessed[0]
				& MAIL_REPLY))
			status = 'b';
		else if (ent->accessed[0] & FILE_MARKED)
			status = 'm';
		else if (ent->accessed[0] & MAIL_REPLY)
			status = 'r';
		else
			status = ' ';
	} else {
		if (ent->accessed[0] & FILE_MARKED)
			status = 'M';
		else
			status = 'N';
	}
	//Modified by IAMFAT 2002-05-30
	if (!strncmp("Re:", ent->title, 3) || !strncmp("RE:", ent->title, 3)) {
		sprintf(title, "Re: %s", ent->title+4);
	} else {
		sprintf(title, "¡ï %s", ent->title);
	}

	ellipsis(title, 49);
	sprintf(buf, " %s%4d[m %c %-12.12s %s%6.6s[m  %s%.49s[m",
			same ? (reflag ? c1 : c2) : "", num, status, b2, color, date,
			same ? (reflag ? c1 : c2) : "", title);
	/*
	 if (!strncmp("Re:", ent->title, 3)) {
	 sprintf(buf, " %s%3d[m %c %-12.12s %s%6.6s[m  %s%.50s[m", same ? c1 : ""
	 ,num, status, b2, color, date, same ? c1 : "", ent->title);
	 } else {
	 sprintf(buf, " %s%3d[m %c %-12.12s %s%6.6s[m  ¡ï  %s%.47s[m"
	 ,same ? c2 : "", num, status, b2, color, date, same ? c2 : "", ent->title);
	 }
	 */
	//End IAMFAT
	return buf;
}

int
mail_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[512], notgenbuf[128];
	char *t;
	int readpn;
	char done = NA, delete_it, replied;
	clear();
	readpn = FULLUPDATE;
	setqtitle(fileinfo->title);
	strcpy(buf, direct);
	if ((t = strrchr(buf, '/')) != NULL)
	*t = '\0';
	/****ÅĞ¶ÏType2¹«¸æµÄ¹²ÏíÎÄ¼ş****/
	if(fileinfo->filename[0] == 's')
	strcpy(notgenbuf, fileinfo->filename);
	else
	sprintf(notgenbuf, "%s/%s", buf, fileinfo->filename);
	delete_it = replied = NA;
	while (!done) {
		ansimore(notgenbuf, NA);
		move(t_lines - 1, 0);
		prints("(R)»ØĞÅ, (D)É¾³ı, (G)¼ÌĞø? [G]: ");
		switch (egetch()) {
			case 'R':
			case 'r':
			replied = YEA;
			mail_reply(ent, fileinfo, direct);
			break;
			case KEY_UP:
			case KEY_PGUP:
			done = YEA;
			readpn = READ_PREV;
			break;
			case ' ':
			case 'j':
			case KEY_RIGHT:
			case KEY_DOWN:
			case KEY_PGDN:
			done = YEA;
			readpn = READ_NEXT;
			break;
			case '*':
			show_file_info(ent, fileinfo, direct);
			break;
			case 'D':
			case 'd':
			delete_it = YEA;
			default:
			done = YEA;
		}
	}
	if (delete_it)
	return mail_del(ent, fileinfo, direct); /* ĞŞ¸ÄĞÅ¼şÖ®bug
	 * ¼ÓÁËreturn */
	else {
		fileinfo->accessed[0] |= FILE_READ;
		substitute_record(currmaildir, fileinfo, sizeof(*fileinfo), ent);
	}
	return readpn;
}
/*ARGSUSED*/
int
mail_reply(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char uid[STRLEN];
	char title[STRLEN];
	char *t;
	modify_user_mode(SMAIL);
	sprintf(genbuf, "MAILER-DAEMON@%s", BBSHOST);
	if (strstr(fileinfo->owner, genbuf)) {
		ansimore("help/mailerror-explain", YEA);
		return FULLUPDATE;
	}
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		clear();
		move(4,0);
		prints("\n\n        ÄúÉĞÎ´Íê³É×¢²á£¬»òÕß·¢ËÍĞÅ¼şµÄÈ¨ÏŞ±»·â½û¡£");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	clear();
	strlcpy(uid, fileinfo->owner, STRLEN);
	if ((uid[strlen(uid) - 1] == '>') && strchr(uid, '<')) {
		t = strtok(uid, "<>");
		if (invalidaddr(t))
		t = strtok(NULL, "<>");
		if (t != NULL)
		strcpy(uid, t);
		else {
			prints("ÎŞ·¨Í¶µİ\n");
			pressreturn();
			return FULLUPDATE;
		}
	}
	if ((t = strchr(uid, ' ')) != NULL)
	*t = '\0';
	if (toupper(fileinfo->title[0]) != 'R' || toupper(fileinfo->title[1]) != 'E' ||
			fileinfo->title[2] != ':')
	strcpy(title, "Re: ");
	else
	title[0] = '\0';
	strncat(title, fileinfo->title, STRLEN - 5);

	sprintf(quote_file, "mail/%c/%s/%s", toupper(currentuser.userid[0]), currentuser.userid, fileinfo->filename);
	strcpy(quote_user, fileinfo->owner);
	switch (do_send(uid, title)) {
		case -1:
		prints("ÎŞ·¨Í¶µİ\n");
		break;
		case -2:
		prints("È¡Ïû»ØĞÅ\n");
		break;
		case -3:
		prints("[%s] ÎŞ·¨ÊÕĞÅ\n", uid);
		break;
		case -4:
		prints("[%s] ĞÅÏäÒÑÂú£¬ÎŞ·¨ÊÕĞÅ\n", uid);
		break;
		case -5:
		prints("[%s] ²»ÏëÊÕµ½ÄúµÄĞÅ¼ş\n",uid);
		break;
		default:
		fileinfo->accessed[0] |= MAIL_REPLY;
		substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
	}
	pressreturn();
	return FULLUPDATE;
}

int
mail_del(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[512];
	char *t;
	extern int cmpfilename();
	extern int SR_BMDELFLAG;

	if(SR_BMDELFLAG==NA)
	{
		sprintf(genbuf, "É¾³ıĞÅ¼ş [%-.55s]", fileinfo->title);
		if (askyn(genbuf, NA, YEA) == NA) {
			move(t_lines - 1, 0);
			prints("·ÅÆúÉ¾³ıĞÅ¼ş...");
			clrtoeol();
			egetch();
			return FULLUPDATE;
		}
	}
	strcpy(buf, direct);
	if ((t = strrchr(buf, '/')) != NULL)
	*t = '\0';
	if (!delete_record(direct, sizeof(*fileinfo), ent, cmpfilename, fileinfo->filename)) {
		sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
		unlink(genbuf);
		check_maxmail();
		return DIRCHANGED;
	}
	if(SR_BMDELFLAG==NA)
	{
		move(t_lines - 1, 0);
		prints("É¾³ıÊ§°Ü...");
		clrtoeol();
		egetch();
	}
	return PARTUPDATE;
}
#ifdef INTERNET_EMAIL

int
mail_forward(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[STRLEN];
	char *p;
	if (!HAS_PERM(PERM_FORWARD)) {
		return DONOTHING;
	}
	/****Type 2¹«¸æ½ûÖ¹×ªĞÅ****/
	if(fileinfo->filename[0] == 's')
	{
		prints("Type 2¹«¸æ½ûÖ¹×ªĞÅ!\n");
		return DONOTHING;
	}
	strlcpy(buf, direct, STRLEN);
	buf[STRLEN - 1] = '\0';
	if ((p = strrchr(buf, '/')) != NULL)
	*p = '\0';
	switch (doforward(buf, fileinfo, 0)) {
		case 0:
			prints("ÎÄÕÂ×ª¼ÄÍê³É!\n");
			break;
		case BBS_EINTNL:
			prints("×ª¼ÄÊ§°Ü: ÏµÍ³·¢Éú´íÎó.\n");
			break;
		case -2:
			prints("×ª¼ÄÊ§°Ü: ²»ÕıÈ·µÄÊÕĞÅµØÖ·.\n");
			break;
		case BBS_EMAILQE:
			prints("ÄúµÄĞÅÏä³¬ÏŞ£¬ÔİÊ±ÎŞ·¨Ê¹ÓÃĞÅ¼ş·şÎñ.\n");
			break;
		case BBS_EACCES:
			prints("ÄúÃ»ÓĞ·¢ĞÅÈ¨ÏŞ£¬ÔİÊ±ÎŞ·¨Ê¹ÓÃĞÅ¼ş·şÎñ.\n");
			break;
		case BBS_EBLKLST:
			prints("¶Ô·½²»ÏëÊÕµ½ÄúµÄĞÅ¼ş.\n");
			break;
		default:
			prints("È¡Ïû×ª¼Ä...\n");
	}
	pressreturn();
	clear();
	return FULLUPDATE;
}

int
mail_u_forward(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[STRLEN];
	char *p;
	if (!HAS_PERM(PERM_FORWARD)) {
		return DONOTHING;
	}
	if(fileinfo->filename[0] == 's')
	{
		prints("Type 2¹«¸æ½ûÖ¹×ªĞÅ!\n");
		return DONOTHING;
	}
	strlcpy(buf, direct, STRLEN);
	buf[STRLEN - 1] = '\0';
	if ((p = strrchr(buf, '/')) != NULL)
	*p = '\0';
	switch (doforward(buf, fileinfo, 1)) {
		case 0:
			prints("ÎÄÕÂ×ª¼ÄÍê³É!\n");
			break;
		case BBS_EINTNL:
			prints("×ª¼ÄÊ§°Ü: ÏµÍ³·¢Éú´íÎó.\n");
			break;
		case -2:
			prints("×ª¼ÄÊ§°Ü: ²»ÕıÈ·µÄÊÕĞÅµØÖ·.\n");
			break;
		case BBS_EMAILQE:
			prints("ÄúµÄĞÅÏä³¬ÏŞ£¬ÔİÊ±ÎŞ·¨Ê¹ÓÃĞÅ¼ş·şÎñ.\n");
			break;
		case BBS_EACCES:
			prints("ÄúÃ»ÓĞ·¢ĞÅÈ¨ÏŞ£¬ÔİÊ±ÎŞ·¨Ê¹ÓÃĞÅ¼ş·şÎñ.\n");
			break;
		case BBS_EBLKLST:
			prints("¶Ô·½²»ÏëÊÕµ½ÄúµÄĞÅ¼ş.\n");
			break;
		default:
			prints("È¡Ïû×ª¼Ä...\n");
	}
	pressreturn();
	clear();
	return FULLUPDATE;
}
#endif

int
mail_del_range(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	return (del_range(ent, fileinfo, direct));
}

int
mail_mark(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (fileinfo->accessed[0] & FILE_MARKED)
	fileinfo->accessed[0] &= ~FILE_MARKED;
	else
	fileinfo->accessed[0] |= FILE_MARKED;
	substitute_record(currmaildir, fileinfo, sizeof(*fileinfo), ent);
	return (PARTUPDATE);
}

extern int mailreadhelp();
extern int SR_BMfunc();

struct one_key mail_comms[] = { 'd', mail_del, 'D', mail_del_range, 'b',
		SR_BMfunc, Ctrl('P'), m_send, 'E', edit_post, 'r', mail_read, 'R',
		mail_reply, 'm', mail_mark, 'i', Save_post, 'I', Import_post,
//Commented by Amigo 2002.06.07
		//	'x', into_myAnnounce,
		KEY_TAB, show_user_notes,
#ifdef INTERNET_EMAIL
		'F', mail_forward,
		'U', mail_u_forward,
#endif
		'a', auth_search_down, 'A', auth_search_up, '/', t_search_down,
		'?', t_search_up, '\'', post_search_down, '\"', post_search_up,
		']', thread_down, '[', thread_up, Ctrl('A'), show_author,
		Ctrl('N'), SR_first_new, '\\', SR_last, '=', SR_first, 'l',
		show_allmsgs, Ctrl('C'), do_cross, Ctrl('S'), SR_read, 'n',
		SR_first_new, 'p', SR_read, Ctrl('X'), SR_read, Ctrl('U'),
		SR_author, 'h', mailreadhelp, Ctrl('J'), mailreadhelp, '!',
		Q_Goodbye, 'S', s_msg, '*', show_file_info, 'Z', send_msg,
//Removed by iamfat 2002.06.11
		//duplicated function with 'o'
		//	'f', t_friends,
		'\0', NULL };

int m_read() {
	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;
	in_mail = YEA;
	i_read(RMAIL, currmaildir, mailtitle, maildoent, &mail_comms[0],
			sizeof(struct fileheader));
	in_mail = NA;
	return 0;
}
int
invalidaddr(addr)
char *addr;
{
	int i=0;
	if (*addr == '\0' || !strchr(addr, '@'))
	return 1;
	while (*addr) {
		if (!isalnum(*addr) && !strchr(".!@:-_", *addr))
		return 1;
		if(strchr("@",*addr)) {
			i++;
			if (i>=2) {
				i=0;
				return 1;
			}
		}
		addr++;
	}
	return 0;
}
#ifdef INTERNET_EMAIL

#ifdef SENDMAIL_MIME_AUTOCONVERT
int
bbs_sendmail(fname, title, receiver, filter, mime)
char *fname, *title, *receiver;
int filter, mime;
#else
int
bbs_sendmail(fname, title, receiver, filter)
char *fname, *title, *receiver;
int filter;
#endif
{
	FILE *fin, *fout;
	sprintf(genbuf, "%s -f %s.bbs@%s %s", MTA,
			currentuser.userid, BBSHOST, receiver);
	fout = popen(genbuf, "w");
	fin = fopen(fname, "r");
	if (fin == NULL || fout == NULL)
	return -1;

	fprintf(fout, "Return-Path: %s.bbs@%s\n", currentuser.userid, BBSHOST);
	fprintf(fout, "Reply-To: %s.bbs@%s\n", currentuser.userid, BBSHOST);
	fprintf(fout, "From: %s.bbs@%s\n", currentuser.userid, BBSHOST);
	fprintf(fout, "To: %s\n", receiver);
	fprintf(fout, "Subject: %s\n", title);
	fprintf(fout, "X-Forwarded-By: %s (%s)\n",
			currentuser.userid,	currentuser.username);

	fprintf(fout, "X-Disclaimer: %s ¶Ô±¾ĞÅÄÚÈİË¡²»¸ºÔğ¡£\n", BoardName);
#ifdef SENDMAIL_MIME_AUTOCONVERT
	if (mime) {
		fprintf(fout, "MIME-Version: 1.0\n");
		fprintf(fout, "Content-Type: text/plain; charset=US-ASCII\n");
		fprintf(fout, "Content-Transfer-Encoding: 8bit\n");
	}
#endif
	fprintf(fout, "Precedence: junk\n\n");

	while (fgets(genbuf, 255, fin) != NULL) {
		if(filter)
			ansi_filter(genbuf, genbuf);
		if (genbuf[0] == '.' && genbuf[1] == '\n')
		fputs(". \n", fout);
		else
		fputs(genbuf, fout);
	}

	fprintf(fout, ".\n");

	fclose(fin);
	pclose(fout);
	return 0;
}
#endif

int g_send() {
	char uident[13], tmp[3];
	int cnt, i, n, fmode = NA;
	char maillists[STRLEN];
	char current_maillist = '0';
	char s_current_maillist[2] = { 0, 0 };

	modify_user_mode(SMAIL);
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		clear();
		move(4, 0);
		prints("\n\n        ÄúÉĞÎ´Íê³É×¢²á£¬»òÕß·¢ËÍĞÅ¼şµÄÈ¨ÏŞ±»·â½û¡£");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	*quote_file = '\0';
	sethomefile(maillists, currentuser.userid, "maillist");
	while (1) {
		clear();
		cnt = listfilecontent(maillists, 3);
		if (cnt > maxrecp - 10) {
			move(1, 0);
			prints("Ä¿Ç°ÏŞÖÆ¼ÄĞÅ¸ø [1m%d[m ÈË", maxrecp);
		}
		move(2, 0);
		prints("ÏÖÔÚÊÇµÚ %c ¸öÃûµ¥ (0~9)Ñ¡ÔñÆäËûÃûµ¥", current_maillist);

		getdata(0, 0, "(A)Ôö¼Ó (D)É¾³ı (I)ÒıÈëºÃÓÑ (C)Çå³ıÄ¿Ç°Ãûµ¥ (E)·ÅÆú (S)¼Ä³ö? [S]£º ",
				tmp, 2, DOECHO, YEA);

		if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's' || tmp[0]
				== 'S') {
			break;
		}
		if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A' || tmp[0]
				== 'D') {
			move(1, 0);
			if (tmp[0] == 'a' || tmp[0] == 'A')
				/**
				 * ÈÕ  ÆÚ: 2007.12.19
				 * Î¬»¤Õß: Anonomous
				 * ´úÂë¶Î: ´ÓÏÂÃæwhile(1)Óï¾ä¿ªÊ¼µ½while½áÊø£¬Ò»¹²34ĞĞ¡£
				 * Ä¿  µÄ: Ôö¼ÓÈºĞÅ·¢ĞÅÈËµÄÊ±ºò²»ĞèÒªÃ¿´Î¶¼°´A¼ü£¬ËùÓĞµÄ²Ù×÷Ò»´Î°´A
				 *         Ö®ºóÍê³É¡£
				 * ±¸  ×¢: Õâ¸ö×ö·¨ÆäÊµ²»ÊÇºÜºÃ£¬²»¹ıÒòÎªÕû¸öFBÏµÍ³Éè¼ÆµÄ¾ÖÏŞĞÔ£¬Ã»ÓĞ
				 *         °ì·¨¸Ä³É±È½ÏºÃµÄÁ÷³Ì£¬Ö»ÄÜÔÚÔ­±¾µÄÁ÷³Ì»ù´¡ÉÏÖØ¸´ÀÍ¶¯¡£FBµÄ
				 *         Éè¼ÆÓĞµãÌ«ËÀ°å£¬Ã¿´ÎÔö¼Ó·¢ĞÅÈËµÄÊ±ºò¶¼Ö»´¦ÀíÒ»¸öid£¬¶øÇÒÕâ
				 *         ¸ö´¦Àí¹ı³ÌÊÇ¼ĞÔÓÔÚÆäËû²Ù×÷ÖĞ¼äµÄ£¬Õû¸öÁ÷³ÌµÄñîºÏ¶ÈÌ«¸ß£¬Ã»
				 *         °ì·¨²ğ·Ö£¬Ö»ºÃ²ÉÈ¡ÏÂÃæµÄ·½Ê½£¬Ã¿´ÎÔö¼Ó·¢ĞÅÈËµÄÊ±ºòÖØ»æÕû¸ö
				 *         ÆÁÄ»£¬²¢ÇÒÍê³ÉÒ»´ÎÌí¼Ó²Ù×÷¡£Ï£ÍûÒÔºó»áÓĞ¸üºÃµÄ°ì·¨¡£-_-||
				 */
				while (1) {
					clear();
					cnt = listfilecontent(maillists, 3);
					if (cnt > maxrecp - 10) {
						move(1, 0);
						prints("Ä¿Ç°ÏŞÖÆ¼ÄĞÅ¸ø [1m%d[m ÈË", maxrecp);
					}
					move(2, 0);
					prints("ÏÖÔÚÊÇµÚ %c ¸öÃûµ¥ (0~9)Ñ¡ÔñÆäËûÃûµ¥", current_maillist);
					move(0, 0);
					prints("(A)Ôö¼Ó (D)É¾³ı (I)ÒıÈëºÃÓÑ (C)Çå³ıÄ¿Ç°Ãûµ¥ (E)·ÅÆú (S)¼Ä³ö? [S]£º ");
					move(1, 0);
					usercomplete("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ", uident);
					move(1, 0);
					clrtoeol();
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(2, 0);
						prints("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
						continue;
					}
					if (!(lookupuser.userlevel & PERM_READMAIL)) {
						move(2, 0);
						prints("ÎŞ·¨ËÍĞÅ¸ø: [1m%s[m\n", lookupuser.userid);
						continue;
					} else if (seek_in_file(maillists, uident)) {
						move(2, 0);
						prints("ÒÑ¾­ÁĞÎªÊÕ¼şÈËÖ®Ò» \n");
						continue;
					}
					add_to_file(maillists, uident);
					cnt++;
				}
			else
				namecomplete("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] == '\0')
				continue;
			if (!getuser(uident)) {
				move(2, 0);
				prints("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
				continue; //added by infotech. rubing Ìá¹©.·ÀÖ¹¼ÓÈë²»´æÔÚµÄÊ¹ÓÃÕß.
			}
		}
		if (tmp[0] >= '0' && tmp[0] <= '9') {
			current_maillist = tmp[0];
			s_current_maillist[0] = tmp[0];
			sethomefile(maillists, currentuser.userid, "maillist");
			if (tmp[0] != '0')
				strcat(maillists, s_current_maillist);
			cnt = listfilecontent(maillists, 3);
			continue;
		}
		switch (tmp[0]) {
			case 'A':
			case 'a':
				/* ÕâÒ»¶ÎcaseÓ¦¸ÃÓÀÔ¶¶¼Ö´ĞĞ²»µ½£¬ÒòÎªÇ°ÃæµÄ²¿·ÖÒÑ¾­Íê³ÉÁËĞ©²Ù×÷£¬
				 * ±£ÏÕÆğ¼û£¬ÔİÊ±±£Áô¡£
				 * by Anonomous */
				if (!(lookupuser.userlevel & PERM_READMAIL)) {
					move(2, 0);
					prints("ÎŞ·¨ËÍĞÅ¸ø: [1m%s[m\n", lookupuser.userid);
					break;
				} else if (seek_in_file(maillists, uident)) {
					move(2, 0);
					prints("ÒÑ¾­ÁĞÎªÊÕ¼şÈËÖ®Ò» \n");
					break;
				}
				add_to_file(maillists, uident);
				cnt++;
				break;
			case 'E':
			case 'e':
			case 'Q':
			case 'q':
				cnt = 0;
				break;
			case 'D':
			case 'd': {
				if (seek_in_file(maillists, uident)) {
					del_from_file(maillists, uident);
					cnt--;
				}
				break;
			}
			case 'I':
			case 'i':
				n = 0;
				clear();
				for (i = cnt; i < maxrecp && n < uinfo.fnum; i++) {
					int key;
					move(2, 0);
					if (getuserid(uident, uinfo.friend[n], sizeof(uident)) == -1)
						exit(0);
					prints("%s\n", uident);
					move(3, 0);
					n++;
					prints("(A)È«²¿¼ÓÈë (Y)¼ÓÈë (N)²»¼ÓÈë (Q)½áÊø? [Y]:");
					if (!fmode)
						key = igetkey();
					else
						key = 'Y';
					if (key == 'q' || key == 'Q')
						break;
					if (key == 'A' || key == 'a') {
						fmode = YEA;
						key = 'Y';
					}
					if (key == '\0' || key == '\n' || key == 'y' || key
							== 'Y') {
						if (!getuser(uident)) {
							move(4, 0);
							prints("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
							i--;
							continue;
						} else if (!(lookupuser.userlevel & PERM_READMAIL)) {
							move(4, 0);
							prints("ÎŞ·¨ËÍĞÅ¸ø: [1m%s[m\n", lookupuser.userid);
							i--;
							continue;
						} else if (seek_in_file(maillists, uident)) {
							i--;
							continue;
						}
						add_to_file(maillists, uident);
						cnt++;
					}
				}
				fmode = NA;
				clear();
				break;
			case 'C':
			case 'c':
				unlink(maillists);
				cnt = 0;
				break;
		}
		if (strchr("EeQq", tmp[0]))
			break;
		move(5, 0);
		clrtobot();
		if (cnt > maxrecp)
			cnt = maxrecp;
		move(3, 0);
		clrtobot();
	}
	if (cnt > 0) {
		G_SENDMODE = 2;
		switch (do_gsend(NULL, NULL, cnt, current_maillist)) {
			case -1:
				prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
				break;
			case -2:
				prints("È¡Ïû\n");
				break;
			default:
				prints("ĞÅ¼şÒÑ¼Ä³ö\n");
		}
		G_SENDMODE = 0;
		pressreturn();
	}
	return 0;
}
/*Add by SmallPig*/

int
do_gsend(userid, title, num, current_maillist)
char *userid[], *title;
int num;
char current_maillist;
{
	struct stat st;
	struct override or;
	char filepath[STRLEN], tmpfile[STRLEN];
	int cnt, result;
	FILE *mp;
	char s_current_maillist[2] = {0, 0};
	extern int cmpfnames();

	s_current_maillist[0] = current_maillist;
	in_mail = YEA;
	sprintf(genbuf, "%s", currentuser.userid);
	header.reply_mode = NA;
	strcpy(header.title, "Ã»Ö÷Ìâ");
	strcpy(header.ds, "¼ÄĞÅ¸øÒ»ÈºÈË");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	result = post_header(&header);
	if( result == -1) {
		clear();
		return -2;
	}
	if( result == YEA) {
		sprintf(save_title, "[ÈºÌåĞÅ¼ş] %-60.60s", header.title);
		//		strncpy(save_filename, fname, 4096);
	}
	do_quote(quote_file, tmpfile, header.include_mode);
	if (vedit(tmpfile, YEA, YEA) == -1) {
		unlink(tmpfile);
		clear();
		return -2;
	}
	clear();
	prints("[5;1;32mÕıÔÚ¼Ä¼şÖĞ£¬ÇëÉÔºò...[m");
	if (G_SENDMODE == 2) {
		char maillists[STRLEN];
		setuserfile(maillists, "maillist");
		if (current_maillist != '0')
		strcat(maillists, s_current_maillist);
		if ((mp = fopen(maillists, "r")) == NULL) {
			return -3;
		}
	}
	for (cnt = 0; cnt < num; cnt++) {
		char uid[13];
		char buf[STRLEN];
		switch (G_SENDMODE) {
			case 1:
				if (getuserid(uid, uinfo.friend[cnt], sizeof(uid)) == -1)
					exit(0);
				break;
			case 2:
				if (fgets(buf, STRLEN, mp) != NULL) {
					if (strtok(buf, " \n\r\t") != NULL)
						strcpy(uid, buf);
					else
						continue;
				} else {
					cnt = num;
					continue;
				}
				break;
			default:
				strcpy(uid, userid[cnt]);
				break;
		}
		sethomefile(filepath, uid, "rejects");
		if(search_record(filepath, &or, sizeof(or), cmpfnames, currentuser.userid))
		continue;
		sprintf(filepath, "mail/%c/%s", toupper(uid[0]), uid);
		if (stat(filepath, &st) == -1) {
			if (mkdir(filepath, 0755) == -1) {
				if (G_SENDMODE == 2)
				fclose(mp);
				return -1;
			}
		} else {
			if (!(st.st_mode & S_IFDIR)) {
				if (G_SENDMODE == 2)
				fclose(mp);
				return -1;
			}
		}
		mail_file(tmpfile, uid, save_title);
		//added by iamfat 2003.11.03 avoid offline for timeout
		uinfo.idle_time = time(0);
		update_ulist(&uinfo, utmpent);
		//added end
	}
	unlink(tmpfile);
	clear();
	if (G_SENDMODE == 2)
	fclose(mp);
	return 0;
}

/********************Type2¹«¸æ¹²ÏíÎÄ¼ş by Ashinmarch on 2008.3.30*********************/
/********************ÎªÁËÌá¸ßĞ§ÂÊ,ÃâÈ¥ºÚÃûµ¥¡¢ĞÅ¼şÈİÁ¿µÈÅĞ¶Ï**************************/
int sharedmail_file(char tmpfile[STRLEN], char userid[STRLEN],
		char title[STRLEN]) {
	struct fileheader newmessage;
	if (!getuser(userid))
		return -1;
	if (!(lookupuser.userlevel & PERM_READMAIL))
		return -3;
	memset(&newmessage, 0, sizeof(newmessage));
	sprintf(genbuf, "%s", currentuser.userid);
	strlcpy(newmessage.owner, genbuf, STRLEN);
	strlcpy(newmessage.title, title, STRLEN);
	strlcpy(save_title, newmessage.title, STRLEN);
	strlcpy(newmessage.filename, tmpfile, STRLEN);

	sprintf(genbuf, "mail/%c/%s/%s", toupper(userid[0]), userid, DOT_DIR);
	if (append_record(genbuf, &newmessage, sizeof(newmessage)) != -1)
		return -1;
	sprintf(genbuf, "mailed %s: %s", userid, title);
	report(genbuf, currentuser.userid);
	return 0;
}

/*Add by SmallPig*/
int ov_send() {
	int all, i;
	modify_user_mode(SMAIL);
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		clear();
		move(4, 0);
		prints("\n\n        ÄúÉĞÎ´Íê³É×¢²á£¬»òÕß·¢ËÍĞÅ¼şµÄÈ¨ÏŞ±»·â½û¡£");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("¼ÄĞÅ¸øºÃÓÑÃûµ¥ÖĞµÄÈË£¬Ä¿Ç°±¾Õ¾ÏŞÖÆ½ö¿ÉÒÔ¼Ä¸ø [1m%d[m Î»¡£\n", maxrecp);
	if (uinfo.fnum <= 0) {
		prints("Äú²¢Ã»ÓĞÉè¶¨ºÃÓÑ¡£\n");
		pressanykey();
		clear();
		return 0;
	} else {
		prints("Ãûµ¥ÈçÏÂ£º\n");
	}
	G_SENDMODE = 1;
	all = (uinfo.fnum >= maxrecp) ? maxrecp : uinfo.fnum;
	for (i = 0; i < all; i++) {
		char uid[IDLEN + 2];
		if (getuserid(uid, uinfo.friend[i], sizeof(uid)) == -1)
			exit(0);
		prints("%-12s ", uid);
		if ((i + 1) % 6 == 0)
			outc('\n');
	}
	pressanykey();
	switch (do_gsend(NULL, NULL, all, '0')) {
		case -1:
			prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
			break;
		case -2:
			prints("ĞÅ¼şÈ¡Ïû\n");
			break;
		default:
			prints("ĞÅ¼şÒÑ¼Ä³ö\n");
	}
	pressreturn();
	G_SENDMODE = 0;
	return 0;
}

int
in_group(uident, cnt)
char uident[maxrecp][STRLEN];
int cnt;
{
	int i;
	for (i = 0; i < cnt; i++)
	if (!strcmp(uident[i], uident[cnt])) {
		return i + 1;
	}
	return 0;
}
#ifdef INTERNET_EMAIL

int
doforward(direct, fh, mode)
char *direct;
struct boardheader *fh;
int mode;
{
	int lookupuserlevel;//added by roly 02.03.25
	static char address[STRLEN];
	char fname[STRLEN], tmpfname[STRLEN];
	char receiver[STRLEN];
	char title[STRLEN];
	int return_no, internet_mail=0;
	int filter=YEA;
	int maxmail;
	FILE *fp;
	extern int cmpfnames();
	extern char fromhost[];

	clear();
	if (!HAS_PERM(PERM_MAIL))
		return BBS_EACCES;
	if (check_maxmail())
		return BBS_EMAILQE;
	if (address[0] == '\0') {
		//strncpy(address, currentuser.email, STRLEN);
		strlcpy(address, currentuser.userid, STRLEN);
	}
	if (HAS_PERM(PERM_SETADDR)) {
		prints("±¾Õ¾Ä¿Ç°Ö»Ìá¹©Õ¾ÄÚ×ªĞÅ£¬ÇëÊäÈëÒª×ª¼ÄµÄÕÊºÅÃû¡£\n");
		prints("ÇëÖ±½Ó°´ Enter ½ÓÊÜÀ¨ºÅÄÚÌáÊ¾µÄµØÖ·, »òÕßÊäÈëÆäËûµØÖ·\n");
		prints("°ÑĞÅ¼ş×ª¼Ä¸ø [%s]\n", address);
		//getdata(3, 0, "==> ", receiver, 70, DOECHO, YEA);
		/*2008.02.24 Ashinmarch: usercomplete*/
		prints("==>");
		usercomplete(NULL, receiver);
	} else strcpy(receiver,currentuser.userid);
	if (receiver[0] != '\0') {
		strlcpy(address, receiver, STRLEN);
	} else
	strlcpy(receiver,address,STRLEN);
	sprintf(genbuf, ".bbs@%s", BBSHOST);
	if (strstr(receiver, genbuf)
			|| strstr(receiver, ".bbs@localhost")) {
		char *pos;
		pos = strchr(address, '.');
		*pos = '\0';
	}
	if( strpbrk(address,"@.")) {
		internet_mail = YEA;
		return -2; /* added by Seaman */
	}
	if(!internet_mail) {
		if (!getuser(address))
			return BBS_EINTNL;
		if(getmailboxsize(lookupuser.userlevel)*2<getmailsize(lookupuser.userid)) {
			prints("[%s] ĞÅÏäÈİÁ¿ÒÑÂú£¬ÎŞ·¨ÊÕĞÅ¡£\n",address);
			return BBS_ERMQE;
		}
		sethomefile(fname, lookupuser.userid, "rejects");
		if(search_record(fname, &fh, sizeof(fh), cmpfnames, currentuser.userid))
			return BBS_EBLKLST;

		/* added by roly 03.03.10*/
		/*   
		 maxmail = (HAS_PERM(PERM_OBOARDS)||HAS_PERM(PERM_LARGEMAIL)) ?
		 MAX_SYSOPMAIL_HOLD : (HAS_PERM(PERM_BOARDS)) ?
		 MAX_BMMAIL_HOLD : MAX_MAIL_HOLD;
		 */
		lookupuserlevel=lookupuser.userlevel;
		maxmail = getmailboxhold(lookupuserlevel);

		if (getmailnum(lookupuser.userid)> maxmail*2) {
			prints("[%s] ĞÅÏäÒÑÂú£¬ÎŞ·¨ÊÕĞÅ¡£\n",address);
			return BBS_ERMQE;
		}
		/* add end */
	}
	sprintf(genbuf, "È·¶¨½«ÎÄÕÂ¼Ä¸ø %s Âğ", address);
	if (askyn(genbuf, YEA, NA) == 0)
		return 1;
	if (invalidaddr(address))
		if (!getuser(address))
			return -2;
	sprintf(tmpfname, "tmp/forward.%s.%05d", currentuser.userid, uinfo.pid);

	sprintf(genbuf, "%s/%s", direct, fh->filename);
	f_cp(genbuf, tmpfname, O_CREAT);

	if (askyn("ÊÇ·ñĞŞ¸ÄÎÄÕÂÄÚÈİ", NA, NA) == 1) {
		if (vedit(tmpfname, NA, NA) == -1) {
			if (askyn("ÊÇ·ñ¼Ä³öÎ´ĞŞ¸ÄµÄÎÄÕÂ", YEA, NA) == 0) {
				unlink(tmpfname);
				clear();
				return 1;
			}
		}
		else if ((fp = fopen(tmpfname, "a")) != NULL) {
			fprintf(fp,
					"\n--\n\033[1;36m¡ù ĞŞ¸Ä:¡¤%s ÓÚ %16.16s ĞŞ¸Ä±¾ÎÄ¡¤[FROM: %-.20s]\033[m\n",
					currentuser.userid, getdatestring(time(NULL), DATE_ZH) + 6, mask_host(fromhost));
			fclose (fp);
		}
		clear();

	}
	if(internet_mail)
	if (askyn("ÊÇ·ñ¹ıÂËANSI¿ØÖÆ·û",YEA,NA) == NA ) filter = NA;
	add_crossinfo(tmpfname, 2);
	prints("×ª¼ÄĞÅ¼ş¸ø %s, ÇëÉÔºò....\n", address);
	refresh();

	if (mode == 0)
	strcpy(fname, tmpfname);
	else if (mode == 1) {
		sprintf(fname, "tmp/file.uu%05d", uinfo.pid);
		sprintf(genbuf, "uuencode %s fb-bbs.%05d > %s",
				tmpfname, uinfo.pid, fname);
		system(genbuf);
	}
	if( !strstr(fh->title,"(×ª¼Ä)"))
	sprintf(title, "%.70s(×ª¼Ä)", fh->title);
	else strcpy(title,fh->title);
	if (!internet_mail)
	return_no = mail_file(fname, lookupuser.userid, title);
	else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
		if (askyn("ÒÔ MIME ¸ñÊ½ËÍĞÅ", NA, NA) == YEA)
		return_no = bbs_sendmail(fname, title, address, filter,YEA);
		else
		return_no = bbs_sendmail(fname, title, address, filter,NA);
#else
		return_no = bbs_sendmail(fname, title, address, filter);
#endif
	}
	if (mode == 1) {
		unlink(fname);
	}
	unlink(tmpfname);
	return (return_no);
}
#endif
