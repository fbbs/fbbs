#ifndef WITHOUT_ADMIN_TOOLS
#ifndef DLM
#include <stdio.h>
#include "bbs.h"

extern int cmpbnames();
extern int cleanmail();
extern char *chgrp();
extern int dowall();
extern int t_cmpuids();
extern void rebuild_brdshm();
int showperminfo(int, int);
char cexplain[STRLEN];
char buf2[STRLEN];
char lookgrp[30];
char bnames[3][STRLEN]; //´æ·ÅÓÃ»§µ£ÈÎ°æÖ÷µÄ°æÃû,×î¶àÎªÈı
FILE *cleanlog;

//ÔÚuseridµÄÖ÷Ä¿Â¼ÏÂ ´ò¿ª.bmfileÎÄ¼ş,²¢½«ÀïÃæµÄÄÚÈİÓëbnameÏà±È½Ï
//              find´æ·Å´Ó1¿ªÊ¼·µ»ØËùÈÎ°æÃæµÄĞòºÅ,Îª0±íÊ¾Ã»ÕÒµ½
//º¯ÊıµÄ·µ»ØÖµÎªuseridµ£ÈÎ°æÖ÷µÄ°æÃæÊı
int getbnames(char *userid, char *bname, int *find) {
	int oldbm = 0;
	FILE *bmfp;
	char bmfilename[STRLEN], tmp[20];
	*find = 0;
	sethomefile(bmfilename, userid, ".bmfile");
	bmfp = fopen(bmfilename, "r");
	if (!bmfp) {
		return 0;
	}
	for (oldbm = 0; oldbm < 3;) {
		fscanf(bmfp, "%s\n", tmp);
		if (!strcmp(bname, tmp)) {
			*find = oldbm + 1;
		}
		strcpy(bnames[oldbm++], tmp);
		if (feof(bmfp)) {
			break;
		}
	}
	fclose(bmfp);
	return oldbm;
}
//      ĞŞ¸ÄÊ¹ÓÃÕß×ÊÁÏ
int m_info() {
	struct userec uinfo;
	char reportbuf[30];
	int id;

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("ĞŞ¸ÄÊ¹ÓÃÕß×ÊÁÏ");
	if (!gettheuserid(1, "ÇëÊäÈëÊ¹ÓÃÕß´úºÅ: ", &id))
		return -1;
	memcpy(&uinfo, &lookupuser, sizeof (uinfo));
	sprintf(reportbuf, "check info: %s", uinfo.userid);
	report(reportbuf, currentuser.userid);

	move(1, 0);
	clrtobot();
	disply_userinfo(&uinfo);
	uinfo_query(&uinfo, 1, id);
	return 0;
}

