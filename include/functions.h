#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#ifdef FDQUAN
#define ENABLE_SHOWONLINE
#endif
#define ENABLE_NOTICE	//置底功能
#define IP_2_NAME //穿梭IP显示域名，同时限制其使用匿名发文功能
// 下面是一些功能的控制，如果屏蔽该控制，则表示不使用这个功能
// 如果该行前面有 "//" 则表示被屏蔽，要屏蔽该功能，则加入 "//"
#ifdef FDQUAN
#define ENABLE_PREFIX
#endif

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

/* 某些限制性代码的相关 define */
#define CHECK_FREQUENTLOGIN /* 频繁登录检查 */

/* 一般不需要变更的 define */
//#define SHOWMETOFRIEND		/* 环顾四方的是否为对方好友的显示 */
#define ALLOWAUTOWRAP		/* 启用自动排版功能 */
#define ALLOWSWITCHCODE		/* 启用 GB 码 <==> Big5 码 切换 */
#define USE_NOTEPAD		/* 使用留言板 */
#define INTERNET_EMAIL		/* 发送 InterNet Mail */
#define COLOR_POST_DATE 	/* 文章日期颜色 */
#define TALK_LOG 		/* 聊天纪录功能 */
#define RNDSIGN			/* 乱数签名档 */

/* 看站长的喜好啦， 想如何就如何吧， 当然你要懂这个啦，呵呵 */
#define LOG_MY_MESG		/* 讯息纪录中纪录自己所发出的讯息 */
#define BIGGER_MOVIE 		/* 加大活动看板空间 (七行) */

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

#endif 
/* _FUNCTIONS_H_ */
