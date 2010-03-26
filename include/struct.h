#ifndef FB_STRUCT_H
#define FB_STRUCT_H

#include "config.h"

typedef unsigned char uschar;

/** User infomation on disk. */
struct userec {
	unsigned int uid;         ///< unique uid, not yet set now.
	unsigned int userlevel;   ///< permission bits.
	unsigned int numlogins;   ///< number of logins.
	unsigned int numposts;    ///< number of posts.
	unsigned int stay;        ///< total online time in seconds.
	int nummedals;            ///< number of medals.
	int money;                ///< money.
	int bet;                  ///< loan.
	char flags[2];            ///< some user preferences.
	char passwd[PASSLEN];     ///< encrypted password.
	unsigned int nummails;    ///< number of mails.
	char gender;              ///< gender.
	unsigned char birthyear;  ///< year of birth.
	unsigned char birthmonth; ///< month of birth.
	unsigned char birthday;   ///< day of birth.
	int signature;            ///< number of signatures.
	unsigned int userdefine;  ///< user preferences.
	unsigned int prefs;       ///< exdended user preferences, not yet used.
	// TODO: remove noteline
	int noteline;             ///< will be removed soon.
	time_t firstlogin;       ///< time of first login.
#if SIZEOF_TIME_T == 4
	char pad1[4];
#endif
	time_t lastlogin;        ///< time of last login.
#if SIZEOF_TIME_T == 4
	char pad2[4];
#endif
	time_t lastlogout;       ///< time of last logout.
#if SIZEOF_TIME_T == 4
	char pad3[4];
#endif
	time_t dateforbet;       ///< loan deadline.
#if SIZEOF_TIME_T == 4
	char pad4[4];
#endif
	// TODO: remove notedate
	int64_t notedate;         ///< will be removed soon.
	char userid[EXT_IDLEN];   ///< userid.
	char lasthost[IP_LEN];    ///< last login IP address.
	char username[NAMELEN];   ///< nick.
	char email[EMAIL_LEN];    ///< email address.
	char reserved[8];         ///< reserved for future use.
};

struct user_info { /* Structure used in UTMP file */
	/* added by roly for compatiable with NJU 0.9 */
	int utmpkey;
	/* add end */
	int active; /* When allocated this field is true */
	int uid; /* Used to find user name in passwd file */
	int pid; /* kill() to notify user of talk request */
	int invisible; /* Used by cloaking function in Xyz menu */
	int sockactive; /* Used to coordinate talk requests */
	int sockaddr; /* ... */
	int destuid; /* talk uses this to identify who called */
	int mode; /* UL/DL, Talk Mode, Chat Mode, ... */
	int pager; /* pager toggle, YEA, or NA */
	int in_chat; /* for in_chat commands   */
	int fnum; /* number of friends */
	int ext_idle; /* has extended idle time, YEA or NA */
	char chatid[ 10 ]; /* chat id, if in chat mode */
	char from[60];
	int currbrdnum;
	time_t idle_time; /* to keep idle time */
	char userid[ 20 ];
	char realname[ 20 ];
	char username[NAMELEN];
	int friend[MAXFRIENDS];
	int reject[MAXREJECTS];
};

/*add by Ashinmarch*/
#define SCHOOLNUMLEN    9
#define IDCARDLEN       18
#define DIPLOMANUMLEN   10
#define MOBILENUMLEN    12
struct schoolmate_info {
	char userid[IDLEN+2];
	char school_num[SCHOOLNUMLEN];
	char email[STRLEN];
	char identity_card_num[IDCARDLEN];
	char diploma_num[DIPLOMANUMLEN];
	char mobile_num[MOBILENUMLEN];
};
/*add end*/

struct override {
	char id[13];
	char exp[40];
};
typedef struct override override_t;

struct boardheader { /* This structure is used to hold data in */
	char filename[STRLEN - 8]; /* the BOARDS files */
	unsigned int nowid;
	int group;
	char owner[STRLEN - BM_LEN];
	char BM[BM_LEN - 4];
	unsigned int flag;
	char title[STRLEN];
	unsigned int level;
	unsigned char accessed[12];
};

