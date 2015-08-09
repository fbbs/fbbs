#ifndef FB_LOG_H
#define FB_LOG_H

extern void log_internal_err(const char *format, ...);
extern void log_internal_warn(const char *format, ...);
extern void log_internal_info(const char *format, ...);
extern void log_internal_dbg(const char *format, ...);
extern void log_internal_verbose(const char *format, ...);
extern void log_internal_info_legacy(const char *format, ...);

#define log_err(format, ...) log_internal_err("E/" format, ##__VA_ARGS__)
#define log_warn(format, ...) log_internal_warn("W/" format, ##__VA_ARGS__)
#define log_info(format, ...) log_internal_info("I/" format, ##__VA_ARGS__)
#define log_dbg(format, ...) log_internal_dbg("D/" format, ##__VA_ARGS__)
#define log_verbose(format, ...) log_internal_verbose("V/" format, ##__VA_ARGS__)

typedef enum {
	LOG_BM_STAYTIME = 0, ///< 停留时间
	LOG_BM_INBOARD = 1,	///< 进版
	LOG_BM_POST = 2, ///< 版内发文
	LOG_BM_DIGIST = 3, ///< 收入文摘
	LOG_BM_UNDIGIST = 4, ///< 去掉文摘
	LOG_BM_MARK = 5, ///< 标记m文
	LOG_BM_UNMARK = 6, ///< 去掉m标记
	LOG_BM_WATER = 7, ///< 标记水文
	LOG_BM_UNWATER = 8, ///< 去掉水文
	LOG_BM_CANNOTRE	= 9, ///< 标记不可re
	LOG_BM_UNCANNOTRE = 10, ///< 去掉不可re
	LOG_BM_DELETE = 11, ///< 删除文章(d/B/L/D)
	LOG_BM_UNDELETE = 12, ///< 恢复删除(y/B/L)
	LOG_BM_DENYPOST = 13, ///< 封禁
	LOG_BM_UNDENY = 14, ///< 解封
	LOG_BM_ADDCLUB = 15, ///< 加入俱乐部
	LOG_BM_DELCLUB = 16, ///< 取消俱乐部
	LOG_BM_ANNOUNCE = 17, ///< 收入精华
	LOG_BM_DOANN = 18, ///< 整理精华
	LOG_BM_COMBINE = 19, ///< 合集文章
	LOG_BM_RANGEANN	= 20, ///< L或者B收入精华
	LOG_BM_RANGEDEL = 21, ///< L或者B删除
	LOG_BM_RANGEOTHER = 23, ///< L或者B其他操作

	LOG_BM_LEN = 32, ///< 记录长度
} log_bm_e;

extern int log_bm(log_bm_e type, int value);

#endif // FB_LOG_H
