/* 快意灌水站 交易市场代码 1999.12.19 */
#include <string.h>
#include <stdlib.h>

#include "bbs.h"
#define MAXBET 5000
typedef struct exchanges {
	char	title[STRLEN];
	int	value;
	char	information[STRLEN];
} EXCHANGES;
int gotomarket(char *title)
{
        if (!strcmp("guest", currentuser.userid)) return 1;
        set_user_status(ST_MARKET);
        clear();
        set_safe_record();
        move(2,0);
        //% prints("欢迎进入 [[32m%s[37m]....\n\n",title);
        prints("\xbb\xb6\xd3\xad\xbd\xf8\xc8\xeb [[32m%s[37m]....\n\n",title);
	return 0;
}

int lending()
{
        int     id, canlending=0,maxnum = 0, num = 0;
	char	ans[STRLEN];
	time_t 	now=time(0);
	extern int gettheuserid();
	//% if(gotomarket("交易市场")) return 0;
	if(gotomarket("\xbd\xbb\xd2\xd7\xca\xd0\xb3\xa1")) return 0;
	maxnum = currentuser.money - currentuser.bet - 1000;
	//% prints("欢迎使用[0;1;33m"BBSNAME"[37m交易市场[32m友情转帐[37m功能[m");
	prints("\xbb\xb6\xd3\xad\xca\xb9\xd3\xc3[0;1;33m"BBSNAME"[37m\xbd\xbb\xd2\xd7\xca\xd0\xb3\xa1[32m\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca[37m\xb9\xa6\xc4\xdc[m");
	//% prints("\n\n您目前的情况是：\n注册天数([32m%d[37m 天) 上站总时数([32m%d[37m 小时) [44;37m可转帐资金([32m%d[37m 元)[m[37m",
	prints("\n\n\xc4\xfa\xc4\xbf\xc7\xb0\xb5\xc4\xc7\xe9\xbf\xf6\xca\xc7\xa3\xba\n\xd7\xa2\xb2\xe1\xcc\xec\xca\xfd([32m%d[37m \xcc\xec) \xc9\xcf\xd5\xbe\xd7\xdc\xca\xb1\xca\xfd([32m%d[37m \xd0\xa1\xca\xb1) [44;37m\xbf\xc9\xd7\xaa\xd5\xca\xd7\xca\xbd\xf0([32m%d[37m \xd4\xaa)[m[37m",
		(now - currentuser.firstlogin)/86400,currentuser.stay/3600,currentuser.money-currentuser.bet-1000);
	if ( currentuser.stay <= 36000 || now - currentuser.firstlogin  <= 2592000 || maxnum <= 0 ) {
		 //% prints("\n\n目前市场规定： 参与[32m友情转帐[m的 ID 必须具备以下全部条件：\n    1. 本帐号注册天数超过 30 天;\n    2. 总上站时数超过 10 小时;\n    3. 最终拥有存款超过 1000 元.\n      (注：指存款减去贷款后的差值.)");
		 prints("\n\n\xc4\xbf\xc7\xb0\xca\xd0\xb3\xa1\xb9\xe6\xb6\xa8\xa3\xba \xb2\xce\xd3\xeb[32m\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca[m\xb5\xc4 ID \xb1\xd8\xd0\xeb\xbe\xdf\xb1\xb8\xd2\xd4\xcf\xc2\xc8\xab\xb2\xbf\xcc\xf5\xbc\xfe\xa3\xba\n    1. \xb1\xbe\xd5\xca\xba\xc5\xd7\xa2\xb2\xe1\xcc\xec\xca\xfd\xb3\xac\xb9\xfd 30 \xcc\xec;\n    2. \xd7\xdc\xc9\xcf\xd5\xbe\xca\xb1\xca\xfd\xb3\xac\xb9\xfd 10 \xd0\xa1\xca\xb1;\n    3. \xd7\xee\xd6\xd5\xd3\xb5\xd3\xd0\xb4\xe6\xbf\xee\xb3\xac\xb9\xfd 1000 \xd4\xaa.\n      (\xd7\xa2\xa3\xba\xd6\xb8\xb4\xe6\xbf\xee\xbc\xf5\xc8\xa5\xb4\xfb\xbf\xee\xba\xf3\xb5\xc4\xb2\xee\xd6\xb5.)");
		//% prints("\n\n根据市场规定，您目前尚没有[32m友情转帐[m的资格。 :P \n");
		prints("\n\n\xb8\xf9\xbe\xdd\xca\xd0\xb3\xa1\xb9\xe6\xb6\xa8\xa3\xac\xc4\xfa\xc4\xbf\xc7\xb0\xc9\xd0\xc3\xbb\xd3\xd0[32m\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca[m\xb5\xc4\xd7\xca\xb8\xf1\xa1\xa3 :P \n");
		pressanykey();
		return 0;
	}
        //% if (!gettheuserid(9,"您想转帐到谁的帐户上？请输入他的帐号: ",&id))
        if (!gettheuserid(9,"\xc4\xfa\xcf\xeb\xd7\xaa\xd5\xca\xb5\xbd\xcb\xad\xb5\xc4\xd5\xca\xbb\xa7\xc9\xcf\xa3\xbf\xc7\xeb\xca\xe4\xc8\xeb\xcb\xfb\xb5\xc4\xd5\xca\xba\xc5: ",&id))
                return 0;
	if(!strcmp(currentuser.userid,lookupuser.userid)) {
		//% prints("\n呵呵，转帐给自己啊？ 嗯，也行。不过本站不提供这个服务。");
		prints("\n\xba\xc7\xba\xc7\xa3\xac\xd7\xaa\xd5\xca\xb8\xf8\xd7\xd4\xbc\xba\xb0\xa1\xa3\xbf \xe0\xc5\xa3\xac\xd2\xb2\xd0\xd0\xa1\xa3\xb2\xbb\xb9\xfd\xb1\xbe\xd5\xbe\xb2\xbb\xcc\xe1\xb9\xa9\xd5\xe2\xb8\xf6\xb7\xfe\xce\xf1\xa1\xa3");
		pressanykey();
		return 0;
	}
        if( lookupuser.money+lookupuser.nummedals*1000 > 90000 ) {
                //% prints("\n对不起，对方目前经济能力尚不需要您的转帐！");
                prints("\n\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xb6\xd4\xb7\xbd\xc4\xbf\xc7\xb0\xbe\xad\xbc\xc3\xc4\xdc\xc1\xa6\xc9\xd0\xb2\xbb\xd0\xe8\xd2\xaa\xc4\xfa\xb5\xc4\xd7\xaa\xd5\xca\xa3\xa1");
                pressanykey();
                return 0;
        }
	move(10,0);
	canlending = maxnum > 90000 ? 90000 : maxnum;
	//% prints("您将转帐到 [1;32m%s[m 的帐号，您可以最多可以转帐 [1;33m%d[m 元。",lookupuser.userid, canlending);
	prints("\xc4\xfa\xbd\xab\xd7\xaa\xd5\xca\xb5\xbd [1;32m%s[m \xb5\xc4\xd5\xca\xba\xc5\xa3\xac\xc4\xfa\xbf\xc9\xd2\xd4\xd7\xee\xb6\xe0\xbf\xc9\xd2\xd4\xd7\xaa\xd5\xca [1;33m%d[m \xd4\xaa\xa1\xa3",lookupuser.userid, canlending);
        //% getdata(12, 0, "确认要转帐，请输入转帐数目，否则，请直接回车取消转帐: ",ans, 6, DOECHO, YEA);
        getdata(12, 0, "\xc8\xb7\xc8\xcf\xd2\xaa\xd7\xaa\xd5\xca\xa3\xac\xc7\xeb\xca\xe4\xc8\xeb\xd7\xaa\xd5\xca\xca\xfd\xc4\xbf\xa3\xac\xb7\xf1\xd4\xf2\xa3\xac\xc7\xeb\xd6\xb1\xbd\xd3\xbb\xd8\xb3\xb5\xc8\xa1\xcf\xfb\xd7\xaa\xd5\xca: ",ans, 6, DOECHO, YEA);
        num = atoi(ans);
        if ( num <= 0 || num > canlending ) {
                //% prints("\n输入有错误哦。 还是算了吧。。。");
                prints("\n\xca\xe4\xc8\xeb\xd3\xd0\xb4\xed\xce\xf3\xc5\xb6\xa1\xa3 \xbb\xb9\xca\xc7\xcb\xe3\xc1\xcb\xb0\xc9\xa1\xa3\xa1\xa3\xa1\xa3");
                pressanykey();
                return 0;
        }
	set_safe_record();
	if(currentuser.money - currentuser.bet - 1000 != maxnum) {
		//% prints("\n对不起，您的可转帐资金有所变化，取消此次交易！");
		prints("\n\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xc4\xfa\xb5\xc4\xbf\xc9\xd7\xaa\xd5\xca\xd7\xca\xbd\xf0\xd3\xd0\xcb\xf9\xb1\xe4\xbb\xaf\xa3\xac\xc8\xa1\xcf\xfb\xb4\xcb\xb4\xce\xbd\xbb\xd2\xd7\xa3\xa1");
		//% prints("\n请重新执行本交易。");
		prints("\n\xc7\xeb\xd6\xd8\xd0\xc2\xd6\xb4\xd0\xd0\xb1\xbe\xbd\xbb\xd2\xd7\xa1\xa3");
		pressanykey();
		return 0;
	}
 	currentuser.money -= num;
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser),usernum);
	lookupuser.money += num;
	substitut_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	//% prints("\n谢谢您，您已经给 [1;32m%s[m 转帐 [1;33m%d[m 元。",lookupuser.userid,num);
	prints("\n\xd0\xbb\xd0\xbb\xc4\xfa\xa3\xac\xc4\xfa\xd2\xd1\xbe\xad\xb8\xf8 [1;32m%s[m \xd7\xaa\xd5\xca [1;33m%d[m \xd4\xaa\xa1\xa3",lookupuser.userid,num);
	//% prints("\n为表示对你的转帐行为的感谢，本站已经用信件通知了他。");
	prints("\n\xce\xaa\xb1\xed\xca\xbe\xb6\xd4\xc4\xe3\xb5\xc4\xd7\xaa\xd5\xca\xd0\xd0\xce\xaa\xb5\xc4\xb8\xd0\xd0\xbb\xa3\xac\xb1\xbe\xd5\xbe\xd2\xd1\xbe\xad\xd3\xc3\xd0\xc5\xbc\xfe\xcd\xa8\xd6\xaa\xc1\xcb\xcb\xfb\xa1\xa3");
	//% sprintf(genbuf,"给 %s 转帐 %d 元",lookupuser.userid,num);
	sprintf(genbuf,"\xb8\xf8 %s \xd7\xaa\xd5\xca %d \xd4\xaa",lookupuser.userid,num);
	gamelog(genbuf);
	//% sprintf(ans,"%s 给您寄来了 %d 元友情转帐",currentuser.userid,num);
	sprintf(ans,"%s \xb8\xf8\xc4\xfa\xbc\xc4\xc0\xb4\xc1\xcb %d \xd4\xaa\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca",currentuser.userid,num);
	//% sprintf(genbuf,"\n %s :\n\t您好！\n\t您的朋友 %s 给您寄来了 %d 元友情转帐资金。\n\t请您在 Market 板对他的无私行为发文表示感谢，\n\t这样，您就可以获得这笔友情转帐资金。\n\n\t当然，您也可以退出一次后，再进入本站，\n\t一样可以获得这笔友情转帐资金。\n  ",lookupuser.userid,currentuser.userid,num);
	sprintf(genbuf,"\n %s :\n\t\xc4\xfa\xba\xc3\xa3\xa1\n\t\xc4\xfa\xb5\xc4\xc5\xf3\xd3\xd1 %s \xb8\xf8\xc4\xfa\xbc\xc4\xc0\xb4\xc1\xcb %d \xd4\xaa\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca\xd7\xca\xbd\xf0\xa1\xa3\n\t\xc7\xeb\xc4\xfa\xd4\xda Market \xb0\xe5\xb6\xd4\xcb\xfb\xb5\xc4\xce\xde\xcb\xbd\xd0\xd0\xce\xaa\xb7\xa2\xce\xc4\xb1\xed\xca\xbe\xb8\xd0\xd0\xbb\xa3\xac\n\t\xd5\xe2\xd1\xf9\xa3\xac\xc4\xfa\xbe\xcd\xbf\xc9\xd2\xd4\xbb\xf1\xb5\xc3\xd5\xe2\xb1\xca\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca\xd7\xca\xbd\xf0\xa1\xa3\n\n\t\xb5\xb1\xc8\xbb\xa3\xac\xc4\xfa\xd2\xb2\xbf\xc9\xd2\xd4\xcd\xcb\xb3\xf6\xd2\xbb\xb4\xce\xba\xf3\xa3\xac\xd4\xd9\xbd\xf8\xc8\xeb\xb1\xbe\xd5\xbe\xa3\xac\n\t\xd2\xbb\xd1\xf9\xbf\xc9\xd2\xd4\xbb\xf1\xb5\xc3\xd5\xe2\xb1\xca\xd3\xd1\xc7\xe9\xd7\xaa\xd5\xca\xd7\xca\xbd\xf0\xa1\xa3\n  ",lookupuser.userid,currentuser.userid,num);
	autoreport(ans,genbuf,NA,lookupuser.userid);
	pressanykey();
	return 1;
}

