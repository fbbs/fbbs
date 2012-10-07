#include <stdio.h>
#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/msg.h"
#include "fbbs/register.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

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
static int getbnames(const char *userid, const char *bname, int *find)
{
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

static int get_grp(char *seekstr)
{
	FILE   *fp;
	char    buf[STRLEN];
	char   *namep;
	if ((fp = fopen("0Announce/.Search", "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			strtok(NULL, "/");
			namep = strtok(NULL, "/");
			if (strlen(namep) < 30) {
				strcpy(lookgrp, namep);
				return 1;
			} else
				return 0;
		}
	}
	fclose(fp);
	return 0;
}

//      ĞŞ¸ÄÊ¹ÓÃÕß×ÊÁÏ
int m_info() {
	struct userec user;
	char reportbuf[30];
	int id;

	if (!(HAS_PERM(PERM_USER)))
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}
	clear();
	stand_title("ĞŞ¸ÄÊ¹ÓÃÕß×ÊÁÏ");
	if (!gettheuserid(1, "ÇëÊäÈëÊ¹ÓÃÕß´úºÅ: ", &id))
		return -1;
	memcpy(&user, &lookupuser, sizeof(user));
	sprintf(reportbuf, "check info: %s", user.userid);
	report(reportbuf, currentuser.userid);

	move(1, 0);
	clrtobot();
	disply_userinfo(&user);
	uinfo_query(&user, 1, id);
	return 0;
}

static const char *ordain_bm_check(const board_t *board, const char *uname)
{
	if (strneq(board->bms, "SYSOPs", 6))
		return "ÌÖÂÛÇøµÄ°æÖ÷ÊÇ SYSOPs Äã²»ÄÜÔÙÈÎÃü°æÖ÷";
	if (strlen(uname) + strlen(board->bms) > BMNAMEMAXLEN)
		return "ÌÖÂÛÇø°æÖ÷ÁĞ±íÌ«³¤,ÎŞ·¨¼ÓÈë!";
	if (streq(uname, "guest"))
		return "Äã²»ÄÜÈÎÃü guest µ±°æÖ÷";

	int find;
	int bms = getbnames(lookupuser.userid, board->name, &find);
	if (find || bms >= 3)
		return "ÒÑ¾­ÊÇ¸Ã/Èı¸ö°æµÄ°æÖ÷ÁË";

	bms = 1;
	for (const char *s = board->bms; *s; ++s) {
		if (*s == ' ')
			++bms;
	}
	if (bms >= BMMAXNUM)
		return "ÌÖÂÛÇøÒÑÓĞ 5 Ãû°æÖ÷";

	return NULL;
}

static bool ordain_bm(int bid, const char *uname)
{
	user_id_t uid = get_user_id(uname);
	if (uid <= 0)
		return false;

	db_res_t *res = db_cmd("INSERT INTO bms (user_id, board_id, stamp) "
			"VALUES (%d, %d, current_timestamp) ", uid, bid);
	db_clear(res);
	return res;
}

int tui_ordain_bm(const char *cmd)
{
	if (!(HAS_PERM(PERM_USER)))
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd())
		return 0;

	clear();
	stand_title("ÈÎÃü°æÖ÷\n");
	clrtoeol();

	int id;
	if (!gettheuserid(2, "ÊäÈëÓûÈÎÃüµÄÊ¹ÓÃÕßÕÊºÅ: ", &id))
		return 0;

	char bname[BOARD_NAME_LEN];
	board_t board;
	board_complete(3, "ÊäÈë¸ÃÊ¹ÓÃÕß½«¹ÜÀíµÄÌÖÂÛÇøÃû³Æ: ", bname, sizeof(bname),
			AC_LIST_BOARDS_ONLY);
	if (!*bname || !get_board(bname, &board))
		return -1;
	board_to_gbk(&board);

	move(4, 0);
	clrtobot();

	const char *error = ordain_bm_check(&board, lookupuser.userid);
	if (error) {
		move(5, 0);
		outs(error);
		pressanykey();
		clear();
		return -1;
	}

	bool bm1 = !board.bms[0];
	const char *bm_s = bm1 ? "Ö÷" : "¸±";
	prints("\nÄã½«ÈÎÃü %s Îª %s °æ°æ%s.\n", lookupuser.userid, bname, bm_s);
	if (askyn("ÄãÈ·¶¨ÒªÈÎÃüÂğ?", NA, NA) == NA) {
		prints("È¡ÏûÈÎÃü°æÖ÷");
		pressanykey();
		clear();
		return -1;
	}

	if (!ordain_bm(board.id, lookupuser.userid)) {
		prints("Error");
		pressanykey();
		clear();
		return -1;
	}

	if (!HAS_PERM2(PERM_BOARDS, &lookupuser)) {
		lookupuser.userlevel |= PERM_BOARDS;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);

		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "°æÖ÷ÈÎÃü, ¸øÓè %s °æÖ÷È¨ÏŞ",
				lookupuser.userid);
		securityreport(buf, 0, 1);
		move(15, 0);
		outs(buf);
		pressanykey();
		clear();
	}

	char old_descr[STRLEN];
	snprintf(old_descr, sizeof(old_descr), "¡ğ %s", board.descr);

	//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
	//¾«»ªÇøµÄÏÔÊ¾: ¶¯Ì¬·ÖÅä        ÏÔÊ¾10¸ö¿Õ¸ñ printf("%*c",10,' ');
	{
		int blanklen; //Ç°Á½¸ö¿Õ¼ä´óĞ¡
		static const char BLANK = ' ';
		blanklen = STRLEN - strlen(old_descr) - strlen(board.bms) - 7;
		blanklen /= 2;
		blanklen = (blanklen > 0) ? blanklen : 1;
		sprintf(genbuf, "%s%*c(BM: %s)",
				old_descr, blanklen, BLANK, board.bms);
	}

	get_grp(board.name);
	edit_grp(board.name, lookgrp, old_descr, genbuf);

	char file[HOMELEN];
	sethomefile(file, lookupuser.userid, ".bmfile");
	FILE *fp = fopen(file, "a");
	if (fp) {
		fprintf(fp, "%s\n", lookupuser.userid);
		fclose(fp);
	}

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
			" 		 [1;33m¦î	[37mÈÎÃü  %s  Îª  %s  °æ°æ%s¡£							   [m\n"
			" 		 [1;33mÍ¨																  [m\n"
			" 		[1m	»¶Ó­  %s  Ç°Íù BM_Home °æºÍ±¾Çø Zone °æÏò´ó¼ÒÎÊºÃ¡£			 [m\n"
			" 		 [1;33m¸æ																  [m\n"
			" 		 [1;33m¦ï	[37m¿ªÊ¼¹¤×÷Ç°£¬ÇëÏÈÍ¨¶ÁBM_Home°æ¾«»ªÇøµÄ°æÖ÷Ö¸ÄÏÄ¿Â¼¡£		   [m\n"
			" 																		 [33m©¦  [m\n"
			" 											 [1;33m¨X¨T¨[¨X¨T¨[¨X¨T¨[¨X¨T¨[   [0;33m ©¦  [m\n"
			" 	 [31m¡ó¡ô©¤[1;35m¡¼Î¬»¤°æÃæÖÈĞò¡¤½¨ÉèºÍĞ³¹â»ª¡½[0;31m©¤©¤[1;33m¨U[31m°æ[33m¨U¨U[31mÖ÷[33m¨U¨U[31mÎ¯[33m¨U¨U[31mÈÎ[33m¨U[0;33m©¤©¤©ï	[m\n"
			" 											 [1;33m¨^¨T¨a¨^¨T¨a¨^¨T¨a¨^¨T¨a		  [m\n"
			" 																			 [m\n", lookupuser.userid, bname,
			bm_s, lookupuser.userid);
	//add end

	char ps[5][STRLEN];
	move(8, 0);
	prints("ÇëÊäÈëÈÎÃü¸½ÑÔ(×î¶àÎåĞĞ£¬°´ Enter ½áÊø)");
	for (int i = 0; i < 5; i++) {
		getdata(i + 9, 0, ": ", ps[i], STRLEN - 5, DOECHO, YEA);
		if (ps[i][0] == '\0')
			break;
	}
	for (int i = 0; i < 5; i++) {
		if (ps[i][0] == '\0')
			break;
		if (i == 0)
			strcat(genbuf, "\n\n");
		strcat(genbuf, "\t");
		strcat(genbuf, ps[i]);
		strcat(genbuf, "\n");
	}

	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "ÈÎÃü %s Îª %s °æ°æ%s", lookupuser.userid,
			board.name, bm_s);
	autoreport(board.name, buf, genbuf, lookupuser.userid, POST_FILE_BMS);
