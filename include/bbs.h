#ifndef  _BBS_H_
#define _BBS_H_

/* Global includes, needed in almost every source file... */
#include <stdio.h>
#include <stdlib.h>
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

#include "config.h"             	/* User-configurable stuff */
#include "functions.h"				/* you can enable functions that you want */
#include "permissions.h"
#include "libBBS.h"

#ifndef LOCK_EX
#define LOCK_EX         2       /* exclusive lock */
#define LOCK_UN         8       /* unlock */
#endif

#ifdef XINU
extern int errno; //出错信息编号
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
#define REG_EXPIRED	3600    	/* 重做身份确认期限 */
#ifdef FDQUAN
#define MAXGUEST	80	   	/* 最多 guest 帐号上站个数 */
#define QUERY_REALNAMES		//站务查询时,是否显示真实姓名
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
#define GOOD_BRC_NUM	70	//收藏夹最大可收藏版面数 2007-04-23 从boards.c移至此
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

#define FLUSH       ".PASSFLUSH"   /* Stores date for user cache flushing */
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

/* these are flags in userec.flags[0] */
#define PAGER_FLAG   	0x1   /* true if pager was OFF last session */
#define CLOAK_FLAG   	0x2   /* true if cloak was ON last session */
#define SIG_FLAG     	0x8   /* true if sig was turned OFF last session */
#define BRDSORT_FLAG 	0x20  /* true if the boards sorted alphabetical */
#define BRDSORT_ONLINE 	0x40
#define GIVEUPBBS_FLAG  0x80  /* true if the user give up BBs now (2003.04.22 stephen)*/
#define ACTIVE_BOARD 	0x200 /* true if user toggled active movie board on */

#define MULTI_LOGINS	2	/* 同时可上站 ID 数 */
#ifndef FDQUAN
#define IPMAXLOGINS		5	// 同IP同时上站ID数
#endif
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

#ifndef NO_STRUCT_H
#include "struct.h"
#endif

#include "modes.h"              /* The list of valid user modes */

#define DONOTHING       0       /* Read menu command return states */
#define FULLUPDATE      1       /* Entire screen was destroyed in this oper*/
#define PARTUPDATE      2       /* Only the top three lines were destroyed */
#define DOQUIT          3       /* Exit read menu was executed */
#define NEWDIRECT       4       /* Directory has changed, re-read files */
#define READ_NEXT       5       /* Direct read next file */
#define READ_PREV       6       /* Direct read prev file */
#define GOTO_NEXT       7       /* Move cursor to next */
#define DIRCHANGED      8       /* Index file was changed */
#define MODECHANGED     9       /* ... */
#define READ_AGAIN		10

#define I_TIMEOUT   (-2)         /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)       /* interface, (-3) will conflict with chinese */

#define SCREEN_SIZE (23)         /* Used by read menu  */

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
extern int dumb_term;
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

struct _shmkey { //共享内存键名与键值
	char key[16];
	int value;
};

const static struct _shmkey shmkeys[] = {
	{ "BCACHE_SHMKEY", 30000 }, { "UCACHE_SHMKEY", 30010 },
	{ "UTMP_SHMKEY", 30020 }, { "ACBOARD_SHMKEY", 30030 },
	{ "ISSUE_SHMKEY", 30040 }, { "GOODBYE_SHMKEY", 30050 },
	{ "WELCOME_SHMKEY", 30060 }, { "STAT_SHMKEY", 30070 },
#ifdef ALLOWSWITCHCODE
	{ "CONV_SHMKEY", 30080 },
#endif
	{ "ACACHE_SHMKEY", 30005 }, { "", 0 } };

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

extern char datestring[];

#define FLOCK(x,y) flock(x,y)

#define SHM_HOMEDIR "tmp"

//add by infotech,supporting for 5 BMS
#define BMMAXNUM		(5)				//每个版面的最大版主数
#define BMNAMELISTLIMIT					//显示超过部分时使用...
#define BMNAMELISTLEN	(56)		//显示版面版主列表的最大数
#define BMNAMEMAXLEN	(56)		//每个版版主名称时最大数
//如果BMNAMELISTLEN与BMNAMEMAXLEN不同,显示版主列表时,将用...来表示超过部分
//BMNAMELISTLEN一定要小于BMNAMEMAXLEN,且均小于56,否则内存越界

