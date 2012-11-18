#ifndef FB_STRUCT_H
#define FB_STRUCT_H

#include "config.h"
#include "fbbs/time.h"

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
	fb_time_t firstlogin;     ///< time of first login.
	fb_time_t lastlogin;      ///< time of last login.
	fb_time_t lastlogout;     ///< time of last logout.
	fb_time_t dateforbet;     ///< loan deadline.
	// TODO: remove notedate
	int64_t notedate;         ///< will be removed soon.
	char userid[EXT_IDLEN];   ///< userid.
	char lasthost[IP_LEN];    ///< last login IP address.
	char username[NAMELEN];   ///< nick.
	char email[EMAIL_LEN];    ///< email address.
	char reserved[8];         ///< reserved for future use.
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

struct postheader {
	bool mail_owner;
	bool reply;
	bool anonymous;
	bool locked;
	char title[STRLEN];
	char ds[40]; //board
	char include_mode;
	int postboard;
	char prefix[10];
};

#endif //FB_STRUCT_H
