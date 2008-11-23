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
$Id: screen.c 2 2005-07-14 15:06:08Z root $
*/

#include "bbs.h"
#include "screen.h"
#include "edit.h"
#include <sys/param.h>
#include <stdarg.h>

extern char clearbuf[];
extern char cleolbuf[];
extern char scrollrev[];
extern char strtstandout[];
extern char endstandout[];
extern int iscolor;
extern int clearbuflen;
extern int cleolbuflen;
extern int scrollrevlen;
extern int strtstandoutlen;
extern int endstandoutlen;
extern int editansi;

extern int automargins;
extern int dumb_term;
#define o_clear()     output(clearbuf,clearbuflen)
#define o_cleol()     output(cleolbuf,cleolbuflen)
#define o_scrollrev() output(scrollrev,scrollrevlen)
#define o_standup()   output(strtstandout,strtstandoutlen)
#define o_standdown() output(endstandout,endstandoutlen)

unsigned char scr_lns, scr_cols;
unsigned char cur_ln = 0, cur_col = 0;
int     roll, scrollcnt;//roll ±íÊ¾Ê×ĞĞÔÚbig_pictureµÄÆ«ÒÆÁ¿
						//ÒòÎªËæ×Å¹â±ê¹ö¶¯,big_picture[0]¿ÉÄÜ²»ÔÙ±£´æµÚÒ»ĞĞµÄÊı¾İ
unsigned char docls;
unsigned char downfrom;
int     standing = NA;
int     inansi = NA;

struct screenline *big_picture = NULL;

//add by wujian
//	Çå³ı·ÇÑÆÖÕ¶ËÖĞ´Óµ±Ç°ĞĞ¿ªÊ¼µÄnĞĞ
//	Èô³¬¹ıÆÁÄ»,Ôò´ÓµÚÒ»ĞĞ½Ó×ÅÇå³ı(½«µÚÒ»ĞĞÖĞµÄÊı¾İ¾ùÖÃÎª0)
void clrnlines(int n)
{
    register struct screenline *slp ;
    register int        i, k;
    if(dumb_term)
      return ;
    for(i=cur_ln; i<cur_ln+n;i++) {
        slp = &big_picture[(i + roll) % scr_lns];
        slp->mode = 0;
        slp->oldlen = 255;
        slp->len = 0;
        for(k=0;k<LINELEN;k++)
           slp->data[k]=0;
    }
}

#ifdef ALLOWAUTOWRAP
//·µ»ØstrÖĞÇ°num¸ö×Ö·ûÖĞÒÔansi¸ñÊ½Êµ¼ÊÏÔÊ¾µÄ×Ö·ûÊı?
int seekthestr(char *str, int num)
{
	int	len, i, ansi= NA;
	len = strlen(str);
	for(i=0;i<len;i++) {
    	if(!(num--)) 
			break;
		if(str[i] == KEY_ESC){
			ansi = YEA;
			continue;
		}
		if( ansi ) {
			if ( !strchr("[0123456789; ", str[i]))  
				ansi = NA;
			continue;
/*                      if (strchr("[0123456789; ", str[i]))
                                continue;
                        else if (isalpha(str[i])) {
                                ansi = NA;
                                continue;
                        }
                        else
                                break;
*/
		}	//if
//		if(!(num--)) break;
	}	//for
    return i;
}			
#endif	

//·µ»Ø×Ö·û´®ÖĞÊôÓÚ ansiµÄ¸öÊı?	¶ÔºóÒ»¸öcontinue²»Ì«Àí½â 
int num_ans_chr(char *str)
{
	int len,i,ansinum,ansi;
	
	ansinum=0;
    ansi=NA;
    len=strlen(str);
    for (i=0; i < len; i++){
		if (str[i] == KEY_ESC){
	        ansi = YEA;
            ansinum++;
            continue;
        }
        if (ansi){
        	if (!strchr("[0123456789; ", str[i])) 
				ansi = NA;
            ansinum++;
            continue;
/*
                        if (strchr("[0123456789; ", str[i]))
                        {
                                ansinum++;
                                continue;
                        }
                        else if (isalpha(str[i]))
                        {
                                ansinum++;
                                ansi = NA;
                                continue;
                        }
                        else
                                break;
*/
    	}
	}
    return ansinum;
}

