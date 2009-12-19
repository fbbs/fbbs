#include "bbs.h"

#define BBS_PAGESIZE    (t_lines - 4)

char *sysconf_str();
extern time_t login_start_time;
struct newpostdata {
	char *name, *title, *BM;
	unsigned int flag;
	int pos, total;
	char unread, zap;
	char status;
}*nbrd;

struct goodboard GoodBrd;

int *zapbuf = NULL;
int brdnum, yank_flag = 0;
int boardparent;
char *boardprefix;
int choosemode;

void EGroup (cmd)
char *cmd;
{
	char buf[STRLEN];
	sprintf (buf, "EGROUP%c", *cmd);
	GoodBrd.num = 0;
	GoodBrd.nowpid = -1;
	choosemode = 1;
	boardprefix = sysconf_str(buf);
	choose_board (DEFINE (DEF_NEWPOST) ? 1 : 0);
}

void BoardGroup() {
	GoodBrd.num = 0;
	boardparent = 0;
	choosemode = 0;
	boardprefix = NULL;
	GoodBrd.nowpid = -1;
	choose_board(DEFINE(DEF_NEWPOST) ? 1 : 0);
}

void Boards() {
	boardprefix = NULL;
	boardparent = -1;
	GoodBrd.num = 0;
	GoodBrd.nowpid = -1;
	choose_board(0);
}

void GoodBrds() {
	if (!strcmp(currentuser.userid, "guest"))
		return;
	//GoodBrd.num = 9999;
	boardprefix = NULL;
	boardparent = -2;
	choosemode = 0;
	GoodBrd.nowpid = 0; //added by cometcaptor 2007-04-21
	choose_board(1);
	GoodBrd.nowpid = -1;
	GoodBrd.num = 0;
}

void New() {
	boardprefix = NULL;
	boardparent = -1;
	GoodBrd.num = 0;
	GoodBrd.nowpid = -1;
	choose_board(1);
}

//int inGoodBrds (char *bname)
int inGoodBrds(int pos) //modified by cometcaptor 2007-04-21 ¼ò»¯²éÕÒÁ÷³Ì
{
	int i;
	for (i = 0; i < GoodBrd.num && i < GOOD_BRC_NUM; i++)
		//if (!strcmp (bname, GoodBrd.ID[i]))
		if ((GoodBrd.boards[i].pid == GoodBrd.nowpid)
				&&(!(GoodBrd.boards[i].flag & BOARD_CUSTOM_FLAG))&&(pos
				== GoodBrd.boards[i].pos)) //modified by cometcaptor
			return i + 1;
	return 0;
}

void load_zapbuf() {
	char fname[STRLEN];
	int fd, size, n;

	size = MAXBOARD * sizeof(int);
	zapbuf = (int *) malloc(size);
	for (n = 0; n < MAXBOARD; n++)
		zapbuf[n] = 1;
	setuserfile(fname, ".lastread");
	if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
		size = numboards * sizeof(int);
		read(fd, zapbuf, size);
		close(fd);
	}
}

void load_GoodBrd() {
	int i;
	char fname[STRLEN];
	FILE *fp;

	GoodBrd.num = 0;
	setuserfile(fname, ".goodbrd");
	//modified by cometcaptor 2007-04-23 ÊÕ²Ø¼Ð×Ô¶¨ÒåÄ¿Â¼
	if ((fp = fopen(fname, "rb"))) {
		while (fread(&GoodBrd.boards[GoodBrd.num],
				sizeof(struct goodbrdheader), 1, fp)) {
			if (GoodBrd.boards[GoodBrd.num].flag & BOARD_CUSTOM_FLAG)
				GoodBrd.num++;
			else {
				i = GoodBrd.boards[GoodBrd.num].pos;
				if ((bcache[i].filename[0]) && (bcache[i].flag
						& BOARD_POST_FLAG //pÏÞÖÆ°æÃæ
						|| HAS_PERM(bcache[i].level) //È¨ÏÞ×ã¹»
				||(bcache[i].flag & BOARD_NOZAP_FLAG))) //²»¿Ézap
					GoodBrd.num++;
			}
			if (GoodBrd.num == GOOD_BRC_NUM)
				break;
		}
		fclose(fp);
	}

	if (GoodBrd.num == 0) {
		GoodBrd.num = 1;
		i = getbnum(DEFAULTBOARD, &currentuser);
		if (i == 0)
			i = getbnum(currboard, &currentuser);
		GoodBrd.boards[0].id = 1;
		GoodBrd.boards[0].pid = 0;
		GoodBrd.boards[0].pos = i-1;
		GoodBrd.boards[0].flag = bcache[i-1].flag;
		strcpy(GoodBrd.boards[0].filename, bcache[i-1].filename);
		strcpy(GoodBrd.boards[0].title, bcache[i-1].title);
	}
}

void save_GoodBrd() {
	int i;
	FILE *fp;
	char fname[STRLEN];

	//modified by cometcaptor 2007-04-21
	if (GoodBrd.num == 0) {
		GoodBrd.num = 1;
		i = getbnum(DEFAULTBOARD, &currentuser);
		if (i == 0)
			i = getbnum(currboard, &currentuser);
		GoodBrd.boards[0].id = 1;
		GoodBrd.boards[0].pid = 0;
		GoodBrd.boards[0].pos = i-1;
		GoodBrd.boards[0].flag = bcache[i-1].flag;
		strcpy(GoodBrd.boards[0].filename, bcache[i-1].filename);
		strcpy(GoodBrd.boards[0].title, bcache[i-1].title);
	}
	setuserfile(fname, ".goodbrd");
	if ((fp = fopen(fname, "wb")) != NULL) {
		for (i = 0; i < GoodBrd.num; i++) {
			fwrite(&GoodBrd.boards[i], sizeof(struct goodbrdheader), 1, fp);
			report(GoodBrd.boards[i].filename, currentuser.userid);
		}
		fclose(fp);
	}
}

