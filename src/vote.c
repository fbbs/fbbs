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
 $Id: vote.c 317 2006-10-27 13:20:07Z danielfree $
 */

#include "bbs.h"
#include "vote.h"

extern cmpbnames();
extern int page, range;
extern struct boardheader *bcache;
extern struct BCACHE *brdshm;
static char *vote_type[] = { "ÊÇ·Ç", "µ¥Ñ¡", "¸´Ñ¡", "Êı×Ö", "ÎÊ´ğ" };
struct votebal currvote; //µ±Ç°Í¶Æ±
char controlfile[STRLEN];
unsigned int result[33]; //Í¶Æ±½á¹ûÊı×é
int vnum;
int voted_flag;
FILE *sug; //Í¶Æ±½á¹ûµÄÎÄ¼şÖ¸Õë
int makevote(struct votebal *ball, char *bname); //ÉèÖÃÍ¶Æ±Ïä

//Added by IAMFAT 2002.06.13
extern void ellipsis(char *str, int len); //¼ÓÊ¡ÂÔºÅ

//Added End
//commented by jacobson

//±¾ÎÄ¼şÖ÷Òª´¦ÀíÍ¶Æ±¹¦ÄÜ

//±È½Ï×Ö·û´®useridºÍÍ¶Æ±Õßuv 
//userid:ÓÃ»§Ãû uv:Í¶Æ±Õß 
//·µ»ØÖµ:0²»µÈ£¬ 1ÏàµÈ
int cmpvuid(char *userid, struct ballot *uv) {
	return !strcmp(userid, uv->uid);
}

//ÉèÖÃ°æÃæÍ¶Æ±µÄ±êÖ¾,           
//bname:°æÃæÃû,flag°æÃæ±êÖ¾
//1:¿ªÆôÍ¶Æ±,0:¹Ø±ÕÍ¶Æ± ·µ»ØÖµ:ÎŞ..
int setvoteflag(char *bname, int flag) {
	int pos;
	struct boardheader fh;

	pos = search_record(BOARDS, &fh, sizeof(fh), cmpbnames, bname);
	if (flag == 0)
		fh.flag = fh.flag & ~BOARD_VOTE_FLAG;
	else
		fh.flag = fh.flag | BOARD_VOTE_FLAG;
	if (substitute_record(BOARDS, &fh, sizeof(fh), pos) == -1)
		prints("Error updating BOARDS file...\n");
}

//ÏÔÊ¾bug±¨¸æ(Ä¿Ç°ºÃÏñÃ»ÓĞÊµÏÖ)
//str:´íÎóĞÅÏ¢×Ö·û´®
void b_report(char *str) {
	char buf[STRLEN];

	sprintf(buf, "%s %s", currboard, str);
	report(buf);
}

//½¨Á¢Ä¿Â¼,Ä¿Â¼Îª vote/°æÃû,È¨ÏŞÎª755
//bname:°æÃæÃû×Ö
void makevdir(char *bname) {
	struct stat st;
	char buf[STRLEN];

	sprintf(buf, "vote/%s", bname);
	if (stat(buf, &st) != 0)
		mkdir(buf, 0755);
}

//ÉèÖÃÎÄ¼şÃû
//bname£º°æÃæÃû
//filename:ÎÄ¼şÃû
//buf:·µ»ØµÄÎÄ¼şÃû
void setvfile(char *buf, char *bname, char *filename) {
	sprintf(buf, "vote/%s/%s", bname, filename);
}

//ÉèÖÃ¿ØÖÆcontrolfileÎÄ¼şÃûÎª vote\°æÃæÃû\control
void setcontrolfile() {
	setvfile(controlfile, currboard, "control");
}