int popshop(void)
{
	int no,num,maxnum,templog;
	char ans[10];
	EXCHANGES exchanges[10] = {
		//% {"上站次数",2},
		{"\xc9\xcf\xd5\xbe\xb4\xce\xca\xfd",2},
		//% {"文章数",5},
		{"\xce\xc4\xd5\xc2\xca\xfd",5},
		//% {"奖章数",10000},
		{"\xbd\xb1\xd5\xc2\xca\xfd",10000},
		//% {"隐身术",16000},
		{"\xd2\xfe\xc9\xed\xca\xf5",16000},
		//% {"看穿隐身术",4500},
		{"\xbf\xb4\xb4\xa9\xd2\xfe\xc9\xed\xca\xf5",4500},
		//% {"帐号永久保留",45000},
		{"\xd5\xca\xba\xc5\xd3\xc0\xbe\xc3\xb1\xa3\xc1\xf4",45000},
		//% {"强制呼叫",54000}, //expired function 06.1.5
		{"\xc7\xbf\xd6\xc6\xba\xf4\xbd\xd0",54000}, //expired function 06.1.5
		//% {"延长发呆时间",9000},//expired function 06.1.5
		{"\xd1\xd3\xb3\xa4\xb7\xa2\xb4\xf4\xca\xb1\xbc\xe4",9000},//expired function 06.1.5
		//% {"大信箱",45000}
		{"\xb4\xf3\xd0\xc5\xcf\xe4",45000}
		};
	//% if(gotomarket("市场典当行")) return 1;
	if(gotomarket("\xca\xd0\xb3\xa1\xb5\xe4\xb5\xb1\xd0\xd0")) return 1;
	//% prints("今日典当报价:");
	prints("\xbd\xf1\xc8\xd5\xb5\xe4\xb5\xb1\xb1\xa8\xbc\xdb:");
	for(no = 0; no < 9; no ++) {
		move(5+no, 2);
		prints("(%2d) %s",no+1,exchanges[no].title);
		move(5+no, 20);
		//% prints("==> %6d 元", exchanges[no].value);
		prints("==> %6d \xd4\xaa", exchanges[no].value);
	}
	move(16,0);
	//% prints("您目前的情况是: (1)上站次数([32m%d[37m)  (2)文章数([32m%d[37m)  (3)奖章数.([32m%d[37m)\n",
	prints("\xc4\xfa\xc4\xbf\xc7\xb0\xb5\xc4\xc7\xe9\xbf\xf6\xca\xc7: (1)\xc9\xcf\xd5\xbe\xb4\xce\xca\xfd([32m%d[37m)  (2)\xce\xc4\xd5\xc2\xca\xfd([32m%d[37m)  (3)\xbd\xb1\xd5\xc2\xca\xfd.([32m%d[37m)\n",
	    currentuser.numlogins,currentuser.numposts,currentuser.nummedals);
	//% getdata(18, 0, "您想典当哪一项？", ans, 10, DOECHO, YEA);
	getdata(18, 0, "\xc4\xfa\xcf\xeb\xb5\xe4\xb5\xb1\xc4\xc4\xd2\xbb\xcf\xee\xa3\xbf", ans, 10, DOECHO, YEA);
	no = atoi(ans);
	if ( no < 1 || no > 9 ) {
		//% prints("\n呵呵，不典当了？ 那，好走。。欢迎再来 ;)");
		prints("\n\xba\xc7\xba\xc7\xa3\xac\xb2\xbb\xb5\xe4\xb5\xb1\xc1\xcb\xa3\xbf \xc4\xc7\xa3\xac\xba\xc3\xd7\xdf\xa1\xa3\xa1\xa3\xbb\xb6\xd3\xad\xd4\xd9\xc0\xb4 ;)");
		pressanykey();
		return 0;
	}
	move(18, 30);
	//% prints("你选择典当“[32m%s[m”。",exchanges[no-1].title);
	prints("\xc4\xe3\xd1\xa1\xd4\xf1\xb5\xe4\xb5\xb1\xa1\xb0[32m%s[m\xa1\xb1\xa1\xa3",exchanges[no-1].title);
if(no>3){
	set_safe_record();
	maxnum = exchanges[no-1].value;
	switch(no) {
		case 4:
			if(!HAS_PERM(PERM_CLOAK)) {
				num = 0;
				break;
			}
			break;
		case 5:
                        if(!HAS_PERM(PERM_SEECLOAK)) {
                                num = 0;
                                break;
                        }
			break;
		case 6:
                        if(!HAS_PERM(PERM_XEMPT)) {
                                num = 0;
                                break;
                        }
			break;
		case 7:
                        //if(!HAS_PERM(PERM_FORCEPAGE)) {
                        //        num = 0;
                        //        break;
                        //} 
                        num = 0;
			break;
		case 8:
                        //if(!HAS_PERM(PERM_EXT_IDLE)) {
                        //        num = 0;
                        //        break;
                        //}
                        num = 0;
			break;
		case 9:
                        if(!HAS_PERM(PERM_LARGEMAIL)) {
                                num = 0;
                                break;
                        }
			break;
	}
	prints("\n\n");
	if(!num) {
		//% prints("对不起, 你还没有这种权限. ");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xe3\xbb\xb9\xc3\xbb\xd3\xd0\xd5\xe2\xd6\xd6\xc8\xa8\xcf\xde. ");
		pressanykey();
		return 0;
	}
        //% if(askyn("您确定要典当吗？",NA,NA) == NA ) {
        if(askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xb5\xe4\xb5\xb1\xc2\xf0\xa3\xbf",NA,NA) == NA ) {
                move(23,0);clrtoeol();
                //% prints("现在不典当了？ 那你下次再来。 要记得哦。");
                prints("\xcf\xd6\xd4\xda\xb2\xbb\xb5\xe4\xb5\xb1\xc1\xcb\xa3\xbf \xc4\xc7\xc4\xe3\xcf\xc2\xb4\xce\xd4\xd9\xc0\xb4\xa1\xa3 \xd2\xaa\xbc\xc7\xb5\xc3\xc5\xb6\xa1\xa3");
                pressanykey();
                return 0;
        }
	set_safe_record();
        switch(no) {
                case 4:
                        num = HAS_PERM(PERM_CLOAK);
			currentuser.userlevel &= ~PERM_CLOAK ;
                        break;
                case 5:
                        num = HAS_PERM(PERM_SEECLOAK);
                        currentuser.userlevel &= ~PERM_SEECLOAK ;
                        break;
                case 6:
                        num = HAS_PERM(PERM_XEMPT);
                        currentuser.userlevel &= ~PERM_XEMPT ;
                        break;
                case 7:
                        //num = HAS_PERM(PERM_FORCEPAGE);
                        //currentuser.userlevel &= ~PERM_FORCEPAGE ;
                        break;
                case 8:
                        //num = HAS_PERM(PERM_EXT_IDLE);
                        //currentuser.userlevel &= ~PERM_EXT_IDLE ;
                        break;
                case 9:
                        num = HAS_PERM(PERM_LARGEMAIL);
                        currentuser.userlevel &= ~PERM_LARGEMAIL ;
                        break;
	}
        if(!num) {
                //% prints("对不起, 你的数据发生了变化, 你目前没有这种权限. ");
                prints("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xe3\xb5\xc4\xca\xfd\xbe\xdd\xb7\xa2\xc9\xfa\xc1\xcb\xb1\xe4\xbb\xaf, \xc4\xe3\xc4\xbf\xc7\xb0\xc3\xbb\xd3\xd0\xd5\xe2\xd6\xd6\xc8\xa8\xcf\xde. ");
                pressanykey();
                return 0;
        }
} else {
	if( no == 1 )maxnum = currentuser.numlogins;
	else if ( no == 2) maxnum = currentuser.numposts;  
	else	maxnum = currentuser.nummedals;
	templog = maxnum;
	//% sprintf(genbuf,"您想典当多少呢(最多%d)？",maxnum);
	sprintf(genbuf,"\xc4\xfa\xcf\xeb\xb5\xe4\xb5\xb1\xb6\xe0\xc9\xd9\xc4\xd8(\xd7\xee\xb6\xe0%d)\xa3\xbf",maxnum);
	getdata(19, 0, genbuf,ans, 10, DOECHO, YEA);
	num = atoi(ans);
	if ( num <= 0 || num > maxnum ) {
		//% prints("输入有错误哦。 还是算了吧。。。");
		prints("\xca\xe4\xc8\xeb\xd3\xd0\xb4\xed\xce\xf3\xc5\xb6\xa1\xa3 \xbb\xb9\xca\xc7\xcb\xe3\xc1\xcb\xb0\xc9\xa1\xa3\xa1\xa3\xa1\xa3");
		pressanykey();
		return 0;
	}
        maxnum = num*exchanges[no-1].value;
	move(19,0);
	//% prints("您共计典当%s[32m%d[m 份， %s [33m%d[m 元。\n",exchanges[no-1].title,num,"可以获得",maxnum);
	prints("\xc4\xfa\xb9\xb2\xbc\xc6\xb5\xe4\xb5\xb1%s[32m%d[m \xb7\xdd\xa3\xac %s [33m%d[m \xd4\xaa\xa1\xa3\n",exchanges[no-1].title,num,"\xbf\xc9\xd2\xd4\xbb\xf1\xb5\xc3",maxnum);
        //% if(askyn("您确定要典当吗？",NA,NA) == NA ) {
        if(askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xb5\xe4\xb5\xb1\xc2\xf0\xa3\xbf",NA,NA) == NA ) {
                move(21,0);clrtoeol();
		//% prints("现在不典当了？ 那你下次再来。 要记得哦。");
		prints("\xcf\xd6\xd4\xda\xb2\xbb\xb5\xe4\xb5\xb1\xc1\xcb\xa3\xbf \xc4\xc7\xc4\xe3\xcf\xc2\xb4\xce\xd4\xd9\xc0\xb4\xa1\xa3 \xd2\xaa\xbc\xc7\xb5\xc3\xc5\xb6\xa1\xa3");
                pressanykey();
                return 0;
        }
	set_safe_record();
	if (no == 1) {
		if(templog==currentuser.numlogins)
			currentuser.numlogins -= num;
		else templog = -1;
	} else if (no == 2)  {
		if(templog == currentuser.numposts)
			currentuser.numposts -= num;
		else templog = -1;
	} else {
		if(templog == currentuser.nummedals)
			 currentuser.nummedals -= num;
		else templog = -1;
	}
	if( templog == -1) {
		move(21,0); clrtoeol();
		//% prints("对不起, 在交易过程中您的数据发生了变化.\n为保证交易的正常进行, 此次交易取消.\n您可以重新进行本交易.");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0, \xd4\xda\xbd\xbb\xd2\xd7\xb9\xfd\xb3\xcc\xd6\xd0\xc4\xfa\xb5\xc4\xca\xfd\xbe\xdd\xb7\xa2\xc9\xfa\xc1\xcb\xb1\xe4\xbb\xaf.\n\xce\xaa\xb1\xa3\xd6\xa4\xbd\xbb\xd2\xd7\xb5\xc4\xd5\xfd\xb3\xa3\xbd\xf8\xd0\xd0, \xb4\xcb\xb4\xce\xbd\xbb\xd2\xd7\xc8\xa1\xcf\xfb.\n\xc4\xfa\xbf\xc9\xd2\xd4\xd6\xd8\xd0\xc2\xbd\xf8\xd0\xd0\xb1\xbe\xbd\xbb\xd2\xd7.");
		pressanykey();
		return 0;
	}
}
	currentuser.money += maxnum;
	if( currentuser.money > 400000000 ){
		move(21,0); clrtoeol();
		//% prints("对不起，交易数据过大，产生溢出，请重新交易！");
		prints("\xb6\xd4\xb2\xbb\xc6\xf0\xa3\xac\xbd\xbb\xd2\xd7\xca\xfd\xbe\xdd\xb9\xfd\xb4\xf3\xa3\xac\xb2\xfa\xc9\xfa\xd2\xe7\xb3\xf6\xa3\xac\xc7\xeb\xd6\xd8\xd0\xc2\xbd\xbb\xd2\xd7\xa3\xa1");
		pressanykey();
		return 0;
	}
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	move(21,0); clrtoeol();
        //% prints("顺利完成交易，欢迎您的再次惠顾。;P");
        prints("\xcb\xb3\xc0\xfb\xcd\xea\xb3\xc9\xbd\xbb\xd2\xd7\xa3\xac\xbb\xb6\xd3\xad\xc4\xfa\xb5\xc4\xd4\xd9\xb4\xce\xbb\xdd\xb9\xcb\xa1\xa3;P");
	//% sprintf(genbuf,"典当%s %d 份，花费 %d 元.",exchanges[no-1].title,num,maxnum);
	sprintf(genbuf,"\xb5\xe4\xb5\xb1%s %d \xb7\xdd\xa3\xac\xbb\xa8\xb7\xd1 %d \xd4\xaa.",exchanges[no-1].title,num,maxnum);
	gamelog(genbuf);
        pressanykey();
        return 1;
}
int doshopping()
{
        int no,hasperm=1,maxnum;
        char ans[10];
        EXCHANGES exchanges[10] = {
                //% {"隐身术",40000},
                {"\xd2\xfe\xc9\xed\xca\xf5",40000},
                //% {"看穿隐身术",10000},
                {"\xbf\xb4\xb4\xa9\xd2\xfe\xc9\xed\xca\xf5",10000},
                //% {"帐号永久保留",100000},
                {"\xd5\xca\xba\xc5\xd3\xc0\xbe\xc3\xb1\xa3\xc1\xf4",100000},
                //% {"强制呼叫",120000},//expired 06.1.5
                {"\xc7\xbf\xd6\xc6\xba\xf4\xbd\xd0",120000},//expired 06.1.5
                //% {"延长发呆时间",20000},//expired 06.1.5
                {"\xd1\xd3\xb3\xa4\xb7\xa2\xb4\xf4\xca\xb1\xbc\xe4",20000},//expired 06.1.5
                //% {"大信箱",100000}
                {"\xb4\xf3\xd0\xc5\xcf\xe4",100000}
                };
        //% if(gotomarket("市场购物中心")) return 1;
        if(gotomarket("\xca\xd0\xb3\xa1\xb9\xba\xce\xef\xd6\xd0\xd0\xc4")) return 1;
        //% prints("今日商品报价:");
        prints("\xbd\xf1\xc8\xd5\xc9\xcc\xc6\xb7\xb1\xa8\xbc\xdb:");
        for(no = 0; no < 6; no ++) {
                move(5+no, 2);
                prints("(%2d) %s",no+1,exchanges[no].title);
                move(5+no, 20);
                //% prints("==> %6d 元", exchanges[no].value);
                prints("==> %6d \xd4\xaa", exchanges[no].value);
        }
        move(16,0);
        //% prints("您目前尚有 %d 元 (奖章 %d 个)\n",
        prints("\xc4\xfa\xc4\xbf\xc7\xb0\xc9\xd0\xd3\xd0 %d \xd4\xaa (\xbd\xb1\xd5\xc2 %d \xb8\xf6)\n",
            currentuser.money,currentuser.nummedals);
        //% getdata(18, 0, "您想购买哪一件商品？", ans, 10, DOECHO, YEA);
        getdata(18, 0, "\xc4\xfa\xcf\xeb\xb9\xba\xc2\xf2\xc4\xc4\xd2\xbb\xbc\xfe\xc9\xcc\xc6\xb7\xa3\xbf", ans, 10, DOECHO, YEA);
        no = atoi(ans);
        if ( no < 1 || no > 6 ) {
                //% prints("\n呵呵，不想买了？ 那，好走。。欢迎再来 ;)");
                prints("\n\xba\xc7\xba\xc7\xa3\xac\xb2\xbb\xcf\xeb\xc2\xf2\xc1\xcb\xa3\xbf \xc4\xc7\xa3\xac\xba\xc3\xd7\xdf\xa1\xa3\xa1\xa3\xbb\xb6\xd3\xad\xd4\xd9\xc0\xb4 ;)");
                pressanykey();
                return 0;
        }
        if ( no == 4 || no == 5 ) {
                //% prints("\n小店不提供该商品了哦 :)");
                prints("\n\xd0\xa1\xb5\xea\xb2\xbb\xcc\xe1\xb9\xa9\xb8\xc3\xc9\xcc\xc6\xb7\xc1\xcb\xc5\xb6 :)");
                pressanykey();
                return 0;
       }
        set_safe_record();
        maxnum = exchanges[no-1].value;
        switch(no) {
                case 1:
                        hasperm = HAS_PERM(PERM_CLOAK);
                        break;
                case 2:
                        hasperm = HAS_PERM(PERM_SEECLOAK);
                        break;
                case 3:
                        hasperm = HAS_PERM(PERM_XEMPT);
                        break;
                case 4:
                        //hasperm = HAS_PERM(PERM_FORCEPAGE);
                        break;
                case 5:
                        //hasperm = HAS_PERM(PERM_EXT_IDLE);
                        break;
                case 6:
                        hasperm = HAS_PERM(PERM_LARGEMAIL);
                        break;
        }
        prints("\n\n");
        if(hasperm) {
                //% prints("您已经有这种权限, 不需要再购买. ");
                prints("\xc4\xfa\xd2\xd1\xbe\xad\xd3\xd0\xd5\xe2\xd6\xd6\xc8\xa8\xcf\xde, \xb2\xbb\xd0\xe8\xd2\xaa\xd4\xd9\xb9\xba\xc2\xf2. ");
                pressanykey();
                return 0;
        }
        if(currentuser.money < maxnum) {
                //% prints("\n对不起, 你没有足够的钱购买这种权限.");
                prints("\n\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xe3\xc3\xbb\xd3\xd0\xd7\xe3\xb9\xbb\xb5\xc4\xc7\xae\xb9\xba\xc2\xf2\xd5\xe2\xd6\xd6\xc8\xa8\xcf\xde.");
                pressanykey();
                return 0;
        }
        //% if(askyn("您确定要购买吗？",NA,NA) == NA ) {
        if(askyn("\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xb9\xba\xc2\xf2\xc2\xf0\xa3\xbf",NA,NA) == NA ) {
                move(23,0);clrtoeol();
                //% prints("现在不买了？ 那你下次再来。 要记得哦。");
                prints("\xcf\xd6\xd4\xda\xb2\xbb\xc2\xf2\xc1\xcb\xa3\xbf \xc4\xc7\xc4\xe3\xcf\xc2\xb4\xce\xd4\xd9\xc0\xb4\xa1\xa3 \xd2\xaa\xbc\xc7\xb5\xc3\xc5\xb6\xa1\xa3");
                pressanykey();
                return 0;
        }
        set_safe_record();
        switch(no) {
                case 1:
                        hasperm = HAS_PERM(PERM_CLOAK);
                        currentuser.userlevel |= PERM_CLOAK ;
                        break;
                case 2:
                        hasperm = HAS_PERM(PERM_SEECLOAK);
                        currentuser.userlevel |= PERM_SEECLOAK ;
                        break;
                case 3:
                        hasperm = HAS_PERM(PERM_XEMPT);
                        currentuser.userlevel |= PERM_XEMPT ;
                        break;
                case 4://expired 06.1.5
                        //hasperm = HAS_PERM(PERM_FORCEPAGE);
                        //currentuser.userlevel |= PERM_FORCEPAGE ;
                        break;
                case 5://expired 06.1.5
                        //hasperm = HAS_PERM(PERM_EXT_IDLE);
                        //currentuser.userlevel |= PERM_EXT_IDLE ;
                        break;
                case 6:
                        hasperm = HAS_PERM(PERM_LARGEMAIL);
                        currentuser.userlevel |= PERM_LARGEMAIL ;
                        break;
        }
        if(hasperm) {
                //% prints("在交易进行前您已经有了这种权限, 所以取消此次交易. ");
                prints("\xd4\xda\xbd\xbb\xd2\xd7\xbd\xf8\xd0\xd0\xc7\xb0\xc4\xfa\xd2\xd1\xbe\xad\xd3\xd0\xc1\xcb\xd5\xe2\xd6\xd6\xc8\xa8\xcf\xde, \xcb\xf9\xd2\xd4\xc8\xa1\xcf\xfb\xb4\xcb\xb4\xce\xbd\xbb\xd2\xd7. ");
                pressanykey();
                return 0;
        }
        if(currentuser.money < maxnum) {
                //% prints("\n对不起, 你没有足够的钱购买这种权限.");
                prints("\n\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xe3\xc3\xbb\xd3\xd0\xd7\xe3\xb9\xbb\xb5\xc4\xc7\xae\xb9\xba\xc2\xf2\xd5\xe2\xd6\xd6\xc8\xa8\xcf\xde.");
                pressanykey();
                return 0;
        }
	currentuser.money -= maxnum;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);        move(23,0); clrtoeol();
        //% prints("顺利完成交易，欢迎您的再次惠顾。;P");
        prints("\xcb\xb3\xc0\xfb\xcd\xea\xb3\xc9\xbd\xbb\xd2\xd7\xa3\xac\xbb\xb6\xd3\xad\xc4\xfa\xb5\xc4\xd4\xd9\xb4\xce\xbb\xdd\xb9\xcb\xa1\xa3;P");
        //% sprintf(genbuf,"购买[%s]，花费 %d 元.",exchanges[no-1].title,maxnum);
        sprintf(genbuf,"\xb9\xba\xc2\xf2[%s]\xa3\xac\xbb\xa8\xb7\xd1 %d \xd4\xaa.",exchanges[no-1].title,maxnum);
        gamelog(genbuf);
        pressanykey();
        return 1;
}	

