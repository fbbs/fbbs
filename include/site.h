#ifndef FB_SITE_H
#define FB_SITE_H

#include "config.h"

#ifdef ENABLE_FDQUAN
# define FDQUAN	//定义复旦泉用
#endif // ENABLE_FDQUAN

// 板主任命时， 如果希望任命文章不仅仅发送到被任命版面，而且同时发送到
// sysop 板，或其他版面，你可以打开下面的定义。其中的 sysop 可以改成你
// 希望发表的版面。注意：这个版面一定要是存在的。
//#define ORDAINBM_POST_BOARDNAME "SysOp"

// 允许开启外部程序的用户数目。 比如：BBSNET、WINMINE、SHOWUSER 等
#ifdef FDQUAN
	#define MAX_USESHELL    700
#else
	#define MAX_USESHELL    250
#endif

#ifdef FDQUAN
/**
 * You may define a default board here, already selected when a user logs in.
 * If this is set to the empty string, the bbs will require users to (S)elect
 * a board before reading or posting. Don't comment this out.
 */
	#define DEFAULTBOARD    "SysOp"
#else
	#define DEFAULTBOARD    "BBS_Help"
#endif

/**
 * You get this many chances to give a valid userid/password combination
 * before the bbs squawks at you and closes the connection.
 * Don't comment out.
 */
#define LOGINATTEMPTS 3

/**
 * Turn this on to allow users to create their own accounts by typing 'new'
 * at the "Enter userid:" prompt. Comment out to restrict access to accounts
 * created by the Sysop (see important note in README.install).
 */
#define LOGINASNEW 1

/**
 * Defined means when BM delete a post, the posting record of the owner will
 * decrease by 1
 */
#define BMDEL_DECREASE 1

/* 
   Define this if you want the Internet Post/Mail Forwarding features
   enabled. You MUST have the sendmail(8) program on your system and
   a cron job to run the bbs mail queue every so often. The bbs does not
   invoke sendmail, it simply creates the queue and data files for it. 
*/

/* Move this define to functions.h 
#define INTERNET_EMAIL 1
*/

/* 
   Define DOTIMEOUT to set a timer to log out users who sit idle on the system.
   Then decide how long to let them stay: MONITOR_TIMEOUT is the time in
   seconds a user can sit idle in Monitor mode; IDLE_TIMEOUT applies to all
   other modes. 
*/
#define DOTIMEOUT 1

/* 
   These are moot if DOTIMEOUT is commented; leave them defined anyway. 
*/
#define IDLE_TIMEOUT    (60*15) 	//空闲时间,十五分钟
#define LOGIN_TIMEOUT   (60*3)		//登陆时空闲,三分钟

/* 
   By default, users with post permission can reply to a post right after
   they read it. This can lead to lots of silly replies, hence the reply
   feature is a little controversial. Uncomment this to disable Reply. 
*/
/*
#define NOREPLY 1 
*/

#ifdef FDQUAN
	#define MAXUSERS  25000  /* Maximum number of users,780 aliament 1k*/
	#define MAXBOARD  1000  /* Maximum number of boards */
	#define MAXACTIVE 1000
							  /* Max users allowed on the system at once. Set this
								 to a reasonable value based on the resources of
								 your system. */
	#define MAXSIGLINES    6  /* max. # of lines appended for post signature */
	#define MAXQUERYLINES 16  /* max. # of lines shown by the Query function */
	#define MAX_MAIL_HOLD		(20000)//(60)
	#define MAX_BMMAIL_HOLD		(20000)//(120)
	#define MAX_SYSOPMAIL_HOLD	(20000)//(300)
	#define MAILBOX_SIZE_SYSOP	(10000)
	#define MAILBOX_SIZE_BM		(5000)
	#define MAILBOX_SIZE_NORMAL	(2000)
	#define MAILBOX_SIZE_LARGE	(10000)
#else
	#define MAXUSERS  100000  /* Maximum number of users,780 aliament 1k*/
	#define MAXBOARD  1000  /* Maximum number of boards */
	#define MAXACTIVE 10000
							  /* Max users allowed on the system at once. Set this
								 to a reasonable value based on the resources of
								 your system. */

	#define MAXSIGLINES    6  /* max. # of lines appended for post signature */
	#define MAXQUERYLINES 16  /* max. # of lines shown by the Query function */
	#define MAX_MAIL_HOLD           (20000)//(200)
	#define MAX_BMMAIL_HOLD         (20000)//(500)
	#define MAX_SYSOPMAIL_HOLD      (50000)//(5000)
    #define MAILBOX_SIZE_SYSOP      (50000)
    #define MAILBOX_SIZE_BM         (10000)
    #define MAILBOX_SIZE_NORMAL     (5000)
    #define MAILBOX_SIZE_LARGE      (50000)
#endif
/* Once you set this, do not change it unless you are restarting your
   bbs from scratch! The PASSWDS file will not work if this is changed.
   If commented out, no real name/address info can be kept in the passwd
   file. Pretty useless to have this if LOGINASNEW is defined. */

/**
 * Define this to stamp mark on edited post
 */
#define ADD_EDITMARK  1

/*
Define this if you have Sendmail 8.8.x that supports MIME autoconvert
You must also set "EightBitMode=pass8" in /etc/sendmail.cf
*/
#define SENDMAIL_MIME_AUTOCONVERT    1

/*
Define using libcrypt type, make it static since you choice you want it
there're common usage "DES" and default for FreeBSD "MD5"
*/
#define DES

/*
Well, there's defined BBS related information that from configure
so we don't need define this by passing make argument to the complier
*/
#ifdef FDQUAN
	#define BBSUID   9999
	#define BBSGID   9999
	#define BBSHOME "/home/bbs"
	#define BBSNAME_UTF8 "复旦泉"
	#define BBSNAME "\xb8\xb4\xb5\xa9\xc8\xaa"
	//% "泉站"
	#define SHORT_BBSNAME  "\xc8\xaa\xd5\xbe"
	#define BBSID   "FDQuanBBS"
	#define BBSHOST "10.8.225.9"
	#define BBSHOST_PUBLIC "bbs.fudan.edu.cn/q"
	#define BBSIP	"10.8.225.9"
#else
	#define BBSUID  9999 
	#define BBSGID  9999
	#define BBSHOME "/home/bbs"
	#define BBSNAME_UTF8 "日月光华"
	#define BBSNAME "\xc8\xd5\xd4\xc2\xb9\xe2\xbb\xaa"
	//% "光华"
	#define SHORT_BBSNAME  "\xb9\xe2\xbb\xaa"
	#define BBSID   "Fudan BBS"
	#define BBSHOST "bbs.fudan.edu.cn"
	#define BBSHOST_PUBLIC BBSHOST
	#define BBSIP	"61.129.42.9"
#endif

#define	MTA "/usr/sbin/ssmtp"   ///< Mail Transfer Agent

#endif // FB_SITE_H
