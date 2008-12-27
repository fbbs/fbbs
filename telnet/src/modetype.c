/*
 Pirate Bulletin Board System
 Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
 Eagles Bulletin Board System
 Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
 Guy Vega, gtvega@seabass.st.usm.edu
 Dominic Tynes, dbtynes@seabass.st.usm.edu
 Firebird Bulletin Board System
 Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
 Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */
#include "config.h"
#include "modes.h"

// 返回模式mode所对应的中文名称
char *ModeType(int mode) {
	switch (mode) {
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
			return "燕圆夜话";
		case LAUSERS:
			return "探视网友";
		case XMENU:
			return "系统资讯";
		case VOTING:
			return "投票";
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
		case GAME:
			return "脑力激汤";
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
			return "察看讯息";
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
			/*2003.04.22 added by stephen*/
		case GIVEUPBBS:
			return "戒网中";
			/*2003.04.22 stephen add end*/
			/* added by roly */
		case 10001:
			return "WWW浏览";
		case 10002:
			return "JABBER";
			/* added end */
		default:
			return "去了那里!?";
	}
}