//±à¼­»òÉ¾³ı°æÃæ±¸ÍüÂ¼
//·µ»ØÖµ:FULLUPDATE
#ifdef ENABLE_PREFIX
int b_notes_edit()
{
	char buf[STRLEN], buf2[STRLEN];
	char ans[4];
	int aborted;
	int notetype;

	if (!chk_currBM(currBM, 0)) { //¼ì²éÊÇ·ñ°æÖ÷
		return 0;
	}
	clear();
	move(0, 0);
	prints("Éè¶¨£º\n\n  (1)Ò»°ã±¸ÍüÂ¼\n  (2)ÃØÃÜ±¸ÍüÂ¼\n");
	prints("  (3)°æÃæÇ°×º±í\n  (4)ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º\n");
	while (1) {
		getdata(7, 0,"µ±Ç°Ñ¡Ôñ[1](0~4): ", ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
		return FULLUPDATE;
		if (ans[0] == '\0')
		strcpy(ans, "1");
		if (ans[0] >= '1' && ans[0] <= '4' )
		break;
	}
	makevdir(currboard); //½¨Á¢±¸ÍüÂ¼Ä¿Â¼
	notetype = ans[0] - '0';
	if (notetype == 2) {
		setvfile(buf, currboard, "secnotes");
	} else if (notetype == 3) {
		setvfile(buf, currboard, "prefix");
	} else if (notetype == 1) {
		setvfile(buf, currboard, "notes");
	} else if (notetype == 4 ) {
		int pos;
		struct boardheader fh;
		pos = search_record(BOARDS, &fh, sizeof(fh), cmpbnames, currboard);

		if (askyn("ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º£¿", (fh.flag & BOARD_PREFIX_FLAG)?YEA:NA,NA)) {
			fh.flag |= BOARD_PREFIX_FLAG;
		} else {
			fh.flag &= ~BOARD_PREFIX_FLAG;
		}
		substitute_record(BOARDS, &fh, sizeof(fh), pos);
		return FULLUPDATE;
	}
	sprintf(buf2, "(E)±à¼­ (D)É¾³ı %4s? [E]: ",
			(notetype == 3)?"°æÃæÇ°×º±í":(notetype == 1) ? "Ò»°ã±¸ÍüÂ¼" : "ÃØÃÜ±¸ÍüÂ¼");
	getdata(8, 0, buf2, ans, 2, DOECHO, YEA); //Ñ¯ÎÊ±à¼­»òÕßÉ¾³ı
	if (ans[0] == 'D' || ans[0] == 'd') { //É¾³ı±¸ÍüÂ¼
		move(9, 0);
		sprintf(buf2, "ÕæµÄÒªÉ¾³ıÃ´£¿");
		if (askyn(buf2, NA, NA)) {
			move(10, 0);
			prints("ÒÑ¾­É¾³ı...\n");
			pressanykey();
			unlink(buf);
			aborted = 1;
		} else
		aborted = -1;
	} else
	aborted = vedit(buf, NA, YEA); //±à¼­±¸ÍüÂ¼
	if (aborted == -1) {
		pressreturn();
	} else {
		if (notetype == 1)
		setvfile(buf, currboard, "noterec");
		else
		setvfile(buf, currboard, "notespasswd");
		unlink(buf);
	}

	return FULLUPDATE;
}
#else
int b_notes_edit() {
	char buf[STRLEN], buf2[STRLEN];
	char ans[4];
	int aborted;
	int notetype;
	if (!chk_currBM(currBM, 0)) { //¼ì²éÊÇ·ñ°æÖ÷
		return 0;
	}
	clear();
	move(1, 0);
	prints("±à¼­/É¾³ı±¸ÍüÂ¼"); //Ñ¯ÎÊ±à¼­ÄÄÖÖ±¸ÍüÂ¼
	while (1) {
		getdata(3, 0, "±à¼­»òÉ¾³ı±¾ÌÖÂÛÇøµÄ (0) Àë¿ª  (1) Ò»°ã±¸ÍüÂ¼  (2) ÃØÃÜ±¸ÍüÂ¼? [1] ",
				ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
			return FULLUPDATE;
		if (ans[0] == '\0')
			strcpy(ans, "1");
		if (ans[0] == '1' || ans[0] == '2')
			break;
	}
	makevdir(currboard); //½¨Á¢±¸ÍüÂ¼Ä¿Â¼
	if (ans[0] == '2') {
		setvfile(buf, currboard, "secnotes");
		notetype = 2;
	} else {
		setvfile(buf, currboard, "notes");
		notetype = 1;
	}
	sprintf(buf2, "(E)±à¼­ (D)É¾³ı %4s±¸ÍüÂ¼? [E]: ", (notetype == 1) ? "Ò»°ã"
			: "ÃØÃÜ");
	getdata(5, 0, buf2, ans, 2, DOECHO, YEA); //Ñ¯ÎÊ±à¼­»òÕßÉ¾³ı
	if (ans[0] == 'D' || ans[0] == 'd') { //É¾³ı±¸ÍüÂ¼
		move(6, 0);
		sprintf(buf2, "ÕæµÄÒªÉ¾³ı%4s±¸ÍüÂ¼", (notetype == 1) ? "Ò»°ã" : "ÃØÃÜ");
		if (askyn(buf2, NA, NA)) {
			move(7, 0);
			prints("±¸ÍüÂ¼ÒÑ¾­É¾³ı...\n");
			pressanykey();
			unlink(buf);
			aborted = 1;
		} else
			aborted = -1;
	} else
		aborted = vedit(buf, NA, YEA); //±à¼­±¸ÍüÂ¼
	if (aborted == -1) {
		pressreturn();
	} else {
		if (notetype == 1)
			setvfile(buf, currboard, "noterec");
		else
			setvfile(buf, currboard, "notespasswd");
		unlink(buf);
	}

	return FULLUPDATE;
}
#endif 
//ÉèÖÃÃØÃÜ±¸ÍüÂ¼ÃÜÂë
int b_notes_passwd() {
	FILE *pass;
	char passbuf[20], prepass[20];
	char buf[STRLEN];

	if (!chk_currBM(currBM, 0)) { //¼ì²éÊÇ·ñ°æÖ÷
		return 0;
	}
	clear();
	move(1, 0);
	prints("Éè¶¨/¸ü¸Ä/È¡Ïû¡¸ÃØÃÜ±¸ÍüÂ¼¡¹ÃÜÂë...");
	setvfile(buf, currboard, "secnotes");
	if (!dashf(buf)) {
		move(3, 0);
		prints("±¾ÌÖÂÛÇøÉĞÎŞ¡¸ÃØÃÜ±¸ÍüÂ¼¡¹¡£\n\n");
		prints("ÇëÏÈÓÃ W ±àºÃ¡¸ÃØÃÜ±¸ÍüÂ¼¡¹ÔÙÀ´Éè¶¨ÃÜÂë...");
		pressanykey();
		return FULLUPDATE;
	}
	if (!check_notespasswd())
		return FULLUPDATE;
	getdata(3, 0, "ÇëÊäÈëĞÂµÄÃØÃÜ±¸ÍüÂ¼ÃÜÂë(Enter È¡ÏûÃÜÂë): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		setvfile(buf, currboard, "notespasswd");
		unlink(buf);
		prints("ÒÑ¾­È¡Ïû±¸ÍüÂ¼ÃÜÂë¡£");
		pressanykey();
		return FULLUPDATE;
	}
	getdata(4, 0, "È·ÈÏĞÂµÄÃØÃÜ±¸ÍüÂ¼ÃÜÂë: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		prints("\nÃÜÂë²»Ïà·û, ÎŞ·¨Éè¶¨»ò¸ü¸Ä....");
		pressanykey();
		return FULLUPDATE;
	}
	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "w")) == NULL) {
		move(5, 0);
		prints("±¸ÍüÂ¼ÃÜÂëÎŞ·¨Éè¶¨....");
		pressanykey();
		return FULLUPDATE;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(5, 0);
	prints("ÃØÃÜ±¸ÍüÂ¼ÃÜÂëÉè¶¨Íê³É...");
	pressanykey();
	return FULLUPDATE;
}

//½«Ò»¸öÎÄ¼şÈ«²¿ÄÚÈİĞ´ÈëÒÑ¾­´ò¿ªµÄÁíÒ»¸öÎÄ¼ş
//fp: ÒÑ¾­´ò¿ªµÄÎÄ¼şÖ¸Õë,£¨±»Ğ´ÈëÎÄ¼ş£©
//fname: ĞèÒªĞ´ÈëµÄÎÄ¼şµÄÂ·¾¶
int b_suckinfile(FILE * fp, char *fname) {
	char inbuf[256];
	FILE *sfp;

	if ((sfp = fopen(fname, "r")) == NULL)
		return -1;
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL)
		fputs(inbuf, fp);
	fclose(sfp);
	return 0;
}

