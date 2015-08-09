#ifndef  _BBS_H_
#define _BBS_H_

/* Global includes, needed in almost every source file... */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <setjmp.h>
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

#include "site.h"             	/* User-configurable stuff */
#include "functions.h"				/* you can enable functions that you want */
#include "permissions.h"
#include "bbserrno.h"

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
#define NUMPERMS   (32)
#ifdef FDQUAN
#define MAXGUEST	80	   	/* 最多 guest 帐号上站个数 */
#else
#define MAXGUEST	500
#endif

#define MAX_POSTRETRY       100
#define MAX_PREFIX			  9    /* 最大版面前缀数*/

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

#define RNDPASSLEN             10        /* 暗码认证的暗码长度 (适宜范围 4~10)*/

#define PASSFILE     ".PASSWDS"    /* Name of file User records stored in */

#define BOARDS      ".BOARDS"      /* File containing list of boards */
#define DOT_DIR     ".DIR"         /* Name of Directory file info */
#define THREAD_DIR  ".THREAD"      /* Name of Thread file info */
#define DIGEST_DIR  ".DIGEST"      /* Name of Digest file info */
#define TRASH_DIR   ".TRASH"
#define JUNK_DIR    ".JUNK"

#define TRASH_MODE       10		//版主垃圾箱模式
#define JUNK_MODE	     11		//站务垃圾箱模式
#define ATTACH_MODE          13         //??
#define ANNPATH_SETMODE	0
#define ANNPATH_GETMODE	1

#ifdef FDQUAN
#define MAX_NOTICE 6
#else
#define MAX_NOTICE 5
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
#define BOARD_CUSTOM_FLAG	0x80000000		//收藏夹自定义目录 defined by cometcaptor 2007-04-16 因为是目录是自定义，故选用标志的最高位，和标准的属性分开

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

//% "★★ 欢迎光临日月光华站! ★★"
#define DEF_VALUE		"\xa1\xef\xa1\xef \xbb\xb6\xd3\xad\xb9\xe2\xc1\xd9\xc8\xd5\xd4\xc2\xb9\xe2\xbb\xaa\xd5\xbe! \xa1\xef\xa1\xef"
#define DEF_FILE		"etc/whatdate"

#define ALL_PAGER       0x1
#define FRIEND_PAGER    0x2
#define ALLMSG_PAGER    0x4
#define FRIENDMSG_PAGER 0x8
#define LOGOFFMSG_PAGER 0x10   /* Amigo 2002.04.03 */

/** Some constants. */
enum {
	EXT_IDLEN = 16,    ///< length of userid field including 'NUL'.
	IP_LEN = 40,       ///< max length of an IP address.
	EMAIL_LEN = 40,    ///< max length of an email address.
};

#include "struct.h"

enum {
	DONOTHING   = 0,  /* Read menu command return states */
    FULLUPDATE  = 1,  /* Entire screen was destroyed in this oper*/
    PARTUPDATE  = 2,  /* Only the top three lines were destroyed */
	DOQUIT      = 3,  /* Exit read menu was executed */
	READ_NEXT   = 5,  /* Direct read next file */
	READ_PREV   = 6,  /* Direct read prev file */
	GOTO_NEXT   = 7,  /* Move cursor to next */
	DIRCHANGED  = 8,  /* Index file was changed */
	READ_AGAIN  = 10,
	MINIUPDATE  = 11,
};

extern int digestmode; /*To control Digestmode*/

extern struct userec currentuser;/*  user structure is loaded from passwd */
/*  file at logon, and remains for the   */
/*  entire session */

extern int usernum; /* Index into passwds file user record */
extern int utmpent; /* Index into this users utmp file entry */

extern struct userec lookupuser; /* Used when searching for other user info */

extern const char *currboard; /* name of currently selected board */
extern char currBM[]; /* BM of currently selected board */

extern char genbuf[1024]; /* generally used global buffer */

extern jmp_buf byebye; /* Used for exception condition like I/O error*/

extern int in_mail;
extern int showansi;
extern time_t uptime; /* save user last key-in time, up every 1min */

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

/* =============== ANSI EDIT ================== */
extern int KEY_ESC_arg;
/* ============================================ */

//add by infotech,supporting for 5 BMS
#define BMMAXNUM		(5)				//每个版面的最大版主数
#define BMNAMEMAXLEN	(56)		//每个版版主名称时最大数
//如果BMNAMELISTLEN与BMNAMEMAXLEN不同,显示版主列表时,将用...来表示超过部分
//BMNAMELISTLEN一定要小于BMNAMEMAXLEN,且均小于56,否则内存越界

#define WRAPMARGIN (255)

enum {
	LINE_BUFSIZE = 256,  ///< Line buffer size.
};

#include "func.h"

void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
int remove_shm(const char *shmstr, int defaultkey, int shmsize);

#endif /* of _BBS_H_ */