//	³õÊ¼»¯ÆÁÄ»,½«ĞĞÊıÉè³É slns ,½«ÁĞÊıÉèÖÃ³ÉLINELENÓëscolsµÄ×îĞ¡Öµ,
//	ÆäÖĞLINELEN±íÊ¾ÏµÍ³ÉèÖÃµÄ×î´óÁĞÊı,Îª256
//		½«·ÖÅäµ½µÄÆÁÄ»»º³åÓ³Éäµ½big_picture, ¹©µ÷ÓÃ
void init_screen(int     slns, int scols)
{
	register struct screenline *slp;
	scr_lns = slns;
	scr_cols = Min(scols, LINELEN);
	big_picture = (struct screenline *) calloc(scr_lns,
		sizeof(struct screenline));
	for (slns = 0; slns < scr_lns; slns++) {
		slp = &big_picture[slns];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	docls = YEA;
	downfrom = 0;
	roll = 0;
}

//¶ÔÓÚÑÆÖÕ¶Ë»òÊÇbig_pictureÖĞÉĞÎŞÄÚ´æÓ³Éä,½«t_columnsÉèÖÃ³ÉWRAPMARGIN
//	µ÷ÓÃinit_screen³õÊ¼»¯ÖÕ¶Ë
void initscr()
{
	if (!dumb_term && !big_picture)
		t_columns = WRAPMARGIN;
	init_screen(t_lines, WRAPMARGIN);
}

int     tc_col, tc_line;	//terminal's current collumn,current line?

//	´ÓÀÏÎ»ÖÃ(was_col,was_ln)ÒÆ¶¯µ½ĞÂÎ»ÖÃ(new_col,new_ln)
void rel_move(int was_col,int was_ln,int new_col,int new_ln)
{
	int     ochar();
	extern char *BC;
	if (new_ln >= t_lines || new_col >= t_columns)	//Ô½½ç,·µ»Ø
		return;
	tc_col = new_col;
	tc_line = new_ln;
	if ((new_col == 0) && (new_ln == was_ln + 1)) {	//»»ĞĞ
		ochar('\n');
		if (was_col != 0)	//µ½µÚÒ»ÁĞÎ»ÖÃ,·µ»Ø
			ochar('\r');
		return;
	}
	if ((new_col == 0) && (new_ln == was_ln)) {	//²»»»ĞĞ,µ½µÚÒ»ÁĞÎ»ÖÃ,²¢·µ»Ø
		if (was_col != 0)
			ochar('\r');
		return;
	}
	if (was_col == new_col && was_ln == new_ln)
		return;
	if (new_col == was_col - 1 && new_ln == was_ln) {	//µ½Ç°Ò»ĞĞ
		if (BC)
			tputs(BC, 1, ochar);
		else
			ochar(Ctrl('H'));
		return;
	}
	do_move(new_col, new_ln, ochar);	//ËùÓĞÇé¿ö¶¼²»Âú×ãÊ±,Ö´ĞĞ´Ëº¯Êı
}

// ±ê×¼Êä³öbufÖĞµÄÊı¾İ,	ds,de±íÊ¾Êı¾İµÄÇø¼ä,sso,esoÒ²ÊÇ
//		µ«µ±ËüÃÇÃ»ÓĞ½»¼¯Ê±,ÒÔds,deÎª×¼
//		ÓĞ½»¼¯Ê±,È¡ºÏ¼¯
//			µ«ÏÂÏŞÒÔdsÎª×¼,ÉÏÏŞÒÔdeÎª×¼				¸úÖ±½ÓÈ¡ds,deÓĞÊ²Ã´Çø±ğ?
///		¶Ôo_standup,o_standdown×÷ÓÃ²»Ì«Çå³ş
void	standoutput(char * buf, int ds, int de,int sso,int eso)
{
	int     st_start, st_end;
	if (eso <= ds || sso >= de) {
		output(buf + ds, de - ds);
		return;
	}
	st_start = Max(sso, ds);
	st_end = Min(eso, de);
	if (sso > ds)
		output(buf + ds, sso - ds);
	o_standup();
	output(buf + st_start, st_end - st_start);
	o_standdown();
	if (de > eso)
		output(buf + eso, de - eso);
}

//	Ë¢ĞÂÆÁÄ»
void redoscr()
{
	register int i, j;
	int     ochar();
	register struct screenline *bp = big_picture;
	if (dumb_term)
		return;
	o_clear();		//Çå³ı»º³å
	tc_col = 0;
	tc_line = 0;
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].len == 0)
			continue;
		rel_move(tc_col, tc_line, 0, i);
		if (bp[j].mode & STANDOUT)
			standoutput(bp[j].data, 0, bp[j].len, bp[j].sso, bp[j].eso);
		else
			output(bp[j].data, bp[j].len);
		tc_col += bp[j].len;
		if (tc_col >= t_columns) {
			if (!automargins) {
				tc_col -= t_columns;
				tc_line++;
				if (tc_line >= t_lines)
					tc_line = t_lines - 1;
			} else
				tc_col = t_columns - 1;
		}
		bp[j].mode &= ~(MODIFIED);
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	docls = NA;
	scrollcnt = 0;
	oflush();
}

