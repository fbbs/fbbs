#include "bbs.h"
#include "record.h"
#include "fbbs/fileio.h"
#include "fbbs/friend.h"
#include "fbbs/helper.h"
#include "fbbs/mail.h"
#include "fbbs/msg.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

extern struct postheader header;
extern char BoardName[];

/*For read.c*/
int auth_search_down();
int auth_search_up();
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
int msg_more();
int show_author();
int SR_last();
int SR_first();
int SR_read();
int SR_author();
int Q_Goodbye();
int G_SENDMODE = NA;
extern char quote_file[], quote_user[];
char currmaildir[STRLEN];
#define maxrecp 300

int in_mail;

int chkmail(void)
{
	static long lasttime = 0;
	static int ismail = 0;
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

int check_query_mail(const char *qry_mail_dir)
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
	/*离线查询新信只要查询最後一封是否为新信，其他并不重要*/
	/*Modify by SmallPig*/
	read(fd, &ch, 1);
	if (!(ch & FILE_READ)) {
		close(fd);
		return YEA;
	}
	close(fd);
	return NA;
}

static int mailmode;

struct mail_all_struct {
	const struct postheader *header;
	const char *file;
};

static int mailto(void *uentpv, int index, void *args) {
	char filename[STRLEN];
	file_temp_name(filename, sizeof(filename));

	struct mail_all_struct *s = args;

	struct userec *uentp = (struct userec *)uentpv;
	if ((!(uentp->userlevel & PERM_BINDMAIL) && mailmode == 1) ||
			(uentp->userlevel & PERM_BOARDS && mailmode == 3)
			|| (uentp->userlevel & PERM_SPECIAL0 && mailmode == 4)
			|| (uentp->userlevel & PERM_SPECIAL9 && mailmode == 5)) {
		mail_file(filename, uentp->userid, s->header->title);
		session_set_idle_cached();
	} else if (uentp->userlevel & PERM_POST && mailmode == 2) {
		sharedmail_file(s->file, uentp->userid, s->header->title);
		session_set_idle_cached();
	}
	return 1;
}

static int mailtoall(int mode, char *fname, const struct postheader *header)
{
	mailmode = mode;
	struct mail_all_struct s = { .header = header, .file = fname };
	if (apply_record(PASSFILE, mailto, sizeof(struct userec),
			&s, 0, 0, false) == -1) {
		prints("No Users Exist");
		pressreturn();
		return 0;
	}
	return 1;
}

int mailall(void)
{
	struct postheader header;
	char ans[4], fname[STRLEN], title[STRLEN];
	char doc[5][STRLEN], buf[STRLEN];
	int i;
	//% strcpy(title, "没主题");
	strcpy(title, "\xc3\xbb\xd6\xf7\xcc\xe2");
	set_user_status(ST_SMAIL);
	screen_clear();
	move(0, 0);
	file_temp_name(fname, sizeof(fname));
	//% prints("你要寄给所有的：\n");
	prints("\xc4\xe3\xd2\xaa\xbc\xc4\xb8\xf8\xcb\xf9\xd3\xd0\xb5\xc4\xa3\xba\n");
	//% prints("(0) 放弃\n");
	prints("(0) \xb7\xc5\xc6\xfa\n");
	//% strcpy(doc[0], "(1) 尚未通过身份确认的使用者");
	strcpy(doc[0], "(1) \xc9\xd0\xce\xb4\xcd\xa8\xb9\xfd\xc9\xed\xb7\xdd\xc8\xb7\xc8\xcf\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf");
	//% strcpy(doc[1], "(2) 所有通过身份确认的使用者");
	strcpy(doc[1], "(2) \xcb\xf9\xd3\xd0\xcd\xa8\xb9\xfd\xc9\xed\xb7\xdd\xc8\xb7\xc8\xcf\xb5\xc4\xca\xb9\xd3\xc3\xd5\xdf");
	//% strcpy(doc[2], "(3) 所有的版主");
	strcpy(doc[2], "(3) \xcb\xf9\xd3\xd0\xb5\xc4\xb0\xe6\xd6\xf7");
	//% strcpy(doc[3], "(4) 所有现任站务");
	strcpy(doc[3], "(4) \xcb\xf9\xd3\xd0\xcf\xd6\xc8\xce\xd5\xbe\xce\xf1");
	//% strcpy(doc[4], "(5) 现任站务以及离任站务");
	strcpy(doc[4], "(5) \xcf\xd6\xc8\xce\xd5\xbe\xce\xf1\xd2\xd4\xbc\xb0\xc0\xeb\xc8\xce\xd5\xbe\xce\xf1");
	for (i = 0; i < 5; i++)
		prints("%s\n", doc[i]);
	//% getdata(8, 0, "请输入模式 (0~5)? [0]: ", ans, 2, DOECHO, YEA);
	getdata(8, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xa3\xca\xbd (0~5)? [0]: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' < 1 || ans[0] - '0' > 5) {
		return NA;
	}
	//% sprintf(buf, "是否确定寄给%s ", doc[ans[0] - '0' - 1]);
	sprintf(buf, "\xca\xc7\xb7\xf1\xc8\xb7\xb6\xa8\xbc\xc4\xb8\xf8%s ", doc[ans[0] - '0' - 1]);
	move(9, 0);
	if (askyn(buf, NA, NA) == NA)
		return NA;
	in_mail = YEA;
	header.reply = false;
	//% strcpy(header.title, "没主题");
	strcpy(header.title, "\xc3\xbb\xd6\xf7\xcc\xe2");
	strcpy(header.ds, doc[ans[0] - '0' - 1]);
	header.postboard = NA;
	i = post_header(&header);
	if (i == -1)
		return NA;
	if (i == YEA) {
		char title[sizeof(header.title)];
		//% snprintf(title, sizeof(title), "[Type %c 公告] %s", ans[0], header.title);
		snprintf(title, sizeof(title), "[Type %c \xb9\xab\xb8\xe6] %s", ans[0], header.title);
		strlcpy(header.title, title, sizeof(header.title));
	}
	setquotefile("");

	/***********Modified by Ashinmarch on 08.3.30 to improve Type 2 mailall*******************/
	/***********Type 2的群信改为共享文件的形式， 目的减少文件的拷贝，防止死机*****************/
	/***********相关改动文件：list.c, bbs.c***************************************************/
	if (ans[0] - '0' == 2)
		sprintf(fname, "sharedmail/mailall.%s.%ld", currentuser.userid,
				time(0));
	/**********Modified end**********/
	do_quote(quote_file, fname, header.include_mode, header.anonymous);
	if (tui_edit(fname, false, true, true, &header) != TUI_EDIT_SAVED) {
		in_mail = NA;
		unlink(fname);
		screen_clear();
		return -2;
	}
	screen_move_clear(-1);
	//% prints("[5;1;32;44m正在寄件中，请稍候.....                                                        [m");
	prints("[5;1;32;44m\xd5\xfd\xd4\xda\xbc\xc4\xbc\xfe\xd6\xd0\xa3\xac\xc7\xeb\xc9\xd4\xba\xf2.....                                                        [m");
	screen_flush();

	mailtoall(ans[0] - '0', fname, &header);

	screen_move_clear(-1);
	/****type 2共享文件不需要删除****/
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
	set_user_status(ST_SMAIL);
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		screen_clear();
		move(4,0);
		//% prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		prints("\n\n        \xc4\xfa\xc9\xd0\xce\xb4\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa3\xac\xbb\xf2\xd5\xdf\xb7\xa2\xcb\xcd\xd0\xc5\xbc\xfe\xb5\xc4\xc8\xa8\xcf\xde\xb1\xbb\xb7\xe2\xbd\xfb\xa1\xa3");
		pressreturn();
		return;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return;
	}

	//% getdata(1, 0, "收信人E-mail：", receiver, 65, DOECHO, YEA);
	getdata(1, 0, "\xca\xd5\xd0\xc5\xc8\xcb""E-mail\xa3\xba", receiver, 65, DOECHO, YEA);
	strtolower(genbuf, receiver);
	if (strstr(genbuf, ".bbs@"BBSHOST)
			|| strstr(genbuf, ".bbs@localhost")) {
		move(3, 0);
		//% prints("站内信件, 请用 (S)end 指令来寄\n");
		prints("\xd5\xbe\xc4\xda\xd0\xc5\xbc\xfe, \xc7\xeb\xd3\xc3 (S)end \xd6\xb8\xc1\xee\xc0\xb4\xbc\xc4\n");
		pressreturn();
	} else if (valid_addr(receiver)) {
		*quote_file = '\0';
		screen_clear();
		do_send(receiver, NULL);
	} else {
		move(3, 0);
		//% prints("收信人不正确, 请重新选取指令\n");
		prints("\xca\xd5\xd0\xc5\xc8\xcb\xb2\xbb\xd5\xfd\xc8\xb7, \xc7\xeb\xd6\xd8\xd0\xc2\xd1\xa1\xc8\xa1\xd6\xb8\xc1\xee\n");
		pressreturn();
	}
	screen_clear();
	screen_flush();
}
#endif