//½«Ò»¸öÎÄ¼şÈ«²¿ÄÚÈİĞ´ÈëÒÑ¾­´ò¿ªµÄÁíÒ»¸öÎÄ¼ş,(ÓÃÓÚ¶ÁÁôÑÔ°å)
//Èç¹û²»ÄÜ´ò¿ªĞ´ÈëÒ»ÌõºáÏß
//fp: ÒÑ¾­´ò¿ªµÄÎÄ¼şÖ¸Õë,£¨±»Ğ´ÈëÎÄ¼ş£©
//fname: ĞèÒªĞ´ÈëµÄÎÄ¼şµÄÂ·¾¶
/*Add by SmallPig*/
int catnotepad(fp, fname)
FILE *fp;
char *fname;
{
	char inbuf[256];
	FILE *sfp;
	int count;

	count = 0;
	if ((sfp = fopen(fname, "r")) == NULL) {
		fprintf(fp,
				"[1;34m  ¡õ[44m__________________________________________________________________________[m \n\n");
		return -1;
	}
	while (fgets(inbuf, sizeof(inbuf), sfp) != NULL) {
		if (count != 0)
		fputs(inbuf, fp);
		else
		count++;
	}
	fclose(sfp);
	return 0;
}

//¹Ø±ÕÍ¶Æ±
//·µ»ØÖµ:¹Ì¶¨Îª0
int b_closepolls() {
	char buf[80];
	time_t now, nextpoll;
	int i, end;

	now = time(0);
	resolve_boards();

	if (now < brdshm->pollvote) { //ÏÖÔÚÊ±¼äĞ¡ÓÚÏÂ´Î¿ÉÍ¶Æ±Ê±¼äÔò·µ»Ø£¿
		return;
	}
	//¹Ø±ÕÏÔÊ¾ º¯Êıµ÷ÓÃÒÆµ½miscd
	/*
	 move(t_lines - 1, 0);
	 prints("¶Ô²»Æğ£¬ÏµÍ³¹Ø±ÕÍ¶Æ±ÖĞ£¬ÇëÉÔºò...");
	 refresh();
	 */

	nextpoll = now + 7 * 3600;

	strcpy(buf, currboard);
	for (i = 0; i < brdshm->number; i++) {
		strcpy(currboard, (&bcache[i])->filename);
		setcontrolfile();
		end = get_num_records(controlfile, sizeof(currvote));
		for (vnum = end; vnum >= 1; vnum--) {
			time_t closetime;

			get_record(controlfile, &currvote, sizeof(currvote), vnum);
			closetime = currvote.opendate + currvote.maxdays * 86400;
			if (now > closetime)
				mk_result(vnum); //ÈôÍ¶Æ±ÆÚÏŞÒÑ¹ıĞ´ÈëÍ¶Æ±½á¹û
			else if (nextpoll > closetime)
				nextpoll = closetime + 300;
		}
	}
	strcpy(currboard, buf);

	brdshm->pollvote = nextpoll; //ÏÂ´Î¿ÉÍ¶Æ±Ê±¼ä£¿
	return 0;
}

//¼ÆËãÒ»´ÎµÄÍ¶Æ±½á¹û,²¢·ÅÈëresultÊı×éÖĞ,ÓÃÓÚmk_resultÖĞµÄapply_recordº¯ÊıÖĞµÄ»Øµ÷º¯Êı -.-!
//result[32]¼ÇÂ¼µ÷ÓÃ´ÎÊı
//²ÎÊıptr:Ò»´ÎµÄÍ¶Æ±½á¹û
//·µ»ØÖµ:¹Ì¶¨Îª0
int count_result(struct ballot *ptr) {
	int i;

	/*	if (ptr == NULL) {
	 if (sug != NULL) {
	 fclose(sug);
	 sug == NULL;
	 }
	 return 0;
	 }
	 */if (ptr->msg[0][0] != '\0') {
		if (currvote.type == VOTE_ASKING) {
			fprintf(sug, "[1m%s [mµÄ×÷´ğÈçÏÂ£º\n", ptr->uid);
		} else
			fprintf(sug, "[1m%s [mµÄ½¨ÒéÈçÏÂ£º\n", ptr->uid);
		for (i = 0; i < 3; i++)
			fprintf(sug, "%s\n", ptr->msg[i]);
	}
	result[32]++;
	if (currvote.type == VOTE_ASKING) {
		return 0;
	}
	if (currvote.type != VOTE_VALUE) {
		for (i = 0; i < 32; i++) {
			if ((ptr->voted >> i) & 1)
				(result[i])++;
		}

	} else {
		result[31] += ptr->voted;
		result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
	}
	return 0;
}

//½«Í¶Æ±µÄÌ§Í·Ğ´ÈësugÍ¶Æ±½á¹ûÎÄ¼ş
get_result_title() {
	char buf[STRLEN];

	getdatestring(currvote.opendate, NA);
	fprintf(sug, "¡Ñ Í¶Æ±¿ªÆôì¶£º[1m%s[m  Àà±ğ£º[1m%s[m\n", datestring,
			vote_type[currvote.type - 1]);
	fprintf(sug, "¡Ñ Ö÷Ìâ£º[1m%s[m\n", currvote.title);
	if (currvote.type == VOTE_VALUE)
		fprintf(sug, "¡Ñ ´Ë´ÎÍ¶Æ±µÄÖµ²»¿É³¬¹ı£º[1m%d[m\n\n", currvote.maxtkt);
	fprintf(sug, "¡Ñ Æ±Ñ¡ÌâÄ¿ÃèÊö£º\n\n");
	sprintf(buf, "vote/%s/desc.%d", currboard, currvote.opendate);
	b_suckinfile(sug, buf);
}

