#ifndef  _BBS_H_
#define _BBS_H_

/* Global includes, needed in almost every source file... */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <strings.h>
#include <limits.h>
#ifndef BSD44
#include <stdlib.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <dirent.h>
#include <syslog.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "site.h"             	/* User-configurable stuff */
#include "functions.h"				/* you can enable functions that you want */
#include "permissions.h"
#include "libBBS.h"
#include "bbserrno.h"

#ifndef LOCK_EX
#define LOCK_EX         2       /* exclusive lock */
#define LOCK_UN         8       /* unlock */
#endif

#define randomize() srand((unsigned)time(NULL))	//随机函数初始化
#define YEA (1)        /* Booleans  (Yep, for true and false) */
#define NA  (0) 

#define DOECHO (1)     /* Flags to getdata input function */
#define NOECHO (0)

/*Added by Ashinmarch on 12.24
 *to support multi-line msgs
 */
#define MAX_MSG_SIZE (240-1)
#define MAX_MSG_LINE 2
#define LINE_LEN 78
/*added end*/
#define MAXFRIENDS (300)
#define MAXREJECTS (32)
#define NUMPERMS   (32)
#ifdef FDQUAN
#define MAXGUEST	80	   	/* 最多 guest 帐号上站个数 */
#else
#define MAXGUEST	500
#endif

#ifdef FDQUAN
#define MAX_ANON    300
#else
#define MAX_ANON	2500	//最大WWW匿名数
#endif

#define FILE_BUFSIZE        200    /* max. length of a file in SHM*/
#define FILE_MAXLINE         25    /* max. line of a file in SHM */
#define MAX_WELCOME          15    /* 欢迎画面数 */
#define MAX_GOODBYE          15    /* 离站画面数 */
#define MAX_ISSUE            15    /* 最大进站画面数 */
#define MAX_DIGEST         1000    /* 最大文摘数 */
#define MAX_POSTRETRY       100
#define MAX_PREFIX			  9    /* 最大版面前缀数*/
#define MORE_BUFSIZE       4096

#ifdef BIGGER_MOVIE
#define MAXMOVIE		8
#else
#define MAXMOVIE		6    /* lines of  activity board  */
#endif

#define ACBOARD_BUFSIZE      512 /* max. length of each line for activity board*/
#define ACBOARD_MAXLINE      256 /* max. lines of  activity board  */
#define STRLEN               80    /* Length of most string data */
#define BM_LEN               60    /* Length of BM id length*/
#define NAMELEN              40    /* Length of username/realname */
#define IDLEN                12    /* Length of userids */
#define HOMELEN              80 // Length of relative pathname
                                // under BBSHOME except BBSHOME/0Announce
#define IPLEN                40 // Length of IP address.

#ifdef MD5		  	   /* MD5 cryptographic support */
#define ENCPASSLEN         35
#else
#define ENCPASSLEN         14  /* Length of encrypted passwd field */
#endif  

#define PASSLEN		 	14    /* User's password length (13 chars) */

//#ifdef CODE_VALID
#define RNDPASSLEN             10        /* 暗码认证的暗码长度 (适宜范围 4~10)*/
//#endif

#define PASSFILE     ".PASSWDS"    /* Name of file User records stored in */
#define ULIST_BASE   ".UTMP"       /* Names of users currently on line */
extern char ULIST[];

#define BOARDS      ".BOARDS"      /* File containing list of boards */
#define DOT_DIR     ".DIR"         /* Name of Directory file info */
#define THREAD_DIR  ".THREAD"      /* Name of Thread file info */
#define DIGEST_DIR  ".DIGEST"      /* Name of Digest file info */
#define MARKED_DIR  ".MARKEDDIR"
#define AUTHOR_DIR  ".AUTHORDIR"
#define KEY_DIR	    ".KEYDIR"
#define BOTTOM_DIR  ".BOTTOM"
#define KEYWORDS    ".KEYWORDS"
#define TRASH_DIR   ".TRASH"
#define JUNK_DIR    ".JUNK"

