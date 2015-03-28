#include <signal.h>
#include <stdio.h>
#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/convert.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/msg.h"
#include "fbbs/post.h"
#include "fbbs/register.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

int showperminfo(int, int);
char cexplain[STRLEN];
char lookgrp[30];
char bnames[3][STRLEN]; //存放用户担任版主的版名,最多为三
FILE *cleanlog;

//在userid的主目录下 打开.bmfile文件,并将里面的内容与bname相比较
//              find存放从1开始返回所任版面的序号,为0表示没找到
//函数的返回值为userid担任版主的版面数
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

//      修改使用者资料
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
	screen_clear();
	//% "修改使用者资料"
	stand_title("\xd0\xde\xb8\xc4\xca\xb9\xd3\xc3\xd5\xdf\xd7\xca\xc1\xcf");
	//% "请输入使用者代号: "
	if (!gettheuserid(1, "\xc7\xeb\xca\xe4\xc8\xeb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5: ", &id))
		return -1;
	memcpy(&user, &lookupuser, sizeof(user));
	sprintf(reportbuf, "check info: %s", user.userid);
	report(reportbuf, currentuser.userid);

	move(1, 0);
	screen_clrtobot();
	disply_userinfo(&user);
	uinfo_query(&user, 1, id);
	return 0;
}