//½áÊøÍ¶Æ±,¼ÆËãÍ¶Æ±½á¹û
//num:Í¶Æ±controlÎÄ¼şÖĞµÚ¼¸¸ö¼ÇÂ¼
int mk_result(int num) {
	char fname[STRLEN], nname[STRLEN];
	char sugname[STRLEN];
	char title[STRLEN];
	int i;
	unsigned int total = 0;

	setcontrolfile();
	sprintf(fname, "vote/%s/flag.%d", currboard, currvote.opendate); //Í¶Æ±¼ÇÂ¼ÎÄ¼şÂ·¾¶Îª vote/°æÃû/flag.¿ªÆôÍ¶Æ±ÈÕ
	/*	count_result(NULL); */
	sug = NULL;
	sprintf(sugname, "vote/%s/tmp.%d", currboard, uinfo.pid); //Í¶Æ±ÁÙÊ±ÎÄ¼şÂ·¾¶Îª vote/°æÃû/tmp.ÓÃ»§id
	if ((sug = fopen(sugname, "w")) == NULL) {
		report("open vote tmp file error");
		//prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
		pressanykey();
	}
	(void) memset(result, 0, sizeof(result));
	if (apply_record(fname, count_result, sizeof(struct ballot), 0, 0, 0)
			== -1) {
		report("Vote apply flag error");
	}
	fprintf(sug, "[1;44;36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
			(currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ğ");
	fclose(sug);
	sprintf(nname, "vote/%s/results", currboard); //Í¶Æ±½á¹ûÎÄ¼şÂ·¾¶Îª vote/°æÃû/results
	if ((sug = fopen(nname, "w")) == NULL) {
		report("open vote newresult file error");
		//prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
	}
	get_result_title(sug);
	//¼ÆËãÍ¶Æ±½á¹û
	fprintf(sug, "** Í¶Æ±½á¹û:\n\n");
	if (currvote.type == VOTE_VALUE) {
		total = result[32];
		for (i = 0; i < 10; i++) {
			fprintf(
					sug,
					"[1m  %4d[m µ½ [1m%4d[m Ö®¼äÓĞ [1m%4d[m Æ±  Ô¼Õ¼ [1m%d%%[m\n",
					(i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1), ((i
							+ 1) * currvote.maxtkt) / 10, result[i],
					(result[i] * 100) / ((total <= 0) ? 1 : total));
		}
		fprintf(sug, "´Ë´ÎÍ¶Æ±½á¹ûÆ½¾ùÖµÊÇ: [1m%d[m\n", result[31]
				/ ((total <= 0) ? 1 : total));
	} else if (currvote.type == VOTE_ASKING) {
		total = result[32];
	} else {
		for (i = 0; i < currvote.totalitems; i++) {
			total += result[i];
		}
		for (i = 0; i < currvote.totalitems; i++) {
			fprintf(sug, "(%c) %-40s  %4d Æ±  Ô¼Õ¼ [1m%d%%[m\n", 'A' + i,
					currvote.items[i], result[i], (result[i] * 100)
							/ ((total <= 0) ? 1 : total));
		}
	}
	fprintf(sug, "\nÍ¶Æ±×ÜÈËÊı = [1m%d[m ÈË\n", result[32]);
	fprintf(sug, "Í¶Æ±×ÜÆ±Êı =[1m %d[m Æ±\n\n", total);
	fprintf(sug, "[1;44;36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
			(currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ğ");
	b_suckinfile(sug, sugname);
	unlink(sugname); //É¾³ıÍ¶Æ±ÁÙÊ±ÎÄ¼ş,²¢½«Í¶Æ±ÎÄ¼şĞ´ÈësugÍ¶Æ±½á¹ûÎÄ¼ş
	fclose(sug);

	sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±½á¹û", currboard);
	Postfile(nname, "vote", title, 1); //Í¶Æ±½á¹ûÌùÈëvote°æ
	if (currboard != "vote")
		Postfile(nname, currboard, title, 1); //Í¶Æ±½á¹ûÌùÈëµ±Ç°°æ
	dele_vote(num); //¹Ø±ÕÍ¶Æ±,É¾³ıÁÙÊ±ÎÄ¼ş
	return;
}

//È¡µÃÑ¡ÔñÌâ¿ÉÑ¡ÏîÄ¿,·ÅÈëbalÖĞ
//·µ»ØÖµ num£º¿ÉÑ¡ÏîÄ¿Êı
int get_vitems(struct votebal *bal) {
	int num;
	char buf[STRLEN];

	move(3, 0);
	prints("ÇëÒÀĞòÊäÈë¿ÉÑ¡ÔñÏî, °´ ENTER Íê³ÉÉè¶¨.\n");
	num = 0;
	for (num = 0; num < 32; num++) {
		sprintf(buf, "%c) ", num + 'A');
		getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36,
				DOECHO, YEA);
		if (strlen(bal->items[num]) == 0) {
			if (num != 0)
				break;
			num = -1;
		}
	}
	bal->totalitems = num;
	return num;
}

//¿ªÆôÍ¶Æ±Ïä²¢ÉèÖÃÍ¶Æ±Ïä
//bname:°æÃû
//·µ»ØÖµ:¹Ì¶¨Îª FULLUPDATE
int vote_maintain(char *bname) {
	char buf[STRLEN];
	struct votebal *ball = &currvote;

	setcontrolfile();
	if (!chk_currBM(currBM, 0)) {
		return 0;
	}
	stand_title("¿ªÆôÍ¶Æ±Ïä");
	makevdir(bname);
	for (;;) {
		getdata(2, 0, "(1)ÊÇ·Ç, (2)µ¥Ñ¡, (3)¸´Ñ¡, (4)ÊıÖµ (5)ÎÊ´ğ (6)È¡Ïû ? : ",
				genbuf, 2, DOECHO, YEA);
		genbuf[0] -= '0';
		if (genbuf[0] < 1 || genbuf[0] > 5) {
			prints("È¡Ïû´Ë´ÎÍ¶Æ±\n");
			return FULLUPDATE;
		}
		ball->type = (int) genbuf[0];
		break;
	}
	ball->opendate = time(NULL);
	if (makevote(ball, bname))
		return FULLUPDATE; //ÉèÖÃÍ¶Æ±Ïä
	setvoteflag(currboard, 1);
	clear();
	strcpy(ball->userid, currentuser.userid);
	if (append_record(controlfile, ball, sizeof(*ball)) == -1) {
		prints("·¢ÉúÑÏÖØµÄ´íÎó£¬ÎŞ·¨¿ªÆôÍ¶Æ±£¬ÇëÍ¨¸æÕ¾³¤");
		b_report("Append Control file Error!!");
	} else {
		char votename[STRLEN];
		int i;

		b_report("OPEN");
		prints("Í¶Æ±Ïä¿ªÆôÁË£¡\n");
		range++;;
		sprintf(votename, "tmp/votetmp.%s.%05d", currentuser.userid,
				uinfo.pid);
		if ((sug = fopen(votename, "w")) != NULL) {
			//Modified by IAMFAT 2002.06.13
			//sprintf(buf, "[Í¨Öª] %s ¾Ù°ìÍ¶Æ±£º%s", currboard, ball->title);
			strcpy(genbuf, ball->title);
			ellipsis(genbuf, 31 - strlen(currboard));
			sprintf(buf, "[Í¨Öª] %s ¾Ù°ìÍ¶Æ±: %s", currboard, ball->title);
			get_result_title(sug);
			if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE) {
				fprintf(sug, "\n¡¾[1mÑ¡ÏîÈçÏÂ[m¡¿\n");
				for (i = 0; i < ball->totalitems; i++) {
					fprintf(sug, "([1m%c[m) %-40s\n", 'A' + i,
							ball->items[i]);
				}
			}
			fclose(sug);
			Postfile(votename, "vote", buf, 1);
			unlink(votename);
		}
	}
	pressreturn();
	return FULLUPDATE;
}

//ÉèÖÃÍ¶Æ±Ïä
//ball: Í¶Æ±Ïä
//bname£º°æÃû
//·µ»ØÖµ0£º Õı³£ÍË³ö 1£ºÓÃ»§È¡Ïû
int makevote(struct votebal *ball, char *bname) {
	char buf[STRLEN];
	int aborted;

	prints("Çë°´ÈÎºÎ¼ü¿ªÊ¼±à¼­´Ë´Î [Í¶Æ±µÄÃèÊö]: \n");
	igetkey();
	setvfile(genbuf, bname, "desc");
	sprintf(buf, "%s.%d", genbuf, ball->opendate);
	aborted = vedit(buf, NA, YEA);
	if (aborted) {
		clear();
		prints("È¡Ïû´Ë´ÎÍ¶Æ±Éè¶¨\n");
		pressreturn();
		return 1;
	}
	clear();
	getdata(0, 0, "´Ë´ÎÍ¶Æ±ËùĞëÌìÊı (²»¿É£°Ìì): ", buf, 3, DOECHO, YEA);

	if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
		strcpy(buf, "1");

	ball->maxdays = atoi(buf);
	for (;;) {
		//Modified by IAMFAT 2002.06.13
		//getdata(1, 0, "Í¶Æ±ÏäµÄ±êÌâ: ", ball->title, 61, DOECHO, YEA);
		getdata(1, 0, "Í¶Æ±ÏäµÄ±êÌâ: ", ball->title, 50, DOECHO, YEA);
		if (strlen(ball->title) > 0)
			break;
		bell();
	}
	switch (ball->type) {
		case VOTE_YN:
			ball->maxtkt = 0;
			strcpy(ball->items[0], "ÔŞ³É  £¨ÊÇµÄ£©");
			strcpy(ball->items[1], "²»ÔŞ³É£¨²»ÊÇ£©");
			strcpy(ball->items[2], "Ã»Òâ¼û£¨²»Çå³ş£©");
			ball->maxtkt = 1;
			ball->totalitems = 3;
			break;
		case VOTE_SINGLE:
			get_vitems(ball);
			ball->maxtkt = 1;
			break;
		case VOTE_MULTI:
			get_vitems(ball);
			for (;;) {
				getdata(21, 0, "Ò»¸öÈË×î¶à¼¸Æ±? [1]: ", buf, 5, DOECHO, YEA);
				ball->maxtkt = atoi(buf);
				if (ball->maxtkt <= 0)
					ball->maxtkt = 1;
				if (ball->maxtkt > ball->totalitems)
					continue;
				break;
			}
			break;
		case VOTE_VALUE:
			for (;;) {
				getdata(3, 0, "ÊäÈëÊıÖµ×î´ó²»µÃ³¬¹ı [100] : ", buf, 4, DOECHO, YEA);
				ball->maxtkt = atoi(buf);
				if (ball->maxtkt <= 0)
					ball->maxtkt = 100;
				break;
			}
			break;
		case VOTE_ASKING:
			/*                    getdata(3,0,"´ËÎÊ´ğÌâ×÷´ğĞĞÊıÖ®ÏŞÖÆ :",buf,3,DOECHO,YEA) ;
			 ball->maxtkt = atof(buf) ;
			 if(ball->maxtkt <= 0) ball->maxtkt = 10;*/
			ball->maxtkt = 0;
			currvote.totalitems = 0;
			break;
		default:
			ball->maxtkt = 1;
			break;
	}
	return 0;
}

// ¼ì²éÊÇ·ñ¶Á¹ıĞÂµÄ±¸ÍüÂ¼»òÕß½øÕ¾welcome »òÕßĞ´Èë
// bname:°æÃû, mode =2Ê±ÉèÎªNULL
// val:  0£º¼ì²éÄ£Ê½    ²»µÈÓÚ0:Ğ´ÈëÄ£Ê½
// mode: 1:¼ì²é±¸ÍüÂ¼   2:¼ì²é½øÕ¾Welcome
// ·µ»ØÖµ 0:Î´¶Á 1:ÒÑ¶Á
int vote_flag(char *bname, char val, int mode) {
	char buf[STRLEN], flag;
	int fd, num, size;

	num = usernum - 1;

	switch (mode) {
		case 2:
			sprintf(buf, "Welcome.rec"); /* ½øÕ¾µÄ Welcome »­Ãæ */
			break;
		case 1:
			setvfile(buf, bname, "noterec"); /* ÌÖÂÛÇø±¸ÍüÂ¼µÄÆì±ê */
			break;
		default:
			return -1;
	}

	if (num >= MAXUSERS) {
		report("Vote Flag, Out of User Numbers");
		return -1;
	}

	if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) {
		return -1;
	}

	FLOCK(fd, LOCK_EX);
	size = (int) lseek(fd, 0, SEEK_END);
	memset(buf, 0, sizeof(buf));
	while (size <= num) {
		write(fd, buf, sizeof(buf));
		size += sizeof(buf);
	}
	lseek(fd, (off_t) num, SEEK_SET);
	read(fd, &flag, 1); //¶ÁÊÇ·ñÒÑ¾­¶Á¹ıµÄ±êÖ¾flag
	if ((flag == 0 && val != 0)) {
		lseek(fd, (off_t) num, SEEK_SET);
		write(fd, &val, 1);
	}
	FLOCK(fd, LOCK_UN);
	close(fd);

	return flag;
}

//¼ì²éÍ¶ÁË¼¸Æ±
//bits: 32Î»µÄÖµ
//·µ»ØÖµ ¶ş½øÖÆ32Î»bitsÖĞ µÈÓÚ1µÄÎ»ÊıµÄÊıÁ¿
int vote_check(int bits) {
	int i, count;

	for (i = count = 0; i < 32; i++) {
		if ((bits >> i) & 1)
			count++;
	}
	return count;
}

//ÏÔÊ¾ÓÃ»§Í¶¹ıµÄÆ±£¬ÒÔ¼°¿ÉÑ¡Ïî
//pbits:Æ±Êı×Ö¶Î i:ÏÔÊ¾Î»ÖÃ flag:ÊÇ·ñÏÔÊ¾ÄãÒÑ¾­Í¶ÁË¼¸Æ± YEA:ÏÔÊ¾ NO:²»ÏÔÊ¾
//·µ»ØÖµ:¹Ì¶¨ÎªYEA
int showvoteitems(unsigned int pbits, int i, int flag) {
	char buf[STRLEN];
	int count;

	if (flag == YEA) {
		count = vote_check(pbits);
		if (count > currvote.maxtkt)
			return NA;
		move(2, 0);
		clrtoeol();
		prints("ÄúÒÑ¾­Í¶ÁË [1m%d[m Æ±", count);
	}
	sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i, ((pbits >> i) & 1 ? "¡Ì"
			: "  "), currvote.items[i]);
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	prints(buf);
	refresh();
	return YEA;
}