#ifdef ORDAINBM_POST_BOARDNAME
	autoreport(ORDAINBM_POST_BOARDNAME, buf, genbuf, lookupuser.userid,
			POST_FILE_BMS);
#endif
	securityreport(buf, 0, 1);
	move(16, 0);
	outs(buf);
	pressanykey();
	return 0;
}

static bool retire_bm(int bid, const char *uname)
{
	db_res_t *res = db_cmd("DELETE FROM bms b USING users u "
			"WHERE b.user_id = u.id AND b.board_id = %d AND u.name = %s",
			bid, uname);
	db_clear(res);
	return res;
}

int tui_retire_bm(const char *cmd)
{
	int id, right = 0, j = 0, bmnum;
	int find, bm = 1;
	FILE *bmfp;
	char bmfilename[STRLEN], usernames[BMMAXNUM][STRLEN];

	if (!(HAS_PERM(PERM_USER)))
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd())
		return 0;

	clear();
	stand_title("°æÖ÷ÀëÖ°\n");
	clrtoeol();
	if (!gettheuserid(2, "ÊäÈëÓûÀëÖ°µÄ°æÖ÷ÕÊºÅ: ", &id))
		return -1;

	char bname[BOARD_NAME_LEN];
	board_t board;
	board_complete(3, "ÇëÊäÈë¸Ã°æÖ÷Òª´ÇÈ¥µÄ°æÃû: ", bname, sizeof(bname),
			AC_LIST_BOARDS_ONLY);
	if (!*bname || !get_board(bname, &board))
		return -1;
	board_to_gbk(&board);

	int oldbm = getbnames(lookupuser.userid, bname, &find);
	if (!oldbm || !find) {
		move(5, 0);
		prints(" %s %s°æ°æÖ÷£¬ÈçÓĞ´íÎó£¬ÇëÍ¨Öª³ÌĞòÕ¾³¤¡£", lookupuser.userid,
				(oldbm) ? "²»ÊÇ¸Ã" : "Ã»ÓĞµ£ÈÎÈÎºÎ");
		pressanykey();
		clear();
		return -1;
	}
	for (int i = find - 1; i < oldbm; i++) {
		if (i != oldbm - 1)
			strcpy(bnames[i], bnames[i + 1]);
	}
	bmnum = 0;
	for (int i = 0; board.bms[i] != '\0'; i++) {
		if (board.bms[i] == ' ') {
			usernames[bmnum][j] = '\0';
			bmnum++;
			j = 0;
		} else {
			usernames[bmnum][j++] = board.bms[i];
		}
	}
	usernames[bmnum++][j] = '\0';
	for (int i = 0; i < bmnum; i++) {
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

	retire_bm(board.id, lookupuser.userid);

	char old_descr[STRLEN];
	snprintf(old_descr, sizeof(old_descr), "¡ğ %s", board.descr);

	if (!streq(board.bms, lookupuser.userid)) {
		//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
		//¾«»ªÇøµÄÏÔÊ¾: ¶¯Ì¬·ÖÅä        ÏÔÊ¾10¸ö¿Õ¸ñ printf("%*c",10,' ');
		{
			int blanklen; //Ç°Á½¸ö¿Õ¼ä´óĞ¡
			static const char BLANK = ' ';
			blanklen = STRLEN - strlen(old_descr) - strlen(board.bms) - 7;
			blanklen /= 2;
			blanklen = (blanklen > 0) ? blanklen : 1;
			sprintf(genbuf, "%s%*c(BM: %s)", old_descr, blanklen,
					BLANK, board.bms);
		}
	} else {
		sprintf(genbuf, "%-38.38s", old_descr);
	}
	get_grp(board.name);
	edit_grp(board.name, lookgrp, old_descr, genbuf);
	sprintf(genbuf, "È¡Ïû %s µÄ %s °æ°æÖ÷Ö°Îñ", lookupuser.userid, board.name);
	securityreport(genbuf, 0, 1);
	move(8, 0);
	outs(genbuf);
	sethomefile(bmfilename, lookupuser.userid, ".bmfile");
	if (oldbm - 1) {
		bmfp = fopen(bmfilename, "w+");
		for (int i = 0; i < oldbm - 1; i++)
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
			outs(secu);
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

	char buf[5][STRLEN];
	for (int i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(14, 0);
	prints("ÇëÊäÈë%s¸½ÑÔ(×î¶àÎåĞĞ£¬°´ Enter ½áÊø)", right ? "°æÖ÷ÀëÈÎ" : "°æÖ÷³·Ö°");
	for (int i = 0; i < 5; i++) {
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
	autoreport(board.name, bmfilename, genbuf, lookupuser.userid, POST_FILE_BMS);

	prints("\nÖ´ĞĞÍê±Ï£¡");
	pressanykey();
	return 0;
}

static bool valid_board_name(const char *name)
{
	for (const char *s = name; *s; ++s) {
		char ch = *s;
		if (!isalnum(ch) && ch != '_' && ch != '.' && ch != '~')
			return false;
	}
	return true;
}

static int select_section(void)
{
	int id = 0;
	char buf[3];
	getdata(5, 0, "ÇëÊäÈë·ÖÇø: ", buf, sizeof(buf), DOECHO, YEA);
	if (*buf) {
		db_res_t *res = db_query("SELECT id FROM board_sectors "
				"WHERE lower(name) = lower(%s)", buf);
		if (res && db_res_rows(res) == 1)
			id = db_get_integer(res, 0, 0);
		db_clear(res);
	}
	return id;
}

const char *chgrp(void)
{
	const char *explain[] = {
		"BBS ÏµÍ³", "¸´µ©´óÑ§", "ÔºÏµ·ç²É", "µçÄÔ¼¼Êõ", "ĞİÏĞÓéÀÖ", "ÎÄÑ§ÒÕÊõ",
		"ÌåÓı½¡Éí", "¸ĞĞÔ¿Õ¼ä", "ĞÂÎÅĞÅÏ¢", "Ñ§¿ÆÑ§Êõ", "ÒôÀÖÓ°ÊÓ", "½»Ò××¨Çø",
		"Òş²Ø·ÖÇø", NULL
	};
	const char *groups[] = {
        "system.faq", "campus.faq", "ccu.faq", "comp.faq", "rec.faq",
		"literal.faq", "sport.faq", "talk.faq", "news.faq", "sci.faq",
		"other.faq", "business.faq", "hide.faq", NULL
	};

	clear();
	move(2, 0);
	prints("Ñ¡Ôñ¾«»ªÇøµÄÄ¿Â¼\n\n");

	int i, ch;
	for (i = 0; ; ++i) {
		if (!explain[i] || !groups[i])
			break;
		prints("\033[1;32m%2d\033[m. %-20s%-20s\n", i, explain[i], groups[i]);
	}

	char buf[STRLEN], ans[6];
	snprintf(buf, sizeof(buf), "ÇëÊäÈëÄúµÄÑ¡Ôñ(0~%d): ", --i);
	while (1) {
		getdata(i + 6, 0, buf, ans, sizeof(ans), DOECHO, YEA);
		if (!isdigit(ans[0]))
			continue;
		ch = atoi(ans);
		if (ch < 0 || ch > i || ans[0] == '\r' || ans[0] == '\0')
			continue;
		else
			break;
	}
	snprintf(cexplain, sizeof(cexplain), "%s", explain[ch]);

	return groups[ch];
}

static int insert_categ(const char *categ)
{
	int id;
	db_res_t *res = db_query("SELECT id FROM board_categs "
			"WHERE name = %s", categ);
	if (res && db_res_rows(res) == 1) {
		id = db_get_integer(res, 0, 0);
		db_clear(res);
		return id;
	}

	res = db_query("INSERT INTO board_categs (name) "
			"VALUES (%s) RETURNING id", categ);
	if (res && db_res_rows(res) == 1) {
		id = db_get_integer(res, 0, 0);
		db_clear(res);
		return id;
	}
	return 0;
}

static int set_board_name(char *bname, size_t size)
{
	while (1) {
		getdata(2, 0, "ÌÖÂÛÇøÃû³Æ:   ", bname, sizeof(bname), DOECHO, YEA);
		if (*bname) {
			board_t board;
			if (get_board(bname, &board)) {
				prints("\n´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ!!");
				pressanykey();
				return -1;
			}
		} else {
			return -1;
		}

		if (valid_board_name(bname))
			break;
		prints("\n²»ºÏ·¨Ãû³Æ!!");
		return -1;
	}
	return 0;
}

int tui_new_board(const char *cmd)
{
	if (!(HAS_PERM(PERM_BLEVELS)))
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}

	clear();
	stand_title("¿ªÆôĞÂÌÖÂÛÇø");

	char bname[BOARD_NAME_LEN + 1];
	if (set_board_name(bname, sizeof(bname)) != 0)
		return -1;

	GBK_UTF8_BUFFER(descr, BOARD_DESCR_CCHARS);
	getdata(3, 0, "ÌÖÂÛÇøËµÃ÷: ", gbk_descr, sizeof(gbk_descr), DOECHO, YEA);
	if (!*gbk_descr)
		return -1;
	convert_g2u(gbk_descr, utf8_descr);

	GBK_UTF8_BUFFER(categ, BOARD_CATEG_CCHARS);
	getdata(4, 0, "ÌÖÂÛÇøÀà±ğ: ", gbk_categ, sizeof(gbk_categ), DOECHO, YEA);
	convert_g2u(gbk_categ, utf8_categ);
	int categ = insert_categ(utf8_categ);
	
	int sector = select_section();

	char pname[BOARD_NAME_LEN];
	board_complete(6, "ÊäÈëËùÊôÄ¿Â¼: ", pname, sizeof(pname),
			AC_LIST_DIR_ONLY);
	board_t parent;
	get_board(pname, &parent);

	int flag = 0, perm = 0;
	move(7, 0);
	clrtobot();
	if (askyn("±¾°æÊÇÄ¿Â¼Âğ?", NA, NA)) {
		flag |= (BOARD_DIR_FLAG | BOARD_JUNK_FLAG
				| BOARD_NOREPLY_FLAG | BOARD_POST_FLAG);
		if (askyn("ÊÇ·ñÏŞÖÆ´æÈ¡È¨Àû?", NA, NA)) {
			char ans[2];
			getdata(8, 0, "ÏŞÖÆ¶Á? [R]: ", ans, sizeof(ans), DOECHO, YEA);
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("Éè¶¨ %s È¨Àû. ÌÖÂÛÇø: '%s'\n", "READ", bname);
			perm = setperms(perm, "È¨ÏŞ", NUMPERMS, showperminfo);
			clear();
		}
	} else {
		if (askyn("¸Ã°æµÄÈ«²¿ÎÄÕÂ¾ù²»¿ÉÒÔ»Ø¸´", NA, NA))
			flag |= BOARD_NOREPLY_FLAG;
		if (askyn("ÊÇ·ñÊÇ¾ãÀÖ²¿°æÃæ", NA, NA)) {
			flag |= BOARD_CLUB_FLAG;
			if (askyn("ÊÇ·ñ¶ÁÏŞÖÆ¾ãÀÖ²¿°æÃæ", NA, NA))
				flag |= BOARD_READ_FLAG;
		}
		if (askyn("ÊÇ·ñ²»¼ÆËãÎÄÕÂÊı", NA, NA))
			flag |= BOARD_JUNK_FLAG;
		if (askyn("ÊÇ·ñÎªÄäÃû°æ", NA, NA))
			flag |= BOARD_ANONY_FLAG;
#ifdef ENABLE_PREFIX
		if (askyn ("ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º", NA, NA))
			flag |= BOARD_PREFIX_FLAG;
#endif
		if (askyn("ÊÇ·ñÏŞÖÆ¶ÁĞ´", NA, NA)) {
			char ans[2];
			getdata(15, 0, "ÏŞÖÆ¶Á(R)/Ğ´(P)? [R]: ", ans, sizeof(ans),
					DOECHO, YEA);
			if (*ans == 'P' || *ans == 'p')
				flag |= BOARD_POST_FLAG;
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("Éè¶¨ %s ÏŞÖÆ. ÌÖÂÛÇø: '%s'\n",
					(flag & BOARD_POST_FLAG ? "Ğ´" : "¶Á"), bname);
			perm = setperms(perm, "È¨ÏŞ", NUMPERMS, showperminfo);
			clear();
		}
	}

	db_res_t *res = db_query("INSERT INTO boards "
			"(name, descr, parent, flag, perm, categ, sector) "
			"VALUES (%s, %s, %d, %d, %d, %d, %d) RETURNING id",
			bname, utf8_descr, parent.id, flag, perm, categ, sector);
	if (!res) {
		prints("\n½¨Á¢ĞÂ°æ³ö´í\n");
		pressanykey();
		clear();
		return -1;
	}
	int bid = db_get_integer(res, 0, 0);
	db_clear(res);

	char *bms = NULL;
	if (!(flag & BOARD_DIR_FLAG)
			&& !askyn("±¾°æ³ÏÕ÷°æÖ÷Âğ(·ñÔòÓÉSYSOPs¹ÜÀí)?", YEA, NA)) {
		bms = "SYSOPs";
		ordain_bm(bid, bms);
	}

	char vdir[HOMELEN];
	snprintf(vdir, sizeof(vdir), "vote/%s", bname);
	char bdir[HOMELEN];
	snprintf(bdir, sizeof(bdir), "boards/%s", bname);
	if (mkdir(bdir, 0755) != 0 || mkdir(vdir, 0755) != 0) {
		prints("\nĞÂ½¨Ä¿Â¼³ö´í!\n");
		pressreturn();
		clear();
		return -1;
	}

	if (!(flag & BOARD_DIR_FLAG)) {
		const char *group = chgrp();
		if (group) {
			char buf[STRLEN];
			if (bms) {
				snprintf(buf, sizeof(buf), "¡ğ %-35.35s(BM: %s)",
						gbk_descr, bms);
			} else {
				snprintf(buf, sizeof(buf), "¡ğ %-35.35s", gbk_descr);
			}
			if (add_grp(group, cexplain, bname, buf) == -1) {
				prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
			} else {
				prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
			}
		}
	}

	rebuild_brdshm(); //add by cometcaptor 2006-10-13
	prints("\nĞÂÌÖÂÛÇø³ÉÁ¢\n");

	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "³ÉÁ¢ĞÂ°æ£º%s", bname);
	securityreport(buf, 0, 1);

	clear();
	return 0;
}

static void show_edit_board_menu(board_t *bp, board_t *pp)
{
	prints("1)ĞŞ¸ÄÃû³Æ:        %s\n", bp->name);
	prints("2)ĞŞ¸ÄËµÃ÷:        %s\n", bp->descr);
	prints("4)ĞŞ¸ÄËùÊôÄ¿Â¼:    %s(%d)\n", pp->name, pp->id);
	if (bp->flag & BOARD_DIR_FLAG) {
		prints("5)ĞŞ¸Ä¶ÁĞ´ÊôĞÔ:    %s\n",
				(bp->perm == 0) ? "Ã»ÓĞÏŞÖÆ" : "r(ÏŞÖÆÔÄ¶Á)");
	} else {
		prints("5)ĞŞ¸Ä¶ÁĞ´ÊôĞÔ:    %s\n",
				(bp->flag & BOARD_POST_FLAG) ? "p(ÏŞÖÆ·¢ÎÄ)"
				: (bp->perm == 0) ? "Ã»ÓĞÏŞÖÆ" : "r(ÏŞÖÆÔÄ¶Á)");
	}

	if (!(bp->flag & BOARD_DIR_FLAG)) {
		prints("8)ÄäÃû°æÃæ:            %s\n",
				(bp->flag & BOARD_ANONY_FLAG) ? "ÊÇ" : "·ñ");
		prints("9)¿ÉÒÔ»Ø¸´:            %s\n",
				(bp->flag & BOARD_NOREPLY_FLAG) ? "·ñ" : "ÊÇ");
		prints("A)ÊÇ·ñ¼ÆËãÎÄÕÂÊı:      %s\n",
				(bp->flag & BOARD_JUNK_FLAG) ? "·ñ" : "ÊÇ");
		prints("B)¾ãÀÖ²¿ÊôĞÔ:          %s\n",
				(bp->flag & BOARD_CLUB_FLAG) ?
				(bp->flag & BOARD_READ_FLAG) ?
				"\033[1;31mc\033[0m(¶ÁÏŞÖÆ)"
				: "\033[1;33mc\033[0m(Ğ´ÏŞÖÆ)"
				: "·Ç¾ãÀÖ²¿");
#ifdef ENABLE_PREFIX
		prints ("C)ÊÇ·ñÇ¿ÖÆÊ¹ÓÃÇ°×º:    %s\n",
				(bp->flag & BOARD_PREFIX_FLAG) ? "ÊÇ" : "·ñ");
#endif
	}
}

static bool alter_board_name(board_t *bp)
{
	char bname[BOARD_NAME_LEN + 1];
	getdata(t_lines - 2, 0, "ĞÂÌÖÂÛÇøÃû³Æ: ", bname, sizeof(bname),
			DOECHO, YEA);
	if (!*bname || streq(bp->name, bname) || !valid_board_name(bname))
		return 0;

	if (!askyn("È·¶¨ĞŞ¸Ä°æÃû?", NA, YEA))
		return 0;

	db_res_t *res = db_cmd("UPDATE boards SET name = %s WHERE id = %d",
			bname, bp->id);
	db_clear(res);
	return res;
}

static bool alter_board_descr(board_t *bp)
{
	GBK_UTF8_BUFFER(descr, BOARD_DESCR_CCHARS);
	getdata(t_lines - 2, 0, "ĞÂÌÖÂÛÇøËµÃ÷: ", gbk_descr, sizeof(gbk_descr),
			DOECHO, YEA);
	if (!*gbk_descr)
		return 0;

	convert_g2u(gbk_descr, utf8_descr);
	db_res_t *res = db_cmd("UPDATE boards SET descr = %s WHERE id = %d",
			utf8_descr, bp->id);
	db_clear(res);
	return res;
}

static bool alter_board_parent(board_t *bp)
{
	GBK_UTF8_BUFFER(bname, BOARD_NAME_LEN / 2);
	board_complete(15, "ÊäÈëËùÊôÌÖÂÛÇøÃû: ", gbk_bname, sizeof(gbk_bname),
			AC_LIST_DIR_ONLY);
	convert_g2u(gbk_bname, utf8_bname);

	board_t parent;
	get_board(utf8_bname, &parent);

	db_res_t *res = db_cmd("UPDATE boards SET parent = %d WHERE id = %d",
			parent.id, bp->id);
	db_clear(res);
	return res;
}

static bool alter_board_perm(board_t *bp)
{
	char buf[STRLEN], ans[2];
	int flag = bp->flag, perm = bp->perm;
	if (bp->flag & BOARD_DIR_FLAG) {
		snprintf(buf, sizeof(buf), "(N)²»ÏŞÖÆ (R)ÏŞÖÆÔÄ¶Á [%c]: ",
				(bp->perm) ? 'R' : 'N');
		getdata(15, 0, buf, ans, sizeof(ans), DOECHO, YEA);
		if (ans[0] == 'N' || ans[0] == 'n') {
			flag &= ~BOARD_POST_FLAG;
			perm = 0;
		} else {
			if (ans[0] == 'R' || ans[0] == 'r')
				flag &= ~BOARD_POST_FLAG;
			clear();
			move(2, 0);
			prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏŞ\n", "ÔÄ¶Á", bp->name);
			perm = setperms(perm, "È¨ÏŞ", NUMPERMS, showperminfo);
			clear();
		}
	} else {
		snprintf(buf, sizeof(buf), "(N)²»ÏŞÖÆ (R)ÏŞÖÆÔÄ¶Á (P)ÏŞÖÆÕÅÌù ÎÄÕÂ [%c]: ",
				(flag & BOARD_POST_FLAG) ? 'P' : (perm) ? 'R' : 'N');
		getdata(15, 0, buf, ans, sizeof(ans), DOECHO, YEA);
		if (ans[0] == 'N' || ans[0] == 'n') {
			flag &= ~BOARD_POST_FLAG;
			perm = 0;
		} else {
			if (ans[0] == 'R' || ans[0] == 'r')
				flag &= ~BOARD_POST_FLAG;
			else if (ans[0] == 'P' || ans[0] == 'p')
				flag |= BOARD_POST_FLAG;
			clear();
			move(2, 0);
			prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏŞ\n",
					(flag & BOARD_POST_FLAG) ? "ÕÅÌù" : "ÔÄ¶Á", bp->name);
			perm = setperms(perm, "È¨ÏŞ", NUMPERMS, showperminfo);
			clear();
		}
	}

	db_res_t *res = db_cmd("UPDATE boards SET flag = %d, perm = %d "
			"WHERE id = %d", flag, perm, bp->id);
	db_clear(res);
	return res;
}