void add_GoodBrd(char *bname, int pid) //cometcaptor 2007-04-21
{
	int i = getbnum(bname, &currentuser);
	if ((i > 0)&&(GoodBrd.num < GOOD_BRC_NUM)) {
		i--;
		GoodBrd.boards[GoodBrd.num].pid = pid;
		GoodBrd.boards[GoodBrd.num].pos = i;
		strcpy(GoodBrd.boards[GoodBrd.num].filename, bcache[i].filename);
		strcpy(GoodBrd.boards[GoodBrd.num].title, bcache[i].title);
		GoodBrd.boards[GoodBrd.num].flag = bcache[i].flag;
		//»¹ÊÇ²»ÒªÓÃÄÇÃ´¸´ÔÓµÄÑ­»·ÕÒ¿ÕÁË£¬Ã»±ØÒª
		if (GoodBrd.num)
			GoodBrd.boards[GoodBrd.num].id
					= GoodBrd.boards[GoodBrd.num-1].id + 1;
		else
			GoodBrd.boards[GoodBrd.num].id = 1;
		GoodBrd.num++;
		save_GoodBrd();
	}
}

void mkdir_GoodBrd(char *dirname, char *dirtitle, int pid)//cometcaptor 2007-04-21
{
	//pidÔÝÊ±ÊÇ¸ö²»Ê¹ÓÃµÄ²ÎÊý£¬ÒòÎª²»´òËã½¨¶þ¼¶Ä¿Â¼£¨É¾³ýÄ¿Â¼µÄ´úÂëÄ¿Â¼ÈÔ²»ÍêÉÆ£©
	if (GoodBrd.num < GOOD_BRC_NUM) {
		GoodBrd.boards[GoodBrd.num].pid = 0;
		strcpy(GoodBrd.boards[GoodBrd.num].filename, dirname);
		strcpy(GoodBrd.boards[GoodBrd.num].title, "~[ÊÕ²Ø] ¡ð ");
		if (dirtitle[0] != '\0')
			strcpy(GoodBrd.boards[GoodBrd.num].title+11, dirtitle);
		else
			strcpy(GoodBrd.boards[GoodBrd.num].title+11, "×Ô¶¨ÒåÄ¿Â¼");
		GoodBrd.boards[GoodBrd.num].flag = BOARD_DIR_FLAG
				| BOARD_CUSTOM_FLAG;
		GoodBrd.boards[GoodBrd.num].pos = -1;
		if (GoodBrd.num)
			GoodBrd.boards[GoodBrd.num].id
					= GoodBrd.boards[GoodBrd.num-1].id + 1;
		else
			GoodBrd.boards[GoodBrd.num].id = 1;
		GoodBrd.num++;
		save_GoodBrd();
	}
}

void rmdir_GoodBrd(int id)//cometcaptor 2007-04-21
{
	//Ä¿Â¼Ã»ÓÐ¶Ô¶þ¼¶Ä¿Â¼Ç¶Ì×É¾³ýµÄ¹¦ÄÜ£¬Ò²ÒòÎªÕâ¸öÏÞÖÆ£¬ÊÕ²Ø¼ÐÄ¿Â¼²»ÔÊÐí½¨Á¢¶þ¼¶Ä¿Â¼
	int i, n = 0;
	for (i = 0; i < GoodBrd.num; i++) {
		if (((GoodBrd.boards[i].flag & BOARD_CUSTOM_FLAG)
				&&(GoodBrd.boards[i].id == id)) ||(GoodBrd.boards[i].pid
				== id))
			continue;
		else {
			if (i != n)
				memcpy(&GoodBrd.boards[n], &GoodBrd.boards[i],
						sizeof(struct goodbrdheader));
			n++;
		}
	}
	GoodBrd.num = n;
	save_GoodBrd();
}

void save_zapbuf() {
	char fname[STRLEN];
	int fd, size;
	setuserfile(fname, ".lastread");
	if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
		size = numboards * sizeof(int);
		write(fd, zapbuf, size);
		close(fd);
	}
}

