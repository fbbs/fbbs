#include "bbs.h"
#include "record.h"
#include "fbbs/board.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/log.h"
#include "fbbs/mail.h"
#include "fbbs/session.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

#define MAXITEMS        1024
#define PATHLEN         256
#define A_PAGESIZE      (screen_lines() - 4)
#define MAXANNPATHS	40

#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2

void a_menu();

extern char BoardName[];
void a_prompt(); /* added by netty */
extern char ANN_LOG_PATH[];
int readonly=NA;

typedef struct {
	char title[84];//modified by money 2002-6-7
	char fname[80];
} ITEM;

int a_fmode = 0;

typedef struct MENU {
	ITEM *item[MAXITEMS];
	char mtitle[STRLEN];
	const char *path;
	int num, page, now;
	int level;
} MENU;

//返回时间t所对应的短日期格式
char *get_short_date(time_t t) {
	static char short_date[15];
	struct tm *tm;

	tm=localtime(&t);
	sprintf(short_date, "%04d.%02d.%02d", tm->tm_year+1900, tm->tm_mon+1,
			tm->tm_mday);
	return short_date;
}
char nowpath[STRLEN]="\0"; //add by Danielfree 06.12.6
//进入精华区时显示本函数运行结果
//	包括	前一行,是显示标题,还是显示信件信息
//			第二,三行,是显示可使用动作与编号
//			中间显示的是精华区列表
//			最后一行是功能键,版主与非版主不一样
void a_showmenu(MENU *pm) {
	struct stat st;
	struct tm *pt;
	char title[STRLEN * 2], kind[20];
	char fname[STRLEN];
	char ch;
	char buf[STRLEN], genbuf[STRLEN * 2];
	time_t mtime;
	int n, len;

	screen_clear();
	if (chkmail()) {
		prints("\033[5m");
		//% sprintf(genbuf, "[您有信件，按 M 看新信]");
		sprintf(genbuf, "[\xc4\xfa\xd3\xd0\xd0\xc5\xbc\xfe\xa3\xac\xb0\xb4 M \xbf\xb4\xd0\xc2\xd0\xc5]");
	} else {
		strcpy(genbuf, pm->mtitle);
	}
	memset(buf, ' ', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	len = strlen(genbuf);
	if (len >= sizeof(buf)) {
		genbuf[sizeof(buf) - 1] = '\0';
		len = sizeof(buf) - 1;
	}
	memcpy(buf + (sizeof(buf) - len) / 2, genbuf, len);
	prints("\033[1;44m%s\033[m\n", buf);
	//% prints("当前精华区目录 x%s\n", nowpath); //by Danielfree 06.12.6
	prints("\xb5\xb1\xc7\xb0\xbe\xab\xbb\xaa\xc7\xf8\xc4\xbf\xc2\xbc x%s\n", nowpath); //by Danielfree 06.12.6
	//% prints("\033[1;44;37m  编号 %-45s 整理者           %8s \033[m",
	prints("\033[1;44;37m  \xb1\xe0\xba\xc5 %-45s \xd5\xfb\xc0\xed\xd5\xdf           %8s \033[m",
			//% "[类别]                标    题", a_fmode ? "档案名称" : "编辑日期");
			"[\xc0\xe0\xb1\xf0]                \xb1\xea    \xcc\xe2", a_fmode ? "\xb5\xb5\xb0\xb8\xc3\xfb\xb3\xc6" : "\xb1\xe0\xbc\xad\xc8\xd5\xc6\xda");
	prints("\n");
	if (pm->num == 0) {
		//% prints("      << 目前没有文章 >>\n");
		prints("      << \xc4\xbf\xc7\xb0\xc3\xbb\xd3\xd0\xce\xc4\xd5\xc2 >>\n");
	}
	for (n = pm->page; n < pm->page + A_PAGESIZE && n < pm->num; n++) {
		strcpy(title, pm->item[n]->title);
		sprintf(fname, "%s", pm->item[n]->fname);
		sprintf(genbuf, "%s/%s", pm->path, fname);
		if (a_fmode &&(pm->level&PERM_BOARDS)!=0) {
			ch = (dashd(genbuf) ? '/' : ' ');
			fname[10] = '\0';
		} else {
			//if (dashf(genbuf) || ch == '/') { //modified by roly 02.03.18
			if (dashf(genbuf) || dashd(genbuf) || ch == '/') {
				stat(genbuf, &st);
				mtime = st.st_mtime;
			} else {
				mtime = time(0);
			}
			pt = localtime(&mtime);
			sprintf(fname, "[%02d.%02d.%02d]",
					pt->tm_year%100, pt->tm_mon + 1, pt->tm_mday);
			ch = ' ';
		}
		if (dashf(genbuf)) {
			//% strcpy(kind, "[[1;36m文件[m]");
			strcpy(kind, "[[1;36m\xce\xc4\xbc\xfe[m]");
		} else if (dashd(genbuf)) {
			//% strcpy(kind, "[[1;37m目录[m]");
			strcpy(kind, "[[1;37m\xc4\xbf\xc2\xbc[m]");
		} else {
			//% strcpy(kind, "[[1;32m错误[m]");
			strcpy(kind, "[[1;32m\xb4\xed\xce\xf3[m]");
		}
		//% if ( !strncmp(title, "[目录] ", 7) || !strncmp(title, "[文件] ", 7)
		if ( !strncmp(title, "[\xc4\xbf\xc2\xbc] ", 7) || !strncmp(title, "[\xce\xc4\xbc\xfe] ", 7)
				//% || !strncmp(title, "[连目] ", 7) || !strncmp(title, "[连文] ",
				|| !strncmp(title, "[\xc1\xac\xc4\xbf] ", 7) || !strncmp(title, "[\xc1\xac\xce\xc4] ",
				7))
			sprintf(genbuf, "%-s %-55.55s%-s%c", kind, title + 7, fname,
					ch);
		else
			sprintf(genbuf, "%-s %-55.55s%-s%c", kind, title, fname, ch);
		strlcpy(title, genbuf, STRLEN * 2);
		title[STRLEN * 2 - 1] = '\0';
		//Modified by IAMFAT 2002-05-30 3->4
		prints("  %4d %s\n", n + 1, title);
	}
	screen_clrtobot();
	screen_move_clear(-1);
	sprintf(
			genbuf,
			//% "[1;44;31m[版  主]  [33m说明 h│离开 q,←│新增文章 a│新增目录 g│编辑档案 E   [%s丝路模式] [m",
			"[1;44;31m[\xb0\xe6  \xd6\xf7]  [33m\xcb\xb5\xc3\xf7 h\xa9\xa6\xc0\xeb\xbf\xaa q,\xa1\xfb\xa9\xa6\xd0\xc2\xd4\xf6\xce\xc4\xd5\xc2 a\xa9\xa6\xd0\xc2\xd4\xf6\xc4\xbf\xc2\xbc g\xa9\xa6\xb1\xe0\xbc\xad\xb5\xb5\xb0\xb8 E   [%s\xcb\xbf\xc2\xb7\xc4\xa3\xca\xbd] [m",
			//% DEFINE(DEF_MULTANNPATH) ? "多" : "单");
			DEFINE(DEF_MULTANNPATH) ? "\xb6\xe0" : "\xb5\xa5");
	prints(
			"%s",
			(pm->level & PERM_BOARDS) ? genbuf
					//% : "[1;44;31m[功能键] [33m 说明 h│离开 q,←│移动游标 k,↑,j,↓│读取资料 Rtn,→               [m");
					: "[1;44;31m[\xb9\xa6\xc4\xdc\xbc\xfc] [33m \xcb\xb5\xc3\xf7 h\xa9\xa6\xc0\xeb\xbf\xaa q,\xa1\xfb\xa9\xa6\xd2\xc6\xb6\xaf\xd3\xce\xb1\xea k,\xa1\xfc,j,\xa1\xfd\xa9\xa6\xb6\xc1\xc8\xa1\xd7\xca\xc1\xcf Rtn,\xa1\xfa               [m");
}

//向精华区当前目录增加一个记录
//	显示名称为 title
//	硬盘存储名为 fname
//	但是只保存在内存中,并未实际存储在硬盘中
static void a_additem(MENU *pm, const char *title, const char *fname)
{
	ITEM *newitem;
	if (pm->num < MAXITEMS) {
		newitem = (ITEM *) malloc(sizeof(ITEM));
		memset(newitem, 0, sizeof(ITEM));
		strcpy(newitem->title, title);
		strcpy(newitem->fname, fname);
		pm->item[(pm->num)++] = newitem;
	}
}

//	引导进入精华区或者进入精华区的某个目录,加载所需要的信息
int a_loadnames(MENU *pm) {
	FILE *fn;
	ITEM litem;
	char buf[PATHLEN], *ptr;

	pm->num = 0;
	sprintf(buf, "%s/.Names", pm->path);
	if ((fn = fopen(buf, "r")) == NULL)
		return 0;
	while (fgets(buf, sizeof(buf), fn) != NULL) {
		//		memset(litem, 0, sizeof(ITEM));
		if ((ptr = strchr(buf, '\n')) != NULL)
			*ptr = '\0';
		if (strncmp(buf, "Name=", 5) == 0) {
			strlcpy(litem.title, buf + 5, 72);
			litem.title[71] = '\0';
		} else if (strncmp(buf, "Path=", 5) == 0) {
			if (strncmp(buf, "Path=~/", 7) == 0)
				strlcpy(litem.fname, buf + 7, 80);
			else
				strlcpy(litem.fname, buf + 5, 80);
			litem.fname[79] = '\0';
			// add judgement of OBOARDS by roly 02.02.26
			//modified by iamfat 2002.06.09
			//if ((!strstr(litem.title+38,"(BM: BMS)")||HAS_PERM(PERM_BOARDS))&&
			if ((!strstr(litem.title, "(BM: BMS)")||chk_currBM(currBM, 0))
					&&(!strstr(litem.title, "(BM: OBOARDS")
							||HAS_PERM(PERM_OBOARDS)) &&(!strstr(
					litem.title, "(BM: SYSOPS)") ||HAS_PERM(PERM_SYSOPS))
					&&(strncmp(litem.title, "<HIDE>", 6) ||!strncmp(
							litem.title+6, currentuser.userid,
							strlen(currentuser.userid)))) {//if
				a_additem(pm, litem.title, litem.fname);
			}
		} else if (strncmp(buf, "# Title=", 8) == 0) {
			//if (pm->mtitle[0] == '\0')
			strlcpy(pm->mtitle, buf + 8, STRLEN);
		}//if (strncmp(buf, "Name=", 5) == 0) {
	}
	fclose(fn);
	return 1;
}

//将pm所保留的信息写入到硬盘
void a_savenames(MENU *pm) {
	FILE *fn;
	ITEM *item;
	char fpath[PATHLEN];
	int n;

	sprintf(fpath, "%s/.Names", pm->path);
	if ((fn = fopen(fpath, "w")) == NULL)
		return;
	fprintf(fn, "#\n");
	//% if (!strncmp(pm->mtitle, "[目录] ", 7) || !strncmp(pm->mtitle, "[文件] ",
	if (!strncmp(pm->mtitle, "[\xc4\xbf\xc2\xbc] ", 7) || !strncmp(pm->mtitle, "[\xce\xc4\xbc\xfe] ",
			//% 7) || !strncmp(pm->mtitle, "[连目] ", 7) || !strncmp(pm->mtitle,
			7) || !strncmp(pm->mtitle, "[\xc1\xac\xc4\xbf] ", 7) || !strncmp(pm->mtitle,
			//% "[连文] ", 7)) {
			"[\xc1\xac\xce\xc4] ", 7)) {
		fprintf(fn, "# Title=%s\n", pm->mtitle + 7);
	} else {
		fprintf(fn, "# Title=%s\n", pm->mtitle);
	}
	fprintf(fn, "#\n");
	for (n = 0; n < pm->num; n++) {
		item = pm->item[n];
		//% if (!strncmp(item->title, "[目录] ", 7)||!strncmp(item->title,
		if (!strncmp(item->title, "[\xc4\xbf\xc2\xbc] ", 7)||!strncmp(item->title,
				//% "[文件] ", 7) ||!strncmp(item->title, "[连目] ", 7)||!strncmp(
				"[\xce\xc4\xbc\xfe] ", 7) ||!strncmp(item->title, "[\xc1\xac\xc4\xbf] ", 7)||!strncmp(
				//% item->title, "[连文] ", 7)) {
				item->title, "[\xc1\xac\xce\xc4] ", 7)) {
			fprintf(fn, "Name=%s\n", item->title + 7);
		} else {
			fprintf(fn, "Name=%s\n", item->title);
		}
		fprintf(fn, "Path=~/%s\n", item->fname);
		fprintf(fn, "Numb=%d\n", n + 1);
		fprintf(fn, "#\n");
	}
	fclose(fn);
	chmod(fpath, 0644);
}

void a_prompt(int bot, char* pmt, char* buf, int len) {
	screen_save_line(bot, true);
	screen_move_clear(bot);
	getdata(bot, 0, pmt, buf, len, DOECHO, YEA);
	screen_save_line(bot, false);
}

int a_Save(const char *gbk_title, const char *file, int nomsg, int full)
{
	char buf[STRLEN];
	if (!nomsg) {
		//% snprintf(buf, sizeof(buf), "确定将 [%-.40s] 存入暂存档吗", gbk_title);
		snprintf(buf, sizeof(buf), "\xc8\xb7\xb6\xa8\xbd\xab [%-.40s] \xb4\xe6\xc8\xeb\xd4\xdd\xb4\xe6\xb5\xb5\xc2\xf0", gbk_title);
		if (!askyn(buf, NA, YEA))
			return FULLUPDATE;
	}

	bool append = false;
	snprintf(buf, sizeof(buf), "bm/%s", currentuser.userid);
	if (dashf(buf)) {
		if (nomsg)
			append = true;
		else
			//% append = askyn("要附加在旧暂存档之後吗", NA, YEA);
			append = askyn("\xd2\xaa\xb8\xbd\xbc\xd3\xd4\xda\xbe\xc9\xd4\xdd\xb4\xe6\xb5\xb5\xd6\xae\xe1\xe1\xc2\xf0", NA, YEA);
	}

	char src[HOMELEN];
	if (in_mail)
		snprintf(src, sizeof(src), "mail/%c/%s/%s",
				toupper(currentuser.userid[0]), currentuser.userid, file);
	else
		strlcpy(src, file, sizeof(src));

	if (full)
		f_cp(src, buf, append ? O_APPEND : O_CREAT);
	else
		part_cp(src, buf, append ? "a+" : "w+");

	FILE *fp = fopen(buf, "a");
	if (fp) {
		fprintf(fp, "\r\n");
		fclose(fp);
	}

	if (!nomsg)
		//% 已将该文章存入暂存档, 请按<Enter>继续...
		presskeyfor("\xd2\xd1\xbd\xab\xb8\xc3\xce\xc4\xd5\xc2\xb4\xe6\xc8\xeb\xd4\xdd\xb4\xe6\xb5\xb5, \xc7\xeb\xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);
	return FULLUPDATE;
}

int a_Save2(char *path, char* key, struct fileheader* fileinfo, int nomsg) {

	char board[40];
	int ans = NA;
	FILE *fp;

	if (!nomsg) {
		//% sprintf(genbuf, "确定将 [%-.40s] 存入暂存档吗", fileinfo->title);
		sprintf(genbuf, "\xc8\xb7\xb6\xa8\xbd\xab [%-.40s] \xb4\xe6\xc8\xeb\xd4\xdd\xb4\xe6\xb5\xb5\xc2\xf0", fileinfo->title);
		if (askyn(genbuf, NA, YEA) == NA)
			return FULLUPDATE;
	}
	sprintf(board, "bm/%s", currentuser.userid);
	if (dashf(board)) {
		if (nomsg)
			ans = YEA;
		else
			//% ans = askyn("要附加在旧暂存档之後吗", NA, YEA);
			ans = askyn("\xd2\xaa\xb8\xbd\xbc\xd3\xd4\xda\xbe\xc9\xd4\xdd\xb4\xe6\xb5\xb5\xd6\xae\xe1\xe1\xc2\xf0", NA, YEA);
	}
	if (in_mail)
		sprintf(genbuf, "/bin/%s mail/%c/%s/%s %s %s", (ans) ? "cat"
				: "cp -r", toupper(currentuser.userid[0]),
				currentuser.userid, fileinfo->filename, (ans) ? ">>" : "",
				board);
	else
		sprintf(genbuf, "/bin/%s boards/%s/%s %s %s", (ans) ? "cat"
				: "cp -r", key, fileinfo->filename, (ans) ? ">>" : "",
				board);
	system(genbuf);
	fp=fopen(board, "a");//增加空行 added by money 2002.3.22
	fprintf(fp, "\r\n");
	fclose(fp);
	if (!nomsg)
		//% 已将该文章存入暂存档, 请按<Enter>继续...
		presskeyfor("\xd2\xd1\xbd\xab\xb8\xc3\xce\xc4\xd5\xc2\xb4\xe6\xc8\xeb\xd4\xdd\xb4\xe6\xb5\xb5, \xc7\xeb\xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);
	return FULLUPDATE;
}

int import_file(const char *title, const char *file, const char *path)
{
	MENU pm = { .path = path, .mtitle = "", };

	char currBM_bak[BM_LEN - 1];
	memcpy(currBM_bak, currBM, BM_LEN - 1);
	sprintf(currBM, "%s", currentuser.userid);
	a_loadnames(&pm);
	memcpy(currBM, currBM_bak, BM_LEN - 1);

	char fname[HOMELEN];
	sprintf(fname, "I%lX", time(0) + getpid() + getppid() + rand());

	char bname[PATHLEN];
	for (char *ip = fname + strlen(fname) - 1; ; ) {
		snprintf(bname, sizeof(bname), "%s/%s", pm.path, fname);
		if (dashf(bname)) {
			if (*ip == 'Z')
				ip++, *ip = 'A', *(ip + 1) = '\0';
			else
				(*ip)++;
		} else {
			break;
		}
	}

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	string_remove_ansi_control_code(gbk_title, title);
	ellipsis(gbk_title, 38);

	char title_buf[STRLEN];
	snprintf(title_buf, sizeof(title_buf), "%-38.38s %s ", title,
			currentuser.userid);

	a_additem(&pm, title_buf, fname);
	a_savenames(&pm);

	if (in_mail) {
		sprintf(genbuf, "/bin/cp -r mail/%c/%s/%s %s",
				toupper(currentuser.userid[0]), currentuser.userid,
				file, bname);
		system(genbuf);
	} else {
		f_cp(file, bname, 0);
	}

	for (int i = 0; i < pm.num; ++i)
		free(pm.item[i]);

	log_bm(LOG_BM_ANNOUNCE, 1);
	char buf[STRLEN];
	//% snprintf(buf, sizeof(buf), "%s %s收录 '%-20.20s..'\n", get_short_date(time(0)),
	snprintf(buf, sizeof(buf), "%s %s\xca\xd5\xc2\xbc '%-20.20s..'\n", get_short_date(time(0)),
			currentuser.userid, title);
	file_append(ANN_LOG_PATH, buf);
	return 0;
}

int a_Import(const char *title, const char *file, int nomsg)
{
	set_user_status(ST_DIGEST);

	char buf[256];
	sethomefile(buf, currentuser.userid, ".announcepath");

	FILE *fp = fopen(buf, "r");
	if (!fp) {
		//% 对不起, 您没有设定丝路. 请先设定丝路.
		presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xc3\xbb\xd3\xd0\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7. \xc7\xeb\xcf\xc8\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7.", -1);
		return DONOTHING;
	}
	fscanf(fp, "%s", buf);
	fclose(fp);

	if (!dashd(buf)) {
		//% 您设定的丝路已丢失, 请重新设定.
		presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xc9\xe8\xb6\xa8.", -1);
		return DONOTHING;
	}

	import_file(title, file, buf);

	if (!nomsg && !DEFINE(DEF_MULTANNPATH)) {
		//% 已将该文章放进精华区, 请按<Enter>继续...
		presskeyfor("\xd2\xd1\xbd\xab\xb8\xc3\xce\xc4\xd5\xc2\xb7\xc5\xbd\xf8\xbe\xab\xbb\xaa\xc7\xf8, \xc7\xeb\xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);
	}
	return FULLUPDATE;
}

// deardragon 0921

int a_menusearch(const char *key, char *found) {
	FILE *fn;
	char bname[20], flag='0';
	char buf[PATHLEN], *ptr;
	int searchmode = 0;
	if (key == NULL) {
		//% a_prompt(-1, "输入欲搜寻之讨论区名称: ", bname, 18);
		a_prompt(-1, "\xca\xe4\xc8\xeb\xd3\xfb\xcb\xd1\xd1\xb0\xd6\xae\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6: ", bname, 18);
		key = bname;
		searchmode = 1;
	}
	sprintf(buf, "0Announce/.Search");
	if (key[0] != '\0' && (fn = fopen(buf, "r")) != NULL) {
		while (fgets(buf, sizeof(buf), fn) != NULL) {
			if (searchmode && !strstr(buf, "groups/"))
				continue;
			ptr = strchr(buf, ':');
			if (!ptr)
				return 0;
			else {
				*ptr = '\0';
				ptr = strtok(ptr + 1, " \t\n");
			}
			if (strcaseeq(buf, key)) {
				board_t board;
				if (get_board(key, &board) && has_read_perm(&board)) {
					sprintf(found, "0Announce/%s", ptr);
					flag = '1';
				}
			}
		}
		fclose(fn);
		if (flag == '0') {
			//% 找不到您所输入的讨论区, 按<Enter>继续...
			presskeyfor("\xd5\xd2\xb2\xbb\xb5\xbd\xc4\xfa\xcb\xf9\xca\xe4\xc8\xeb\xb5\xc4\xcc\xd6\xc2\xdb\xc7\xf8, \xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);
			return 0;
		}
		return 1;
	}

	return 0;
}
int b_menusearch(MENU *pm) {
	char key[20];
	int i;
	//% a_prompt(-1, "输入欲搜寻目录之名称: ", key, 18);
	a_prompt(-1, "\xca\xe4\xc8\xeb\xd3\xfb\xcb\xd1\xd1\xb0\xc4\xbf\xc2\xbc\xd6\xae\xc3\xfb\xb3\xc6: ", key, 18);
	if (key[0] == '\0' || pm->num == 0)
		return 0;
	for (i=0; i< (pm->num); i++) {
		if (strstr(pm->item[i]->title, key) )
			return i;
	}
	return 0;
}

#ifdef INTERNET_EMAIL
void a_forward(char *path,ITEM* pitem,int mode) {
	struct fileheader fhdr;
	char fname[PATHLEN], *mesg;
	sprintf(fname, "%s/%s", path, pitem->fname);
	if (dashf(fname)) {
		strlcpy(fhdr.title, pitem->title, sizeof(fhdr.title));
		strlcpy(fhdr.filename, pitem->fname, sizeof(fhdr.filename));
		switch (doforward(path, &fhdr, mode)) {
			case 0:
				//% mesg = "文章转寄完成!\n";
				mesg = "\xce\xc4\xd5\xc2\xd7\xaa\xbc\xc4\xcd\xea\xb3\xc9!\n";
				break;
			case BBS_EINTNL:
				mesg = "System error!!.\n";
				break;
			case -2:
				mesg = "Invalid address.\n";
				break;
			case BBS_EBLKLST:
				//% mesg = "对方不想收到您的信件.\n";
				mesg = "\xb6\xd4\xb7\xbd\xb2\xbb\xcf\xeb\xca\xd5\xb5\xbd\xc4\xfa\xb5\xc4\xd0\xc5\xbc\xfe.\n";
				break;
			default:
				//% mesg = "取消转寄动作.\n";
				mesg = "\xc8\xa1\xcf\xfb\xd7\xaa\xbc\xc4\xb6\xaf\xd7\xf7.\n";
		}
		outs(mesg);
	} else {
		screen_move_clear(-1);
		//% prints("无法转寄此项目.\n");
		prints("\xce\xde\xb7\xa8\xd7\xaa\xbc\xc4\xb4\xcb\xcf\xee\xc4\xbf.\n");
	}
	pressanykey();

}
#endif

void a_newitem(MENU *pm, int mode) {
	char uident[STRLEN];
	char board[STRLEN], title[STRLEN];
	char fname[STRLEN], fpath[PATHLEN], fpath2[PATHLEN];
	char *mesg;
	FILE *pn;
	char head;
	srand(time(0));

	pm->page = 9999;
	head = 'X';
	switch (mode) {
		case ADDITEM:
			head = 'A'; /* article */
			break;
		case ADDGROUP:
			head = 'D'; /* directory */
			break;
		case ADDMAIL:
			sprintf(board, "bm/%s", currentuser.userid);
			if (!dashf(board)) {
				//% "请先至该讨论区将文章存入暂存档, 按<Enter>继续...
				presskeyfor("\xc7\xeb\xcf\xc8\xd6\xc1\xb8\xc3\xcc\xd6\xc2\xdb\xc7\xf8\xbd\xab\xce\xc4\xd5\xc2\xb4\xe6\xc8\xeb\xd4\xdd\xb4\xe6\xb5\xb5, \xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);
				return;
			}
			//% mesg = "请输入档案之英文名称(可含数字)：";
			mesg = "\xc7\xeb\xca\xe4\xc8\xeb\xb5\xb5\xb0\xb8\xd6\xae\xd3\xa2\xce\xc4\xc3\xfb\xb3\xc6(\xbf\xc9\xba\xac\xca\xfd\xd7\xd6)\xa3\xba";
			break;
	}
	/* edwardc.990320 system will assign a filename for you .. */
	sprintf(fname, "%c%lX", head, time(0) + getpid() + getppid() + rand());
	sprintf(fpath, "%s/%s", pm->path, fname);
	//% mesg = "请输入文件或目录之中文名称： ";
	mesg = "\xc7\xeb\xca\xe4\xc8\xeb\xce\xc4\xbc\xfe\xbb\xf2\xc4\xbf\xc2\xbc\xd6\xae\xd6\xd0\xce\xc4\xc3\xfb\xb3\xc6\xa3\xba ";
	a_prompt(-1, mesg, title, 35);
	if (*title == '\0')
		return;
	switch (mode) {
		case ADDITEM:
			if (editor(fpath, false, false, true, NULL) != EDITOR_SAVE)
				return;
			chmod(fpath, 0644);
			break;
		case ADDGROUP:
			if (strlen(fpath)>=230) {
				//% 对不起, 目录层次太深, 取消操做!
				presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xbf\xc2\xbc\xb2\xe3\xb4\xce\xcc\xab\xc9\xee, \xc8\xa1\xcf\xfb\xb2\xd9\xd7\xf6!", -1);
				return;
			}
			mkdir(fpath, 0755);
			chmod(fpath, 0755);
			break;
		case ADDMAIL:
			rename(board, fpath);
			break;
	}
	if (mode != ADDGROUP)
		sprintf(genbuf, "%-38.38s %s", title, currentuser.userid);
	else {
		/*Add by SmallPig*/
		screen_move_clear(1);
		//% getdata(1, 0, "版主: ", uident, 35, DOECHO, YEA);
		getdata(1, 0, "\xb0\xe6\xd6\xf7: ", uident, 35, DOECHO, YEA);
		if (uident[0] != '\0')
			sprintf(genbuf, "%-38.38s(BM: %s)", title, uident);
		else
			sprintf(genbuf, "%-38.38s", title);
	}
	a_additem(pm, genbuf, fname);
	a_savenames(pm);
	if (mode == ADDGROUP) {
		sprintf(fpath2, "%s/%s/.Names", pm->path, fname);
		if ((pn = fopen(fpath2, "w")) != NULL) {
			fprintf(pn, "#\n");
			fprintf(pn, "# Title=%s\n", genbuf);
			fprintf(pn, "#\n");
			fclose(pn);
		}
	}
	//add by fangu 2003.2.26, add log
	//sprintf(genbuf, "ANN newitem pm:%s - fname:%s - mode:%d", pm->path,fname,mode);
	//% sprintf(genbuf, "%s %s建立%s '%-20.20s..'\n", get_short_date(time(0)),
	sprintf(genbuf, "%s %s\xbd\xa8\xc1\xa2%s '%-20.20s..'\n", get_short_date(time(0)),
			//% currentuser.userid, (head=='D') ? "目录" : "文章", title);
			currentuser.userid, (head=='D') ? "\xc4\xbf\xc2\xbc" : "\xce\xc4\xd5\xc2", title);
	file_append(ANN_LOG_PATH, genbuf);
	log_bm(LOG_BM_DOANN, 1);
}

void a_moveitem(MENU *pm) {
	ITEM *tmp;
	char newnum[STRLEN];
	int num, n;
	//% sprintf(genbuf, "请输入第 %d 项的新次序: ", pm->now + 1);
	sprintf(genbuf, "\xc7\xeb\xca\xe4\xc8\xeb\xb5\xda %d \xcf\xee\xb5\xc4\xd0\xc2\xb4\xce\xd0\xf2: ", pm->now + 1);
	a_prompt(-2, genbuf, newnum, 6);
	num = (newnum[0] == '$') ? 9999 : atoi(newnum) - 1;
	if (num >= pm->num)
		num = pm->num - 1;
	else if (num < 0)
		return;
	tmp = pm->item[pm->now];
	if (num > pm->now) {
		for (n = pm->now; n < num; n++)
			pm->item[n] = pm->item[n + 1];
	} else {
		for (n = pm->now; n > num; n--)
			pm->item[n] = pm->item[n - 1];
	}
	pm->item[num] = tmp;
	pm->now = num;
	a_savenames(pm);
	log_bm(LOG_BM_DOANN, 1);
}

void a_copypaste(MENU *pm, int paste) {
	ITEM *item;
	static char title[sizeof(item->title)], filename[STRLEN], fpath[PATHLEN];
	char newpath[PATHLEN]/*,ans[5]*/;
	FILE *fn;
	move(-1, 0);
	sethomefile(fpath, currentuser.userid, ".copypaste");

	if (!paste) {
		fn = fopen(fpath, "w+");
		item = pm->item[pm->now];
		fwrite(item->title, sizeof(item->title), 1, fn);
		fwrite(item->fname, sizeof(item->fname), 1, fn);
		sprintf(genbuf, "%s/%s", pm->path, item->fname);
		strlcpy(fpath, genbuf, PATHLEN);
		fpath[PATHLEN - 1] = '\0';
		fwrite(fpath, sizeof(fpath), 1, fn);
		fclose(fn);
		//% 档案标识完成. (注意! 粘贴文章後才能将文章 delete!)
		presskeyfor("\xb5\xb5\xb0\xb8\xb1\xea\xca\xb6\xcd\xea\xb3\xc9. (\xd7\xa2\xd2\xe2! \xd5\xb3\xcc\xf9\xce\xc4\xd5\xc2\xe1\xe1\xb2\xc5\xc4\xdc\xbd\xab\xce\xc4\xd5\xc2 delete!)", -1);
	} else {
		if ((fn = fopen(fpath /*genbuf*/, "r")) == NULL) {
			//% 请先使用 copy 命令再使用 paste 命令.
			presskeyfor("\xc7\xeb\xcf\xc8\xca\xb9\xd3\xc3 copy \xc3\xfc\xc1\xee\xd4\xd9\xca\xb9\xd3\xc3 paste \xc3\xfc\xc1\xee.", -1);
			return;
		}
		fread(title, sizeof(item->title), 1, fn);
		fread(filename, sizeof(item->fname), 1, fn);
		fread(fpath, sizeof(fpath), 1, fn);
		fclose(fn);
		sprintf(newpath, "%s/%s", pm->path, filename);
		if (*title == '\0' || *filename == '\0') {
			//% 请先使用 copy 命令再使用 paste 命令.
			presskeyfor("\xc7\xeb\xcf\xc8\xca\xb9\xd3\xc3 copy \xc3\xfc\xc1\xee\xd4\xd9\xca\xb9\xd3\xc3 paste \xc3\xfc\xc1\xee.", -1);
		} else if (!(dashf(fpath) || dashd(fpath))) {
			//% sprintf(genbuf, "您拷贝的%s档案/目录不存在,可能被删除,取消粘贴.", filename);
			sprintf(genbuf, "\xc4\xfa\xbf\xbd\xb1\xb4\xb5\xc4%s\xb5\xb5\xb0\xb8/\xc4\xbf\xc2\xbc\xb2\xbb\xb4\xe6\xd4\xda,\xbf\xc9\xc4\xdc\xb1\xbb\xc9\xbe\xb3\xfd,\xc8\xa1\xcf\xfb\xd5\xb3\xcc\xf9.", filename);
			presskeyfor(genbuf, -1);
		} else if (dashf(newpath) || dashd(newpath)) {
			//% sprintf(genbuf, "%s 档案/目录已经存在. ", filename);
			sprintf(genbuf, "%s \xb5\xb5\xb0\xb8/\xc4\xbf\xc2\xbc\xd2\xd1\xbe\xad\xb4\xe6\xd4\xda. ", filename);
			presskeyfor(genbuf, -1);
		} else if (strstr(newpath, fpath) != NULL) {
			//% 无法将目录搬进自己的子目录中, 会造成死回圈.
			presskeyfor("\xce\xde\xb7\xa8\xbd\xab\xc4\xbf\xc2\xbc\xb0\xe1\xbd\xf8\xd7\xd4\xbc\xba\xb5\xc4\xd7\xd3\xc4\xbf\xc2\xbc\xd6\xd0, \xbb\xe1\xd4\xec\xb3\xc9\xcb\xc0\xbb\xd8\xc8\xa6.", -1);
		} else {
			if (dashd(fpath)) { /* 是目录 */
				sprintf(genbuf, "/bin/cp -rp %s %s", fpath, newpath);
				system(genbuf);
			} else if (paste == 1) { //copy
				f_cp(fpath, newpath, O_CREAT);
			} else if (paste == 2) { //link
				f_ln(fpath, newpath);
			} else if (paste == 3) {//cut
				f_cp(fpath, newpath, O_CREAT);
			}
			a_additem(pm, title, filename);
			a_savenames(pm);
			//add by fangu 2003.2.26, add log
			//% sprintf(genbuf, "%s %s粘贴 '%-20.20s..' 到 '%-20.20s..'\n",
			sprintf(genbuf, "%s %s\xd5\xb3\xcc\xf9 '%-20.20s..' \xb5\xbd '%-20.20s..'\n",
					get_short_date(time(0)), currentuser.userid, title,
					pm->mtitle);
			file_append(ANN_LOG_PATH, genbuf);
			log_bm(LOG_BM_DOANN, 1);
		}
	}

	pm->page = 9999;
}

//删除精华区的当前项,目录或文件
void a_delete(MENU *pm) {
	ITEM *item;
	char fpath[PATHLEN], ans[5];
	int n;
	FILE *fn;
	item = pm->item[pm->now];
	screen_move_clear(-2);
	prints("%5d  %-50s\n", pm->now + 1, item->title);
	sethomefile(fpath, currentuser.userid, ".copypaste");
	if ((fn = fopen(fpath /*genbuf*/, "r")) != NULL) {
		//判断文件是否存在,并试图读出如下信息,title,fname,fpath
		fread(genbuf, sizeof(item->title), 1, fn);
		fread(genbuf, sizeof(item->fname), 1, fn);
		fread(genbuf, sizeof(fpath), 1, fn);
		fclose(fn);
	}
	sprintf(fpath, "%s/%s", pm->path, item->fname);
	if (!strncmp(genbuf, fpath, sizeof(fpath)))
		a_prompt(
				-1,
				//% "[1;5;31m警告[m: 该档案/目录是被版主做了标记, [1;33m如果删除, 则不能再粘贴该文章!![m",
				"[1;5;31m\xbe\xaf\xb8\xe6[m: \xb8\xc3\xb5\xb5\xb0\xb8/\xc4\xbf\xc2\xbc\xca\xc7\xb1\xbb\xb0\xe6\xd6\xf7\xd7\xf6\xc1\xcb\xb1\xea\xbc\xc7, [1;33m\xc8\xe7\xb9\xfb\xc9\xbe\xb3\xfd, \xd4\xf2\xb2\xbb\xc4\xdc\xd4\xd9\xd5\xb3\xcc\xf9\xb8\xc3\xce\xc4\xd5\xc2!![m",
				ans, 2);
	if (dashf(fpath)) {
		//% if (askyn("删除此档案, 确定吗", NA, YEA) == NA)
		if (askyn("\xc9\xbe\xb3\xfd\xb4\xcb\xb5\xb5\xb0\xb8, \xc8\xb7\xb6\xa8\xc2\xf0", NA, YEA) == NA)
			return;
		unlink(fpath);
	} else if (dashd(fpath)) {
		//% if (askyn("删除整个子目录, 别开玩笑哦, 确定吗", NA, YEA) == NA)
		if (askyn("\xc9\xbe\xb3\xfd\xd5\xfb\xb8\xf6\xd7\xd3\xc4\xbf\xc2\xbc, \xb1\xf0\xbf\xaa\xcd\xe6\xd0\xa6\xc5\xb6, \xc8\xb7\xb6\xa8\xc2\xf0", NA, YEA) == NA)
			return;
		deltree(fpath);
	}
	//add by iamfat 2003.2.26, add log
	//% sprintf(genbuf, "%s %s从 '%-20.20s..' 删除 '%-20.20s..'\n",
	sprintf(genbuf, "%s %s\xb4\xd3 '%-20.20s..' \xc9\xbe\xb3\xfd '%-20.20s..'\n",
			get_short_date(time(0)), currentuser.userid, pm->mtitle,
			item->title);
	file_append(ANN_LOG_PATH, genbuf);
	log_bm(LOG_BM_DOANN, 1);
	//	sprintf(genbuf, "ANN DEL '%s'", item->title);
	//	report(genbuf);
	free(item);
	(pm->num)--;
	for (n = pm->now; n < pm->num; n++)
		pm->item[n] = pm->item[n + 1];
	a_savenames(pm);
}

/* addedd by roly 02.05.15 */
void a_RangeDelete(MENU *pm, int num1, int num2) {
	ITEM *item;
	char fpath[PATHLEN];
	int i, n;
	for (n=num1; n<=num2; n++) {
		item = pm->item[n];
		sprintf(fpath, "%s/%s", pm->path, item->fname);
		if (dashf(fpath)) {
			unlink(fpath);
		} else if (dashd(fpath)) {
			deltree(fpath);
		}
	}

	for (n=num1; n<=num2; n++) {
		free(pm->item[n]);
		(pm->num)--;
	}

	for (i = num1; i < pm->num; i++)
		pm->item[i] = pm->item[i + num2-num1+1];
	a_savenames(pm);

	//add by fangu 2003.2.26, add log
	//% sprintf(genbuf, "%s %s从 '%10.10s..' 删除第%d到第%d篇\n",
	sprintf(genbuf, "%s %s\xb4\xd3 '%10.10s..' \xc9\xbe\xb3\xfd\xb5\xda%d\xb5\xbd\xb5\xda%d\xc6\xaa\n",
			get_short_date(time(0)), currentuser.userid, pm->mtitle, num1, num2);
	file_append(ANN_LOG_PATH, genbuf);
	log_bm(LOG_BM_DOANN, 1);
	//sprintf(genbuf, "ANN RANGE_DEL %s",fpath);
	//report(genbuf);

}

int a_Rangefunc(MENU *pm) {
	struct fileheader;
	char annpath[512];
	char buf[STRLEN], ans[8], info[STRLEN];

	ITEM *item;

	//% char items[2][20] = { "删除", "拷贝至丝路" };
	char items[2][20] = { "\xc9\xbe\xb3\xfd", "\xbf\xbd\xb1\xb4\xd6\xc1\xcb\xbf\xc2\xb7" };
	int type, num1, num2, i, max=2;

	screen_save_line(-1, true);
	//% strcpy(info, "区段:");
	strcpy(info, "\xc7\xf8\xb6\xce:");
	for (i=0; i<max; i++) {
		sprintf(buf, " %d)%s", i+1, items[i]);
		strcat(info, buf);
	}
	strcat(info, " [0]: ");
	getdata(-1, 0, info, ans, 2, DOECHO, YEA);
	type = atoi(ans);
	if (type <= 0 || type > max) {
		screen_save_line(-1, false);
		return DONOTHING;
	}

	screen_move_clear(-1);
	//% prints("区段操作: %s", items[type-1]);
	prints("\xc7\xf8\xb6\xce\xb2\xd9\xd7\xf7: %s", items[type-1]);
	//% 首篇文章编号: 
	getdata(-1, 20, "\xca\xd7\xc6\xaa\xce\xc4\xd5\xc2\xb1\xe0\xba\xc5: ", ans, 6, DOECHO, YEA);
	num1=atoi(ans);
	if (num1>0) {
		//% "末片文章编号: "
		getdata(-1, 40, "\xc4\xa9\xc6\xac\xce\xc4\xd5\xc2\xb1\xe0\xba\xc5: ", ans, 6, DOECHO, YEA);
		num2=atoi(ans);
	}
	if (num1<=0||num2<=0||num2<=num1 ||num2>pm->num) {
		move(-1, 60);
		//% prints("操作错误...");
		prints("\xb2\xd9\xd7\xf7\xb4\xed\xce\xf3...");
		egetch();
		screen_save_line(-1, false);
		return DONOTHING;
	}

	//% sprintf(info, "区段 [%s] 操作范围 [ %d -- %d ]，确定吗", items[type-1], num1,
	sprintf(info, "\xc7\xf8\xb6\xce [%s] \xb2\xd9\xd7\xf7\xb7\xb6\xce\xa7 [ %d -- %d ]\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0", items[type-1], num1,
			num2);
	if (askyn(info, NA, YEA)==NA) {
		screen_save_line(-1, false);
		return DONOTHING;
	}

	if (type == 2) {
		FILE *fn;
		char genbuf[512];
		if (DEFINE(DEF_MULTANNPATH) && set_ann_path(NULL, NULL,
				ANNPATH_GETMODE) == 0)
			return DONOTHING;

		sethomefile(annpath, currentuser.userid, ".announcepath");
		if ((fn = fopen(annpath, "r")) == NULL) {
			//% 对不起, 您没有设定丝路. 请先用 f 设定丝路.
			presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xc3\xbb\xd3\xd0\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7. \xc7\xeb\xcf\xc8\xd3\xc3 f \xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7.", -1);
			screen_save_line(-1, false);
			return DONOTHING;
		}
		fscanf(fn, "%s", annpath);
		fclose(fn);
		if (!dashd(annpath)) {
			//% 您设定的丝路已丢失, 请重新用 f 设定.
			presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xd3\xc3 f \xc9\xe8\xb6\xa8.", -1);
			screen_save_line(-1, false);
			return DONOTHING;
		}
		if (!strcmp(annpath, pm->path)) {
			//% 您设定的丝路与当前目录相同，本次操作取消.
			presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xd3\xeb\xb5\xb1\xc7\xb0\xc4\xbf\xc2\xbc\xcf\xe0\xcd\xac\xa3\xac\xb1\xbe\xb4\xce\xb2\xd9\xd7\xf7\xc8\xa1\xcf\xfb.", -1);
			return DONOTHING;
		}

		for (i=num1-1; i<=num2-1; i++) {
			item = pm->item[i];
			sprintf(genbuf, "%s/%s", pm->path, item->fname);
			if (dashd(genbuf))
				if (strstr(annpath, genbuf)!=NULL) {
					//% 您设定的丝路位于所选某个目录中，会导致嵌套目录，本次操作取消.
					presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xce\xbb\xd3\xda\xcb\xf9\xd1\xa1\xc4\xb3\xb8\xf6\xc4\xbf\xc2\xbc\xd6\xd0\xa3\xac\xbb\xe1\xb5\xbc\xd6\xc2\xc7\xb6\xcc\xd7\xc4\xbf\xc2\xbc\xa3\xac\xb1\xbe\xb4\xce\xb2\xd9\xd7\xf7\xc8\xa1\xcf\xfb.", -1);
					return DONOTHING;
				}
		}
	}

	switch (type) {
		case 1:
			//% sprintf(info, "您真的要删除这些文件和目录\?\?!!，确定吗??");
			sprintf(info, "\xc4\xfa\xd5\xe6\xb5\xc4\xd2\xaa\xc9\xbe\xb3\xfd\xd5\xe2\xd0\xa9\xce\xc4\xbc\xfe\xba\xcd\xc4\xbf\xc2\xbc\?\?!!\xa3\xac\xc8\xb7\xb6\xa8\xc2\xf0??");
			if (askyn(info, NA, YEA)==NA) {
				screen_save_line(-1, false);
				return DONOTHING;
			}
			a_RangeDelete(pm, num1-1, num2-1);
			pm->page = 9999;
			break;
		case 2:
			for (i=num1-1; i<=num2-1; i++) {
				a_a_Import(pm, NA, i);
			}

			break;
	}
	log_bm(LOG_BM_DOANN, 1);
	screen_save_line(-1, false);
	return DIRCHANGED;
}

/* add end */

/* added by roly 02.05.15 */
/* msg用来判断是否提示已完成操作 */
/* menuitem 用于在执行range a_a_Import的是否传入pm->now的值 */
int a_a_Import(MENU *pm, int msg, int menuitem) {
	ITEM *item;
	char fpath[PATHLEN], annpath[512], fname[512], dfname[512], *ip,
			genbuf[1024];
	int ch;
	FILE *fn;
	MENU newpm;

	if (menuitem<0)
		item = pm->item[pm->now];
	else
		item = pm->item[menuitem];

	sethomefile(annpath, currentuser.userid, ".announcepath");
	if ((fn = fopen(annpath, "r")) == NULL) {
		//% 对不起, 您没有设定丝路. 请先用 f 设定丝路.
		presskeyfor("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xc3\xbb\xd3\xd0\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7. \xc7\xeb\xcf\xc8\xd3\xc3 f \xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7.", -1);
		return 1;
	}
	fscanf(fn, "%s", annpath);
	fclose(fn);
	if (!dashd(annpath)) {
		//% 您设定的丝路已丢失, 请重新用 f 设定.
		presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xd3\xc3 f \xc9\xe8\xb6\xa8.", -1);
		return 1;
	}

	if (!strcmp(annpath, pm->path)) {
		//% 您设定的丝路与当前目录相同，本次操作取消.
		presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xd3\xeb\xb5\xb1\xc7\xb0\xc4\xbf\xc2\xbc\xcf\xe0\xcd\xac\xa3\xac\xb1\xbe\xb4\xce\xb2\xd9\xd7\xf7\xc8\xa1\xcf\xfb.", -1);
		return 1;
	}

	sprintf(genbuf, "%s/%s", pm->path, item->fname);

	if (dashd(genbuf))
		if (strstr(annpath, genbuf)!=NULL) {
			//% 您设定的丝路位于所选目录中，会导致嵌套目录，本次操作取消.
			presskeyfor("\xc4\xfa\xc9\xe8\xb6\xa8\xb5\xc4\xcb\xbf\xc2\xb7\xce\xbb\xd3\xda\xcb\xf9\xd1\xa1\xc4\xbf\xc2\xbc\xd6\xd0\xa3\xac\xbb\xe1\xb5\xbc\xd6\xc2\xc7\xb6\xcc\xd7\xc4\xbf\xc2\xbc\xa3\xac\xb1\xbe\xb4\xce\xb2\xd9\xd7\xf7\xc8\xa1\xcf\xfb.", -1);
			return 1;
		}

	//以上检查丝路

	newpm.path = annpath;
	strcpy(newpm.mtitle, "");
	a_loadnames(&newpm);
	strcpy(fname, item->fname);
	sprintf(dfname, "%s/%s", newpm.path, fname);
	ip = &fname[strlen(fname) - 1];
	if (dashf(dfname)||dashd(dfname)) {
		ip++, *ip = 'A', *(ip + 1) = '\0';
	}
	while (dashf(dfname)||dashd(dfname)) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(dfname, "%s/%s", newpm.path, fname);
	}
	sprintf(genbuf, "%-72.72s", item->title);
	//sprintf(genbuf, "%-38.38s %s ", item->title, currentuser.userid);
	a_additem(&newpm, genbuf, fname);
	a_savenames(&newpm);

	//以上为添加newpm

	sprintf(fpath, "%s/%s", pm->path, item->fname);
	sprintf(genbuf, "/bin/cp -r %s %s", fpath, dfname);
	system(genbuf);

	if (msg)
		//% 已将该文章/目录放进丝路, 请按<Enter>继续...
		presskeyfor("\xd2\xd1\xbd\xab\xb8\xc3\xce\xc4\xd5\xc2/\xc4\xbf\xc2\xbc\xb7\xc5\xbd\xf8\xcb\xbf\xc2\xb7, \xc7\xeb\xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);

	for (ch = 0; ch < newpm.num; ch++)
		free(newpm.item[ch]);
	pm->page=9999;
	//add by fangu 2003.2.26, add log
	//% sprintf(genbuf, "%s %s从 '%-20.20s..' 收录 '%-20.20s..'\n",
	sprintf(genbuf, "%s %s\xb4\xd3 '%-20.20s..' \xca\xd5\xc2\xbc '%-20.20s..'\n",
			get_short_date(time(0)), currentuser.userid, pm->mtitle,
			item->title);
	file_append(ANN_LOG_PATH, genbuf);
	log_bm(LOG_BM_DOANN, 1);
	//sprintf(genbuf, "ANN AA_IMP %s ", dfname);//记录 丝路-源文件
	//report(genbuf);
	return 1;

}
/* add end */

void a_newname(MENU *pm) {
	ITEM *item;
	char fname[STRLEN];
	char fpath[PATHLEN];
	char *mesg;
	item = pm->item[pm->now];
	//% a_prompt(-2, "新档名: ", fname, 12);
	a_prompt(-2, "\xd0\xc2\xb5\xb5\xc3\xfb: ", fname, 12);
	if (*fname == '\0')
		return;
	sprintf(fpath, "%s/%s", pm->path, fname);
	if (!valid_fname(fname)) {
		//% mesg = "不合法档案名称.";
		mesg = "\xb2\xbb\xba\xcf\xb7\xa8\xb5\xb5\xb0\xb8\xc3\xfb\xb3\xc6.";
	} else if (dashf(fpath) || dashd(fpath)) {
		//% mesg = "系统中已有此档案存在了.";
		mesg = "\xcf\xb5\xcd\xb3\xd6\xd0\xd2\xd1\xd3\xd0\xb4\xcb\xb5\xb5\xb0\xb8\xb4\xe6\xd4\xda\xc1\xcb.";
	} else {
		sprintf(genbuf, "%s/%s", pm->path, item->fname);
		if (rename(genbuf, fpath) == 0) {
			//add by fangu 2003.2.26 add log
			//sprintf(genbuf, "ANN RENAME %s >> %s", item->fname,fname,pm->path);
			//记录 旧名 - 新名 -文件
			//report(genbuf);
			strcpy(item->fname, fname);
			a_savenames(pm);
			return;
		}
		//% mesg = "档名更改失败!!";
		mesg = "\xb5\xb5\xc3\xfb\xb8\xfc\xb8\xc4\xca\xa7\xb0\xdc!!";
	}
	outs(mesg);
	egetch();

}

/* added by roly 02.05.20,convert path to title */
void ann_to_title(char *annpath) {
	FILE *fn;
	int ipos, ipos2, ipos3, ipos4, icount=0;
	char *ptr;
	char tmppath[1000], buf[1000], result[1000];
	char dirname[STRLEN], titlename[STRLEN];
	for (ipos=0; ipos<strlen(annpath); ipos++) {
		if (annpath[ipos]=='/') {
			icount++;
			if (icount==3)
				break;
		}
	}
	if (icount<3)
		return;

	//如果'/'少于3个，表示不是处于版面精华区，返回

	ipos++;
	ipos2=ipos;
	sprintf(tmppath, "%s", annpath);
	for (; ipos<strlen(annpath); ipos++)
		if (annpath[ipos]=='/')
			break;
	tmppath[ipos]='\0';
	//% sprintf(result, "%s版/", tmppath+ipos2);
	sprintf(result, "%s\xb0\xe6/", tmppath+ipos2);
	//读取版面

	if (ipos==strlen(annpath)) { //位于某版面根目录
		sprintf(annpath, "%s", result);
		return;
	}

	sprintf(buf, "%s/.Names", tmppath);
	tmppath[ipos]='/';
	ipos++;
	ipos2=ipos;
	size_t len = strlen(annpath);
	for (; ipos < len; ipos++) {
		if (annpath[ipos] == '/' || annpath[ipos + 1] == '\0') {
			if (annpath[ipos]=='/') {
				strlcpy(dirname, annpath + ipos2, ipos - ipos2 + 1);
				ipos4 = ipos - ipos2;
			} else {
				strlcpy(dirname, annpath + ipos2, ipos - ipos2 + 2);
				ipos4=ipos-ipos2+1;
			}

			if ((fn = fopen(buf, "r")) == NULL)
				break;
			while (fgets(buf, sizeof(buf), fn) != NULL) {
				if ((ptr = strchr(buf, '\n')) != NULL)
					*ptr = '\0';
				if (strncmp(buf, "Name=", 5) == 0) {
					strlcpy(titlename, buf + 5, 72);
				} else if (strncmp(buf, "Path=", 5) == 0) {

					if (((strncmp(buf, "Path=~/", 7) == 0) && (strncmp(buf
							+7, dirname, ipos4)==0)) ||(strncmp(buf+5,
							dirname, ipos4)==0)) {

						for (ipos3=36; ipos3>=1; ipos3--) {
							if (titlename[ipos3]!=' ') {
								titlename[ipos3+1]='/';
								titlename[ipos3+2]='\0';
								break;
							}
						}
						strcat(result, titlename);
						report(titlename, currentuser.userid);
						break;
					}
				}
			}
			fclose(fn);
			tmppath[ipos]='\0';
			sprintf(buf, "%s/.Names", tmppath);
			tmppath[ipos]='/';
			ipos2=ipos+1;
		}
	}

	sprintf(annpath, "%s", result);
	return;
}

/* add end */
//对用户在精华区的按键进行反应
void a_manager(MENU *pm, int ch) {
	char uident[STRLEN];
	ITEM *item=NULL;
	MENU xpm;
	char fpath[PATHLEN], changed_T[STRLEN];//, ans[5];
	char fowner[IDLEN]=""; //add by Danielfree 06.2.20
	int i=0;
	if (pm->num > 0) {
		item = pm->item[pm->now];
		sprintf(fpath, "%s/%s", pm->path, item->fname);
	}
	//% if (item && strstr(item->title+38, "只读"))
	if (item && strstr(item->title+38, "\xd6\xbb\xb6\xc1"))
		readonly=YEA;
	else
		readonly=NA;

	switch (ch) {
		case 'a':
			a_newitem(pm, ADDITEM);
			break;
		case 'g':
			a_newitem(pm, ADDGROUP);
			break;
		case 'i':
			a_newitem(pm, ADDMAIL);
			break;
		case 'p':
			a_copypaste(pm, 1);
			break;
			/* added by roly 02.05.15 */
		case 'l':
			a_copypaste(pm, 2);
			break;
		case 'X':
			a_copypaste(pm, 3);
			break;
		case 't':
			if (DEFINE(DEF_MULTANNPATH))
				currentuser.userdefine &= ~DEF_MULTANNPATH;
			else
				currentuser.userdefine |= DEF_MULTANNPATH;
			substitut_record(PASSFILE, &currentuser, sizeof(currentuser),
					usernum);
			pm->page = 9999;
			break;
		case 'S': {
			FILE *fn;
			pm->page = 9999;
			sethomefile(genbuf, currentuser.userid, ".announcepath");
			fn = fopen(genbuf, "r");
			if (fn) {
				char tmpath[1000];
				fgets(tmpath, 1000, fn);
				fclose(fn);
				ann_to_title(tmpath);
				//% sprintf(genbuf, "当前丝路: %s", tmpath);
				sprintf(genbuf, "\xb5\xb1\xc7\xb0\xcb\xbf\xc2\xb7: %s", tmpath);
			} else
				//% strcpy(genbuf, "您还没有用 f 设置丝路.");
				strcpy(genbuf, "\xc4\xfa\xbb\xb9\xc3\xbb\xd3\xd0\xd3\xc3 f \xc9\xe8\xd6\xc3\xcb\xbf\xc2\xb7.");
			presskeyfor(genbuf, -1);
			break;
		}
			/* add end */

			/* add by roly 02.05.15*/
		case 'I':
			pm->page = 9999;
			if (DEFINE(DEF_MULTANNPATH) && set_ann_path(NULL, NULL,
					ANNPATH_GETMODE) == 0)
				break;
			a_a_Import(pm, YEA, -1);
			break;
		case 'L':
			pm->page = 9999;
			a_Rangefunc(pm);
			break;
			/* add end */
		case 'f':
			pm->page = 9999;
			if (DEFINE(DEF_MULTANNPATH)) {
				set_ann_path(pm->mtitle, pm->path, ANNPATH_SETMODE);
			} else {
				FILE *fn;
				sethomefile(genbuf, currentuser.userid, ".announcepath");
				if ((fn = fopen(genbuf, "w+")) != NULL) {
					fprintf(fn, "%s", pm->path);
					fclose(fn);
					//% 已将该路径设为丝路, 请按<Enter>继续...
					presskeyfor("\xd2\xd1\xbd\xab\xb8\xc3\xc2\xb7\xbe\xb6\xc9\xe8\xce\xaa\xcb\xbf\xc2\xb7, \xc7\xeb\xb0\xb4<Enter>\xbc\xcc\xd0\xf8...", -1);
				}
			}
			break;
			/* add by jacobson,20050515 */
			/* 寻找丢失条目 */
		case 'z':
			if (HAS_PERM(PERM_SYSOPS)) {
				int i;
				i=a_repair(pm);
				if (i>0) {
					//% sprintf(genbuf, "发现 %d 个丢失条目,已恢复,请按Enter继续...", i);
					sprintf(genbuf, "\xb7\xa2\xcf\xd6 %d \xb8\xf6\xb6\xaa\xca\xa7\xcc\xf5\xc4\xbf,\xd2\xd1\xbb\xd6\xb8\xb4,\xc7\xeb\xb0\xb4""Enter\xbc\xcc\xd0\xf8...", i);
				} else {
					//% sprintf(genbuf, "未发现丢失条目,请按Enter继续...");
					sprintf(genbuf, "\xce\xb4\xb7\xa2\xcf\xd6\xb6\xaa\xca\xa7\xcc\xf5\xc4\xbf,\xc7\xeb\xb0\xb4""Enter\xbc\xcc\xd0\xf8...");
				}
				//  a_prompt(-1,genbuf,ans,39);
				presskeyfor(genbuf, -1);
				pm->page = 9999;
			}
			break;
			/* add end */
	}
	if (pm->num > 0)
		switch (ch) {
			case 's':
				//if (++a_fmode >= 3)
				//	a_fmode = 1;
				a_fmode = !a_fmode;
				pm->page = 9999;
				break;
			case 'm':
				a_moveitem(pm);
				pm->page = 9999;
				break;
			case 'd':
			case 'D':
				a_delete(pm);
				pm->page = 9999;
				break;
			case 'V':
			case 'v':
				if (HAS_PERM(PERM_SYSOPS)) {
					if (ch == 'v')
						sprintf(fpath, "%s/.Names", pm->path);
					else
						sprintf(fpath, "0Announce/.Search");

					if (dashf(fpath)) {
						set_user_status(ST_EDITANN);
						editor(fpath, false ,false, true, NULL);
						set_user_status(ST_DIGEST);
					}
					pm->page = 9999;
				}
				break;
			case 'T':
				if (readonly==YEA)
					break;
				//Modified by IAMFAT 2002-05-25
				string_remove_ansi_control_code(changed_T, item->title);
				//add by Danielfree 06.2.20
				for (i =0; i<IDLEN; i++) {
					if (changed_T[39+i]) {
						fowner[i]=changed_T[39+i];
					}
				}
				fowner[sizeof(fowner) - 1] = '\0';
				//add end
				changed_T[38]='\0';
				rtrim(changed_T);
				screen_save_line(-2, true);
				screen_move_clear(-2);
				//Modified by IAMFAT 2002-05-30
				//% "新标题: 
				getdata(-2, 0, "\xd0\xc2\xb1\xea\xcc\xe2: ", changed_T, 39, DOECHO, NA);
				screen_save_line(-2, false);
				//Modify End.
				/*
				 * modified by netty to properly handle title
				 * change,add bm by SmallPig
				 */
				if (*changed_T) {
					sprintf(genbuf,
							//% "%s %s将 '%-20.20s..' 改名为 '%-20.20s..'\n",
							"%s %s\xbd\xab '%-20.20s..' \xb8\xc4\xc3\xfb\xce\xaa '%-20.20s..'\n",
							get_short_date(time(0)), currentuser.userid,
							item->title, changed_T);
					file_append(ANN_LOG_PATH, genbuf);
					log_bm(LOG_BM_DOANN, 1);
					if (dashf(fpath)) {
						sprintf(genbuf, "%-38.38s %s", changed_T, fowner); //suggest by Humorous
						//sprintf(genbuf, "%-38.38s %s", changed_T, currentuser.userid);
						strlcpy(item->title, genbuf, 72);
						item->title[71] = '\0';
					} else if (dashd(fpath)) {
						screen_move_clear(1);
						/*
						 //% * usercomplete("版主:
						 * usercomplete("\xb0\xe6\xd6\xf7:
						 * ",uident) ;
						 */
						//% getdata(1, 0, "版主: ", uident, 35, DOECHO, YEA);
						getdata(1, 0, "\xb0\xe6\xd6\xf7: ", uident, 35, DOECHO, YEA);
						if (uident[0] != '\0')
							sprintf(genbuf, "%-38.38s(BM: %s)", changed_T,
									uident);
						else
							sprintf(genbuf, "%-38.38s", changed_T);
						xpm.path=fpath;
						a_loadnames(&xpm);
						strcpy(xpm.mtitle, genbuf);
						a_savenames(&xpm);

						strlcpy(item->title, genbuf, 72);
					}
					item->title[71] = '\0';
					a_savenames(pm);
				}
				pm->page = 9999;
				break;
			case 'E':
				if (readonly==YEA)
					break;
				if (dashf(fpath)) {
					set_user_status(ST_EDITANN);
					editor(fpath, false, false, true, NULL);
					set_user_status(ST_DIGEST);
				}
				pm->page = 9999;
				break;
			case 'n':
				a_newname(pm);
				pm->page = 9999;
				break;
			case 'c':
				a_copypaste(pm, 0);
				break;

		}
}

void a_file_info(MENU *pm)
{
	char fname[1024], weblink[1024], tmp[80], type[10];
	struct stat st;
	int i, len;

	/*精华区*/

	sprintf(fname, "%s/%s", pm->path, pm->item[pm->now]->fname);
	if (dashf(fname) ) {
		snprintf(weblink, 1024, "http://%s/cgi-bin/bbs/bbsanc?path=%s\n",
				BBSHOST, fname + 9);
		//% strcpy(type, "文件");
		strcpy(type, "\xce\xc4\xbc\xfe");
	} else if (dashd(fname) ) {
		snprintf(weblink, 1024, "http://%s/cgi-bin/bbs/bbs0an?path=%s\n",
				BBSHOST, fname + 9);
		//% strcpy(type, "目录");
		strcpy(type, "\xc4\xbf\xc2\xbc");
	} else {
		return;
	}
	len = strlen(weblink);
	stat(fname, &st);

	screen_clear();

	//% prints("精华区%s详细信息:\n\n", type);
	prints("\xbe\xab\xbb\xaa\xc7\xf8%s\xcf\xea\xcf\xb8\xd0\xc5\xcf\xa2:\n\n", type);
	//% prints("序    号:     第 %d 篇\n", pm->now);
	prints("\xd0\xf2    \xba\xc5:     \xb5\xda %d \xc6\xaa\n", pm->now);
	//% prints("类    别:     %s\n", type);
	prints("\xc0\xe0    \xb1\xf0:     %s\n", type);
	strlcpy(tmp, pm->item[pm->now]->title, 38);
	tmp[38] = '\0';
	//% prints("标    题:     %s\n", tmp);
	prints("\xb1\xea    \xcc\xe2:     %s\n", tmp);
	//% prints("修 改 者:     %s\n", pm->item[pm->now]->title + 39);
	prints("\xd0\xde \xb8\xc4 \xd5\xdf:     %s\n", pm->item[pm->now]->title + 39);
	//% prints("档    名:     %s\n", pm->item[pm->now]->fname);
	prints("\xb5\xb5    \xc3\xfb:     %s\n", pm->item[pm->now]->fname);
	//% prints("编辑日期:     %s\n", format_time(st.st_mtime, TIME_FORMAT_ZH));
	prints("\xb1\xe0\xbc\xad\xc8\xd5\xc6\xda:     %s\n", format_time(st.st_mtime, TIME_FORMAT_ZH));
	//% prints("大    小:     %d 字节\n", st.st_size);
	prints("\xb4\xf3    \xd0\xa1:     %d \xd7\xd6\xbd\xda\n", st.st_size);
	//% prints("URL 地址:\n");
	prints("URL \xb5\xd8\xd6\xb7:\n");
	for (i = 0; i < len; i +=78) {
		strlcpy(tmp, weblink+i, 78);
		tmp[78] = '\n';
		tmp[79] = '\0';
		outs(tmp);
	}

	pressanykey();
}

void a_menu(char *maintitle, char* path, int lastlevel, int lastbmonly)
{
	MENU me;
	char fname[PATHLEN], tmp[STRLEN];
	int ch;
	char *bmstr;
	char buf[STRLEN];
	int bmonly;
	int number = 0;
	int savemode;
	char something[PATHLEN+20];//add by wujian

	set_user_status(ST_DIGEST);
	strcpy(something, path);
	strcat(something, "/welcome");
	me.path = path;
	if (dashf(something))
		show_help(something);
	strcpy(me.mtitle, maintitle);
	me.level = lastlevel;
	bmonly = lastbmonly;
	a_loadnames(&me);

	memset(buf, 0, STRLEN);
	strlcpy(buf, me.mtitle, STRLEN);
	bmstr = strstr(buf, "(BM:");
	if (bmstr != NULL) {
		if (chk_currBM(bmstr + 4, 0))
			me.level |= PERM_BOARDS;
		else if (bmonly == 1 && !HAS_PERM(PERM_BOARDS))
			return;
	}
	if ((strstr(me.mtitle, "(BM: BMS)") && !chk_currBM(currBM, 0))
			|| (strstr(me.mtitle, "(BM: SYSOPS)")
					&& !HAS_PERM(PERM_SYSOPS)) || (strstr(me.mtitle,
			"(BM: OBOARDS)") && !HAS_PERM(PERM_OBOARDS)) || (strstr(
			me.mtitle, "(BM: SECRET)") && !chk_currBM(currBM, 0))) {
		for (ch = 0; ch < me.num; ch++)
			free(me.item[ch]);
		return;
	}

	strcpy(buf, me.mtitle);
	bmstr = strstr(buf, "(BM:");

	me.page = 9999;
	me.now = 0;
	while (1) {
		if (me.now >= me.num && me.num > 0) {
			me.now = me.num - 1;
		} else if (me.now < 0) {
			me.now = 0;
		}
		if (me.now < me.page || me.now >= me.page + A_PAGESIZE) {
			me.page = me.now - (me.now % A_PAGESIZE);
			a_showmenu(&me);
		}
		move(3 + me.now - me.page, 0);
		prints("->");
		ch = egetch();
		move(3 + me.now - me.page, 0);
		prints("  ");
		if (ch == 'Q' || ch == 'q' || ch == KEY_LEFT || ch == EOF)
			break;
		EXPRESS: /* Leeward 98.09.13 */
		switch (ch) {
			/*Added by kit 2001.01.29*/
			case '*':
				if (me.num > 0) {
					a_file_info(&me);
					me.page = 9999;
				}
				break;
			case 'o':
				show_online_users();
				a_showmenu(&me);
				break;
				/* End */
			case KEY_UP:
			case 'K':
			case 'k':
				if (--me.now < 0)
					me.now = me.num - 1;
				break;
			case KEY_DOWN:
			case 'J':
			case 'j':
				if (++me.now >= me.num)
					me.now = 0;
				break;
			case KEY_PGUP:
			case Ctrl('B'):
				if (me.now >= A_PAGESIZE)
					me.now -= A_PAGESIZE;
				else if (me.now > 0)
					me.now = 0;
				else
					me.now = me.num - 1;
				break;
			case KEY_PGDN:
			case Ctrl('F'):
			case ' ':
				if (me.now < me.num - A_PAGESIZE)
					me.now += A_PAGESIZE;
				else if (me.now < me.num - 1)
					me.now = me.num - 1;
				else
					me.now = 0;
				break;
			case KEY_HOME:
				me.now = 0;
				break;
			case KEY_END:
			case '$':
				me.now = me.num - 1;
				break;
			case Ctrl('C'):
				if (me.num==0)
					break;
				set_safe_record();
				if (!HAS_PERM(PERM_POST))
					break;
				sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
				if (!dashf(fname))
					break;
				if (me.now < me.num) {
					char bname[30];
					screen_clear();
					//% board_complete(0, "请输入要转贴的讨论区名称: ",
					board_complete(0, "\xc7\xeb\xca\xe4\xc8\xeb\xd2\xaa\xd7\xaa\xcc\xf9\xb5\xc4\xcc\xd6\xc2\xdb\xc7\xf8\xc3\xfb\xb3\xc6: ",
							bname, sizeof(bname), AC_LIST_BOARDS_ONLY);
					if (*bname) {
						move(1, 0);
						
						board_t board;
						if (!get_board(bname, &board)
								|| !has_post_perm(&board)) {
							//% prints("\n\n您尚无权限在 %s 发表文章.", bname);
							prints("\n\n\xc4\xfa\xc9\xd0\xce\xde\xc8\xa8\xcf\xde\xd4\xda %s \xb7\xa2\xb1\xed\xce\xc4\xd5\xc2.", bname);
							pressreturn();
							me.page = 9999;
							break;
						}
						//% sprintf(tmp, "您确定要转贴到 %s 版吗", bname);
						sprintf(tmp, "\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xd7\xaa\xcc\xf9\xb5\xbd %s \xb0\xe6\xc2\xf0", bname);
						if (askyn(tmp, NA, NA) == 1) {
							Postfile(fname, bname, me.item[me.now]->title,
									4);
							screen_move_clear(3);
							//% sprintf(tmp, "[1m已经帮您转贴至 %s 版了[m", bname);
							sprintf(tmp, "[1m\xd2\xd1\xbe\xad\xb0\xef\xc4\xfa\xd7\xaa\xcc\xf9\xd6\xc1 %s \xb0\xe6\xc1\xcb[m", bname);
							outs(tmp);
							screen_flush();
							sleep(1);
						}
					}
					me.page = 9999;
				}
				show_message(NULL);
				break;
			case 'M':
				savemode = session_status();
				m_new();
				set_user_status(savemode);
				me.page = 9999;
				break;
			case 'h':
				show_help("help/announcereadhelp");
				me.page = 9999;
				break;
			case '\n':
			case '\r':
				if (number > 0) {
					me.now = number - 1;
					number = 0;
					continue;
				}
			case 'R':
			case 'r':
			case KEY_RIGHT:
				if (me.now < me.num) {
					sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
					if (dashf(fname) ) {
						/* Leeward 98.09:在精华区阅读文章到末尾时，用上／下箭头直接跳转到前／后一项 */
						if (ansimore(fname, NA)==KEY_UP) {
							if (--me.now < 0)
								me.now = me.num -1;
							ch = KEY_RIGHT;
							goto EXPRESS;
						}
						//Modified by IAMFAT 2002-05-28
						//Roll Back by IAMFAT 2002-05-29
						//Modified by IAMFAT 2002.06.11
						//% prints("[1;44;31m[阅读精华区资料]  [33m结束Q, ← │ 上一项资料 U,↑│ 下一项资料 <Enter>,<Space>,↓ [m");
						prints("[1;44;31m[\xd4\xc4\xb6\xc1\xbe\xab\xbb\xaa\xc7\xf8\xd7\xca\xc1\xcf]  [33m\xbd\xe1\xca\xf8Q, \xa1\xfb \xa9\xa6 \xc9\xcf\xd2\xbb\xcf\xee\xd7\xca\xc1\xcf U,\xa1\xfc\xa9\xa6 \xcf\xc2\xd2\xbb\xcf\xee\xd7\xca\xc1\xcf <Enter>,<Space>,\xa1\xfd [m");
						switch (ch = egetch() ) {
							case KEY_DOWN:
							case ' ':
							case '\n':
								if ( ++me.now >= me.num)
									me.now = 0;
								ch = KEY_RIGHT;
								goto EXPRESS;
							case KEY_UP:
							case 'u':
							case 'U':
								if (--me.now < 0)
									me.now = me.num - 1;
								ch = KEY_RIGHT;
								goto EXPRESS;
							case 'h':
								goto EXPRESS;
							default:
								break;
						}
					} else if (dashd(fname)) {
						//add by Danielfree 06.12.6
						char *end = nowpath + strlen(nowpath);
						sprintf(end, "-%d", (me.now) + 1);
						a_menu(me.item[me.now]->title, fname, me.level,
								bmonly);
					}
					me.page = 9999;
				}
				break;
			case '/': {
				int found;
				found=b_menusearch(&me);
				if (found)
					me.now=found;
			}
				break;
#ifdef INTERNET_EMAIL

				case 'F':
				case 'U':
				if (me.now < me.num && HAS_PERM(PERM_MAIL)) {
					a_forward(path, me.item[me.now], ch == 'U');
					me.page = 9999;
				}
				break;
#endif
			case '!':
				if (!Q_Goodbye())
					break; /* youzi leave */
		}
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
		if (me.level & PERM_BOARDS)
			a_manager(&me, ch);
	}
	for (ch = 0; ch < me.num; ch++)
		free(me.item[ch]);
	//add by Danielfree 06.12.6 
	char oldpath[STRLEN-17]="\0"; //for safety
	if (strlen(nowpath) >=1) {
		char *tmpchar;
		tmpchar = strrchr(nowpath, '-');//find the last "->"
		strncpy(oldpath, nowpath, tmpchar-nowpath);
	}
	snprintf(nowpath, STRLEN-17, "%s", oldpath);
}

//将文件名为fname,标题为title的文件加入到精华区中路径为path处保存
static int linkto(const char *path, const char *fname, const char *title)
{
	MENU pm;
	pm.path = path;

	//strcpy(pm.mtitle, f_title);
	a_loadnames(&pm);
	if (pm.mtitle[0] == '\0')
		strcpy(pm.mtitle, title);
	a_additem(&pm, title, fname);
	a_savenames(&pm);
	return 0;
}

int add_grp(const char *group, const char *gname,
		const char *bname, const char *title)
{
	FILE *fn;
	char buf[PATHLEN];
	char searchname[STRLEN];
	char gpath[STRLEN * 2]; //高层目录
	char bpath[STRLEN * 2]; //低层目录
	sprintf(buf, "0Announce/.Search");
	sprintf(searchname, "%s: groups/%s/%s\n", bname, group, bname);
	sprintf(gpath, "0Announce/groups/%s", group);
	sprintf(bpath, "%s/%s", gpath, bname);
	if (!dashd("0Announce")) {
		mkdir("0Announce", 0755);
		chmod("0Announce", 0755);
		if ((fn = fopen("0Announce/.Names", "w")) == NULL)
			return -1;
		fprintf(fn, "#\n");
		//% fprintf(fn, "# Title=%s 精华区公布栏\n", BoardName);
		fprintf(fn, "# Title=%s \xbe\xab\xbb\xaa\xc7\xf8\xb9\xab\xb2\xbc\xc0\xb8\n", BoardName);
		fprintf(fn, "#\n");
		fclose(fn);
	}
	if (!seek_in_file(buf, bname))
		file_append(buf, searchname);
	if (!dashd("0Announce/groups")) {
		mkdir("0Announce/groups", 0755);
		chmod("0Announce/groups", 0755);

		if ( (fn = fopen("0Announce/groups/.Names", "w")) == NULL)
			return -1;
		fprintf(fn, "#\n");
		//% fprintf(fn, "# Title=讨论区精华\n");
		fprintf(fn, "# Title=\xcc\xd6\xc2\xdb\xc7\xf8\xbe\xab\xbb\xaa\n");
		fprintf(fn, "#\n");
		fclose(fn);

		//% linkto("0Announce", "groups", "讨论区精华");
		linkto("0Announce", "groups", "\xcc\xd6\xc2\xdb\xc7\xf8\xbe\xab\xbb\xaa");
	}
	if (!dashd(gpath)) {
		mkdir(gpath, 0755);
		chmod(gpath, 0755);
		sprintf(buf, "%s/.Names", gpath);
		if ( (fn = fopen(buf, "w")) == NULL)
			return -1;
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s\n", gname);
		fprintf(fn, "#\n");
		fclose(fn);
		//linkto("0Announce/groups", gname, group, gname);
		linkto("0Announce/groups", group, gname);
	}
	if (!dashd(bpath)) {
		mkdir(bpath, 0755);
		chmod(bpath, 0755);
		//linkto(gpath, title, bname, title);
		linkto(gpath, bname, title);
		sprintf(buf, "%s/.Names", bpath);
		if ((fn = fopen(buf, "w")) == NULL) {
			return -1;
		}
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s\n", title);
		fprintf(fn, "#\n");
		fclose(fn);
	}
	//add by fangu 2003.2.26 ,add log
	//sprintf(genbuf, "ANN add_grp %s", bpath);
	//report(genbuf);
	return 0;

}

int del_grp(char grp[STRLEN], char bname[STRLEN], char title[STRLEN])
{
	char buf[STRLEN], buf2[STRLEN], buf3[30];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];
	char check[30];
	int i, n;
	MENU pm;
	strlcpy(buf3, grp, 29);
	buf3[29] = '\0';
	sprintf(buf, "0Announce/.Search");
	sprintf(gpath, "0Announce/groups/%s", buf3);
	sprintf(bpath, "%s/%s", gpath, bname);
	deltree(bpath);

	pm.path = gpath;
	a_loadnames(&pm);
	for (i = 0; i < pm.num; i++) {
		strcpy(buf2, pm.item[i]->title);
		strcpy(check, strtok(pm.item[i]->fname, "/~\n\b"));
		if (strstr(buf2, title) && !strcmp(check, bname)) {
			free(pm.item[i]);
			(pm.num)--;
			for (n = i; n < pm.num; n++)
				pm.item[n] = pm.item[n + 1];
			a_savenames(&pm);
			break;
		}
	}
	//add by fangu 2003.2.26, add log
	//sprintf(genbuf, "ANN del_grp %s", bpath);
	//report(genbuf);
	return 0;
}

int edit_grp(char bname[STRLEN], char grp[STRLEN], char title[STRLEN],
		char newtitle[STRLEN]) {
	char buf[STRLEN], buf2[STRLEN], buf3[30];
	char gpath[STRLEN * 2];
	char bpath[STRLEN * 2];
	int i;
	MENU pm;
	strlcpy(buf3, grp, 29);
	buf3[29] = '\0';
	sprintf(buf, "0Announce/.Search");
	sprintf(gpath, "0Announce/groups/%s", buf3);
	sprintf(bpath, "%s/%s", gpath, bname);
	if (!seek_in_file(buf, bname))
		return 0;

	pm.path = gpath;
	a_loadnames(&pm);
	for (i = 0; i < pm.num; i++) {
		strlcpy(buf2, pm.item[i]->title, STRLEN);
		buf2[STRLEN - 1] = '\0';
		if (strstr(buf2, title) && strstr(pm.item[i]->fname, bname)) {
			//add by fangu 2003.2.26, add log
			//sprintf(genbuf, "ANN edit_grp file:%s-old title:%s -
			//new title:%s", gpath,pm.item[i]->title,newtitle);
			//report(genbuf);
			strlcpy(pm.item[i]->title, newtitle, STRLEN);
			break;
		}
	}
	a_savenames(&pm);
	pm.path = bpath;
	a_loadnames(&pm);
	strlcpy(pm.mtitle, newtitle, STRLEN);
	a_savenames(&pm);
	return 0;
}

int AddPCorpus() {
	FILE *fn;
	char digestpath[80] = "0Announce/groups/GROUP_0/PersonalCorpus";
	char personalpath[80], Log[200], title[200], ftitle[80];
	char *title1 = title;
	int id;
	set_user_status(ST_DIGEST);
	sprintf(Log, "%s/Log", digestpath);
	if (!check_systempasswd()) {
		return 1;
	}
	screen_clear();
	//% stand_title("创建个人文集");
	stand_title("\xb4\xb4\xbd\xa8\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf");

	move(4, 0);
	//% if (!gettheuserid(1, "请输入使用者代号: ", &id))
	if (!gettheuserid(1, "\xc7\xeb\xca\xe4\xc8\xeb\xca\xb9\xd3\xc3\xd5\xdf\xb4\xfa\xba\xc5: ", &id))
		return 1;

	sprintf(personalpath, "%s/%c/%s", digestpath,
			toupper(lookupuser.userid[0]), lookupuser.userid);
	if (dashd(personalpath)) {
		//% presskeyfor("该用户的个人文集目录已存在, 按任意键取消..", 4);
		presskeyfor("\xb8\xc3\xd3\xc3\xbb\xa7\xb5\xc4\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf\xc4\xbf\xc2\xbc\xd2\xd1\xb4\xe6\xd4\xda, \xb0\xb4\xc8\xce\xd2\xe2\xbc\xfc\xc8\xa1\xcf\xfb..", 4);
		return 1;
	}

	screen_move_clear(4);
	//% if (askyn("确定要为该用户创建一个个人文集吗?", YEA, NA)==NA) {
	if (askyn("\xc8\xb7\xb6\xa8\xd2\xaa\xce\xaa\xb8\xc3\xd3\xc3\xbb\xa7\xb4\xb4\xbd\xa8\xd2\xbb\xb8\xf6\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf\xc2\xf0?", YEA, NA)==NA) {
		//% presskeyfor("您选择取消创建. 按任意键取消...", 6);
		presskeyfor("\xc4\xfa\xd1\xa1\xd4\xf1\xc8\xa1\xcf\xfb\xb4\xb4\xbd\xa8. \xb0\xb4\xc8\xce\xd2\xe2\xbc\xfc\xc8\xa1\xcf\xfb...", 6);
		return 1;
	}
	mkdir(personalpath, 0755);
	chmod(personalpath, 0755);

	screen_move_clear(7);
	//% prints("[直接按 ENTER 键, 则标题缺省为: [32m%s 的个人文集[m]", lookupuser.userid);
	prints("[\xd6\xb1\xbd\xd3\xb0\xb4 ENTER \xbc\xfc, \xd4\xf2\xb1\xea\xcc\xe2\xc8\xb1\xca\xa1\xce\xaa: [32m%s \xb5\xc4\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf[m]", lookupuser.userid);
	//% getdata(6, 0, "请输入个人文集之标题: ", title, 39, DOECHO, YEA);
	getdata(6, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf\xd6\xae\xb1\xea\xcc\xe2: ", title, 39, DOECHO, YEA);
	if (title[0] == '\0')
		//% title1 += sprintf(title, "%s 的个人文集", lookupuser.userid);
		title1 += sprintf(title, "%s \xb5\xc4\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf", lookupuser.userid);
	sprintf(title1, "%-38.38s(BM: %s)", title, lookupuser.userid);
	sprintf(digestpath, "0Announce/groups/GROUP_0/PersonalCorpus/%c", 
			toupper(lookupuser.userid[0]));
	//% sprintf(ftitle, "个人文集 －－ (首字母 %c)", toupper(lookupuser.userid[0]));
	sprintf(ftitle, "\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf \xa3\xad\xa3\xad (\xca\xd7\xd7\xd6\xc4\xb8 %c)", toupper(lookupuser.userid[0]));
	//linkto(digestpath, ftitle, lookupuser.userid, title);
	linkto(digestpath, lookupuser.userid, title);
	sprintf(personalpath, "%s/.Names", personalpath);
	if ((fn = fopen(personalpath, "w")) == NULL) {
		return -1;
	}
	fprintf(fn, "#\n");
	fprintf(fn, "# Title=%s\n", title);
	fprintf(fn, "#\n");
	fclose(fn);
	if (!(lookupuser.userlevel & PERM_BOARDS)) {
		char secu[STRLEN];
		lookupuser.userlevel |= PERM_BOARDS;
		substitut_record(PASSFILE, &lookupuser, sizeof(struct userec), id);
		//% sprintf(secu, "建立个人文集, 给予 %s 版主权限", lookupuser.userid);
		sprintf(secu, "\xbd\xa8\xc1\xa2\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf, \xb8\xf8\xd3\xe8 %s \xb0\xe6\xd6\xf7\xc8\xa8\xcf\xde", lookupuser.userid);
		securityreport(secu, 0, 0);
		move(9, 0);
		outs(secu);
		sethomefile(genbuf, lookupuser.userid, ".announcepath");
		report(genbuf, currentuser.userid);
		if ((fn = fopen(genbuf, "w+")) != NULL) {
			fprintf(fn, "%s/%s", digestpath, lookupuser.userid);
			fclose(fn);
		}
	}
	sprintf(genbuf, "\033[36m%-12.12s\033[m %14.14s \033[32m %.38s\033[m\n",
			lookupuser.userid, format_time(fb_time(), TIME_FORMAT_ZH), title);
	file_append(Log, genbuf);
	//% presskeyfor("已经创建个人文集, 请按任意键继续...", 12);
	presskeyfor("\xd2\xd1\xbe\xad\xb4\xb4\xbd\xa8\xb8\xf6\xc8\xcb\xce\xc4\xbc\xaf, \xc7\xeb\xb0\xb4\xc8\xce\xd2\xe2\xbc\xfc\xbc\xcc\xd0\xf8...", 12);
	return 0;
}

void Announce() {
	//% sprintf(genbuf, "%s 精华区公布栏", BoardName);
	sprintf(genbuf, "%s \xbe\xab\xbb\xaa\xc7\xf8\xb9\xab\xb2\xbc\xc0\xb8", BoardName);
	a_menu(genbuf, "0Announce", HAS_PERM(PERM_ANNOUNCE) ? PERM_BOARDS : 0,
			0);
	screen_clear();
}

/* coded by stiger, immigrate to fudan by jacobson 2005.5.21 */

/* 寻找丢失条目 */
/* 返回值：添加了几条 */

int a_repair(MENU *pm)
{
	DIR *dirp;
	struct dirent *direntp;
	int i, changed;

	changed=0;
	dirp=opendir(pm->path);
	if (dirp==NULL)
		return 0;

	while ( (direntp=readdir(dirp)) != NULL) {
		if (direntp->d_name[0]=='.')
			continue;
		if (strcasecmp(direntp->d_name, "counter.person")==0)
			continue;
		for (i=0; i < pm->num; i++) {
			if (strcmp(pm->item[i]->fname, direntp->d_name)==0) {
				i=-1;
				break;
			}
		}
		if (i!=-1) {
			a_additem(pm, direntp->d_name, direntp->d_name);
			changed++;
		}
	}
	closedir(dirp);
	if (changed>0) {
		//if(a_savenames(pm) != 0)
		a_savenames(pm);
		//changed = 0 - changed;
	}
	return changed;
}
/* add end */

struct AnnPath {
	int num;
	char title[STRLEN];
	char path[PATHLEN];
} import_path[MAXANNPATHS];
int curr_annpath;
int ann_mode;
int show_mode = 0;
//% char show_items[3][8] = { "[名称]", "[目录]", "[路径]" };
char show_items[3][8] = { "[\xc3\xfb\xb3\xc6]", "[\xc4\xbf\xc2\xbc]", "[\xc2\xb7\xbe\xb6]" };

void ann_set_title_show(char* title) {
	char buf[256], path[256];
	move(0, 0);
	rtrim(title);
	sprintf(path, "[%-1.56s] %s", title, show_items[show_mode]);
	//% sprintf(buf, "[1;44;33m 设定丝路 %69.69s[m\n", path);
	sprintf(buf, "[1;44;33m \xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7 %69.69s[m\n", path);
	outs(buf);
	//% prints(" 退出[[1;32m←[m] 增加/替换[[1;32m→ Enter[m] 改名[[1;32mt[m] 删除[[1;32md[m] 清空[[1;32mc[m] 移动[[1;32mm[m] 进入[[1;32mg[m] 帮助[[1;32mh[m]\n");
	prints(" \xcd\xcb\xb3\xf6[[1;32m\xa1\xfb[m] \xd4\xf6\xbc\xd3/\xcc\xe6\xbb\xbb[[1;32m\xa1\xfa Enter[m] \xb8\xc4\xc3\xfb[[1;32mt[m] \xc9\xbe\xb3\xfd[[1;32md[m] \xc7\xe5\xbf\xd5[[1;32mc[m] \xd2\xc6\xb6\xaf[[1;32mm[m] \xbd\xf8\xc8\xeb[[1;32mg[m] \xb0\xef\xd6\xfa[[1;32mh[m]\n");
	//% prints("[1;44;37m 编号  丝路                                                                    [m\n");
	prints("[1;44;37m \xb1\xe0\xba\xc5  \xcb\xbf\xc2\xb7                                                                    [m\n");
}

void ann_get_title_show() {
	char buf[256];
	move(0, 0);
	//% sprintf(buf, "[1;44;33m 使用丝路 %69.69s[m\n", show_items[show_mode]);
	sprintf(buf, "[1;44;33m \xca\xb9\xd3\xc3\xcb\xbf\xc2\xb7 %69.69s[m\n", show_items[show_mode]);
	outs(buf);
	//% prints(" 退出[[1;32m←[m] 查看[[1;32m→[m] 移动[[1;32m↑[m,[1;32m↓[m] 查看[[1;32mS[m] 模式[[1;32mf[m] 查寻[[1;32m/[m] 帮助[[1;32mh[m]\n");
	prints(" \xcd\xcb\xb3\xf6[[1;32m\xa1\xfb[m] \xb2\xe9\xbf\xb4[[1;32m\xa1\xfa[m] \xd2\xc6\xb6\xaf[[1;32m\xa1\xfc[m,[1;32m\xa1\xfd[m] \xb2\xe9\xbf\xb4[[1;32mS[m] \xc4\xa3\xca\xbd[[1;32mf[m] \xb2\xe9\xd1\xb0[[1;32m/[m] \xb0\xef\xd6\xfa[[1;32mh[m]\n");
	//% prints("[1;44;37m 编号  丝路                                                                    [m\n");
	prints("[1;44;37m \xb1\xe0\xba\xc5  \xcb\xbf\xc2\xb7                                                                    [m\n");
}

int save_import_path() {
	FILE *fp;
	sethomefile(genbuf, currentuser.userid, "annpath");
	if ((fp = fopen(genbuf, "w")) != NULL) {
		fwrite(import_path, sizeof(struct AnnPath), MAXANNPATHS, fp);
		fclose(fp);
	}
	return 1;
}

int load_import_path() {
	FILE *fp;
	int count = 0;
	sethomefile(genbuf, currentuser.userid, "annpath");
	if ((fp = fopen(genbuf, "r")) != NULL) {
		count = fread(import_path, sizeof(struct AnnPath), MAXANNPATHS,	fp);
		fclose(fp);
	}
	if (count != MAXANNPATHS)
		clear_ann_path();
	return 1;
}

static int change_ann_path(int index, const char *title, const char *path, int mode)
{
	import_path[index].num = index;
	char anntitle[STRLEN];
	strlcpy(anntitle, title, STRLEN);
	if (index != -1) {
		move(1, 0);
		rtrim(anntitle);
		//% getdata(1, 0, "设定丝路名:", anntitle, STRLEN, DOECHO, NA);
		getdata(1, 0, "\xc9\xe8\xb6\xa8\xcb\xbf\xc2\xb7\xc3\xfb:", anntitle, STRLEN, DOECHO, NA);
		if (anntitle[0]=='\0')
			return 0;
		strlcpy(import_path[index].title, anntitle, STRLEN);
		if (mode == 0) {
			strlcpy(import_path[index].path, path, PATHLEN);
		}
	}
	return save_import_path();
}

int del_ann_path(int index) {
	import_path[index].num = -1;
	import_path[index].title[0] = '\0';
	import_path[index].path[0] = '\0';
	return save_import_path();
}

int clear_ann_path(void)
{
	int i;
	for (i = 0; i < MAXANNPATHS; i++) {
		import_path[i].num = -1;
		import_path[i].title[0] = '\0';
		import_path[i].path[0] = '\0';
	}
	return save_import_path();
}

//mode == 0 f设定丝路
//mode == 1 I选择丝路
//return 0	丝路设定失败
//return 1	丝路设定成功
//show_mode == 0	丝路名
//show_mode == 1	丝路精华区目录
//show_mode == 2	丝路档名
int set_ann_path(const char *title, const char *path, int mode)
{
	int from = curr_annpath / A_PAGESIZE * A_PAGESIZE, to = 0;
	int ch;
	int redrawflag = 1;
	int y = 3, number=0;
	char buf[1024];
	FILE *fn;

	ann_mode = mode;

	load_import_path();
	show_mode = 0;

	while (1) {
		if (redrawflag) {
			screen_clear();

			if (ann_mode == ANNPATH_SETMODE) {
				strcpy(buf, path);
				ann_to_title(buf);
				ann_set_title_show(buf);
			} else if (ann_mode == ANNPATH_GETMODE)
				ann_get_title_show();

			to=from;
			y=3;
			move(y, 0);
			while (y < screen_lines()-1 && to < MAXANNPATHS) {
				if (show_mode == 0)
					sprintf(genbuf, "%4d  %-72.72s", to+1,
							//% import_path[to].num == -1 ? "[32m<尚未设定>[m"
							import_path[to].num == -1 ? "[32m<\xc9\xd0\xce\xb4\xc9\xe8\xb6\xa8>[m"
									: import_path[to].title);
				else if (show_mode == 2)
					sprintf(genbuf, "%4d  %-72.72s", to+1,
							//% import_path[to].num == -1 ? "[32m<尚未设定>[m"
							import_path[to].num == -1 ? "[32m<\xc9\xd0\xce\xb4\xc9\xe8\xb6\xa8>[m"
									: import_path[to].path);
				else if (show_mode == 1) {
					strlcpy(buf, import_path[to].path, PATHLEN);
					ann_to_title(buf);
					sprintf(genbuf, "%4d  %-72.72s", to+1,
							//% import_path[to].num == -1 ? "[32m<尚未设定>[m"
							import_path[to].num == -1 ? "[32m<\xc9\xd0\xce\xb4\xc9\xe8\xb6\xa8>[m"
									: buf);
				}

				strtok(genbuf, "\n");
				prints(" %-78s\n", genbuf);
				to++;
				y++;
			}//while
			if (from==to) {
				from-=(A_PAGESIZE-1);
				if (from<0)
					from=0;
				curr_annpath=from;
				continue;
			}
			if (curr_annpath>to-1)
				curr_annpath= to -1;
			tui_update_status_line();
		}//if 
		move(3+curr_annpath-from, 0);
		prints(">");
		ch = egetch();
		redrawflag=0;
		move(3+curr_annpath-from, 0);
		prints(" ");

		switch (ch) {
			case 'h':
			case 'H':
				show_help("help/import_announcehelp");
				redrawflag=1;
				break;
			case 'k':
			case 'K':
			case KEY_UP:
				curr_annpath--;
				if (curr_annpath < 0) {
					curr_annpath = MAXANNPATHS-1;
					from += MAXANNPATHS - A_PAGESIZE;
					redrawflag=1;
				} else if (curr_annpath < from) {
					from -= A_PAGESIZE;
					if (from < 0)
						from = 0;
					if (curr_annpath<from)
						curr_annpath=from;
					redrawflag=1;
				}
				break;
			case 'j':
			case 'J':
			case KEY_DOWN:
				curr_annpath++;
				if (curr_annpath > MAXANNPATHS-1) {
					from = 0;
					curr_annpath = 0;
					redrawflag=1;
				} else if (curr_annpath > to - 1) {
					from += A_PAGESIZE;
					if (from > MAXANNPATHS-1)
						from = 0;
					if (curr_annpath<from)
						curr_annpath=from;
					redrawflag=1;
				}
				break;
			case Ctrl('B'):
			case KEY_PGUP:
				from-=A_PAGESIZE;
				curr_annpath-=A_PAGESIZE;
				if (from == -A_PAGESIZE) {
					from+=MAXANNPATHS;
					curr_annpath= (curr_annpath+A_PAGESIZE) % A_PAGESIZE;

				} else if (from<0) {
					from=0;
				}
				if (curr_annpath<from)
					curr_annpath=from;
				redrawflag=1;
				break;
			case Ctrl('F'):
			case KEY_PGDN:
				from+=A_PAGESIZE;
				curr_annpath+=A_PAGESIZE;
				if (from > MAXANNPATHS-1) {
					from = 0;
					curr_annpath %= A_PAGESIZE;
				}
				if (curr_annpath<from)
					curr_annpath=from;
				redrawflag=1;
				break;
			case KEY_HOME:
				from = 0;
				curr_annpath = 0;
				redrawflag=1;
				break;
			case KEY_END:
				from = MAXANNPATHS-A_PAGESIZE;
				curr_annpath = MAXANNPATHS-1;
				redrawflag=1;
				break;
			case KEY_LEFT:
			case KEY_ESC:
				return 0;
			case 'd': //删除
			case 'D': //删除
				move(1, 0);
				redrawflag=1;
				//% if (askyn("删除丝路吗？", NA, NA)==NA)
				if (askyn("\xc9\xbe\xb3\xfd\xcb\xbf\xc2\xb7\xc2\xf0\xa3\xbf", NA, NA)==NA)
					break;
				del_ann_path(curr_annpath);
				break;
			case 'c': //清除
			case 'C': //清除
				move(1, 0);
				redrawflag=1;
				//% if (askyn("清除所有丝路吗?", NA, NA)==NA)
				if (askyn("\xc7\xe5\xb3\xfd\xcb\xf9\xd3\xd0\xcb\xbf\xc2\xb7\xc2\xf0?", NA, NA)==NA)
					break;
				clear_ann_path();
				break;
			case 'S':
				sethomefile(genbuf, currentuser.userid, ".announcepath");
				fn = fopen(genbuf, "r");
				if (fn) {
					fgets(buf, 1000, fn);
					fclose(fn);
					ann_to_title(buf);
					//% sprintf(genbuf, "当前丝路: %s", buf);
					sprintf(genbuf, "\xb5\xb1\xc7\xb0\xcb\xbf\xc2\xb7: %s", buf);
				} else
					//% strcpy(genbuf, "您还没有用 f 设置丝路.");
					strcpy(genbuf, "\xc4\xfa\xbb\xb9\xc3\xbb\xd3\xd0\xd3\xc3 f \xc9\xe8\xd6\xc3\xcb\xbf\xc2\xb7.");
				presskeyfor(genbuf, -1);
				redrawflag=1;
				break;
			case 't'://重名名
			case 'T'://重名名
				if (import_path[curr_annpath].num == -1)
					break;
				if (ann_mode == ANNPATH_SETMODE) {
					move(1, 0);
					change_ann_path(curr_annpath,
							import_path[curr_annpath].title, NULL, 1);
				}
				redrawflag=1;
				break;
			case 's':
				show_mode = !show_mode;
				redrawflag=1;
				break;
			case 'f'://修改模式
			case 'F'://修改模式
				show_mode = (show_mode + 1) % 3;
				redrawflag=1;
				break;
			case 'g'://进入丝路
			case 'G'://进入丝路
				if (import_path[curr_annpath].num == -1)
					break;
				if (ann_mode == ANNPATH_SETMODE) {
					if (dashd(import_path[curr_annpath].path)) {
						a_menu("", import_path[curr_annpath].path,
								PERM_BOARDS, 0);
						return 0;
					} else
						//% 该丝路已丢失, 请重新设定
						presskeyfor("\xb8\xc3\xcb\xbf\xc2\xb7\xd2\xd1\xb6\xaa\xca\xa7, \xc7\xeb\xd6\xd8\xd0\xc2\xc9\xe8\xb6\xa8", -1);

				}
				break;
			case 'm':
			case 'M':
				if (import_path[curr_annpath].num != -1) {
					buf[0] = '\0';
					//% getdata(1, 0, "要移动的位置:", buf, 4, DOECHO, NA);
					getdata(1, 0, "\xd2\xaa\xd2\xc6\xb6\xaf\xb5\xc4\xce\xbb\xd6\xc3:", buf, 4, DOECHO, NA);
					if ((buf[0] != 0) && isdigit(buf[0])) {
						int new_pos;
						new_pos = atoi(buf);
						new_pos--;
						if ((new_pos >= 0) && (new_pos < MAXANNPATHS)
								&& (new_pos != curr_annpath)) {
							if (import_path[new_pos].num != -1) {
								strlcpy(buf,
										import_path[curr_annpath].title,
										STRLEN);
								strlcpy(import_path[curr_annpath].title,
										import_path[new_pos].title, STRLEN);
								strlcpy(import_path[new_pos].title, buf,
										STRLEN);

								strlcpy(buf,
										import_path[curr_annpath].path,
										STRLEN);
								strlcpy(import_path[curr_annpath].path,
										import_path[new_pos].path, STRLEN);
								strlcpy(import_path[new_pos].path, buf,
										STRLEN);
							} else {
								strlcpy(import_path[new_pos].title,
										import_path[curr_annpath].title,
										STRLEN);
								strlcpy(import_path[new_pos].path,
										import_path[curr_annpath].path,
										STRLEN);
								import_path[new_pos].num = new_pos;
								import_path[curr_annpath].num = -1;
							}
							save_import_path();
							curr_annpath = new_pos;
						}
					}
					redrawflag=1;
				}
				break;
			case KEY_RIGHT:
			case '\n':
				redrawflag=1;
				if (number > 0) {
					curr_annpath = number - 1;
					from = curr_annpath /A_PAGESIZE * A_PAGESIZE;
					number = 0;

				} else {
					move(1, 0);
					if (ann_mode == ANNPATH_SETMODE) {
						if (import_path[curr_annpath].num == -1) {
							change_ann_path(curr_annpath, title, path, 0);
						//% } else if (askyn("要覆盖已有的丝路么？", NA, NA)== YEA)
						} else if (askyn("\xd2\xaa\xb8\xb2\xb8\xc7\xd2\xd1\xd3\xd0\xb5\xc4\xcb\xbf\xc2\xb7\xc3\xb4\xa3\xbf", NA, NA)== YEA)
							change_ann_path(curr_annpath, title, path, 0);
					} else if (ann_mode == ANNPATH_GETMODE) {
						if (import_path[curr_annpath].num != -1) {
							//% sprintf(genbuf, "放入丝路%d [%-1.50s]吗?",
							sprintf(genbuf, "\xb7\xc5\xc8\xeb\xcb\xbf\xc2\xb7%d [%-1.50s]\xc2\xf0?",
									import_path[curr_annpath].num + 1,
									import_path[curr_annpath].title);
							if (askyn(genbuf, YEA, NA)==YEA) {
								sethomefile(genbuf, currentuser.userid,
										".announcepath");
								if ((fn = fopen(genbuf, "w+")) != NULL) {
									fprintf(fn, "%s",
											import_path[curr_annpath].path);
									fclose(fn);
								}
								return 1;
							}
						}

					}
				}
				break;
			default:
				if (ch >= '0' && ch <= '9') {
					number = number * 10 + (ch - '0');
					ch = '\0';
				} else {
					number = 0;
				}
				break;
		}// switch
	}//while (1)
	return 0;
}

//若dst为目录,且并非.,..,最后一个字符不为/,
//			将其删除,成功返回	1
//					 否则返回	0
int deltree(const char *dst)
{
	if (strstr(dst, "//") || strstr(dst, "..") || strchr(dst, ' '))
		return 0; /* precaution */
	if (dst[strlen(dst) - 1] == '/')
		return 0;
	if (dashd(dst)) {
		sprintf(genbuf, "/bin/rm -rf %s", dst);
		system(genbuf);
		return 1;
	} else
		return 0;
}