static bool alter_board_flag(board_t *bp, const char *prompt, int flag)
{
	int f = bp->flag;
	if (askyn(prompt, (bp->flag & flag) ? YEA : NA, YEA)) {
		f |= flag;
	} else {
		f &= ~flag;
	}

	if (flag == BOARD_CLUB_FLAG && (f & BOARD_CLUB_FLAG)) {
		if (askyn("ÊÇ·ñ¶ÁÏŞÖÆ¾ãÀÖ²¿?",
					(bp->flag & BOARD_READ_FLAG) ? YEA : NA, NA)) {
			f |= BOARD_READ_FLAG;
		} else {
			f &= ~BOARD_READ_FLAG;
		}
	}

	db_res_t *res = db_cmd("UPDATE boards SET flag = %d WHERE id = %d",
			f, bp->id);
	db_clear(res);
	return res;
}

int tui_edit_board(const char *cmd)
{
	if (!(HAS_PERM(PERM_BLEVELS)))
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd())
		return 0;

	clear();
	stand_title("ĞŞ¸ÄÌÖÂÛÇøÉèÖÃ");

	char bname[BOARD_NAME_LEN + 1];
	board_complete(2, "ÊäÈëÌÖÂÛÇøÃû³Æ: ", bname, sizeof(bname),
			AC_LIST_BOARDS_AND_DIR);
	board_t board;
	if (!*bname || !get_board(bname, &board))
		return -1;
	board_to_gbk(&board);

	board_t parent = { .id = 0, .name = { '\0' } };
	if (board.parent) {
		get_board_by_bid(board.parent, &parent);
		board_to_gbk(&parent);
	}

	clear();
	stand_title("ĞŞ¸ÄÌÖÂÛÇøÉèÖÃ");
	move(2, 0);

	show_edit_board_menu(&board, &parent);

	char ans[2];
	getdata(14, 0, "¸ü¸ÄÄÄÏîÉèÖÃ[0]", ans, sizeof(ans), DOECHO, YEA);
	if (!ans[0])
		return 0;

	int res = 0;
	move(15, 0);
	switch (ans[0]) {
		case '1':
			res = alter_board_name(&board);
			break;
		case '2':
			res = alter_board_descr(&board);
			break;
		case '4':
			res = alter_board_parent(&board);
			break;
		case '5':
			res = alter_board_perm(&board);
			break;
		default:
			break;
	}

	if (!(board.flag & BOARD_DIR_FLAG)) {
		switch (ans[0]) {
			case '7':
				res = askyn("ÒÆ¶¯¾«»ªÇø", NA, YEA);
				break;
			case '8':
				res = alter_board_flag(&board, "ÊÇ·ñÄäÃû?", BOARD_ANONY_FLAG);
				break;
			case '9':
				res = alter_board_flag(&board, "½ûÖ¹»Ø¸´?", BOARD_NOREPLY_FLAG);
				break;
			case 'a':
			case 'A':
				res = alter_board_flag(&board, "²»¼ÆÎÄÕÂÊı?", BOARD_JUNK_FLAG);
				break;
			case 'b':
			case 'B':
				res = alter_board_flag(&board, "ÊÇ·ñ¾ãÀÖ²¿?", BOARD_CLUB_FLAG);
				break;
#ifdef ENABLE_PREFIX
			case 'c':
			case 'C':
				res = alter_board_flag(&board, "Ç¿ÖÆÇ°×º?", BOARD_PREFIX_FLAG);
				break;
#endif
		}
	}

	if (res) {
		board_t nb;
		get_board_by_bid(board.id, &nb);
		board_to_gbk(&board);

		if (ans[0] == '1') {
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸ÄÌÖÂÛÇø£º%s(%s)", board.name, nb.name);
			securityreport(secu, 0, 1);

			char old[HOMELEN], tar[HOMELEN];
			setbpath(old, board.name);
			setbpath(tar, nb.name);
			rename(old, tar);
			sprintf(old, "vote/%s", board.name);
			sprintf(tar, "vote/%s", nb.name);
			rename(old, tar);
		}

		char vbuf[STRLEN];
		if (*nb.bms) {
			snprintf(vbuf, sizeof(vbuf), "¡ğ %-35.35s(BM: %s)",
					nb.descr, nb.bms);
		} else {
			snprintf(vbuf, sizeof(vbuf), "¡ğ %-35.35s", nb.descr);
		}

		char old_descr[STRLEN];
		snprintf(old_descr, sizeof(old_descr), "¡ğ %s", board.descr);

		if (ans[1] == '2') {
			get_grp(board.name);
			edit_grp(board.name, lookgrp, old_descr, vbuf);
		}

		if (ans[1] == '1' || ans[1] == '7') {
			const char *group = chgrp();
			get_grp(board.name);
			char tmp_grp[STRLEN];
			strcpy(tmp_grp, lookgrp);
			if (strcmp(tmp_grp, group)) {
				char tmpbuf[160];
				sprintf(tmpbuf, "%s:", board.name);
				del_from_file("0Announce/.Search", tmpbuf);
				if (group != NULL) {
					if (add_grp(group, cexplain, nb.name, vbuf) == -1)
						prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
					else
						prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");

					char newpath[HOMELEN], oldpath[HOMELEN];
					sprintf(newpath, "0Announce/groups/%s/%s",
							group, nb.name);
					sprintf(oldpath, "0Announce/groups/%s/%s",
							tmp_grp, board.name);
					if (strcmp(oldpath, newpath) != 0 && dashd(oldpath)) {
						deltree(newpath);
						rename(oldpath, newpath);
						del_grp(tmp_grp, board.name, old_descr);
					}
				}
			}
		}
		char buf[STRLEN];
		snprintf(buf, sizeof(buf), "¸ü¸ÄÌÖÂÛÇø %s µÄ×ÊÁÏ --> %s", board.name, nb.name);
		report(buf, currentuser.userid);
	}

	pressanykey();
	clear();
	return 0;
}