int load_boards() {
	struct boardheader *bptr;
	struct newpostdata *ptr;
	int n, addto = 0, goodbrd = 0;
	// resolve_boards ();
	if (zapbuf == NULL) {
		load_zapbuf();
	}
	brdnum = 0;
	if (GoodBrd.nowpid >= 0) {
		load_GoodBrd();
		goodbrd = 1;
	}
	for (n = 0; n < numboards; n++) {
		bptr = &bcache[n];
		if (!(bptr->filename[0]))
			continue; /* Òþ²Ø±»É¾³ýµÄ°æÃæ */
		if (goodbrd == 0) {
			if (!(bptr->flag & BOARD_POST_FLAG) && !HAS_PERM(bptr->level)
					&& !(bptr->flag & BOARD_NOZAP_FLAG))
				continue;
			if ((bptr->flag & BOARD_CLUB_FLAG)&& (bptr->flag
					& BOARD_READ_FLAG )&& !chkBM(bptr, &currentuser)
					&& !isclubmember(currentuser.userid, bptr->filename))
				continue;
			if (choosemode == 0) {
				if (boardparent > 0 && boardparent != bptr->group - 1)
					continue;
				if (boardparent == 0 && bptr->group != 0)
					continue;
				if (boardparent > 0 && bptr->title[0] == '*')
					continue;
			} else {
				if (boardprefix != NULL && strchr(boardprefix,
						bptr->title[0]) == NULL && boardprefix[0] != '*')
					continue;
				if (boardprefix != NULL && boardprefix[0] == '*') {
					if (!strstr(bptr->title, "¡ñ") && !strstr(bptr->title,
							"¡Ñ") && bptr->title[0] != '*')
						continue;
				}
				if (boardprefix == NULL && bptr->title[0] == '*')
					continue;
			}
			addto = yank_flag || zapbuf[n] != 0 || (bptr->flag
					& BOARD_NOZAP_FLAG);
		} else
			//addto = inGoodBrds (bptr->filename);
			addto = inGoodBrds(n); //modified by cometcaptor 2007-04-17
		if (addto) {
			ptr = &nbrd[brdnum++];
			ptr->name = bptr->filename;
			ptr->title = bptr->title;
			ptr->BM = bptr->BM;
			ptr->flag = bptr->flag;
			ptr->pos = n;
			ptr->total = -1;
			ptr->zap = (zapbuf[n] == 0);
			if (bptr ->flag & BOARD_DIR_FLAG) {
				if (bptr->level != 0)
					ptr->status = 'r';
				else
					ptr->status = ' ';
			} else {
				if (bptr->flag & BOARD_NOZAP_FLAG)
					ptr->status = 'z';
				else if (bptr->flag & BOARD_POST_FLAG)
					ptr->status = 'p';
				else if (bptr->flag & BOARD_NOREPLY_FLAG)
					ptr->status = 'x';
				else if (bptr->level != 0)
					ptr->status = 'r';
				else
					ptr->status = ' ';
			}
		}
	}
	//added by cometcaptor 2007-04-21 ¶ÁÈ¡×Ô¶¨ÒåÄ¿Â¼
	if (goodbrd) {
		for (n = 0; n<GoodBrd.num && n<GOOD_BRC_NUM; n++) {
			if ((GoodBrd.boards[n].flag & BOARD_CUSTOM_FLAG)
					&&(GoodBrd.boards[n].pid == GoodBrd.nowpid)) {
				ptr = &nbrd[brdnum++];
				ptr->name = GoodBrd.boards[n].filename;
				ptr->title = GoodBrd.boards[n].title;
				ptr->BM = NULL;
				ptr->flag = GoodBrd.boards[n].flag;
				ptr->pos = GoodBrd.boards[n].id;
				ptr->zap = 0;
				ptr->total = 0;
				ptr->status = ' ';
			}
		}
	}
	//add end
	if (brdnum == 0 && !yank_flag && boardparent == -1) {
		brdnum = -1;
		yank_flag = 1;
		return -1;
	}
	return 0;
}

int search_board(int *num) {
	static int i = 0, find = YEA;
	static char bname[STRLEN];
	int n, ch, tmpn = NA;
	if (find == YEA) {
		bzero(bname, sizeof (bname));
		find = NA;
		i = 0;
	}
	while (1) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s", bname);
		ch = egetch();

		if (isprint2(ch)) {
			bname[i++] = ch;
			for (n = 0; n < brdnum; n++) {
				if (!strncasecmp(nbrd[n].name, bname, i)) {
					tmpn = YEA;
					*num = n;
					if (!strcmp(nbrd[n].name, bname))
						return 1 /* ÕÒµ½ÀàËÆµÄ°æ£¬»­ÃæÖØ»­
						 */;
				}
			}
			if (tmpn)
				return 1;
			if (find == NA) {
				bname[--i] = '\0';
			}
			continue;
		} else if (ch == Ctrl('H') || ch == KEY_LEFT || ch == KEY_DEL
				|| ch == '\177') {
			i--;
			if (i < 0) {
				find = YEA;
				break;
			} else {
				bname[i] = '\0';
				continue;
			}
		} else if (ch == '\t') {
			find = YEA;
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_RIGHT) {
			find = YEA;
			break;
		}
		bell(1);
	}
	if (find) {
		move(t_lines - 1, 0);
		clrtoeol();
		return 2 /* ½áÊøÁË */;
	}
	return 1;
}

int check_newpost (ptr)
struct newpostdata *ptr;
{
	ptr->unread = 0;
	ptr->total = (brdshm->bstatus[ptr->pos]).total;
	if (!brc_initial (currentuser.userid, ptr->name)) {
		ptr->unread = 1;
	} else {
		if (brc_unread1 ((brdshm->bstatus[ptr->pos]).lastpost)) {
			ptr->unread = 1;
		}
	}
	return 1;
}