//ÏÔÊ¾Í¶Æ±ÄÚÈİ
void show_voteing_title() {
	time_t closedate;
	char buf[STRLEN];

	if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
		sprintf(buf, "¿ÉÍ¶Æ±Êı: [1m%d[m Æ±", currvote.maxtkt);
	else
		buf[0] = '\0';
	closedate = currvote.opendate + currvote.maxdays * 86400;
	getdatestring(closedate, NA);
	prints("Í¶Æ±½«½áÊøì¶: [1m%s[m  %s  %s\n", datestring, buf,
			(voted_flag) ? "([5;1mĞŞ¸ÄÇ°´ÎÍ¶Æ±[m)" : "");
	prints("Í¶Æ±Ö÷ÌâÊÇ: [1m%-50s[mÀàĞÍ: [1m%s[m \n", currvote.title,
			vote_type[currvote.type - 1]);
}

//È¡µÃÌáÎÊĞÍÍ¶Æ±´ğ°¸
//uv:ÓÃ»§Í¶Æ±µÄÊı¾İ,·µ»ØºóÓÃ»§ÊäÈëµÄ´ğ°¸·ÅÔÚ uv->msgÀï,×î¶à3ĞĞ
//·µ»ØÖµ: ÓÃ»§ÊäÈëµÄ´ğ°¸ĞĞÊı
int getsug(struct ballot *uv) {
	int i, line;

	move(0, 0);
	clrtobot();
	if (currvote.type == VOTE_ASKING) {
		show_voteing_title();
		line = 3;
		prints("ÇëÌîÈëÄúµÄ×÷´ğ(ÈıĞĞ):\n");
	} else {
		line = 1;
		prints("ÇëÌîÈëÄú±¦¹óµÄÒâ¼û(ÈıĞĞ):\n");
	}
	move(line, 0);
	for (i = 0; i < 3; i++) {
		prints(": %s\n", uv->msg[i]);
	}
	for (i = 0; i < 3; i++) {
		getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NA);
		if (uv->msg[i][0] == '\0')
			break;
	}
	return i;
}