//Ë¢ĞÂ»º³åÇø,ÖØĞÂÏÔÊ¾ÆÁÄ»?
void refresh()
{
	register int i, j;
	register struct screenline *bp = big_picture;
	extern int automargins;
	extern int scrollrevlen;
#ifndef BBSD
	if (dumb_term)
		return;
#endif		
	if (num_in_buf() != 0)
		return;
	if ((docls) || (abs(scrollcnt) >= (scr_lns - 3))) {
		redoscr();
		return;
	}
	if (scrollcnt < 0) {
		if (!scrollrevlen) {
			redoscr();
			return;
		}
		rel_move(tc_col, tc_line, 0, 0);
		while (scrollcnt < 0) {
			o_scrollrev();
			scrollcnt++;
		}
	}
	if (scrollcnt > 0) {
		rel_move(tc_col, tc_line, 0, t_lines - 1);
		while (scrollcnt > 0) {
			ochar('\n');
			scrollcnt--;
		}
	}
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].mode & MODIFIED && bp[j].smod < bp[j].len) {
			bp[j].mode &= ~(MODIFIED);	//Èô±»ĞŞ¸Ä,ÔòÊä³ö
			if (bp[j].emod >= bp[j].len)
				bp[j].emod = bp[j].len - 1;
			rel_move(tc_col, tc_line, bp[j].smod, i);
			if (bp[j].mode & STANDOUT)
				standoutput(bp[j].data, bp[j].smod, bp[j].emod + 1,
					bp[j].sso, bp[j].eso);
			else
				output(&bp[j].data[bp[j].smod], bp[j].emod - bp[j].smod + 1);
			tc_col = bp[j].emod + 1;
			if (tc_col >= t_columns) {
				if (automargins) {
					tc_col -= t_columns;
					tc_line++;
					if (tc_line >= t_lines)
						tc_line = t_lines - 1;
				} else
					tc_col = t_columns - 1;
			}
		}
		if (bp[j].oldlen > bp[j].len) {
			rel_move(tc_col, tc_line, bp[j].len, i);
			o_cleol();
		}
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	oflush();
}

/*ÒÆ¶¯µ½µÚyĞĞ,µÚxÁĞ*/
void move(int y, int x)
{
	cur_col = x /* +c_shift(y,x) */ ;
	cur_ln = y;
}

//	·µ»Øµ±Ç°µÄĞĞÊıµ½y,ÁĞÊıµ½x
void getyx(int *y,int *x)
{
	*y = cur_ln;
	*x = cur_col /*-c_shift(y,x)*/ ;
}