int unread_position (dirfile, ptr)
char *dirfile;
struct newpostdata *ptr;
{
	struct fileheader fh;
	char filename[STRLEN];
	int fd, offset, step, num;
	num = ptr->total + 1;
	if (ptr->unread && (fd = open (dirfile, O_RDWR))> 0) {
		if (!brc_initial (currentuser.userid, ptr->name)) {
			num = 1;
		} else {
			offset = (int) ((char *) &(fh.filename[0]) - (char *) &(fh));
			num = ptr->total - 1;
			step = 4;
			while (num> 0) {
				lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
				if (read (fd, filename, STRLEN) <= 0 || !brc_unread (filename))
				break;
				num -= step;
				if (step < 32)
				step += step / 2;
			}
			if (num < 0)
			num = 0;
			while (num < ptr->total) {
				lseek (fd, (off_t) (offset + num * sizeof (fh)), SEEK_SET);
				if (read (fd, filename, STRLEN) <= 0 || brc_unread (filename))
				break;
				num++;
			}
		}
		close (fd);
	}
	if (num < 0)
	num = 0;
	return num;
}

#ifndef NEWONLINECOUNT
int *online_num;
int _cntbrd(struct user_info *ui) {
	if (ui->active && ui->pid) {
		if (ui->currbrdnum > 0 && ui->currbrdnum <= numboards)
			online_num[ui->currbrdnum - 1]++;
	}
}

void countbrdonline() {
	register int i;
	static time_t lasttime = 0;
	static time_t now = 0;
	int semid;

	now = time(0);
	if (now - lasttime < 5)
		return;
	semid = sem(SEM_COUNTONLINE);
	if (0 == p_nowait(semid)) {
		lasttime = now;
		online_num = calloc(numboards, sizeof(int));
		bzero(online_num, sizeof(int) * numboards);
		apply_ulist(_cntbrd);
		if (resolve_boards() < 0)
			exit(1);
		for (i = 0; i < numboards; i++) {
			bcache[i].online_num = online_num[i];
		}
		free(online_num);
		v(semid);
	}
}
#endif

void show_brdlist (page, clsflag, newflag)
int page, clsflag, newflag;
{
	struct newpostdata *ptr;
	int n;
	char tmpBM[BM_LEN - 1];
	char cate[7];
	char title[80];
	cate[6] = '\0';
	char buf[20];

	if (currentuser.flags[0] & BRDSORT_FLAG) {
		strcpy (buf, "[ÌÖÂÛÇøÁÐ±í] [×ÖÄ¸]");
	} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
		strcpy (buf, "[ÌÖÂÛÇøÁÐ±í] [ÔÚÏß]");
	} else {
		strcpy (buf, "[ÌÖÂÛÇøÁÐ±í] [·ÖÀà]");
	}

	//countbrdonline();
	//resolve_bcache();
	if (clsflag) {
		clear ();
		docmdtitle (buf,
				" [mÖ÷Ñ¡µ¥[[1;32m¡û[m,[1;32me[m] ÔÄ¶Á[[1;32m¡ú[m,[1;32mRtn[m] Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ý[m] ÁÐ³ö[[1;32my[m] ÅÅÐò[[1;32ms[m] ËÑÑ°[[1;32m/[m] ÇÐ»»[[1;32mc[m] ÇóÖú[[1;32mh[m]\n");
		/*
		 prints("[1;44;37m %s ÌÖÂÛÇøÃû³Æ       V  Àà±ð  ×ª %-25s S °æ  Ö÷   %s   [m\n",
		 newflag ? "È«²¿  Î´" : "±àºÅ  ", "ÖÐ  ÎÄ  Ðð  Êö", newflag ? "" : "   "); */
		//Modified by IAMFAT 2002-05-26
		//Modified by IAMFAT 2002-05-29
		//Modified by IAMFAT 2002-06-11
		/*
		 prints("[1;44;37m %s ÌÖÂÛÇøÃû³Æ       V  Àà±ð  ×ª %-25s S °æ  Ö÷   %s  [m\n",
		 newflag ? "È« ²¿  Î´" : "±à ºÅ  ", "ÖÐ  ÎÄ  Ðð  Êö", newflag ? "" : "   "); */
		prints
		("[1;44;37m %s ÌÖÂÛÇøÃû³Æ        V  Àà±ð  %-20s S °æ  Ö÷        ÔÚÏß [m\n",
				newflag ? "È« ²¿  Î´" : "±à ºÅ  Î´", "ÖÐ  ÎÄ  Ðð  Êö");
		//              prints("[1;44;37m %s ÌÖÂÛÇøÃû³Æ       V  Àà±ð  %-25s S °æ  Ö÷   %s     [m\n",
		//                      newflag ? "È« ²¿  Î´" : "±à ºÅ  ", "ÖÐ  ÎÄ  Ðð  Êö", newflag ? "" : "  ");
	}
	move (3, 0);
	for (n = page; n < page + BBS_PAGESIZE; n++) {
		if (n >= brdnum) {
			prints ("\n");
			continue;
		}
		ptr = &nbrd[n];
		if (ptr->total == -1) {
			refresh ();
			check_newpost (ptr);
		}
		//Modified by IAMFAT 2002-05-26
		if (!newflag)
		prints (" %5d", (n + 1));
		else if (ptr->flag & BOARD_DIR_FLAG)
		prints ("  Ä¿Â¼");
		else
		prints (" %5d", ptr->total);

		if (ptr->flag & BOARD_DIR_FLAG)
		prints ("  £«");
		else
		prints ("  %s", ptr->unread ? "¡ô" : "¡ó");
		if (!(ptr->flag & BOARD_CUSTOM_FLAG)) //added by cometcaptor 2007-04-23   
		strcpy (tmpBM, ptr->BM);
		strlcpy (cate, ptr->title + 1, 7);
		strcpy (title, ptr->title + 11);
		ellipsis (title, 20);
		prints ("%c%-17s %s%s%6s %-20s %c ",
				(ptr->zap && !(ptr->flag & BOARD_NOZAP_FLAG)) ? '*' : ' ',
				ptr->name,
				(ptr->flag & BOARD_VOTE_FLAG) ? "[1;31mV[m" : " ",
				(ptr->flag & BOARD_CLUB_FLAG) ? (ptr->flag & BOARD_READ_FLAG)
				? "[1;31mc[m" : "[1;33mc[m" : " ",
				cate, title, HAS_PERM (PERM_POST) ? ptr->status : ' ');
		if (ptr->flag & BOARD_DIR_FLAG)
		prints ("[Ä¿Â¼]\n");
		else
		prints ("%-12s %4d\n",
				ptr->BM[0] <= ' ' ? "³ÏÕ÷°æÖ÷ÖÐ" : strtok (tmpBM, " ")
#ifdef NEWONLINECOUNT
				, brdshm->bstatus[ptr->pos].inboard
#else
				, brdshm->bstatus[ptr->pos].online_num
#endif
		);
	}
	refresh ();
}