int
payoff()
{
	//% if(gotomarket("水站还贷处")) return 0;
	if(gotomarket("\xcb\xae\xd5\xbe\xbb\xb9\xb4\xfb\xb4\xa6")) return 0;
        //% prints("本处规定: 偿还贷款必须一次还清. \n\n");
        prints("\xb1\xbe\xb4\xa6\xb9\xe6\xb6\xa8: \xb3\xa5\xbb\xb9\xb4\xfb\xbf\xee\xb1\xd8\xd0\xeb\xd2\xbb\xb4\xce\xbb\xb9\xc7\xe5. \n\n");
	if(currentuser.bet == 0 ) {
		//% prints("你并没有在本市场借钱，所以无需还钱，呵呵");
		prints("\xc4\xe3\xb2\xa2\xc3\xbb\xd3\xd0\xd4\xda\xb1\xbe\xca\xd0\xb3\xa1\xbd\xe8\xc7\xae\xa3\xac\xcb\xf9\xd2\xd4\xce\xde\xd0\xe8\xbb\xb9\xc7\xae\xa3\xac\xba\xc7\xba\xc7");
		pressanykey();
		return 0;
	}
	if(currentuser.money < currentuser.bet) {
		//% prints("你的钱不够还贷款，请下次再来还罗。");
		prints("\xc4\xe3\xb5\xc4\xc7\xae\xb2\xbb\xb9\xbb\xbb\xb9\xb4\xfb\xbf\xee\xa3\xac\xc7\xeb\xcf\xc2\xb4\xce\xd4\xd9\xc0\xb4\xbb\xb9\xc2\xde\xa1\xa3");
		pressanykey();
		return 0;
	}
	//% prints("您在本处共贷款 %d 元.\n\n", currentuser.bet);
	prints("\xc4\xfa\xd4\xda\xb1\xbe\xb4\xa6\xb9\xb2\xb4\xfb\xbf\xee %d \xd4\xaa.\n\n", currentuser.bet);
	 //% if(askyn("您现在就想还清贷款吗？",NA,NA) == NA ) {
	 if(askyn("\xc4\xfa\xcf\xd6\xd4\xda\xbe\xcd\xcf\xeb\xbb\xb9\xc7\xe5\xb4\xfb\xbf\xee\xc2\xf0\xa3\xbf",NA,NA) == NA ) {
		//% prints("现在不还了？ 那你下次再来。 要记得哦。");
		prints("\xcf\xd6\xd4\xda\xb2\xbb\xbb\xb9\xc1\xcb\xa3\xbf \xc4\xc7\xc4\xe3\xcf\xc2\xb4\xce\xd4\xd9\xc0\xb4\xa1\xa3 \xd2\xaa\xbc\xc7\xb5\xc3\xc5\xb6\xa1\xa3");
		pressanykey();
		return 0;
	}
        currentuser.money -= currentuser.bet;
        //% sprintf(genbuf,"还清贷款 %d 元.",currentuser.bet);
        sprintf(genbuf,"\xbb\xb9\xc7\xe5\xb4\xfb\xbf\xee %d \xd4\xaa.",currentuser.bet);
        gamelog(genbuf);
        currentuser.bet = 0;
        currentuser.dateforbet = 0;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
        //% prints("您已经还清了在本市场所借的钱。欢迎您的再次惠顾。;P");
        prints("\xc4\xfa\xd2\xd1\xbe\xad\xbb\xb9\xc7\xe5\xc1\xcb\xd4\xda\xb1\xbe\xca\xd0\xb3\xa1\xcb\xf9\xbd\xe8\xb5\xc4\xc7\xae\xa1\xa3\xbb\xb6\xd3\xad\xc4\xfa\xb5\xc4\xd4\xd9\xb4\xce\xbb\xdd\xb9\xcb\xa1\xa3;P");
        pressanykey();
        return 1;
}
	
