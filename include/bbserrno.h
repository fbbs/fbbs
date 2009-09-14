#ifndef FB_BBSERRNO_H
#define FB_BBSERRNO_H

enum {
	BBS_EINVAL  = -1,  ///< Invalid argument.
	BBS_ELGNREQ = -2,  ///< Login required.
	BBS_EACCES  = -3,  ///< Permission denied.
	BBS_EPST    = -4,  ///< Post request denied.
	BBS_ENOFILE = -5,  ///< File not found.
	BBS_ENODIR  = -6,  ///< Directory not found.
	BBS_ENOBRD  = -7,  ///< Board not found.
	BBS_ENOUSR  = -8,  ///< User not found.
	BBS_ENOURL  = -9,  ///< URL not found.
	BBS_EDUPLGN = -10, ///< Duplicate web logins.
	BBS_EWPSWD  = -11, ///< Wrong password.
	BBS_EBLKLST = -12, ///< Blacklisted by recipient.
	BBS_ELGNQE  = -13, ///< Login quota exceeded.
	BBS_EBRDQE  = -14, ///< Favorite boards quota exceeded.
	BBS_EATTQE  = -15, ///< Attachment quota exceeded.
	BBS_EMAILQE = -16, ///< Mail quota exceeded.
	BBS_EFRNDQE = -17, ///< Friends quota exceeded.
	BBS_EFBIG   = -18, ///< File too big.
	BBS_ELFREQ  = -19, ///< Login too frequent.
	BBS_EPFREQ  = -20, ///< Post too frequent.
	BBS_E2MANY  = -21, ///< Too many online users.
	BBS_EINTNL  = -22, ///< Internal Error.
};

#endif // FB_BBSERRNO_H