int cmpboard (brd, tmp)
struct newpostdata *brd, *tmp;
{
	register int type = 0;
	if (currentuser.flags[0] & BRDSORT_FLAG) {
		return strcasecmp (brd->name, tmp->name);
	} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
		return brdshm->bstatus[tmp->pos].inboard - brdshm->bstatus[brd->pos].inboard;
	}

	type = brd->title[0] - tmp->title[0];
	if (type == 0)
	type = strncasecmp (brd->title + 1, tmp->title + 1, 6);
	if (type == 0)
	type = strcasecmp (brd->name, tmp->name);
	return type;
}

int show_board_info(char *board) {
	int i;
	struct boardheader *bp;
	struct bstat *bs;
	char secu[40];
	bp = getbcache(board);
	bs = getbstat(board);
	clear();
	prints("°æÃæÏêÏ¸ÐÅÏ¢:\n\n");
	prints("number  :     %d\n", getbnum(bp->filename, &currentuser));
	prints("Ó¢ÎÄÃû³Æ:     %s\n", bp->filename);
	prints("ÖÐÎÄÃû³Æ:     %s\n", (HAS_PERM(PERM_SPECIAL0)) ? bp->title
			: (bp->title + 11));
	prints("°æ    Ö÷:     %s\n", bp->BM);
	prints("ËùÊôÌÖÂÛÇø:   %s\n", bp->group ? bcache[bp->group - 1].filename
			: "ÎÞ");
	prints("ÊÇ·ñÄ¿Â¼:     %s\n", (bp->flag & BOARD_DIR_FLAG) ? "Ä¿Â¼" : "°æÃæ");
	prints("¿ÉÒÔ ZAP:     %s\n", (bp->flag & BOARD_NOZAP_FLAG) ? "²»¿ÉÒÔ"
			: "¿ÉÒÔ");

	if (!(bp->flag & BOARD_DIR_FLAG)) {
		prints("ÔÚÏßÈËÊý:     %d ÈË\n", bs->inboard);
		prints("ÎÄ ÕÂ Êý:     %s\n", (bp->flag & BOARD_JUNK_FLAG) ? "²»¼ÆËã"
				: "¼ÆËã");
		prints("¿ÉÒÔ»Ø¸´:     %s\n", (bp->flag & BOARD_NOREPLY_FLAG) ? "²»¿ÉÒÔ"
				: "¿ÉÒÔ");
		//prints ("¿ÉÒÔ ZAP:     %s\n",
		//	    (bp->flag & BOARD_NOZAP_FLAG) ? "²»¿ÉÒÔ" : "¿ÉÒÔ");
		prints("Ää Ãû °æ:     %s\n", (bp->flag & BOARD_ANONY_FLAG) ? "ÊÇ"
				: "·ñ");
#ifdef ENABLE_PREFIX
		prints ("Ç¿ÖÆÇ°×º:     %s\n",
				(bp->flag & BOARD_PREFIX_FLAG) ? "±ØÐë" : "²»±Ø");
#endif
		prints("¾ã ÀÖ ²¿:     %s\n", (bp->flag & BOARD_CLUB_FLAG) ? (bp-> flag
				& BOARD_READ_FLAG) ? "¶ÁÏÞÖÆ¾ãÀÖ²¿" : "ÆÕÍ¨¾ãÀÖ²¿" : "·Ç¾ãÀÖ²¿");
		prints("now id  :     %d\n", bs->nowid);
		prints("¶ÁÐ´ÏÞÖÆ:     %s\n", (bp->flag & BOARD_POST_FLAG) ? "ÏÞÖÆ·¢ÎÄ"
				: (bp->level ==0) ? "Ã»ÓÐÏÞÖÆ" : "ÏÞÖÆÔÄ¶Á");
	}
	if (HAS_PERM(PERM_SPECIAL0) && bp->level != 0) {
		prints("È¨    ÏÞ:     ");
		strcpy(secu, "ltmprbBOCAMURS#@XLEast0123456789");
		for (i = 0; i < 32; i++) {
			if (!(bp->level & (1 << i)))
				secu[i] = '-';
			else {
				prints("%s\n              ", permstrings[i]);
			}

		}
		prints("\nÈ¨ ÏÞ Î»:     %s\n", secu);
	}

	prints("URL µØÖ·:     http://"BBSHOST"/bbs/doc?bid=%d\n",
			bp - bcache + 1);
	pressanykey();
	return FULLUPDATE;

}