// Åú×¢²áµ¥Ê±ÏÔÊ¾µÄ±êÌâ
int regtitle(void)
{
	prints("\033[1;33;44mÅú×¢²áµ¥ NEW VERSION wahahaha              "
			"                                     \033[m\n"
			" Àë¿ª[\033[1;32m¡û\033[m,\033[1;32me\033[m] "
			"Ñ¡Ôñ[\033[1;32m¡ü\033[m,\033[1;32m¡ı\033[m] "
			"ÔÄ¶Á[\033[1;32m¡ú\033[m,\033[1;32mRtn\033[m] "
			"Åú×¼[\033[1;32my\033[m] É¾³ı[\033[1;32md\033[m]\n"
			"\033[1;37;44m  ±àºÅ ÓÃ»§ID       ĞÕ  Ãû       Ïµ±ğ"
			"             ×¡Ö·             ×¢²áÊ±¼ä     \033[m\n");
	return 0;
}

//      ÔÚÅú×¢²áµ¥Ê±ÏÔÊ¾µÄ×¢²áIDÁĞ±í
char *regdoent(int num, reginfo_t* ent) {
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
	sprintf(buf, "  %4d %-12s %-12s %-16s %-16s %s", num, ent->userid,
			rname, dept, addr, getdatestring(ent->regdate, DATE_SHORT));
	return buf;
}