struct fileheader { /* This structure is used to hold data in */
	char filename[STRLEN-8]; /* the DIR files */
	unsigned int id;
	unsigned int gid;
	char owner[STRLEN];
	char title[STRLEN-IDLEN-1];
	char szEraser[IDLEN+1];
	unsigned level;
	unsigned char accessed[4]; /* struct size = 256 bytes */
	unsigned int reid;
	int timeDeleted;
};

//move  shortfile to bstat. eefree 06.04.26
struct bstat { /* used for caching files and boards */
	int total;
	int lastpost;
	int inboard;
	unsigned int nowid;
};

struct one_key { /* Used to pass commands to the readmenu */
	int key; //输入字符与动作函数一一对应
	int (*fptr)();
};

#define USHM_SIZE       (MAXACTIVE + 150)
struct UTMPFILE {
	struct user_info uinfo[ USHM_SIZE ]; // Cache for online users.
	int usersum; // Count of all users.
	int max_login_num;
	int total_num; // Count of online users.
};

struct BCACHE { //版面的缓冲?
	struct bstat bstatus[MAXBOARD ];
	int number;
	time_t uptime;
	time_t pollvote;
	time_t fresh_date;
	char date[60];
	time_t friendbook; /* friendbook mtime */
	time_t inboarduptime;
};

struct UCACHE { //用户的缓冲
	char userid[MAXUSERS ][IDLEN + 1 ];
	int number;	// last occupied slot in 'userid' array.
	time_t uptime;
	/* add by stiger */
	int next[MAXUSERS];
	int prev[MAXUSERS];
	int hash[26][26][256];
	struct userec passwd[MAXUSERS]; //内存映射的数目太多了一点?
	/* add end */
	int status[MAXUSERS];
};

struct postheader { //.DIR中的记录格式
	char title[STRLEN];
	char ds[40]; //board
#ifdef ENABLE_PREFIX
	char reply_mode;
	char include_mode;
	char chk_anony;
	char postboard;
	char prefix[10];
#else
	int reply_mode;
	char include_mode;
	int chk_anony;
	int postboard;
#endif
};

//Added by IAMFAT 2002.06.14 For Count Function
struct countheader {
	char id[IDLEN+1];
	int all_num;
	int m_num;
	int g_num;
	int w_num;
	int other_num;
	struct countheader* next;
};
//End IAMFAT

typedef struct reginfo { //注册表单信息
	char userid[IDLEN+1];
	char realname[NAMELEN];
	char dept[STRLEN];
	char addr[STRLEN];
	char phone[STRLEN];
	char email[STRLEN-12];
	char assoc[STRLEN];
	time_t regdate;
} REGINFO;

typedef struct _ANONCACHE { //www匿名用户缓冲
	time_t item[MAX_ANON];
	int next[MAX_ANON];
	int freenode;
	int usednode;
	int used;
} ANONCACHE;

typedef struct _NUMFONT {
	int width;
	int height;
	char empty[80];
	char map[10][20][80];
} NUMFONT;

#define TEMPLATE_DIR ".templ"
#define MAX_TEMPLATE 10
#define MAX_CONTENT 20
#define TMPL_BM_FLAG 0x1
#define TMPL_NOW_VERSION 1

struct s_content {
	char text[50];
	int length;
};

struct s_template {
	char title[50];
	char title_prefix[20];
	int content_num;
	char filename[STRLEN];
	int flag;
	int version;
	char unused[16];
	char title_tmpl[STRLEN];
};

struct a_template {
	struct s_template * tmpl;
	struct s_content * cont;
};

struct smenuitem {
	int line, col, level;
	char *name, *desc, *arg;
	int (*fptr)();
};

struct sdefine {
	char *key, *str;
	int val;
};

struct sysheader {
	char *buf;
	int menu, key, len;
};

typedef struct {
	char *buf;
	struct smenuitem *item;
	struct sdefine *var;
	int len;
	int items;
	int keys;
} sysconf_t;

#endif //FB_STRUCT_H

