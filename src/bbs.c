#include "bbs.h"
#include <time.h>
#if defined(BSD44)
#include <stdlib.h>
#elif defined(LINUX)
/* include nothing :-) */
#else
#include <rpcsvc/rstat.h>
#endif
#include "mmap.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/brc.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/msg.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"
#include "fbbs/tui_list.h"

#ifndef DLM
#undef	ALLOWGAME
#endif

extern sigjmp_buf bus_jump;

//用于保存版主对精华区操作记录的文件名,硬盘上的位置是logs/boardname
char ANN_LOG_PATH[256];

int digestmode;
int usernum = 0;
char currBM[BM_LEN - 1];
char someoneID[31];
char topic[STRLEN] = "";
char genbuf[1024];
char quote_file[120], quote_user[120];
#ifndef NOREPLY
char replytitle[STRLEN];
int o_id = 0;
int o_gid = 0;
#endif

int	getlist(char *, char **, int, char **, int, char **, int, char **,
				int);
char *filemargin();
void board_usage();
void canceltotrash();
int thesis_mode();

/*For read.c*/
int auth_search_down();
int auth_search_up();
int post_search_down();
int post_search_up();
int thread_up();
int thread_down();
int deny_user();
int club_user();
int show_author();
int SR_first();
int SR_read();
int SR_read();
int SR_author();
int SR_BMfunc();
int Q_Goodbye();
int show_online_followings(void);
int b_notes_passwd();
int post_cross(char islocal, int mod);
int lock();
extern int x_lockscreen();
extern time_t login_start_time;
extern char BoardName[];
extern char fromhost[];

//  取得用户ID为userid的用户信息,保存在currentuser中
static int getcurrentuser(char *userid)
{
	int uid = searchuser(userid);
	if (uid == 0)
		return 0;
	//get_record(PASSFILE, &lookupuser, sizeof(lookupuser), uid);
	memcpy(&currentuser, &(uidshm->passwd[uid-1]), sizeof(currentuser));
	return uid;
}

//取得用户信息,不成功返回-1
int set_safe_record(void)
{
	if (getcurrentuser(currentuser.userid) == 0)
		return -1;
	return 0;
}

/*
 Commented by Erebus 2004-11-03
 input: userid
 output: path such as "home/a/abc" when userid="abc"
 */
char *sethomepath(char *buf, const char *userid)
{
	sprintf(buf, "home/%c/%s", toupper(userid[0]), userid);
	return buf;
}

/*
 Commented by Erebus 2004-11-03
 copy the title from stitle to topic
 ignore string "Re: "
 */
/*Add by SmallPig*/
//Modified by IAMFAT 2002-05-27
//      取得stitle字符串所表示的标题,保存在全局变量topic中返回
//              如果是回复文章,去掉前面的 "Re: "这4个字符
void setqtitle(char *stitle, int gid)
{
	o_gid = gid;
	if (strncmp(stitle, "Re: ", 4) != 0)
		//commented by iamfat 2002.07.26
		//&& strncmp (stitle, "RE: ", 4) != 0)
		strcpy(topic, stitle);
	else
		strcpy(topic, stitle + 4);
}

/*
 Commented by Erebus 2004-11-03
 check whether the current user is BM	
 */
int chk_currBM(char BMstr[BM_LEN-1], int isclub)
//STRLEN --> BM_LEN  changed by iamfat 2002.07.26
{
	char *ptr;
	char BMstrbuf[BM_LEN - 1];

	if (isclub) {
		if (HAS_PERM(PERM_OCLUB))
			return YEA;
	} else {
		if (HAS_PERM(PERM_BLEVELS))
			return YEA;
	}

	if (!HAS_PERM(PERM_BOARDS))
		return NA;

	strcpy(BMstrbuf, BMstr);
	ptr = strtok(BMstrbuf, ",: ;|&()\0\n");
	while (ptr) {
		if (!strcmp(ptr, currentuser.userid))
			return YEA;
		ptr = strtok(NULL, ",: ;|&()\0\n");
	}
	return NA;
}

// set quotefile as filepath
void setquotefile(const char *filepath)
{
	strcpy(quote_file, filepath);
}

//返回用户主目录下所在的文件filename路径
char *setuserfile(char *buf, char *filename) {
	sprintf(buf, "home/%c/%s/%s", toupper(currentuser.userid[0]),
			currentuser.userid, filename);
	return buf;
}

char *setbdir(char *buf, const char *boardname)
{
	const char *dir;
	switch (digestmode) {
		case YEA:
			dir = DIGEST_DIR;
			break;
		case 2:
			dir = THREAD_DIR;
			break;
		case 3:
			dir = MARKED_DIR;
			break;
		case 4:
			dir = AUTHOR_DIR;
			break;
			//      case 5:                 /* 同作者 */
			//      case 6:                 /* 同作者 */
			//      case 7:                 /* 标题关键字 */
		case TRASH_MODE:
			dir = TRASH_DIR;
			break;
		case JUNK_MODE:
			dir = JUNK_DIR;
			break;
#ifdef ENABLE_NOTICE
		case NOTICE_MODE:
			dir = NOTICE_DIR;
			break;
#endif
		default:
			dir = DOT_DIR;
			break;
	}

	if (digestmode == 5 || digestmode == 6)
		sprintf(buf, "boards/%s/SOMEONE.%s.DIR.%d", boardname, someoneID,
				digestmode - 5);
	else if (digestmode == 7)
		sprintf(buf, "boards/%s/KEY.%s.DIR", boardname, currentuser.userid);
	else
		sprintf(buf, "boards/%s/%s", boardname, dir);
	return buf;
}

int shownotepad(void)
{
	set_user_status(ST_NOTEPAD);
	ansimore("etc/notepad", YEA);
	return 0;
}

void Poststring(const char *str, const char *nboard, const char *posttitle,
		int mode)
{
	FILE *se;
	char fname[STRLEN];

	int status = session.status;
	sprintf(fname, "tmp/AutoPoster.%s.%05d", currentuser.userid, session.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
		Postfile(fname, nboard, posttitle, mode);
		unlink(fname);
		set_user_status(status);
	}
}

int cmpfilename(void *fhdr, void *filename)
{
	if (!strncmp(((struct fileheader *)fhdr)->filename,
			(char *)filename, STRLEN))
		return 1;
	return 0;
}

static int cmpdigestfilename(void *digest_name, void *fhdr)
{
	if (!strcmp(((struct fileheader *)fhdr)->filename, (char *)digest_name))
		return 1;
	return 0;
} /* comapare file names for dele_digest function. Luzi 99.3.30 */

int tui_select_board(int current_bid)
{
	set_user_status(ST_SELECT);

	move(0, 0);
	clrtoeol();
	//% prints("选择一个讨论区 (英文字母大小写皆可)\n");
	prints("\xd1\xa1\xd4\xf1\xd2\xbb\xb8\xf6\xcc\xd6\xc2\xdb\xc7\xf8 (\xd3\xa2\xce\xc4\xd7\xd6\xc4\xb8\xb4\xf3\xd0\xa1\xd0\xb4\xbd\xd4\xbf\xc9)\n");

	char bname[BOARD_NAME_LEN];
	//% board_complete(1, "输入讨论区名 (按空白键自动搜寻): ",
	board_complete(1, "\xca\xe4\xc8\xeb\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb (\xb0\xb4\xbf\xd5\xb0\xd7\xbc\xfc\xd7\xd4\xb6\xaf\xcb\xd1\xd1\xb0): ",
			bname, sizeof(bname), AC_LIST_BOARDS_ONLY);
	if (*bname == '\0')
		return 0;

	board_t board;
	if (!get_board(bname, &board))
		return 0;

	if (!has_read_perm(&currentuser, &board)) {
		clear();
		move(5, 10);
		//% prints("您不是俱乐部版 %s 的成员，无权进入该版", bname);
		prints("\xc4\xfa\xb2\xbb\xca\xc7\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6 %s \xb5\xc4\xb3\xc9\xd4\xb1\xa3\xac\xce\xde\xc8\xa8\xbd\xf8\xc8\xeb\xb8\xc3\xb0\xe6", bname);
		pressanykey();
		return 0;
	}

	move(0, 0);
	clrtoeol();
	move(1, 0);
	clrtoeol();

	set_user_status(ST_READING);
	if (board.id != current_bid) {
		brc_update(currentuser.userid, currboard);
		brc_initial(currentuser.userid, bname);
		change_board(&board);
		set_current_board(board.id);
		return board.id;
	}
	return 0;
}

int board_select(void)
{
	tui_select_board(0);
	return 0;
}

void dele_digest(char *dname, char *direc) {
	char digest_name[STRLEN];
	char new_dir[STRLEN];
	char buf[STRLEN];
	char *ptr;
	struct fileheader fh;
	int pos;

	strlcpy(digest_name, dname, STRLEN);
	strcpy(new_dir, direc);
	digest_name[0] = 'G';
	ptr = strrchr(new_dir, '/') + 1;
	strcpy(ptr, DIGEST_DIR);
	pos = search_record(new_dir, &fh, sizeof (fh), cmpdigestfilename,
			digest_name);
	if (pos <= 0) {
		return;
	}
	delete_record(new_dir, sizeof(struct fileheader), pos, cmpfilename,
			digest_name);
	*ptr = '\0';
	sprintf(buf, "%s%s", new_dir, digest_name);
	unlink(buf);
	return;
}