#ifdef INTERNET_EMAIL
static int bbs_sendmail(const char *fname, const char *title, const char *receiver, int filter, int mime)
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

	//% fprintf(fout, "X-Disclaimer: %s 对本信内容恕不负责。\n", BoardName);
	fprintf(fout, "X-Disclaimer: %s \xb6\xd4\xb1\xbe\xd0\xc5\xc4\xda\xc8\xdd\xcb\xa1\xb2\xbb\xb8\xba\xd4\xf0\xa1\xa3\n", BoardName);
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
			string_remove_ansi_control_code(genbuf, genbuf);
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

int do_send(const char *userid, const char *title)
{
	struct postheader header;
	int lookupuserlevel; //added by roly 02.03.25
	struct fileheader newmessage;
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
		file_temp_name(tmp_fname, sizeof(tmp_fname));
		strcpy(filepath, tmp_fname);
		goto edit_mail_file;
	}
	/* end of kludge for internet mail */

	if (!getuser(userid))
	return -1;
	if (!(lookupuser.userlevel & PERM_READMAIL))
	return -3;

	if (is_blocked(userid))
		return -5;
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
	sprintf(fname, "M.%ld.A", time(NULL));
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
		header.reply = false;
		//% strcpy(header.title, "没主题");
		strcpy(header.title, "\xc3\xbb\xd6\xf7\xcc\xe2");
	} else {
		header.reply = true;
		strcpy(header.title, title);
	}
	header.postboard = NA;
	in_mail = YEA;

	setuserfile(genbuf, "signatures");
	ansimore2(genbuf, NA, 0, 18);
	strcpy(header.ds, userid);
	result = post_header(&header);
	if( result == -1 ) {
		screen_clear();
		return -2;
	}
	if( result == YEA) {
		memcpy(newmessage.title, header.title, sizeof(header.title));
		sprintf(save_title2, "{%.16s} %.60s", userid, newmessage.title);
	}
	do_quote(quote_file, filepath, header.include_mode, header.anonymous);

	if (internet_mail) {
#ifndef INTERNET_EMAIL
		//% prints("对不起，本站暂不提供 InterNet Mail 服务！");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xb1\xbe\xd5\xbe\xd4\xdd\xb2\xbb\xcc\xe1\xb9\xa9 InterNet Mail \xb7\xfe\xce\xf1\xa3\xa1");
		pressanykey();
#else
		int res;
		if (tui_edit(filepath, false, true, true, &header) != TUI_EDIT_SAVED) {
			unlink(filepath);
			screen_clear();
			return -2;
		}
		screen_clear();
		//% prints("信件即将寄给 %s \n", userid);
		prints("\xd0\xc5\xbc\xfe\xbc\xb4\xbd\xab\xbc\xc4\xb8\xf8 %s \n", userid);
		//% prints("标题为： %s \n", header.title);
		prints("\xb1\xea\xcc\xe2\xce\xaa\xa3\xba %s \n", header.title);
		//% if (askyn("确定要寄出吗", YEA, NA) == NA) {
		if (askyn("\xc8\xb7\xb6\xa8\xd2\xaa\xbc\xc4\xb3\xf6\xc2\xf0", YEA, NA) == NA) {
			//% prints("\n信件已取消...\n");
			prints("\n\xd0\xc5\xbc\xfe\xd2\xd1\xc8\xa1\xcf\xfb...\n");
			res = -2;
		} else {
			int filter=YEA;
#ifdef SENDMAIL_MIME_AUTOCONVERT
			int ans;
			//% ans = askyn("以 MIME 格式送信", NA, NA);
			ans = askyn("\xd2\xd4 MIME \xb8\xf1\xca\xbd\xcb\xcd\xd0\xc5", NA, NA);
			//% if (askyn("是否过滤ANSI控制符",YEA,NA) == NA)
			if (askyn("\xca\xc7\xb7\xf1\xb9\xfd\xc2\xcb""ANSI\xbf\xd8\xd6\xc6\xb7\xfb",YEA,NA) == NA)
			filter = NA;
			//% if (askyn("是否备份给自己", NA, NA) == YEA)
			if (askyn("\xca\xc7\xb7\xf1\xb1\xb8\xb7\xdd\xb8\xf8\xd7\xd4\xbc\xba", NA, NA) == YEA)
			mail_file(tmp_fname, currentuser.userid, save_title2);
			//% prints("请稍候, 信件传递中...\n");
			prints("\xc7\xeb\xc9\xd4\xba\xf2, \xd0\xc5\xbc\xfe\xb4\xab\xb5\xdd\xd6\xd0...\n");
			screen_flush();
			res = bbs_sendmail(tmp_fname, header.title, userid, filter,ans);
#else

			//% if (askyn("是否过滤ANSI控制符",YEA,NA) == NA)
			if (askyn("\xca\xc7\xb7\xf1\xb9\xfd\xc2\xcbANSI\xbf\xd8\xd6\xc6\xb7\xfb",YEA,NA) == NA)
			filter = NA;
			//% if (askyn("是否备份给自己", NA, NA) == YEA)
			if (askyn("\xca\xc7\xb7\xf1\xb1\xb8\xb7\xdd\xb8\xf8\xd7\xd4\xbc\xba", NA, NA) == YEA)
			mail_file(tmp_fname, currentuser.userid, save_title2);
			//% prints("请稍候, 信件传递中...\n");
			prints("\xc7\xeb\xc9\xd4\xba\xf2, \xd0\xc5\xbc\xfe\xb4\xab\xb5\xdd\xd6\xd0...\n");
			screen_flush();
			res = bbs_sendmail(tmp_fname, header.title, userid, filter);
#endif
		}
		unlink(tmp_fname);
		sprintf(genbuf, "mailed %s: %s", userid, header.title);
		report(genbuf, currentuser.userid);
		return res;
#endif
	} else {
		if (tui_edit(filepath, false, true, true, &header) != TUI_EDIT_SAVED) {
			unlink(filepath);
			screen_clear();
			return -2;
		}

		//backup
		screen_clear();
		//% if (askyn("是否备份给自己", NA, NA) == YEA)
		if (askyn("\xca\xc7\xb7\xf1\xb1\xb8\xb7\xdd\xb8\xf8\xd7\xd4\xbc\xba", NA, NA) == YEA)
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

int m_send(const char *userid)
{
	char uident[STRLEN];
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		screen_clear();
		move(4,0);
		//% prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		prints("\n\n        \xc4\xfa\xc9\xd0\xce\xb4\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa3\xac\xbb\xf2\xd5\xdf\xb7\xa2\xcb\xcd\xd0\xc5\xbc\xfe\xb5\xc4\xc8\xa8\xcf\xde\xb1\xbb\xb7\xe2\xbd\xfb\xa1\xa3");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (session_status() != ST_LUSERS && session_status() != ST_LAUSERS
			&& session_status() != ST_FRIEND && session_status() != ST_GMENU) {
		screen_move_clear(1);
		set_user_status(ST_SMAIL);
		//% usercomplete("收信人： ", uident);
		usercomplete("\xca\xd5\xd0\xc5\xc8\xcb\xa3\xba ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		}
	} else
	strcpy(uident, userid);
	set_user_status(ST_SMAIL);
	screen_clear();
	*quote_file = '\0';
	switch (do_send(uident, NULL)) {
		case -1:
		//% prints("收信者不正确\n");
		prints("\xca\xd5\xd0\xc5\xd5\xdf\xb2\xbb\xd5\xfd\xc8\xb7\n");
		break;
		case -2:
		//% prints("取消\n");
		prints("\xc8\xa1\xcf\xfb\n");
		break;
		case -3:
		//% prints("[%s] 无法收信\n", uident);
		prints("[%s] \xce\xde\xb7\xa8\xca\xd5\xd0\xc5\n", uident);
		break;
		case -4:
		//% prints("[%s] 信箱已满，无法收信\n",uident);
		prints("[%s] \xd0\xc5\xcf\xe4\xd2\xd1\xc2\xfa\xa3\xac\xce\xde\xb7\xa8\xca\xd5\xd0\xc5\n",uident);
		break;
		case -5:
		//% prints("[%s] 不想收到您的信件\n",uident);
		prints("[%s] \xb2\xbb\xcf\xeb\xca\xd5\xb5\xbd\xc4\xfa\xb5\xc4\xd0\xc5\xbc\xfe\n",uident);
		break;
		default:
		//% prints("信件已寄出\n");
		prints("\xd0\xc5\xbc\xfe\xd2\xd1\xbc\xc4\xb3\xf6\n");
	}
	pressreturn();
	return FULLUPDATE;
}

int read_mail(struct fileheader *fptr) {
	/****判断是否为sharedmail，如果是则从共享文件读取****/
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

static int mail_reply(int ent, struct fileheader *fileinfo, char *direct)
{
	char uid[STRLEN];
	char title[STRLEN];
	char *t;
	set_user_status(ST_SMAIL);
	sprintf(genbuf, "MAILER-DAEMON@%s", BBSHOST);
	if (strstr(fileinfo->owner, genbuf)) {
		ansimore("help/mailerror-explain", YEA);
		return FULLUPDATE;
	}
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		screen_clear();
		move(4,0);
		//% prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		prints("\n\n        \xc4\xfa\xc9\xd0\xce\xb4\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa3\xac\xbb\xf2\xd5\xdf\xb7\xa2\xcb\xcd\xd0\xc5\xbc\xfe\xb5\xc4\xc8\xa8\xcf\xde\xb1\xbb\xb7\xe2\xbd\xfb\xa1\xa3");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	screen_clear();
	strlcpy(uid, fileinfo->owner, STRLEN);
	if ((uid[strlen(uid) - 1] == '>') && strchr(uid, '<')) {
		t = strtok(uid, "<>");
		if (!valid_addr(t))
			t = strtok(NULL, "<>");
		if (t != NULL)
			strcpy(uid, t);
		else {
			//% prints("无法投递\n");
			prints("\xce\xde\xb7\xa8\xcd\xb6\xb5\xdd\n");
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
		//% prints("无法投递\n");
		prints("\xce\xde\xb7\xa8\xcd\xb6\xb5\xdd\n");
		break;
		case -2:
		//% prints("取消回信\n");
		prints("\xc8\xa1\xcf\xfb\xbb\xd8\xd0\xc5\n");
		break;
		case -3:
		//% prints("[%s] 无法收信\n", uid);
		prints("[%s] \xce\xde\xb7\xa8\xca\xd5\xd0\xc5\n", uid);
		break;
		case -4:
		//% prints("[%s] 信箱已满，无法收信\n", uid);
		prints("[%s] \xd0\xc5\xcf\xe4\xd2\xd1\xc2\xfa\xa3\xac\xce\xde\xb7\xa8\xca\xd5\xd0\xc5\n", uid);
		break;
		case -5:
		//% prints("[%s] 不想收到您的信件\n",uid);
		prints("[%s] \xb2\xbb\xcf\xeb\xca\xd5\xb5\xbd\xc4\xfa\xb5\xc4\xd0\xc5\xbc\xfe\n",uid);
		break;
		default:
		fileinfo->accessed[0] |= MAIL_REPLY;
		substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
		//% prints("信件已寄出\n");
		prints("\xd0\xc5\xbc\xfe\xd2\xd1\xbc\xc4\xb3\xf6\n");
	}
	pressreturn();
	return FULLUPDATE;
}

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
	//% prints("读取 %s 寄来的 '%s' ?\n", fptr->owner, fptr->title);
	prints("\xb6\xc1\xc8\xa1 %s \xbc\xc4\xc0\xb4\xb5\xc4 '%s' ?\n", fptr->owner, fptr->title);
	//prints("(Yes, or No): ");
	//% getdata(1, 0, "(Y)读取 (N)不读 (Q)离开 [Y]: ", genbuf, 3, DOECHO, YEA);
	getdata(1, 0, "(Y)\xb6\xc1\xc8\xa1 (N)\xb2\xbb\xb6\xc1 (Q)\xc0\xeb\xbf\xaa [Y]: ", genbuf, 3, DOECHO, YEA);
	if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
		screen_clear();
		return QUIT;
	}
	if (genbuf[0] != 'y' && genbuf[0] != 'Y' && genbuf[0] != '\0') {
		screen_clear();
		return 0;
	}
	read_mail(fptr);
	strlcpy(fname, genbuf, sizeof(fname));

	//mrd = 1;
	if (substitute_record(currmaildir, fptr, sizeof(*fptr), index))
		return -1;
	delete_it = NA;
	while (!done) {
		move(-1, 0);
		//% prints("(R)回信, (D)删除, (G)继续? [G]: ");
		prints("(R)\xbb\xd8\xd0\xc5, (D)\xc9\xbe\xb3\xfd, (G)\xbc\xcc\xd0\xf8? [G]: ");
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
		screen_clear();
		//% sprintf(genbuf, "删除信件 [%-.55s]", fptr->title);
		sprintf(genbuf, "\xc9\xbe\xb3\xfd\xd0\xc5\xbc\xfe [%-.55s]", fptr->title);
		if (askyn(genbuf, NA, NA) == YEA) {
			sprintf(genbuf, "mail/%c/%s/%s",
					toupper(currentuser.userid[0]), currentuser.userid,
					fptr->filename);
			unlink(genbuf);
			delmsgs[delcnt++] = index;
		}
	}
	screen_clear();
	return 0;
}

int m_new(void)
{
	screen_clear();
	mrd = 0;
	set_user_status(ST_RMAIL);
	read_new_mail(NULL, 0, NULL);
	apply_record(currmaildir, read_new_mail, sizeof(struct fileheader), 0,
			1, 0, false);
	while (delcnt--)
		delete_record(currmaildir, sizeof(struct fileheader),
				delmsgs[delcnt], NULL, NULL);
	if (!mrd) {
		screen_clear();
		move(10, 30);
		//% prints("您现在没有新信件!");
		prints("\xc4\xfa\xcf\xd6\xd4\xda\xc3\xbb\xd3\xd0\xd0\xc2\xd0\xc5\xbc\xfe!");
		pressanykey();
	}
	return -1;
}

int mailtitle(void)
{
	int total, used;
	total=getmailboxsize(currentuser.userlevel) ;
	used=getmailsize(currentuser.userid);
	//% showtitle("信件选单    ", BoardName);
	showtitle("\xd0\xc5\xbc\xfe\xd1\xa1\xb5\xa5    ", BoardName);
	//% prints(" 离开[\033[1;32m←\033[m,\033[1;32me\033[m] 选择[\033[1;32m↑\033[m, \033[1;32m↓\033[m] 阅读信件[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 回 信[\033[1;32mR\033[m] 砍信／清除旧信[\033[1;32md\033[m,\033[1;32mD\033[m] 求助[\033[1;32mh\033[m]\033[m\n");
	prints(" \xc0\xeb\xbf\xaa[\033[1;32m\xa1\xfb\033[m,\033[1;32me\033[m] \xd1\xa1\xd4\xf1[\033[1;32m\xa1\xfc\033[m, \033[1;32m\xa1\xfd\033[m] \xd4\xc4\xb6\xc1\xd0\xc5\xbc\xfe[\033[1;32m\xa1\xfa\033[m,\033[1;32mRtn\033[m] \xbb\xd8 \xd0\xc5[\033[1;32mR\033[m] \xbf\xb3\xd0\xc5\xa3\xaf\xc7\xe5\xb3\xfd\xbe\xc9\xd0\xc5[\033[1;32md\033[m,\033[1;32mD\033[m] \xc7\xf3\xd6\xfa[\033[1;32mh\033[m]\033[m\n");
	prints(
			//% "\033[1;44m 编号   发信者        日期   标题    (\033[33m您的信箱容量为[%5dK]，当前已用[%4dK]\033[37m) \033[m\n",
			"\033[1;44m \xb1\xe0\xba\xc5   \xb7\xa2\xd0\xc5\xd5\xdf        \xc8\xd5\xc6\xda   \xb1\xea\xcc\xe2    (\033[33m\xc4\xfa\xb5\xc4\xd0\xc5\xcf\xe4\xc8\xdd\xc1\xbf\xce\xaa[%5dK]\xa3\xac\xb5\xb1\xc7\xb0\xd2\xd1\xd3\xc3[%4dK]\033[37m) \033[m\n",
			total, used);
	screen_clrtobot();
	return 0;
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
		screen_clear();
		move(4, 0);
		if (currentuser.nummails > maxmail)
			//% prints("您的私人信件高达 %d 封, 您的信件上限: %d 封\n",
			prints("\xc4\xfa\xb5\xc4\xcb\xbd\xc8\xcb\xd0\xc5\xbc\xfe\xb8\xdf\xb4\xef %d \xb7\xe2, \xc4\xfa\xb5\xc4\xd0\xc5\xbc\xfe\xc9\xcf\xcf\xde: %d \xb7\xe2\n",
				currentuser.nummails, maxmail);
		if (mailsize > maxsize)
			//% prints("您的信件容量高达 %d K，您的容量上限: %d K\n",
			prints("\xc4\xfa\xb5\xc4\xd0\xc5\xbc\xfe\xc8\xdd\xc1\xbf\xb8\xdf\xb4\xef %d K\xa3\xac\xc4\xfa\xb5\xc4\xc8\xdd\xc1\xbf\xc9\xcf\xcf\xde: %d K\n",
				mailsize, maxsize);
		//% prints("您的私人信件已经超限, 请整理信箱，"
		prints("\xc4\xfa\xb5\xc4\xcb\xbd\xc8\xcb\xd0\xc5\xbc\xfe\xd2\xd1\xbe\xad\xb3\xac\xcf\xde, \xc7\xeb\xd5\xfb\xc0\xed\xd0\xc5\xcf\xe4\xa3\xac"
			//% "否则无法使用本站的送信功能。\n");
			"\xb7\xf1\xd4\xf2\xce\xde\xb7\xa8\xca\xb9\xd3\xc3\xb1\xbe\xd5\xbe\xb5\xc4\xcb\xcd\xd0\xc5\xb9\xa6\xc4\xdc\xa1\xa3\n");
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
	/****判断是否是Type2的共享文件:文件名sharedmail/mailall.$userid.$time****/
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
		if (!valid_addr(t))
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
		//% sprintf(title, "★ %s", ent->title);
		sprintf(title, "\xa1\xef %s", ent->title);
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
	 //% sprintf(buf, " %s%3d[m %c %-12.12s %s%6.6s[m  ★  %s%.47s[m"
	 sprintf(buf, " %s%3d[m %c %-12.12s %s%6.6s[m  \xa1\xef  %s%.47s[m"
	 ,same ? c2 : "", num, status, b2, color, date, same ? c2 : "", ent->title);
	 }
	 */
	//End IAMFAT
	return buf;
}