//	ÇåÁã	big_pictureÖĞµÄÊı¾İ,roll,docls,downfrom
//	ÒÆ¶¯µ½Î»ÖÃ(0,0)
void clear()
{
	register int i;
	register struct screenline *slp;
	if (dumb_term)/*ÑÆÖÕ¶Ë*/
		return;
	roll = 0;
	docls = YEA;
	downfrom = 0;
	for (i = 0; i < scr_lns; i++) {
		slp = &big_picture[i];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	move(0, 0);
}

//Çå³ıbig_pictureÖĞµÄµÚiĞĞ,½«modeÓëlenÖÃ0
void clear_whole_line( int	i)
{
	register struct screenline *slp = &big_picture[i];
	slp->mode = slp->len = 0;
	slp->oldlen = 79;
}

//	½«´Óµ±Ç°¹â±êµ½ĞĞÄ©µÄËùÓĞ×Ö·û±ä³É¿Õ¸ñ,´ïµ½Çå³ıµÄĞ§¹û
void clrtoeol()
{
	register struct screenline *slp;
	register int ln;

	if (dumb_term)
		return;
	standing = NA;
	ln = cur_ln + roll;
	while (ln >= scr_lns)	//Ïàµ±ÓÚln%=scr_lns,È¡µ±Ç°ĞĞÔÚbig_pictureÖĞµÄĞòºÅ
		ln -= scr_lns;
	slp = &big_picture[ln];
	if (cur_col <= slp->sso)
		slp->mode &= ~STANDOUT;		//½«slp->modeµÚ0Î»ÖÃ0
	if (cur_col > slp->oldlen) {
		register int i;
		for (i = slp->len; i <= cur_col; i++)
			slp->data[i] = ' ';
	}
	slp->len = cur_col;
}

//´Óµ±Ç°ĞĞÇå³ıµ½×îºóÒ»ĞĞ
void clrtobot()
{
	register struct screenline *slp;
	register int i, j;
	if (dumb_term)
		return;
	for (i = cur_ln; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)	//Çój%scr_lns ? ÒòÎª¼õ·¨±ÈÈ¡ÓàÊ±¼äÉÙ?
			j -= scr_lns;
		slp = &big_picture[j];
		slp->mode = 0;
		slp->len = 0;
		if (slp->oldlen > 0)
			slp->oldlen = 255;
	}
}

//	½«big_pictureµÄSTANDOUTÎ»ÖÃ0
void clrstandout()
{
	register int i;
	if (dumb_term)
		return;
	for (i = 0; i < scr_lns; i++)
		big_picture[i].mode &= ~(STANDOUT);
}

static char nullstr[] = "(null)";

//ÒÔANSI¸ñÊ½Êä³ö×Ö·ûc,Í¨³£ÊÇ×÷ÓÃÔÚÒ»¸ö×Ö·û´®ÉÏ,ÒÔANSI¸ñÊ½Êä³öÒ»¸öÒ»¸ö×Ö·û
void outc(register unsigned char c)
{
	register struct screenline *slp;
	register unsigned char reg_col;
#ifndef BIT8
	c &= 0x7f;	/*Èô¶¨ÒåÁËË«×Ö½Ú,È¥µô×î¸ßÎ»*/
#endif
	if (inansi == 1) {	//inansi±íÊ¾ÊÇ·ñÔÚansi×´Ì¬ÄÚ
		if (c == 'm') {
			inansi = 0;
			return;
		}
		return;
	}
	if (c == KEY_ESC && iscolor == NA) {//½øÈëansi×´Ì¬
		inansi = 1;
		return;
	}
	if (dumb_term) {
		if (!isprint2(c)) {
			if (c == '\n') {	//»»ĞĞ
				ochar('\r');
			} else if (c != KEY_ESC || !showansi) {//²»¿É´òÓ¡×Ö·ûÏÔÊ¾Îª'*'
				c = '*';
			}
		}
		ochar(c);
		return;
	}
	if (1) {
		register int reg_line = cur_ln + roll;
		register int reg_scrln = scr_lns;
		while (reg_line > 0 && reg_line >= reg_scrln)
			reg_line -= reg_scrln;
		slp = &big_picture[reg_line];//»ñµÃµ±Ç°ĞĞµÄÓ³Éä
	}
	reg_col = cur_col;
	/* deal with non-printables */
	if (!isprint2(c)) {
		if (c == '\n' || c == '\r') {	/* do the newline thing */
			if (standing) {
				slp->eso = Max(slp->eso, reg_col);
				standing = NA;
			}
			if (reg_col > slp->len) {//ÒÔ¿Õ¸ñÀ©³äÁĞ
				register int i;
				for (i = slp->len; i <= reg_col; i++)
					slp->data[i] = ' ';
			}
			slp->len = reg_col;
			cur_col = 0;	/* reset cur_col */
			if (cur_ln < scr_lns)
				cur_ln++;
			return;
		} else if (c != KEY_ESC || !showansi) {
			c = '*';/* else substitute a '*' for non-printable */
		}
	}
	if (reg_col >= slp->len) {	//	>= »¹ÊÇ > ?
		register int i;
		for (i = slp->len; i < reg_col; i++)
			slp->data[i] = ' ';
		slp->data[reg_col] = '\0';
		slp->len = reg_col + 1;
	}
	if (slp->data[reg_col] != c) {
		if ((slp->mode & MODIFIED) != MODIFIED)
			slp->smod = (slp->emod = reg_col);
		else {
			if (reg_col > slp->emod)
				slp->emod = reg_col;
			if (reg_col < slp->smod)
				slp->smod = reg_col;
		}
		slp->mode |= MODIFIED;
	}
	slp->data[reg_col] = c;	//ÔÚµ±Ç°ĞĞreg_colÁĞ´æ´¢×Ö·ûc
	reg_col++;
	if (reg_col >= scr_cols) {	//³¬¹ıÆÁÄ»×î´ó¿í¶È
		if (standing && slp->mode & STANDOUT) {
			standing = NA;
			slp->eso = Max(slp->eso, reg_col);
		}
		reg_col = 0;
		if (cur_ln < scr_lns)
			cur_ln++;
	}
	cur_col = reg_col;	/* store cur_col back */
}