int digest_post(int ent, struct fileheader *fhdr, char *direct) {
	struct fileheader chkfileinfo; // add by quickmouse 01-05-30 检查一下 避免出现.DIR破坏

	if (!am_curr_bm()) {
		return DONOTHING;
	}
	if (digestmode == YEA)
		return DONOTHING;
#ifdef ENABLE_NOTICE
	if (fhdr->accessed[1] & FILE_NOTICE) {
		return DONOTHING;
	}
#endif
	if (fhdr->accessed[0] & FILE_DIGEST) {
		fhdr->accessed[0] = (fhdr->accessed[0] & ~FILE_DIGEST);
		dele_digest(fhdr->filename, direct);
		bm_log(currentuser.userid, currboard, BMLOG_UNDIGIST, 1);
	} else {
		struct fileheader digest;
		char *ptr, buf[64];

		memcpy(&digest, fhdr, sizeof (digest));
		digest.filename[0] = 'G';
		strcpy(buf, direct);
		ptr = strrchr(buf, '/') + 1;
		ptr[0] = '\0';
		sprintf(genbuf, "%s%s", buf, digest.filename);
		if (dashf(genbuf)) {
			fhdr->accessed[0] = fhdr->accessed[0] | FILE_DIGEST;
			if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo),
					ent, 1) != 1) // add by quickmouse 01-05-30
			{
				return DONOTHING;
			}
			if (strcmp(fhdr->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
			{
				return DONOTHING;
			}
			substitute_record(direct, fhdr, sizeof (*fhdr), ent);
			return PARTUPDATE;
		}
		digest.accessed[0] = 0;
		sprintf(&genbuf[512], "%s%s", buf, fhdr->filename);
		link(&genbuf[512], genbuf);
		strcpy(ptr, DIGEST_DIR);
		if (get_num_records(buf, sizeof (digest)) >= MAX_DIGEST) {
			move(3, 0);
			clrtobot();
			move(4, 10);
			//% prints("抱歉，您的文摘文章已经超过 %d 篇，无法再加入...\n", MAX_DIGEST);
			prints("\xb1\xa7\xc7\xb8\xa3\xac\xc4\xfa\xb5\xc4\xce\xc4\xd5\xaa\xce\xc4\xd5\xc2\xd2\xd1\xbe\xad\xb3\xac\xb9\xfd %d \xc6\xaa\xa3\xac\xce\xde\xb7\xa8\xd4\xd9\xbc\xd3\xc8\xeb...\n", MAX_DIGEST);
			pressanykey();
			return PARTUPDATE;
		}
		append_record(buf, &digest, sizeof (digest));
		fhdr->accessed[0] = fhdr->accessed[0] | FILE_DIGEST;
		fhdr->accessed[0] &= ~FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_DIGIST, 1);
	}
	if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo), ent, 1)
			!= 1) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	if (strcmp(fhdr->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	substitute_record(direct, fhdr, sizeof (*fhdr), ent);
	return PARTUPDATE;
}

#ifndef NOREPLY
int do_reply(struct fileheader *fh) {
	strcpy(replytitle, fh->title);
	o_id = fh->id;
	o_gid = fh->gid;
//	post_article(currboard, fh->owner);
	replytitle[0] = '\0';
	return FULLUPDATE;
}
#endif

/**
 * Quote in given mode.
 * @param orig The file to be quoted.
 * @param file The output file.
 * @param mode Quote mode (R/S/Y/A/N).
 */
// TODO: do not use quote_file in callers.
void do_quote(const char *orig, const char *file, char mode, bool anony)
{
	bool mail = strneq(orig, "mail", 4);
	quote_file_(orig, file, mode, mail, NULL);

	FILE *fp = fopen(file, "a");
	if (!fp)
		return;
	if (currentuser.signature && !anony)
		add_signature(fp, currentuser.userid, currentuser.signature);
	fclose(fp);
}

static void getcross(board_t *board, const char *input, const char *output,
		int mode, struct postheader *header)
{
	FILE *inf, *of;
	char buf[256];
	char owner[STRLEN];
	int owner_found = 0;
	char *p = NULL;
	time_t now;

	now = time(0);
	inf = fopen(input, "r");
	of = fopen(output, "w");
	if (inf == NULL || of == NULL) {
		report("Cross Post error", currentuser.userid);
		return;
	}
	if (mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN) {
		if (in_mail) {
			in_mail = NA;
			write_header(of, header);
			in_mail = YEA;
		} else
			write_header(of, header);

		//% if (fgets(buf, 256, inf) && (p = strstr(buf, "信人: "))) {
		if (fgets(buf, 256, inf) && (p = strstr(buf, "\xd0\xc5\xc8\xcb: "))) {
			p += 6;
			strtok(p, " \n\r");
			strcpy(owner, p);
			owner_found = 1;
		} else {
			owner_found = 0;
			strcpy(owner, "Unkown User");
		}

		if (in_mail)
			//% fprintf(of, "\033[1;37m【 以下文字转载自 \033[32m%s \033[37m的信箱 】\033[m\n",
			fprintf(of, "\033[1;37m\xa1\xbe \xd2\xd4\xcf\xc2\xce\xc4\xd7\xd6\xd7\xaa\xd4\xd8\xd7\xd4 \033[32m%s \033[37m\xb5\xc4\xd0\xc5\xcf\xe4 \xa1\xbf\033[m\n",
					currentuser.userid);
		else {
			//% fprintf(of, "\033[1;37m【 以下文字转载自 \033[32m%s \033[37m%s区 】\033[m\n",
			fprintf(of, "\033[1;37m\xa1\xbe \xd2\xd4\xcf\xc2\xce\xc4\xd7\xd6\xd7\xaa\xd4\xd8\xd7\xd4 \033[32m%s \033[37m%s\xc7\xf8 \xa1\xbf\033[m\n",
					((board->flag & BOARD_POST_FLAG) || (board->perm == 0)) ? board->name
							//% : "未知", mode ? "精华" : "讨论");
							: "\xce\xb4\xd6\xaa", mode ? "\xbe\xab\xbb\xaa" : "\xcc\xd6\xc2\xdb");
		}

		if (owner_found) {
			while (fgets(buf, 256, inf) && buf[0] != '\n')
				;
			//% fprintf(of, "\033[1m【 原文由\033[32m %s\033[37m 所发表 】"
			fprintf(of, "\033[1m\xa1\xbe \xd4\xad\xce\xc4\xd3\xc9\033[32m %s\033[37m \xcb\xf9\xb7\xa2\xb1\xed \xa1\xbf"
					"\033[m\n\n", owner);
		} else
			fseek(inf, (long) 0, SEEK_SET);
	} else if (mode == POST_FILE_DELIVER) {
		//% fprintf(of, "\033[1;33m发信人: deliver (自动发信系统), "
		fprintf(of, "\033[1;33m\xb7\xa2\xd0\xc5\xc8\xcb: deliver (\xd7\xd4\xb6\xaf\xb7\xa2\xd0\xc5\xcf\xb5\xcd\xb3), "
				//% "信区: %s\033[m\n", board->name);
				"\xd0\xc5\xc7\xf8: %s\033[m\n", board->name);
		//% fprintf(of, "标  题: %s\n", header->title);
		fprintf(of, "\xb1\xea  \xcc\xe2: %s\n", header->title);
		//% fprintf(of, "发信站: %s自动发信系统 (%s)\n\n", BoardName,
		fprintf(of, "\xb7\xa2\xd0\xc5\xd5\xbe: %s\xd7\xd4\xb6\xaf\xb7\xa2\xd0\xc5\xcf\xb5\xcd\xb3 (%s)\n\n", BoardName,
				getdatestring(now, DATE_ZH));
	} else if (mode == POST_FILE_BMS) {
		//% fprintf(of, "\033[1;33m发信人: BMS (版主管理员), 信区: %s\033[m\n",
		fprintf(of, "\033[1;33m\xb7\xa2\xd0\xc5\xc8\xcb: BMS (\xb0\xe6\xd6\xf7\xb9\xdc\xc0\xed\xd4\xb1), \xd0\xc5\xc7\xf8: %s\033[m\n",
				board->name);
		//% fprintf(of, "标  题: %s\n", header->title);
		fprintf(of, "\xb1\xea  \xcc\xe2: %s\n", header->title);
		//% fprintf(of, "发信站: %s自动发信系统 (%s)\n\n", BoardName,
		fprintf(of, "\xb7\xa2\xd0\xc5\xd5\xbe: %s\xd7\xd4\xb6\xaf\xb7\xa2\xd0\xc5\xcf\xb5\xcd\xb3 (%s)\n\n", BoardName,
				getdatestring(now, DATE_ZH));
	} else if (mode == POST_FILE_AUTO) {
		write_header(of, header);
	}
	while (fgets(buf, 256, inf) != NULL) {
		fprintf(of, "%s", buf);
	}
	fclose(inf);
	fclose(of);
	*quote_file = '\0';
}

int post_reply(const char *owner, const char *title, const char *file)
{
	char uid[STRLEN];

	int status = session.status;

	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;
	clear();
	// Added by Amigo 2002.06.10. To add mail right check.
	if (!HAS_PERM(PERM_MAIL)) {
		move(4, 0);
		//% prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		prints("\n\n        \xc4\xfa\xc9\xd0\xce\xb4\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa3\xac\xbb\xf2\xd5\xdf\xb7\xa2\xcb\xcd\xd0\xc5\xbc\xfe\xb5\xc4\xc8\xa8\xcf\xde\xb1\xbb\xb7\xe2\xbd\xfb\xa1\xa3");
		pressreturn();
		return FULLUPDATE;
	}
	// Added end.
	if (check_maxmail()) {
		pressreturn();
		return FULLUPDATE;
	}
	set_user_status(ST_SMAIL);

	// indicate the quote file/user
	strlcpy(quote_file, file, sizeof(quote_file));
	strcpy(quote_user, owner);

	if (!getuser(quote_user)) {
		//% prints("对不起，该帐号已经不存在!\n");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xb8\xc3\xd5\xca\xba\xc5\xd2\xd1\xbe\xad\xb2\xbb\xb4\xe6\xd4\xda!\n");
		pressreturn();
	} else
		strcpy(uid, quote_user);

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	snprintf(gbk_title, sizeof(gbk_title), "%s%s",
			strncmp(title, "Re: ", 4) ? "Re: " : "", title);

	switch (do_send(uid, title)) {
		case -1:
			//% prints("系统无法送信\n");
			prints("\xcf\xb5\xcd\xb3\xce\xde\xb7\xa8\xcb\xcd\xd0\xc5\n");
			break;
		case -2:
			//% prints("送信动作已经中止\n");
			prints("\xcb\xcd\xd0\xc5\xb6\xaf\xd7\xf7\xd2\xd1\xbe\xad\xd6\xd0\xd6\xb9\n");
			break;
		case -3:
			//% prints("使用者 '%s' 无法收信\n", uid);
			prints("\xca\xb9\xd3\xc3\xd5\xdf '%s' \xce\xde\xb7\xa8\xca\xd5\xd0\xc5\n", uid);
			break;
		case -4:
			//% prints("使用者 '%s' 无法收信，信箱已满\n", uid);
			prints("\xca\xb9\xd3\xc3\xd5\xdf '%s' \xce\xde\xb7\xa8\xca\xd5\xd0\xc5\xa3\xac\xd0\xc5\xcf\xe4\xd2\xd1\xc2\xfa\n", uid);
			break;
		default:
			//% prints("信件已成功地寄给原作者 %s\n", uid);
			prints("\xd0\xc5\xbc\xfe\xd2\xd1\xb3\xc9\xb9\xa6\xb5\xd8\xbc\xc4\xb8\xf8\xd4\xad\xd7\xf7\xd5\xdf %s\n", uid);
	}
	pressreturn();
	set_user_status(status);
	in_mail = NA;
	return FULLUPDATE;
}

static int undelcheck(void *fh1, void *fh2)
{
	return (atoi(((struct fileheader *)fh1)->filename + 2)
		> atoi(((struct fileheader *)fh2)->filename + 2));
}

int date_to_fname(char *postboard, time_t now, char *fname) {
	static unsigned ref = 0;
	
	sprintf(fname, "M.%ld.%X%X", now, session.pid, ref);
	ref++;
	return 0;
}

static post_id_t post_cross_legacy(board_t *board, const char *file,
		const char *title, int mode)
{
	if ((mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN)
			&& !has_post_perm(&currentuser, board)) {
		//% prints("\n\n 您尚无权限在 %s 版发表文章.\n", currboard);
		prints("\n\n \xc4\xfa\xc9\xd0\xce\xde\xc8\xa8\xcf\xde\xd4\xda %s \xb0\xe6\xb7\xa2\xb1\xed\xce\xc4\xd5\xc2.\n", currboard);
		return -1;
	}

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	if (mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN) {
		//% if (!strneq(title, "[转载]", 6) && !strneq(title, "Re: [转载]", 10))
		if (!strneq(title, "[\xd7\xaa\xd4\xd8]", 6) && !strneq(title, "Re: [\xd7\xaa\xd4\xd8]", 10))
			//% snprintf(gbk_title, sizeof(gbk_title), "[转载]%.70s", title);
			snprintf(gbk_title, sizeof(gbk_title), "[\xd7\xaa\xd4\xd8]%.70s", title);
		//% else if (strneq(title, "Re: [转载]", 10))
		else if (strneq(title, "Re: [\xd7\xaa\xd4\xd8]", 10))
			//% snprintf(gbk_title, sizeof(gbk_title), "[转载]Re: %.70s",
			snprintf(gbk_title, sizeof(gbk_title), "[\xd7\xaa\xd4\xd8]Re: %.70s",
					title + 10);
		else
			strlcpy(gbk_title, title, sizeof(gbk_title));
	} else {
		strlcpy(gbk_title, title, sizeof(gbk_title));
	}

	valid_title(gbk_title);

	struct postheader header = {
		.locked = mode == POST_FILE_DELIVER ||
			//% (mode == POST_FILE_AUTO && strneq(gbk_title, "[合集]", 6)),
			(mode == POST_FILE_AUTO && strneq(gbk_title, "[\xba\xcf\xbc\xaf]", 6)),
		.postboard = true,
	};
	strlcpy(header.title, gbk_title, sizeof(header.title));
	strlcpy(header.ds, board->name, sizeof(header.ds));

	char output[HOMELEN];
	snprintf(output, sizeof(output), "tmp/cp_buffer.%d", getpid());
	getcross(board, file, output, mode, &header);
	if (mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN)
		add_crossinfo(file, 1);

	const char *uname;
	if (mode == POST_FILE_DELIVER)
		uname = "deliver";
	else if (mode == POST_FILE_BMS)
		uname = "BMS";
	else
		uname = currentuser.userid;

	bool autopost = mode == POST_FILE_DELIVER || mode == POST_FILE_AUTO
	            || mode == POST_FILE_BMS;
	post_request_t req = {
		.autopost = autopost,
		.crosspost = mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN,
		.uname = uname,
		.nick = currentuser.username,
		.user = autopost ? NULL : &currentuser,
		.board = board,
		.title = gbk_title,
		.content = NULL,
		.gbk_file = output,
		.sig = 0,
		.ip = NULL,
		.reid = 0,
		.tid = 0,
		.locked = mode == POST_FILE_DELIVER ||
			//% (mode == POST_FILE_AUTO && strneq(gbk_title, "[合集]", 6)),
			(mode == POST_FILE_AUTO && strneq(gbk_title, "[\xba\xcf\xbc\xaf]", 6)),
		.marked = false,
		.anony = false,
		.cp = NULL,
	};

	post_id_t pid = publish_post(&req);
	
	char buf[STRLEN];
	if (!pid) {
		if (mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN) {
			snprintf(buf, sizeof(buf),
					"cross_posting '%s' on %s: append_record failed!",
					gbk_title, board->name);
		} else {
			snprintf(buf, sizeof(buf),
					"Posting '%s' on %s: append_record failed!",
					gbk_title, board->name);
		}
		report(buf, currentuser.userid);
		pressreturn();
		clear();
	} else {
		if (mode == POST_FILE_NORMAL || mode == POST_FILE_CP_ANN) {
			snprintf(buf, sizeof(buf), "cross_posted '%s' on %s",
					gbk_title, board->name);
			report(buf, currentuser.userid);
		}
	}
	unlink(output);
	return pid;
}

post_id_t Postfile(const char *file, const char *bname, const char *title,
		post_file_e mode)
{
	board_t board;
	if (!get_board(bname, &board))
		return 0;

	return post_cross_legacy(&board, file, title, mode);
}

int tui_cross_post_legacy(const char *file, const char *title)
{
	clear();
	//% prints("您选择转载的文章是: [\033[1;33m%s\033[m]\n", title);
	prints("\xc4\xfa\xd1\xa1\xd4\xf1\xd7\xaa\xd4\xd8\xb5\xc4\xce\xc4\xd5\xc2\xca\xc7: [\033[1;33m%s\033[m]\n", title);

	char bname[STRLEN];
	//% board_complete(1, "请输入要转贴的讨论区名称(取消转载请按回车): ",
	board_complete(1, "\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xd7\xaa\xcc\xf9\xb5\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6(\xc8\xa1\xcf\xfb\xd7\xaa\xd4\xd8\xc7\xeb\xb0\xb4\xbb\xd8\xb3\xb5): ",
			bname, sizeof(bname), AC_LIST_BOARDS_ONLY);
	if (!*bname)
		return FULLUPDATE;

	if (streq(bname, currboard) && session.status != ST_RMAIL) {
		//% prints("\n\n             很抱歉，您不能把文章转到同一个版上。");
		prints("\n\n             \xba\xdc\xb1\xa7\xc7\xb8\xa3\xac\xc4\xfa\xb2\xbb\xc4\xdc\xb0\xd1\xce\xc4\xd5\xc2\xd7\xaa\xb5\xbd\xcd\xac\xd2\xbb\xb8\xf6\xb0\xe6\xc9\xcf\xa1\xa3");
		pressreturn();
		clear();
		return FULLUPDATE;
	}

	move(3, 0);
	clrtobot();
	//% prints("转载 ' %s ' 到 %s 版 ", title, bname);
	prints("\xd7\xaa\xd4\xd8 ' %s ' \xb5\xbd %s \xb0\xe6 ", title, bname);
	move(4, 0);
	char ans[10];
	//% getdata(4, 0, "(S)转信 (L)本站 (A)取消? [A]: ", ans, 9, DOECHO, YEA);
	getdata(4, 0, "(S)\xd7\xaa\xd0\xc5 (L)\xb1\xbe\xd5\xbe (A)\xc8\xa1\xcf\xfb? [A]: ", ans, 9, DOECHO, YEA);
	if (ans[0] == 's' || ans[0] == 'S' || ans[0] == 'L' || ans[0] == 'l') {
		board_t brd;
		if (!get_board(bname, &brd) ||
				!post_cross_legacy(&brd, file, title, POST_FILE_NORMAL)) {
			pressreturn();
			move(2, 0);
			return FULLUPDATE;
		}
		//% prints("\n已把文章 \'%s\' 转贴到 %s 版\n", title, bname);
		prints("\n\xd2\xd1\xb0\xd1\xce\xc4\xd5\xc2 \'%s\' \xd7\xaa\xcc\xf9\xb5\xbd %s \xb0\xe6\n", title, bname);
	} else {
		//% prints("取消");
		prints("\xc8\xa1\xcf\xfb");
	}
	move(2, 0);
	pressreturn();
	return FULLUPDATE;
}

void add_crossinfo(const char *filepath, bool post)
{
	int color = (currentuser.numlogins % 7) + 31;
	FILE *fp = fopen(filepath, "a");
	if (fp) {
		//% fprintf(fp, "--\n\033[m\033[1;%2dm※ 转%s:·%s %s·[FROM: %s]\033[m\n",
		fprintf(fp, "--\n\033[m\033[1;%2dm\xa1\xf9 \xd7\xaa%s:\xa1\xa4%s %s\xa1\xa4[FROM: %s]\033[m\n",
				//% color, post ? "载" : "寄", BoardName, BBSHOST,
				color, post ? "\xd4\xd8" : "\xbc\xc4", BoardName, BBSHOST,
				mask_host(fromhost));
		fclose(fp);
	}
}

// 显示版面提示
int show_board_notes(const char *bname, int command)
{
	char buf[256];

	sprintf(buf, "vote/%s/notes", bname);
	if (dashf(buf)) {
		if (command == 1) {
			ansimore2(buf, NA, 0, 19);
		} else {
			ansimore(buf, YEA);
		}
		return 1;
	}
	strcpy(buf, "vote/notes");
	if (dashf(buf)) {
		if (command == 1) {
			ansimore2(buf, NA, 0, 19);
		} else {
			ansimore(buf, YEA);
		}
		return 1;
	}
	return -1;
}

int show_user_notes() {
	char buf[256];

	setuserfile(buf, "notes");
	if (dashf(buf)) {
		ansimore(buf, YEA);
		return FULLUPDATE;
	}
	clear();
	move(10, 15);
	//% prints("您尚未在 InfoEdit->WriteFile 编辑个人备忘录。\n");
	prints("\xc4\xfa\xc9\xd0\xce\xb4\xd4\xda InfoEdit->WriteFile \xb1\xe0\xbc\xad\xb8\xf6\xc8\xcb\xb1\xb8\xcd\xfc\xc2\xbc\xa1\xa3\n");
	pressanykey();
	return FULLUPDATE;
}

int IsTheFileOwner(struct fileheader *fileinfo) {
	char buf[512];
	int posttime;

	if (fileinfo->owner[0] == '-' || strstr(fileinfo->owner, "."))
		return 0;
	if (strcmp(currentuser.userid, fileinfo->owner))
		return 0;
	strcpy(buf, &(fileinfo->filename[2]));
	buf[strlen (buf) - 2] = '\0';
	posttime = atoi(buf);
	if (posttime < currentuser.firstlogin)
		return 0;
	return 1;
}

int edit_post(int ent, struct fileheader *fileinfo, char *direct) {
	char buf[512];
	char *t;
	extern char currmaildir[STRLEN];

	if (!strcmp(currboard, "GoneWithTheWind") || digestmode == ATTACH_MODE)
		return DONOTHING;

	if (!in_mail) {
		if (!am_curr_bm()) {
			if (!IsTheFileOwner(fileinfo))
				return DONOTHING;
			board_t board;
			get_board(currboard, &board);
			if ((board.flag & BOARD_ANONY_FLAG)
					&& streq(fileinfo->owner, currboard))
				return DONOTHING;
		}
	}
	set_user_status(ST_EDIT);
	clear();
	if (in_mail)
		strcpy(buf, currmaildir);
	else
		strcpy(buf, direct);
	if ((t = strrchr(buf, '/')) != NULL)
		*t = '\0';
	//Added by Ashinmarch to support sharedmail
	if (fileinfo->filename[0] == 's') {
		prints("Type 2 Forbidden to Edit post!\n");
		return DONOTHING;
	}
	//added end

	char file[HOMELEN];
	sprintf(file, "%s/%s", buf, fileinfo->filename);
	if (vedit(file, NA, NA, NULL) == -1)
		return FULLUPDATE;
	valid_gbk_file(file, '?');

	if (!in_mail) {
		sprintf(genbuf, "edited post '%s' on %s", fileinfo->title,
				currboard);
		report(genbuf, currentuser.userid);
	}
	return FULLUPDATE;
}

int underline_post(int ent, struct fileheader *fileinfo, char *direct) {
	struct fileheader chkfileinfo; // add by quickmouse 01-05-30 检查一下 避免出现.DIR破坏
	char *path = direct;

	if (!am_curr_bm()) {
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE) {
		char notice[STRLEN];
		struct fileheader tmpfh;

		get_noticedirect (direct, notice);
		ent =
		search_record (notice, &tmpfh, sizeof (struct fileheader), cmpfilename,
				fileinfo->filename);
		if (ent <= 0)
		return DONOTHING;
		path = notice;
	} else {
#endif
	if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo), ent, 1)
			!= 1) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	if (strcmp(fileinfo->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
}
#endif
	if (fileinfo->accessed[0] & FILE_NOREPLY) {
		fileinfo->accessed[0] &= ~FILE_NOREPLY;
		bm_log(currentuser.userid, currboard, BMLOG_UNCANNOTRE, 1);
	} else {
		fileinfo->accessed[0] |= FILE_NOREPLY;
		bm_log(currentuser.userid, currboard, BMLOG_CANNOTRE, 1);
	}
	substitute_record(path, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

int makeDELETEDflag(int ent, struct fileheader *fileinfo, char *direct) {
	if (!(am_curr_bm()) || fileinfo->accessed[0] & (FILE_MARKED
			| FILE_DIGEST)) {
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE)
	return DONOTHING;
#endif
	if (fileinfo->accessed[0] & FILE_DELETED) {
		fileinfo->accessed[0] &= ~FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_UNWATER, 1);
	} else {
		fileinfo->accessed[0] |= FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_WATER, 1);
	}
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

//added by cometcaptor 2006-05-29
int move_notice(int ent, struct fileheader *fh, char *direct) {
	char path[256], tmppath[256];
	int lockfd;
	if (digestmode!=NA)
		return DONOTHING;
	if (!am_curr_bm())
		return DONOTHING;
	if (fh->accessed[1]&FILE_NOTICE) {
		get_noticedirect(direct, path);
		struct fileheader tmpfh;
		int pos=search_record(path, &tmpfh, sizeof(struct fileheader),
				cmpfilename, fh->filename);
		strcat(tmppath, "tmp/");
		strcat(tmppath, currboard);
		strcat(tmppath, ".lock");
		lockfd = creat(tmppath, 0600);
		fb_flock(lockfd, LOCK_EX);
		if (pos > 0) {
			delete_record(path, sizeof(struct fileheader), pos,
					cmpfilename, fh->filename);
			append_record(path, fh, sizeof(struct fileheader));
		} else
			return DONOTHING;
		fb_flock(lockfd, LOCK_UN);
		close(lockfd);
	}
	updatelastpost(currbp);
	return DIRCHANGED;
}
//add end
int mark_post(int ent, struct fileheader *fileinfo, char *direct) {
	struct fileheader chkfileinfo; // add by quickmouse 01-05-30 检查一下 避免出现.DIR破坏

	if (!am_curr_bm()) {
		return DONOTHING;
	}
#ifdef ENABLE_NOTICE
	if (fileinfo->accessed[1] & FILE_NOTICE)
	return DONOTHING;
#endif

	if (get_records(direct, &chkfileinfo, sizeof (chkfileinfo), ent, 1)
			!= 1) {
		// add by quickmouse 01-05-30

		return DONOTHING;
	}
	if (strcmp(fileinfo->filename, chkfileinfo.filename)) // add by quickmouse 01-05-30
	{
		return DONOTHING;
	}
	if (fileinfo->accessed[0] & FILE_MARKED) {
		fileinfo->accessed[0] &= ~FILE_MARKED;
		bm_log(currentuser.userid, currboard, BMLOG_UNMARK, 1);
	} else {
		fileinfo->accessed[0] |= FILE_MARKED;
		fileinfo->accessed[0] &= ~FILE_DELETED;
		bm_log(currentuser.userid, currboard, BMLOG_MARK, 1);
	}
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	return PARTUPDATE;
}

// 区段删除,从id1到id2;版面文件与精华区有区别
int delete_range(char *filename, int id1, int id2)
{
	struct fileheader *ptr, *wptr, *rptr;
	char dirpath[80], *p;
	int ret;
	int deleted=0;
	int subflag;
	int lookupuid;
	mmap_t m;

	strcpy(dirpath, filename);
	p=strrchr(dirpath, '/');
	if (!p) {
		return -1;
	}
	p++;

	BBS_TRY {
		m.oflag = O_RDWR;
		if (mmap_open(filename, &m) < 0)
			BBS_RETURN(-1);
		ret = 0;
		ptr = m.ptr;
		if (id1 > id2 || id2 * sizeof(struct fileheader) > m.size) {
			ret = -2; //区段范围超过文件大小
		} else {
			wptr = rptr = ptr + id1 - 1;
			while (id1 <= id2) {
				if (rptr->accessed[0] & FILE_MARKED) {
					if (wptr != rptr)
						memcpy(wptr, rptr, sizeof(struct fileheader));
					wptr++;
					rptr++;
				} else if (in_mail == NA) {
					if (digestmode == NA) {
						subflag = rptr->accessed[0] & FILE_DELETED ? YEA : NA;
						canceltotrash(filename, currentuser.userid, rptr,
								subflag, !HAS_PERM(PERM_OBOARDS));
						if (subflag == YEA && !is_junk_board(currbp)) {
							lookupuid = getuser(rptr->owner);
							if (lookupuid> 0 && lookupuser.numposts > 0) {
								lookupuser.numposts--;
								substitut_record (PASSFILE, &lookupuser, 
										sizeof (struct userec), lookupuid);
							}
						}
					} else if (digestmode == YEA || digestmode == ATTACH_MODE) {
						*p = '\0';
						strcat(dirpath, rptr->filename);
						unlink(dirpath);
					}
					rptr++;
					deleted++;
				} else {
					*p = '\0';
					strcat(dirpath, rptr->filename);
					unlink(dirpath);
					rptr++;
					deleted++;
				}
				id1++;
			}
		}
		if (ret == 0) {
			memmove(wptr, rptr, m.size - sizeof(struct fileheader) * id2);
			mmap_truncate(&m, m.size - sizeof(struct fileheader) * deleted);
		}
	}
	BBS_CATCH {
		ret = -3;
	}
	BBS_END mmap_close(&m);
	//add by danielfree to recount the attach files size.06-10-31
	if (digestmode==ATTACH_MODE) {
		char apath[256], cmd[256];
		sprintf(apath, "%s/upload/%s", BBSHOME, currboard);
		if (dashd(apath)) {
			sprintf(cmd, "du %s|cut -f1>%s/.size", apath, apath);
			system(cmd);
		}
	}//add end
	return ret;
}

int del_range(int ent, struct fileheader *fileinfo, char *direct) {
	char num[8];
	int inum1, inum2;

	if (session.status == ST_READING) {
		if (!am_curr_bm()) {
			return DONOTHING;
		}
	}
	if (digestmode > 1 && digestmode != ATTACH_MODE)
		return DONOTHING;
	//% getdata(t_lines - 1, 0, "首篇文章编号: ", num, 6, DOECHO, YEA);
	getdata(t_lines - 1, 0, "\xca\xd7\xc6\xaa\xce\xc4\xd5\xc2\xb1\xe0\xba\xc5: ", num, 6, DOECHO, YEA);
	inum1 = atoi(num);
	if (inum1 <= 0) {
		move(t_lines - 1, 50);
		//% prints("错误编号...");
		prints("\xb4\xed\xce\xf3\xb1\xe0\xba\xc5...");
		egetch();
		return PARTUPDATE;
	}
	//% getdata(t_lines - 1, 25, "末篇文章编号: ", num, 6, DOECHO, YEA);
	getdata(t_lines - 1, 25, "\xc4\xa9\xc6\xaa\xce\xc4\xd5\xc2\xb1\xe0\xba\xc5: ", num, 6, DOECHO, YEA);
	inum2 = atoi(num);
	if (inum2 < inum1) {
		move(t_lines - 1, 50);
		//% prints("错误区间...");
		prints("\xb4\xed\xce\xf3\xc7\xf8\xbc\xe4...");
		egetch();
		return PARTUPDATE;
	}
	move(t_lines - 1, 50);
	//% if (askyn("确定删除", NA, NA) == YEA) {
	if (askyn("\xc8\xb7\xb6\xa8\xc9\xbe\xb3\xfd", NA, NA) == YEA) {
		delete_range(direct, inum1, inum2);
		fixkeep(direct, inum1, inum2);
		if (session.status == ST_READING) {
			sprintf(genbuf, "Range delete %d-%d on %s", inum1, inum2,
					currboard);
			//securityreport (genbuf, 0, 2);
			bm_log(currentuser.userid, currboard, BMLOG_DELETE, 1);
			updatelastpost(currbp);
		} else {
			sprintf(genbuf, "Range delete %d-%d in mailbox", inum1, inum2);
			report(genbuf, currentuser.userid);
		}
		return DIRCHANGED;
	}
	move(t_lines - 1, 50);
	clrtoeol();
	//% prints("放弃删除...");
	prints("\xb7\xc5\xc6\xfa\xc9\xbe\xb3\xfd...");
	egetch();
	return PARTUPDATE;
}

//added by iamfat 2002.07.24
//extern int insert_record(char *filename, char *rptr, int size, int offset, int size2);
//modified by iamfat 2002.07.24
int UndeleteArticle(int ent, struct fileheader *fileinfo, char *direct) /* undelete 一篇文章 Leeward 98.05.18 */
{
	return _UndeleteArticle(ent, fileinfo, direct, YEA);
}

//加了个response变量 方便区域恢复的时候不回显
int _UndeleteArticle(int ent, struct fileheader *fileinfo, char *direct,
		int response) /* undelete 一篇文章 Leeward 98.05.18 */
{
	char buf[1024];
	struct stat st;
	int owned;
	int subflag;

	/* Modified by Amigo 2002.06.27. Add undelete for big D board. */
	// if (strcmp(currboard, "deleted")&&strcmp(currboard,"junk")) {
	if (digestmode != TRASH_MODE && digestmode != JUNK_MODE) {
		return DONOTHING;
	}

	sprintf(buf, "boards/%s/%s", currboard, fileinfo->filename);
	stat(buf, &st);
	if (!S_ISREG(st.st_mode))
		return DONOTHING;

	//if(digestmode==TRASH_MODE)
	setwbdir(buf, currboard);
	subflag = (fileinfo->accessed[1] & FILE_SUBDEL);
	fileinfo->accessed[1] &= ~FILE_SUBDEL;
	if (insert_record(buf, sizeof(struct fileheader), undelcheck, fileinfo)
			!= 0) {
		return DONOTHING;
	}
	updatelastpost(currbp);

	sprintf(buf, "boards/%s/%s", currboard,
			digestmode == TRASH_MODE ? ".TRASH" : ".JUNK");
	delete_record(buf, sizeof(struct fileheader), ent, cmpfilename,
			fileinfo->filename);
	owned = getuser(fileinfo->owner);
	if (!is_junk_board(currbp) && subflag && owned != 0 && atoi(fileinfo->filename
			+ 2) > lookupuser.firstlogin) {
		lookupuser.numposts++;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec),
				owned);
	}
	sprintf(genbuf, "UNDEL '%s' on %s", fileinfo->title, currboard);
	report(genbuf, currentuser.userid);
	bm_log(currentuser.userid, currboard, BMLOG_UNDELETE, 1);
	return DIRCHANGED;
}