//ÈÎÃü°æÖ÷
int m_ordainBM() {

	int id, pos, oldbm = 0, i, find, bm = 1;
	struct boardheader fh;
	FILE *bmfp;
	char bmfilename[STRLEN], bname[STRLEN];
	char buf[5][STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return;

	clear();
	stand_title("ÈÎÃü°æÖ÷\n");
	clrtoeol();
	if (!gettheuserid(2, "ÊäÈëÓûÈÎÃüµÄÊ¹ÓÃÕßÕÊºÅ: ", &id))
		return 0;
	if (!gettheboardname(3, "ÊäÈë¸ÃÊ¹ÓÃÕß½«¹ÜÀíµÄÌÖÂÛÇøÃû³Æ: ", &pos, &fh, bname, 0))
		return -1;
	if (fh.BM[0] != '\0') {
		if (!strncmp(fh.BM, "SYSOPs", 6)) {
			move(5, 0);
			prints("%s ÌÖÂÛÇøµÄ°æÖ÷ÊÇ SYSOPs Äã²»ÄÜÔÙÈÎÃü°æÖ÷", bname);
			pressreturn();
			clear();
			return -1;
		}
		for (i = 0, oldbm = 1; fh.BM[i] != '\0'; i++) {
			if (fh.BM[i] == ' ')
				oldbm++;
		}
		//added by infotech,·ÀÖ¹°æÖ÷ÁĞ±í¹ı³¤
		if (i + strlen(lookupuser.userid) > BMNAMEMAXLEN) {
			move(5, 0);
			prints("%s ÌÖÂÛÇø°æÖ÷ÁĞ±íÌ«³¤,ÎŞ·¨¼ÓÈë!", bname);
			pressreturn();
			clear();
			return -1;
		}
		//add end
		if (oldbm >= 3) {
			move(5, 0);
			prints("%s ÌÖÂÛÇøÒÑÓĞ %d Ãû°æÖ÷", bname, oldbm);
			pressreturn();
			if (oldbm >= BMMAXNUM) {
				clear();
				return -1;
			}
		}

		bm = 0;
	}
	if (!strcmp(lookupuser.userid, "guest")) {
		move(5, 0);
		prints("Äã²»ÄÜÈÎÃü guest µ±°æÖ÷");
		pressanykey();
		clear();
		return -1;
	}
	oldbm = getbnames(lookupuser.userid, bname, &find);
	if (find || oldbm == 3) { //Í¬Ò»ID²»ÄÜ¼æÈÎ³¬¹ıÈı¸ö°æµÄ°æÖ÷
		move(5, 0);
		prints(" %s ÒÑ¾­ÊÇ%s°æµÄ°æÖ÷ÁË", lookupuser.userid, find ? "¸Ã" : "Èı¸ö");
		pressanykey();
		clear();
		return -1;
	}
	prints("\nÄã½«ÈÎÃü %s Îª %s °æ°æ%s.\n", lookupuser.userid, bname, bm ? "Ö÷"
			: "¸±");
	if (askyn("ÄãÈ·¶¨ÒªÈÎÃüÂğ?", NA, NA) == NA) {
		prints("È¡ÏûÈÎÃü°æÖ÷");
		pressanykey();
		clear();
		return -1;
	}
	strcpy(bnames[oldbm], bname);
	if (!oldbm) { //µÚÒ»´Î×ö°æÖ÷
		char secu[STRLEN];

		lookupuser.userlevel |= PERM_BOARDS;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		sprintf(secu, "°æÖ÷ÈÎÃü, ¸øÓè %s µÄ°æÖ÷È¨ÏŞ", lookupuser.userid);
		securityreport(secu, 0, 1);
		move(15, 0);
		prints(secu);
		pressanykey();
		clear();
	}
	if (fh.BM[0] == '\0')
		strcpy(genbuf, lookupuser.userid);
	else
		sprintf(genbuf, "%s %s", fh.BM, lookupuser.userid);
	strlcpy(fh.BM, genbuf, sizeof (fh.BM));
	//added by infotech
	strcpy(buf[0], fh.BM);
#ifdef  BMNAMELISTLIMIT
	for (i = 0; i < BMNAMELISTLEN && buf[0][i]; i++);
	if (i == BMNAMELISTLEN) {
		buf[0][i++] = '.';
		buf[0][i++] = '.';
		buf[0][i++] = '.';
		buf[0][i] = '\0';
	}
#endif
	//endadd
	//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
	//¾«»ªÇøµÄÏÔÊ¾: ¶¯Ì¬·ÖÅä        ÏÔÊ¾10¸ö¿Õ¸ñ printf("%*c",10,' ');
	{
		int blanklen; //Ç°Á½¸ö¿Õ¼ä´óĞ¡
		static const char BLANK = ' ';
		blanklen = STRLEN - strlen(fh.title + 8) - strlen(buf[0]) - 7;
		blanklen /= 2;
		blanklen = (blanklen > 0) ? blanklen : 1;
		sprintf(genbuf, "%s%*c(BM: %s)", fh.title + 8, blanklen, BLANK,
				buf[0]);
	}
	buf[0][0] = '\0';
	get_grp(fh.filename);
	edit_grp(fh.filename, lookgrp, fh.title + 8, genbuf);
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	sethomefile(bmfilename, lookupuser.userid, ".bmfile");
	bmfp = fopen(bmfilename, "w+");
	for (i = 0; i < oldbm + 1; i++) {
		fprintf(bmfp, "%s\n", bnames[i]);
	}
	fclose(bmfp);
	/* Modified by Amigo 2002.07.01. Add reference to BM-Guide. */
	//sprintf (genbuf, "\n\t\t\t¡¾ Í¨¸æ ¡¿\n\n"
	//	   "\tÈÎÃü %s Îª %s °æ%s£¡\n"
	//	   "\t»¶Ó­ %s Ç°Íù BM_Home °æºÍ±¾Çø Zone °æÏò´ó¼ÒÎÊºÃ¡£\n"
	//	   "\t¿ªÊ¼¹¤×÷Ç°£¬ÇëÏÈÍ¨¶ÁBM_Home°æ¾«»ªÇøµÄ°æÖ÷Ö¸ÄÏÄ¿Â¼¡£\n",
	//	   lookupuser.userid, bname, bm ? "°æÖ÷" : "°æ¸±", lookupuser.userid);

	//the new version add by Danielfree 06.11.12
	sprintf(
			genbuf,
			"\n"
				" 		[1;31m   ¨X¨T¨[¨X¨T¨[¨X¨T¨[¨X¨T¨[										 [m\n"
				" 	 [31m©ï©¤©¤[1m¨U[33mÈÕ[31m¨U¨U[33mÔÂ[31m¨U¨U[33m¹â[31m¨U¨U[33m»ª[31m¨U[0;33m©¤©¤[1;36m¡¼Áì»áÕ¾¹æ¾«Éñ¡¤ÊìÏ¤°æÖ÷²Ù×÷¡½[0;33m©¤¡ó¡ô  [m\n"
				" 	 [31m©¦    [1m¨^¨T¨a¨^¨T¨a¨^¨T¨a¨^¨T¨a										  [m\n"
				" 	 [31m©¦																	  [m\n"
				" 		 [1;33m¦î	[37mÈÎÃü  %s  Îª  %s  °æ%s¡£							   [m\n"
				" 		 [1;33mÍ¨																  [m\n"
				" 		[1m	»¶Ó­  %s  Ç°Íù BM_Home °æºÍ±¾Çø Zone °æÏò´ó¼ÒÎÊºÃ¡£			 [m\n"
				" 		 [1;33m¸æ																  [m\n"
				" 		 [1;33m¦ï	[37m¿ªÊ¼¹¤×÷Ç°£¬ÇëÏÈÍ¨¶ÁBM_Home°æ¾«»ªÇøµÄ°æÖ÷Ö¸ÄÏÄ¿Â¼¡£		   [m\n"
				" 																		 [33m©¦  [m\n"
				" 											 [1;33m¨X¨T¨[¨X¨T¨[¨X¨T¨[¨X¨T¨[   [0;33m ©¦  [m\n"
				" 	 [31m¡ó¡ô©¤[1;35m¡¼Î¬»¤°æÃæÖÈĞò¡¤½¨ÉèºÍĞ³¹â»ª¡½[0;31m©¤©¤[1;33m¨U[31m°æ[33m¨U¨U[31mÖ÷[33m¨U¨U[31mÎ¯[33m¨U¨U[31mÈÎ[33m¨U[0;33m©¤©¤©ï	[m\n"
				" 											 [1;33m¨^¨T¨a¨^¨T¨a¨^¨T¨a¨^¨T¨a		  [m\n"
				" 																			 [m\n", lookupuser.userid, bname,
			bm ? "°æÖ÷" : "°æ¸±", lookupuser.userid);
	//add end
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(8, 0);
	prints("ÇëÊäÈëÈÎÃü¸½ÑÔ(×î¶àÎåĞĞ£¬°´ Enter ½áÊø)");
	for (i = 0; i < 5; i++) {
		getdata(i + 9, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
	}
	for (i = 0; i < 5; i++) {
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			strcat(genbuf, "\n\n");
		strcat(genbuf, "\t");
		strcat(genbuf, buf[i]);
		strcat(genbuf, "\n");
	}
	strcpy(currboard, bname);
	sprintf(bmfilename, "ÈÎÃü %s Îª %s °æ%s", lookupuser.userid, fh.filename,
			bm ? "°æÖ÷" : "°æ¸±");
	//autoreport(bmfilename,genbuf,YEA,NULL);
	autoreport(bmfilename, genbuf, YEA, lookupuser.userid, 1); //3x rubing and erebus:)   
#ifdef ORDAINBM_POST_BOARDNAME
	strcpy (currboard, ORDAINBM_POST_BOARDNAME);
	//autoreport(bmfilename,genbuf,YEA,NULL);
	autoreport (bmfilename, genbuf, YEA, lookupuser.userid, 1);
#endif
	/* Added by Amigo 2002.07.01. Send BM assign mail to user's mail box. */
	//{
	//  FILE *se;
	//  char fname[STRLEN];

	//  sprintf( fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid );
	//  if( ( se = fopen(fname,"w") ) != NULL ){
	//          fprintf( se, "%s", genbuf );
	//          fclose( se );
	//          if( lookupuser.userid != NULL )
	//          mail_file( fname, lookupuser.userid, bmfilename );
	//  }
	//}
	/* Add end. */
	securityreport(bmfilename, 0, 1);
	move(16, 0);
	prints(bmfilename);
	pressanykey();
	return 0;
}

//°æÖ÷ÀëÖ°
int m_retireBM() {
	int id, pos, right = 0, oldbm = 0, i, j, bmnum;
	int find, bm = 1;
	struct boardheader fh;
	FILE *bmfp;
	char bmfilename[STRLEN], buf[5][STRLEN];
	char bname[STRLEN], usernames[BMMAXNUM][STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return;

	clear();
	stand_title("°æÖ÷ÀëÖ°\n");
	clrtoeol();
	if (!gettheuserid(2, "ÊäÈëÓûÀëÖ°µÄ°æÖ÷ÕÊºÅ: ", &id))
		return -1;
	if (!gettheboardname(3, "ÇëÊäÈë¸Ã°æÖ÷Òª´ÇÈ¥µÄ°æÃû: ", &pos, &fh, bname, 0))
		return -1;
	oldbm = getbnames(lookupuser.userid, bname, &find);
	if (!oldbm || !find) {
		move(5, 0);
		prints(" %s %s°æ°æÖ÷£¬ÈçÓĞ´íÎó£¬ÇëÍ¨Öª³ÌĞòÕ¾³¤¡£", lookupuser.userid,
				(oldbm) ? "²»ÊÇ¸Ã" : "Ã»ÓĞµ£ÈÎÈÎºÎ");
		pressanykey();
		clear();
		return -1;
	}
	for (i = find - 1; i < oldbm; i++) {
		if (i != oldbm - 1)
			strcpy(bnames[i], bnames[i + 1]);
	}
	bmnum = 0;
	for (i = 0, j = 0; fh.BM[i] != '\0'; i++) {
		if (fh.BM[i] == ' ') {
			usernames[bmnum][j] = '\0';
			bmnum++;
			j = 0;
		} else {
			usernames[bmnum][j++] = fh.BM[i];
		}
	}
	usernames[bmnum++][j] = '\0';
	for (i = 0, right = 0; i < bmnum; i++) {
		if (!strcmp(usernames[i], lookupuser.userid)) {
			right = 1;
			if (i)
				bm = 0;
		}
		if (right && i != bmnum - 1) //while(right&&i<bmnum)ËÆºõ¸üºÃÒ»Ğ©;infotech
			strcpy(usernames[i], usernames[i + 1]);
	}
	if (!right) {
		move(5, 0);
		prints("¶Ô²»Æğ£¬ %s °æ°æÖ÷Ãûµ¥ÖĞÃ»ÓĞ %s £¬ÈçÓĞ´íÎó£¬ÇëÍ¨Öª¼¼ÊõÕ¾³¤¡£", bname,
				lookupuser.userid);
		pressanykey();
		clear();
		return -1;
	}
	prints("\nÄã½«È¡Ïû %s µÄ %s °æ°æ%sÖ°Îñ.\n", lookupuser.userid, bname, bm ? "Ö÷"
			: "¸±");
	if (askyn("ÄãÈ·¶¨ÒªÈ¡ÏûËûµÄ¸Ã°æ°æÖ÷Ö°ÎñÂğ?", NA, NA) == NA) {
		prints("\nºÇºÇ£¬Äã¸Ä±äĞÄÒâÁË£¿ %s ¼ÌĞøÁôÈÎ %s °æ°æÖ÷Ö°Îñ£¡", lookupuser.userid, bname);
		pressanykey();
		clear();
		return -1;
	}
	if (bmnum - 1) { //»¹ÓĞ°æÖ÷,ÎªÊ²Ã´²»ÓÃstrcat ?
		sprintf(genbuf, "%s", usernames[0]);
		for (i = 1; i < bmnum - 1; i++)
			sprintf(genbuf, "%s %s", genbuf, usernames[i]);
	} else {
		genbuf[0] = '\0';
	}
	strlcpy(fh.BM, genbuf, sizeof (fh.BM));
	if (fh.BM[0] != '\0') {
		//added by infotech
		strcpy(buf[0], fh.BM);
#ifdef BMNAMELISTLIMIT
		for (i = 0; i < BMNAMELISTLEN && buf[0][i]; i++);
		if (i == BMNAMELISTLEN) {
			buf[0][i++] = '.';
			buf[0][i++] = '.';
			buf[0][i++] = '.';
			buf[0][i] = '\0';
		}
#endif
		//endadd
		//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
		//¾«»ªÇøµÄÏÔÊ¾: ¶¯Ì¬·ÖÅä        ÏÔÊ¾10¸ö¿Õ¸ñ printf("%*c",10,' ');
		{
			int blanklen; //Ç°Á½¸ö¿Õ¼ä´óĞ¡
			static const char BLANK = ' ';
			blanklen = STRLEN - strlen(fh.title + 8) - strlen(buf[0]) - 7;
			blanklen /= 2;
			blanklen = (blanklen > 0) ? blanklen : 1;
			sprintf(genbuf, "%s%*c(BM: %s)", fh.title + 8, blanklen,
					BLANK, buf[0]);
		}
	} else {
		sprintf(genbuf, "%-38.38s", fh.title + 8);
	}
	get_grp(fh.filename);
	edit_grp(fh.filename, lookgrp, fh.title + 8, genbuf);
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	sprintf(genbuf, "È¡Ïû %s µÄ %s °æ°æÖ÷Ö°Îñ", lookupuser.userid, fh.filename);
	securityreport(genbuf, 0, 1);
	move(8, 0);
	prints(genbuf);
	sethomefile(bmfilename, lookupuser.userid, ".bmfile");
	if (oldbm - 1) {
		bmfp = fopen(bmfilename, "w+");
		for (i = 0; i < oldbm - 1; i++)
			fprintf(bmfp, "%s\n", bnames[i]);
		fclose(bmfp);
	} else {
		char secu[STRLEN];

		unlink(bmfilename);
		if (!(lookupuser.userlevel & PERM_OBOARDS) //Ã»ÓĞÌÖÂÛÇø¹ÜÀíÈ¨ÏŞ
				&& !(lookupuser.userlevel & PERM_SYSOPS) //Ã»ÓĞÕ¾ÎñÈ¨ÏŞ
		) {
			lookupuser.userlevel &= ~PERM_BOARDS;
			substitut_record(PASSFILE, &lookupuser, sizeof(struct userec),
					id);
			sprintf(secu, "°æÖ÷Ğ¶Ö°, È¡Ïû %s µÄ°æÖ÷È¨ÏŞ", lookupuser.userid);
			securityreport(secu, 0, 1);
			move(9, 0);
			prints(secu);
		}
	}
	prints("\n\n");
	if (askyn("ĞèÒªÔÚÏà¹Ø°æÃæ·¢ËÍÍ¨¸æÂğ?", YEA, NA) == NA) {
		pressanykey();
		return 0;
	}
	prints("\n");
	if (askyn("Õı³£ÀëÈÎÇë°´ Enter ¼üÈ·ÈÏ£¬³·Ö°³Í·£°´ N ¼ü", YEA, NA) == YEA)
		right = 1;
	else
		right = 0;
	if (right)
		sprintf(bmfilename, "%s °æ%s %s ÀëÈÎÍ¨¸æ", bname, bm ? "°æÖ÷" : "°æ¸±",
				lookupuser.userid);
	else
		sprintf(bmfilename, "[Í¨¸æ]³·³ı %s °æ%s %s", bname, bm ? "°æÖ÷" : "°æ¸±",
				lookupuser.userid);
	strcpy(currboard, bname);
	if (right) {
		sprintf(genbuf, "\n\t\t\t¡¾ Í¨¸æ ¡¿\n\n"
			"\t¾­Õ¾ÎñÎ¯Ô±»áÌÖÂÛ£º\n"
			"\tÍ¬Òâ %s ´ÇÈ¥ %s °æµÄ%sÖ°Îñ¡£\n"
			"\tÔÚ´Ë£¬¶ÔÆäÔø¾­ÔÚ %s °æµÄĞÁ¿àÀÍ×÷±íÊ¾¸ĞĞ»¡£\n\n"
			"\tÏ£Íû½ñºóÒ²ÄÜÖ§³Ö±¾°æµÄ¹¤×÷.\n", lookupuser.userid, bname, bm ? "°æÖ÷"
				: "°æ¸±", bname);
	} else {
		sprintf(genbuf, "\n\t\t\t¡¾³·Ö°Í¨¸æ¡¿\n\n"
			"\t¾­Õ¾ÎñÎ¯Ô±»áÌÖÂÛ¾ö¶¨£º\n"
			"\t³·³ı %s °æ%s %s µÄ%sÖ°Îñ¡£\n", bname, bm ? "°æÖ÷" : "°æ¸±",
				lookupuser.userid, bm ? "°æÖ÷" : "°æ¸±");
	}
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(14, 0);
	prints("ÇëÊäÈë%s¸½ÑÔ(×î¶àÎåĞĞ£¬°´ Enter ½áÊø)", right ? "°æÖ÷ÀëÈÎ" : "°æÖ÷³·Ö°");
	for (i = 0; i < 5; i++) {
		getdata(i + 15, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
		//      if(i == 0) strcat(genbuf,right?"\n\nÀëÈÎ¸½ÑÔ£º\n":"\n\n³·Ö°ËµÃ÷£º\n");
		if (i == 0)
			strcat(genbuf, "\n\n");
		strcat(genbuf, "\t");
		strcat(genbuf, buf[i]);
		strcat(genbuf, "\n");
	}
	//autoreport(bmfilename,genbuf,YEA,NULL);
	autoreport(bmfilename, genbuf, YEA, lookupuser.userid, 1);

	/* Added by Amigo 2002.07.01. Send BM assign mail to mail box. */
	/*	{
	 FILE *	se;
	 char 	fname[STRLEN];

	 sprintf( fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid );
	 if( ( se = fopen(fname,"w") ) != NULL ){
	 fprintf( se, "%s", genbuf );
	 fclose( se );
	 if( lookupuser.userid != NULL )
	 mail_file( fname, lookupuser.userid, bmfilename );
	 }
	 }*/
	/* Add end. */
	prints("\nÖ´ĞĞÍê±Ï£¡");
	pressanykey();
	return 0;
}

//  ¿ªÉèĞÂ°æ
int m_newbrd() {
	struct boardheader newboard, fh;
	char ans[20];
	char vbuf[100];
	char *group;
	int bid, pos;

	if (!(HAS_PERM(PERM_BLEVELS)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("¿ªÆôĞÂÌÖÂÛÇø");
	memset(&newboard, 0, sizeof (newboard));
	move(2, 0);
	ansimore2("etc/boardref", NA, 3, 7);
	while (1) {
		getdata(10, 0, "ÌÖÂÛÇøÃû³Æ:   ", newboard.filename, 18, DOECHO, YEA);
		if (newboard.filename[0] != 0) {
			struct boardheader dh;
			if (search_record(BOARDS, &dh, sizeof (dh), cmpbnames,
					newboard.filename)) {
				prints("\n´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ!!");
				pressanykey();
				return -1;
			}
		} else {
			return -1;
		}
		if (valid_brdname(newboard.filename))
			break;
		prints("\n²»ºÏ·¨Ãû³Æ!!");
	}
	newboard.flag = 0;
	while (1) {
		getdata(11, 0, "ÌÖÂÛÇøËµÃ÷:   ", newboard.title, 60, DOECHO, YEA);
		if (newboard.title[0] == '\0')
			return -1;
		if (strstr(newboard.title, "¡ñ") || strstr(newboard.title, "¡Ñ")) {
			newboard.flag |= BOARD_OUT_FLAG;
			break;
		} else if (strstr(newboard.title, "¡ğ")) {
			newboard.flag &= ~BOARD_OUT_FLAG;
			break;
		} else {
			prints("´íÎóµÄ¸ñÊ½, ÎŞ·¨ÅĞ¶ÏÊÇ·ñ×ªĞÅ!!");
		}
	}
	strcpy(vbuf, "vote/");
	strcat(vbuf, newboard.filename);
	setbpath(genbuf, newboard.filename);
	if (getbnum(newboard.filename, &currentuser) > 0 || mkdir(genbuf, 0755) == -1
			|| mkdir(vbuf, 0755) == -1) {
		prints("\n´íÎóµÄÌÖÂÛÇøÃû³Æ!!\n");
		pressreturn();
		clear();
		return -1;
	}
	//sprintf(vbuf, "/dev/shm/bbs/boards/%s", newboard.filename);
	//mkdir(vbuf, 0755);

	move(12, 0);
	if (gettheboardname(12, "ÊäÈëËùÊôÌÖÂÛÇøÃû: ", &pos, &fh, ans, 2)) {
		newboard.group = pos;
	} else {
		newboard.group = 0;
		newboard.flag |= BOARD_NOZAP_FLAG; //root dir can't zap.Danielfree 06.2.22
	}
	if (askyn("±¾°æÊÇÄ¿Â¼Âğ?", NA, NA) == YEA) {
		newboard.flag |= BOARD_DIR_FLAG;
		//suggest by monoply.06.2.22
		newboard.flag |= BOARD_JUNK_FLAG;
		newboard.flag |= BOARD_NOREPLY_FLAG;
		newboard.flag |= BOARD_POST_FLAG;
		if (askyn("ÊÇ·ñÏŞÖÆ´æÈ¡È¨Á¦", NA, NA) == YEA) {
			getdata(14, 0, "ÏŞÖÆ Read? [R]: ", ans, 2, DOECHO, YEA);
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("Éè¶¨ %s È¨Á¦. ÌÖÂÛÇø: '%s'\n", "READ", newboard.filename);
			newboard.level = setperms(newboard.level, "È¨ÏŞ", NUMPERMS,
					showperminfo);
			clear();
		} else {
			newboard.level = 0;
		}
		//add  end
	} else {
		newboard.flag &= ~BOARD_DIR_FLAG;

		if (askyn("±¾°æ³ÏÕ÷°æÖ÷Âğ(·ñÔòÓÉSYSOPs¹ÜÀí)?", YEA, NA) == NA) {
			strcpy(newboard.BM, "SYSOPs");
		} else {
			newboard.BM[0] = '\0';
		}

		if (askyn("¸Ã°æµÄÈ«²¿ÎÄÕÂ¾ù²»¿ÉÒÔ»Ø¸´", NA, NA) == YEA) {
			newboard.flag |= BOARD_NOREPLY_FLAG;
		} else {
			newboard.flag &= ~BOARD_NOREPLY_FLAG;
		}

		if (askyn("ÊÇ·ñÊÇ¾ãÀÖ²¿°æÃæ", NA, NA) == YEA) {
			newboard.flag |= BOARD_CLUB_FLAG;
			if (askyn("ÊÇ·ñ¶ÁÏŞÖÆ¾ãÀÖ²¿°æÃæ", NA, NA) == YEA) {
				newboard.flag |= BOARD_READ_FLAG;
			} else {
				newboard.flag &= ~BOARD_READ_FLAG;
			}
		} else {
			newboard.flag &= ~BOARD_CLUB_FLAG;
		}

		if (askyn("ÊÇ·ñ²»¼ÆËãÎÄÕÂÊı", NA, NA) == YEA) {
			newboard.flag |= BOARD_JUNK_FLAG;
		} else {
			newboard.flag &= ~BOARD_JUNK_FLAG;
		}

		if (askyn("ÊÇ·ñ¼ÓÈëÄäÃû°æ", NA, NA) == YEA) {
			newboard.flag |= BOARD_ANONY_FLAG;
		} else {
			newboard.flag &= ~BOARD_ANONY_FLAG;
		}
#ifdef ENABLE_PREFIX
		if (askyn ("ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º", NA, NA) == YEA) {
			newboard.flag |= BOARD_PREFIX_FLAG;
		} else {
			newboard.flag &= ~BOARD_PREFIX_FLAG;
		}
#endif
		if (askyn("ÊÇ·ñÏŞÖÆ´æÈ¡È¨Á¦", NA, NA) == YEA) {
			getdata(14, 0, "ÏŞÖÆ Read/Post? [R]: ", ans, 2, DOECHO, YEA);
			if (*ans == 'P' || *ans == 'p') {
				newboard.flag |= BOARD_POST_FLAG;
			} else {
				newboard.flag &= ~BOARD_POST_FLAG;
			}
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("Éè¶¨ %s È¨Á¦. ÌÖÂÛÇø: '%s'\n", (newboard.flag
					& BOARD_POST_FLAG ? "POST" : "READ"),
					newboard.filename);
			newboard.level = setperms(newboard.level, "È¨ÏŞ", NUMPERMS,
					showperminfo);
			clear();
		} else {
			newboard.level = 0;
		}
	}
	if (askyn("ÊÇ·ñ ¿ÉÒÔ ZAPÌÖÂÛÇø£¿", (newboard.flag & BOARD_NOZAP_FLAG) ? NA
			: YEA, YEA) == NA) {
		newboard.flag |= BOARD_NOZAP_FLAG;
	} else {
		newboard.flag &= ~BOARD_NOZAP_FLAG;
	}
	if ((bid = getblankbnum()) > 0) {
		substitute_record(BOARDS, &newboard, sizeof (newboard), bid);
		flush_bcache();
	} else if (append_record(BOARDS, &newboard, sizeof (newboard)) == -1) {
		pressreturn();
		clear();
		return -1;
	}

	if (!(newboard.flag & BOARD_DIR_FLAG)) {
		group = chgrp();
		if (group != NULL) {
			if (newboard.BM[0] != '\0') {
				sprintf(vbuf, "%-38.38s(BM: %s)", newboard.title + 8,
						newboard.BM);
			} else {
				sprintf(vbuf, "%-38.38s", newboard.title + 8);
			}
			if (add_grp(group, cexplain, newboard.filename, vbuf) == -1) {
				prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
			} else {
				prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
			}
		}
	}

	flush_bcache();
	rebuild_brdshm(); //add by cometcaptor 2006-10-13
	prints("\nĞÂÌÖÂÛÇø³ÉÁ¢\n");

	char secu[STRLEN];
	sprintf(secu, "³ÉÁ¢ĞÂ°æ£º%s", newboard.filename);
	securityreport(secu, 0, 1);

	pressreturn();
	clear();
	return 0;
}

//      ĞŞ¸ÄÌÖÂÛÇøÉè¶¨
int m_editbrd() {
	char bname[STRLEN], buf[STRLEN], oldtitle[STRLEN], vbuf[256], *group;
	char type[10];
	char oldpath[STRLEN], newpath[STRLEN], tmp_grp[30];
	int pos, tmppos, a_mv;
	struct boardheader fh, newfh, tmpfh;

	a_mv = 0; // added by Danielfree 05.12.4

	//added by roly 02.03.07
	if (!(HAS_PERM(PERM_BLEVELS)))
		return;
	//add end

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("ĞŞ¸ÄÌÖÂÛÇø×ÊÑ¶");
	if (!gettheboardname(2, "ÊäÈëÌÖÂÛÇøÃû³Æ: ", &pos, &fh, bname, 0))
		return -1;
	if (fh.flag & BOARD_DIR_FLAG)
		sprintf(type, "Ä¿Â¼");
	else
		sprintf(type, "°æÃæ");
	move(2, 0);
	memcpy(&newfh, &fh, sizeof (newfh));
	while (1) {
		clear();
		stand_title("ĞŞ¸ÄÌÖÂÛÇø×ÊÑ¶");
		move(2, 0);
		prints("1)ĞŞ¸Ä%sÃû³Æ:            %s\n", type, newfh.filename);
		prints("2)ĞŞ¸Ä%sËµÃ÷:            %s\n", type, newfh.title);
		prints("3)ĞŞ¸Ä%s¹ÜÀíÔ±:          %s\n", type, newfh.BM);
		prints("4)ĞŞ¸Ä%sËùÊôÄ¿Â¼:        %s(%d)\n", type,
				bcache[fh.group - 1].filename, newfh.group);
		if (fh.flag & BOARD_DIR_FLAG) {
			prints("5)ĞŞ¸Ä%s¶ÁĞ´ÊôĞÔ:        %s\n", type,
					(newfh.level == 0) ? "Ã»ÓĞÏŞÖÆ" : "r(ÏŞÖÆÔÄ¶Á)");
		} else {
			prints("5)ĞŞ¸Ä%s¶ÁĞ´ÊôĞÔ:        %s\n", type, (newfh.flag
					& BOARD_POST_FLAG) ? "p(ÏŞÖÆ·¢ÎÄ)"
					: (newfh.level == 0) ? "Ã»ÓĞÏŞÖÆ" : "r(ÏŞÖÆÔÄ¶Á)");
		}
		//zap dir and board. Danielfree 06.2.22
		prints("6)¿ÉÒÔZAP%s:             %s\n", type, (newfh.flag
				& BOARD_NOZAP_FLAG) ? "¿É" : "·ñ");
		if (!(newfh.flag & BOARD_DIR_FLAG)) {
			prints("7)ÒÆ¶¯¾«»ªÇøÎ»ÖÃ\n");
			//prints ("7)¿ÉÒÔZAP°æÃæ:             %s\n",
			//    (newfh.flag & BOARD_POST_FLAG) ? "¿É" : "·ñ");
			prints("8)ÄäÃû°æÃæ:                %s\n", (newfh.flag
					& BOARD_ANONY_FLAG) ? "ÄäÃû" : "²»ÄäÃû");
			prints("9)¿ÉÒÔ»Ø¸´:                %s\n", (newfh.flag
					& BOARD_NOREPLY_FLAG) ? "²»¿É»Ø¸´" : "¿ÉÒÔ»Ø¸´");
			prints("A)ÊÇ·ñ¼ÆËãÎÄÕÂÊı:          %s\n", (newfh.flag
					& BOARD_JUNK_FLAG) ? "²»¼ÆËã" : "¼ÆËã");
			prints(
					"B)¾ãÀÖ²¿ÊôĞÔ:              %s\n",
					(newfh.flag & BOARD_CLUB_FLAG) ? (newfh.flag
							& BOARD_READ_FLAG) ? "\033[1;31mc\033[0m(¶ÁÏŞÖÆ¾ãÀÖ²¿)"
							: "\033[1;33mc\033[0m(ÆÕÍ¨¾ãÀÖ²¿)"
							: "²»ÊÇ¾ãÀÖ²¿");
#ifdef ENABLE_PREFIX
			prints ("C)ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º:        %s\n",
					(newfh.flag & BOARD_PREFIX_FLAG) ? "±ØĞë" : "²»±Ø");
#endif
			getdata(14, 0, "¸ü¸ÄÄÄÏî×ÊÑ¶£¿[1-9,A,B][0]", genbuf, 2, DOECHO, YEA);
		} else {
			getdata(14, 0, "¸ü¸ÄÄÄÏî×ÊÑ¶£¿[1-6][0]", genbuf, 2, DOECHO, YEA);
		}
		if (genbuf[0] == '0' || genbuf[0] == 0)
			break;
		move(15, 0);
		strcpy(oldtitle, fh.title);
		switch (genbuf[0]) {
			case '1':
				while (1) {
					sprintf(buf, "ĞÂÌÖÂÛÇøÃû³Æ[%s]: ", fh.filename);
					getdata(15, 0, buf, genbuf, 18, DOECHO, YEA);
					if (*genbuf != 0) {
						struct boardheader dh;
						if (search_record(BOARDS, &dh, sizeof (dh),
								cmpbnames, genbuf)) {
							move(16, 0);
							prints("´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ!!");
							move(0, 0);
							clrtoeol();
							continue;
						}
						if (valid_brdname(genbuf)) {
							strlcpy(newfh.filename, genbuf,
									sizeof (newfh.filename));
							strcpy(bname, genbuf);
							break;
						} else {
							move(16, 0);
							prints("²»ºÏ·¨µÄÌÖÂÛÇøÃû³Æ!");
							move(0, 0);
							clrtoeol();
							continue;
						}
					} else {
						break;
					}
				}
				break;
			case '2':
				ansimore2("etc/boardref", NA, 11, 7);
				snprintf(genbuf, sizeof(newfh.title), "%s", newfh.title);
				while (1) {
					getdata(22, 0, "ĞÂÌÖÂÛÇøËµÃ÷: ", genbuf, 60, DOECHO, YEA);
					if (*genbuf != 0) {
						strlcpy(newfh.title, genbuf, sizeof (newfh.title));
					} else {
						break;
					}
					if (strstr(newfh.title, "¡ñ") || strstr(newfh.title,
							"¡Ñ")) {
						newfh.flag |= BOARD_OUT_FLAG;
						break;
					} else if (strstr(newfh.title, "¡ğ")) {
						newfh.flag &= ~BOARD_OUT_FLAG;
						break;
					} else {
						prints("\n´íÎóµÄ¸ñÊ½, ÎŞ·¨ÅĞ¶ÏÊÇ·ñ×ªĞÅ!!");
					}
				}
				break;
			case '3':
				if (fh.BM[0] != '\0' && strcmp(fh.BM, "SYSOPs")) {
					if (askyn("ĞŞ¸ÄÌÖÂÛÇø¹ÜÀíÔ±¡£×¢Òâ£º½ö¹©³ö´íĞŞÕıÊ¹ÓÃ£¬°æÖ÷ÈÎÃâÇëÎğ¸Ä¶¯´Ë´¦£¡", NA, NA)
							== YEA) {
						getdata(16, 0, "ÌÖÂÛÇø¹ÜÀíÔ±: ", newfh.BM,
								sizeof (newfh.BM), DOECHO, YEA);
						if (newfh.BM[0] == '\0') {
							strcpy(newfh.BM, fh.BM);
						} else if (newfh.BM[0] == ' ') {
							newfh.BM[0] = '\0';
						}
					}
				} else {
					if (askyn("±¾°æ³ÏÕ÷°æÖ÷Âğ(·ñ£¬ÔòÓÉSYSOPs¹ÜÀí)?", YEA, NA) == NA) {
						strlcpy(newfh.BM, "SYSOPs", sizeof (newfh.BM));
					} else {
						strlcpy(newfh.BM, "\0", sizeof (newfh.BM));
					}
				}
				break;
			case '4':
				if (gettheboardname(15, "ÊäÈëËùÊôÌÖÂÛÇøÃû: ", &tmppos, &tmpfh,
						genbuf, 2))
					newfh.group = tmppos;
				else if (askyn("ËùÊôÌÖÂÛÇøÎª¸ùÄ¿Â¼Ã´£¿", NA, NA) == YEA)
					newfh.group = 0;
				break;
			case '5':
				if (fh.flag & BOARD_DIR_FLAG) { //modiy for dir. Danielfree 06.2.23
					sprintf(buf, "(N)²»ÏŞÖÆ (R)ÏŞÖÆÔÄ¶Á [%c]: ",
							(newfh.level) ? 'R' : 'N');
					getdata(15, 0, buf, genbuf, 2, DOECHO, YEA);
					if (genbuf[0] == 'N' || genbuf[0] == 'n') {
						newfh.flag &= ~BOARD_POST_FLAG;
						newfh.level = 0;
					} else {
						if (genbuf[0] == 'R' || genbuf[0] == 'r')
							newfh.flag &= ~BOARD_POST_FLAG;
						clear();
						move(2, 0);
						prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏŞ\n", "ÔÄ¶Á", newfh.filename);
						newfh.level = setperms(newfh.level, "È¨ÏŞ",
								NUMPERMS, showperminfo);
						clear();
					}
				} // if dir
				else { //if board
					sprintf(buf, "(N)²»ÏŞÖÆ (R)ÏŞÖÆÔÄ¶Á (P)ÏŞÖÆÕÅÌù ÎÄÕÂ [%c]: ",
							(newfh.flag & BOARD_POST_FLAG) ? 'P' : (newfh.
							level) ? 'R' : 'N');
					getdata(15, 0, buf, genbuf, 2, DOECHO, YEA);
					if (genbuf[0] == 'N' || genbuf[0] == 'n') {
						newfh.flag &= ~BOARD_POST_FLAG;
						newfh.level = 0;
					} else {
						if (genbuf[0] == 'R' || genbuf[0] == 'r')
							newfh.flag &= ~BOARD_POST_FLAG;
						else if (genbuf[0] == 'P' || genbuf[0] == 'p')
							newfh.flag |= BOARD_POST_FLAG;
						clear();
						move(2, 0);
						prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏŞ\n", newfh.flag
								& BOARD_POST_FLAG ? "ÕÅÌù" : "ÔÄ¶Á",
								newfh.filename);
						newfh.level = setperms(newfh.level, "È¨ÏŞ",
								NUMPERMS, showperminfo);
						clear();
					}
				}
				break;
				//both dir and board can zap. Danielfree 06.2.22
			case '6':
				if (askyn("ÊÇ·ñ ¿ÉÒÔ ZAPÌÖÂÛÇø£¿",
						(fh.flag & BOARD_NOZAP_FLAG) ? NA : YEA, YEA)
						== NA) {
					newfh.flag |= BOARD_NOZAP_FLAG;
				} else {
					newfh.flag &= ~BOARD_NOZAP_FLAG;
				}
				break;
				//modify end
			default:
				if (!(fh.flag & BOARD_DIR_FLAG)) {
					switch (genbuf[0]) {
						case '7':
							a_mv = 2;
							break; // move from out of default into default -.- Danielfree 05.12.4
						case '8':
							if (askyn("ÊÇ·ñÄäÃû°æ£¿", (fh.flag
									& BOARD_ANONY_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_ANONY_FLAG;
							} else {
								newfh.flag &= ~BOARD_ANONY_FLAG;
							}

							break;
						case '9':
							if (askyn("ÎÄÕÂÊÇ·ñ ¿ÉÒÔ »Ø¸´£¿", (fh.flag
									& BOARD_NOREPLY_FLAG) ? NA : YEA, YEA)
									== NA) {
								newfh.flag |= BOARD_NOREPLY_FLAG;
							} else {
								newfh.flag &= ~BOARD_NOREPLY_FLAG;
							}
							break;
						case 'a':
						case 'A':
							if (askyn("ÊÇ·ñ ²»¼ÆËã ÎÄÕÂÊı£¿", (fh.flag
									& BOARD_JUNK_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_JUNK_FLAG;
							} else {
								newfh.flag &= ~BOARD_JUNK_FLAG;
							}
							break;
						case 'b':
						case 'B':
							if (askyn("ÊÇ·ñ¾ãÀÖ²¿°æÃæ£¿", (fh.flag
									& BOARD_CLUB_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_CLUB_FLAG;
								if (askyn("ÊÇ·ñ¶ÁÏŞÖÆ¾ãÀÖ²¿£¿", (fh.flag
										& BOARD_READ_FLAG) ? YEA : NA, NA)
										== YEA) {
									newfh.flag |= BOARD_READ_FLAG;
								} else {
									newfh.flag &= ~BOARD_READ_FLAG;
								}
							} else {
								newfh.flag &= ~BOARD_CLUB_FLAG;
								newfh.flag &= ~BOARD_READ_FLAG;
							}
							break;
#ifdef ENABLE_PREFIX
							case 'c':
							case 'C':
							if (askyn("ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º£¿", (fh.flag & BOARD_PREFIX_FLAG) ? YEA : NA, NA)
									== YEA) {
								newfh.flag |= BOARD_PREFIX_FLAG;
							} else {
								newfh.flag &= ~BOARD_PREFIX_FLAG;
							}
#endif
					}//wswitch
				}//if dir
		}//switch
	}//while
	getdata(23, 0, "È·¶¨Òª¸ü¸ÄÂğ? (Y/N) [N]: ", genbuf, 4, DOECHO, YEA);
	if (*genbuf == 'Y' || *genbuf == 'y') {
		char secu[STRLEN];
		sprintf(secu, "ĞŞ¸ÄÌÖÂÛÇø£º%s(%s)", fh.filename, newfh.filename);
		securityreport(secu, 0, 1);
		if (strcmp(fh.filename, newfh.filename)) {
			char old[256], tar[256];
			a_mv = 1;
			setbpath(old, fh.filename);
			setbpath(tar, newfh.filename);
			rename(old, tar);
			sprintf(old, "vote/%s", fh.filename);
			sprintf(tar, "vote/%s", newfh.filename);
			rename(old, tar);
		}
		if (newfh.BM[0] != '\0')
			sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 8, newfh.BM);
		else
			sprintf(vbuf, "%-38.38s", newfh.title + 8);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, oldtitle + 8, vbuf);
		if (a_mv >= 1) {
			group = chgrp();
			get_grp(fh.filename);
			strcpy(tmp_grp, lookgrp);
			if (strcmp(tmp_grp, group) || a_mv == 1) {
				char tmpbuf[160];
				sprintf(tmpbuf, "%s:", fh.filename);
				del_from_file("0Announce/.Search", tmpbuf);
				if (group != NULL) {
					if (newfh.BM[0] != '\0')
						sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 8,
								newfh.BM);
					else
						sprintf(vbuf, "%-38.38s", newfh.title + 8);
					if (add_grp(group, cexplain, newfh.filename, vbuf)
							== -1)
						prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
					else
						prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
					sprintf(newpath, "0Announce/groups/%s/%s", group,
							newfh.filename);
					sprintf(oldpath, "0Announce/groups/%s/%s", tmp_grp,
							fh.filename);
					if (strcmp(oldpath, newpath) != 0 && dashd(oldpath)) {
						deltree(newpath);
						rename(oldpath, newpath);
						del_grp(tmp_grp, fh.filename, fh.title + 8);
					} // add by Danielfree,suggest by fancitron 05.12.4
				} // if group !=NULL
			} // if strcmp
		} // if a_mv >= 1
		substitute_record(BOARDS, &newfh, sizeof (newfh), pos);
		sprintf(genbuf, "¸ü¸ÄÌÖÂÛÇø %s µÄ×ÊÁÏ --> %s", fh.filename, newfh.filename);
		report(genbuf, currentuser.userid);
		// numboards = -1;/* force re-caching */
		flush_bcache();
	} // if askyn
	clear();
	return 0;

}

// Åú×¢²áµ¥Ê±ÏÔÊ¾µÄ±êÌâ
void regtitle() {
	prints("[1;33;44mÅú×¢²áµ¥ NEW VERSION wahahaha                                                   [m\n");
	prints(" Àë¿ª[[1;32m¡û[m,[1;32me[m] Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m] ÔÄ¶Á[[1;32m¡ú[m,[1;32mRtn[m] Åú×¼[[1;32my[m] É¾³ı[[1;32md[m]\n");

	prints("[1;37;44m  ±àºÅ ÓÃ»§ID       ĞÕ  Ãû       Ïµ±ğ             ×¡Ö·             ×¢²áÊ±¼ä     [m\n");
}

//      ÔÚÅú×¢²áµ¥Ê±ÏÔÊ¾µÄ×¢²áIDÁĞ±í
char *regdoent(int num, REGINFO * ent) {
	static char buf[128];
	char rname[17];
	char dept[17];
	char addr[17];
	//struct tm* tm;
	//tm=gmtime(&ent->regdate);
	strlcpy(rname, ent->realname, 12);
	strlcpy(dept, ent->dept, 16);
	strlcpy(addr, ent->addr, 16);
	ellipsis(rname, 12);
	ellipsis(dept, 16);
	ellipsis(addr, 16);
	getdatestring(ent->regdate, 2);
	sprintf(buf, "  %4d %-12s %-12s %-16s %-16s %s", num, ent->userid,
			rname, dept, addr, datestring);
	return buf;
}

//      ·µ»Øuserid Óëent->useridÊÇ·ñÏàµÈ
int filecheck(REGINFO * ent, char *userid) {
	return !strcmp(ent->userid, userid);
}

// É¾³ı×¢²áµ¥ÎÄ¼şÀïµÄÒ»¸ö¼ÇÂ¼
int delete_register(int index, REGINFO * ent, char *direct) {
	delete_record(direct, sizeof(REGINFO), index, filecheck, ent->userid);
	return DIRCHANGED;
}

//      Í¨¹ı×¢²áµ¥
int pass_register(int index, REGINFO * ent, char *direct) {
	int unum;
	struct userec uinfo;
	char buf[80];
	FILE *fout;

	strlcpy(uinfo.realname, ent->realname, NAMELEN);
	unum = getuser(ent->userid);
	if (!unum) {
		clear();
		prints("ÏµÍ³´íÎó! ²éÎŞ´ËÕËºÅ!\n"); //      ÔÚ»Øµµ»òÕßÄ³Ğ©Çé¿öÏÂ,ÕÒ²»µ½ÔÚ×¢²áµ¥ÎÄ¼ş
		pressanykey(); // unregisterÖĞµÄ´Ë¼ÇÂ¼,¹ÊÉ¾³ı
		delete_record(direct, sizeof(REGINFO), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	delete_record(direct, sizeof(REGINFO), index, filecheck, ent->userid);

	memcpy(&uinfo, &lookupuser, sizeof (uinfo));
	strlcpy(uinfo.address, ent->addr, NAMELEN);
	sprintf(genbuf, "%s$%s@%s", ent->dept, ent->phone, currentuser.userid);
	genbuf[STRLEN - 16] = '\0';
	strlcpy(uinfo.reginfo, genbuf, STRLEN - 17);
#ifdef ALLOWGAME
	uinfo.money = 1000;
#endif
	uinfo.lastjustify = time(0);
	substitut_record(PASSFILE, &uinfo, sizeof (uinfo), unum);
	sethomefile(buf, uinfo.userid, "register");
	if ((fout = fopen(buf, "a")) != NULL) {
		getdatestring(ent->regdate, YEA);
		fprintf(fout, "×¢²áÊ±¼ä     : %s\n", datestring);
		fprintf(fout, "ÉêÇëÕÊºÅ     : %s\n", ent->userid);
		fprintf(fout, "ÕæÊµĞÕÃû     : %s\n", ent->realname);
		fprintf(fout, "Ñ§Ğ£Ïµ¼¶     : %s\n", ent->dept);
		fprintf(fout, "Ä¿Ç°×¡Ö·     : %s\n", ent->addr);
		fprintf(fout, "ÁªÂçµç»°     : %s\n", ent->phone);
#ifndef FDQUAN
		fprintf(fout, "µç×ÓÓÊ¼ş     : %s\n", ent->email);
#endif
		fprintf(fout, "Ğ£ ÓÑ »á     : %s\n", ent->assoc);
		getdatestring(time(0), YEA);
		fprintf(fout, "³É¹¦ÈÕÆÚ     : %s\n", datestring);
		fprintf(fout, "Åú×¼ÈË       : %s\n", currentuser.userid);
		fclose(fout);
	}
	mail_file("etc/s_fill", uinfo.userid, "¹§ìûÄú£¬ÄúÒÑ¾­Íê³É×¢²á¡£");
	sethomefile(buf, uinfo.userid, "mailcheck");
	unlink(buf);
	sprintf(genbuf, "ÈÃ %s Í¨¹ıÉí·ÖÈ·ÈÏ.", uinfo.userid);
	securityreport(genbuf, 0, 0);

	return DIRCHANGED;
}

//      ´¦Àí×¢²áµ¥
int do_register(int index, REGINFO * ent, char *direct) {
	int unum;
	struct userec uinfo;
	//char ps[80];
	register int ch;
	static char *reason[] = { "ÇëÈ·ÊµÌîĞ´ÕæÊµĞÕÃû.", "ÇëÏêÌîÑ§Ğ£¿ÆÏµÓëÄê¼¶.", "ÇëÌîĞ´ÍêÕûµÄ×¡Ö·×ÊÁÏ.",
			"ÇëÏêÌîÁªÂçµç»°.", "ÇëÈ·ÊµÌîĞ´×¢²áÉêÇë±í.", "ÇëÓÃÖĞÎÄÌîĞ´ÉêÇëµ¥.", "ÆäËû" };
	unsigned char rejectindex = 4;

	if (!ent)
		return DONOTHING;

	unum = getuser(ent->userid);
	if (!unum) {
		prints("ÏµÍ³´íÎó! ²éÎŞ´ËÕËºÅ!\n"); //É¾³ı²»´æÔÚµÄ¼ÇÂ¼,Èç¹ûÓĞµÄ»°
		delete_record(direct, sizeof(REGINFO), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	memcpy(&uinfo, &lookupuser, sizeof (uinfo));
	clear();
	move(0, 0);
	prints("[1;33;44m ÏêÏ¸×ÊÁÏ                                                                      [m\n");
	prints("[1;37;42m [.]½ÓÊÜ [+]¾Ü¾ø [d]É¾³ı [0-6]²»·ûºÏÔ­Òò                                       [m");

	//strcpy(ps, "(ÎŞ)");
	for (;;) {
		disply_userinfo(&uinfo);
		move(14, 0);
		printdash(NULL);
		getdatestring(ent->regdate, YEA);
		prints("   ×¢²áÊ±¼ä   : %s\n", datestring);
		prints("   ÉêÇëÕÊºÅ   : %s\n", ent->userid);
		prints("   ÕæÊµĞÕÃû   : %s\n", ent->realname);
		prints("   Ñ§Ğ£Ïµ¼¶   : %s\n", ent->dept);
		prints("   Ä¿Ç°×¡Ö·   : %s\n", ent->addr);
		prints("   ÁªÂçµç»°   : %s\n", ent->phone);
#ifndef FDQUAN
		prints("   µç×ÓÓÊ¼ş   : %s\n", ent->email);
#endif
		prints("   Ğ£ ÓÑ »á   : %s\n", ent->assoc);
		ch = egetch();
		switch (ch) {
			case '.':
				pass_register(index, ent, direct);
				return READ_AGAIN;
			case '+':
				uinfo.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &uinfo, sizeof (uinfo), unum);
				//mail_file("etc/f_fill", uinfo.userid, "ÇëÖØĞÂÌîĞ´ÄúµÄ×¢²á×ÊÁÏ");
				mail_file("etc/f_fill", uinfo.userid, reason[rejectindex]);
			case 'd':
				uinfo.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &uinfo, sizeof (uinfo), unum);
				delete_register(index, ent, direct);
				return READ_AGAIN;
			case KEY_DOWN:
			case '\r':
				return READ_NEXT;
			case KEY_LEFT:
				return DIRCHANGED;
			default:
				if (ch >= '0' && ch <= '6') {
					rejectindex = ch - '0';
					//strcpy(uinfo.address, reason[ch-'0']);
				}
				break;
		}
	}
	return 0;
}

struct one_key reg_comms[] = { 'r', do_register, 'y', pass_register, 'd',
		delete_register, '\0', NULL };
void show_register() {
	FILE *fn;
	int x; //, y, wid, len;
	char uident[STRLEN];
	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("²éÑ¯Ê¹ÓÃÕß×¢²á×ÊÁÏ");
	move(1, 0);
	usercomplete("ÇëÊäÈëÒª²éÑ¯µÄ´úºÅ: ", uident);
	if (uident[0] != '\0') {
		if (!getuser(uident)) {
			move(2, 0);
			prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
		} else {
			sprintf(genbuf, "home/%c/%s/register",
					toupper(lookupuser.userid[0]), lookupuser.userid);
			if ((fn = fopen(genbuf, "r")) != NULL) {
				prints("\n×¢²á×ÊÁÏÈçÏÂ:\n\n");
				for (x = 1; x <= 15; x++) {
					if (fgets(genbuf, STRLEN, fn))
						prints("%s", genbuf);
					else
						break;
				}
			} else {
				prints("\n\nÕÒ²»µ½Ëû/ËıµÄ×¢²á×ÊÁÏ!!\n");
			}
		}
	}
	pressanykey();
}
//  ½øÈë ×¢²áµ¥²ì¿´À¸,¿´Ê¹ÓÃÕßµÄ×¢²á×ÊÁÏ»ò½ø×¢²áµ¥¹ÜÀí³ÌĞò
int m_register() {
	FILE *fn;
	char ans[3]; //, *fname;
	int x; //, y, wid, len;
	char uident[STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();

	stand_title("Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
	for (;;) {
		getdata(1, 0, "(0)Àë¿ª  (1)Éó²éĞÂ×¢²á (2)²éÑ¯Ê¹ÓÃÕß×¢²á×ÊÁÏ ? : ", ans, 2, DOECHO,
				YEA);
		if (ans[0] == '1' || ans[0] == '2') { // || ans[0]=='3') ÏÖÔÚÖ»ÓĞ0,1,2
			break;
		} else {
			return 0;
		}
	}
	switch (ans[0]) {
		/*
		 case '1':
		 fname = "new_register";
		 if ((fn = fopen(fname, "r")) == NULL) {
		 prints("\n\nÄ¿Ç°²¢ÎŞĞÂ×¢²á×ÊÁÏ.");
		 pressreturn();
		 } else {
		 y = 3, x = wid = 0;
		 while (fgets(genbuf, STRLEN, fn) != NULL && x < 65) {
		 if (strncmp(genbuf, "userid: ", 8) == 0) {
		 move(y++, x);
		 prints("%s",genbuf + 8);
		 len = strlen(genbuf + 8);
		 if (len > wid)
		 wid = len;
		 if (y >= t_lines - 2) {
		 y = 3;
		 x += wid + 2;
		 }
		 }
		 }
		 fclose(fn);
		 if (askyn("Éè¶¨×ÊÁÏÂğ", NA, YEA) == YEA) {
		 securityreport("Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
		 scan_register_form(fname);
		 }
		 }
		 break; */
		case '2':
			move(1, 0);
			usercomplete("ÇëÊäÈëÒª²éÑ¯µÄ´úºÅ: ", uident);
			if (uident[0] != '\0') {
				if (!getuser(uident)) {
					move(2, 0);
					prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				} else {
					sprintf(genbuf, "home/%c/%s/register",
							toupper(lookupuser.userid[0]),
							lookupuser.userid);
					if ((fn = fopen(genbuf, "r")) != NULL) {
						prints("\n×¢²á×ÊÁÏÈçÏÂ:\n\n");
						for (x = 1; x <= 15; x++) {
							if (fgets(genbuf, STRLEN, fn))
								prints("%s", genbuf);
							else
								break;
						}
					} else {
						prints("\n\nÕÒ²»µ½Ëû/ËıµÄ×¢²á×ÊÁÏ!!\n");
					}
				}
			}
			pressanykey();
			break;
		case '1':
			i_read(ADMIN, "unregistered", regtitle, regdoent,
					&reg_comms[0], sizeof(REGINFO));
			break;
	}
	clear();
	return 0;
}

//      É¾³ıÌÖÂÛÇø
int d_board() {
	struct boardheader binfo;
	int bid, ans;
	char bname[STRLEN];
	extern char lookgrp[];
	char genbuf_rm[STRLEN]; //added by roly 02.03.24

	if (!HAS_PERM(PERM_BLEVELS)) {
		return 0;
	}
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("É¾³ıÌÖÂÛÇø");
	make_blist(0); //Éú³ÉÌÖÂÛÇøÁĞ±í
	move(1, 0);
	namecomplete("ÇëÊäÈëÌÖÂÛÇø: ", bname);
	if (bname[0] == '\0')
		return 0;
	bid = getbnum(bname, &currentuser);
	if (get_record(BOARDS, &binfo, sizeof (binfo), bid) == -1) { //È¡µÃÌÖÂÛÇøµÄ¼ÇÂ¼
		move(2, 0);
		prints("²»ÕıÈ·µÄÌÖÂÛÇø\n");
		pressreturn();
		clear();
		return 0;
	}
	if (binfo.BM[0] != '\0' && strcmp(binfo.BM, "SYSOPs")) { //»¹ÓĞ²»ÊÇ½ĞSYSOPsµÄ°æÖ÷
		move(5, 0);
		prints("¸Ã°æ»¹ÓĞ°æÖ÷£¬ÔÚÉ¾³ı±¾°æÇ°£¬ÇëÏÈÈ¡Ïû°æÖ÷µÄÈÎÃü¡£\n");
		pressanykey();
		clear();
		return 0;
	}
	ans = askyn("ÄãÈ·¶¨ÒªÉ¾³ıÕâ¸öÌÖÂÛÇø", NA, NA);
	if (ans != 1) {
		move(2, 0);
		prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
		pressreturn();
		clear();
		return 0;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "É¾³ıÌÖÂÛÇø£º%s", binfo.filename);
		securityreport(secu, 0, 1);
	}
	if (seek_in_file("0Announce/.Search", bname)) {
		move(4, 0);
		if (askyn("ÒÆ³ı¾«»ªÇø", NA, NA) == YEA) {
			get_grp(binfo.filename);
			del_grp(lookgrp, binfo.filename, binfo.title + 8);
		}
	}
	if (seek_in_file("etc/junkboards", bname))
		del_from_file("etc/junkboards", bname);
	if (seek_in_file("0Announce/.Search", bname)) {
		char tmpbuf[160];
		sprintf(tmpbuf, "%s:", bname);
		del_from_file("0Announce/.Search", tmpbuf);
	}
	if (binfo.filename[0] == '\0') {
		return -1; /* rrr - precaution */
	}
	sprintf(genbuf, "boards/%s", binfo.filename);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm); //Óëf_rm(genbuf)ÊÇ²»ÊÇÖØ¸´ÁË?
	/* add end */
	sprintf(genbuf, "vote/%s", binfo.filename);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm);
	/* add end */
	sprintf(genbuf, " << '%s' ±» %s É¾³ı >>", binfo.filename,
			currentuser.userid);
	memset(&binfo, 0, sizeof (binfo));
	strlcpy(binfo.title, genbuf, STRLEN);
	binfo.level = PERM_SYSOPS;
	substitute_record(BOARDS, &binfo, sizeof (binfo), bid);

	move(4, 0);
	prints("\n±¾ÌÖÂÛÇøÒÑ¾­É¾³ı...\n");
	pressreturn();
	flush_bcache();
	clear();
	return 0;
}

//      É¾³ıÒ»¸öÕÊºÅ
int d_user(char *cid) {
	int id, num, i;
	char secu[STRLEN];
	char genbuf_rm[STRLEN]; //added by roly 02.03.24
	char passbuf[PASSLEN];

	if (!(HAS_PERM(PERM_USER)))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	stand_title("É¾³ıÊ¹ÓÃÕßÕÊºÅ");
	// Added by Ashinmarch in 2008.10.20 
	// ¿³µôÕËºÅÊ±Ôö¼ÓÃÜÂëÑéÖ¤
	getdata(1, 0, "[1;37mÇëÊäÈëÃÜÂë: [m", passbuf, PASSLEN, NOECHO, YEA);
	passbuf[8] = '\0';
	if (!checkpasswd(currentuser.passwd, passbuf)) {
		prints("[1;31mÃÜÂëÊäÈë´íÎó...[m\n");
		return 0;
	}
	// Add end.
	if (!gettheuserid(1, "ÇëÊäÈëÓûÉ¾³ıµÄÊ¹ÓÃÕß´úºÅ: ", &id))
		return 0;
	if (!strcmp(lookupuser.userid, "SYSOP")) {
		prints("\n¶Ô²»Æğ£¬Äã²»¿ÉÒÔÉ¾³ı SYSOP ÕÊºÅ!!\n");
		pressreturn();
		clear();
		return 0;
	}
	if (!strcmp(lookupuser.userid, currentuser.userid)) {
		prints("\n¶Ô²»Æğ£¬Äã²»¿ÉÒÔÉ¾³ı×Ô¼ºµÄÕâ¸öÕÊºÅ!!\n");
		pressreturn();
		clear();
		return 0;
	}
	prints("\n\nÒÔÏÂÊÇ [%s] µÄ²¿·Ö×ÊÁÏ:\n", lookupuser.userid);
	prints("    User ID:  [%s]\n", lookupuser.userid);
	prints("    êÇ   ³Æ:  [%s]\n", lookupuser.username);
	prints("    ĞÕ   Ãû:  [%s]\n", lookupuser.realname);
	strcpy(secu, "ltmprbBOCAMURS#@XLEast0123456789\0");
	for (num = 0; num < strlen(secu) - 1; num++) {
		if (!(lookupuser.userlevel & (1 << num)))
			secu[num] = '-';
	}
	prints("    È¨   ÏŞ: %s\n\n", secu);

	num = getbnames(lookupuser.userid, secu, &num);
	if (num) {
		prints("[%s] Ä¿Ç°ÉĞµ£ÈÎÁË %d ¸ö°æµÄ°æÖ÷: ", lookupuser.userid, num);
		for (i = 0; i < num; i++)
			prints("%s ", bnames[i]);
		prints("\nÇëÏÈÊ¹ÓÃ°æÖ÷Ğ¶Ö°¹¦ÄÜÈ¡ÏûÆä°æÖ÷Ö°ÎñÔÙ×ö¸Ã²Ù×÷.");
		pressanykey();
		clear();
		return 0;
	}

	sprintf(genbuf, "ÄãÈ·ÈÏÒªÉ¾³ı [%s] Õâ¸ö ID Âğ", lookupuser.userid);
	if (askyn(genbuf, NA, NA) == NA) {
		prints("\nÈ¡ÏûÉ¾³ıÊ¹ÓÃÕß...\n");
		pressreturn();
		clear();
		return 0;
	}
	sprintf(secu, "É¾³ıÊ¹ÓÃÕß£º%s", lookupuser.userid);
	securityreport(secu, 0, 0);
	sprintf(genbuf, "mail/%c/%s", toupper(lookupuser.userid[0]),
			lookupuser.userid);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm);
	/* add end */
	sprintf(genbuf, "home/%c/%s", toupper(lookupuser.userid[0]),
			lookupuser.userid);
	//f_rm(genbuf);
	/* added by roly 02.03.24 */
	sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
	system(genbuf_rm);
	/* add end */
	lookupuser.userlevel = 0;
	strcpy(lookupuser.address, "");
#ifdef ALLOWGAME
	lookupuser.money = 0;
	lookupuser.nummedals = 0;
	lookupuser.bet = 0;
#endif
	strcpy(lookupuser.username, "");
	strcpy(lookupuser.realname, "");
	strcpy(lookupuser.termtype, "");
	prints("\n%s ÒÑ¾­±»Ãğ¾øÁË...\n", lookupuser.userid);
	lookupuser.userid[0] = '\0';
	substitut_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	setuserid(id, lookupuser.userid);
	pressreturn();
	clear();
	return 1;
}

//      ¸ü¸ÄÊ¹ÓÃÕßµÄÈ¨ÏŞ
int x_level() {
	int id;
	char reportbuf[60];
	unsigned int newlevel;

	if (!HAS_PERM(PERM_SYSOPS))
		return;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(0, 0);
	prints("¸ü¸ÄÊ¹ÓÃÕßÈ¨ÏŞ\n");
	clrtoeol();
	move(1, 0);
	usercomplete("ÊäÈëÓû¸ü¸ÄµÄÊ¹ÓÃÕßÕÊºÅ: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("Éè¶¨Ê¹ÓÃÕß '%s' µÄÈ¨ÏŞ \n", genbuf);
	newlevel
			= setperms(lookupuser.userlevel, "È¨ÏŞ", NUMPERMS, showperminfo);
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		prints("Ê¹ÓÃÕß '%s' È¨ÏŞÃ»ÓĞ±ä¸ü\n", lookupuser.userid);
	else {
		sprintf(reportbuf, "change level: %s %.8x -> %.8x",
				lookupuser.userid, lookupuser.userlevel, newlevel);
		report(reportbuf, currentuser.userid);
		lookupuser.userlevel = newlevel;
		{
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸Ä %s µÄÈ¨ÏŞ", lookupuser.userid);
			securityreport(secu, 0, 0);
		}

		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		if (!(lookupuser.userlevel & PERM_REGISTER)) {
			char src[STRLEN], dst[STRLEN];
			sethomefile(dst, lookupuser.userid, "register.old");
			if (dashf(dst))
				unlink(dst);
			sethomefile(src, lookupuser.userid, "register");
			if (dashf(src))
				rename(src, dst);
		}
		prints("Ê¹ÓÃÕß '%s' È¨ÏŞÒÑ¾­¸ü¸ÄÍê±Ï.\n", lookupuser.userid);
	}
	pressreturn();
	clear();
	return 0;
}

//added by iamfat 2002.07.22
//rewrite by iamfat 2002.08.19
extern void list_text(char *fname, void (*title_show) (), int (*key_deal) (), int (*ifcheck) ());
extern void changereason(char *fname);
extern void setreasondefault();
extern void setreason(char *rsn, int i);
extern char *getreason();
extern char *getdetailreason();
char *denylevellist = ".DenyLevel";
int add_denylevel(char *line);

char date_name[STRLEN];

//»Ö¸´ËùÓĞµ½ÆÚÈ«Õ¾´¦·£
extern int text_find(char *fname, int no, char *str, char *line);

//ÏÔÊ¾´¦·£µ½ÆÚµÄIDÁĞ±í±êÌâ.
void denylist_title_show() {
	move(0, 0);
	prints("[1;44;36m ´¦·£µ½ÆÚµÄIDÁĞ±í                                                              [m\n");
	prints(" Àë¿ª[[1;32m¡û[m] Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m] Ìí¼Ó[[1;32ma[m]  ĞŞ¸Ä[[1;32mc[m] »Ö¸´[[1;32md[m] µ½ÆÚ[[1;32mx[m] ²éÕÒ[[1;32m/[m]\n");
	prints("[1;44m ÓÃ»§´úºÅ     ´¦·£ËµÃ÷(A-Z;'.[])                 È¨ÏŞ ½áÊøÈÕÆÚ   Õ¾Îñ          [m\n");
}

// ´¦ÀíÏÔÊ¾·â½ûÁĞ±íÊ±µÄ°´¼ü´¦Àí
int denylist_key_deal(char *fname, int ch, char *line) {
	switch (ch) {
		case 'a': //Ìí¼Ó
			add_denylevel(0);
			break;
		case 'c': //ĞŞ¸Ä
			if (!line)
				break;
			add_denylevel(line);
			break;
		case 'd': //»Ö¸´
			if (!line)
				break;
			move(1, 0);
			if (askyn("ÄúÈ·¶¨Âğ?", NA, NA) == NA) {
				denylist_title_show();
				return 0;
			}
			del_from_denylist(fname, line);
			break;
		case 'x': //ÊÍ·ÅËùÓĞµ½ÆÚµÄ
			move(1, 0);
			if (askyn("ÄúÈ·¶¨Âğ?", NA, NA) == NA) {
				denylist_title_show();
				return 0;
			}
			del_from_denylist(fname, NULL);
			break;
		case Ctrl('A'):
		case KEY_RIGHT: //ÓÃ»§ĞÅÏ¢
			if (!line)
				return 0;
			t_query(line);
			break;
	}
	return 1;
}

//  ½øÈëĞŞ¸Ä·â½ûÈ¨ÏŞÁĞ±í
int x_new_denylevel() {
	if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_SPECIAL0))
		return;
	modify_user_mode(ADMIN);
	list_text(denylevellist, denylist_title_show, denylist_key_deal, NULL);
}

// ·µ»ØÈ¨ÏŞchÏà¶ÔÓ¦µÄÏûÏ¢
char *clevel(char ch) {
	switch (ch) {
		case '1':
			return "·¢ÎÄ";
		case '2':
			return "µÇÂ½";
		case '3':
			return "ÁÄÌì";
		case '4':
			return "ĞÅ¼ş";
		case '5':
			return "ÏûÏ¢";
	}
	return "Î´Öª";
}

// ·µ»ØstÏà¶ÔÓ¦µÄÕûÊıÊ±¼äÖµ.
time_t get_denydate(char *st) {
	time_t tt;
	struct tm tms;
	int ch1, ch2, ch3;
	if (!strncmp(st, "´ı¶¨", 4))
		return 0;
	if (!strncmp(st, "ÖÕÉí", 4))
		return 0x7fffffff;
	if (!strncmp(st, "ÓÀ¾Ã", 4))
		return 0x7fffffff;
	ch1 = st[4];
	st[4] = '\0'; //Äê
	ch2 = st[7];
	st[7] = '\0'; //ÔÂ
	ch3 = st[10];
	st[10] = '\0'; //ÈÕ
	tms.tm_year = atoi(st) - 1900; //      Äê
	tms.tm_mon = atoi(st + 5) - 1; //      ÔÂ
	tms.tm_mday = atoi(st + 8); //      ÈÕ
	st[4] = ch1;
	st[7] = ch2;
	st[10] = ch3;
	tms.tm_hour = tms.tm_min = tms.tm_sec = 0;
	tt = mktime(&tms); //Éú³ÉtmsÏà¶ÔÓ¦µÄÊ±¼ä
	return tt;
}

//      Ìí¼Óµ½·â½ûÁĞ±í
int

add_to_denylist(char *uident, char ch, int day, char *msg) {
	char strtosave[STRLEN];
	char line[256];
	char luid[IDLEN + 1];
	char fname[STRLEN];
	char ps[40];
	FILE *fpr, *fpw;
	int added = 0;
	int change = YEA;
	struct tm *tmtime;
	time_t tt;
	time_t tt2;

	getdata(12, 0, "ÊäÈëËµÃ÷: ", ps, 40, DOECHO, YEA);
	move(13, 0);
	if (askyn("ÄúÈ·¶¨Âğ?", NA, NA) == NA)
		return NA;
	sprintf(fname, "%s.%d", denylevellist, uinfo.pid);
	if (!(fpr = fopen(denylevellist, "r")))
		return NA;
	if (!(fpw = fopen(fname, "w")))
		return NA;
	if (day != 999) {
		day = (day > 999) ? 999 : day;
	}
	if (day == 999) {
		tt = 0x7fffffff;
	} else if (day > 0) {
		tt = time(0) + (day) * 24 * 60 * 60;
	} else {
		tt = 0;
	}
	//commented by iamfat 2002.10.08
	/*
	 while(fgets(line,256,fpr))
	 {
	 strncpy(luid,line,IDLEN);
	 luid[IDLEN]='\0';
	 strtok(luid," \r\n\t");
	 if(!strcmp(luid, uident) && !strncmp(line+48, clevel(ch),4))
	 {
	 if(tt<=get_denydate(line+53))
	 {
	 change=NA;
	 break;
	 }
	 }
	 } */
	if (change == YEA) {
		if (day == 999) {
			sprintf(strtosave, "%-12s %-34s %-4s ÖÕÉí       %-12s\n",
					uident, getreason(), clevel(ch), currentuser.userid);
			sprintf(msg, "%sÒò:\n%s\nÓ¦±»·â½û %s È¨ÏŞÖÕÉí\n\nÈç²»·ş±¾¾ö¶¨,"
				" ¿ÉÒÔÁªÏµ´¦·£¾ö¶¨ÈË»òÔÚ7ÈÕÄÚµ½AppealÉêÇë¸´Òé¡£\n"
				"P.S.: %s\n\nÖ´ĞĞÈË: %s\n", uident, getdetailreason(),
					clevel(ch), ps, currentuser.userid);
		} else if (day > 0) {
			tmtime = localtime(&tt);
			sprintf(strtosave, "%-12s %-34s %-4s %04d.%02d.%02d %-12s\n",
					uident, getreason(), clevel(ch), 1900
							+ tmtime->tm_year, tmtime->tm_mon + 1,
					tmtime->tm_mday, currentuser.userid);
			sprintf(msg,
					"%sÒò:\n%s\nÓ¦±»·â½û %s È¨ÏŞ%dÌì\n\nÇëÔÚ´¦·£ÆÚÂúºó(%04d.%02d.%02d),"
						" Ïò%sĞ´ĞÅÒªÇó½â³ı´¦·£.\nÈç²»·ş±¾¾ö¶¨,"
						" ¿ÉÒÔÁªÏµ´¦·£¾ö¶¨ÈË»òÔÚ7ÈÕÄÚµ½AppealÉêÇë¸´Òé¡£\n"
						"P.S.: %s\n\nÖ´ĞĞÈË: %s\n", uident,
					getdetailreason(), clevel(ch), day, 1900
							+ tmtime->tm_year, tmtime->tm_mon + 1,
					tmtime->tm_mday, currentuser.userid, ps,
					currentuser.userid);
		} else { //´ı¶¨
			sprintf(strtosave, "%-12s %-34s %-4s ´ı¶¨       %-12s\n",
					uident, getreason(), clevel(ch), currentuser.userid);
			sprintf(msg, "%sÒò:\n%s\nÔİÊ±±»·â½û %s È¨ÏŞ\n\n´¦·£¾ö¶¨ÉÔºó×ö³ö\n"
				"Èç²»·ş±¾¾ö¶¨, ¿ÉÒÔÁªÏµ´¦·£¾ö¶¨ÈË»òÔÚ7ÈÕÄÚµ½AppealÉêÇë¸´Òé¡£\n"
				"P.S.: %s\n\nÖ´ĞĞÈË: %s\n", uident, getdetailreason(),
					clevel(ch), ps, currentuser.userid);
		}
		fseek(fpr, 0, SEEK_SET);
		while (fgets(line, 256, fpr)) {
			tt2 = get_denydate(line + 53);
			strlcpy(luid, line, IDLEN);
			luid[IDLEN] = '\0';
			strtok(luid, " \r\n\t");
			if (!strcmp(luid, uident)
					&& !strncmp(line + 48, clevel(ch), 4))
				continue;
			if (!added && tt < tt2) {
				fputs(strtosave, fpw);
				added = 1;
			}
			fputs(line, fpw);
		} // while(fgets(line,256,fpr)
		if (!added)
			fputs(strtosave, fpw);
	} // if change == YEA
	fclose(fpw);
	fclose(fpr);

	if (change == YEA)
		rename(fname, denylevellist);
	else
		unlink(fname);

	return change;
}

// »Ö¸´ÓÃ»§µÄÈ¨ÏŞ
int release_user(char *line) {
	int id;
	FILE *se;
	char uident[IDLEN + 1];
	char fname[STRLEN];
	char secu[STRLEN];
	char rep[STRLEN];
	char msgbuf[256];
	strlcpy(uident, line, IDLEN);
	uident[IDLEN] = '\0';
	strtok(uident, " \r\n\t");
	if (!(id = getuser(uident)))
		return -1;
	if (!strncmp(&line[48], clevel('1'), 4)) {
		lookupuser.userlevel |= PERM_POST;
		sprintf(secu, "[Õ¾ÄÚ¹«¸æ]»Ö¸´%s·¢±íÎÄÕÂµÄÈ¨Á¦", lookupuser.userid);
		sprintf(rep, "±»%s»Ö¸´·¢±íÎÄÕÂµÄÈ¨Á¦", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else if (!strncmp(&line[48], clevel('2'), 4)) {
		lookupuser.userlevel |= PERM_LOGIN;
		sprintf(secu, "[Õ¾ÄÚ¹«¸æ]»Ö¸´%s»ù±¾ÉÏÕ¾µÄÈ¨Á¦", lookupuser.userid);
		sprintf(rep, "±»%s»Ö¸´»ù±¾ÉÏÕ¾µÄÈ¨Á¦", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else if (!strncmp(&line[48], clevel('3'), 4)) {
		lookupuser.userlevel |= PERM_TALK;
		sprintf(secu, "[Õ¾ÄÚ¹«¸æ]»Ö¸´%sÓëËûÈËÁÄÌìµÄÈ¨Á¦", lookupuser.userid);
		sprintf(rep, "±»%s»Ö¸´ÓëËûÈËÁÄÌìµÄÈ¨Á¦", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else if (!strncmp(&line[48], clevel('4'), 4)) {
		lookupuser.userlevel |= PERM_MAIL;
		sprintf(secu, "[Õ¾ÄÚ¹«¸æ]»Ö¸´%s·¢ËÍĞÅ¼şµÄÈ¨Á¦", lookupuser.userid);
		sprintf(rep, "±»%s»Ö¸´·¢ËÍĞÅ¼şµÄÈ¨Á¦", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	} else {
		lookupuser.userlevel |= PERM_POST;
		sprintf(secu, "[Õ¾ÄÚ¹«¸æ]»Ö¸´%sÎ´ÖªÈ¨Á¦", lookupuser.userid);
		sprintf(rep, "±»%s»Ö¸´Î´ÖªÈ¨Á¦", currentuser.userid);
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	}
	//·¢ĞÅ¸ø±¾ÈË
	sprintf(fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, uinfo.pid);
	sprintf(msgbuf, "Ö´ĞĞÈË:%s\n", currentuser.userid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", msgbuf);
		fclose(se);
		if (lookupuser.userid != NULL)
			mail_file(fname, lookupuser.userid, secu);
	}
	Poststring(msgbuf, "Notice", secu, 1);
	//°²È«¼ÇÂ¼
	securityreport(secu, 0, 0);
	log_DOTFILE(lookupuser.userid, rep);
	substitute_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	sprintf(msgbuf, "ÊÍ·Å %s", line);
	do_report(".Released", strtok(msgbuf, "\r\n"));
}

//      ´Ó·â½ûÁĞ±íÖĞÊÍ·Å
int del_from_denylist(char *fname, char *line) {
	FILE *fpr, *fpw;
	int deleted = NA;
	char tmpbuf[256], fnnew[STRLEN];
	char tmpdate[11];

	if (!(fpr = fopen(fname, "r")))
		return -1;
	sprintf(fnnew, "%s.%d", fname, uinfo.pid);
	if (!(fpw = fopen(fnnew, "w")))
		return -1;
	if (line) {
		while (fgets(tmpbuf, 256, fpr)) {
			if (deleted == NA && !strcmp(tmpbuf, line))
				deleted = YEA;
			else
				fputs(tmpbuf, fpw);
		}
		if (deleted == YEA)
			release_user(line);
	} else {
		time_t tt;
		time_t now = time(0);
		while (fgets(tmpbuf, 256, fpr)) {
			//pighead      A-----------------------------     ·¢ÎÄ 2002.07.24 iamfat
			//123456789012345678901234567890123456789012345678901234
			strlcpy(tmpdate, tmpbuf + 53, 10);
			tt = get_denydate(tmpdate);
			if (tt != 0 && tt <= now) {
				deleted = YEA;
				release_user(tmpbuf);
			} else {
				fputs(tmpbuf, fpw);
			}
		}
	}
	fclose(fpw);
	fclose(fpr);
	if (deleted == YEA)
		return rename(fnnew, fname);
	return 0;
}

//  ĞŞ¸Ä·â½ûÈ¨ÏŞ
int add_denylevel(char *line) {
	int id;
	char ans[7];
	char buf2[5];
	char msgbuf[4096];
	char genbuf[STRLEN];
	char secu[STRLEN];
	char rep[STRLEN];
	char deny_uid[IDLEN + 1];

	int bDeny = NA;
	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return 0;
	move(1, 0);
	if (line) {
		strlcpy(deny_uid, line, IDLEN);
		deny_uid[IDLEN] = '\0';
		strtok(deny_uid, " \n\r\t");
		setreason(line + IDLEN + 1, strlen(getreason()));
		prints("ĞŞ¸Ä%sµÄ·â½ûÊ±¼ä\n", deny_uid);
	} else {
		usercomplete("·â½ûÊ¹ÓÃÕß: ", deny_uid);
		if (*deny_uid == '\0')
			return 0;
		setreasondefault();
		prints("\n");
	}
	if (!(id = getuser(deny_uid))) {
		prints("·Ç·¨µÄÊ¹ÓÃÕßÕÊºÅ!\n");
		clrtoeol();
		egetch();
		return 0;
	}
	if (line) {
		if (!strncmp(&line[48], clevel('1'), 4))
			ans[0] = '1';
		else if (!strncmp(&line[48], clevel('2'), 4))
			ans[0] = '2';
		else if (!strncmp(&line[48], clevel('3'), 4))
			ans[0] = '3';
		else if (!strncmp(&line[48], clevel('4'), 4))
			ans[0] = '4';
		else
			ans[0] = '0';
		sprintf(secu, "[Õ¾ÄÚ¹«¸æ]ĞŞ¸Ä%sµÄ·â½ûÊ±¼ä", lookupuser.userid);
	} else {
		changereason("etc/denylevel");
		clear();
		move(2, 0);
		prints("Éè¶¨Ê¹ÓÃÕß%sµÄ»ù±¾È¨ÏŞ»ò×ÊÁÏ \n\n", deny_uid);
		prints("(1) ·â½û·¢±íÎÄÕÂÈ¨Á¦ %s\n",
				(lookupuser.userlevel & PERM_POST) ? "ON" : "OFF");
		prints("(2) È¡Ïû»ù±¾ÉÏÕ¾È¨Á¦ %s\n",
				(lookupuser.userlevel & PERM_LOGIN) ? "ON" : "OFF");
		prints("(3) ½ûÖ¹ÓëËûÈËÁÄÌì   %s\n",
				(lookupuser.userlevel & PERM_TALK) ? "ON" : "OFF");
		prints("(4) ½ûÖ¹·¢ËÍĞÅ¼ş     %s\n",
				(lookupuser.userlevel & PERM_MAIL) ? "ON" : "OFF");
		while (1) {
			move(12, 0);
			clrtobot();
			getdata(10, 0, "ÇëÊäÈëÄãµÄ´¦Àí: ", ans, 3, DOECHO, YEA);
			switch (ans[0]) {
				case '1':
					if (!(lookupuser.userlevel & PERM_POST)) {
						prints("\n%s·¢±íÎÄÕÂµÄÈ¨Á¦ÒÑ±»·â½û!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[Õ¾ÄÚ¹«¸æ]·â½û%s·¢±íÎÄÕÂµÄÈ¨Á¦", lookupuser.userid);
					sprintf(rep, "±»%s·â½û[Õ¾ÄÚ]·¢±íÎÄÕÂµÄÈ¨Á¦", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_POST;
					break;
				case '2':
					if (!(lookupuser.userlevel & PERM_LOGIN)) {
						prints("\n%s»ù±¾ÉÏÕ¾µÄÈ¨Á¦ÒÑ±»·â½û!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[Õ¾ÄÚ¹«¸æ]·â½û%s»ù±¾ÉÏÕ¾µÄÈ¨Á¦", lookupuser.userid);
					sprintf(rep, "±»%s·â½û[Õ¾ÄÚ]»ù±¾ÉÏÕ¾µÄÈ¨Á¦", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_LOGIN;
					break;
				case '3':
					if (!(lookupuser.userlevel & PERM_TALK)) {
						prints("\n%sÓëËûÈËÁÄÌìµÄÈ¨Á¦ÒÑ±»·â½û!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[Õ¾ÄÚ¹«¸æ]·â½û%sÓëËûÈËÁÄÌìµÄÈ¨Á¦", lookupuser.userid);
					sprintf(rep, "±»%s·â½û[Õ¾ÄÚ]ÓëËûÈËÁÄÌìµÄÈ¨Á¦", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_TALK;
					break;
				case '4':
					if (!(lookupuser.userlevel & PERM_MAIL)) {
						prints("\n%s·¢ËÍĞÅ¼şµÄÈ¨Á¦ÒÑ±»·â½û!\n", lookupuser.userid);
						egetch();
					}
					sprintf(secu, "[Õ¾ÄÚ¹«¸æ]·â½û%s·¢ËÍĞÅ¼şµÄÈ¨Á¦", lookupuser.userid);
					sprintf(rep, "±»%s·â½û[Õ¾ÄÚ]·¢ËÍĞÅ¼şµÄÈ¨Á¦", currentuser.userid);
					bDeny = YEA;
					lookupuser.userlevel &= ~PERM_MAIL;
					break;
			} //switch ans[]
			if (bDeny == YEA)
				break;
		} // while (1)
		// switch (ans[0]) {
		// case '1':
		//lookupuser.userlevel &= ~PERM_POST;
		//break;
		// case '2':
		//lookupuser.userlevel &= ~PERM_LOGIN;
		//break;
		//  case '3':
		//lookupuser.userlevel &= ~PERM_TALK;
		//break;
		//  case '4':
		//lookupuser.userlevel &= ~PERM_MAIL;
		//break;
		//  default:
		//break;
		//  }
	}
	getdata(11, 0, "·â½ûÌìÊı(999-ÖÕÉí, 0-´ı¶¨): ", buf2, 5, DOECHO, YEA);
	if (add_to_denylist(lookupuser.userid, ans[0], atoi(buf2), msgbuf)
			== NA) {
		return 0;
	}
	substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	sethomefile(genbuf, lookupuser.userid, "giveupBBS");
	if (dashf(genbuf))
		unlink(genbuf);
	{
		//·¢ĞÅ¸ø±¾ÈË
		FILE *se;
		char fname[STRLEN];
		sprintf(fname, "tmp/AutoPoster.%s.%05d", currentuser.userid,
				uinfo.pid);
		if ((se = fopen(fname, "w")) != NULL) {
			fprintf(se, "%s", msgbuf);
			fclose(se);
			if (lookupuser.userid != NULL)
				mail_file(fname, lookupuser.userid, secu);
		}
	}
	Poststring(msgbuf, "Notice", secu, 1);
	//°²È«¼ÍÂ¼
	securityreport(secu, 0, 0);
	log_DOTFILE(lookupuser.userid, rep);
	//if (!line)
	//  substitut_record (PASSFILE, &lookupuser, sizeof (struct userec), id);
	return 1;
}

void a_edits() {
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	static char *e_file[] = { "../Welcome", "../Welcome2", "issue",
			"logout", "../vote/notes", "hotspot", "menu.ini",
			"../.badname", "../.bad_email", "../.bad_host", "autopost",
			"junkboards", "sysops", "whatdate", "../NOLOGIN",
			"../NOREGISTER", "special.ini", "hosts", "restrictip",
			"freeip", "s_fill", "f_fill", "register", "firstlogin",
			"chatstation", "notbackupboards", "bbsnet.ini", "bbsnetip",
			"bbsnet2.ini", "bbsnetip2", NULL };
	static char *explain_file[] = { "ÌØÊâ½øÕ¾¹«²¼À¸", "½øÕ¾»­Ãæ", "½øÕ¾»¶Ó­µµ", "ÀëÕ¾»­Ãæ",
			"¹«ÓÃ±¸ÍüÂ¼", "ÏµÍ³ÈÈµã", "menu.ini", "²»¿É×¢²áµÄ ID", "²»¿ÉÈ·ÈÏÖ®E-Mail",
			"²»¿ÉÉÏÕ¾Ö®Î»Ö·", "Ã¿ÈÕ×Ô¶¯ËÍĞÅµµ", "²»ËãPOSTÊıµÄ°æ", "¹ÜÀíÕßÃûµ¥", "¼ÍÄîÈÕÇåµ¥",
			"ÔİÍ£µÇÂ½(NOLOGIN)", "ÔİÍ£×¢²á(NOREGISTER)", "¸öÈËipÀ´Ô´Éè¶¨µµ", "´©ËóipÀ´Ô´Éè¶¨µµ",
			"Ö»ÄÜµÇÂ½5idµÄipÉè¶¨µµ", "²»ÊÜ5 idÏŞÖÆµÄipÉè¶¨µµ", "×¢²á³É¹¦ĞÅ¼ş", "×¢²áÊ§°ÜĞÅ¼ş",
			"ĞÂÓÃ»§×¢²á·¶Àı", "ÓÃ»§µÚÒ»´ÎµÇÂ½¹«¸æ", "¹ú¼Ê»áÒéÌüÇåµ¥", "Çø¶ÎÉ¾³ı²»Ğè±¸·İÖ®Çåµ¥",
			"BBSNET ×ªÕ¾Çåµ¥", "´©ËóÏŞÖÆip", "BBSNET2 ×ªÕ¾Çåµ¥", "´©Ëó2ÏŞÖÆIP", NULL };
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(1, 0);
	prints("±àĞŞÏµÍ³µµ°¸\n\n");
	for (num = 0; (HAS_PERM(PERM_ESYSFILE)) ? e_file[num] != NULL
			&& explain_file[num] != NULL : explain_file[num] != "menu.ini"; num++) {
		prints("[[1;32m%2d[m] %s", num + 1, explain_file[num]);
		if (num < 17)
			move(4 + num, 0);
		else
			move(num - 14, 50);
	}
	prints("[[1;32m%2d[m] ¶¼²»Ïë¸Ä\n", num + 1);

	getdata(23, 0, "ÄãÒª±àĞŞÄÄÒ»ÏîÏµÍ³µµ°¸: ", ans, 3, DOECHO, YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n'
			|| ans[0] == '\0')
		return;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		sprintf(buf, "ÄãÈ·¶¨ÒªÉ¾³ı %s Õâ¸öÏµÍ³µµ", explain_file[ch]);
		confirm = askyn(buf, NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
			pressreturn();
			clear();
			return;
		}
		{
			char secu[STRLEN];
			sprintf(secu, "É¾³ıÏµÍ³µµ°¸£º%s", explain_file[ch]);
			securityreport(secu, 0, 0);
		}
		unlink(buf2);
		move(5, 0);
		prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
		pressreturn();
		clear();
		return;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA); /* ²»Ìí¼ÓÎÄ¼şÍ·, ÔÊĞíĞŞ¸ÄÍ·²¿ĞÅÏ¢ */
	clear();
	if (aborted != -1) {
		prints("%s ¸üĞÂ¹ı", explain_file[ch]);
		{
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸ÄÏµÍ³µµ°¸£º%s", explain_file[ch]);
			securityreport(secu, 0, 0);
		}

		if (!strcmp(e_file[ch], "../Welcome")) {
			unlink("Welcome.rec");
			prints("\nWelcome ¼ÇÂ¼µµ¸üĞÂ");
		} else if (!strcmp(e_file[ch], "whatdate")) {
			brdshm->fresh_date = time(0);
			prints("\n¼ÍÄîÈÕÇåµ¥ ¸üĞÂ");
		}
	}
	pressreturn();
}

// È«Õ¾¹ã²¥...
int wall() {
	char passbuf[PASSLEN];

	if (!HAS_PERM(PERM_SYSOPS))
		return 0;
	// Added by Ashinmarch on 2008.10.20
	// È«Õ¾¹ã²¥Ç°Ôö¼ÓÃÜÂëÑéÖ¤
	clear();
	stand_title("È«Õ¾¹ã²¥!");
	getdata(1, 0, "[1;37mÇëÊäÈëÃÜÂë: [m", passbuf, PASSLEN, NOECHO, YEA);
	passbuf[8] = '\0';
	if (!checkpasswd(currentuser.passwd, passbuf)) {
		prints("[1;31mÃÜÂëÊäÈë´íÎó...[m\n");
		return 0;
	}
	// Add end.

	modify_user_mode(MSG);
	move(2, 0);
	clrtobot();
	if (!get_msg("ËùÓĞÊ¹ÓÃÕß", buf2, 1)) {
		return 0;
	}
	if (apply_ulist(dowall) == -1) {
		move(2, 0);
		prints("ÏßÉÏ¿ÕÎŞÒ»ÈË\n");
		pressanykey();
	}
	prints("\nÒÑ¾­¹ã²¥Íê±Ï...\n");
	pressanykey();
	return 1;
}

// Éè¶¨ÏµÍ³ÃÜÂë
int setsystempasswd() {
	FILE *pass;
	char passbuf[20], prepass[20];
	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return;
	if (strcmp(currentuser.userid, "SYSOP")) {
		clear();
		move(10, 20);
		prints("¶Ô²»Æğ£¬ÏµÍ³ÃÜÂëÖ»ÄÜÓÉ SYSOP ĞŞ¸Ä£¡");
		pressanykey();
		return;
	}
	getdata(2, 0, "ÇëÊäÈëĞÂµÄÏµÍ³ÃÜÂë(Ö±½Ó»Ø³µÔòÈ¡ÏûÏµÍ³ÃÜÂë): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		if (askyn("ÄãÈ·¶¨ÒªÈ¡ÏûÏµÍ³ÃÜÂëÂğ?", NA, NA) == YEA) {
			unlink("etc/.syspasswd");
			securityreport("È¡ÏûÏµÍ³ÃÜÂë", 0, 0);
		}
		return;
	}
	getdata(3, 0, "È·ÈÏĞÂµÄÏµÍ³ÃÜÂë: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		move(4, 0);
		prints("Á½´ÎÃÜÂë²»ÏàÍ¬, È¡Ïû´Ë´ÎÉè¶¨.");
		pressanykey();
		return;
	}
	if ((pass = fopen("etc/.syspasswd", "w")) == NULL) {
		move(4, 0);
		prints("ÏµÍ³ÃÜÂëÎŞ·¨Éè¶¨....");
		pressanykey();
		return;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(4, 0);
	prints("ÏµÍ³ÃÜÂëÉè¶¨Íê³É....");
	pressanykey();
	return;
}

//      ½øÈëc shell»·¾³
int x_csh() {
	char buf[PASSLEN];
	int save_pager;
	int magic;

	if (!HAS_PERM(PERM_SYSOPS)) {
		return 0;
	}
	if (!check_systempasswd()) {
		return;
	}
	modify_user_mode(SYSINFO);
	clear();
	getdata(1, 0, "ÇëÊäÈëÍ¨ĞĞ°µºÅ: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !checkpasswd(currentuser.passwd, buf)) {
		prints("\n\n°µºÅ²»ÕıÈ·, ²»ÄÜÖ´ĞĞ¡£\n");
		pressreturn();
		clear();
		return;
	}
	randomize();
	magic = rand() % 1000;
	prints("\nMagic Key: %d", magic * 5 - 2);
	getdata(4, 0, "Your Key : ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !(atoi(buf) == magic)) {
		securityreport("Fail to shell out", 0, 0);
		prints("\n\nKey ²»ÕıÈ·, ²»ÄÜÖ´ĞĞ¡£\n");
		pressreturn();
		clear();
		return;
	}
	securityreport("Shell out", 0, 0);
	modify_user_mode(SYSINFO);
	clear();
	refresh();
	reset_tty();
	save_pager = uinfo.pager;
	uinfo.pager = 0;
	update_utmp();
	do_exec("csh", NULL);
	restore_tty();
	uinfo.pager = save_pager;
	update_utmp();
	clear();
	return 0;
}
#endif
#endif