//digest mode
#define NORMAL_MODE       0		//一般模式
#define DIGIST_MODE       1		//文摘模式（G文）
#define THREAD_MODE       2		//同主题模式
#define MARK_MODE         3		//m文模式
#define ORIGIN_MODE       4		//原作模式
#define AUTHOR1_MODE      5             //同作者模糊搜索模式
#define AUTHOR2_MODE      6             //同作者精确搜索模式
#define KEYWORD_MODE      7		//标题搜索模式
#define TRASH_MODE       10		//版主垃圾箱模式
#define JUNK_MODE	     11		//站务垃圾箱模式
#define BOTTOM_MODE	     12		//??
#define ATTACH_MODE          13         //??
#define ANNPATH_SETMODE	0
#define ANNPATH_GETMODE	1

#ifdef ENABLE_NOTICE
#define NOTICE_MODE 13
#define NOTICE_DIR ".NOTICE"
#ifdef FDQUAN
#define MAX_NOTICE 6
#else
#define MAX_NOTICE 5
#endif
#endif

#define QUIT 			0x666    	/* Return value to abort recursive functions */

#define FILE_READ  		0x1        	/* Ownership flags used in fileheader structure */
#define FILE_OWND  		0x2        	/* accessed array */
#define FILE_VISIT 		0x4
#define FILE_MARKED 	0x8
#define FILE_DIGEST 	0x10      	/* Digest Mode*/
#define FILE_FORWARDED 	0x20  		/* Undelete file */
#define MAIL_REPLY 		0x20 		/* Mail Reply */
#define FILE_NOREPLY 	0x40      	/* No Allow Replay */
#define FILE_DELETED 	0x80

#define FILE_NOTICE     0x01
#define FILE_SUBDEL     0x02
#define FILE_LASTONE	0x04
#define FILE_IMPORTED   0x08

/*	版面的标志		*/
#define BOARD_VOTE_FLAG    	0x1		//投票模式
#define BOARD_NOZAP_FLAG   	0x2		//no zap
#define BOARD_OUT_FLAG     	0x4		//转信
#define BOARD_ANONY_FLAG   	0x8		//匿名模式
#define BOARD_NOREPLY_FLAG 	0x10		//No reply board
#define BOARD_JUNK_FLAG	        0x20		//不记文章数
#define BOARD_CLUB_FLAG         0x40		//俱乐部版面
#define BOARD_READ_FLAG		0x80		//读限制版面
#define BOARD_POST_FLAG         0x100		//postmask
#define BOARD_DIR_FLAG		0x200		//目录
#define BOARD_PREFIX_FLAG	0x400	//前缀
#define BOARD_CUSTOM_FLAG	0x80000000		//收藏夹自定义目录 defined by cometcaptor 2007-04-16 因为是目录是自定义，故选用标志的最高位，和标准的属性分开
#define ZAPPED  		0x1         /* For boards...tells if board is Zapped */

/** These are flags in userec.flags[0] */
enum {
	PAGER_FLAG     = 0x1,  ///< True if pager was OFF last session.
	CLOAK_FLAG     = 0x2,  ///< True if cloak was ON last session.
	BRD_HIDEREAD   = 0x4,  ///< True if read boards are hidden.
	BRDSORT_UPDATE = 0x8,  ///< True if boards are sorted by update time.
	BRDSORT_UDEF   = 0x10, ///< True if boards are sorted in user-defined order.
	BRDSORT_FLAG   = 0x20, ///< True if boards are sorted alphabetical.
	BRDSORT_ONLINE = 0x40, ///< True if boards are sorted by online users.
	GIVEUPBBS_FLAG = 0x80, ///< True if the user gives up BBS now.
};

#define DEF_VALUE		"★★ 欢迎光临日月光华站! ★★"
#define DEF_FILE		"etc/whatdate"

#define ALL_PAGER       0x1
#define FRIEND_PAGER    0x2
#define ALLMSG_PAGER    0x4
#define FRIENDMSG_PAGER 0x8
#define LOGOFFMSG_PAGER 0x10   /* Amigo 2002.04.03 */