//ÊäÈë¶àÑ¡/µ¥Ñ¡/ÊÇ·ÇµÄ´ğ°¸
//uv:ÓÃ»§Í¶Æ±µÄÊı¾İ,·µ»ØºóÓÃ»§ÊäÈëµÄ´ğ°¸·ÅÔÚ uv->msgÀï
//·µ»ØÖµ: ³É¹¦1 ÓÃ»§È¡Ïû-1
int multivote(struct ballot *uv) {
	unsigned int i;

	i = uv->voted;
	move(0, 0);
	show_voteing_title();
	uv->voted = setperms(uv->voted, "Ñ¡Æ±", currvote.totalitems,
			showvoteitems);
	if (uv->voted == i)
		return -1;
	return 1;
}

//ÊäÈëÖµĞÍÑ¡ÏîµÄ´ğ°¸
//uv:ÓÃ»§Í¶Æ±µÄÊı¾İ,·µ»ØºóÓÃ»§ÊäÈëµÄ´ğ°¸·ÅÔÚ uv->msgÀï
//·µ»ØÖµ: ³É¹¦1 ÓÃ»§È¡Ïû-1
int valuevote(struct ballot *uv) {
	unsigned int chs;
	char buf[10];

	chs = uv->voted;
	move(0, 0);
	show_voteing_title();
	prints("´Ë´Î×÷´ğµÄÖµ²»ÄÜ³¬¹ı [1m%d[m", currvote.maxtkt);
	if (uv->voted != 0)
		sprintf(buf, "%d", uv->voted);
	else
		memset(buf, 0, sizeof(buf));
	do {
		getdata(3, 0, "ÇëÊäÈëÒ»¸öÖµ? [0]: ", buf, 5, DOECHO, NA);
		uv->voted = abs(atoi(buf));
	} while (uv->voted > currvote.maxtkt && buf[0] != '\n' && buf[0]
			!= '\0');
	if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
		return -1;
	return 1;
}