static const char *ordain_bm_check(const board_t *board, const char *uname)
{
	if (strneq(board->bms, "SYSOPs", 6))
		//% "讨论区的版主是 SYSOPs 你不能再任命版主"
		return "\xcc\xd6\xc2\xdb\xc7\xf8\xb5\xc4\xb0\xe6\xd6\xf7\xca\xc7 SYSOPs \xc4\xe3\xb2\xbb\xc4\xdc\xd4\xd9\xc8\xce\xc3\xfc\xb0\xe6\xd6\xf7";
	if (strlen(uname) + strlen(board->bms) > BMNAMEMAXLEN)
		//% "讨论区版主列表太长,无法加入!"
		return "\xcc\xd6\xc2\xdb\xc7\xf8\xb0\xe6\xd6\xf7\xc1\xd0\xb1\xed\xcc\xab\xb3\xa4,\xce\xde\xb7\xa8\xbc\xd3\xc8\xeb!";
	if (streq(uname, "guest"))
		//% "你不能任命 guest 当版主"
		return "\xc4\xe3\xb2\xbb\xc4\xdc\xc8\xce\xc3\xfc guest \xb5\xb1\xb0\xe6\xd6\xf7";

	int find;
	int bms = getbnames(lookupuser.userid, board->name, &find);
	if (find || bms >= 4)
		//% "已经是该/三个版的版主了"
		return "\xd2\xd1\xbe\xad\xca\xc7\xb8\xc3/\xc8\xfd\xb8\xf6\xb0\xe6\xb5\xc4\xb0\xe6\xd6\xf7\xc1\xcb";

	bms = 1;
	for (const char *s = board->bms; *s; ++s) {
		if (*s == ' ')
			++bms;
	}
	if (bms >= BMMAXNUM)
		//% "讨论区已有 5 名版主"
		return "\xcc\xd6\xc2\xdb\xc7\xf8\xd2\xd1\xd3\xd0 5 \xc3\xfb\xb0\xe6\xd6\xf7";

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

	screen_clear();
	//% "任命版主\n"
	stand_title("\xc8\xce\xc3\xfc\xb0\xe6\xd6\xf7\n");

	int id;
	//% "输入欲任命的使用者帐号: "
	if (!gettheuserid(2, "\xca\xe4\xc8\xeb\xd3\xfb\xc8\xce\xc3\xfc\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xd5\xca\xba\xc5: ", &id))
		return 0;

	char bname[BOARD_NAME_LEN];
	board_t board;
	//% "输入该使用者将管理的讨论区名称: "
	board_complete(3, "\xca\xe4\xc8\xeb\xb8\xc3\xca\xb9\xd3\xc3\xd5\xdf\xbd\xab\xb9\xdc\xc0\xed\xb5\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6: ", bname, sizeof(bname),
			AC_LIST_BOARDS_ONLY);
	if (!*bname || !get_board(bname, &board))
		return -1;
	board_to_gbk(&board);

	move(4, 0);
	screen_clrtobot();

	const char *error = ordain_bm_check(&board, lookupuser.userid);
	if (error) {
		move(5, 0);
		outs(error);
		pressanykey();
		screen_clear();
		return -1;
	}

	bool bm1 = !board.bms[0];
	//% const char *bm_s = bm1 ? "主" : "副";
	const char *bm_s = bm1 ? "\xd6\xf7" : "\xb8\xb1";
	//% prints("\n你将任命 %s 为 %s 版版%s.\n", lookupuser.userid, bname, bm_s);
	prints("\n\xc4\xe3\xbd\xab\xc8\xce\xc3\xfc %s \xce\xaa %s \xb0\xe6\xb0\xe6%s.\n", lookupuser.userid, bname, bm_s);
	//% if (askyn("你确定要任命吗?", NA, NA) == NA) {
	if (askyn("\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xc8\xce\xc3\xfc\xc2\xf0?", NA, NA) == NA) {
		//% prints("取消任命版主");
		prints("\xc8\xa1\xcf\xfb\xc8\xce\xc3\xfc\xb0\xe6\xd6\xf7");
		pressanykey();
		screen_clear();
		return -1;
	}

	if (!ordain_bm(board.id, lookupuser.userid)) {
		prints("Error");
		pressanykey();
		screen_clear();
		return -1;
	}

	if (!HAS_PERM2(PERM_BOARDS, &lookupuser)) {
		lookupuser.userlevel |= PERM_BOARDS;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);

		char buf[STRLEN];
		//% snprintf(buf, sizeof(buf), "版主任命, 给予 %s 版主权限",
		snprintf(buf, sizeof(buf), "\xb0\xe6\xd6\xf7\xc8\xce\xc3\xfc, \xb8\xf8\xd3\xe8 %s \xb0\xe6\xd6\xf7\xc8\xa8\xcf\xde",
				lookupuser.userid);
		securityreport(buf, 0, 1);
		move(15, 0);
		outs(buf);
		pressanykey();
		screen_clear();
	}

	char old_descr[STRLEN];
	//% snprintf(old_descr, sizeof(old_descr), "○ %s", board.descr);
	snprintf(old_descr, sizeof(old_descr), "\xa1\xf0 %s", board.descr);

	//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
	//精华区的显示: 动态分配        显示10个空格 printf("%*c",10,' ');
	{
		int blanklen; //前两个空间大小
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
		fprintf(fp, "%s\n", board.name);
		fclose(fp);
	}

	/* Modified by Amigo 2002.07.01. Add reference to BM-Guide. */
	//sprintf (genbuf, "\n\t\t\t【 通告 】\n\n"
	//	   "\t任命 %s 为 %s 版%s！\n"
	//	   "\t欢迎 %s 前往 BM_Home 版和本区 Zone 版向大家问好。\n"
	//	   "\t开始工作前，请先通读BM_Home版精华区的版主指南目录。\n",
	//	   lookupuser.userid, bname, bm ? "版主" : "版副", lookupuser.userid);

	//the new version add by Danielfree 06.11.12
	sprintf(
			genbuf,
			"\n"
			//% " 		[1;31m   ╔═╗╔═╗╔═╗╔═╗										 [m\n"
			" 		[1;31m   \xa8\x58\xa8\x54\xa8\x5b\xa8\x58\xa8\x54\xa8\x5b\xa8\x58\xa8\x54\xa8\x5b\xa8\x58\xa8\x54\xa8\x5b										 [m\n"
			//% " 	 [31m╋──[1m║[33m日[31m║║[33m月[31m║║[33m光[31m║║[33m华[31m║[0;33m──[1;36m〖领会站规精神·熟悉版主操作〗[0;33m─◇◆  [m\n"
			" 	 [31m\xa9\xef\xa9\xa4\xa9\xa4[1m\xa8\x55[33m\xc8\xd5[31m\xa8\x55\xa8\x55[33m\xd4\xc2[31m\xa8\x55\xa8\x55[33m\xb9\xe2[31m\xa8\x55\xa8\x55[33m\xbb\xaa[31m\xa8\x55[0;33m\xa9\xa4\xa9\xa4[1;36m\xa1\xbc\xc1\xec\xbb\xe1\xd5\xbe\xb9\xe6\xbe\xab\xc9\xf1\xa1\xa4\xca\xec\xcf\xa4\xb0\xe6\xd6\xf7\xb2\xd9\xd7\xf7\xa1\xbd[0;33m\xa9\xa4\xa1\xf3\xa1\xf4  [m\n"
			//% " 	 [31m│    [1m╚═╝╚═╝╚═╝╚═╝										  [m\n"
			" 	 [31m\xa9\xa6    [1m\xa8\x5e\xa8\x54\xa8\x61\xa8\x5e\xa8\x54\xa8\x61\xa8\x5e\xa8\x54\xa8\x61\xa8\x5e\xa8\x54\xa8\x61										  [m\n"
			//% " 	 [31m│																	  [m\n"
			" 	 [31m\xa9\xa6																	  [m\n"
			//% " 		 [1;33m︻	[37m任命  %s  为  %s  版版%s。							   [m\n"
			" 		 [1;33m\xa6\xee	[37m\xc8\xce\xc3\xfc  %s  \xce\xaa  %s  \xb0\xe6\xb0\xe6%s\xa1\xa3							   [m\n"
			//% " 		 [1;33m通																  [m\n"
			" 		 [1;33m\xcd\xa8																  [m\n"
			//% " 		[1m	欢迎  %s  前往 BM_Home 版和本区 Zone 版向大家问好。			 [m\n"
			" 		[1m	\xbb\xb6\xd3\xad  %s  \xc7\xb0\xcd\xf9 BM_Home \xb0\xe6\xba\xcd\xb1\xbe\xc7\xf8 Zone \xb0\xe6\xcf\xf2\xb4\xf3\xbc\xd2\xce\xca\xba\xc3\xa1\xa3			 [m\n"
			//% " 		 [1;33m告																  [m\n"
			" 		 [1;33m\xb8\xe6																  [m\n"
			//% " 		 [1;33m︼	[37m开始工作前，请先通读BM_Home版精华区的版主指南目录。		   [m\n"
			" 		 [1;33m\xa6\xef	[37m\xbf\xaa\xca\xbc\xb9\xa4\xd7\xf7\xc7\xb0\xa3\xac\xc7\xeb\xcf\xc8\xcd\xa8\xb6\xc1""BM_Home\xb0\xe6\xbe\xab\xbb\xaa\xc7\xf8\xb5\xc4\xb0\xe6\xd6\xf7\xd6\xb8\xc4\xcf\xc4\xbf\xc2\xbc\xa1\xa3		   [m\n"
			//% " 																		 [33m│  [m\n"
			" 																		 [33m\xa9\xa6  [m\n"
			//% " 											 [1;33m╔═╗╔═╗╔═╗╔═╗   [0;33m │  [m\n"
			" 											 [1;33m\xa8\x58\xa8\x54\xa8\x5b\xa8\x58\xa8\x54\xa8\x5b\xa8\x58\xa8\x54\xa8\x5b\xa8\x58\xa8\x54\xa8\x5b   [0;33m \xa9\xa6  [m\n"
			//% " 	 [31m◇◆─[1;35m〖维护版面秩序·建设和谐光华〗[0;31m──[1;33m║[31m版[33m║║[31m主[33m║║[31m委[33m║║[31m任[33m║[0;33m──╋	[m\n"
			" 	 [31m\xa1\xf3\xa1\xf4\xa9\xa4[1;35m\xa1\xbc\xce\xac\xbb\xa4\xb0\xe6\xc3\xe6\xd6\xc8\xd0\xf2\xa1\xa4\xbd\xa8\xc9\xe8\xba\xcd\xd0\xb3\xb9\xe2\xbb\xaa\xa1\xbd[0;31m\xa9\xa4\xa9\xa4[1;33m\xa8\x55[31m\xb0\xe6[33m\xa8\x55\xa8\x55[31m\xd6\xf7[33m\xa8\x55\xa8\x55[31m\xce\xaf[33m\xa8\x55\xa8\x55[31m\xc8\xce[33m\xa8\x55[0;33m\xa9\xa4\xa9\xa4\xa9\xef	[m\n"
			//% " 											 [1;33m╚═╝╚═╝╚═╝╚═╝		  [m\n"
			" 											 [1;33m\xa8\x5e\xa8\x54\xa8\x61\xa8\x5e\xa8\x54\xa8\x61\xa8\x5e\xa8\x54\xa8\x61\xa8\x5e\xa8\x54\xa8\x61		  [m\n"
			" 																			 [m\n", lookupuser.userid, bname,
			bm_s, lookupuser.userid);
	//add end

	char ps[5][STRLEN];
	move(8, 0);
	//% prints("请输入任命附言(最多五行，按 Enter 结束)");
	prints("\xc7\xeb\xca\xe4\xc8\xeb\xc8\xce\xc3\xfc\xb8\xbd\xd1\xd4(\xd7\xee\xb6\xe0\xce\xe5\xd0\xd0\xa3\xac\xb0\xb4 Enter \xbd\xe1\xca\xf8)");
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
	//% snprintf(buf, sizeof(buf), "任命 %s 为 %s 版版%s", lookupuser.userid,
	snprintf(buf, sizeof(buf), "\xc8\xce\xc3\xfc %s \xce\xaa %s \xb0\xe6\xb0\xe6%s", lookupuser.userid,
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

	screen_clear();
	//% stand_title("版主离职\n");
	stand_title("\xb0\xe6\xd6\xf7\xc0\xeb\xd6\xb0\n");
	//% if (!gettheuserid(2, "输入欲离职的版主帐号: ", &id))
	if (!gettheuserid(2, "\xca\xe4\xc8\xeb\xd3\xfb\xc0\xeb\xd6\xb0\xb5\xc4\xb0\xe6\xd6\xf7\xd5\xca\xba\xc5: ", &id))
		return -1;

	char bname[BOARD_NAME_LEN];
	board_t board;
	//% board_complete(3, "请输入该版主要辞去的版名: ", bname, sizeof(bname),
	board_complete(3, "\xc7\xeb\xca\xe4\xc8\xeb\xb8\xc3\xb0\xe6\xd6\xf7\xd2\xaa\xb4\xc7\xc8\xa5\xb5\xc4\xb0\xe6\xc3\xfb: ", bname, sizeof(bname),
			AC_LIST_BOARDS_ONLY);
	if (!*bname || !get_board(bname, &board))
		return -1;
	board_to_gbk(&board);

	int oldbm = getbnames(lookupuser.userid, bname, &find);
	if (!oldbm || !find) {
		move(5, 0);
		//% prints(" %s %s版版主，如有错误，请通知程序站长。", lookupuser.userid,
		prints(" %s %s\xb0\xe6\xb0\xe6\xd6\xf7\xa3\xac\xc8\xe7\xd3\xd0\xb4\xed\xce\xf3\xa3\xac\xc7\xeb\xcd\xa8\xd6\xaa\xb3\xcc\xd0\xf2\xd5\xbe\xb3\xa4\xa1\xa3", lookupuser.userid,
				//% (oldbm) ? "不是该" : "没有担任任何");
				(oldbm) ? "\xb2\xbb\xca\xc7\xb8\xc3" : "\xc3\xbb\xd3\xd0\xb5\xa3\xc8\xce\xc8\xce\xba\xce");
		pressanykey();
		screen_clear();
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
		if (right && i != bmnum - 1) //while(right&&i<bmnum)似乎更好一些;infotech
			strcpy(usernames[i], usernames[i + 1]);
	}
	if (!right) {
		move(5, 0);
		//% prints("对不起， %s 版版主名单中没有 %s ，如有错误，请通知技术站长。", bname,
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac %s \xb0\xe6\xb0\xe6\xd6\xf7\xc3\xfb\xb5\xa5\xd6\xd0\xc3\xbb\xd3\xd0 %s \xa3\xac\xc8\xe7\xd3\xd0\xb4\xed\xce\xf3\xa3\xac\xc7\xeb\xcd\xa8\xd6\xaa\xbc\xbc\xca\xf5\xd5\xbe\xb3\xa4\xa1\xa3", bname,
				lookupuser.userid);
		pressanykey();
		screen_clear();
		return -1;
	}
	//% prints("\n你将取消 %s 的 %s 版版%s职务.\n", lookupuser.userid, bname, bm ? "主"
	prints("\n\xc4\xe3\xbd\xab\xc8\xa1\xcf\xfb %s \xb5\xc4 %s \xb0\xe6\xb0\xe6%s\xd6\xb0\xce\xf1.\n", lookupuser.userid, bname, bm ? "\xd6\xf7"
			//% : "副");
			: "\xb8\xb1");
	//% if (askyn("你确定要取消他的该版版主职务吗?", NA, NA) == NA) {
	if (askyn("\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xc8\xa1\xcf\xfb\xcb\xfb\xb5\xc4\xb8\xc3\xb0\xe6\xb0\xe6\xd6\xf7\xd6\xb0\xce\xf1\xc2\xf0?", NA, NA) == NA) {
		//% prints("\n呵呵，你改变心意了？ %s 继续留任 %s 版版主职务！", lookupuser.userid, bname);
		prints("\n\xba\xc7\xba\xc7\xa3\xac\xc4\xe3\xb8\xc4\xb1\xe4\xd0\xc4\xd2\xe2\xc1\xcb\xa3\xbf %s \xbc\xcc\xd0\xf8\xc1\xf4\xc8\xce %s \xb0\xe6\xb0\xe6\xd6\xf7\xd6\xb0\xce\xf1\xa3\xa1", lookupuser.userid, bname);
		pressanykey();
		screen_clear();
		return -1;
	}

	retire_bm(board.id, lookupuser.userid);

	char old_descr[STRLEN];
	//% snprintf(old_descr, sizeof(old_descr), "○ %s", board.descr);
	snprintf(old_descr, sizeof(old_descr), "\xa1\xf0 %s", board.descr);

	if (!streq(board.bms, lookupuser.userid)) {
		//sprintf(genbuf, "%-38.38s(BM: %s)", fh.title +8, fh.BM);
		//精华区的显示: 动态分配        显示10个空格 printf("%*c",10,' ');
		{
			int blanklen; //前两个空间大小
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
	//% sprintf(genbuf, "取消 %s 的 %s 版版主职务", lookupuser.userid, board.name);
	sprintf(genbuf, "\xc8\xa1\xcf\xfb %s \xb5\xc4 %s \xb0\xe6\xb0\xe6\xd6\xf7\xd6\xb0\xce\xf1", lookupuser.userid, board.name);
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
		if (!(lookupuser.userlevel & PERM_OBOARDS) //没有讨论区管理权限
				&& !(lookupuser.userlevel & PERM_SYSOPS) //没有站务权限
		) {
			lookupuser.userlevel &= ~PERM_BOARDS;
			substitut_record(PASSFILE, &lookupuser, sizeof(struct userec),
					id);
			//% sprintf(secu, "版主卸职, 取消 %s 的版主权限", lookupuser.userid);
			sprintf(secu, "\xb0\xe6\xd6\xf7\xd0\xb6\xd6\xb0, \xc8\xa1\xcf\xfb %s \xb5\xc4\xb0\xe6\xd6\xf7\xc8\xa8\xcf\xde", lookupuser.userid);
			securityreport(secu, 0, 1);
			move(9, 0);
			outs(secu);
		}
	}
	prints("\n\n");
	//% if (askyn("需要在相关版面发送通告吗?", YEA, NA) == NA) {
	if (askyn("\xd0\xe8\xd2\xaa\xd4\xda\xcf\xe0\xb9\xd8\xb0\xe6\xc3\xe6\xb7\xa2\xcb\xcd\xcd\xa8\xb8\xe6\xc2\xf0?", YEA, NA) == NA) {
		pressanykey();
		return 0;
	}
	prints("\n");
	//% if (askyn("正常离任请按 Enter 键确认，撤职惩罚按 N 键", YEA, NA) == YEA)
	if (askyn("\xd5\xfd\xb3\xa3\xc0\xeb\xc8\xce\xc7\xeb\xb0\xb4 Enter \xbc\xfc\xc8\xb7\xc8\xcf\xa3\xac\xb3\xb7\xd6\xb0\xb3\xcd\xb7\xa3\xb0\xb4 N \xbc\xfc", YEA, NA) == YEA)
		right = 1;
	else
		right = 0;
	if (right)
		//% sprintf(bmfilename, "%s 版%s %s 离任通告", bname, bm ? "版主" : "版副",
		sprintf(bmfilename, "%s \xb0\xe6%s %s \xc0\xeb\xc8\xce\xcd\xa8\xb8\xe6", bname, bm ? "\xb0\xe6\xd6\xf7" : "\xb0\xe6\xb8\xb1",
				lookupuser.userid);
	else
		//% sprintf(bmfilename, "[通告]撤除 %s 版%s %s", bname, bm ? "版主" : "版副",
		sprintf(bmfilename, "[\xcd\xa8\xb8\xe6]\xb3\xb7\xb3\xfd %s \xb0\xe6%s %s", bname, bm ? "\xb0\xe6\xd6\xf7" : "\xb0\xe6\xb8\xb1",
				lookupuser.userid);
	if (right) {
		//% sprintf(genbuf, "\n\t\t\t【 通告 】\n\n"
		sprintf(genbuf, "\n\t\t\t\xa1\xbe \xcd\xa8\xb8\xe6 \xa1\xbf\n\n"
			//% "\t经站务委员会讨论：\n"
			"\t\xbe\xad\xd5\xbe\xce\xf1\xce\xaf\xd4\xb1\xbb\xe1\xcc\xd6\xc2\xdb\xa3\xba\n"
			//% "\t同意 %s 辞去 %s 版的%s职务。\n"
			"\t\xcd\xac\xd2\xe2 %s \xb4\xc7\xc8\xa5 %s \xb0\xe6\xb5\xc4%s\xd6\xb0\xce\xf1\xa1\xa3\n"
			//% "\t在此，对其曾经在 %s 版的辛苦劳作表示感谢。\n\n"
			"\t\xd4\xda\xb4\xcb\xa3\xac\xb6\xd4\xc6\xe4\xd4\xf8\xbe\xad\xd4\xda %s \xb0\xe6\xb5\xc4\xd0\xc1\xbf\xe0\xc0\xcd\xd7\xf7\xb1\xed\xca\xbe\xb8\xd0\xd0\xbb\xa1\xa3\n\n"
			//% "\t希望今后也能支持本版的工作.\n", lookupuser.userid, bname, bm ? "版主"
			"\t\xcf\xa3\xcd\xfb\xbd\xf1\xba\xf3\xd2\xb2\xc4\xdc\xd6\xa7\xb3\xd6\xb1\xbe\xb0\xe6\xb5\xc4\xb9\xa4\xd7\xf7.\n", lookupuser.userid, bname, bm ? "\xb0\xe6\xd6\xf7"
				//% : "版副", bname);
				: "\xb0\xe6\xb8\xb1", bname);
	} else {
		//% sprintf(genbuf, "\n\t\t\t【撤职通告】\n\n"
		sprintf(genbuf, "\n\t\t\t\xa1\xbe\xb3\xb7\xd6\xb0\xcd\xa8\xb8\xe6\xa1\xbf\n\n"
			//% "\t经站务委员会讨论决定：\n"
			"\t\xbe\xad\xd5\xbe\xce\xf1\xce\xaf\xd4\xb1\xbb\xe1\xcc\xd6\xc2\xdb\xbe\xf6\xb6\xa8\xa3\xba\n"
			//% "\t撤除 %s 版%s %s 的%s职务。\n", bname, bm ? "版主" : "版副",
			"\t\xb3\xb7\xb3\xfd %s \xb0\xe6%s %s \xb5\xc4%s\xd6\xb0\xce\xf1\xa1\xa3\n", bname, bm ? "\xb0\xe6\xd6\xf7" : "\xb0\xe6\xb8\xb1",
				//% lookupuser.userid, bm ? "版主" : "版副");
				lookupuser.userid, bm ? "\xb0\xe6\xd6\xf7" : "\xb0\xe6\xb8\xb1");
	}

	char buf[5][STRLEN];
	for (int i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(14, 0);
	//% prints("请输入%s附言(最多五行，按 Enter 结束)", right ? "版主离任" : "版主撤职");
	prints("\xc7\xeb\xca\xe4\xc8\xeb%s\xb8\xbd\xd1\xd4(\xd7\xee\xb6\xe0\xce\xe5\xd0\xd0\xa3\xac\xb0\xb4 Enter \xbd\xe1\xca\xf8)", right ? "\xb0\xe6\xd6\xf7\xc0\xeb\xc8\xce" : "\xb0\xe6\xd6\xf7\xb3\xb7\xd6\xb0");
	for (int i = 0; i < 5; i++) {
		getdata(i + 15, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			strcat(genbuf, "\n\n");
		strcat(genbuf, "\t");
		strcat(genbuf, buf[i]);
		strcat(genbuf, "\n");
	}
	autoreport(board.name, bmfilename, genbuf, lookupuser.userid, POST_FILE_BMS);

	//% prints("\n执行完毕！");
	prints("\n\xd6\xb4\xd0\xd0\xcd\xea\xb1\xcf\xa3\xa1");
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
	//% getdata(5, 0, "请输入分区: ", buf, sizeof(buf), DOECHO, YEA);
	getdata(5, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xb7\xd6\xc7\xf8: ", buf, sizeof(buf), DOECHO, YEA);
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
		//% "BBS 系统", "复旦大学", "院系风采", "电脑技术", "休闲娱乐", "文学艺术",
		"BBS \xcf\xb5\xcd\xb3", "\xb8\xb4\xb5\xa9\xb4\xf3\xd1\xa7", "\xd4\xba\xcf\xb5\xb7\xe7\xb2\xc9", "\xb5\xe7\xc4\xd4\xbc\xbc\xca\xf5", "\xd0\xdd\xcf\xd0\xd3\xe9\xc0\xd6", "\xce\xc4\xd1\xa7\xd2\xd5\xca\xf5",
		//% "体育健身", "感性空间", "新闻信息", "学科学术", "音乐影视", "交易专区",
		"\xcc\xe5\xd3\xfd\xbd\xa1\xc9\xed", "\xb8\xd0\xd0\xd4\xbf\xd5\xbc\xe4", "\xd0\xc2\xce\xc5\xd0\xc5\xcf\xa2", "\xd1\xa7\xbf\xc6\xd1\xa7\xca\xf5", "\xd2\xf4\xc0\xd6\xd3\xb0\xca\xd3", "\xbd\xbb\xd2\xd7\xd7\xa8\xc7\xf8",
		//% "隐藏分区", NULL
		"\xd2\xfe\xb2\xd8\xb7\xd6\xc7\xf8", NULL
	};
	const char *groups[] = {
        "system.faq", "campus.faq", "ccu.faq", "comp.faq", "rec.faq",
		"literal.faq", "sport.faq", "talk.faq", "news.faq", "sci.faq",
		"other.faq", "business.faq", "hide.faq", NULL
	};

	screen_clear();
	move(2, 0);
	//% prints("选择精华区的目录\n\n");
	prints("\xd1\xa1\xd4\xf1\xbe\xab\xbb\xaa\xc7\xf8\xb5\xc4\xc4\xbf\xc2\xbc\n\n");

	int i, ch;
	for (i = 0; ; ++i) {
		if (!explain[i] || !groups[i])
			break;
		prints("\033[1;32m%2d\033[m. %-20s%-20s\n", i, explain[i], groups[i]);
	}

	char buf[STRLEN], ans[6];
	//% snprintf(buf, sizeof(buf), "请输入您的选择(0~%d): ", --i);
	snprintf(buf, sizeof(buf), "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xd1\xa1\xd4\xf1(0~%d): ", --i);
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

static int set_board_name(char *bname, size_t size)
{
	while (1) {
		//% "讨论区名称:   "
		getdata(2, 0, "\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6:   ", bname, size, DOECHO, YEA);
		if (*bname) {
			board_t board;
			if (get_board(bname, &board)) {
				//% prints("\n错误! 此讨论区已经存在!!");
				prints("\n\xb4\xed\xce\xf3! \xb4\xcb\xcc\xd6\xc2\xdb\xc7\xf8\xd2\xd1\xbe\xad\xb4\xe6\xd4\xda!!");
				pressanykey();
				return -1;
			}
		} else {
			return -1;
		}

		if (valid_board_name(bname))
			break;
		//% prints("\n不合法名称!!");
		prints("\n\xb2\xbb\xba\xcf\xb7\xa8\xc3\xfb\xb3\xc6!!");
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

	screen_clear();
	//% stand_title("开启新讨论区");
	stand_title("\xbf\xaa\xc6\xf4\xd0\xc2\xcc\xd6\xc2\xdb\xc7\xf8");

	char bname[BOARD_NAME_LEN + 1];
	if (set_board_name(bname, sizeof(bname)) != 0)
		return -1;

	GBK_UTF8_BUFFER(descr, BOARD_DESCR_CCHARS);
	//% getdata(3, 0, "讨论区说明: ", gbk_descr, sizeof(gbk_descr), DOECHO, YEA);
	getdata(3, 0, "\xcc\xd6\xc2\xdb\xc7\xf8\xcb\xb5\xc3\xf7: ", gbk_descr, sizeof(gbk_descr), DOECHO, YEA);
	if (!*gbk_descr)
		return -1;
	convert_g2u(gbk_descr, utf8_descr);

	GBK_UTF8_BUFFER(categ, BOARD_CATEG_CCHARS);
	//% getdata(4, 0, "讨论区类别: ", gbk_categ, sizeof(gbk_categ), DOECHO, YEA);
	getdata(4, 0, "\xcc\xd6\xc2\xdb\xc7\xf8\xc0\xe0\xb1\xf0: ", gbk_categ, sizeof(gbk_categ), DOECHO, YEA);
	convert_g2u(gbk_categ, utf8_categ);
	
	int sector = select_section();

	char pname[BOARD_NAME_LEN];
	//% board_complete(6, "输入所属目录: ", pname, sizeof(pname),
	board_complete(6, "\xca\xe4\xc8\xeb\xcb\xf9\xca\xf4\xc4\xbf\xc2\xbc: ", pname, sizeof(pname),
			AC_LIST_DIR_ONLY);
	board_t parent;
	get_board(pname, &parent);

	int flag = 0, perm = 0;
	move(7, 0);
	screen_clrtobot();
	//% if (askyn("本版是目录吗?", NA, NA)) {
	if (askyn("\xb1\xbe\xb0\xe6\xca\xc7\xc4\xbf\xc2\xbc\xc2\xf0?", NA, NA)) {
		flag |= (BOARD_FLAG_DIR | BOARD_FLAG_JUNK
				| BOARD_FLAG_NOREPLY | BOARD_FLAG_POST);
		//% if (askyn("是否限制存取权利?", NA, NA)) {
		if (askyn("\xca\xc7\xb7\xf1\xcf\xde\xd6\xc6\xb4\xe6\xc8\xa1\xc8\xa8\xc0\xfb?", NA, NA)) {
			char ans[2];
			//% getdata(8, 0, "限制读? [R]: ", ans, sizeof(ans), DOECHO, YEA);
			getdata(8, 0, "\xcf\xde\xd6\xc6\xb6\xc1? [R]: ", ans, sizeof(ans), DOECHO, YEA);
			move(1, 0);
			screen_clrtobot();
			move(2, 0);
			//% prints("设定 %s 权利. 讨论区: '%s'\n", "READ", bname);
			prints("\xc9\xe8\xb6\xa8 %s \xc8\xa8\xc0\xfb. \xcc\xd6\xc2\xdb\xc7\xf8: '%s'\n", "READ", bname);
			//% perm = setperms(perm, "权限", NUMPERMS, showperminfo);
			perm = setperms(perm, "\xc8\xa8\xcf\xde", NUMPERMS, showperminfo);
			screen_clear();
		}
	} else {
		//% if (askyn("该版的全部文章均不可以回复", NA, NA))
		if (askyn("\xb8\xc3\xb0\xe6\xb5\xc4\xc8\xab\xb2\xbf\xce\xc4\xd5\xc2\xbe\xf9\xb2\xbb\xbf\xc9\xd2\xd4\xbb\xd8\xb8\xb4", NA, NA))
			flag |= BOARD_FLAG_NOREPLY;
		//% if (askyn("是否是俱乐部版面", NA, NA)) {
		if (askyn("\xca\xc7\xb7\xf1\xca\xc7\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6\xc3\xe6", NA, NA)) {
			flag |= BOARD_FLAG_CLUB;
			//% if (askyn("是否读限制俱乐部版面", NA, NA))
			if (askyn("\xca\xc7\xb7\xf1\xb6\xc1\xcf\xde\xd6\xc6\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6\xc3\xe6", NA, NA))
				flag |= BOARD_FLAG_READ;
		}
		//% if (askyn("是否不计算文章数", NA, NA))
		if (askyn("\xca\xc7\xb7\xf1\xb2\xbb\xbc\xc6\xcb\xe3\xce\xc4\xd5\xc2\xca\xfd", NA, NA))
			flag |= BOARD_FLAG_JUNK;
		//% if (askyn("是否为匿名版", NA, NA))
		if (askyn("\xca\xc7\xb7\xf1\xce\xaa\xc4\xe4\xc3\xfb\xb0\xe6", NA, NA))
			flag |= BOARD_FLAG_ANONY;
#ifdef ENABLE_PREFIX
		//% if (askyn ("是否强制使用前缀", NA, NA))
		if (askyn ("\xca\xc7\xb7\xf1\xc7\xbf\xd6\xc6\xca\xb9\xd3\xc3\xc7\xb0\xd7\xba", NA, NA))
			flag |= BOARD_FLAG_PREFIX;
#endif
		//% if (askyn("是否限制读写", NA, NA)) {
		if (askyn("\xca\xc7\xb7\xf1\xcf\xde\xd6\xc6\xb6\xc1\xd0\xb4", NA, NA)) {
			char ans[2];
			//% getdata(15, 0, "限制读(R)/写(P)? [R]: ", ans, sizeof(ans),
			getdata(15, 0, "\xcf\xde\xd6\xc6\xb6\xc1(R)/\xd0\xb4(P)? [R]: ", ans, sizeof(ans),
					DOECHO, YEA);
			if (*ans == 'P' || *ans == 'p')
				flag |= BOARD_FLAG_POST;
			move(1, 0);
			screen_clrtobot();
			move(2, 0);
			//% prints("设定 %s 限制. 讨论区: '%s'\n",
			prints("\xc9\xe8\xb6\xa8 %s \xcf\xde\xd6\xc6. \xcc\xd6\xc2\xdb\xc7\xf8: '%s'\n",
					//% (flag & BOARD_FLAG_POST ? "写" : "读"), bname);
					(flag & BOARD_FLAG_POST ? "\xd0\xb4" : "\xb6\xc1"), bname);
			//% perm = setperms(perm, "权限", NUMPERMS, showperminfo);
			perm = setperms(perm, "\xc8\xa8\xcf\xde", NUMPERMS, showperminfo);
			screen_clear();
		}
	}

	db_res_t *res = db_query("INSERT INTO boards "
			"(name, descr, parent, flag, perm, categ, sector) "
			"VALUES (%s, %s, %d, %d, %d, %s, %d) RETURNING id",
			bname, utf8_descr, parent.id, flag, perm, utf8_categ, sector);
	if (!res) {
		//% prints("\n建立新版出错\n");
		prints("\n\xbd\xa8\xc1\xa2\xd0\xc2\xb0\xe6\xb3\xf6\xb4\xed\n");
		pressanykey();
		screen_clear();
		return -1;
	}
	int bid = db_get_integer(res, 0, 0);
	db_clear(res);

	char *bms = NULL;
	if (!(flag & BOARD_FLAG_DIR)
			//% && !askyn("本版诚征版主吗(否则由SYSOPs管理)?", YEA, NA)) {
			&& !askyn("\xb1\xbe\xb0\xe6\xb3\xcf\xd5\xf7\xb0\xe6\xd6\xf7\xc2\xf0(\xb7\xf1\xd4\xf2\xd3\xc9SYSOPs\xb9\xdc\xc0\xed)?", YEA, NA)) {
		bms = "SYSOPs";
		ordain_bm(bid, bms);
	}

	char vdir[HOMELEN];
	snprintf(vdir, sizeof(vdir), "vote/%s", bname);
	char bdir[HOMELEN];
	snprintf(bdir, sizeof(bdir), "boards/%s", bname);
	if (mkdir(bdir, 0755) != 0 || mkdir(vdir, 0755) != 0) {
		//% prints("\n新建目录出错!\n");
		prints("\n\xd0\xc2\xbd\xa8\xc4\xbf\xc2\xbc\xb3\xf6\xb4\xed!\n");
		pressreturn();
		screen_clear();
		return -1;
	}

	if (!(flag & BOARD_FLAG_DIR)) {
		const char *group = chgrp();
		if (group) {
			char buf[STRLEN];
			if (bms) {
				//% snprintf(buf, sizeof(buf), "○ %-35.35s(BM: %s)",
				snprintf(buf, sizeof(buf), "\xa1\xf0 %-35.35s(BM: %s)",
						gbk_descr, bms);
			} else {
				//% snprintf(buf, sizeof(buf), "○ %-35.35s", gbk_descr);
				snprintf(buf, sizeof(buf), "\xa1\xf0 %-35.35s", gbk_descr);
			}
			if (add_grp(group, cexplain, bname, buf) == -1) {
				//% prints("\n成立精华区失败....\n");
				prints("\n\xb3\xc9\xc1\xa2\xbe\xab\xbb\xaa\xc7\xf8\xca\xa7\xb0\xdc....\n");
			} else {
				//% prints("已经置入精华区...\n");
				prints("\xd2\xd1\xbe\xad\xd6\xc3\xc8\xeb\xbe\xab\xbb\xaa\xc7\xf8...\n");
			}
		}
	}

	//% prints("\n新讨论区成立\n");
	prints("\n\xd0\xc2\xcc\xd6\xc2\xdb\xc7\xf8\xb3\xc9\xc1\xa2\n");

	char buf[STRLEN];
	//% snprintf(buf, sizeof(buf), "成立新版：%s", bname);
	snprintf(buf, sizeof(buf), "\xb3\xc9\xc1\xa2\xd0\xc2\xb0\xe6\xa3\xba%s", bname);
	securityreport(buf, 0, 1);

	screen_clear();
	return 0;
}

static void show_edit_board_menu(board_t *bp, board_t *pp)
{
	//% prints("1)修改名称:        %s\n", bp->name);
	prints("1)\xd0\xde\xb8\xc4\xc3\xfb\xb3\xc6:        %s\n", bp->name);
	//% prints("2)修改说明:        %s\n", bp->descr);
	prints("2)\xd0\xde\xb8\xc4\xcb\xb5\xc3\xf7:        %s\n", bp->descr);
	//% prints("4)修改所属目录:    %s(%d)\n", pp->name, pp->id);
	prints("4)\xd0\xde\xb8\xc4\xcb\xf9\xca\xf4\xc4\xbf\xc2\xbc:    %s(%d)\n", pp->name, pp->id);
	if (bp->flag & BOARD_FLAG_DIR) {
		//% prints("5)修改读写属性:    %s\n",
		prints("5)\xd0\xde\xb8\xc4\xb6\xc1\xd0\xb4\xca\xf4\xd0\xd4:    %s\n",
				//% (bp->perm == 0) ? "没有限制" : "r(限制阅读)");
				(bp->perm == 0) ? "\xc3\xbb\xd3\xd0\xcf\xde\xd6\xc6" : "r(\xcf\xde\xd6\xc6\xd4\xc4\xb6\xc1)");
	} else {
		//% prints("5)修改读写属性:    %s\n",
		prints("5)\xd0\xde\xb8\xc4\xb6\xc1\xd0\xb4\xca\xf4\xd0\xd4:    %s\n",
				//% (bp->flag & BOARD_FLAG_POST) ? "p(限制发文)"
				(bp->flag & BOARD_FLAG_POST) ? "p(\xcf\xde\xd6\xc6\xb7\xa2\xce\xc4)"
				//% : (bp->perm == 0) ? "没有限制" : "r(限制阅读)");
				: (bp->perm == 0) ? "\xc3\xbb\xd3\xd0\xcf\xde\xd6\xc6" : "r(\xcf\xde\xd6\xc6\xd4\xc4\xb6\xc1)");
	}

	if (!(bp->flag & BOARD_FLAG_DIR)) {
		//% prints("8)匿名版面:            %s\n",
		prints("8)\xc4\xe4\xc3\xfb\xb0\xe6\xc3\xe6:            %s\n",
				//% (bp->flag & BOARD_FLAG_ANONY) ? "是" : "否");
				(bp->flag & BOARD_FLAG_ANONY) ? "\xca\xc7" : "\xb7\xf1");
		//% prints("9)可以回复:            %s\n",
		prints("9)\xbf\xc9\xd2\xd4\xbb\xd8\xb8\xb4:            %s\n",
				//% (bp->flag & BOARD_FLAG_NOREPLY) ? "否" : "是");
				(bp->flag & BOARD_FLAG_NOREPLY) ? "\xb7\xf1" : "\xca\xc7");
		//% prints("A)是否计算文章数:      %s\n",
		prints("A)\xca\xc7\xb7\xf1\xbc\xc6\xcb\xe3\xce\xc4\xd5\xc2\xca\xfd:      %s\n",
				//% (bp->flag & BOARD_FLAG_JUNK) ? "否" : "是");
				(bp->flag & BOARD_FLAG_JUNK) ? "\xb7\xf1" : "\xca\xc7");
		//% prints("B)俱乐部属性:          %s\n",
		prints("B)\xbe\xe3\xc0\xd6\xb2\xbf\xca\xf4\xd0\xd4:          %s\n",
				(bp->flag & BOARD_FLAG_CLUB) ?
				(bp->flag & BOARD_FLAG_READ) ?
				//% "\033[1;31mc\033[0m(读限制)"
				"\033[1;31mc\033[0m(\xb6\xc1\xcf\xde\xd6\xc6)"
				//% : "\033[1;33mc\033[0m(写限制)"
				: "\033[1;33mc\033[0m(\xd0\xb4\xcf\xde\xd6\xc6)"
				//% : "非俱乐部");
				: "\xb7\xc7\xbe\xe3\xc0\xd6\xb2\xbf");
#ifdef ENABLE_PREFIX
		//% prints ("C)是否强制使用前缀:    %s\n",
		prints ("C)\xca\xc7\xb7\xf1\xc7\xbf\xd6\xc6\xca\xb9\xd3\xc3\xc7\xb0\xd7\xba:    %s\n",
				//% (bp->flag & BOARD_FLAG_PREFIX) ? "是" : "否");
				(bp->flag & BOARD_FLAG_PREFIX) ? "\xca\xc7" : "\xb7\xf1");
#endif
	}
}

static bool alter_board_name(board_t *bp)
{
	char bname[BOARD_NAME_LEN + 1];
	//% 新讨论区名称: 
	getdata(-2, 0, "\xd0\xc2\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6: ", bname, sizeof(bname),
			DOECHO, YEA);
	if (!*bname || streq(bp->name, bname) || !valid_board_name(bname))
		return 0;

	//% if (!askyn("确定修改版名?", NA, YEA))
	if (!askyn("\xc8\xb7\xb6\xa8\xd0\xde\xb8\xc4\xb0\xe6\xc3\xfb?", NA, YEA))
		return 0;

	db_res_t *res = db_cmd("UPDATE boards SET name = %s WHERE id = %d",
			bname, bp->id);
	db_clear(res);
	return res;
}

static bool alter_board_descr(board_t *bp)
{
	GBK_UTF8_BUFFER(descr, BOARD_DESCR_CCHARS);
	//% 新讨论区说明:
	getdata(-2, 0, "\xd0\xc2\xcc\xd6\xc2\xdb\xc7\xf8\xcb\xb5\xc3\xf7: ",
			gbk_descr, sizeof(gbk_descr), DOECHO, YEA);
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
	//% board_complete(15, "输入所属讨论区名: ", gbk_bname, sizeof(gbk_bname),
	board_complete(15, "\xca\xe4\xc8\xeb\xcb\xf9\xca\xf4\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb: ", gbk_bname, sizeof(gbk_bname),
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
	if (bp->flag & BOARD_FLAG_DIR) {
		//% snprintf(buf, sizeof(buf), "(N)不限制 (R)限制阅读 [%c]: ",
		snprintf(buf, sizeof(buf), "(N)\xb2\xbb\xcf\xde\xd6\xc6 (R)\xcf\xde\xd6\xc6\xd4\xc4\xb6\xc1 [%c]: ",
				(bp->perm) ? 'R' : 'N');
		getdata(15, 0, buf, ans, sizeof(ans), DOECHO, YEA);
		if (ans[0] == 'N' || ans[0] == 'n') {
			flag &= ~BOARD_FLAG_POST;
			perm = 0;
		} else {
			if (ans[0] == 'R' || ans[0] == 'r')
				flag &= ~BOARD_FLAG_POST;
			screen_clear();
			move(2, 0);
			//% prints("设定 %s '%s' 讨论区的权限\n", "阅读", bp->name);
			prints("\xc9\xe8\xb6\xa8 %s '%s' \xcc\xd6\xc2\xdb\xc7\xf8\xb5\xc4\xc8\xa8\xcf\xde\n", "\xd4\xc4\xb6\xc1", bp->name);
			//% perm = setperms(perm, "权限", NUMPERMS, showperminfo);
			perm = setperms(perm, "\xc8\xa8\xcf\xde", NUMPERMS, showperminfo);
			screen_clear();
		}
	} else {
		//% snprintf(buf, sizeof(buf), "(N)不限制 (R)限制阅读 (P)限制张贴 文章 [%c]: ",
		snprintf(buf, sizeof(buf), "(N)\xb2\xbb\xcf\xde\xd6\xc6 (R)\xcf\xde\xd6\xc6\xd4\xc4\xb6\xc1 (P)\xcf\xde\xd6\xc6\xd5\xc5\xcc\xf9 \xce\xc4\xd5\xc2 [%c]: ",
				(flag & BOARD_FLAG_POST) ? 'P' : (perm) ? 'R' : 'N');
		getdata(15, 0, buf, ans, sizeof(ans), DOECHO, YEA);
		if (ans[0] == 'N' || ans[0] == 'n') {
			flag &= ~BOARD_FLAG_POST;
			perm = 0;
		} else {
			if (ans[0] == 'R' || ans[0] == 'r')
				flag &= ~BOARD_FLAG_POST;
			else if (ans[0] == 'P' || ans[0] == 'p')
				flag |= BOARD_FLAG_POST;
			screen_clear();
			move(2, 0);
			//% prints("设定 %s '%s' 讨论区的权限\n",
			prints("\xc9\xe8\xb6\xa8 %s '%s' \xcc\xd6\xc2\xdb\xc7\xf8\xb5\xc4\xc8\xa8\xcf\xde\n",
					//% (flag & BOARD_FLAG_POST) ? "张贴" : "阅读", bp->name);
					(flag & BOARD_FLAG_POST) ? "\xd5\xc5\xcc\xf9" : "\xd4\xc4\xb6\xc1", bp->name);
			//% perm = setperms(perm, "权限", NUMPERMS, showperminfo);
			perm = setperms(perm, "\xc8\xa8\xcf\xde", NUMPERMS, showperminfo);
			screen_clear();
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

	if (flag == BOARD_FLAG_CLUB && (f & BOARD_FLAG_CLUB)) {
		//% if (askyn("是否读限制俱乐部?",
		if (askyn("\xca\xc7\xb7\xf1\xb6\xc1\xcf\xde\xd6\xc6\xbe\xe3\xc0\xd6\xb2\xbf?",
					(bp->flag & BOARD_FLAG_READ) ? YEA : NA, NA)) {
			f |= BOARD_FLAG_READ;
		} else {
			f &= ~BOARD_FLAG_READ;
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

	screen_clear();
	//% stand_title("修改讨论区设置");
	stand_title("\xd0\xde\xb8\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xc9\xe8\xd6\xc3");

	char bname[BOARD_NAME_LEN + 1];
	//% board_complete(2, "输入讨论区名称: ", bname, sizeof(bname),
	board_complete(2, "\xca\xe4\xc8\xeb\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6: ", bname, sizeof(bname),
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

	screen_clear();
	//% stand_title("修改讨论区设置");
	stand_title("\xd0\xde\xb8\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xc9\xe8\xd6\xc3");
	move(2, 0);

	show_edit_board_menu(&board, &parent);

	char ans[2];
	//% getdata(14, 0, "更改哪项设置[0]", ans, sizeof(ans), DOECHO, YEA);
	getdata(14, 0, "\xb8\xfc\xb8\xc4\xc4\xc4\xcf\xee\xc9\xe8\xd6\xc3[0]", ans, sizeof(ans), DOECHO, YEA);
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

	if (!(board.flag & BOARD_FLAG_DIR)) {
		switch (ans[0]) {
			case '7':
				//% res = askyn("移动精华区", NA, YEA);
				res = askyn("\xd2\xc6\xb6\xaf\xbe\xab\xbb\xaa\xc7\xf8", NA, YEA);
				break;
			case '8':
				//% res = alter_board_flag(&board, "是否匿名?", BOARD_FLAG_ANONY);
				res = alter_board_flag(&board, "\xca\xc7\xb7\xf1\xc4\xe4\xc3\xfb?", BOARD_FLAG_ANONY);
				break;
			case '9':
				//% res = alter_board_flag(&board, "禁止回复?", BOARD_FLAG_NOREPLY);
				res = alter_board_flag(&board, "\xbd\xfb\xd6\xb9\xbb\xd8\xb8\xb4?", BOARD_FLAG_NOREPLY);
				break;
			case 'a':
			case 'A':
				//% res = alter_board_flag(&board, "不计文章数?", BOARD_FLAG_JUNK);
				res = alter_board_flag(&board, "\xb2\xbb\xbc\xc6\xce\xc4\xd5\xc2\xca\xfd?", BOARD_FLAG_JUNK);
				break;
			case 'b':
			case 'B':
				//% res = alter_board_flag(&board, "是否俱乐部?", BOARD_FLAG_CLUB);
				res = alter_board_flag(&board, "\xca\xc7\xb7\xf1\xbe\xe3\xc0\xd6\xb2\xbf?", BOARD_FLAG_CLUB);
				break;
#ifdef ENABLE_PREFIX
			case 'c':
			case 'C':
				//% res = alter_board_flag(&board, "强制前缀?", BOARD_FLAG_PREFIX);
				res = alter_board_flag(&board, "\xc7\xbf\xd6\xc6\xc7\xb0\xd7\xba?", BOARD_FLAG_PREFIX);
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
			//% sprintf(secu, "修改讨论区：%s(%s)", board.name, nb.name);
			sprintf(secu, "\xd0\xde\xb8\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xa3\xba%s(%s)", board.name, nb.name);
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
			//% snprintf(vbuf, sizeof(vbuf), "○ %-35.35s(BM: %s)",
			snprintf(vbuf, sizeof(vbuf), "\xa1\xf0 %-35.35s(BM: %s)",
					nb.descr, nb.bms);
		} else {
			//% snprintf(vbuf, sizeof(vbuf), "○ %-35.35s", nb.descr);
			snprintf(vbuf, sizeof(vbuf), "\xa1\xf0 %-35.35s", nb.descr);
		}

		char old_descr[STRLEN];
		//% snprintf(old_descr, sizeof(old_descr), "○ %s", board.descr);
		snprintf(old_descr, sizeof(old_descr), "\xa1\xf0 %s", board.descr);

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
						//% prints("\n成立精华区失败....\n");
						prints("\n\xb3\xc9\xc1\xa2\xbe\xab\xbb\xaa\xc7\xf8\xca\xa7\xb0\xdc....\n");
					else
						//% prints("已经置入精华区...\n");
						prints("\xd2\xd1\xbe\xad\xd6\xc3\xc8\xeb\xbe\xab\xbb\xaa\xc7\xf8...\n");

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
		//% snprintf(buf, sizeof(buf), "更改讨论区 %s 的资料 --> %s", board.name, nb.name);
		snprintf(buf, sizeof(buf), "\xb8\xfc\xb8\xc4\xcc\xd6\xc2\xdb\xc7\xf8 %s \xb5\xc4\xd7\xca\xc1\xcf --> %s", board.name, nb.name);
		report(buf, currentuser.userid);
	}

	pressanykey();
	screen_clear();
	return 0;
}

// 批注册单时显示的标题
int regtitle(void)
{
	//% prints("\033[1;33;44m批注册单 NEW VERSION wahahaha              "
	prints("\033[1;33;44m\xc5\xfa\xd7\xa2\xb2\xe1\xb5\xa5 NEW VERSION wahahaha              "
			"                                     \033[m\n"
			//% " 离开[\033[1;32m←\033[m,\033[1;32me\033[m] "
			" \xc0\xeb\xbf\xaa[\033[1;32m\xa1\xfb\033[m,\033[1;32me\033[m] "
			//% "选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] "
			"\xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd\033[m] "
			//% "阅读[\033[1;32m→\033[m,\033[1;32mRtn\033[m] "
			"\xd4\xc4\xb6\xc1[\033[1;32m\xa1\xfa\033[m,\033[1;32mRtn\033[m] "
			//% "批准[\033[1;32my\033[m] 删除[\033[1;32md\033[m]\n"
			"\xc5\xfa\xd7\xbc[\033[1;32my\033[m] \xc9\xbe\xb3\xfd[\033[1;32md\033[m]\n"
			//% "\033[1;37;44m  编号 用户ID       姓  名       系别"
			"\033[1;37;44m  \xb1\xe0\xba\xc5 \xd3\xc3\xbb\xa7ID       \xd0\xd5  \xc3\xfb       \xcf\xb5\xb1\xf0"
			//% "             住址             注册时间     \033[m\n");
			"             \xd7\xa1\xd6\xb7             \xd7\xa2\xb2\xe1\xca\xb1\xbc\xe4     \033[m\n");
	return 0;
}

//      在批注册单时显示的注册ID列表
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
			rname, dept, addr, format_time(ent->regdate, TIME_FORMAT_SHORT));
	return buf;
}

//      返回userid 与ent->userid是否相等
static int filecheck(void *ent, void *userid)
{
	return !strcmp(((reginfo_t *)ent)->userid, (char *)userid);
}

// 删除注册单文件里的一个记录
int delete_register(int index, reginfo_t* ent, char *direct) {
	delete_record(direct, sizeof(reginfo_t), index, filecheck, ent->userid);
	return DIRCHANGED;
}

//      通过注册单
int pass_register(int index, reginfo_t* ent, char *direct) {
	int unum;
	struct userec user;
	char buf[80];
	FILE *fout;

	unum = getuser(ent->userid);
	if (!unum) {
		screen_clear();
		//% "系统错误! 查无此账号!\n"
		prints("\xcf\xb5\xcd\xb3\xb4\xed\xce\xf3! \xb2\xe9\xce\xde\xb4\xcb\xd5\xcb\xba\xc5!\n"); //在回档或者某些情况下,找不到在注册单文件
		pressanykey(); // unregister中的此记录,故删除
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
		//% fprintf(fout, "注册时间     : %s\n", format_time(ent->regdate, TIME_FORMAT_EN));
		fprintf(fout, "\xd7\xa2\xb2\xe1\xca\xb1\xbc\xe4     : %s\n", format_time(ent->regdate, TIME_FORMAT_EN));
		//% fprintf(fout, "申请帐号     : %s\n", ent->userid);
		fprintf(fout, "\xc9\xea\xc7\xeb\xd5\xca\xba\xc5     : %s\n", ent->userid);
		//% fprintf(fout, "真实姓名     : %s\n", ent->realname);
		fprintf(fout, "\xd5\xe6\xca\xb5\xd0\xd5\xc3\xfb     : %s\n", ent->realname);
		//% fprintf(fout, "学校系级     : %s\n", ent->dept);
		fprintf(fout, "\xd1\xa7\xd0\xa3\xcf\xb5\xbc\xb6     : %s\n", ent->dept);
		//% fprintf(fout, "目前住址     : %s\n", ent->addr);
		fprintf(fout, "\xc4\xbf\xc7\xb0\xd7\xa1\xd6\xb7     : %s\n", ent->addr);
		//% fprintf(fout, "联络电话     : %s\n", ent->phone);
		fprintf(fout, "\xc1\xaa\xc2\xe7\xb5\xe7\xbb\xb0     : %s\n", ent->phone);
#ifndef FDQUAN
		//% fprintf(fout, "电子邮件     : %s\n", ent->email);
		fprintf(fout, "\xb5\xe7\xd7\xd3\xd3\xca\xbc\xfe     : %s\n", ent->email);
#endif
		//% fprintf(fout, "校 友 会     : %s\n", ent->assoc);
		fprintf(fout, "\xd0\xa3 \xd3\xd1 \xbb\xe1     : %s\n", ent->assoc);
		//% fprintf(fout, "成功日期     : %s\n", format_time(time(NULL), TIME_FORMAT_EN));
		fprintf(fout, "\xb3\xc9\xb9\xa6\xc8\xd5\xc6\xda     : %s\n", format_time(fb_time(), TIME_FORMAT_EN));
		//% fprintf(fout, "批准人       : %s\n", currentuser.userid);
		fprintf(fout, "\xc5\xfa\xd7\xbc\xc8\xcb       : %s\n", currentuser.userid);
		fclose(fout);
	}
	//% mail_file("etc/s_fill", user.userid, "恭禧您，您已经完成注册。");
	mail_file("etc/s_fill", user.userid, "\xb9\xa7\xec\xfb\xc4\xfa\xa3\xac\xc4\xfa\xd2\xd1\xbe\xad\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa1\xa3");
	sethomefile(buf, user.userid, "mailcheck");
	unlink(buf);
	//% sprintf(genbuf, "让 %s 通过身分确认.", user.userid);
	sprintf(genbuf, "\xc8\xc3 %s \xcd\xa8\xb9\xfd\xc9\xed\xb7\xd6\xc8\xb7\xc8\xcf.", user.userid);
	securityreport(genbuf, 0, 0);

	return DIRCHANGED;
}

//      处理注册单
int do_register(int index, reginfo_t* ent, char *direct) {
	int unum;
	struct userec user;
	//char ps[80];
	register int ch;
	//% static char *reason[] = { "请确实填写真实姓名.", "请详填学校科系与年级.", "请填写完整的住址资料.",
	static char *reason[] = { "\xc7\xeb\xc8\xb7\xca\xb5\xcc\xee\xd0\xb4\xd5\xe6\xca\xb5\xd0\xd5\xc3\xfb.", "\xc7\xeb\xcf\xea\xcc\xee\xd1\xa7\xd0\xa3\xbf\xc6\xcf\xb5\xd3\xeb\xc4\xea\xbc\xb6.", "\xc7\xeb\xcc\xee\xd0\xb4\xcd\xea\xd5\xfb\xb5\xc4\xd7\xa1\xd6\xb7\xd7\xca\xc1\xcf.",
			//% "请详填联络电话.", "请确实填写注册申请表.", "请用中文填写申请单.", "其他" };
			"\xc7\xeb\xcf\xea\xcc\xee\xc1\xaa\xc2\xe7\xb5\xe7\xbb\xb0.", "\xc7\xeb\xc8\xb7\xca\xb5\xcc\xee\xd0\xb4\xd7\xa2\xb2\xe1\xc9\xea\xc7\xeb\xb1\xed.", "\xc7\xeb\xd3\xc3\xd6\xd0\xce\xc4\xcc\xee\xd0\xb4\xc9\xea\xc7\xeb\xb5\xa5.", "\xc6\xe4\xcb\xfb" };
	unsigned char rejectindex = 4;

	if (!ent)
		return DONOTHING;

	unum = getuser(ent->userid);
	if (!unum) {
		//% prints("系统错误! 查无此账号!\n"); //删除不存在的记录,如果有的话
		prints("\xcf\xb5\xcd\xb3\xb4\xed\xce\xf3! \xb2\xe9\xce\xde\xb4\xcb\xd5\xcb\xba\xc5!\n"); //\xc9\xbe\xb3\xfd\xb2\xbb\xb4\xe6\xd4\xda\xb5\xc4\xbc\xc7\xc2\xbc,\xc8\xe7\xb9\xfb\xd3\xd0\xb5\xc4\xbb\xb0
		delete_record(direct, sizeof(reginfo_t), index, filecheck,
				ent->userid);
		return DIRCHANGED;
	}

	memcpy(&user, &lookupuser, sizeof (user));
	screen_clear();
	move(0, 0);
	//% prints("[1;33;44m 详细资料                                                                      [m\n");
	prints("[1;33;44m \xcf\xea\xcf\xb8\xd7\xca\xc1\xcf                                                                      [m\n");
	//% prints("[1;37;42m [.]接受 [+]拒绝 [d]删除 [0-6]不符合原因                                       [m");
	prints("[1;37;42m [.]\xbd\xd3\xca\xdc [+]\xbe\xdc\xbe\xf8 [d]\xc9\xbe\xb3\xfd [0-6]\xb2\xbb\xb7\xfb\xba\xcf\xd4\xad\xd2\xf2                                       [m");

	for (;;) {
		disply_userinfo(&user);
		move(14, 0);
		printdash(NULL);
		//% prints("   注册时间   : %s\n", format_time(ent->regdate, TIME_FORMAT_EN));
		prints("   \xd7\xa2\xb2\xe1\xca\xb1\xbc\xe4   : %s\n", format_time(ent->regdate, TIME_FORMAT_EN));
		//% prints("   申请帐号   : %s\n", ent->userid);
		prints("   \xc9\xea\xc7\xeb\xd5\xca\xba\xc5   : %s\n", ent->userid);
		//% prints("   真实姓名   : %s\n", ent->realname);
		prints("   \xd5\xe6\xca\xb5\xd0\xd5\xc3\xfb   : %s\n", ent->realname);
		//% prints("   学校系级   : %s\n", ent->dept);
		prints("   \xd1\xa7\xd0\xa3\xcf\xb5\xbc\xb6   : %s\n", ent->dept);
		//% prints("   目前住址   : %s\n", ent->addr);
		prints("   \xc4\xbf\xc7\xb0\xd7\xa1\xd6\xb7   : %s\n", ent->addr);
		//% prints("   联络电话   : %s\n", ent->phone);
		prints("   \xc1\xaa\xc2\xe7\xb5\xe7\xbb\xb0   : %s\n", ent->phone);
#ifndef FDQUAN
		//% prints("   电子邮件   : %s\n", ent->email);
		prints("   \xb5\xe7\xd7\xd3\xd3\xca\xbc\xfe   : %s\n", ent->email);
#endif
		//% prints("   校 友 会   : %s\n", ent->assoc);
		prints("   \xd0\xa3 \xd3\xd1 \xbb\xe1   : %s\n", ent->assoc);
		ch = egetch();
		switch (ch) {
			case '.':
				pass_register(index, ent, direct);
				return READ_AGAIN;
			case '+':
				user.userlevel &= ~PERM_SPECIAL4;
				substitut_record(PASSFILE, &user, sizeof (user), unum);
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
	screen_clear();
	//% stand_title("查询使用者注册资料");
	stand_title("\xb2\xe9\xd1\xaf\xca\xb9\xd3\xc3\xd5\xdf\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf");
	move(1, 0);
	//% usercomplete("请输入要查询的代号: ", uident);
	usercomplete("\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xb2\xe9\xd1\xaf\xb5\xc4\xb4\xfa\xba\xc5: ", uident);
	if (uident[0] != '\0') {
		if (!getuser(uident)) {
			move(2, 0);
			//% prints("错误的使用者代号...");
			prints("\xb4\xed\xce\xf3\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5...");
		} else {
			db_res_t *r = db_query("SELECT addr"
					" FROM alive_users u JOIN emails e ON u.email = e.id"
					" WHERE lower(name) = lower(%s)", uident);
			if (r && db_res_rows(r) == 1) {
				prints("\033[1;32m%s\033[m\n", db_get_value(r, 0, 0));
			}
			db_clear(r);

			sprintf(genbuf, "home/%c/%s/register",
					toupper(lookupuser.userid[0]), lookupuser.userid);
			if ((fn = fopen(genbuf, "r")) != NULL) {
				//% prints("\n注册资料如下:\n\n");
				prints("\n\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf\xc8\xe7\xcf\xc2:\n\n");
				for (x = 1; x <= 15; x++) {
					if (fgets(genbuf, STRLEN, fn))
						prints("%s", genbuf);
					else
						break;
				}
			} else {
				//% prints("\n\n找不到他/她的注册资料!!\n");
				prints("\n\n\xd5\xd2\xb2\xbb\xb5\xbd\xcb\xfb/\xcb\xfd\xb5\xc4\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf!!\n");
			}
		}
	}
	pressanykey();
}

//  进入 注册单察看栏,看使用者的注册资料或进注册单管理程序
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
	screen_clear();

	//% stand_title("设定使用者注册资料");
	stand_title("\xc9\xe8\xb6\xa8\xca\xb9\xd3\xc3\xd5\xdf\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf");
	for (;;) {
		//% getdata(1, 0, "(0)离开  (1)审查新注册 (2)查询使用者注册资料 ? : ", ans, 2, DOECHO,
		getdata(1, 0, "(0)\xc0\xeb\xbf\xaa  (1)\xc9\xf3\xb2\xe9\xd0\xc2\xd7\xa2\xb2\xe1 (2)\xb2\xe9\xd1\xaf\xca\xb9\xd3\xc3\xd5\xdf\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf ? : ", ans, 2, DOECHO,
				YEA);
		//% if (ans[0] == '1' || ans[0] == '2') { // || ans[0]=='3') 现在只有0,1,2
		if (ans[0] == '1' || ans[0] == '2') { // || ans[0]=='3') \xcf\xd6\xd4\xda\xd6\xbb\xd3\xd00,1,2
			break;
		} else {
			return 0;
		}
	}
	switch (ans[0]) {
		case '2':
			move(1, 0);
			//% usercomplete("请输入要查询的代号: ", uident);
			usercomplete("\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xb2\xe9\xd1\xaf\xb5\xc4\xb4\xfa\xba\xc5: ", uident);
			if (uident[0] != '\0') {
				if (!getuser(uident)) {
					move(2, 0);
					//% prints("错误的使用者代号...");
					prints("\xb4\xed\xce\xf3\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5...");
				} else {
					sprintf(genbuf, "home/%c/%s/register",
							toupper(lookupuser.userid[0]),
							lookupuser.userid);
					if ((fn = fopen(genbuf, "r")) != NULL) {
						//% prints("\n注册资料如下:\n\n");
						prints("\n\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf\xc8\xe7\xcf\xc2:\n\n");
						for (x = 1; x <= 15; x++) {
							if (fgets(genbuf, STRLEN, fn))
								prints("%s", genbuf);
							else
								break;
						}
					} else {
						//% prints("\n\n找不到他/她的注册资料!!\n");
						prints("\n\n\xd5\xd2\xb2\xbb\xb5\xbd\xcb\xfb/\xcb\xfd\xb5\xc4\xd7\xa2\xb2\xe1\xd7\xca\xc1\xcf!!\n");
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
	screen_clear();
	return 0;
}

//      更改使用者的权限
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
	screen_clear();
	//% prints("更改使用者权限\n");
	prints("\xb8\xfc\xb8\xc4\xca\xb9\xd3\xc3\xd5\xdf\xc8\xa8\xcf\xde\n");
	move(1, 0);
	//% usercomplete("输入欲更改的使用者帐号: ", genbuf);
	usercomplete("\xca\xe4\xc8\xeb\xd3\xfb\xb8\xfc\xb8\xc4\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf\xd5\xca\xba\xc5: ", genbuf);
	if (genbuf[0] == '\0') {
		screen_clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		screen_move_clear(3);
		prints("Invalid User Id");
		pressreturn();
		screen_clear();
		return 0;
	}
	move(1, 0);
	screen_clrtobot();
	move(2, 0);
	//% prints("设定使用者 '%s' 的权限 \n", genbuf);
	prints("\xc9\xe8\xb6\xa8\xca\xb9\xd3\xc3\xd5\xdf '%s' \xb5\xc4\xc8\xa8\xcf\xde \n", genbuf);
	newlevel
			//% = setperms(lookupuser.userlevel, "权限", NUMPERMS, showperminfo);
			= setperms(lookupuser.userlevel, "\xc8\xa8\xcf\xde", NUMPERMS, showperminfo);
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		//% prints("使用者 '%s' 权限没有变更\n", lookupuser.userid);
		prints("\xca\xb9\xd3\xc3\xd5\xdf '%s' \xc8\xa8\xcf\xde\xc3\xbb\xd3\xd0\xb1\xe4\xb8\xfc\n", lookupuser.userid);
	else {
		sprintf(reportbuf, "change level: %s %.8x -> %.8x",
				lookupuser.userid, lookupuser.userlevel, newlevel);
		report(reportbuf, currentuser.userid);
		lookupuser.userlevel = newlevel;
		{
			char secu[STRLEN];
			//% sprintf(secu, "修改 %s 的权限", lookupuser.userid);
			sprintf(secu, "\xd0\xde\xb8\xc4 %s \xb5\xc4\xc8\xa8\xcf\xde", lookupuser.userid);
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
		//% prints("使用者 '%s' 权限已经更改完毕.\n", lookupuser.userid);
		prints("\xca\xb9\xd3\xc3\xd5\xdf '%s' \xc8\xa8\xcf\xde\xd2\xd1\xbe\xad\xb8\xfc\xb8\xc4\xcd\xea\xb1\xcf.\n", lookupuser.userid);
	}
	pressreturn();
	screen_clear();
	return 0;
}

void a_edits() {
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
	//% static char *explain_file[] = { "特殊进站公布栏", "进站画面", "进站欢迎档", "离站画面",
	static char *explain_file[] = { "\xcc\xd8\xca\xe2\xbd\xf8\xd5\xbe\xb9\xab\xb2\xbc\xc0\xb8", "\xbd\xf8\xd5\xbe\xbb\xad\xc3\xe6", "\xbd\xf8\xd5\xbe\xbb\xb6\xd3\xad\xb5\xb5", "\xc0\xeb\xd5\xbe\xbb\xad\xc3\xe6",
			//% "公用备忘录", "系统热点", "menu.ini", "不可注册的 ID", "不可确认之E-Mail",
			"\xb9\xab\xd3\xc3\xb1\xb8\xcd\xfc\xc2\xbc", "\xcf\xb5\xcd\xb3\xc8\xc8\xb5\xe3", "menu.ini", "\xb2\xbb\xbf\xc9\xd7\xa2\xb2\xe1\xb5\xc4 ID", "\xb2\xbb\xbf\xc9\xc8\xb7\xc8\xcf\xd6\xae""E-Mail",
			//% "不可上站之位址", "每日自动送信档", "不算POST数的版", "管理者名单", "纪念日清单",
			"\xb2\xbb\xbf\xc9\xc9\xcf\xd5\xbe\xd6\xae\xce\xbb\xd6\xb7", "\xc3\xbf\xc8\xd5\xd7\xd4\xb6\xaf\xcb\xcd\xd0\xc5\xb5\xb5", "\xb2\xbb\xcb\xe3POST\xca\xfd\xb5\xc4\xb0\xe6", "\xb9\xdc\xc0\xed\xd5\xdf\xc3\xfb\xb5\xa5", "\xbc\xcd\xc4\xee\xc8\xd5\xc7\xe5\xb5\xa5",
			//% "暂停登陆(NOLOGIN)", "暂停注册(NOREGISTER)", "个人ip来源设定档", "穿梭ip来源设定档",
			"\xd4\xdd\xcd\xa3\xb5\xc7\xc2\xbd(NOLOGIN)", "\xd4\xdd\xcd\xa3\xd7\xa2\xb2\xe1(NOREGISTER)", "\xb8\xf6\xc8\xcbip\xc0\xb4\xd4\xb4\xc9\xe8\xb6\xa8\xb5\xb5", "\xb4\xa9\xcb\xf3ip\xc0\xb4\xd4\xb4\xc9\xe8\xb6\xa8\xb5\xb5",
			//% "只能登陆5id的ip设定档", "不受5 id限制的ip设定档", "注册成功信件", "注册失败信件",
			"\xd6\xbb\xc4\xdc\xb5\xc7\xc2\xbd""5id\xb5\xc4""ip\xc9\xe8\xb6\xa8\xb5\xb5", "\xb2\xbb\xca\xdc""5 id\xcf\xde\xd6\xc6\xb5\xc4""ip\xc9\xe8\xb6\xa8\xb5\xb5", "\xd7\xa2\xb2\xe1\xb3\xc9\xb9\xa6\xd0\xc5\xbc\xfe", "\xd7\xa2\xb2\xe1\xca\xa7\xb0\xdc\xd0\xc5\xbc\xfe",
			//% "新用户注册范例", "用户第一次登陆公告", "国际会议厅清单", "区段删除不需备份之清单",
			"\xd0\xc2\xd3\xc3\xbb\xa7\xd7\xa2\xb2\xe1\xb7\xb6\xc0\xfd", "\xd3\xc3\xbb\xa7\xb5\xda\xd2\xbb\xb4\xce\xb5\xc7\xc2\xbd\xb9\xab\xb8\xe6", "\xb9\xfa\xbc\xca\xbb\xe1\xd2\xe9\xcc\xfc\xc7\xe5\xb5\xa5", "\xc7\xf8\xb6\xce\xc9\xbe\xb3\xfd\xb2\xbb\xd0\xe8\xb1\xb8\xb7\xdd\xd6\xae\xc7\xe5\xb5\xa5",
			//% "BBSNET 转站清单", "穿梭限制ip", "BBSNET2 转站清单", "穿梭2限制IP", NULL };
			"BBSNET \xd7\xaa\xd5\xbe\xc7\xe5\xb5\xa5", "\xb4\xa9\xcb\xf3\xcf\xde\xd6\xc6""ip", "BBSNET2 \xd7\xaa\xd5\xbe\xc7\xe5\xb5\xa5", "\xb4\xa9\xcb\xf3""2\xcf\xde\xd6\xc6""IP", NULL };
	set_user_status(ST_ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	screen_clear();
	move(1, 0);
	//% prints("编修系统档案\n\n");
	prints("\xb1\xe0\xd0\xde\xcf\xb5\xcd\xb3\xb5\xb5\xb0\xb8\n\n");
	for (num = 0; (HAS_PERM(PERM_ESYSFILE)) ? e_file[num] != NULL
			&& explain_file[num] != NULL : strcmp(explain_file[num], "menu.ini"); num++) {
		prints("[\033[1;32m%2d\033[m] %s", num + 1, explain_file[num]);
		if (num < 17)
			move(4 + num, 0);
		else
			move(num - 14, 50);
	}
	//% prints("[\033[1;32m%2d\033[m] 都不想改\n", num + 1);
	prints("[\033[1;32m%2d\033[m] \xb6\xbc\xb2\xbb\xcf\xeb\xb8\xc4\n", num + 1);

	//% getdata(23, 0, "你要编修哪一项系统档案: ", ans, 3, DOECHO, YEA);
	getdata(23, 0, "\xc4\xe3\xd2\xaa\xb1\xe0\xd0\xde\xc4\xc4\xd2\xbb\xcf\xee\xcf\xb5\xcd\xb3\xb5\xb5\xb0\xb8: ", ans, 3, DOECHO, YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n'
			|| ans[0] == '\0')
		return;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	screen_clrtobot();
	//% sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
	sprintf(buf, "(E)\xb1\xe0\xbc\xad (D)\xc9\xbe\xb3\xfd %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		//% sprintf(buf, "你确定要删除 %s 这个系统档", explain_file[ch]);
		sprintf(buf, "\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xc9\xbe\xb3\xfd %s \xd5\xe2\xb8\xf6\xcf\xb5\xcd\xb3\xb5\xb5", explain_file[ch]);
		confirm = askyn(buf, NA, NA);
		if (confirm != 1) {
			move(5, 0);
			//% prints("取消删除行动\n");
			prints("\xc8\xa1\xcf\xfb\xc9\xbe\xb3\xfd\xd0\xd0\xb6\xaf\n");
			pressreturn();
			screen_clear();
			return;
		}
		{
			char secu[STRLEN];
			//% sprintf(secu, "删除系统档案：%s", explain_file[ch]);
			sprintf(secu, "\xc9\xbe\xb3\xfd\xcf\xb5\xcd\xb3\xb5\xb5\xb0\xb8\xa3\xba%s", explain_file[ch]);
			securityreport(secu, 0, 0);
		}
		unlink(buf2);
		move(5, 0);
		//% prints("%s 已删除\n", explain_file[ch]);
		prints("%s \xd2\xd1\xc9\xbe\xb3\xfd\n", explain_file[ch]);
		pressreturn();
		screen_clear();
		return;
	}
	set_user_status(ST_EDITSFILE);
	editor_e status = editor(buf2, false, false, true, NULL);
	screen_clear();
	if (status == EDITOR_SAVE) {
		//% prints("%s 更新过", explain_file[ch]);
		prints("%s \xb8\xfc\xd0\xc2\xb9\xfd", explain_file[ch]);
		{
			char secu[STRLEN];
			//% sprintf(secu, "修改系统档案：%s", explain_file[ch]);
			sprintf(secu, "\xd0\xde\xb8\xc4\xcf\xb5\xcd\xb3\xb5\xb5\xb0\xb8\xa3\xba%s", explain_file[ch]);
			securityreport(secu, 0, 0);
		}

		if (!strcmp(e_file[ch], "../Welcome")) {
			unlink("Welcome.rec");
			//% prints("\nWelcome 记录档更新");
			prints("\nWelcome \xbc\xc7\xc2\xbc\xb5\xb5\xb8\xfc\xd0\xc2");
		} else if (!strcmp(e_file[ch], "whatdate")) {
			brdshm->fresh_date = time(0);
			//% prints("\n纪念日清单 更新");
			prints("\n\xbc\xcd\xc4\xee\xc8\xd5\xc7\xe5\xb5\xa5 \xb8\xfc\xd0\xc2");
		}
	}
	pressreturn();
}

// 全站广播...
int wall() {
	char passbuf[PASSLEN];

	if (!HAS_PERM(PERM_SYSOPS))
		return 0;
	// Added by Ashinmarch on 2008.10.20
	// 全站广播前增加密码验证
	screen_clear();
	//% stand_title("全站广播!");
	stand_title("\xc8\xab\xd5\xbe\xb9\xe3\xb2\xa5!");
	//% getdata(1, 0, "[1;37m请输入密码: [m", passbuf, PASSLEN, NOECHO, YEA);
	getdata(1, 0, "[1;37m\xc7\xeb\xca\xe4\xc8\xeb\xc3\xdc\xc2\xeb: [m", passbuf, PASSLEN, NOECHO, YEA);
	passbuf[8] = '\0';
	if (!passwd_check(currentuser.userid, passbuf)) {
		//% prints("[1;31m密码输入错误...[m\n");
		prints("[1;31m\xc3\xdc\xc2\xeb\xca\xe4\xc8\xeb\xb4\xed\xce\xf3...[m\n");
		return 0;
	}
	// Add end.

	set_user_status(ST_MSG);
	move(2, 0);
	screen_clrtobot();

	char msg[MAX_MSG_SIZE + 2];
	//% if (!get_msg("所有使用者", msg, 1)) {
	if (!get_msg("\xcb\xf9\xd3\xd0\xca\xb9\xd3\xc3\xd5\xdf", msg, 1)) {
		return 0;
	}

	if (!broadcast_msg(msg)) {
		move(2, 0);
		//% prints("线上空无一人\n");
		prints("\xcf\xdf\xc9\xcf\xbf\xd5\xce\xde\xd2\xbb\xc8\xcb\n");
		pressanykey();
	}
	//% prints("\n已经广播完毕...\n");
	prints("\n\xd2\xd1\xbe\xad\xb9\xe3\xb2\xa5\xcd\xea\xb1\xcf...\n");
	pressanykey();
	return 1;
}

// 设定系统密码
int setsystempasswd() {
	FILE *pass;
	char passbuf[20], prepass[20];
	set_user_status(ST_ADMIN);
	if (!check_systempasswd())
		return 0;
	if (strcmp(currentuser.userid, "SYSOP")) {
		screen_clear();
		move(10, 20);
		//% prints("对不起，系统密码只能由 SYSOP 修改！");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb\xd6\xbb\xc4\xdc\xd3\xc9 SYSOP \xd0\xde\xb8\xc4\xa3\xa1");
		pressanykey();
		return 0;
	}
	//% getdata(2, 0, "请输入新的系统密码(直接回车则取消系统密码): ", passbuf, 19, NOECHO, YEA);
	getdata(2, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xd0\xc2\xb5\xc4\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb(\xd6\xb1\xbd\xd3\xbb\xd8\xb3\xb5\xd4\xf2\xc8\xa1\xcf\xfb\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb): ", passbuf, 19, NOECHO, YEA);
	if (passbuf[0] == '\0') {
		//% if (askyn("你确定要取消系统密码吗?", NA, NA) == YEA) {
		if (askyn("\xc4\xe3\xc8\xb7\xb6\xa8\xd2\xaa\xc8\xa1\xcf\xfb\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb\xc2\xf0?", NA, NA) == YEA) {
			unlink("etc/.syspasswd");
			//% securityreport("取消系统密码", 0, 0);
			securityreport("\xc8\xa1\xcf\xfb\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb", 0, 0);
		}
		return 0;
	}
	//% getdata(3, 0, "确认新的系统密码: ", prepass, 19, NOECHO, YEA);
	getdata(3, 0, "\xc8\xb7\xc8\xcf\xd0\xc2\xb5\xc4\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		move(4, 0);
		//% prints("两次密码不相同, 取消此次设定.");
		prints("\xc1\xbd\xb4\xce\xc3\xdc\xc2\xeb\xb2\xbb\xcf\xe0\xcd\xac, \xc8\xa1\xcf\xfb\xb4\xcb\xb4\xce\xc9\xe8\xb6\xa8.");
		pressanykey();
		return 0;
	}
	if ((pass = fopen("etc/.syspasswd", "w")) == NULL) {
		move(4, 0);
		//% prints("系统密码无法设定....");
		prints("\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb\xce\xde\xb7\xa8\xc9\xe8\xb6\xa8....");
		pressanykey();
		return 0;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(4, 0);
	//% prints("系统密码设定完成....");
	prints("\xcf\xb5\xcd\xb3\xc3\xdc\xc2\xeb\xc9\xe8\xb6\xa8\xcd\xea\xb3\xc9....");
	pressanykey();
	return 0;
}

#define DENY_LEVEL_LIST ".DenyLevel"
extern int denylist_key_deal(const char *file, int ch, const char *line);

/**
 * 全站处罚列表标题.
 */
static void denylist_title_show(void)
{
	move(0, 0);
	//% prints("\033[1;44;36m 处罚到期的ID列表\033[K\033[m\n"
	prints("\033[1;44;36m \xb4\xa6\xb7\xa3\xb5\xbd\xc6\xda\xb5\xc4ID\xc1\xd0\xb1\xed\033[K\033[m\n"
			//% " 离开[\033[1;32m←\033[m] 选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] 添加[\033[1;32ma\033[m]  修改[\033[1;32mc\033[m] 恢复[\033[1;32md\033[m] 到期[\033[1;32mx\033[m] 查找[\033[1;32m/\033[m]\n"
			" \xc0\xeb\xbf\xaa[\033[1;32m\xa1\xfb\033[m] \xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m,\033[1;32m\xa1\xfd\033[m] \xcc\xed\xbc\xd3[\033[1;32ma\033[m]  \xd0\xde\xb8\xc4[\033[1;32mc\033[m] \xbb\xd6\xb8\xb4[\033[1;32md\033[m] \xb5\xbd\xc6\xda[\033[1;32mx\033[m] \xb2\xe9\xd5\xd2[\033[1;32m/\033[m]\n"
			//% "\033[1;44m 用户代号     处罚说明(A-Z;'.[])                 权限 结束日期   站务          \033[m\n");
			"\033[1;44m \xd3\xc3\xbb\xa7\xb4\xfa\xba\xc5     \xb4\xa6\xb7\xa3\xcb\xb5\xc3\xf7(A-Z;'.[])                 \xc8\xa8\xcf\xde \xbd\xe1\xca\xf8\xc8\xd5\xc6\xda   \xd5\xbe\xce\xf1          \033[m\n");
}

/**
 * 全站处罚列表入口函数.
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

	//% stand_title("踢使用者下站");
	stand_title("\xcc\xdf\xca\xb9\xd3\xc3\xd5\xdf\xcf\xc2\xd5\xbe");
	move(1, 0);

	char uname[IDLEN + 1];
	//% usercomplete("输入使用者帐号: ", uname);
	usercomplete("\xca\xe4\xc8\xeb\xca\xb9\xd3\xc3\xd5\xdf\xd5\xca\xba\xc5: ", uname);
	if (*uname == '\0') {
		screen_clear();
		return -1;
	}

	user_id_t uid = get_user_id(uname);
	if (!uid) {
		//% presskeyfor("无此用户..", 3);
		presskeyfor("\xce\xde\xb4\xcb\xd3\xc3\xbb\xa7..", 3);
		screen_clear();
		return 0;
	}

	screen_move_clear(1);
	char buf[STRLEN];
	//% snprintf(buf, sizeof(buf), "踢掉使用者 : [%s].", uname);
	snprintf(buf, sizeof(buf), "\xcc\xdf\xb5\xf4\xca\xb9\xd3\xc3\xd5\xdf : [%s].", uname);
	move(2, 0);
	if (!askyn(buf, NA, NA)) {
		//% presskeyfor("取消踢使用者..", 2);
		presskeyfor("\xc8\xa1\xcf\xfb\xcc\xdf\xca\xb9\xd3\xc3\xd5\xdf..", 2);
		screen_clear();
		return 0;
	}

	session_basic_info_t *res = get_sessions(uid);
	if (res && session_basic_info_count(res) > 0) {
		for (int i = 0; i < session_basic_info_count(res); ++i) {
			bbs_kill(session_basic_info_sid(res, i),
					session_basic_info_pid(res, i), SIGHUP);
		}
		//% presskeyfor("该用户已经被踢下站", 4);
		presskeyfor("\xb8\xc3\xd3\xc3\xbb\xa7\xd2\xd1\xbe\xad\xb1\xbb\xcc\xdf\xcf\xc2\xd5\xbe", 4);
	} else {
		move(3, 0);
		//% presskeyfor("该用户不在线上或无法踢出站外..", 3);
		presskeyfor("\xb8\xc3\xd3\xc3\xbb\xa7\xb2\xbb\xd4\xda\xcf\xdf\xc9\xcf\xbb\xf2\xce\xde\xb7\xa8\xcc\xdf\xb3\xf6\xd5\xbe\xcd\xe2..", 3);
	}

	session_basic_info_clear(res);
	screen_clear();
	return 0;
}

static void filter_post(query_t *q, const post_filter_t *filter)
{
	query_where(q, "TRUE");
	if (filter->uid)
		query_and(q, "user_id = %"DBIdUID, filter->uid);
	if (*filter->utf8_keyword)
		query_and(q, "title ILIKE '%%' || %s || '%%'", filter->utf8_keyword);
	if (filter->min)
		query_and(q, "id >= %"DBIdPID, filter->min);
}

static void search_all_boards(post_filter_t *filter, const char *user_name,
		int days, bool remove)
{
	char file[HOMELEN];
	file_temp_name(file, sizeof(file));
	FILE *fp = fopen(file, "w");
	if (!fp)
		return;

	GBK_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
	convert_u2g(filter->utf8_keyword, gbk_keyword);
	//% 在所有板查询%s网友%d天以内的大作, 关键字'%s'
	fprintf(fp, "\xd4\xda\xcb\xf9\xd3\xd0\xb0\xe5\xb2\xe9\xd1\xaf%s"
			"\xcd\xf8\xd3\xd1%d\xcc\xec\xd2\xd4\xc4\xda\xb5\xc4"
			"\xb4\xf3\xd7\xf7, \xb9\xd8\xbc\xfc\xd7\xd6'%s'.\n\n",
			user_name, days, gbk_keyword);

	filter->min = post_id_from_stamp(fb_time() - days * 24 * 60 * 60);

	query_t *q = query_new(0);
	query_select(q, "id, board_id, title");
	query_from(q, "post.recent");
	filter_post(q, filter);
	query_orderby(q, "id", true);

	db_res_t *res = query_exec(q);

	if (remove)
		post_delete(filter, true, false, false);

	int matched = 0;
	if (res && db_res_rows(res) > 0) {
		matched = db_res_rows(res);
		for (int i = matched - 1; i >= 0; --i) {
			GBK_BUFFER(title, POST_TITLE_CCHARS);
			const char *utf8_title = db_get_value(res, i, 2);
			convert_u2g(utf8_title, gbk_title);

			char date[26];
			time_t stamp = post_stamp(db_get_post_id(res, i, 0));
			ctime_r(&stamp, date);
			date[24] = '\0';
			fprintf(fp, " %s [%d] %s\n", date, db_get_integer(res, i, 1),
					gbk_title);
		}
	}
	db_clear(res);

	fprintf(fp, "%d matched found.\n", matched);
	fclose(fp);

	char buf[80];
	//% [%s]查询%s在%d天内关键字'%.10s'
	snprintf(buf, sizeof(buf), "[%s]\xb2\xe9\xd1\xaf%s\xd4\xda%d\xcc\xec"
			"\xc4\xda\xb9\xd8\xbc\xfc\xd7\xd6'%.10s'",
			//% 版面
			is_deleted(filter->type) ? "Deleted" : "\xb0\xe6\xc3\xe6",
			user_name, days, gbk_keyword);
	mail_file(file, currentuser.userid, buf);
	unlink(file);
}

int tui_search_all_boards(void)
{
	post_filter_t filter = { .bid = 0 };

	set_user_status(ST_QUERY);

	char user_name[IDLEN + 1];
	screen_clear();
	//% 请输入您想查询的作者帐号
	usercomplete("\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xcf\xeb\xb2\xe9\xd1\xaf"
			"\xb5\xc4\xd7\xf7\xd5\xdf\xd5\xca\xba\xc5: ", user_name);

	if (!*user_name) {
		move(1, 0);
		//% 查询所有的作者吗
		if (!askyn("\xb2\xe9\xd1\xaf\xcb\xf9\xd3\xd0\xb5\xc4\xd7\xf7\xd5\xdf"
					"\xc2\xf0?", true, false))
			return 0;
	} else {
		filter.uid = get_user_id(user_name);
		if (!filter.uid) {
			//% 不正确的使用者代号
			prints("\xb2\xbb\xd5\xfd\xc8\xb7\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf"
					"\xb4\xfa\xba\xc5\n");
			pressreturn();
			return 0;
		}
	}

	GBK_BUFFER(keyword, POST_LIST_KEYWORD_LEN);
	//% 请输入文章标题关键字
	getdata(2, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xce\xc4\xd5\xc2\xb1\xea\xcc\xe2"
			"\xb9\xd8\xbc\xfc\xd7\xd6: ", gbk_keyword, sizeof(gbk_keyword),
			DOECHO, YEA);
	if (*gbk_keyword) {
		convert_g2u(gbk_keyword, filter.utf8_keyword);
	}

	char buf[4];
	//% 查询距今多少天以内的文章
	getdata(3, 0, "\xb2\xe9\xd1\xaf\xbe\xe0\xbd\xf1\xb6\xe0\xc9\xd9\xcc\xec"
			"\xd2\xd4\xc4\xda\xb5\xc4\xce\xc4\xd5\xc2?: ", buf, sizeof(buf),
			DOECHO, YEA);
	int days = strtol(buf, NULL, 10);
	if (days <= 0)
		return 0;

	bool remove = false;
	move(5, 0);
	//% 是否删除文章
	remove = askyn("\xca\xc7\xb7\xf1\xc9\xbe\xb3\xfd\xce\xc4\xd5\xc2?",
			false, false);

	search_all_boards(&filter, user_name, days, remove);
	//% 网友大作查询
	report("\xcd\xf8\xd3\xd1\xb4\xf3\xd7\xf7\xb2\xe9\xd1\xaf",
			currentuser.userid);
	return 0;
}