int _del_post(int ent, struct fileheader *fileinfo, char *direct,
		int subflag, int hasjudge) {
	char buf[512];
	char usrid[STRLEN];
	char *t;
	int owned = 0, fail, IScurrent = 0;
	int posttime;
	extern int SR_BMDELFLAG;

	//if (fileinfo->accessed[0] & (FILE_MARKED|FILE_DIGEST)) return DONOTHING;
	//modified by roly 02.03.29

	if ((SR_BMDELFLAG) && (fileinfo->accessed[0] & (FILE_MARKED)))
		return DONOTHING;

	if (fileinfo->accessed[1] & (FILE_NOTICE))
		return DONOTHING;

	if (fileinfo->accessed[0] & (FILE_DELETED))
		subflag = YEA;

	if (digestmode > 1 && digestmode != ATTACH_MODE)
		return DONOTHING;
	if (fileinfo->owner[0] == '-')
		return DONOTHING;
	strcpy(usrid, fileinfo->owner);
	if (!strstr(usrid, "."))
		IScurrent = !strcmp(usrid, currentuser.userid);

	strcpy(buf, &(fileinfo->filename[2]));
	buf[strlen (buf) - 2] = '\0';
	posttime = atoi(buf);
	if (!IScurrent) {
		owned = getuser(usrid);
		if (owned != 0) {
			if (posttime < lookupuser.firstlogin)
				owned = 0;
		}
	} else
		owned = posttime > currentuser.firstlogin;

	if (hasjudge == YEA && !am_curr_bm()) {
		if (!(owned && IScurrent))
			return DONOTHING;
		board_t board;
		get_board(currboard, &board);
		if ((board.flag & BOARD_ANONY_FLAG) && streq(usrid, currboard))
			return DONOTHING;
	}
	if (!SR_BMDELFLAG) {
		//% sprintf(genbuf, "删除文章 [%-.55s]", fileinfo->title);
		sprintf(genbuf, "\xc9\xbe\xb3\xfd\xce\xc4\xd5\xc2 [%-.55s]", fileinfo->title);
		if (askyn(genbuf, NA, YEA) == NA) {
			move(t_lines - 1, 0);
			//% prints("放弃删除文章...");
			prints("\xb7\xc5\xc6\xfa\xc9\xbe\xb3\xfd\xce\xc4\xd5\xc2...");
			clrtoeol();
			egetch();
			return PARTUPDATE;
		}
	}
	fail = delete_record(direct, sizeof(struct fileheader), ent,
			cmpfilename, fileinfo->filename);
	if (!fail) {
		strcpy(buf, direct);
		if ((t = strrchr(buf, '/')) != NULL)
			*t = '\0';
		sprintf(genbuf, "Del '%s' on %s", fileinfo->title, currboard);
		report(genbuf, currentuser.userid);

		updatelastpost(currbp);
		/*if(subflag==NA)
		 cancelpost (currboard, currentuser.userid, fileinfo, 2);
		 else cancelpost (currboard, currentuser.userid, fileinfo, owned && IScurrent); */
		//added by iamfat 2003.03.01
		if (digestmode == NA)
			canceltotrash(direct, currentuser.userid, fileinfo, subflag,
					(SR_BMDELFLAG || !IScurrent)
							&& !HAS_PERM(PERM_OBOARDS));
		//add by danielfree to recount the attach files size.06-10-31
		if (digestmode==ATTACH_MODE) {
			char apath[256], cmd[256];
			sprintf(apath, "%s/upload/%s", BBSHOME, currboard);
			if (dashd(apath)) {
				sprintf(cmd, "du %s|cut -f1>%s/.size", apath, apath);
				system(cmd);
			}
		}//add end
		sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
		if (digestmode > 0)
			unlink(genbuf);
		if (!is_junk_board(currbp) && !digestmode) {
			if (owned && IScurrent) {
				set_safe_record();
				if (currentuser.numposts > 0 && subflag == YEA)
					currentuser.numposts--;
				substitut_record(PASSFILE, &currentuser,
						sizeof (currentuser), usernum);
			} else if (owned && BMDEL_DECREASE) {
				if (lookupuser.numposts > 0 && subflag == YEA)
					lookupuser.numposts--;
				substitut_record(PASSFILE, &lookupuser,
						sizeof(struct userec), owned);
			}
		}
		return DIRCHANGED;
	} else if (SR_BMDELFLAG) {
		return -1;
	}
	move(t_lines - 1, 0);
	//% prints("删除失败...");
	prints("\xc9\xbe\xb3\xfd\xca\xa7\xb0\xdc...");
	clrtoeol();
	egetch();
	return PARTUPDATE;
}