#define USERIDSIZE 		(16)
#define USERNAMESZ 		(24)
#define TERMTYPESZ 		(10)
/* END */

/** Some constants. */
enum {
	EXT_IDLEN = 16,    ///< length of userid field including 'NUL'.
	IP_LEN = 40,       ///< max length of an IP address.
	EMAIL_LEN = 40,    ///< max length of an email address.
	IOBUFSIZE = 4096,  ///< I/O buffer size.
	OBUFSIZE = 4096,   ///< output buffer size.
};

#ifndef NO_STRUCT_H
#include "struct.h"
#endif

#include "modes.h"              /* The list of valid user modes */

enum {
	DONOTHING   = 0,  /* Read menu command return states */
    FULLUPDATE  = 1,  /* Entire screen was destroyed in this oper*/
    PARTUPDATE  = 2,  /* Only the top three lines were destroyed */
	DOQUIT      = 3,  /* Exit read menu was executed */
	NEWDIRECT   = 4,  /* Directory has changed, re-read files */
	READ_NEXT   = 5,  /* Direct read next file */
	READ_PREV   = 6,  /* Direct read prev file */
	GOTO_NEXT   = 7,  /* Move cursor to next */
	DIRCHANGED  = 8,  /* Index file was changed */
	MODECHANGED = 9,  /* ... */
	READ_AGAIN  = 10,
	MINIUPDATE  = 11,
};

#define I_TIMEOUT   (-2)         /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)       /* interface, (-3) will conflict with chinese */

#define BMLOGLEN			32
#define BMLOG_STAYTIME		0			//停留时间
#define BMLOG_INBOARD		1			//进版
#define BMLOG_POST			2			//版内发文
#define BMLOG_DIGIST		3			//收入文摘
#define BMLOG_UNDIGIST		4			//去掉文摘
#define BMLOG_MARK          5           //标记m文 
#define BMLOG_UNMARK        6           //去掉m标记
#define BMLOG_WATER         7           //标记水文
#define BMLOG_UNWATER       8           //去掉水文
#define BMLOG_CANNOTRE		9           //标记不可re
#define BMLOG_UNCANNOTRE	10          //去掉不可re
#define BMLOG_DELETE        11          //删除文章(d/B/L/D)
#define BMLOG_UNDELETE      12          //恢复删除(y/B/L)
#define BMLOG_DENYPOST      13          //封禁
#define BMLOG_UNDENY        14          //解封
#define BMLOG_ADDCLUB       15          //加入俱乐部
#define BMLOG_DELCLUB       16          //取消俱乐部
#define BMLOG_ANNOUNCE      17          //收入精华
#define BMLOG_DOANN         18          //整理精华
#define BMLOG_COMBINE       19          //合集文章
#define BMLOG_RANGEANN		20          //L或者B收入精华
#define BMLOG_RANGEDEL      21          //L或者B删除
#define BMLOG_RANGEOTHER    23          //L或者B其他操作
extern int scrint; /* Set when screen has been initialized */
/* Used by namecomplete *only* */

extern int digestmode; /*To control Digestmode*/

#ifndef NO_STRUCT_H				 /* NO_STRUCT_H */

extern struct userec currentuser;/*  user structure is loaded from passwd */
/*  file at logon, and remains for the   */
/*  entire session */

extern struct user_info uinfo; /* Ditto above...utmp entry is stored here
 and written back to the utmp file when
 necessary (pretty darn often). */
extern int usernum; /* Index into passwds file user record */
extern int utmpent; /* Index into this users utmp file entry */
extern int count_friends; /*Add by SmallPig for count users and friends*/

extern int t_lines, t_columns; /* Screen size / width */
extern struct userec lookupuser; /* Used when searching for other user info */

#endif					/* NO_STRUCT_H */

extern char currboard[]; /* name of currently selected board */
extern char currBM[]; /* BM of currently selected board */
extern struct boardheader *currbp; // boardheader pointer to currently selected board