static int show_mail_info(int ent, struct fileheader *fileinfo, char *direct)
{
	char filepath[HOMELEN];
	setmfile(filepath, currentuser.userid, fileinfo->filename);

	struct stat filestat;
	if (stat(filepath, &filestat) < 0) {
		screen_clear();
		move(10, 30);
		//% prints("对不起，当前文章不存在！\n", filepath);
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xb5\xb1\xc7\xb0\xce\xc4\xd5\xc2\xb2\xbb\xb4\xe6\xd4\xda\xa3\xa1\n", filepath);
		pressanykey();
		screen_clear();
		return FULLUPDATE;
	}

	bool unread = !(fileinfo->accessed[0] & FILE_READ);
	const char *type;
	if (fileinfo->accessed[0] & MAIL_REPLY) {
		if (fileinfo->accessed[0] & FILE_MARKED)
			//% type = "保留且已回复";
			type = "\xb1\xa3\xc1\xf4\xc7\xd2\xd2\xd1\xbb\xd8\xb8\xb4";
		else
			//% type = "已回复";
			type = "\xd2\xd1\xbb\xd8\xb8\xb4";
	} else if (fileinfo->accessed[0] & FILE_MARKED)
		//% type = "保留";
		type = "\xb1\xa3\xc1\xf4";
	else
		//% type = "普通";
		type = "\xc6\xd5\xcd\xa8";

	screen_clear();
	move(0, 0);
	//% prints("%s的详细信息:\n\n", "邮箱信件");
	prints("%s\xb5\xc4\xcf\xea\xcf\xb8\xd0\xc5\xcf\xa2:\n\n", "\xd3\xca\xcf\xe4\xd0\xc5\xbc\xfe");
	//% prints("序    号:     第 %d %s\n", ent, "封");
	prints("\xd0\xf2    \xba\xc5:     \xb5\xda %d %s\n", ent, "\xb7\xe2");
	//% prints("标    题:     %s\n", fileinfo->title);
	prints("\xb1\xea    \xcc\xe2:     %s\n", fileinfo->title);
	//% prints("发 信 人:     %s\n", fileinfo->owner);
	prints("\xb7\xa2 \xd0\xc5 \xc8\xcb:     %s\n", fileinfo->owner);
	fb_time_t filetime = strtoul(fileinfo->filename + 2, NULL, 10);
	//% prints("时    间:     %s\n", format_time(filetime, TIME_FORMAT_ZH));
	prints("\xca\xb1    \xbc\xe4:     %s\n", format_time(filetime, TIME_FORMAT_ZH));
	//% prints("阅读状态:     %s\n", unread ? "未读" : "已读");
	prints("\xd4\xc4\xb6\xc1\xd7\xb4\xcc\xac:     %s\n", unread ? "\xce\xb4\xb6\xc1" : "\xd2\xd1\xb6\xc1");
	//% prints("文章类型:     %s\n", type);
	prints("\xce\xc4\xd5\xc2\xc0\xe0\xd0\xcd:     %s\n", type);
	//% prints("大    小:     %d 字节\n", filestat.st_size);
	prints("\xb4\xf3    \xd0\xa1:     %d \xd7\xd6\xbd\xda\n", filestat.st_size);

	char link[STRLEN];
	snprintf(link, sizeof(link), "http://%s/bbs/mailcon?f=%s&n=%d\n",
			BBSHOST, fileinfo->filename, ent - 1);
	//% prints("URL 地址:\n%s", link);
	prints("URL \xb5\xd8\xd6\xb7:\n%s", link);
	pressanykey();
	return FULLUPDATE;
}