//	ÀûÓÃoutcÊä³ö×Ö·û´®str
void outs(register char *str)
{
	while (*str != '\0') {
		outc(*str++);
	}
}

//	cc±íÊ¾ÊÇ·ñAnsi·½Ê½Êä³ö?
//	n±íÊ¾Êä³öµÄ×Ö·û´®³¤¶È,strÊÇÏàÓ¦µÄ×Ö·û´®
//	
void outns(register char * str,register int  n, register int cc)
{
	if (!cc) {
		for (; n > 0; n--) {
			outc(*str++);
		}
	} else {
		/*
		 * need to do find out how many color control char used. and
		 * then add to 'n'.
		 * 
		 * n = n + count_of_color_controler
		 */
		int     lock = 0, i = 0, j, k;
		char   *foo;
		foo = (char *) malloc(strlen(str) + 100);
		strcpy(foo, str);

		for (j = 0, k = n; k > 0; k--, j++) {	//kËÆºõÊÇ¶àÓàµÄ,ÓÃj¾Í¿ÉÒÔ?
			if (foo[j] == '' && lock == 0) {	//lockÎªÕæ,±íÊ¾½øÈëansiµÄ¿ØÖÆ±êÖ¾
				lock = 1;
				i++;
				continue;
			} else if (isalpha(foo[j]) && lock > 0) {
				i++;
				lock = 0;
				continue;
			} else if (lock > 0) {
				i++;
			}
		}

		n += i;				//iÎªÇó³öµÄ¿ØÖÆ±êÖ¾×Ö·û¸öÊı
		for (; n > 0; n--)
			outc(*str++);
		outs("[m");

		free(foo);	//avoid memory overflow, iamfat 2004.01.12
	}
}