int del_post(int ent, struct fileheader *fileinfo, char *direct) {
	if (!strcmp(currboard, "GoneWithTheWind"))
		return DONOTHING;

	return _del_post(ent, fileinfo, direct, YEA, YEA);
}

int Save_post(int ent, struct fileheader *fp, char *direct)
{
	if (!HAS_PERM(PERM_BOARDS) || digestmode == ATTACH_MODE)
		return DONOTHING;
	if (!in_mail && !am_curr_bm())
		return DONOTHING;

	return (a_Save(fp->title, fp->filename, NA, YEA));
}

int Import_post(int ent, struct fileheader *fp, char *direct) {
	if (!HAS_PERM(PERM_BOARDS) || digestmode == ATTACH_MODE)
		return DONOTHING;
	if (DEFINE(DEF_MULTANNPATH)
			&& set_ann_path(NULL, NULL, ANNPATH_GETMODE) == 0)
		return FULLUPDATE;
	a_Import(fp->title, fp->filename, NA);
	if (DEFINE(DEF_MULTANNPATH))
		return FULLUPDATE;
	return DONOTHING;
}

int check_notespasswd(void)
{
	FILE *pass;
	char passbuf[20], prepass[STRLEN];
	char buf[STRLEN];

	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen (prepass) - 1] = '\0';
		//% getdata(2, 0, "请输入秘密备忘录密码: ", passbuf, 19, NOECHO, YEA);
		getdata(2, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!passwd_match(prepass, passbuf)) {
			move(3, 0);
			//% prints("错误的秘密备忘录密码...");
			prints("\xb4\xed\xce\xf3\xb5\xc4\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xc3\xdc\xc2\xeb...");
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

int show_b_secnote() {
	char buf[256];

	clear();
	setvfile(buf, currboard, "secnotes");
	if (dashf(buf)) {
		if (!check_notespasswd())
			return FULLUPDATE;
		clear();
		ansimore(buf, NA);
	} else {
		move(3, 25);
		//% prints("此讨论区尚无「秘密备忘录」。");
		prints("\xb4\xcb\xcc\xd6\xc2\xdb\xc7\xf8\xc9\xd0\xce\xde\xa1\xb8\xc3\xd8\xc3\xdc\xb1\xb8\xcd\xfc\xc2\xbc\xa1\xb9\xa1\xa3");
	}
	pressanykey();
	return FULLUPDATE;
}

int show_b_note() {
	clear();
	if (show_board_notes(currboard, 2) == -1) {
		move(3, 30);
		//% prints("此讨论区尚无「备忘录」。");
		prints("\xb4\xcb\xcc\xd6\xc2\xdb\xc7\xf8\xc9\xd0\xce\xde\xa1\xb8\xb1\xb8\xcd\xfc\xc2\xbc\xa1\xb9\xa1\xa3");
		pressanykey();
	}
	return FULLUPDATE;
}

int into_announce() {
	char found[STRLEN];

	if (a_menusearch(currboard, found)) {
		sprintf(ANN_LOG_PATH, "logs/%s", currboard);
		a_menu("", found, (HAS_PERM(PERM_ANNOUNCE) ? PERM_BOARDS : 0), 0);
		return MODECHANGED;
	}
	return DONOTHING;
}

int into_myAnnounce() {
	Personal("*");
	return FULLUPDATE;
}

int into_PAnnounce() {
	Personal(NULL);
	return FULLUPDATE;
}

int Personal(const char *userid)
{
	char found[256], lookid[IDLEN + 6];
	int id;

	if (!userid) {
		clear();
		move(2, 0);
		//% usercomplete("您想看谁的个人文集: ", lookid);
		usercomplete("\xc4\xfa\xcf\xeb\xbf\xb4\xcb\xad\xb5\xc4\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf: ", lookid);
		if (lookid[0] == '\0') {
			clear();
			return 1;
		}
	} else
		strcpy(lookid, userid);
	if (lookid[0] == '*') {
		sprintf(lookid, "/%c/%s", toupper(currentuser.userid[0]),
				currentuser.userid);
	} else {
		if (!(id = getuser(lookid))) {
			lookid[1] = toupper(lookid[0]);
			if (lookid[1] < 'A' || lookid[1] > 'Z')
				lookid[0] = '\0';
			else {
				lookid[0] = '/';
				lookid[2] = '\0';
			}
		} else {
			sprintf(lookid, "/%c/%s", toupper(lookupuser.userid[0]),
					lookupuser.userid);
		}
	}
	sprintf(found, "0Announce/groups/GROUP_0/PersonalCorpus/%s", lookid);
	if (!dashd(found))
		sprintf(found, "0Announce/groups/GROUP_0/PersonalCorpus");
	a_menu("", found, (HAS_PERM(PERM_ANNOUNCE) ? PERM_BOARDS : 0), 0);
	return 1;
}

int read_attach(int ent, struct fileheader *fileinfo, char *direct) {
	extern char currdirect[STRLEN];

	digestmode = ATTACH_MODE;
	sprintf(currdirect, "upload/%s/.DIR", currboard);
	if (!dashf(currdirect)) {
		digestmode = NA;
		setbdir(currdirect, currboard);
		//% presskeyfor("版面附件区无内容，按任意键继续...", t_lines-1);
		presskeyfor("\xb0\xe6\xc3\xe6\xb8\xbd\xbc\xfe\xc7\xf8\xce\xde\xc4\xda\xc8\xdd\xa3\xac\xb0\xb4\xc8\xce\xd2\xe2\xbc\xfc\xbc\xcc\xd0\xf8...", t_lines-1);
		update_endline();
		return DONOTHING;
	}
	return NEWDIRECT;
}

int show_online(void)
{
	if (currbp->flag & BOARD_ANONY_FLAG) {
		// TODO: prompt at bottom.
		return DONOTHING;
	}
#ifndef FDQUAN
	if (!(currbp->flag & BOARD_CLUB_FLAG) || !(am_curr_bm()
			|| isclubmember(currentuser.userid, currboard))) {
		return DONOTHING;
	}
#endif
	show_users_in_board();
	return FULLUPDATE;
}

extern int mainreadhelp();
extern int b_notes_edit();

struct one_key read_comms[] = {
#ifdef ENABLE_NOTICE
		{';', move_notice},
#endif
		{',', read_attach},
		{'\'', post_search_down},
		{'\"', post_search_up},
		{'\0', NULL}
};

int board_read(void)
{
	char notename[STRLEN];
	time_t usetime;
	struct stat st;

	if (!currbp->id) {
		move(2, 0);
		//% prints("请先选择讨论区\n");
		prints("\xc7\xeb\xcf\xc8\xd1\xa1\xd4\xf1\xcc\xd6\xc2\xdb\xc7\xf8\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}

	in_mail = NA;

	board_t board;
	get_board(currboard, &board);
	if (board.flag & BOARD_DIR_FLAG)
		return FULLUPDATE;

	if (!has_read_perm(&currentuser, &board)) {
		clear();
		move(5, 10);
		//% prints("您不是俱乐部版 %s 的成员，无权进入该版", currboard);
		prints("\xc4\xfa\xb2\xbb\xca\xc7\xbe\xe3\xc0\xd6\xb2\xbf\xb0\xe6 %s \xb5\xc4\xb3\xc9\xd4\xb1\xa3\xac\xce\xde\xc8\xa8\xbd\xf8\xc8\xeb\xb8\xc3\xb0\xe6", currboard);
		pressanykey();
		return FULLUPDATE;
	}

	brc_initial(currentuser.userid, currboard);
	set_current_board(board.id);

	setvfile(notename, currboard, "notes");
	if (stat(notename, &st) != -1) {
		if (st.st_mtime < (time(NULL) - 7 * 86400)) {
			utimes(notename, NULL);
			setvfile(genbuf, currboard, "noterec");
			unlink(genbuf);
		}
	}

	if (vote_flag(currboard, '\0', 1 /* 检查读过新的备忘录没 */) == 0) {
		if (dashf(notename)) {
			ansimore(notename, YEA);
			vote_flag(currboard, 'R', 1 /* 写入读过新的备忘录 */);
		}
	}

	usetime = time(0);
	extern int post_list_board(int);
	post_list_board(board.id);

//	i_read(ST_READING, buf, readtitle, readdoent, &read_comms[0],
//			sizeof(struct fileheader));
	//commented by iamfat 2004.03.14
	board_usage(currboard, time(0) - usetime);
	bm_log(currentuser.userid, currboard, BMLOG_STAYTIME, time(0)
			- usetime);
	bm_log(currentuser.userid, currboard, BMLOG_INBOARD, 1);

	set_current_board(0);
	
	brc_update(currentuser.userid, currboard);
	return 0;
}

/*Add by SmallPig*/
void notepad() {
	char tmpname[STRLEN], note1[4];
	char note[3][STRLEN - 4];
	char tmp[STRLEN];
	FILE *in;
	int i, n;
	time_t thetime = time(0);

	clear();
	move(0, 0);
	//% prints("开始您的留言吧！大家正拭目以待....\n");
	prints("\xbf\xaa\xca\xbc\xc4\xfa\xb5\xc4\xc1\xf4\xd1\xd4\xb0\xc9\xa3\xa1\xb4\xf3\xbc\xd2\xd5\xfd\xca\xc3\xc4\xbf\xd2\xd4\xb4\xfd....\n");
	set_user_status(ST_WNOTEPAD);
	sprintf(tmpname, "tmp/notepad.%s.%05d", currentuser.userid, session.pid);
	if ((in = fopen(tmpname, "w")) != NULL) {
		for (i = 0; i < 3; i++)
			memset(note[i], 0, STRLEN - 4);
		while (1) {
			for (i = 0; i < 3; i++) {
				getdata(1 + i, 0, ": ", note[i], STRLEN - 5, DOECHO, NA);
				if (note[i][0] == '\0')
					break;
			}
			if (i == 0) {
				fclose(in);
				unlink(tmpname);
				return;
			}
			//% getdata(5, 0, "是否把您的大作放入留言板 (Y)是的 (N)不要 (E)再编辑 [Y]: ", note1,
			getdata(5, 0, "\xca\xc7\xb7\xf1\xb0\xd1\xc4\xfa\xb5\xc4\xb4\xf3\xd7\xf7\xb7\xc5\xc8\xeb\xc1\xf4\xd1\xd4\xb0\xe5 (Y)\xca\xc7\xb5\xc4 (N)\xb2\xbb\xd2\xaa (E)\xd4\xd9\xb1\xe0\xbc\xad [Y]: ", note1,
					3, DOECHO, YEA);
			if (note1[0] == 'e' || note1[0] == 'E')
				continue;
			else
				break;
		}
		if (note1[0] != 'N' && note1[0] != 'n') {
			//% sprintf(tmp, "[1;32m%s[37m（%.18s）", currentuser.userid,
			sprintf(tmp, "[1;32m%s[37m\xa3\xa8%.18s\xa3\xa9", currentuser.userid,
					currentuser.username);

			fprintf(
					in,
					//% "[1;34m▕[44m▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔[36m酸[32m甜[33m苦[31m辣[37m板[34m▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔[44m◣[m\n");
					"[1;34m\xa8\x8a[44m\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89[36m\xcb\xe1[32m\xcc\xf0[33m\xbf\xe0[31m\xc0\xb1[37m\xb0\xe5[34m\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89\xa8\x89[44m\xa8\x8e[m\n");
			
			fprintf(
					in,
					//% "[1;34m▕[32;44m %-44s[32m在 [36m%23.23s[32m 离开时留下的话  [m\n",
					"[1;34m\xa8\x8a[32;44m %-44s[32m\xd4\xda [36m%23.23s[32m \xc0\xeb\xbf\xaa\xca\xb1\xc1\xf4\xcf\xc2\xb5\xc4\xbb\xb0  [m\n",
					tmp, getdatestring(thetime, DATE_ZH) + 6);
			for (n = 0; n < i; n++) {
				if (note[n][0] == '\0')
					break;
				//% fprintf(in, "[1;34m▕[33;44m %-75.75s[1;34m[m \n",
				fprintf(in, "[1;34m\xa8\x8a[33;44m %-75.75s[1;34m[m \n",
						note[n]);
			}
			fprintf(in,
					//% "[1;34m▕[44m ───────────────────────────────────── [m \n");
					"[1;34m\xa8\x8a[44m \xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4 [m \n");
			catnotepad(in, "etc/notepad");
			fclose(in);
			f_cp(tmpname, "etc/notepad", O_CREAT);
			unlink(tmpname);
		} else {
			fclose(in);
			unlink(tmpname);
		}
	}
	clear();
	return;
}

int Q_Goodbye(void)
{
	char fname[STRLEN];
	int logouts;

	/* added by roly 02.03.21*/
	FILE *sysops;
	int i, choose;
	char buf[STRLEN], spbuf[STRLEN];
	char bye_msgs[9][STRLEN];

	/* add end */

	setuserfile(fname, "msgfile");

#ifdef LOG_MY_MESG
	unlink (fname);
	setuserfile (fname, "msgfile.me");
#endif

	/* edwardc.990423 讯息浏览器 */
	if (dashf(fname)) {
		clear();
		msg_more();
	}
	clear();
	prints("\n\n\n\n");

	/* added by roly 02.03.21 */
	if (session.visible) {
		int byes = 0;
		if ((sysops = fopen("etc/friendbye", "r")) != NULL) {
			while (byes < 8 && fgets(buf, STRLEN, sysops) != NULL) {
				if (buf[0] != '\0') {
					buf[strlen (buf) - 1] = '\0';
					strcpy(bye_msgs[byes], buf);
					byes++;
				}
			}
			fclose(sysops);
		}
		clear();
		move(0, 0);
		//% prints("您就要离开 %s 向好友们告个别?\n", BoardName);
		prints("\xc4\xfa\xbe\xcd\xd2\xaa\xc0\xeb\xbf\xaa %s \xcf\xf2\xba\xc3\xd3\xd1\xc3\xc7\xb8\xe6\xb8\xf6\xb1\xf0?\n", BoardName);
		for (i = 0; i < byes; i++)
			prints(" %1d. %s\n", i, bye_msgs[i]);
		//% prints(" %1d. (其他)\n", byes);
		prints(" %1d. (\xc6\xe4\xcb\xfb)\n", byes);
		//% prints(" %1d. (不通知好友,悄悄离去)\n", byes + 1);
		prints(" %1d. (\xb2\xbb\xcd\xa8\xd6\xaa\xba\xc3\xd3\xd1,\xc7\xc4\xc7\xc4\xc0\xeb\xc8\xa5)\n", byes + 1);
		//% sprintf(spbuf, "您的选择是 [%1d]:", byes + 1);
		sprintf(spbuf, "\xc4\xfa\xb5\xc4\xd1\xa1\xd4\xf1\xca\xc7 [%1d]:", byes + 1);
		getdata(byes + 3, 0, spbuf, genbuf, 4, DOECHO, YEA);
		choose = atoi(genbuf);
		choose = genbuf[0] - '0';

		if (choose >= 0 && choose <= byes) {
			char buftemp[STRLEN]; //added by roly 02.03.24;

			if (choose >= 0 && choose < byes)
				strlcpy(buftemp, bye_msgs[choose], STRLEN);

			if (choose == byes)
				//% get_msg("在线好友", buftemp, byes + 4);
				get_msg("\xd4\xda\xcf\xdf\xba\xc3\xd3\xd1", buftemp, byes + 4);

			char msg[MAX_MSG_SIZE + 2];
			sprintf(msg, "%s", buftemp); //added by roly 02.03.24
			logout_msg(msg);
		}
		clear();
		prints("\n\n\n\n\n\n\n");
	}

	/* added end */

	setuserfile(fname, "notes");
	if (dashf(fname))
		ansimore(fname, YEA);

	setuserfile(fname, "logout");
	if (dashf(fname)) {
		logouts = countlogouts(fname);
		if (logouts >= 1) {
			user_display(fname, (logouts == 1) ? 1
					: (currentuser.numlogins % (logouts)) + 1, YEA);
		}
	} else {
		if (fill_shmfile(2, "etc/logout", "GOODBYE_SHMKEY"))
			show_goodbyeshm();
	}
	pressreturn();
	report("exit", currentuser.userid);

	CreateNameList();

	if (session.id) {
		time_t stay;

		stay = time(0) - login_start_time;
		//iamfat added 2004.01.05 to avoid overflow
		currentuser.username[NAMELEN - 1] = 0;
		sprintf(genbuf, "Stay:%3ld (%s)", stay / 60, currentuser.username);
		log_usies("EXIT ", genbuf, &currentuser);
		u_exit();
	}

	set_current_board(0);

	sleep(1);
	exit(0);
	return -1;
}

int Goodbye(void)
{
	char sysoplist[20][15], syswork[20][21], buf[STRLEN];
	int i, num_sysop, choose;
	FILE *sysops;
	char *ptr;

	*quote_file = '\0';
	i = 0;
	if ((sysops = fopen("etc/sysops", "r")) != NULL) {
		while (fgets(buf, STRLEN, sysops) != NULL && i <= 19) {
			if (buf[0] == '#')
				continue;
			ptr = strtok(buf, " \n\r\t");
			if (ptr) {
				strlcpy(sysoplist[i], ptr, 14);
				ptr = strtok(NULL, " \n\r\t");
				if (ptr) {
					strlcpy(syswork[i], ptr, 20);
				} else
					//% strcpy(syswork[i], "[职务不明]");
					strcpy(syswork[i], "[\xd6\xb0\xce\xf1\xb2\xbb\xc3\xf7]");
				i++;
			}
		}
		fclose(sysops);
	}

	num_sysop = i;
	move(1, 0);
	alarm(0);
	clear();
	move(0, 0);
	//% prints("您就要离开 %s ，可有什麽建议吗？\n", BoardName);
	prints("\xc4\xfa\xbe\xcd\xd2\xaa\xc0\xeb\xbf\xaa %s \xa3\xac\xbf\xc9\xd3\xd0\xca\xb2\xf7\xe1\xbd\xa8\xd2\xe9\xc2\xf0\xa3\xbf\n", BoardName);
	//% prints("[[1;33m1[m] 寄信给站长/服务组寻求帮助\n");
	prints("[[1;33m1[m] \xbc\xc4\xd0\xc5\xb8\xf8\xd5\xbe\xb3\xa4/\xb7\xfe\xce\xf1\xd7\xe9\xd1\xb0\xc7\xf3\xb0\xef\xd6\xfa\n");
	//% prints("[[1;33m2[m] 按错了啦，我还要玩\n");
	prints("[[1;33m2[m] \xb0\xb4\xb4\xed\xc1\xcb\xc0\xb2\xa3\xac\xce\xd2\xbb\xb9\xd2\xaa\xcd\xe6\n");
#ifdef USE_NOTEPAD
	if (strcmp (currentuser.userid, "guest") != 0) {
		//% prints ("[[1;33m3[m] 写写[1;32m留[33m言[35m板[m罗\n");
		prints ("[[1;33m3[m] \xd0\xb4\xd0\xb4[1;32m\xc1\xf4[33m\xd1\xd4[35m\xb0\xe5[m\xc2\xde\n");
	}
#endif
	//% prints("[[1;33m4[m] 不寄罗，要离开啦\n");
	prints("[[1;33m4[m] \xb2\xbb\xbc\xc4\xc2\xde\xa3\xac\xd2\xaa\xc0\xeb\xbf\xaa\xc0\xb2\n");
	//% sprintf(buf, "您的选择是 [[1;32m4[m]：");
	sprintf(buf, "\xc4\xfa\xb5\xc4\xd1\xa1\xd4\xf1\xca\xc7 [[1;32m4[m]\xa3\xba");
	getdata(7, 0, buf, genbuf, 4, DOECHO, YEA);
	clear();
	choose = genbuf[0] - '0';
	if (choose == 1) {
		//% prints("     站长的 ID    负 责 的 职 务\n");
		prints("     \xd5\xbe\xb3\xa4\xb5\xc4 ID    \xb8\xba \xd4\xf0 \xb5\xc4 \xd6\xb0 \xce\xf1\n");
		prints("     ============ =====================\n");
		for (i = 1; i <= num_sysop; i++) {
			prints("[[1;33m%2d[m] %-12s %s\n", i, sysoplist[i - 1],
					syswork[i - 1]);
		}
		//% prints("[[1;33m%2d[m] 还是走了罗！\n", num_sysop + 1);
		prints("[[1;33m%2d[m] \xbb\xb9\xca\xc7\xd7\xdf\xc1\xcb\xc2\xde\xa3\xa1\n", num_sysop + 1);
		//% sprintf(buf, "您的选择是 [[1;32m%d[m]：", num_sysop + 1);
		sprintf(buf, "\xc4\xfa\xb5\xc4\xd1\xa1\xd4\xf1\xca\xc7 [[1;32m%d[m]\xa3\xba", num_sysop + 1);
		getdata(num_sysop + 5, 0, buf, genbuf, 4, DOECHO, YEA);
		choose = atoi(genbuf);
		if (choose >= 1 && choose <= num_sysop)
			//% do_send(sysoplist[choose - 1], "使用者寄来的建议信");
			do_send(sysoplist[choose - 1], "\xca\xb9\xd3\xc3\xd5\xdf\xbc\xc4\xc0\xb4\xb5\xc4\xbd\xa8\xd2\xe9\xd0\xc5");
		choose = -1;
	}
	if (choose == 2)
		return FULLUPDATE;
	if (strcmp(currentuser.userid, "guest") != 0) {
#ifdef USE_NOTEPAD
		if (HAS_PERM (PERM_POST) && choose == 3)
		notepad ();
#endif
	}
	return Q_Goodbye();

}

void do_report(const char *filename, const char *s) {
	char buf[512];
	sprintf(buf, "%-12.12s %16.16s %s\n", currentuser.userid,
			getdatestring(time(NULL), DATE_ZH) + 6, s);
	file_append(filename, buf);
}

void gamelog(char *s) {
	do_report("game/trace", s);
}

/* added by money to provide a method of logging by metalog daemon 2004.01.07 */
#ifdef USE_METALOG
void board_usage (char *mode, time_t usetime)
{
	syslog (LOG_LOCAL5 | LOG_INFO, "USE %-20.20s Stay: %5ld (%s)", mode,
			usetime, currentuser.userid);
}
#else
void board_usage(char *mode, time_t usetime) {
	char buf[256];

	sprintf(buf, "%.22s USE %-20.20s Stay: %5ld (%s)\n",
			getdatestring(time(NULL), DATE_ZH),
			mode, usetime, currentuser.userid);
	file_append("use_board", buf);
}
#endif

int Info() {
	set_user_status(ST_XMENU);
	ansimore("Version.Info", YEA);
	clear();
	return 0;
}

int Conditions() {
	set_user_status(ST_XMENU);
	ansimore("COPYING", YEA);
	clear();
	return 0;
}

int Welcome() {
	char ans[3];

	set_user_status(ST_XMENU);
	if (!dashf("Welcome2"))
		ansimore("Welcome", YEA);
	else {
		clear();
		//% stand_title("观看进站画面");
		stand_title("\xb9\xdb\xbf\xb4\xbd\xf8\xd5\xbe\xbb\xad\xc3\xe6");
		for (;;) {
			//% getdata(1, 0, "(1)特殊进站公布栏  (2)本站进站画面 ? : ", ans, 2, DOECHO,
			getdata(1, 0, "(1)\xcc\xd8\xca\xe2\xbd\xf8\xd5\xbe\xb9\xab\xb2\xbc\xc0\xb8  (2)\xb1\xbe\xd5\xbe\xbd\xf8\xd5\xbe\xbb\xad\xc3\xe6 ? : ", ans, 2, DOECHO,
					YEA);

			/* skyo.990427 modify  按 Enter 跳出  */
			if (ans[0] == '\0') {
				clear();
				return 0;
			}
			if (ans[0] == '1' || ans[0] == '2')
				break;
		}
		if (ans[0] == '1')
			ansimore("Welcome", YEA);
		else
			ansimore("Welcome2", YEA);
	}
	clear();
	return 0;
}

void canceltotrash(char *path, char *userid, struct fileheader *fh,
		int subflag, int totrash) {
	char tpath[80];
	char *ptr;

	strcpy(tpath, path);
	ptr = strrchr(tpath, '/');
	if (!ptr)
		return;
	ptr++;
	*ptr = '\0';
	strcat(tpath, fh->filename);
	if (!dashf(tpath))
		return;
	*ptr = '\0';
	strcat(tpath, totrash ? TRASH_DIR : JUNK_DIR);
	fh->timeDeleted = time(0);
	strcpy(fh->szEraser, userid);
	fh->accessed[0] = 0;
	fh->accessed[1] = 0;
	if (subflag == YEA)
		fh->accessed[1] |= FILE_SUBDEL;
	else
		fh->accessed[1] &= ~FILE_SUBDEL;
	append_record(tpath, fh, sizeof(struct fileheader));
}

int thesis_mode() {
	int id; //, i;
	unsigned int pbits;

	//i = 'W' - 'A';
	id = getuser(currentuser.userid);
	pbits = lookupuser.userdefine;
	//pbits ^= (1 << i);
	pbits ^= DEF_THESIS;
	lookupuser.userdefine = pbits;
	currentuser.userdefine = pbits;
	substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
	return FULLUPDATE;
}

/* Add by everlove 07/08/2001 制作合集 */
void Add_Combine(const char *board, struct fileheader * fileinfo, int has_cite) //added by roly 02.05.20
{
	FILE *fp;
	char buf[STRLEN];
	char temp2[1024];

	sprintf(buf, "tmp/%s.combine", currentuser.userid);
	fp = fopen(buf, "at");
	//% fprintf(fp, "[1;32m☆──────────────────────────────────────☆[0;1m\n");
	fprintf(fp, "[1;32m\xa1\xee\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa1\xee[0;1m\n");
	{
		FILE *fp1;
		char buf[80];
		char *s_ptr, *e_ptr;
		int blankline = 0;

		//modified by iamfat 2003.03.25
		if (in_mail == NA)
			setbfile(buf, board, fileinfo->filename);
		else
			setmfile(buf, currentuser.userid, fileinfo->filename);
		fp1 = fopen(buf, "rt");
		if (fgets(temp2, 256, fp1) != NULL) {
			e_ptr = strchr(temp2, ',');
			if (e_ptr != NULL)
				*e_ptr = '\0';
			s_ptr = &temp2[7];
			//% fprintf(fp, "    [0;1;32m%s [0;1m于", s_ptr);
			fprintf(fp, "    [0;1;32m%s [0;1m\xd3\xda", s_ptr);
		}
		fgets(temp2, 256, fp1);
		if (fgets(temp2, 256, fp1) != NULL) {
			e_ptr = strchr(temp2, ')');
			if (e_ptr != NULL)
				*(e_ptr) = '\0';
			s_ptr = strchr(temp2, '(');
			if (s_ptr == NULL)
				s_ptr = temp2;
			else
				s_ptr++;
			//% fprintf(fp, " [1;36m%s[0;1m 提到：[0m\n", s_ptr);
			fprintf(fp, " [1;36m%s[0;1m \xcc\xe1\xb5\xbd\xa3\xba[0m\n", s_ptr);
		}
		while (!feof(fp1)) {
			fgets(temp2, 256, fp1);
			if ((unsigned) *temp2 < '\x1b') {
				if (blankline)
					continue;
				else
					blankline = 1;
			} else
				blankline = 0;
			//% if (strstr(temp2, "【"))
			if (strstr(temp2, "\xa1\xbe"))
				continue;
			if ((has_cite != YEA) && (+(*temp2 == ':')))
				continue;
			//modified by roly 02.05.20 for yinyan
			if (strncmp(temp2, "--", 2) == 0)
				break;
			fputs(temp2, fp);
		}
		fclose(fp1);
	}
	fprintf(fp, "\n");
	fclose(fp);
}