int mail_read(int ent, struct fileheader *fileinfo, char *direct)
{
	char buf[512], notgenbuf[128];
	char *t;
	int readpn;
	char done = NA, delete_it, replied;
	screen_clear();
	readpn = FULLUPDATE;
	setqtitle(fileinfo->title, 0);
	strcpy(buf, direct);
	if ((t = strrchr(buf, '/')) != NULL)
	*t = '\0';
	/****判断Type2公告的共享文件****/
	if(fileinfo->filename[0] == 's')
	strcpy(notgenbuf, fileinfo->filename);
	else
	sprintf(notgenbuf, "%s/%s", buf, fileinfo->filename);
	delete_it = replied = NA;
	while (!done) {
		ansimore(notgenbuf, NA);
		move(-1, 0);
		//% prints("(R)回信, (D)删除, (G)继续? [G]: ");
		prints("(R)\xbb\xd8\xd0\xc5, (D)\xc9\xbe\xb3\xfd, (G)\xbc\xcc\xd0\xf8? [G]: ");
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
			show_mail_info(ent, fileinfo, direct);
			break;
			case 'D':
			case 'd':
			delete_it = YEA;
			default:
			done = YEA;
		}
	}
	if (delete_it)
	return mail_del(ent, fileinfo, direct); /* 修改信件之bug
	 * 加了return */
	else {
		fileinfo->accessed[0] |= FILE_READ;
		substitute_record(currmaildir, fileinfo, sizeof(*fileinfo), ent);
	}
	return readpn;
}