//      ·µ»Øuserid Óëent->useridÊÇ·ñÏàµÈ
static int filecheck(void *ent, void *userid)
{
	return !strcmp(((reginfo_t *)ent)->userid, (char *)userid);
}

// É¾³ı×¢²áµ¥ÎÄ¼şÀïµÄÒ»¸ö¼ÇÂ¼
int delete_register(int index, reginfo_t* ent, char *direct) {
	delete_record(direct, sizeof(reginfo_t), index, filecheck, ent->userid);
	return DIRCHANGED;
}

//      Í¨¹ı×¢²áµ¥
int pass_register(int index, reginfo_t* ent, char *direct) {
	int unum;
	struct userec user;
	char buf[80];
	FILE *fout;

	unum = getuser(ent->userid);
	if (!unum) {
		clear();
		prints("ÏµÍ³´íÎó! ²éÎŞ´ËÕËºÅ!\n"); //      ÔÚ»Øµµ»òÕßÄ³Ğ©Çé¿öÏÂ,ÕÒ²»µ½ÔÚ×¢²áµ¥ÎÄ¼ş
		pressanykey(); // unregisterÖĞµÄ´Ë¼ÇÂ¼,¹ÊÉ¾³ı
		delete_record(direct, sizeof(reginfo_t), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	delete_record(direct, sizeof(reginfo_t), index, filecheck, ent->userid);

	memcpy(&user, &lookupuser, sizeof(user));
#ifdef ALLOWGAME
	user.money = 1000;
#endif
	substitut_record(PASSFILE, &user, sizeof (user), unum);
	sethomefile(buf, user.userid, "register");
	if ((fout = fopen(buf, "a")) != NULL) {
		fprintf(fout, "×¢²áÊ±¼ä     : %s\n", getdatestring(ent->regdate, DATE_EN));
		fprintf(fout, "ÉêÇëÕÊºÅ     : %s\n", ent->userid);
		fprintf(fout, "ÕæÊµĞÕÃû     : %s\n", ent->realname);
		fprintf(fout, "Ñ§Ğ£Ïµ¼¶     : %s\n", ent->dept);
		fprintf(fout, "Ä¿Ç°×¡Ö·     : %s\n", ent->addr);
		fprintf(fout, "ÁªÂçµç»°     : %s\n", ent->phone);
#ifndef FDQUAN
		fprintf(fout, "µç×ÓÓÊ¼ş     : %s\n", ent->email);
#endif
		fprintf(fout, "Ğ£ ÓÑ »á     : %s\n", ent->assoc);
		fprintf(fout, "³É¹¦ÈÕÆÚ     : %s\n", getdatestring(time(NULL), DATE_EN));
		fprintf(fout, "Åú×¼ÈË       : %s\n", currentuser.userid);
		fclose(fout);
	}
	mail_file("etc/s_fill", user.userid, "¹§ìûÄú£¬ÄúÒÑ¾­Íê³É×¢²á¡£");
	sethomefile(buf, user.userid, "mailcheck");
	unlink(buf);
	sprintf(genbuf, "ÈÃ %s Í¨¹ıÉí·ÖÈ·ÈÏ.", user.userid);
	securityreport(genbuf, 0, 0);

	return DIRCHANGED;
}

//      ´¦Àí×¢²áµ¥
int do_register(int index, reginfo_t* ent, char *direct) {
	int unum;
	struct userec user;
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
		delete_record(direct, sizeof(reginfo_t), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	memcpy(&user, &lookupuser, sizeof (user));
	clear();
	move(0, 0);
	prints("[1;33;44m ÏêÏ¸×ÊÁÏ                                                                      [m\n");
	prints("[1;37;42m [.]½ÓÊÜ [+]¾Ü¾ø [d]É¾³ı [0-6]²»·ûºÏÔ­Òò                                       [m");

	//strcpy(ps, "(ÎŞ)");
	for (;;) {
		disply_userinfo(&user);
		move(14, 0);
		printdash(NULL);
		prints("   ×¢²áÊ±¼ä   : %s\n", getdatestring(ent->regdate, DATE_EN));
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
				user.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &user, sizeof (user), unum);
				//mail_file("etc/f_fill", user.userid, "ÇëÖØĞÂÌîĞ´ÄúµÄ×¢²á×ÊÁÏ");
				mail_file("etc/f_fill", user.userid, reason[rejectindex]);
			case 'd':
				user.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &user, sizeof (user), unum);
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
				}
				break;
		}
	}
	return 0;
}