//ÓÃ»§½øĞĞÍ¶Æ±,ÓÉvote_key,b_voteº¯Êıµ÷ÓÃ
//num:Í¶Æ±controlfileÖĞµÚ¼¸¸ö¼ÇÂ¼
//·µ»ØÖµ:ÎŞ
int user_vote(int num) {
	char fname[STRLEN], bname[STRLEN];
	char buf[STRLEN];
	struct ballot uservote, tmpbal;
	int votevalue;
	int aborted = NA, pos;

	move(t_lines - 2, 0);
	get_record(controlfile, &currvote, sizeof(struct votebal), num);
	if (currentuser.firstlogin > currvote.opendate) { //×¢²áÈÕÔÚÍ¶Æ±¿ªÆôÈÕÇ°²»ÄÜÍ¶Æ±
		prints("¶Ô²»Æğ, Í¶Æ±Ãû²áÉÏÕÒ²»µ½ÄúµÄ´óÃû\n");
		pressanykey();
		return;
	}
	sprintf(fname, "vote/%s/flag.%d", currboard, currvote.opendate);
	if ((pos = search_record(fname, &uservote, sizeof(uservote), cmpvuid,
			currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof(uservote));
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	strcpy(uservote.uid, currentuser.userid);
	sprintf(bname, "desc.%d", currvote.opendate);
	setvfile(buf, currboard, bname);
	ansimore(buf, YEA);
	move(0, 0);
	clrtobot();
	switch (currvote.type) {
		case VOTE_SINGLE:
		case VOTE_MULTI:
		case VOTE_YN:
			votevalue = multivote(&uservote);
			if (votevalue == -1)
				aborted = YEA;
			break;
		case VOTE_VALUE:
			votevalue = valuevote(&uservote);
			if (votevalue == -1)
				aborted = YEA;
			break;
		case VOTE_ASKING:
			uservote.voted = 0;
			aborted = !getsug(&uservote);
			break;
	}
	clear();
	if (aborted == YEA) {
		prints("±£Áô ¡¾[1m%s[m¡¿Ô­À´µÄµÄÍ¶Æ±¡£\n", currvote.title);
	} else {
		if (currvote.type != VOTE_ASKING)
			getsug(&uservote);
		pos = search_record(fname, &tmpbal, sizeof(tmpbal), cmpvuid,
				currentuser.userid);
		if (pos) {
			substitute_record(fname, &uservote, sizeof(uservote), pos);
		} else if (append_record(fname, &uservote, sizeof(uservote)) == -1) {
			move(2, 0);
			clrtoeol();
			prints("Í¶Æ±Ê§°Ü! ÇëÍ¨ÖªÕ¾³¤²Î¼ÓÄÇÒ»¸öÑ¡ÏîÍ¶Æ±\n");
			pressreturn();
		}
		prints("\nÒÑ¾­°ïÄúÍ¶ÈëÆ±ÏäÖĞ...\n");
	}
	pressanykey();
	return;
}

//ÏÔÊ¾Í¶Æ±ÏäĞÅÏ¢µÄÍ·²¿
void voteexp() {
	clrtoeol();
	prints("[1;44m±àºÅ ¿ªÆôÍ¶Æ±ÏäÕß ¿ªÆôÈÕ %-39s Àà±ğ ÌìÊı ÈËÊı[m\n", "Í¶Æ±Ö÷Ìâ");
}

//ÏÔÊ¾Í¶Æ±ÏäĞÅÏ¢
//ent Í¶Æ±ĞÅÏ¢
int printvote(struct votebal *ent) {
	static int i;
	struct ballot uservote;
	char buf[STRLEN + 10];
	char flagname[STRLEN];
	int num_voted;

	//Added by IAMFAT 2002.06.13
	char title[STRLEN];

	//Added End

	if (ent == NULL) {
		move(2, 0);
		voteexp();
		i = 0;
		return 0;
	}
	i++;
	if (i > page + 19 || i > range)
		return QUIT;
	else if (i <= page)
		return 0;
	sprintf(buf, "flag.%d", ent->opendate);
	setvfile(flagname, currboard, buf);
	if (search_record(flagname, &uservote, sizeof(uservote), cmpvuid,
			currentuser.userid) <= 0) {
		voted_flag = NA;
	} else
		voted_flag = YEA;
	num_voted = get_num_records(flagname, sizeof(struct ballot));
	getdatestring(ent->opendate, NA);
	//Modified by IAMFAT 2002.06.13
	/*
	 sprintf(buf, " %s%3d %-12.12s %6.6s %-40.40s%-4.4s %3d  %4d[m\n", (voted_flag == NA) ? "[1m" : "", i, ent->userid,
	 datestring+6, ent->title, vote_type[ent->type - 1], ent->maxdays, num_voted);
	 */
	strcpy(title, ent->title);
	ellipsis(title, 39);
	sprintf(buf, " %s%3d %-12.12s %6.6s %-39.39s %-4.4s %3d  %4d[m\n",
			(voted_flag == NA) ? "[1m" : "", i, ent->userid, datestring
					+ 6, title, vote_type[ent->type - 1], ent->maxdays,
			num_voted);
	//Ended IAMFAT
	prints("%s", buf);
}

//É¾³ıÍ¶Æ±ÎÄ¼ş
//num Í¶Æ±controlfileÖĞµÚ¼¸¸ö¼ÇÂ¼
//·µ»ØÖµ ÎŞ
int dele_vote(num)
int num;
{
	char buf[STRLEN];

	sprintf(buf, "vote/%s/flag.%d", currboard, currvote.opendate);
	unlink(buf);
	sprintf(buf, "vote/%s/desc.%d", currboard, currvote.opendate);
	unlink(buf);
	if (delete_record(controlfile, sizeof(currvote), num, NULL, NULL) == -1) {
		prints("·¢Éú´íÎó£¬ÇëÍ¨ÖªÕ¾³¤....");
		pressanykey();
	}
	range--;
	if (get_num_records(controlfile, sizeof(currvote)) == 0) {
		setvoteflag(currboard, 0);
	}
}

//ÏÔÊ¾Í¶Æ±½á¹û
//bname:°æÃû
//·µ»ØÖµ:¹Ì¶¨ÎªFULLUPDATE
int vote_results(char *bname) {
	char buf[STRLEN];

	setvfile(buf, bname, "results");
	if (ansimore(buf, YEA) == -1) {
		move(3, 0);
		prints("Ä¿Ç°Ã»ÓĞÈÎºÎÍ¶Æ±µÄ½á¹û¡£\n");
		clrtobot();
		pressreturn();
	} else
		clear();
	return FULLUPDATE;
}

//¿ªÆôÍ¶Æ±Ïä²¢ÉèÖÃÍ¶Æ±Ïä
int b_vote_maintain() {
	return vote_maintain(currboard);
}

//ÏÔÊ¾Í¶Æ±ÏäÑ¡Ïî
void vote_title() {

	docmdtitle(
			"[Í¶Æ±ÏäÁĞ±í]",
			"[[1;32m¡û[m,[1;32me[m] Àë¿ª [[1;32mh[m] ÇóÖú [[1;32m¡ú[m,[1;32mr <cr>[m] ½øĞĞÍ¶Æ± [[1;32m¡ü[m,[1;32m¡ı[m] ÉÏ,ÏÂÑ¡Ôñ [1m¸ßÁÁ¶È[m±íÊ¾ÉĞÎ´Í¶Æ±");
	update_endline();
}

//¸ù¾İÓÃ»§µÄ°´¼ü¶ÔÍ¶Æ±Ïä½øĞĞ²Ù×÷,¿ÉÒÔ½áÊø/ĞŞ¸Ä/Ç¿ÖÆ¹Ø±Õ/ÏÔÊ¾Í¶Æ±½á¹û
//ch: ÓÃ»§µÄ°´¼ü
//allnum:Í¶Æ±controlfileµÄµÚ¼¸¸ö¼ÇÂ¼
//pagenum:Î´Ê¹ÓÃ
//·µ»ØÖµ 0:Ê§°Ü 1:³É¹¦
int vote_key(int ch, int allnum, int pagenum) {
	int deal = 0, ans;
	char buf[STRLEN];

	switch (ch) {
		case 'v':
		case 'V':
		case '\n':
		case '\r':
		case 'r':
		case KEY_RIGHT:
			user_vote(allnum + 1);
			deal = 1;
			break;
		case 'R':
			vote_results(currboard);
			deal = 1;
			break;
		case 'H':
		case 'h':
			show_help("help/votehelp");
			deal = 1;
			break;
		case 'A':
		case 'a':
			if (!chk_currBM(currBM, 0))
				return YEA;
			vote_maintain(currboard);
			deal = 1;
			break;
		case 'O':
		case 'o':
			if (!chk_currBM(currBM, 0))
				return YEA;
			clear();
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			prints("[5;1;31m¾¯¸æ!![m\n");
			prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
			ans = askyn("ÄúÈ·¶¨ÒªÌáÔç½áÊøÕâ¸öÍ¶Æ±Âğ", NA, NA);

			if (ans != 1) {
				move(2, 0);
				prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
				pressreturn();
				clear();
				break;
			}
			mk_result(allnum + 1);
			sprintf(buf, "[½áÊø] ÌáÔç½áÊøÍ¶Æ± %s", currvote.title);
			securityreport(buf, 0, 4);
			break;
		case 'M':
		case 'm':
			if (!chk_currBM(currBM, 0))
				return YEA;
			clear();
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			prints("[5;1;31m¾¯¸æ!![m\n");
			prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
			ans = askyn("ÄúÈ·¶¨ÒªĞŞ¸ÄÕâ¸öÍ¶Æ±µÄÉè¶¨Âğ", NA, NA);

			if (ans != 1) {
				move(2, 0);
				prints("È¡ÏûĞŞ¸ÄĞĞ¶¯\n");
				pressreturn();
				clear();
				break;
			}
			makevote(&currvote, currboard);
			substitute_record(controlfile, &currvote,
					sizeof(struct votebal), allnum + 1);
			sprintf(buf, "[ĞŞ¸Ä] ĞŞ¸ÄÍ¶Æ±Éè¶¨ %s", currvote.title);
			securityreport(buf, 0, 4);
			break;
		case 'D':
		case 'd':
			if (!chk_currBM(currBM, 0)) {
				return 1;
			}
			deal = 1;
			get_record(controlfile, &currvote, sizeof(struct votebal),
					allnum + 1);
			clear();
			prints("[5;1;31m¾¯¸æ!![m\n");
			prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
			ans = askyn("ÄúÈ·¶¨ÒªÇ¿ÖÆ¹Ø±ÕÕâ¸öÍ¶Æ±Âğ", NA, NA);

			if (ans != 1) {
				move(2, 0);
				prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
				pressreturn();
				clear();
				break;
			}
			sprintf(buf, "[¹Ø±Õ] Ç¿ÖÆ¹Ø±ÕÍ¶Æ± %s", currvote.title);
			securityreport(buf, 0, 4);
			dele_vote(allnum + 1);
			break;
		default:
			return 0;
	}
	if (deal) {
		Show_Votes();
		vote_title();
	}
	return 1;
}

//ÏÔÊ¾Í¶Æ±ÏäĞÅÏ¢
int Show_Votes() {

	move(3, 0);
	clrtobot();
	printvote(NULL);
	setcontrolfile();
	if (apply_record(controlfile, printvote, sizeof(struct votebal), 0, 0,
			0) == -1) {
		prints("´íÎó£¬Ã»ÓĞÍ¶Æ±Ïä¿ªÆô....");
		pressreturn();
		return 0;
	}
	clrtobot();
	return 0;
}

//ÓÃ»§¶Ô±¾°æ½øĞĞÍ¶Æ±£¬bbs.cµ÷ÓÃ
//·µ»ØÖµ:¹Ì¶¨ÎªFULLUPDATE
int b_vote() {
	int num_of_vote;
	int voting;

	if (!HAS_PERM(PERM_VOTE) || (currentuser.stay < 1800)) {
		return;
	}
	setcontrolfile();
	num_of_vote = get_num_records(controlfile, sizeof(struct votebal));
	if (num_of_vote == 0) {
		move(2, 0);
		clrtobot();
		prints("\n±§Ç¸, Ä¿Ç°²¢Ã»ÓĞÈÎºÎÍ¶Æ±¾ÙĞĞ¡£\n");
		pressreturn();
		setvoteflag(currboard, 0);
		return FULLUPDATE;
	}
	setlistrange(num_of_vote);
	clear();
	voting = choose(NA, 0, vote_title, vote_key, Show_Votes, user_vote); //?
	clear();
	return /* user_vote( currboard ) */FULLUPDATE;
}

//ÏÔÊ¾Í¶Æ±½á¹û  bbs.cµ÷ÓÃ
int b_results() {
	return vote_results(currboard);
}

//SYSOP°æ¿ªÆôÍ¶Æ±Ïä
int m_vote() {
	char buf[STRLEN];

	strcpy(buf, currboard);
	strcpy(currboard, DEFAULTBOARD);
	modify_user_mode(ADMIN);
	vote_maintain(DEFAULTBOARD);
	strcpy(currboard, buf);
	return;
}

//¶ÔSYSOP°æ½øĞĞÍ¶Æ±
int x_vote() {
	char buf[STRLEN];

	modify_user_mode(XMENU);
	strcpy(buf, currboard);
	strcpy(currboard, DEFAULTBOARD);
	b_vote();
	strcpy(currboard, buf);
	return;
}

//ÏÔÊ¾sysop°æÍ¶Æ±½á¹û
int x_results() {
	modify_user_mode(XMENU); //¸ü¸ÄÓÃ»§ Ä£Ê½×´Ì¬ÖÁ??
	return vote_results(DEFAULTBOARD); //ÏÔÊ¾sysop°æÍ¶Æ±½á¹û
}
