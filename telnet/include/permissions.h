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
//#define PERM_DEFAULT    (PERM_BASIC | PERM_CHAT | PERM_PAGE | \
//                         PERM_POST | PERM_LOGINOK|PERM_MESSAGE)
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
#define DEFINE(x)     ((x)?currentuser.userdefine&(x):1)
#define HAS_DEFINE(x,y)	((y)?x&(y):1)
/* HAS_DEFINE(userdefine, flag) */

#ifndef EXTERN
extern char *permstrings[];
#else

/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
   depending on how/if you use them. */
/* skyo.0507 modify 加入後面的 PERM 方便跟 menu.ini 对照） */
char *permstrings[] = {
	"上站权力       (PERM_LOGIN)",  /* PERM_LOGIN */
	"与他人聊天     (TALK)",        /* PERM_TALK */
	"发送信件       (MAIL)",        /* PERM_MAIL */
	"发表文章       (POST)",        /* PERM_POST */
	"使用者资料正确 (REGISTER)",    /* PERM_REGISTER*/
	"绑定邮箱       (BINDMAIL)",    /* PERM_BINDMAIL */
	"版主           (BOARDS)",      /* PERM_BOARDS */
	"讨论区总管     (OBOARDS)",     /* PERM_OBOARDS */
	"俱乐部总管     (OCLUB)",       /* PERM_OCLUB */
	"精华区总管     (ANNOUNCE)",    /* PERM_ANNOUNCE*/
	"活动看版总管   (OCBOARD)",     /* PERM_OCBOARD */
	"帐号管理员     (USER)",        /* PERM_USER*/
	"聊天室管理员   (OCHAT)",       /* PERM_OCHAT*/
	"系统维护管理员 (SYSOPS)",      /* PERM_SYSOPS*/
	"隐身术         (CLOAK)",       /* PERM_CLOAK */
	"看穿隐身术     (SEECLOAK)",    /* PERM_SEECLOAK */
	"帐号永久保留   (XEMPT)",       /* PERM_XEMPT */
	"生命值增强权限 (LONGLIFE)",    /* PERM_LONGLIFE */
	"大信箱         (LARGEMAIL)",   /* PERM_LARGEMAIL*/
	"仲裁组         (ARBI)",        /* PERM_ARBI*/
	"服务组         (SERV)",        /* PERM_SERV*/
	"技术组         (TECH)",        /* PERM_TECH*/
	"特殊权限 0     (SPECIAL0)",    /* PERM_SPECIAL0*/
	"特殊权限 1     (SPECIAL1)",    /* PERM_SPECIAL1*/
	"特殊权限 2     (SPECIAL2)",    /* PERM_SPECIAL2*/
	"特殊权限 3     (SPECIAL3)",    /* PERM_SPECIAL3*/
	"特殊权限 4     (SPECIAL4)",    /* PERM_SPECIAL4*/
	"特殊权限 5     (SPECIAL5)",    /* PERM_SPECIAL5*/
	"特殊权限 6     (SPECIAL6)",    /* PERM_SPECIAL6*/
	"特殊权限 7     (SPECIAL7)",    /* PERM_SPECIAL7*/
	"特殊权限 8     (SPECIAL8)",    /* PERM_SPECIAL8*/
	"特殊权限 9     (SPECIAL9)",    /* PERM_SPECIAL9*/
	NULL
};
#endif

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
#define DEF_DELDBLCHAR	 0x02000000
#define DEF_AUTOWRAP	 0x04000000
#define DEF_USEGB        0x08000000     /*  KCN  */
#define DEF_NOTHIDEIP	 0x10000000
#define DEF_LOGOFFMSG	 0x20000000     /* Amigo 2002.04.03*/
#define DEF_MULTANNPATH  0x40000000

#define NUMDEFINES 31

#ifndef EXTERN
extern char *user_definestr[];
#else
/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
   depending on how/if you use them. */
char *user_definestr[] = {
        "呼叫器关闭时可让好友呼叫",     /* DEF_FRIENDCALL */
        "接受所有人的讯息",             /* DEF_ALLMSG */
        "接受好友的讯息",               /* DEF_FRIENDMSG */
        "收到讯息发出声音",             /* DEF_SOUNDMSG */
        "使用彩色",            		/* DEF_COLOR */
        "显示活动看板",             	/* DEF_ACBOARD */
        "显示选单的讯息栏",             /* DEF_ENDLINE */
        "编辑时显示状态栏",     	/* DEF_EDITMSG */
        "讯息栏采用一般/精简模式",	/* DEF_NOTMSGFRIEND */
        "选单采用一般/精简模式",	/* DEF_NORMALSCR */
        "分类讨论区以 New 显示",	/* DEF_NEWPOST */
        "阅读文章是否使用绕卷选择",   	/* DEF_CIRCLE */
        "阅读文章游标停於第一篇未读",	/* DEF_FIRSTNEW */
        "进站时显示好友名单",   	/* DEF_LOGFRIEND */
        "好友上站通知",                 /* DEF_LOGINFROM */
        "观看留言板",                   /* DEF_NOTEPAD*/
        "不要送出上站通知给好友",       /* DEF_NOLOGINSEND */
        "主题式看版",                   /* DEF_THESIS */
        "收到讯息等候回应或清除",       /* DEF_MSGGETKEY */
        "进站时观看上站人次图",         /* DEF_GRAPH */
        "进站时观看十大排行榜",         /* DEF_TOP10 */
        "使用乱数签名档",		/* DEF_RANDSIGN */
        "显示星座",			/* DEF_S_HOROSCOPE */
        "星座使用颜色来显示性别",	/* DEF_COLOREDSEX */
		"使用\'+\'标记未读文章",	/* DEF_NOT_N_MASK */
		"汉字整字删除",                 /* DEF_DELDBLCHAR */
		"自动排版宽度预设为 78 列",	/* DEF_AUTOWRAP */
		"使用GB码阅读",                 /* DEF_USEGB KCN 99.09.03 */	
		"不隐藏自己的 IP",		/* DEF_NOTHIDEIP */
		"好友离站通知",                 /* DEF_LOGOFFMSG Amigo 2002.04.03 */
		"使用多丝路(版主权限有效)",			/* DEF_MULTANNPATH*/	
		NULL
};
#endif
