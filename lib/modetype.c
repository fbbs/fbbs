#include "config.h"
#include "modes.h"
#include <stdbool.h>

/**
 * Get descriptions of user mode.
 * @param mode user mode.
 * @return a string describing the mode.
 */
const char *mode_type(int mode)
{
	switch (mode & 0x3fffffff) {
		case IDLE:
			return "无所事事";
		case NEW:
			return "新站友注册";
		case LOGIN:
			return "进入本站";
		case DIGEST:
			return "汲取精华";
		case MMENU:
			return "游大街";
		case ADMIN:
			return "修路铺桥";
		case SELECT:
			return "选择讨论区";
		case READBRD:
			return "览遍天下";
		case READNEW:
			return "览新文章";
		case READING:
			return "品味文章";
		case POSTING:
			return "文豪挥笔";
		case MAIL:
			return "处理信笺";
		case SMAIL:
			return "寄语信鸽";
		case RMAIL:
			return "阅览信笺";
		case TMENU:
			return "上鹊桥";
		case LUSERS:
			return "环顾四方";
		case FRIEND:
			return "夜探好友";
		case MONITOR:
			return "探视民情";
		case QUERY:
			return "查询网友";
		case TALK:
			return "鹊桥细语";
		case PAGE:
			return "巴峡猿啼";
		case CHAT2:
			return "燕园夜话";
		case CHAT1:
			return "燕园夜话";
		case LAUSERS:
			return "探视网友";
		case XMENU:
			return "系统资讯";
		case BBSNET:
#ifdef FDQUAN
			return "有泉穿梭";
#else
			return "饮复旦泉";
#endif
		case EDITUFILE:
			return "编辑个人档";
		case EDITSFILE:
			return "动手动脚";
		case SYSINFO:
			return "检查系统";
		case DICT:
			return "翻查字典";
		case LOCKSCREEN:
			return "屏幕锁定";
		case NOTEPAD:
			return "留言板";
		case GMENU:
			return "工具箱";
		case MSG:
			return "送讯息";
		case USERDEF:
			return "自订参数";
		case EDIT:
			return "修改文章";
		case OFFLINE:
			return "自杀中..";
		case EDITANN:
			return "编修精华";
		case LOOKMSGS:
			return "查看讯息";
		case WFRIEND:
			return "寻人名册";
		case WNOTEPAD:
			return "欲走还留";
		case BBSPAGER:
			return "网路传呼";
		case M_BLACKJACK:
			return "★黑甲克★";
		case M_XAXB:
			return "★猜数字★";
		case M_DICE:
			return "★西八拉★";
		case M_GP:
			return "金扑克梭哈";
		case M_NINE:
			return "天地九九";
		case WINMINE:
			return "键盘扫雷";
		case M_BINGO:
			return "宾果宾果";
		case FIVE:
			return "决战五子棋";
		case MARKET:
			return "交易市场";
		case PAGE_FIVE:
			return "邀请下棋";
		case CHICK:
			return "电子小鸡";
		case MARY:
			return "超级玛丽";
		case CHICKEN:
			return "星空战斗鸡";
		case GOODWISH:
			return "给朋友祝福";
		case GIVEUPBBS:
			return "戒网中";
		case BBSST_UPLOAD:
			return "上传文件";
		default:
			return "去了哪里!?";
	}
}

/**
 * Get whether user is web browsing.
 * @param mode user mode.
 * @return true if web browsing, false otherwise.
 */
bool is_web_user(int mode)
{
	return (mode & WWW);
}

/**
 * Get web mode.
 * @param mode user mode.
 * @return correspoding web mode.
 */
int get_web_mode(int mode)
{
	return (mode | WWW);
}