int     dec[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

/*ÒÔANSI¸ñÊ½Êä³ö¿É±ä²ÎÊıµÄ×Ö·û´®ĞòÁĞ*/
void prints(char *fmt, ...)
{
	va_list ap;
	char   *bp;
	register int i, count, hd, indx;
	va_start(ap, fmt);
	while (*fmt != '\0') {
		if (*fmt == '%') {
			int     sgn = 1;
			int     sgn2 = 1;
			int     val = 0;
			int     len, negi;
			fmt++;
			switch (*fmt) {
			case '-':
				while (*fmt == '-') {
					sgn *= -1;
					fmt++;
				}
				break;
			case '.':
				sgn2 = 0;
				fmt++;
				break;
			}
			while (isdigit(*fmt)) {
				val *= 10;
				val += *fmt - '0';
				fmt++;
			}
			switch (*fmt) {
			case 's':
				bp = va_arg(ap, char *);
				if (bp == NULL)
					bp = nullstr;
				if (val) {
					register int slen = strlen(bp);
					if (!sgn2) {
						if (val <= slen)
							outns(bp, val, 1);
						else
							outns(bp, slen, 1);
					} else if (val <= slen)
						outns(bp, val, 0);
					else if (sgn > 0) {
						for (slen = val - slen; slen > 0; slen--)
							outc(' ');
						outs(bp);
					} else {
						outs(bp);
						for (slen = val - slen; slen > 0; slen--)
							outc(' ');
					}
				} else
					outs(bp);
				break;
			case 'd':
				i = va_arg(ap, int);

				negi = NA;
				if (i < 0) {
					negi = YEA;
					i *= -1;
				}
				for (indx = 0; indx < 10; indx++)
					if (i >= dec[indx])
						break;
				if (i == 0)
					len = 1;
				else
					len = 10 - indx;
				if (negi)
					len++;
				if (val >= len && sgn > 0) {
					register int slen;
					for (slen = val - len; slen > 0; slen--)
						outc(' ');
				}
				if (negi)
					outc('-');
				hd = 1, indx = 0;
				while (indx < 10) {
					count = 0;
					while (i >= dec[indx]) {
						count++;
						i -= dec[indx];
					}
					indx++;
					if (indx == 10)
						hd = 0;
					if (hd && !count)
						continue;
					hd = 0;
					outc('0' + count);
				}
				if (val >= len && sgn < 0) {
					register int slen;
					for (slen = val - len; slen > 0; slen--)
						outc(' ');
				}
				break;
			case 'c':
				i = va_arg(ap, int);
				outc(i);
				break;
			case '\0':
				goto endprint;
			default:
				outc(*fmt);
				break;
			}
			fmt++;
			continue;
		}
		outc(*fmt);
		fmt++;
	}
	va_end(ap);
endprint:
	return;
}
//	Êä³öÒ»¸ö×Ö·û
void addch(int ch)
{
	outc(ch);
}
// ¾í¶¯Ò»ĞĞ
void scroll()
{
	if (dumb_term) {
		prints("\n");
		return;
	}
	scrollcnt++;
	roll++;
	if (roll >= scr_lns)
		roll -= scr_lns;
	move(scr_lns - 1, 0);
	clrtoeol();
}
//ÏòÉÏ¾í¶¯Ò»ĞĞ
void rscroll()
{
	if (dumb_term) {
		prints("\n\n");
		return;
	}
	scrollcnt--;
	if (roll > 0)
		roll--;
	else
		roll = scr_lns - 1;
	move(0, 0);
	clrtoeol();
}

//½«big_pictureÊä³öÎ»ÖÃ1,±ê×¼Êä³öÇø¼äÎª(cur_col,cur_col)
void standout()
{
	register struct screenline *slp;
	register int ln;
	if (dumb_term || !strtstandoutlen)
		return;
	if (!standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = YEA;
		slp->sso = cur_col;
		slp->eso = cur_col;
		slp->mode |= STANDOUT;
	}
}
//	Èç¹ûstandingÎªÕæ,½«µ±Ç°ĞĞÔÚbig_pictureÖĞµÄÓ³ÉäÉè³ÉÕæ
//		²¢½«esoÉè³Éeso,cur_colµÄ×î´óÖµ
void standend()
{
	register struct screenline *slp;
	register int ln;
	if (dumb_term || !strtstandoutlen)
		return;
	if (standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = NA;
		slp->eso = Max(slp->eso, cur_col);
	}
}
//	¸ù¾İmodeÀ´¾ö¶¨ ±£´æ»ò»Ö¸´ĞĞlineµÄÄÚÈİ
//		×î¶àÖ»ÄÜ±£´æÒ»ĞĞ,·ñÔò»á±»Ä¨È¥
void saveline(int line, int mode)  /* 0,2 : save, 1,3 : restore */
{
    register struct screenline *bp = big_picture ;
    static char tmp[2][256];
    int x,y;

    switch (mode) {
        case 0: 
		case 2: 
        	strncpy(tmp[mode/2], bp[line].data, LINELEN);
        	tmp[mode/2][bp[line].len]='\0';
            break;
        case 1: 
		case 3:
            getyx(&x,&y);
            move(line,0);
	        clrtoeol();
	        refresh();
	        prints("%s",tmp[(mode-1)/2]);
            move(x,y);
	        refresh();
    }
}