//uinfo.c
char *cexpstr(int exp);
#ifdef ALLOWGAME
char *cnummedals(int num);
char *cmoney(int num);
#endif
char *cperf(int perf);
int countexp(struct userec *udata);
int countperf(struct userec *udata);
int julian_day(int year, int month, int day);
int days_elapsed(int year, int month, int day, time_t now);

//brdcache.c (bcache.c)
extern struct BCACHE *brdshm;
extern struct boardheader *bcache;
extern int numboards;
void *attach_shm(const char *shmstr, int defaultkey, int shmsize);
void *attach_shm2(const char *shmstr, int defaultkey, int shmsize, int *iscreate);
void remove_shm(const char *shmstr, int defaultkey, int shmsize);
int updatelastpost(const char *board);
void resolve_boards(void);
void flush_bcache(void);
void rebuild_brdshm(void);
int get_nextid(const char* boardname);
int getblankbnum(void);
struct boardheader *getbcache(const char *bname);
struct bstat *getbstat(const char *bname);
int getbnum(const char *bname, const struct userec *cuser);
int apply_boards(int (*func) (), const struct userec *cuser);
#ifdef NEWONLINECOUNT
void bonlinesync(time_t now);
#endif

//ucache.c (bcache.c)
extern struct UCACHE *uidshm;
extern struct UTMPFILE *utmpshm;
extern struct userec lookupuser;
int cmpuids(void *uid, void *up);
int dosearchuser(const char *userid, struct userec *user, int *unum);
int uhashkey(const char *userid, int *a1, int *a2);
int del_uidshm(int num, char *userid);
int load_ucache(void);
int substitut_record(char *filename, char *rptr, int size, int id);
int flush_ucache(void);
void resolve_ucache(void);
void setuserid(int num, char *userid);
void getuserid(char *userid, int uid, size_t len);
int searchuser(const char *userid);
int getuserec(const char *userid, struct userec *u);
int getuser(const char *userid);
int getuserbyuid(struct userec *u, int uid);
void resolve_utmp(void);
int allusers(void);
int get_online(void);
int get_status(int uid);
int refresh_utmp(void);
int getnewutmpent(struct user_info *up);
int apply_ulist(int (*fptr)());
int search_ulist(struct user_info *uentp, int (*fptr)(), int farg);
int search_ulistn(struct user_info *uentp, int (*fptr)(), int farg, int unum);
void update_ulist(struct user_info *uentp, int uent);
int who_callme(struct user_info *uentp, int (*fptr)(), int farg, int me);

//log.c
void report(const char *s, const char *userid);
void log_usies(const char *mode, const char *mesg, const struct userec *user);

//sysconf.c
extern char *sysconf_buf;
extern int sysconf_menu, sysconf_key, sysconf_len;
extern struct smenuitem *menuitem;
extern struct sdefine *sysvar;
char *sysconf_str(const char *key);
int sysconf_eval(const char *key);
void build_sysconf(const char *configfile, const char *imgfile);

//stuffs.c
char *sethomefile(char *buf, const char *userid, const char *filename);
char *setbpath(char *buf, const char *boardname);
char *setwbdir(char *buf, const char *boardname);
char *setbfile(char *buf, const char *boardname, const char *filename);
char *setmfile(char *buf, const char *userid, const char *filename);
char *setmdir(char *buf, const char *userid);
int safe_mmapfile(const char *filename, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size, int *ret_fd);
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size);
void end_mmapfile(void *ptr, int size, int fd);
extern sigjmp_buf bus_jump;
void sigbus(int signo);
int bbskill(const struct user_info *user, int sig);

//board.c
void changeboard(struct boardheader **bp, char *cboard, const char *board);
int chkBM(const struct boardheader *bp, const struct userec *up);
int isclubmember(const char *member, const char *board);
int hasreadperm(const struct userec *user, const struct boardheader *bp);
int junkboard(const struct boardheader *bp);

//mail.c
int check_maxmail(void);

//io.c
int egetch(void);
void update_endline(void);

#endif /* of _BBS_H_ */