int mail_del(int ent, struct fileheader *fileinfo, char *direct)
{
	char buf[512];
	char *t;
	extern int cmpfilename();
	extern int SR_BMDELFLAG;

	if(SR_BMDELFLAG==NA)
	{
		//% sprintf(genbuf, "删除信件 [%-.55s]", fileinfo->title);
		sprintf(genbuf, "\xc9\xbe\xb3\xfd\xd0\xc5\xbc\xfe [%-.55s]", fileinfo->title);
		if (askyn(genbuf, NA, YEA) == NA) {
			screen_move_clear(-1);
			//% prints("放弃删除信件...");
			prints("\xb7\xc5\xc6\xfa\xc9\xbe\xb3\xfd\xd0\xc5\xbc\xfe...");
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
	if (!SR_BMDELFLAG) {
		screen_move_clear(-1);
		//% prints("删除失败...");
		prints("\xc9\xbe\xb3\xfd\xca\xa7\xb0\xdc...");
		egetch();
	}
	return PARTUPDATE;
}

int _mail_forward(const char *path, const struct fileheader *fp, bool uuencode)
{
	if (!HAS_PERM(PERM_FORWARD))
		return DONOTHING;

	if (fp->filename[0] == 's') {
		//% prints("Type 2公告禁止转信!\n");
		prints("Type 2\xb9\xab\xb8\xe6\xbd\xfb\xd6\xb9\xd7\xaa\xd0\xc5!\n");
		return DONOTHING;
	}

	doforward(path, fp, uuencode);
	pressreturn();
	screen_clear();
	return FULLUPDATE;
}

int mail_forward(int ent, struct fileheader *fileinfo, const char *direct)
{
	return _mail_forward(direct, fileinfo, false);
}

int mail_u_forward(int ent, struct fileheader *fileinfo, const char *direct)
{
	return _mail_forward(direct, fileinfo, true);
}

int mail_mark(int ent, struct fileheader *fileinfo, char *direct)
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
extern int tui_cross_post_legacy(const char *file, const char *title);

static int do_cross(int ent, struct fileheader *fp, char *direct)
{
	set_safe_record();
	if (!HAS_PERM(PERM_POST) || digestmode == ATTACH_MODE)
		return DONOTHING;

	if (fp->filename[0] == 's') {
		return DONOTHING;
	}

	char file[HOMELEN];
	setmfile(file, currentuser.userid, fp->filename);

	return tui_cross_post_legacy(file, fp->title);
}

struct one_key mail_comms[] = {
		{ 'd', mail_del },
		{ 'D', del_range },
		{ 'b', SR_BMfunc },
		{ Ctrl('P'), m_send },
		{ 'E', edit_post },
		{ 'r', mail_read },
		{ 'R', mail_reply },
		{ 'm', mail_mark },
		{ 'i', Save_post },
		{'I', Import_post },
		{ KEY_TAB, show_user_notes },
#ifdef INTERNET_EMAIL
		{ 'F', mail_forward },
		{ 'U', mail_u_forward },
#endif
		{ 'a', auth_search_down },
		{ 'A', auth_search_up },
		{ '/', t_search_down },
		{ '?', t_search_up },
		{ '\'', post_search_down },
		{ '\"', post_search_up },
		{ ']', thread_down },
		{ '[', thread_up },
		{ Ctrl('A'), show_author },
		{ '\\', SR_last },
		{ '=', SR_first },
		{ 'l', msg_more },
		{ Ctrl('C'), do_cross },
		{ Ctrl('S'), SR_read },
		{ 'p', SR_read },
		{ Ctrl('X'), SR_read },
		{ Ctrl('U'), SR_author },
		{ 'h', mailreadhelp },
		{ Ctrl('J'), mailreadhelp },
		{ '!', Q_Goodbye },
		{ 'S', s_msg },
		{ '*', show_mail_info },
		{ 'Z', msg_author },
		{ '\0', NULL }
};

int m_read(void)
{
	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;

	tui_suppress_notice(true);
	in_mail = YEA;
	i_read(ST_RMAIL, currmaildir, mailtitle, maildoent, &mail_comms[0],
			sizeof(struct fileheader));
	in_mail = NA;
	tui_suppress_notice(false);
	return 0;
}

static int listfilecontent(char *fname, int y)
{
	FILE *fp;
	int x = 0, cnt = 0, max = 0, len;
	//char    u_buf[20], line[STRLEN], *nick;
	char u_buf[20], line[512], *nick;
	//modified by roly 02.03.22 缓存区溢出
	move(y, x);
	CreateNameList();
	strcpy(genbuf, fname);
	if ((fp = fopen(genbuf, "r")) == NULL) {
		prints("(none)\n");
		return 0;
	}
	while (fgets(genbuf, 1024, fp) != NULL) {
		strtok(genbuf, " \n\r\t");
		strlcpy(u_buf, genbuf, 20);
		u_buf[19] = '\0';
		if (!AddNameList(u_buf))
			continue;
		nick = (char *) strtok(NULL, "\n\r\t");
		if (nick != NULL) {
			while (*nick == ' ')
				nick++;
			if (*nick == '\0')
				nick = NULL;
		}
		if (nick == NULL) {
			strcpy(line, u_buf);
		} else {
			sprintf(line, "%-12s%s", u_buf, nick);
		}
		if ((len = strlen(line)) > max)
			max = len;
		if (x + len > 78)
			line[78 - x] = '\0';
		prints("%s", line);
		cnt++;
		if ((++y) >= screen_lines() - 1) {
			y = 3;
			x += max + 2;
			max = 0;
			if (x > 70)
				break;
		}
		move(y, x);
	}
	fclose(fp);
	if (cnt == 0)
		prints("(none)\n");
	return cnt;
}

static db_res_t *load_names_of_follows(void)
{
	return db_query("SELECT u.name"
			" FROM follows f JOIN alive_users u ON f.user_id = u.id"
			" WHERE f.follower = %"DBIdUID, session_uid());
}

static int do_gsend(char **userid, char *title, int num, char current_maillist)
{
	struct stat st;
	char filepath[STRLEN], tmpfile[STRLEN];
	int cnt, result;
	FILE *mp;
	char s_current_maillist[2] = {0, 0};
	extern int cmpfnames();

	s_current_maillist[0] = current_maillist;
	in_mail = YEA;
	sprintf(genbuf, "%s", currentuser.userid);
	struct postheader header;
	header.reply = false;
	//% strcpy(header.title, "没主题");
	strcpy(header.title, "\xc3\xbb\xd6\xf7\xcc\xe2");
	//% strcpy(header.ds, "寄信给一群人");
	strcpy(header.ds, "\xbc\xc4\xd0\xc5\xb8\xf8\xd2\xbb\xc8\xba\xc8\xcb");
	header.postboard = NA;
	file_temp_name(tmpfile, sizeof(tmpfile));
	result = post_header(&header);
	if( result == -1) {
		screen_clear();
		return -2;
	}
	if (result == YEA) {
		char title[sizeof(header.title)];
		//% snprintf(title, sizeof(title), "[群体信件] %s", header.title);
		snprintf(title, sizeof(title), "[\xc8\xba\xcc\xe5\xd0\xc5\xbc\xfe] %s", header.title);
		strlcpy(header.title, title, sizeof(header.title));
	}
	do_quote(quote_file, tmpfile, header.include_mode, header.anonymous);
	if (tui_edit(tmpfile, false, true, true, &header) != TUI_EDIT_SAVED) {
		unlink(tmpfile);
		screen_clear();
		return -2;
	}
	screen_clear();
	//% prints("[5;1;32m正在寄件中，请稍候...[m");
	prints("[5;1;32m\xd5\xfd\xd4\xda\xbc\xc4\xbc\xfe\xd6\xd0\xa3\xac\xc7\xeb\xc9\xd4\xba\xf2...[m");

	db_res_t *res = NULL;
	if (G_SENDMODE == 2) {
		char maillists[STRLEN];
		setuserfile(maillists, "maillist");
		if (current_maillist != '0')
		strcat(maillists, s_current_maillist);
		if ((mp = fopen(maillists, "r")) == NULL) {
			return -3;
		}
		res = load_names_of_follows();
	}

	for (cnt = 0; cnt < num; cnt++) {
		char uid[13];
		char buf[STRLEN];
		switch (G_SENDMODE) {
			case 1:
				if (cnt >= db_res_rows(res))
					break;
				strlcpy(uid, db_get_value(res, cnt, 0), sizeof(uid));
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
		if (is_blocked(uid))
			continue;
		sprintf(filepath, "mail/%c/%s", toupper(uid[0]), uid);
		if (stat(filepath, &st) == -1) {
			if (mkdir(filepath, 0755) == -1) {
				if (G_SENDMODE == 2) {
					fclose(mp);
					db_clear(res);
				}
				return -1;
			}
		} else {
			if (!(st.st_mode & S_IFDIR)) {
				if (G_SENDMODE == 2) {
					fclose(mp);
					db_clear(res);
				}
				return -1;
			}
		}
		mail_file(tmpfile, uid, header.title);
		session_set_idle_cached();
	}
	unlink(tmpfile);
	screen_clear();
	if (G_SENDMODE == 2) {
		fclose(mp);
		db_clear(res);
	}
	return 0;
}

int g_send() {
	char uident[13], tmp[3];
	int cnt, i, n, fmode = NA;
	char maillists[STRLEN], buf[STRLEN];
	char current_maillist = '0';
	char s_current_maillist[2] = { 0, 0 };

	set_user_status(ST_SMAIL);
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		screen_clear();
		move(4, 0);
		//% prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		prints("\n\n        \xc4\xfa\xc9\xd0\xce\xb4\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa3\xac\xbb\xf2\xd5\xdf\xb7\xa2\xcb\xcd\xd0\xc5\xbc\xfe\xb5\xc4\xc8\xa8\xcf\xde\xb1\xbb\xb7\xe2\xbd\xfb\xa1\xa3");
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
		screen_clear();
		cnt = listfilecontent(maillists, 3);
		if (cnt > maxrecp - 10) {
			move(1, 0);
			//% prints("目前限制寄信给 [1m%d[m 人", maxrecp);
			prints("\xc4\xbf\xc7\xb0\xcf\xde\xd6\xc6\xbc\xc4\xd0\xc5\xb8\xf8 [1m%d[m \xc8\xcb", maxrecp);
		}
		move(2, 0);
		//% prints("现在是第 %c 个名单 (0~9)选择其他名单", current_maillist);
		prints("\xcf\xd6\xd4\xda\xca\xc7\xb5\xda %c \xb8\xf6\xc3\xfb\xb5\xa5 (0~9)\xd1\xa1\xd4\xf1\xc6\xe4\xcb\xfb\xc3\xfb\xb5\xa5", current_maillist);

		//% getdata(0, 0, "(A)增加 (D)删除 (I)引入好友 (C)清除目前名单 (E)放弃 (S)寄出? [S]： ",
		getdata(0, 0, "(A)\xd4\xf6\xbc\xd3 (D)\xc9\xbe\xb3\xfd (I)\xd2\xfd\xc8\xeb\xba\xc3\xd3\xd1 (C)\xc7\xe5\xb3\xfd\xc4\xbf\xc7\xb0\xc3\xfb\xb5\xa5 (E)\xb7\xc5\xc6\xfa (S)\xbc\xc4\xb3\xf6? [S]\xa3\xba ",
				tmp, 2, DOECHO, YEA);

		if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's' || tmp[0]
				== 'S') {
			break;
		}
		if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A' || tmp[0]
				== 'D') {
			move(1, 0);
			if (tmp[0] == 'a' || tmp[0] == 'A')
				/*
				 * 日  期: 2007.12.19
				 * 维护者: Anonomous
				 * 代码段: 从下面while(1)语句开始到while结束，一共34行。
				 * 目  的: 增加群信发信人的时候不需要每次都按A键，所有的操作一次按A
				 *         之后完成。
				 * 备  注: 这个做法其实不是很好，不过因为整个FB系统设计的局限性，没有
				 *         办法改成比较好的流程，只能在原本的流程基础上重复劳动。FB的
				 *         设计有点太死板，每次增加发信人的时候都只处理一个id，而且这
				 *         个处理过程是夹杂在其他操作中间的，整个流程的耦合度太高，没
				 *         办法拆分，只好采取下面的方式，每次增加发信人的时候重绘整个
				 *         屏幕，并且完成一次添加操作。希望以后会有更好的办法。-_-||
				 */
				while (1) {
					screen_clear();
					cnt = listfilecontent(maillists, 3);
					if (cnt > maxrecp - 10) {
						move(1, 0);
						//% prints("目前限制寄信给 [1m%d[m 人", maxrecp);
						prints("\xc4\xbf\xc7\xb0\xcf\xde\xd6\xc6\xbc\xc4\xd0\xc5\xb8\xf8 [1m%d[m \xc8\xcb", maxrecp);
					}
					move(2, 0);
					//% prints("现在是第 %c 个名单 (0~9)选择其他名单", current_maillist);
					prints("\xcf\xd6\xd4\xda\xca\xc7\xb5\xda %c \xb8\xf6\xc3\xfb\xb5\xa5 (0~9)\xd1\xa1\xd4\xf1\xc6\xe4\xcb\xfb\xc3\xfb\xb5\xa5", current_maillist);
					move(0, 0);
					//% prints("(A)增加 (D)删除 (I)引入好友 (C)清除目前名单 (E)放弃 (S)寄出? [S]： ");
					prints("(A)\xd4\xf6\xbc\xd3 (D)\xc9\xbe\xb3\xfd (I)\xd2\xfd\xc8\xeb\xba\xc3\xd3\xd1 (C)\xc7\xe5\xb3\xfd\xc4\xbf\xc7\xb0\xc3\xfb\xb5\xa5 (E)\xb7\xc5\xc6\xfa (S)\xbc\xc4\xb3\xf6? [S]\xa3\xba ");
					move(1, 0);
					//% usercomplete("请依次输入使用者代号(只按 ENTER 结束输入): ", uident);
					usercomplete("\xc7\xeb\xd2\xc0\xb4\xce\xca\xe4\xc8\xeb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5(\xd6\xbb\xb0\xb4 ENTER \xbd\xe1\xca\xf8\xca\xe4\xc8\xeb): ", uident);
					screen_move_clear(1);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(2, 0);
						//% prints("这个使用者代号是错误的.\n");
						prints("\xd5\xe2\xb8\xf6\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5\xca\xc7\xb4\xed\xce\xf3\xb5\xc4.\n");
						continue;
					}
					if (!(lookupuser.userlevel & PERM_READMAIL)) {
						move(2, 0);
						//% prints("无法送信给: [1m%s[m\n", lookupuser.userid);
						prints("\xce\xde\xb7\xa8\xcb\xcd\xd0\xc5\xb8\xf8: [1m%s[m\n", lookupuser.userid);
						continue;
					} else if (seek_in_file(maillists, uident)) {
						move(2, 0);
						//% prints("已经列为收件人之一 \n");
						prints("\xd2\xd1\xbe\xad\xc1\xd0\xce\xaa\xca\xd5\xbc\xfe\xc8\xcb\xd6\xae\xd2\xbb \n");
						continue;
					}
					snprintf(buf, sizeof(buf), "%s\n", uident);
					file_append(maillists, buf);
					cnt++;
				}
			else
				//% namecomplete("请依次输入使用者代号(只按 ENTER 结束输入): ", uident);
				namecomplete("\xc7\xeb\xd2\xc0\xb4\xce\xca\xe4\xc8\xeb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5(\xd6\xbb\xb0\xb4 ENTER \xbd\xe1\xca\xf8\xca\xe4\xc8\xeb): ", uident);
			screen_move_clear(1);
			if (uident[0] == '\0')
				continue;
			if (!getuser(uident)) {
				move(2, 0);
				//% prints("这个使用者代号是错误的.\n");
				prints("\xd5\xe2\xb8\xf6\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5\xca\xc7\xb4\xed\xce\xf3\xb5\xc4.\n");
				continue; //added by infotech. rubing 提供.防止加入不存在的使用者.
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
				/* 这一段case应该永远都执行不到，因为前面的部分已经完成了些操作，
				 //% * 保险起见，暂时保留。
				 * \xb1\xa3\xcf\xd5\xc6\xf0\xbc\xfb\xa3\xac\xd4\xdd\xca\xb1\xb1\xa3\xc1\xf4\xa1\xa3
				 * by Anonomous */
				if (!(lookupuser.userlevel & PERM_READMAIL)) {
					move(2, 0);
					//% prints("无法送信给: [1m%s[m\n", lookupuser.userid);
					prints("\xce\xde\xb7\xa8\xcb\xcd\xd0\xc5\xb8\xf8: [1m%s[m\n", lookupuser.userid);
					break;
				} else if (seek_in_file(maillists, uident)) {
					move(2, 0);
					//% prints("已经列为收件人之一 \n");
					prints("\xd2\xd1\xbe\xad\xc1\xd0\xce\xaa\xca\xd5\xbc\xfe\xc8\xcb\xd6\xae\xd2\xbb \n");
					break;
				}
				snprintf(buf, sizeof(buf), "%s\n", uident);
				file_append(maillists, buf);
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
			case 'i': {
					n = 0;
					screen_clear();

					db_res_t *res = load_names_of_follows();
					for (i = cnt; i < maxrecp && n < db_res_rows(res); i++) {
						int key;
						move(2, 0);
						const char *uname = db_get_value(res, n, 0);
						prints("%s\n", uname);
						move(3, 0);
						n++;
						//% prints("(A)全部加入 (Y)加入 (N)不加入 (Q)结束? [Y]:");
						prints("(A)\xc8\xab\xb2\xbf\xbc\xd3\xc8\xeb (Y)\xbc\xd3\xc8\xeb (N)\xb2\xbb\xbc\xd3\xc8\xeb (Q)\xbd\xe1\xca\xf8? [Y]:");
						if (!fmode)
							key = terminal_getchar();
						else
							key = 'Y';
						if (key == 'q' || key == 'Q')
							break;
						if (key == 'A' || key == 'a') {
							fmode = YEA;
							key = 'Y';
						}
						if (key == '\0' || key == '\n' || key == 'y' || key == 'Y') {
							if (!getuser(uname)) {
								move(4, 0);
								//% prints("这个使用者代号是错误的.\n");
								prints("\xd5\xe2\xb8\xf6\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5\xca\xc7\xb4\xed\xce\xf3\xb5\xc4.\n");
								i--;
								continue;
							} else if (!(lookupuser.userlevel & PERM_READMAIL)) {
								move(4, 0);
								//% prints("无法送信给: [1m%s[m\n", lookupuser.userid);
								prints("\xce\xde\xb7\xa8\xcb\xcd\xd0\xc5\xb8\xf8: [1m%s[m\n", lookupuser.userid);
								i--;
								continue;
							} else if (seek_in_file(maillists, uname)) {
								i--;
								continue;
							}
							snprintf(buf, sizeof(buf), "%s\n", uname);
							file_append(maillists, buf);
							cnt++;
						}
					}
					db_clear(res);
					fmode = NA;
					screen_clear();
				}
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
		screen_clrtobot();
		if (cnt > maxrecp)
			cnt = maxrecp;
		move(3, 0);
		screen_clrtobot();
	}
	if (cnt > 0) {
		G_SENDMODE = 2;
		switch (do_gsend(NULL, NULL, cnt, current_maillist)) {
			case -1:
				//% prints("信件目录错误\n");
				prints("\xd0\xc5\xbc\xfe\xc4\xbf\xc2\xbc\xb4\xed\xce\xf3\n");
				break;
			case -2:
				//% prints("取消\n");
				prints("\xc8\xa1\xcf\xfb\n");
				break;
			default:
				//% prints("信件已寄出\n");
				prints("\xd0\xc5\xbc\xfe\xd2\xd1\xbc\xc4\xb3\xf6\n");
		}
		G_SENDMODE = 0;
		pressreturn();
	}
	return 0;
}
/*Add by SmallPig*/

/********************Type2公告共享文件 by Ashinmarch on 2008.3.30*********************/
/********************为了提高效率,免去黑名单、信件容量等判断**************************/
int sharedmail_file(const char *tmpfile, const char *userid, const char *title)
{
	struct fileheader newmessage;
	if (!getuser(userid))
		return -1;
	if (!(lookupuser.userlevel & PERM_READMAIL))
		return -3;
	memset(&newmessage, 0, sizeof(newmessage));
	sprintf(genbuf, "%s", currentuser.userid);
	strlcpy(newmessage.owner, genbuf, STRLEN);
	strlcpy(newmessage.title, title, STRLEN);
	strlcpy(newmessage.filename, tmpfile, STRLEN);

	char dir[HOMELEN];
	snprintf(dir, sizeof(dir), "mail/%c/%s/%s", toupper(userid[0]), userid, DOT_DIR);
	if (append_record(dir, &newmessage, sizeof(newmessage)) != -1)
		return -1;
	return 0;
}

/*Add by SmallPig*/
int ov_send() {
	int all, i;
	set_user_status(ST_SMAIL);
	/* Added by Amigo 2002.06.10. To add mail right check. */
	if (!HAS_PERM(PERM_MAIL)) {
		screen_clear();
		move(4, 0);
		//% prints("\n\n        您尚未完成注册，或者发送信件的权限被封禁。");
		prints("\n\n        \xc4\xfa\xc9\xd0\xce\xb4\xcd\xea\xb3\xc9\xd7\xa2\xb2\xe1\xa3\xac\xbb\xf2\xd5\xdf\xb7\xa2\xcb\xcd\xd0\xc5\xbc\xfe\xb5\xc4\xc8\xa8\xcf\xde\xb1\xbb\xb7\xe2\xbd\xfb\xa1\xa3");
		pressreturn();
		return 0;
	}
	/* Add end. */
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	move(1, 0);
	screen_clrtobot();
	move(2, 0);
	//% prints("寄信给好友名单中的人，目前本站限制仅可以寄给 [1m%d[m 位。\n", maxrecp);
	prints("\xbc\xc4\xd0\xc5\xb8\xf8\xba\xc3\xd3\xd1\xc3\xfb\xb5\xa5\xd6\xd0\xb5\xc4\xc8\xcb\xa3\xac\xc4\xbf\xc7\xb0\xb1\xbe\xd5\xbe\xcf\xde\xd6\xc6\xbd\xf6\xbf\xc9\xd2\xd4\xbc\xc4\xb8\xf8 [1m%d[m \xce\xbb\xa1\xa3\n", maxrecp);

	db_res_t *res = load_names_of_follows();
	if (!res || db_res_rows(res) < 1) {
		//% prints("您并没有设定好友。\n");
		prints("\xc4\xfa\xb2\xa2\xc3\xbb\xd3\xd0\xc9\xe8\xb6\xa8\xba\xc3\xd3\xd1\xa1\xa3\n");
		pressanykey();
		screen_clear();
		db_clear(res);
		return 0;
	} else {
		//% prints("名单如下：\n");
		prints("\xc3\xfb\xb5\xa5\xc8\xe7\xcf\xc2\xa3\xba\n");
	}
	G_SENDMODE = 1;
	all = (db_res_rows(res) >= maxrecp) ? maxrecp : db_res_rows(res);
	for (i = 0; i < all; i++) {
		prints("%-12s ", db_get_value(res, i, 0));
		if ((i + 1) % 6 == 0)
			outc('\n');
	}
	db_clear(res);

	pressanykey();
	switch (do_gsend(NULL, NULL, all, '0')) {
		case -1:
			//% prints("信件目录错误\n");
			prints("\xd0\xc5\xbc\xfe\xc4\xbf\xc2\xbc\xb4\xed\xce\xf3\n");
			break;
		case -2:
			//% prints("信件取消\n");
			prints("\xd0\xc5\xbc\xfe\xc8\xa1\xcf\xfb\n");
			break;
		default:
			//% prints("信件已寄出\n");
			prints("\xd0\xc5\xbc\xfe\xd2\xd1\xbc\xc4\xb3\xf6\n");
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

static int forward_file(const char *receiver, const char *file,
		const char *gbk_title, bool uuencode)
{
	add_crossinfo(file, false);

	char fname[HOMELEN];
	if (uuencode) {
		char buf[STRLEN];
		file_temp_name(fname, sizeof(fname));
		snprintf(buf, sizeof(buf), "uuencode %s fb-bbs.%05d > %s",
				file, session_pid(), fname);
		system(buf);
	} else {
		strlcpy(fname, file, sizeof(fname));
	}

	char title[STRLEN];
	//% if (!strstr(gbk_title, "(转寄)"))
	if (!strstr(gbk_title, "(\xd7\xaa\xbc\xc4)"))
		//% snprintf(title, sizeof(title), "%.70s(转寄)", gbk_title);
		snprintf(title, sizeof(title), "%.70s(\xd7\xaa\xbc\xc4)", gbk_title);
	else
		strlcpy(title, gbk_title, sizeof(title));

	int ret = mail_file(fname, receiver, title);
	if (uuencode)
		unlink(fname);
	return ret;
}

extern char fromhost[];

int tui_forward(const char *file, const char *gbk_title, bool uuencode)
{
	static char address[STRLEN];

	if (!HAS_PERM(PERM_MAIL))
		return BBS_EACCES;
	if (check_maxmail())
		return BBS_EMAILQE;

	if (address[0] == '\0')
		strlcpy(address, currentuser.userid, sizeof(address));

	char receiver[STRLEN];
	screen_clear();
	if (HAS_PERM(PERM_SETADDR)) {
		//% prints("本站目前只提供站内转信，请输入要转寄的帐号名。\n"
		prints("\xb1\xbe\xd5\xbe\xc4\xbf\xc7\xb0\xd6\xbb\xcc\xe1\xb9\xa9\xd5\xbe\xc4\xda\xd7\xaa\xd0\xc5\xa3\xac\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xd7\xaa\xbc\xc4\xb5\xc4\xd5\xca\xba\xc5\xc3\xfb\xa1\xa3\n"
				//% "请直接按 Enter 接受括号内提示的地址, 或者输入其他地址\n"
				"\xc7\xeb\xd6\xb1\xbd\xd3\xb0\xb4 Enter \xbd\xd3\xca\xdc\xc0\xa8\xba\xc5\xc4\xda\xcc\xe1\xca\xbe\xb5\xc4\xb5\xd8\xd6\xb7, \xbb\xf2\xd5\xdf\xca\xe4\xc8\xeb\xc6\xe4\xcb\xfb\xb5\xd8\xd6\xb7\n"
				//% "把信件转寄给 [%s]\n", address);
				"\xb0\xd1\xd0\xc5\xbc\xfe\xd7\xaa\xbc\xc4\xb8\xf8 [%s]\n", address);
		prints("==>");
		usercomplete(NULL, receiver);
	} else {
		strlcpy(receiver, currentuser.userid, sizeof(receiver));
	}

	if (receiver[0] != '\0')
		strlcpy(address, receiver, sizeof(address));
	else
		strlcpy(receiver, address, sizeof(receiver));

	char buf[STRLEN];
	//% snprintf(buf, sizeof(buf), "确定将文章寄给 %s 吗", address);
	snprintf(buf, sizeof(buf), "\xc8\xb7\xb6\xa8\xbd\xab\xce\xc4\xd5\xc2\xbc\xc4\xb8\xf8 %s \xc2\xf0", address);
	if (!askyn(buf, YEA, NA))
		return 1;

	struct userec user;
	if (!getuserec(address, &user))
		return BBS_EINTNL;
	if (getmailboxsize(user.userlevel) * 2 < getmailsize(user.userid)) {
		//% prints("[%s] 信箱容量已满，无法收信。\n",address);
		prints("[%s] \xd0\xc5\xcf\xe4\xc8\xdd\xc1\xbf\xd2\xd1\xc2\xfa\xa3\xac\xce\xde\xb7\xa8\xca\xd5\xd0\xc5\xa1\xa3\n",address);
		return BBS_ERMQE;
	}
	if (is_blocked(user.userid))
		return BBS_EBLKLST;

	int maxmail = getmailboxhold(user.userlevel);
	if (getmailnum(user.userid) > maxmail * 2) {
		//% prints("[%s] 信箱已满，无法收信。\n", address);
		prints("[%s] \xd0\xc5\xcf\xe4\xd2\xd1\xc2\xfa\xa3\xac\xce\xde\xb7\xa8\xca\xd5\xd0\xc5\xa1\xa3\n", address);
		return BBS_ERMQE;
	}

	char tmpfile[HOMELEN];
	file_temp_name(tmpfile, sizeof(tmpfile));
	f_cp(file, tmpfile, O_CREAT);

	//% if (askyn("是否修改文章内容", NA, NA)) {
	if (askyn("\xca\xc7\xb7\xf1\xd0\xde\xb8\xc4\xce\xc4\xd5\xc2\xc4\xda\xc8\xdd", NA, NA)) {
		if (tui_edit(tmpfile, false, false, false, NULL) != TUI_EDIT_SAVED) {
			//% if (askyn("是否寄出未修改的文章", YEA, NA) == 0) {
			if (askyn("\xca\xc7\xb7\xf1\xbc\xc4\xb3\xf6\xce\xb4\xd0\xde\xb8\xc4\xb5\xc4\xce\xc4\xd5\xc2", YEA, NA) == 0) {
				unlink(tmpfile);
				screen_clear();
				return 1;
			}
		} else {
			FILE *fp = fopen(tmpfile, "a");
			if (fp) {
				//% fprintf(fp, "\n--\n\033[1;36m※ 修改:·%s 于 %16.16s 修改本文·"
				fprintf(fp, "\n--\n\033[1;36m\xa1\xf9 \xd0\xde\xb8\xc4:\xa1\xa4%s \xd3\xda %16.16s \xd0\xde\xb8\xc4\xb1\xbe\xce\xc4\xa1\xa4"
						"[FROM: %s]\033[m\n", currentuser.userid,
						format_time(fb_time(), TIME_FORMAT_ZH) + 6,
						mask_host(fromhost));
				fclose(fp);
			}
		}
		screen_clear();
	}

	//% prints("转寄信件给 %s, 请稍候....\n", address);
	prints("\xd7\xaa\xbc\xc4\xd0\xc5\xbc\xfe\xb8\xf8 %s, \xc7\xeb\xc9\xd4\xba\xf2....\n", address);
	screen_flush();

	int ret = forward_file(user.userid, tmpfile, gbk_title, uuencode);
	unlink(tmpfile);

	switch (ret) {
		case 0:
			//% prints("文章转寄完成!\n");
			prints("\xce\xc4\xd5\xc2\xd7\xaa\xbc\xc4\xcd\xea\xb3\xc9!\n");
			break;
		case BBS_EINTNL:
			//% prints("转寄失败: 系统发生错误.\n");
			prints("\xd7\xaa\xbc\xc4\xca\xa7\xb0\xdc: \xcf\xb5\xcd\xb3\xb7\xa2\xc9\xfa\xb4\xed\xce\xf3.\n");
			break;
		case -2:
			//% prints("转寄失败: 不正确的收信地址.\n");
			prints("\xd7\xaa\xbc\xc4\xca\xa7\xb0\xdc: \xb2\xbb\xd5\xfd\xc8\xb7\xb5\xc4\xca\xd5\xd0\xc5\xb5\xd8\xd6\xb7.\n");
			break;
		case BBS_EMAILQE:
			//% prints("您的信箱超限，暂时无法使用信件服务.\n");
			prints("\xc4\xfa\xb5\xc4\xd0\xc5\xcf\xe4\xb3\xac\xcf\xde\xa3\xac\xd4\xdd\xca\xb1\xce\xde\xb7\xa8\xca\xb9\xd3\xc3\xd0\xc5\xbc\xfe\xb7\xfe\xce\xf1.\n");
			break;
		case BBS_EACCES:
			//% prints("您没有发信权限，暂时无法使用信件服务.\n");
			prints("\xc4\xfa\xc3\xbb\xd3\xd0\xb7\xa2\xd0\xc5\xc8\xa8\xcf\xde\xa3\xac\xd4\xdd\xca\xb1\xce\xde\xb7\xa8\xca\xb9\xd3\xc3\xd0\xc5\xbc\xfe\xb7\xfe\xce\xf1.\n");
			break;
		case BBS_EBLKLST:
			//% prints("对方不想收到您的信件.\n");
			prints("\xb6\xd4\xb7\xbd\xb2\xbb\xcf\xeb\xca\xd5\xb5\xbd\xc4\xfa\xb5\xc4\xd0\xc5\xbc\xfe.\n");
			break;
		default:
			//% prints("取消转寄...\n");
			prints("\xc8\xa1\xcf\xfb\xd7\xaa\xbc\xc4...\n");
	}
	return ret;
}

int doforward(const char *path, const struct fileheader *fp, bool uuencode)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "%s/%s", path, fp->filename);
	return tui_forward(file, fp->title, uuencode);
}