struct one_key reg_comms[] = {
		{'r', do_register},
		{'y', pass_register},
		{'d', delete_register},
		{'\0', NULL}
};

void show_register() {
	FILE *fn;
	int x; //, y, wid, len;
	char uident[STRLEN];
	if (!(HAS_PERM(PERM_USER)))
		return;

	set_user_status(ST_ADMIN);
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
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return 0;
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
			i_read(ST_ADMIN, "unregistered", regtitle, regdoent,
					&reg_comms[0], sizeof(reginfo_t));
			break;
	}
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
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}
	clear();
	stand_title("É¾³ıÊ¹ÓÃÕßÕÊºÅ");
	// Added by Ashinmarch in 2008.10.20 
	// ¿³µôÕËºÅÊ±Ôö¼ÓÃÜÂëÑéÖ¤
	getdata(1, 0, "[1;37mÇëÊäÈëÃÜÂë: [m", passbuf, PASSLEN, NOECHO, YEA);
	passbuf[8] = '\0';
	if (!passwd_check(currentuser.userid, passbuf)) {
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
#ifdef ALLOWGAME
	lookupuser.money = 0;
	lookupuser.nummedals = 0;
	lookupuser.bet = 0;
#endif
	strcpy(lookupuser.username, "");
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
		return 0;

	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return 0;
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
	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(1, 0);
	prints("±àĞŞÏµÍ³µµ°¸\n\n");
	for (num = 0; (HAS_PERM(PERM_ESYSFILE)) ? e_file[num] != NULL
			&& explain_file[num] != NULL : strcmp(explain_file[num], "menu.ini"); num++) {
		prints("[\033[1;32m%2d\033[m] %s", num + 1, explain_file[num]);
		if (num < 17)
			move(4 + num, 0);
		else
			move(num - 14, 50);
	}
	prints("[\033[1;32m%2d\033[m] ¶¼²»Ïë¸Ä\n", num + 1);

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
	set_user_status(ST_EDITSFILE);
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
	if (!passwd_check(currentuser.userid, passbuf)) {
		prints("[1;31mÃÜÂëÊäÈë´íÎó...[m\n");
		return 0;
	}
	// Add end.

	set_user_status(ST_MSG);
	move(2, 0);
	clrtobot();

	char msg[MAX_MSG_SIZE + 2];
	if (!get_msg("ËùÓĞÊ¹ÓÃÕß", msg, 1)) {
		return 0;
	}

	if (!broadcast_msg(msg)) {
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
	set_user_status(ST_ADMIN);
	if (!check_systempasswd())
		return 0;
	if (strcmp(currentuser.userid, "SYSOP")) {
		clear();
		move(10, 20);
		prints("¶Ô²»Æğ£¬ÏµÍ³ÃÜÂëÖ»ÄÜÓÉ SYSOP ĞŞ¸Ä£¡");
		pressanykey();
		return 0;
	}
	getdata(2, 0, "ÇëÊäÈëĞÂµÄÏµÍ³ÃÜÂë(Ö±½Ó»Ø³µÔòÈ¡ÏûÏµÍ³ÃÜÂë): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		if (askyn("ÄãÈ·¶¨ÒªÈ¡ÏûÏµÍ³ÃÜÂëÂğ?", NA, NA) == YEA) {
			unlink("etc/.syspasswd");
			securityreport("È¡ÏûÏµÍ³ÃÜÂë", 0, 0);
		}
		return 0;
	}
	getdata(3, 0, "È·ÈÏĞÂµÄÏµÍ³ÃÜÂë: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		move(4, 0);
		prints("Á½´ÎÃÜÂë²»ÏàÍ¬, È¡Ïû´Ë´ÎÉè¶¨.");
		pressanykey();
		return 0;
	}
	if ((pass = fopen("etc/.syspasswd", "w")) == NULL) {
		move(4, 0);
		prints("ÏµÍ³ÃÜÂëÎŞ·¨Éè¶¨....");
		pressanykey();
		return 0;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(4, 0);
	prints("ÏµÍ³ÃÜÂëÉè¶¨Íê³É....");
	pressanykey();
	return 0;
}

#define DENY_LEVEL_LIST ".DenyLevel"
extern int denylist_key_deal(const char *file, int ch, const char *line);

/**
 * È«Õ¾´¦·£ÁĞ±í±êÌâ.
 */
static void denylist_title_show(void)
{
	move(0, 0);
	prints("\033[1;44;36m ´¦·£µ½ÆÚµÄIDÁĞ±í\033[K\033[m\n"
			" Àë¿ª[\033[1;32m¡û\033[m] Ñ¡Ôñ[\033[1;32m¡ü\033[m,\033[1;32m¡ı\033[m] Ìí¼Ó[\033[1;32ma\033[m]  ĞŞ¸Ä[\033[1;32mc\033[m] »Ö¸´[\033[1;32md\033[m] µ½ÆÚ[\033[1;32mx\033[m] ²éÕÒ[\033[1;32m/\033[m]\n"
			"\033[1;44m ÓÃ»§´úºÅ     ´¦·£ËµÃ÷(A-Z;'.[])                 È¨ÏŞ ½áÊøÈÕÆÚ   Õ¾Îñ          \033[m\n");
}

/**
 * È«Õ¾´¦·£ÁĞ±íÈë¿Úº¯Êı.
 * @return 0/1.
 */
int x_new_denylevel(void)
{
	if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_SPECIAL0))
		return DONOTHING;
	set_user_status(ST_ADMIN);
	list_text(DENY_LEVEL_LIST, denylist_title_show, denylist_key_deal, NULL);
	return FULLUPDATE;
}