int read_board(struct newpostdata *ptr, int newflag) {
	char buf[STRLEN];
	if (ptr->flag & BOARD_DIR_FLAG) {
		int tmpgrp, tmpmode;
		int oldpid; //added by cometcaptor 2007-04-21
		tmpgrp = boardparent;
		tmpmode = choosemode;
		choosemode = 0;
		boardparent = getbnum(ptr->name, &currentuser) - 1;
		oldpid = GoodBrd.nowpid; //cometcaptor ±£´æGoodBrd.nowpid
		if (ptr->flag & BOARD_CUSTOM_FLAG)
			GoodBrd.nowpid = ptr->pos;
		else
			GoodBrd.nowpid = -1;
		choose_board(newflag);
		GoodBrd.nowpid = oldpid;
		boardparent = tmpgrp;
		choosemode = tmpmode;
		brdnum = -1;
	} else {
		brc_initial(currentuser.userid, ptr->name);
		changeboard(&currbp, currboard, ptr->name);
		memcpy(currBM, ptr->BM, BM_LEN - 1);
		if (DEFINE(DEF_FIRSTNEW)) {
			setbdir(buf, currboard);
			int tmp = unread_position(buf, ptr);
			int page = tmp - t_lines / 2;
			getkeep(buf, page > 1 ? page : 1, tmp + 1);
		}
		Read();
		brc_zapbuf(zapbuf + ptr->pos);
		ptr->total = -1;
		currBM[0] = '\0'; //added by cometcaptor 2007-06-30
	}
}

