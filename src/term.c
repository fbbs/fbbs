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
 $Id: term.c 2 2005-07-14 15:06:08Z root $
 */
#include "bbs.h"
#include <sys/ioctl.h>

#ifdef HP_UX
#define O_HUPCL 01
#define O_XTABS 02
#endif

#ifdef TERMIOS
#include <termios.h>
#define stty(fd, data) tcsetattr( fd, TCSANOW, data )
//设置fd所关联的终端属性为termios结构的指针data,且立刻生效(TCSANOW)
#define gtty(fd, data) tcgetattr( fd, data )
//取得fd所关联的终端的属性,并存放在data所指向的结构中
struct termios tty_state, tty_new;
#else
struct sgttyb tty_state, tty_new;
#endif

#ifndef TANDEM
#define TANDEM	0x00000001
#endif

#ifndef CBREAK
#define CBREAK  0x00000002
#endif

#ifdef TERMIOS
//	初始化tty设置
void init_tty()
{
	long vdisable;
	memcpy(&tty_new, &tty_state, sizeof(tty_new));
	tty_new.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ISIG);
	tty_new.c_cflag &= ~CSIZE;
	tty_new.c_cflag |= CS8;
	tty_new.c_cc[VMIN] = 1;
	tty_new.c_cc[VTIME] = 0;
	if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0) {
		//fpathconf(fd,name) 取得相对于文件描述符fd的设置选项name的值
		//_PC_VDISABLE returns nonzero if special character processing can be disabled
		tty_new.c_cc[VSTART] = vdisable;
		tty_new.c_cc[VSTOP] = vdisable;
		tty_new.c_cc[VLNEXT] = vdisable;
	}
	tcsetattr(1, TCSANOW, &tty_new);
	//相当于stty(1,&tty_new);
}
#else
//初始化终端,设定终端的属性
void init_tty() {
	memcpy(&tty_new, &tty_state, sizeof(tty_new));
	tty_new.sg_flags |= RAW;

#ifdef HP_UX
	tty_new.sg_flags &= ~(O_HUPCL | O_XTABS | LCASE | ECHO | CRMOD);
#else
	tty_new.sg_flags &= ~(TANDEM | CBREAK | LCASE | ECHO | CRMOD);
#endif

	stty(1, &tty_new);
}
#endif

#define TERMCOMSIZE (1024)

int dumb_term = YEA; //哑终端设置成真

char clearbuf[TERMCOMSIZE];
int clearbuflen;

char cleolbuf[TERMCOMSIZE];
int cleolbuflen;

char cursorm[TERMCOMSIZE];
char *cm;

char scrollrev[TERMCOMSIZE];
int scrollrevlen;

char strtstandout[TERMCOMSIZE];
int strtstandoutlen;

char endstandout[TERMCOMSIZE];
int endstandoutlen;

int t_lines = 24; //终端的行数
int t_columns = 255; //终端的列数

int automargins; //如果到达边界,是否自动转到下一行

char *outp;
int *outlp; //输出的指针

//将ch放入outlp所指向的字符数组中
static outcf(char ch) {
	if (*outlp < TERMCOMSIZE) {
		(*outlp)++;
		*outp++ = ch;
	}
}

//终端初始化工作,成功获得term的入口时,进行善后处理并返回YEA,失败返回NA
int term_init(char *term) {
	extern char PC, *UP, *BC;
	extern short ospeed;
	static char UPbuf[TERMCOMSIZE];
	static char BCbuf[TERMCOMSIZE];
	static char buf[5120];
	char sbuf[5120];
	char *sbp, *s;
	char *tgetstr();
#ifdef TERMIOS
	ospeed = cfgetospeed(&tty_state); //返回tty_state属性的终端传输速率
#else
	ospeed = tty_state.sg_ospeed;
#endif

	if (tgetent(buf, term) != 1) //获得term的入口,成功返回1,失败则0,若无数据库
		//	返回-1;	buf似乎不起作用
		return NA;

	sbp = sbuf;
	s = tgetstr("pc", &sbp);/* get pad character */
	//返回"pc"的字符串入口,若返回0表示不可用
	//字符串结果也保存在sbp中,以null结束
	if (s)
		PC = *s; //PC is set by tgetent to the terminfo entry's data for pad_char
	t_lines = tgetnum("li"); //tgetnum返回"li"的数值入口,返回值为-1表示不可用
	t_columns = tgetnum("co");
	automargins = tgetflag("am");//tgetflag返回"am"的布尔值入口,0表示不可用
	outp = clearbuf; /* fill clearbuf with clear screen command */
	outlp = &clearbuflen;
	clearbuflen = 0;
	sbp = sbuf;
	s = tgetstr("cl", &sbp); //returns the string entry for "cl",Use tputs() to output
	//the rerurned string.The return value will also be copied
	//to the buffer pointed to by "sbp".
	if (s)
		tputs(s, t_lines, outcf); //输出返回的字符值
	outp = cleolbuf; /* fill cleolbuf with clear to eol command */
	outlp = &cleolbuflen;
	cleolbuflen = 0;
	sbp = sbuf;
	s = tgetstr("ce", &sbp);
	if (s)
		tputs(s, 1, outcf);
	outp = scrollrev;
	outlp = &scrollrevlen;
	scrollrevlen = 0;
	sbp = sbuf;
	s = tgetstr("sr", &sbp);
	if (s)
		tputs(s, 1, outcf);
	outp = strtstandout;
	outlp = &strtstandoutlen;
	strtstandoutlen = 0;
	sbp = sbuf;
	s = tgetstr("so", &sbp);
	if (s)
		tputs(s, 1, outcf);
	outp = endstandout;
	outlp = &endstandoutlen;
	endstandoutlen = 0;
	sbp = sbuf;
	s = tgetstr("se", &sbp);
	if (s)
		tputs(s, 1, outcf);
	sbp = cursorm;
	cm = tgetstr("cm", &sbp);
	if (cm)
		dumb_term = NA;
	else
		dumb_term = YEA;
	sbp = UPbuf;
	UP = tgetstr("up", &sbp);
	sbp = BCbuf;
	BC = tgetstr("bc", &sbp);
	if (dumb_term) {
		t_lines = 24;
		t_columns = 255;
	}

	return YEA;
}

//移动到位置(destline,destcol),同时用outc输出
void do_move(int destcol, int destline, int (*outc)()) {
	tputs(tgoto(cm, destcol, destline), 0, outc);
	//tgoto对传递进来的参数实现对应的能力,并把结果给tputs调用
	//tputs利用outc函数输出tgoto传进来的参数
}

