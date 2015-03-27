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
 $Id: permissions.h 321 2006-10-27 14:13:07Z danielfree $
 */

/* These are all the permissions available in the BBS */

#define PERM_LOGIN      0x1
#define PERM_TALK       0x2
#define PERM_MAIL       0x4
#define PERM_POST       0x8
#define PERM_REGISTER   0x10
#define PERM_BINDMAIL   0x20
#define PERM_BOARDS     0x40
#define PERM_OBOARDS    0x80
#define PERM_OCLUB      0x100
#define PERM_ANNOUNCE   0x200
#define PERM_OCBOARD    0x400
#define PERM_USER       0x800
#define PERM_OCHAT      0x1000
#define PERM_SYSOPS     0x2000
#define PERM_CLOAK      0x4000
#define PERM_SEECLOAK   0x8000
#define PERM_XEMPT      0x10000
#define PERM_LONGLIFE   0x20000
#define PERM_LARGEMAIL  0x40000
#define PERM_ARBI       0x80000
#define PERM_SERV       0x100000
#define PERM_TECH       0x200000
#define PERM_SPECIAL0   0x400000
#define PERM_SPECIAL1   0x800000
#define PERM_SPECIAL2   0x1000000
#define PERM_SPECIAL3   0x2000000
#define PERM_SPECIAL4   0x4000000
#define PERM_SPECIAL5   0x8000000
#define PERM_SPECIAL6   0x10000000
#define PERM_SPECIAL7   0x20000000
#define PERM_SPECIAL8   0x40000000
#define PERM_SPECIAL9   0x80000000

/* This is the default permission granted to all new accounts. */
/* Following 2 lines modified by Amigo 2002.06.08. To add mail right. */
#define PERM_DEFAULT    (PERM_LOGIN | PERM_TALK | PERM_MAIL | PERM_POST | PERM_REGISTER)

/* These permissions are bitwise ORs of the basic bits. They work that way
 too. For example, anyone with PERM_SYSOP or PERM_OBOARDS or both has
 PERM_BLEVELS. */

#define PERM_ADMINMENU  (PERM_SYSOPS |PERM_OBOARDS |PERM_OCBOARD)
#define PERM_MULTILOG   PERM_SYSOPS
#define PERM_ESYSFILE   PERM_SYSOPS
#define PERM_LOGINCLOAK (PERM_SYSOPS | PERM_CLOAK)
#define PERM_SEEULEVELS (PERM_SYSOPS | PERM_SEECLOAK)
#define PERM_BLEVELS    (PERM_SYSOPS | PERM_OBOARDS)
#define PERM_MARKPOST   (PERM_SYSOPS | PERM_BOARDS)
#define PERM_UCLEAN     (PERM_SYSOPS | PERM_ACCOUNTS)
#define PERM_NOTIMEOUT  PERM_SYSOPS
#define PERM_READMAIL   PERM_LOGIN
#define PERM_VOTE       PERM_LOGIN

/* These are used only in Internet Mail Forwarding */
/* You may want to be more restrictive than the default, especially for an
 open access BBS. */

/* Following line modified by Amigo 2002.06.08. To add mail right. */
//#define PERM_SETADDR    PERM_POST      /* to set address for forwarding */
#define PERM_SETADDR    PERM_MAIL      /* to set address for forwarding */
#define PERM_FORWARD    PERM_LOGIN     /* to do the forwarding */

/* Don't mess with this. */
#define HAS_PERM(x)     ((x)?currentuser.userlevel&(x):1)
#define HAS_PERM2(x, y) ((x)?(y)->userlevel&(x):1)
#define DEFINE(x)     ((x)?currentuser.userdefine&(x):1)
#define HAS_DEFINE(x,y)	((y)?x&(y):1)
/* HAS_DEFINE(userdefine, flag) */

#define DEF_FRIENDCALL   0x00000001
#define DEF_ALLMSG       0x00000002
#define DEF_FRIENDMSG    0x00000004
#define DEF_SOUNDMSG     0x00000008
#define DEF_COLOR        0x00000010
#define DEF_ACBOARD      0x00000020
#define DEF_ENDLINE      0x00000040
#define DEF_EDITMSG      0x00000080
#define DEF_NOTMSGFRIEND 0x00000100
#define DEF_NORMALSCR    0x00000200
#define DEF_NEWPOST      0x00000400
#define DEF_CIRCLE       0x00000800
#define DEF_FIRSTNEW     0x00001000
#define DEF_LOGFRIEND    0x00002000
#define DEF_LOGINFROM    0x00004000
#define DEF_NOTEPAD      0x00008000
#define DEF_NOLOGINSEND  0x00010000
#define DEF_THESIS	 0x00020000	/* youzi */
#define DEF_MSGGETKEY    0x00040000
#define DEF_GRAPH        0x00080000
#define DEF_TOP10        0x00100000
#define DEF_RANDSIGN	 0x00200000
#define DEF_S_HOROSCOPE  0x00400000
#define DEF_COLOREDSEX	 0x00800000
#define DEF_NOT_N_MASK   0x01000000
#define DEF_AUTOWRAP	 0x04000000
#define DEF_USEGB        0x08000000     /*  KCN  */
#define DEF_NOTHIDEIP	 0x10000000
#define DEF_LOGOFFMSG	 0x20000000     /* Amigo 2002.04.03*/
#define DEF_MULTANNPATH  0x40000000
