#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#ifdef FDQUAN
#define ENABLE_SHOWONLINE
#endif
#define ENABLE_NOTICE	//置底功能
#define IP_2_NAME //穿梭IP显示域名，同时限制其使用匿名发文功能
#define NEWONLINECOUNT
// 下面是一些功能的控制，如果屏蔽该控制，则表示不使用这个功能
// 如果该行前面有 "//" 则表示被屏蔽，要屏蔽该功能，则加入 "//"
#ifdef FDQUAN
#define ENABLE_PREFIX
#endif
#define SHOW_THANKYOU		/* 显示源代码提供者信息 */

/*  注册相关部分代码的 define */
/*  一鼓作气把注册这部分做了一些变更， 这样可能可以满足比较多朋友的需要 */
#define NEWCOMERREPORT		/* 新手上路在 newcomers 版自动发文 */
//#define MAILCHECK		/* 提供邮件注册功能 */
//#define CODE_VALID 		/* 暗码认证 */
//#define SAVELIVE		/* 锁定帐号 防止用户长时间不上线而死亡*/
//#define AUTOGETPERM		/* 无需注册即获取基本权限 */
//#define PASSAFTERTHREEDAYS	/* 新手上路三天限制 */
//#define MAILCHANGED		/* 修改 e-mail 后要求重新注册确认 */

/* 游戏相关代码的 define */
#ifdef FDQUAN
#define ALLOWGAME		/* 支持游戏, 提供金钱显示 */
//#define FIVEGAME 		/* 五子棋 */
#endif
/* 系统安全相关代码的 define */
//#define MUDCHECK_BEFORELOGIN 	// 登陆前按键确认 
//#define AddWaterONLY		// 仅仅是快意灌水站自己使用的代码，请屏蔽

/* 某些限制性代码的相关 define */
//#define KEEP_DELETED_HEADER 	/* 保留删除文章记录 */
//#define BELL_DELAY_FILTER	/* 过滤文章中的响铃和延时控制 */
#define CHECK_FREQUENTLOGIN /* 频繁登录检查 */
//#define MARK_X_FLAG		/* 将灌水文章加上 'X' 标记 */

/* 一般不需要变更的 define */
//#define SHOWMETOFRIEND		/* 环顾四方的是否为对方好友的显示 */
#define BBSD 			/* 使用 BBS daemon, 不使用 bbsrf */
#define ALLOWAUTOWRAP		/* 启用自动排版功能 */
#define ALLOWSWITCHCODE		/* 启用 GB 码 <==> Big5 码 切换 */
#define USE_NOTEPAD		/* 使用留言板 */
#define INTERNET_EMAIL		/* 发送 InterNet Mail */
#define CHKPASSWDFORK           /* 系统检查密码时采用 fork 进行 */
#define COLOR_POST_DATE 	/* 文章日期颜色 */
#define TALK_LOG 		/* 聊天纪录功能 */
#define RNDSIGN			/* 乱数签名档 */
//#define DOMAIN_NAME		/* 登陆时进行域名反查 */

/* 看站长的喜好啦， 想如何就如何吧， 当然你要懂这个啦，呵呵 */
//#define MSG_CANCLE_BY_CTRL_C 	/* 用 ctrl-c 来否略讯息 */
#define LOG_MY_MESG		/* 讯息纪录中纪录自己所发出的讯息 */
#define BIGGER_MOVIE 		/* 加大活动看板空间 (七行) */
//#define ALWAYS_SHOW_BRDNOTE 	/* 每次进版都会 show 出进版画面 */

/* 下面的是准备取消的功能， 你看着办吧 */
/*
 系统寻人功能是当初没有好友上站通知功能时做的一个寻人功能。
 而且容易其提示信息极容易被忽略，而且忽略后就再也找不到了。
 我曾想完善这个功能，为了保存信息，想到了同时发一份信给对方，
 这样一来，这个功能也就是多余的了，因为写信本身就是一个很好
 的寻人功能。 :) 所以我把他缺省为取消了。
 不过，如果你想要这个功能，就自己打开吧，呵呵。
 */
//#define CHK_FRIEND_BOOK		/* 设定系统寻人名单 */

#define USE_TRY extern sigjmp_buf bus_jump
#define BBS_TRY \
    	if (!sigsetjmp(bus_jump, 1)) { \
        	signal(SIGBUS, sigbus);

#define BBS_CATCH \
	} \
	else { \

#define BBS_END } \
	signal(SIGBUS, SIG_IGN);

#define BBS_RETURN(x) {signal(SIGBUS, SIG_IGN);return (x);}
#define BBS_RETURN_VOID {signal(SIGBUS, SIG_IGN);return;}

void sigbus(int signo);
int safe_mmapfile(char *filename, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size, int *ret_fd);
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size);
void end_mmapfile(void *ptr, int size, int fd);

typedef int (*RECORD_FUNC_ARG)(void *, void *);
typedef int (*APPLY_FUNC_ARG)(void *, int, void *);

#endif 
/* _FUNCTIONS_H_ */