int
borrow()
{
	time_t now = time(0);
	int tempbet,maxbet;
	char 	buf[STRLEN];
	char *dstr;
	//% if(gotomarket("水站借贷处"))return 0;
	if(gotomarket("\xcb\xae\xd5\xbe\xbd\xe8\xb4\xfb\xb4\xa6"))return 0;
	//% prints("本处规定: 目前每人最多可以贷款 %d 元.\n\n", MAXBET);
	prints("\xb1\xbe\xb4\xa6\xb9\xe6\xb6\xa8: \xc4\xbf\xc7\xb0\xc3\xbf\xc8\xcb\xd7\xee\xb6\xe0\xbf\xc9\xd2\xd4\xb4\xfb\xbf\xee %d \xd4\xaa.\n\n", MAXBET);
	if(!currentuser.bet)
		//% prints("您目前还没有在本处贷款.\n\n");
		prints("\xc4\xfa\xc4\xbf\xc7\xb0\xbb\xb9\xc3\xbb\xd3\xd0\xd4\xda\xb1\xbe\xb4\xa6\xb4\xfb\xbf\xee.\n\n");
	else {
		//% prints("您已经在本处贷款 %d 元.\n",currentuser.bet);
		prints("\xc4\xfa\xd2\xd1\xbe\xad\xd4\xda\xb1\xbe\xb4\xa6\xb4\xfb\xbf\xee %d \xd4\xaa.\n",currentuser.bet);
		dstr = getdatestring(currentuser.dateforbet, NA);
		//% sprintf(genbuf, "您偿还贷款的最后期限是:  %14.14s(%s) \n\n", dstr, dstr + 23);
		sprintf(genbuf, "\xc4\xfa\xb3\xa5\xbb\xb9\xb4\xfb\xbf\xee\xb5\xc4\xd7\xee\xba\xf3\xc6\xda\xcf\xde\xca\xc7:  %14.14s(%s) \n\n", dstr, dstr + 23);
		prints(genbuf);
		if( currentuser.bet>=MAXBET) {
               		//% prints("对不起, 您的贷款已经达到规定数目, 不能再享受贷款服务.");
               		prints("\xb6\xd4\xb2\xbb\xc6\xf0, \xc4\xfa\xb5\xc4\xb4\xfb\xbf\xee\xd2\xd1\xbe\xad\xb4\xef\xb5\xbd\xb9\xe6\xb6\xa8\xca\xfd\xc4\xbf, \xb2\xbb\xc4\xdc\xd4\xd9\xcf\xed\xca\xdc\xb4\xfb\xbf\xee\xb7\xfe\xce\xf1.");
                        pressanykey();
                        return 0;
                }

	}
	//% if(askyn("您现在想向本站贷款吗？",NA,NA) == NA ) return 0;
	if(askyn("\xc4\xfa\xcf\xd6\xd4\xda\xcf\xeb\xcf\xf2\xb1\xbe\xd5\xbe\xb4\xfb\xbf\xee\xc2\xf0\xa3\xbf",NA,NA) == NA ) return 0;
	maxbet = MAXBET-currentuser.bet;
	if( maxbet > 1000 ) {
		//% sprintf(genbuf,  "您可以贷款: 至少 1000 元, 最多 %d 元。您想借多少呢？",maxbet);
		sprintf(genbuf,  "\xc4\xfa\xbf\xc9\xd2\xd4\xb4\xfb\xbf\xee: \xd6\xc1\xc9\xd9 1000 \xd4\xaa, \xd7\xee\xb6\xe0 %d \xd4\xaa\xa1\xa3\xc4\xfa\xcf\xeb\xbd\xe8\xb6\xe0\xc9\xd9\xc4\xd8\xa3\xbf",maxbet);
		getdata(10, 0, genbuf, buf, 10, DOECHO, YEA);
		tempbet = atoi(buf);
	} else {
		//% sprintf(genbuf,"您可以贷款 %d 元，您确定要贷款吗？",maxbet);
		sprintf(genbuf,"\xc4\xfa\xbf\xc9\xd2\xd4\xb4\xfb\xbf\xee %d \xd4\xaa\xa3\xac\xc4\xfa\xc8\xb7\xb6\xa8\xd2\xaa\xb4\xfb\xbf\xee\xc2\xf0\xa3\xbf",maxbet);
		if( askyn(genbuf,YEA,NA) == NA) {
			//% prints("\n嗯，不借了？ 那好，下次再来。 ;p");
			prints("\n\xe0\xc5\xa3\xac\xb2\xbb\xbd\xe8\xc1\xcb\xa3\xbf \xc4\xc7\xba\xc3\xa3\xac\xcf\xc2\xb4\xce\xd4\xd9\xc0\xb4\xa1\xa3 ;p");
			pressanykey();
			return 0;
		}
		tempbet = maxbet;
	}
	if ( (maxbet > 1000 && tempbet >= 1000 && tempbet <= maxbet)
		||  maxbet <= 1000 ) {
		currentuser.money += tempbet;
		currentuser.bet += tempbet;
		currentuser.dateforbet = now + 10*24*60*60;
		substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
		dstr = getdatestring(currentuser.dateforbet, NA);
		//% sprintf(genbuf, "\n你向本站总共借款 %d 元，您需要在 %14.14s(%s) 还清贷款。",currentuser.bet, dstr, dstr + 23);
		sprintf(genbuf, "\n\xc4\xe3\xcf\xf2\xb1\xbe\xd5\xbe\xd7\xdc\xb9\xb2\xbd\xe8\xbf\xee %d \xd4\xaa\xa3\xac\xc4\xfa\xd0\xe8\xd2\xaa\xd4\xda %14.14s(%s) \xbb\xb9\xc7\xe5\xb4\xfb\xbf\xee\xa1\xa3",currentuser.bet, dstr, dstr + 23);
		prints(genbuf);
		//% sprintf(genbuf,"%s 借款 %d 元.",currentuser.userid,tempbet);
		sprintf(genbuf,"%s \xbd\xe8\xbf\xee %d \xd4\xaa.",currentuser.userid,tempbet);
		gamelog(genbuf);
		pressanykey();
		return 1;
        }
	//% prints("\n您输入的数目不正确，取消此次交易。");
	prints("\n\xc4\xfa\xca\xe4\xc8\xeb\xb5\xc4\xca\xfd\xc4\xbf\xb2\xbb\xd5\xfd\xc8\xb7\xa3\xac\xc8\xa1\xcf\xfb\xb4\xcb\xb4\xce\xbd\xbb\xd2\xd7\xa1\xa3");
	pressanykey();
	return 0;
}