extern int selboard; /* THis flag is true if above is active */

extern char genbuf[1024]; /* generally used global buffer */

extern jmp_buf byebye; /* Used for exception condition like I/O error*/

extern char save_title[]; /* These are used by the editor when inserting */
extern int save_gid;
extern char save_filename[]; /* header information */
extern int in_mail;
extern bool dumb_term;
extern int showansi;
extern time_t uptime; /* save user last key-in time, up every 1min */

#define Min(a,b) ((a<b)?a:b)
#define Max(a,b) ((a>b)?a:b)

/*SREAD Define*/
#define SR_BMBASE       (10)
#define SR_BMDEL		(11)
#define SR_BMMARK       (12)
#define SR_BMDIGEST     (13)
#define SR_BMIMPORT     (14)
#define SR_BMTMP        (15)
#define SR_BMUNDERLINE 	(16) 
/*SREAD Define*/

#ifndef EXTEND_KEY
#define EXTEND_KEY0
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_BACKSPACE   0x7f
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206
#endif	//EXTEND_KEY	扩展键
/* edwardc.990706 move shmkeys from sysconf.ini */

#define SEM_COUNTONLINE 30000

#define Ctrl(c)         ( c & 037 )		//可以考虑将函数宏改写成inline函数
#define isprint2(c)     ( (c & 0x80) || isprint(c) )	//扩充的可打印字符
//包括ASCII中所有可打印字符和第一位为1的字符

#ifdef  SYSV						//SYSV中没有定义以下函数
//#define bzero(tgt, len)         memset( tgt, 0, len )
#define bcopy(src, tgt, len)    memcpy( tgt, src, len)

#define usleep(usec)            {               \
    struct timeval t;                           \
    t.tv_sec = usec / 1000000;                  \
    t.tv_usec = usec % 1000000;                 \
    select( 0, NULL, NULL, NULL, &t);           \
}

#endif  /* SYSV */

/* =============== ANSI EDIT ================== */
#define   ANSI_RESET    	"\033[0m"
#define   ANSI_REVERSE  	"\033[7m\033[4m"
extern int editansi;
extern int KEY_ESC_arg;
/* ============================================ */

#define FLOCK(x,y) flock(x,y)

#define SHM_HOMEDIR "tmp"

//add by infotech,supporting for 5 BMS
#define BMMAXNUM		(5)				//每个版面的最大版主数
#define BMNAMELISTLIMIT					//显示超过部分时使用...
#define BMNAMELISTLEN	(56)		//显示版面版主列表的最大数
#define BMNAMEMAXLEN	(56)		//每个版版主名称时最大数
//如果BMNAMELISTLEN与BMNAMEMAXLEN不同,显示版主列表时,将用...来表示超过部分
//BMNAMELISTLEN一定要小于BMNAMEMAXLEN,且均小于56,否则内存越界

enum DATE_FORMAT {
	DATE_ZH = 0,      ///< "2001年02月03日04:05:06 星期六"
	DATE_EN = 1,      ///< "02/03/01 04:05:06"
	DATE_SHORT = 2,   ///< "02.03 04:05"
	DATE_ENWEEK = 4,  ///< "02/03/01 04:05:06 Sat"
	DATE_XML = 8,     ///< "2001-02-03T04:05:06"
	DATE_RSS = 16,    ///< "Sat,03 Feb 2001 04:05:06 +0800"
};

/**
 * Telnet option negotiation sequence status.
 */
enum {
	TELST_NOR,  ///< normal byte
	TELST_IAC,  ///< right after IAC
	TELST_COM,  ///< right after IAC DO/DONT/WILL/WONT
	TELST_SUB,  ///< right after IAC SB
	TELST_SBC,  ///< right after IAC SB [COMMAND]
	TELST_END,  ///< end of an telnet option
};

enum {
	LINE_BUFSIZE = 256,  ///< Line buffer size.
};

#include "func.h"
#include "../lib/goodbrd.h"

#endif /* of _BBS_H_ */