int choose_board(int newflag) {
	static int num;
	struct newpostdata newpost_buffer[MAXBOARD];
	struct newpostdata *ptr;
	struct newpostdata *oldptr = nbrd; //added by cometcaptor 2006-10-20
	int page, ch, tmp, number, tmpnum;
	int loop_mode = 0;
	char ans[2];
	static char addname[STRLEN-8];
	if (!strcmp(currentuser.userid, "guest"))
		yank_flag = 1;
	nbrd = newpost_buffer;
	modify_user_mode(newflag ? READNEW : READBRD);
	brdnum = number = 0;
	clear();
	//show_brdlist(0, 1, newflag);
	while (1) {
		digestmode = NA;
		if (brdnum <= 0) {
			if (load_boards() == -1)
				continue;
			qsort(nbrd, brdnum, sizeof (nbrd[0]), cmpboard);
			page = -1;
			if (brdnum < 0)
				// prints ("brdnum <=0 ");
				break;
		}
		if (num < 0)
			num = 0;
		if (num >= brdnum)
			num = brdnum - 1;
		if (page < 0) {
			if (newflag) {
				tmp = num;
				while (num < brdnum) {
					ptr = &nbrd[num];
					if (!(ptr->flag & BOARD_DIR_FLAG)) {
						if (ptr->total == -1)
							check_newpost(ptr);
						if (ptr->unread)
							break;
					}
					num++;
				}
				if (num >= brdnum)
					num = tmp;
			}
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			show_brdlist(page, 1, newflag);
			update_endline();
		}
		if (num < page || num >= page + BBS_PAGESIZE) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			show_brdlist(page, 0, newflag);
			update_endline();
		}
		move(3 + num - page, 0);
		prints(">", number);
		if (loop_mode == 0) {
			ch = egetch();
		}
		move(3 + num - page, 0);
		prints(" ");
		if (ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF)
			break;
		switch (ch) {
			case '*':
				if (nbrd[num].flag & BOARD_CUSTOM_FLAG)
					break; //added by cometcaptor 2007-04-22
				ptr = &nbrd[num];
				show_board_info(ptr->name);
				page = -1;
				break;
			case 'b':
			case Ctrl('B'):
			case KEY_PGUP:
				if (num == 0)
					num = brdnum - 1;
				else
					num -= BBS_PAGESIZE;
				break;
			case 'C':
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.num==0)&&(GoodBrd.nowpid == -1))
					break;
				if (nbrd[num].flag & BOARD_CUSTOM_FLAG)
					break;
				strlcpy(addname, nbrd[num].name, STRLEN-8);
				presskeyfor("°æÃûÒÑ¸´ÖÆ Çë°´PÕ³Ìù", t_lines - 1);
				brdnum=-1;
				break;
			case 'c':
				if (newflag == 1)
					newflag = 0;
				else
					newflag = 1;
				show_brdlist(page, 1, newflag);
				break;
			case 'L':
				m_read();
				page = -1;
				break;
			case 'M':
				m_new();
				page = -1;
				break;
			case 'u':
				modify_user_mode(QUERY);
				t_query();
				page = -1;
				break;
			case 'H':
				getdata(t_lines - 1, 0, "ÄúÑ¡Ôñ? (1) ±¾ÈÕÊ®´ó  (2) ÏµÍ³ÈÈµã [1]",
						ans, 2, DOECHO, YEA);
				if (ans[0] == '2')
					show_help("etc/hotspot");
				else
					show_help("0Announce/bbslist/day");
				page = -1;
				break;
			case 'l':
				show_allmsgs();
				page = -1;
				break;
			case 'N':
			case ' ':
			case Ctrl('F'):
			case KEY_PGDN:
				if (num == brdnum - 1)
					num = 0;
				else
					num += BBS_PAGESIZE;
				break;
			case 'P':
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.num==0)&&(GoodBrd.nowpid == -1))
					break;
				add_GoodBrd(addname, GoodBrd.nowpid);
				addname[0]='\0';
				brdnum = -1;
				break;
			case 'p':
			case 'k':
			case KEY_UP:
				if (num-- <= 0)
					num = brdnum - 1;
				break;
			case 'n':
			case 'j':
			case KEY_DOWN:
				if (++num >= brdnum)
					num = 0;
				break;
			case '$':
				num = brdnum - 1;
				break;
			case '!': /* youzi leave */
				return Goodbye();
				break;
			case 'h':
				show_help("help/boardreadhelp");
				page = -1;
				break;
			case '/':
				move(3 + num - page, 0);
				prints(">", number);
				tmpnum = num;
				tmp = search_board(&num);
				move(3 + tmpnum - page, 0);
				prints(" ", number);
				if (tmp == 1)
					loop_mode = 1;
				else {
					loop_mode = 0;
					update_endline();
				}
				break;
			case 'i': //sort by online num
				currentuser.flags[0] ^= BRDSORT_ONLINE;
				qsort(nbrd, brdnum, sizeof (nbrd[0]), cmpboard);
				page = -1;
				substitut_record(PASSFILE, &currentuser,
						sizeof(currentuser), usernum);
				break;
			case 's': /* sort/unsort -mfchen */
				if (currentuser.flags[0] & BRDSORT_FLAG) {
					currentuser.flags[0] ^= BRDSORT_FLAG;
					currentuser.flags[0] |= BRDSORT_ONLINE;
				} else if (currentuser.flags[0] & BRDSORT_ONLINE) {
					currentuser.flags[0] ^= BRDSORT_ONLINE;
				} else {
					currentuser.flags[0] ^= BRDSORT_FLAG;
				}
				qsort(nbrd, brdnum, sizeof (nbrd[0]), cmpboard);
				substitut_record(PASSFILE, &currentuser,
						sizeof(currentuser), usernum);
				page = -1;
				break;
			case 'y':
				if (GoodBrd.num)
					break;
				yank_flag = !yank_flag;
				brdnum = -1;
				break;
			case 'z':
				if (GoodBrd.num)
					break;
				if (HAS_PERM(PERM_LOGIN) && !(nbrd[num].flag
						& BOARD_NOZAP_FLAG)) {
					ptr = &nbrd[num];
					ptr->zap = !ptr->zap;
					ptr->total = -1;
					zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
					page = 999;
				}
				break;
			case 'a':
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.num)&&(GoodBrd.nowpid == -1))
					break; //added by cometcaptor 2007-04-24 ·ÀÖ¹ÔÚ·Ç×Ô¶¨ÒåÄ¿Â¼¼Ó°æÃæ
				if (GoodBrd.num >= GOOD_BRC_NUM) {
					presskeyfor("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ", t_lines - 1);
					//ÊÕ²Ø¼Ð
				} else if (GoodBrd.num) {
					int pos;
					char bname[STRLEN];
					struct boardheader fh;
					if (gettheboardname(1, "ÊäÈëÌÖÂÛÇøÃû (°´¿Õ°×¼ü×Ô¶¯ËÑÑ°): ", &pos,
							&fh, bname, 1)) {
						if (!inGoodBrds(getbnum(bname, &currentuser)-1)) {
							//strcpy (GoodBrd.ID[GoodBrd.num++], bname);
							add_GoodBrd(bname, GoodBrd.nowpid); //modified by cometcaptor 2007-04-21 
							//save_GoodBrd ();
							//GoodBrd.num = 9999;
							brdnum = -1;
							break;
						}
					}
					page = -1;
					//ÆäËûÇé¿ö
				} else { //added by iamfat 2003.11.20 add goodbrd directly
					load_GoodBrd();
					//added by iamfat 2003.12.28 to avoid flow bug
					//struct boardheader *bp1 =NULL;
					//bp1 = getbcache(nbrd[num].name);
					//if (!((nbrd[num].flag & BOARD_DIR_FLAG)&& (bp1->group == 0))){ //¸ùÄ¿Â¼²»¿É¼ÓÈëÊÕ²Ø¼ÐDanielfree 06.3.5
					if (GoodBrd.num >= GOOD_BRC_NUM) {
						presskeyfor("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ", t_lines - 1);
					} else if (!inGoodBrds(getbnum(nbrd[num].name, &currentuser)-1)) {
						sprintf(genbuf, "ÄúÈ·¶¨ÒªÌí¼Ó%sµ½ÊÕ²Ø¼ÐÂð?", nbrd[num].name);
						if (askyn(genbuf, NA, YEA) == YEA) {
							//strcpy (GoodBrd.ID[GoodBrd.num++], nbrd[num].name);
							add_GoodBrd(nbrd[num].name, 0); //modified by cometcaptor 2007-04-21
							//save_GoodBrd ();
						}
						brdnum = -1;
					}
					//}
					GoodBrd.num = 0;
				}
				break;
			case 'A':
				//added by cometcaptor 2007-04-22 ÕâÀïÐ´ÈëµÄÊÇ´´½¨×Ô¶¨ÒåÄ¿Â¼µÄ´úÂë
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if (GoodBrd.nowpid == 0) {
					if (GoodBrd.num >= GOOD_BRC_NUM)
						presskeyfor("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ", t_lines - 1);
					else {
						//ÒªÇóÊäÈëÄ¿Â¼Ãû
						char dirname[STRLEN];
						char dirtitle[STRLEN];
						//.....ÕâÀï
						dirname[0] = '\0'; //Çå³ýÉÏÒ»´Î´´½¨µÄÄ¿Â¼Ãû
						getdata(t_lines - 1, 0, "´´½¨×Ô¶¨ÒåÄ¿Â¼: ", dirname, 17,
								DOECHO, NA); //Éè³É17ÊÇÒòÎªÖ»ÏÔÊ¾16¸ö×Ö·û£¬ÎªÁË·ÀÖ¹´´½¨µÄÄ¿Â¼ÃûºÍÏÔÊ¾µÄ²»Í¬¶øÏÞ¶¨
						if (dirname[0] != '\0') {
							strcpy(dirtitle, "×Ô¶¨ÒåÄ¿Â¼");
							getdata(t_lines - 1, 0, "×Ô¶¨ÒåÄ¿Â¼ÃèÊö: ", dirtitle,
									21, DOECHO, NA);
							mkdir_GoodBrd(dirname, dirtitle, 0);
						}
						brdnum = -1;
					}
				}
				break;
			case 'T':
				//added by cometcaptor 2007-04-25 ÐÞ¸Ä×Ô¶¨ÒåÄ¿Â¼Ãû
				if (!HAS_PERM(PERM_LOGIN))
					break;
				if ((GoodBrd.nowpid == 0)&&(brdnum)&&(nbrd[num].flag
						& BOARD_CUSTOM_FLAG)) {
					char dirname[STRLEN];
					char dirtitle[STRLEN];
					int gbid = 0;
					for (gbid = 0; gbid < GoodBrd.num; gbid++)
						if (GoodBrd.boards[gbid].id == nbrd[num].pos)
							break;
					if (gbid == GoodBrd.num)
						break;
					strcpy(dirname, GoodBrd.boards[gbid].filename);
					getdata(t_lines - 1, 0, "ÐÞ¸Ä×Ô¶¨ÒåÄ¿Â¼Ãû: ", dirname, 17,
							DOECHO, NA);
					if (dirname[0] != '\0') {
						strcpy(dirtitle, GoodBrd.boards[gbid].title+11);
						getdata(t_lines - 1, 0, "×Ô¶¨ÒåÄ¿Â¼ÃèÊö: ", dirtitle, 21,
								DOECHO, NA);
						if (dirtitle[0] == '\0')
							strcpy(dirtitle, GoodBrd.boards[gbid].title+11);
						strcpy(GoodBrd.boards[gbid].filename, dirname);
						strcpy(GoodBrd.boards[gbid].title+11, dirtitle);
						save_GoodBrd();
					}
					brdnum = -1;
				}
				break;
			case 'd':
				if ((GoodBrd.num)&&(brdnum > 0)) { //modified by cometcaptor 2007-04-24
					int i, pos;
					char ans[5];
					sprintf(genbuf, "Òª°Ñ %s ´ÓÊÕ²Ø¼ÐÖÐÈ¥µô£¿[y/N]", nbrd[num].name);
					getdata(t_lines - 1, 0, genbuf, ans, 2, DOECHO, YEA);
					//if (ans[0] == 'n' || ans[0] == 'N') {
					//  page = -1;
					//  break;
					//}
					if (ans[0] == 'y' || ans[0] == 'Y') {
						/*pos = inGoodBrds (nbrd[num].name);
						 for (i = pos - 1; i < GoodBrd.num - 1; i++)
						 strcpy (GoodBrd.ID[i], GoodBrd.ID[i + 1]);
						 GoodBrd.num--;
						 */
						//modified by cometcaptor 2007-04-21
						if (nbrd[num].flag & BOARD_CUSTOM_FLAG)
							rmdir_GoodBrd(nbrd[num].pos);
						else {
							pos = inGoodBrds(nbrd[num].pos);
							if (pos) {
								for (i = pos-1; i < GoodBrd.num-1; i++)
									memcpy(&GoodBrd.boards[i],
											&GoodBrd.boards[i+1],
											sizeof(struct goodbrdheader));
								GoodBrd.num--;
							}
							save_GoodBrd();
						}
						//GoodBrd.num = 9999;
						brdnum = -1;
					} else {
						page = -1;
						break;
					}
				}
				break;
			case KEY_HOME:
				num = 0;
				break;
			case KEY_END:
				num = brdnum - 1;
				break;
			case '\n':
			case '\r':
				if (number > 0) {
					num = number - 1;
					break;
				}
				/* fall through */
			case KEY_RIGHT:
				tmp = num;
				num = 0;
				if (brdnum > 0)
					read_board(&nbrd[tmp], newflag); //modified by cometcaptor 2007-04-23
				//read_board (&nbrd[tmp], newflag);
				num = tmp;
				page = -1;
				break;
			case 'S': /* sendmsg ... youzi */
				if (!HAS_PERM(PERM_TALK))
					break;
				s_msg();
				page = -1;
				break;
			case 'o': /* show friends ... youzi */
				if (!HAS_PERM(PERM_LOGIN))
					break;
				t_friends();
				page = -1;
				break;

			default:
				;
		}
		modify_user_mode(newflag ? READNEW : READBRD);
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
	}
	clear();
	save_zapbuf();
	nbrd = oldptr; //added by cometcaptor 2006-10-20
	return -1;
}