int inmoney(unsigned int money)
{
	set_safe_record();
        if(currentuser.money + money < 400000000)currentuser.money += money ;
	else currentuser.money = 400000000;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
        return currentuser.money;
}

void demoney(unsigned int money)
{
	set_safe_record();
	if(currentuser.money > money ) currentuser.money -= money;
	else currentuser.money = 0;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
}

check_money(unsigned int money)
{
  set_safe_record();
  if(currentuser.money < money)
    {
        move(22, 0);
        clrtobot();
        //% prints("抱歉！您不可以下注 %d 元, 因为您现在身上只有 %d 元！",
        prints("\xb1\xa7\xc7\xb8\xa3\xa1\xc4\xfa\xb2\xbb\xbf\xc9\xd2\xd4\xcf\xc2\xd7\xa2 %d \xd4\xaa, \xd2\xf2\xce\xaa\xc4\xfa\xcf\xd6\xd4\xda\xc9\xed\xc9\xcf\xd6\xbb\xd3\xd0 %d \xd4\xaa\xa3\xa1",
		money,currentuser.money);
	pressanykey();
        return 1;
    }
    return 0;
}
void
show_money(int m, char *welcome,int Clear)
{
	if(Clear) clear();
	if(welcome) {
                ansimore(welcome, NA);
        }
        move(22, 0);
        clrtobot();
        set_safe_record();
        //% prints("[0;1;37;44m                  你现有现金: [36m%-18d[37m押注金额: [36m%-20d[m  ", currentuser.money, m);
        prints("[0;1;37;44m                  \xc4\xe3\xcf\xd6\xd3\xd0\xcf\xd6\xbd\xf0: [36m%-18d[37m\xd1\xba\xd7\xa2\xbd\xf0\xb6\xee: [36m%-20d[m  ", currentuser.money, m);
}

int get_money(int m, char *welcome)
{
   unsigned int	money;
   char	buf[5];
   do {
      show_money(m,welcome,YEA);
      //% getdata(23,16,"☆要押注多少钱(1 - 2000)? ", buf, 5, DOECHO, YEA);
      getdata(23,16,"\xa1\xee\xd2\xaa\xd1\xba\xd7\xa2\xb6\xe0\xc9\xd9\xc7\xae(1 - 2000)? ", buf, 5, DOECHO, YEA);
      if(buf[0] == '\0') return 0;
      money = abs(atoi(buf));
      if ( money <= 0) return 0;
      if(check_money(money))return 0;
   } while ((money < 1) || (money > 2000));
   demoney(money);
   show_money(money,NULL,YEA);
   return money;
}
