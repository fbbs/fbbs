#include "site.h"
#include <stdbool.h>
#include "fbbs/session.h"

/**
 * Get descriptions of user mode.
 * @param mode user mode.
 * @return a string describing the mode.
 */
const char *status_descr(int status)
{
	switch (status) {
		case ST_IDLE:
			return "无所事事";
		case ST_NEW:
			return "新站友注册";
		case ST_LOGIN:
			return "进入本站";
		case ST_DIGEST:
			return "汲取精华";
		case ST_MMENU:
			return "游大街";
		case ST_ADMIN:
			return "修路铺桥";
		case ST_SELECT:
			return "选择讨论区";
		case ST_READBRD:
			return "览遍天下";
		case ST_READNEW:
			return "览新文章";
		case ST_READING:
			return "品味文章";
		case ST_POSTING:
			return "文豪挥笔";
		case ST_MAIL:
			return "处理信笺";
		case ST_SMAIL:
			return "寄语信鸽";
		case ST_RMAIL:
			return "阅览信笺";
		case ST_TMENU:
			return "上鹊桥";
		case ST_LUSERS:
			return "环顾四方";
		case ST_FRIEND:
			return "夜探好友";
		case ST_MONITOR:
			return "探视民情";
		case ST_QUERY:
			return "查询网友";
		case ST_TALK:
			return "鹊桥细语";
		case ST_PAGE:
			return "巴峡猿啼";
		case ST_CHAT2:
			return "燕园夜话";
		case ST_CHAT1:
			return "燕园夜话";
		case ST_LAUSERS:
			return "探视网友";
		case ST_XMENU:
			return "系统资讯";
		case ST_BBSNET:
#ifdef FDQUAN
			return "有泉穿梭";
#else
			return "饮复旦泉";
#endif
		case ST_EDITUFILE:
			return "编辑个人档";
		case ST_EDITSFILE:
			return "动手动脚";
		case ST_SYSINFO:
			return "检查系统";
		case ST_DICT:
			return "翻查字典";
		case ST_LOCKSCREEN:
			return "屏幕锁定";
		case ST_NOTEPAD:
			return "留言板";
		case ST_GMENU:
			return "工具箱";
		case ST_MSG:
			return "送讯息";
		case ST_USERDEF:
			return "自订参数";
		case ST_EDIT:
			return "修改文章";
		case ST_OFFLINE:
			return "自杀中..";
		case ST_EDITANN:
			return "编修精华";
		case ST_LOOKMSGS:
			return "查看讯息";
		case ST_WFRIEND:
			return "寻人名册";
		case ST_WNOTEPAD:
			return "欲走还留";
		case ST_BBSPAGER:
			return "网路传呼";
		case ST_M_BLACKJACK:
			return "★黑甲克★";
		case ST_M_XAXB:
			return "★猜数字★";
		case ST_M_DICE:
			return "★西八拉★";
		case ST_M_GP:
			return "金扑克梭哈";
		case ST_M_NINE:
			return "天地九九";
		case ST_WINMINE:
			return "键盘扫雷";
		case ST_M_BINGO:
			return "宾果宾果";
		case ST_FIVE:
			return "决战五子棋";
		case ST_MARKET:
			return "交易市场";
		case ST_PAGE_FIVE:
			return "邀请下棋";
		case ST_CHICK:
			return "电子小鸡";
		case ST_MARY:
			return "超级玛丽";
		case ST_CHICKEN:
			return "星空战斗鸡";
		case ST_GOODWISH:
			return "给朋友祝福";
		case ST_GIVEUPBBS:
			return "戒网中";
		case ST_UPLOAD:
			return "上传文件";
		case ST_PROP:
			return "聚宝盆";
		case ST_MY_PROP:
			return "藏经阁";
		default:
			return "去了哪里!?";
	}
}
