/* Note the protocol field is not inside an #ifdef FILES...
 this is a waste but allows you to add/remove UL/DL support without
 rebuilding the PASSWDS file (and it's only a lil ole int anyway).
 */

typedef unsigned char uschar;
struct userec { /* Structure used to hold information in */
	char userid[IDLEN+2]; /* PASSFILE */
	time_t firstlogin;
	char lasthost[16]; // Too short for IPv6 addresses
	unsigned int numlogins;
	unsigned int numposts;
	int nummedals; /* 奖章数 money modified u_int to int 2002.11.19*/
	int money; /* 存款 */
	int bet; /* 贷款 */
	time_t dateforbet;
	char flags[2];
#ifdef ENCPASSLEN
	char passwd[ENCPASSLEN];
#else
	char passwd[PASSLEN];
#endif
	char username[NAMELEN];
	char ident[NAMELEN];
	char termtype[16];
	char reginfo[STRLEN-16];
	unsigned int userlevel;
	//unsigned long   dwExLevel;
	time_t lastlogin;
	time_t lastlogout;/* 最近离线时间 */
	time_t stay;
	char realname[NAMELEN];
	char address[STRLEN];
	char email[STRLEN-12];
	unsigned int nummails;
	time_t lastjustify;
	char gender; //性别
	unsigned char birthyear; //出生年
	unsigned char birthmonth; //出生月
	unsigned char birthday; //出生日
	int signature; //签名档数目
	unsigned int userdefine;
	time_t notedate;
	int noteline;
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
#ifndef BBSD
	char tty[ 20 ]; /* tty port */
#else
	time_t idle_time; /* to keep idle time */
#endif		
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
	time_t timeDeleted;
};

//added by cometcaptor 2007-04-21 修改.goodbrd的数据结构
struct goodbrdheader {
	int id; //自身ID
	int pid; //父ID
	int pos;
	unsigned int flag;
	char filename[STRLEN - 8];
	char title[STRLEN];
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
	struct user_info uinfo[ USHM_SIZE ];
	//    time_t		      uptime;
	int usersum;
	int max_login_num;
	int total_num;
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
	int number;
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

//Added by IAMFAT 2002.11.02 For KeyWord Filter
#ifdef CERTIFYMODE
struct KEYWORDS_SHM {
	char word[MAXKEYWORDS][80];
	int number;
	//time_t uptime;
};
#endif

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
#define MAX_CONTENT_LENGTH 555
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