int kick_user(void)
{
	if (!(HAS_PERM(PERM_OBOARDS)))
		return -1;
	set_user_status(ST_ADMIN);

	stand_title("ÌßÊ¹ÓÃÕßÏÂÕ¾");
	move(1, 0);

	char uname[IDLEN + 1];
	usercomplete("ÊäÈëÊ¹ÓÃÕßÕÊºÅ: ", uname);
	if (*uname == '\0') {
		clear();
		return -1;
	}

	user_id_t uid = get_user_id(uname);
	if (!uid) {
		presskeyfor("ÎŞ´ËÓÃ»§..", 3);
		clear();
		return 0;
	}

	move(1, 0);
	clrtoeol();
	char buf[STRLEN];
	snprintf(buf, sizeof(buf), "ÌßµôÊ¹ÓÃÕß : [%s].", uname);
	move(2, 0);
	if (!askyn(buf, NA, NA)) {
		presskeyfor("È¡ÏûÌßÊ¹ÓÃÕß..", 2);
		clear();
		return 0;
	}

	basic_session_info_t *res = get_sessions(uid);
	if (res && basic_session_info_count(res) > 0) {
		for (int i = 0; i < basic_session_info_count(res); ++i) {
			bbs_kill(basic_session_info_sid(res, i),
					basic_session_info_pid(res, i), SIGHUP);
		}
		presskeyfor("¸ÃÓÃ»§ÒÑ¾­±»ÌßÏÂÕ¾", 4);
	} else {
		move(3, 0);
		presskeyfor("¸ÃÓÃ»§²»ÔÚÏßÉÏ»òÎŞ·¨Ìß³öÕ¾Íâ..", 3);
	}

	basic_session_info_clear(res);
	clear();
	return 0;
}
