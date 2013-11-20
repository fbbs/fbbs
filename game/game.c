#include "bbs.h"
#define MSG_SEPERATOR   "\
//% ———————————————————————————————————————"
\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa"

/* 黑杰克游戏 */
int 
BlackJack()
{
	int             num[52] = {11, 11, 11, 11, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
		7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10};
	int             cardlist[52] = {0};
	int             i, j, m, tmp = 0, tmp2, ch;
	int             win = 2, win_jack = 5;	/* win 为赢时的倍率, win_jack
						 * 1 点倍率 */
	int             six = 10, seven = 20, aj = 10, super_jack = 20;	/* 777, A+J, spade A+J 的倍率 */
	int             host_count = 2, guest_count = 1, card_count = 3,
	                A_count = 0, AA_count = 0;
	int             host_point = 0, guest_point = 0, mov_y = 4;
	int             host_card[12] = {0}, guest_card[12] = {0};
	long int        money;

	int             CHEAT = 0;	/* 做弊参数, 1 就作弊, 0 就不作 */

	set_user_status(ST_M_BLACKJACK);
	money = get_money(0,"game/blackjack.welcome");
	if(!money) return 0;
	move(1, 0);
	//% prints("【黑杰克】游戏  [按 y 续牌, n 不续牌, d double, q 认输退出]");
	prints("\xa1\xbe\xba\xda\xbd\xdc\xbf\xcb\xa1\xbf\xd3\xce\xcf\xb7  [\xb0\xb4 y \xd0\xf8\xc5\xc6, n \xb2\xbb\xd0\xf8\xc5\xc6, d double, q \xc8\xcf\xca\xe4\xcd\xcb\xb3\xf6]");
	move(0, 0);
	clrtoeol();
        srandom(time(0));
	for (i = 1; i <= 52; i++) {
		m = 0;
		do {
			j = random() % 52;
			if (cardlist[j] == 0) {
				cardlist[j] = i;
				m = 1;
			}
		} while (m == 0);
	};
	for (i = 0; i < 52; i++)
		cardlist[i]--;	/* 洗牌 */

	if (money >= 20000)
		CHEAT = 1;
	if (CHEAT == 1) {
		if (cardlist[1] <= 3) {
			tmp2 = cardlist[50];
			cardlist[50] = cardlist[1];
			cardlist[1] = tmp2;
		}
	}			/* 作弊码 */
	host_card[0] = cardlist[0];
	if (host_card[0] < 4)
		AA_count++;
	guest_card[0] = cardlist[1];

	if (guest_card[0] < 4)
		A_count++;
	host_card[1] = cardlist[2];
	if (host_card[1] < 4)
		AA_count++;	/* 发前三张牌 */

	move(5, 0);
	//% prints("╭───╮");
	prints("\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72");
	move(6, 0);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(7, 0);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(8, 0);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(9, 0);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(10, 0);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(11, 0);
	//% prints("╰───╯");
	prints("\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73");
	print_card(host_card[1], 5, 4);
	print_card(guest_card[0], 15, 0);	/* 印出前三张牌 */

	host_point = num[host_card[1]];
	guest_point = num[guest_card[0]];

	do {
		m = 1;
		guest_card[guest_count] = cardlist[card_count];
		if (guest_card[guest_count] < 4)
			A_count++;
		print_card(guest_card[guest_count], 15, mov_y);
		guest_point += num[guest_card[guest_count]];

		if ((guest_card[0] >= 24 && guest_card[0] <= 27) && (guest_card[1] >= 24 && guest_card[1] <= 27) && (guest_card[2] >= 24 && guest_card[2] <= 27)) {
			move(18, 3);
			//% prints("[1;41;33m     ７７７     [m");
			prints("[1;41;33m     \xa3\xb7\xa3\xb7\xa3\xb7     [m");
			move(3, 0);
			//% sprintf(genbuf,"[1;41;33m７７７ !!! 得奖金 %d 银两[m", money * seven);
			sprintf(genbuf,"[1;41;33m\xa3\xb7\xa3\xb7\xa3\xb7 !!! \xb5\xc3\xbd\xb1\xbd\xf0 %d \xd2\xf8\xc1\xbd[m", money * seven);
			prints(genbuf);
			inmoney(money * seven);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if ((guest_card[0] == 40 && guest_card[1] == 0) || (guest_card[0] == 0 && guest_card[1] == 40)) {
			move(18, 3);
			//% prints("[1;41;33m 超级正统 BLACK JACK  [m");
			prints("[1;41;33m \xb3\xac\xbc\xb6\xd5\xfd\xcd\xb3 BLACK JACK  [m");
			move(3, 0);
			//% sprintf(genbuf,"[1;41;33m超级正统 BLACK JACK !!! 得奖金 %d 银两[m", money * super_jack);
			sprintf(genbuf,"[1;41;33m\xb3\xac\xbc\xb6\xd5\xfd\xcd\xb3 BLACK JACK !!! \xb5\xc3\xbd\xb1\xbd\xf0 %d \xd2\xf8\xc1\xbd[m", money * super_jack);
			prints(genbuf);
			inmoney(money * super_jack);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if ((guest_card[0] <= 3 && guest_card[0] >= 0) && (guest_card[1] <= 43 && guest_card[1] >= 40))
			tmp = 1;

		if ((tmp == 1) || ((guest_card[1] <= 3 && guest_card[1] >= 0) && (guest_card[0] <= 43 && guest_card[0] >= 40))) {
			move(18, 3);
			prints("[1;41;33m SUPER BLACK JACK  [m");
			move(3, 0);
			//% sprintf(genbuf,"[1;41;33mSUPER BLACK JACK !!! 得奖金 %d 银两[m", money * aj);
			sprintf(genbuf,"[1;41;33mSUPER BLACK JACK !!! \xb5\xc3\xbd\xb1\xbd\xf0 %d \xd2\xf8\xc1\xbd[m", money * aj);
			prints(genbuf);
			inmoney(money * aj);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if (guest_point == 21 && guest_count == 1) {
			move(18, 3);
			prints("[1;41;33m  BLACK JACK  [m");
			move(3, 0);
			//% sprintf(genbuf,"[1;41;33mBLACK JACK !!![44m 得奖金 %d 银两[m", money * win_jack);
			sprintf(genbuf,"[1;41;33mBLACK JACK !!![44m \xb5\xc3\xbd\xb1\xbd\xf0 %d \xd2\xf8\xc1\xbd[m", money * win_jack);
			prints(genbuf);
			inmoney(money * win_jack);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}		/* 前两张就 21 点 */
		if (guest_point > 21) {
			if (A_count > 0) {
				guest_point -= 10;
				A_count--;
			};
		}
		move(19, 0);
		//clrtoeol();
		//% prints("[1;32m点数: [33m%d[m", host_point);
		prints("[1;32m\xb5\xe3\xca\xfd: [33m%d[m", host_point);
		move(20, 0);
		//clrtoeol();
		//% prints("[1;32m点数: [33m%d[m", guest_point);
		prints("[1;32m\xb5\xe3\xca\xfd: [33m%d[m", guest_point);
		if (guest_point > 21) {
			move(20, 0);
			//clrtoeol();
			//% prints("  爆掉啦~~~  ");
			prints("  \xb1\xac\xb5\xf4\xc0\xb2~~~  ");
			pressanykey();
			return 0;
		}
		if (guest_count == 5) {
			move(18, 3);
			//% prints("[1;41;33m            过六关            [m");
			prints("[1;41;33m            \xb9\xfd\xc1\xf9\xb9\xd8            [m");
			move(3, 0);
			//% sprintf(genbuf,"[1;41;33m过六关 !!! 得奖金 %d 银两[m", money * six);
			sprintf(genbuf,"[1;41;33m\xb9\xfd\xc1\xf9\xb9\xd8 !!! \xb5\xc3\xbd\xb1\xbd\xf0 %d \xd2\xf8\xc1\xbd[m", money * six);
			prints(genbuf);
			inmoney(money * six);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		guest_count++;
		card_count++;
		mov_y += 4;

		do {
			if (ch == 'd')
				m = 0;
			if (m != 0)
				ch = egetch();
		} while (ch != 'y' && ch != 'Y' && ch != 'n' && ch != 'N' 
                      && ch != 'd' && ch != 'D' && ch != 'q' && ch != 'Q'
                      && m != 0 );	/* 抓 key */

		if (ch == 'd' && m != 0 && guest_count == 2) {
			if (currentuser.money >= money) {
				demoney(money);
				money *= 2;
			} else
				ch = 'n';
		}		/* double */
		if (ch == 'd' && guest_count > 2)
			ch = 'n';
		if (ch == 'q' || ch == 'Q') return ;
		if (guest_point == 21)
			ch = 'n';
	} while (ch != 'n' && m != 0);

	mov_y = 8;

	print_card(host_card[0], 5, 0);
	print_card(host_card[1], 5, 4);
	host_point += num[host_card[0]];

	do {

		if (host_point < guest_point) {
			host_card[host_count] = cardlist[card_count];
			print_card(host_card[host_count], 5, mov_y);
			if (host_card[host_count] < 4)
				AA_count++;
			host_point += num[host_card[host_count]];
		}
		if (host_point > 21) {
			if (AA_count > 0) {
				host_point -= 10;
				AA_count--;
			};
		}
		move(19, 0);
		//clrtoeol();
		//% prints("[1;32m点数: [33m%d[m", host_point);
		prints("[1;32m\xb5\xe3\xca\xfd: [33m%d[m", host_point);
		move(20, 0);
		//clrtoeol();
		//% prints("[1;32m点数: [33m%d[m", guest_point);
		prints("[1;32m\xb5\xe3\xca\xfd: [33m%d[m", guest_point);
		if (host_point > 21) {
			move(20, 0);
			//clrtoeol();
			//% prints("[1;32m点数: [33m%d [1;41;33m WINNER [m", guest_point);
			prints("[1;32m\xb5\xe3\xca\xfd: [33m%d [1;41;33m WINNER [m", guest_point);

			move(3, 0);
			//% sprintf(genbuf,"赢了~~~~ 得奖金 %d 银两", money * win);
			sprintf(genbuf,"\xd3\xae\xc1\xcb~~~~ \xb5\xc3\xbd\xb1\xbd\xf0 %d \xd2\xf8\xc1\xbd", money * win);
			prints(genbuf);
			gamelog(genbuf);
			inmoney(money * win);
			pressanykey();
			return 0;
		}
		host_count++;
		card_count++;
		mov_y += 4;
	} while (host_point < guest_point);

	//% sprintf(genbuf,"输了~~~~ 没收 %d 银两!", money);
	sprintf(genbuf,"\xca\xe4\xc1\xcb~~~~ \xc3\xbb\xca\xd5 %d \xd2\xf8\xc1\xbd!", money);
	prints(genbuf);
	gamelog(genbuf);
        pressanykey();
	return 0;
}


int 
print_card(int card, int x, int y)
{
	//% char           *flower[4] = {"Ｓ", "Ｈ", "Ｄ", "Ｃ"};
	char           *flower[4] = {"\xa3\xd3", "\xa3\xc8", "\xa3\xc4", "\xa3\xc3"};
	//% char           *poker[52] = {"Ａ", "Ａ", "Ａ", "Ａ", "２", "２", "２", "２", "３", "３", "３", "３",
	char           *poker[52] = {"\xa3\xc1", "\xa3\xc1", "\xa3\xc1", "\xa3\xc1", "\xa3\xb2", "\xa3\xb2", "\xa3\xb2", "\xa3\xb2", "\xa3\xb3", "\xa3\xb3", "\xa3\xb3", "\xa3\xb3",
		//% "４", "４", "４", "４", "５", "５", "５", "５", "６", "６", "６", "６",
		"\xa3\xb4", "\xa3\xb4", "\xa3\xb4", "\xa3\xb4", "\xa3\xb5", "\xa3\xb5", "\xa3\xb5", "\xa3\xb5", "\xa3\xb6", "\xa3\xb6", "\xa3\xb6", "\xa3\xb6",
		//% "７", "７", "７", "７", "８", "８", "８", "８", "９", "９", "９", "９",
		"\xa3\xb7", "\xa3\xb7", "\xa3\xb7", "\xa3\xb7", "\xa3\xb8", "\xa3\xb8", "\xa3\xb8", "\xa3\xb8", "\xa3\xb9", "\xa3\xb9", "\xa3\xb9", "\xa3\xb9",
		//% "10", "10", "10", "10", "Ｊ", "Ｊ", "Ｊ", "Ｊ", "Ｑ", "Ｑ", "Ｑ", "Ｑ",
		"10", "10", "10", "10", "\xa3\xca", "\xa3\xca", "\xa3\xca", "\xa3\xca", "\xa3\xd1", "\xa3\xd1", "\xa3\xd1", "\xa3\xd1",
	//% "Ｋ", "Ｋ", "Ｋ", "Ｋ"};
	"\xa3\xcb", "\xa3\xcb", "\xa3\xcb", "\xa3\xcb"};

	move(x, y);
	//% prints("╭───╮");
	prints("\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72");
	move(x + 1, y);
	//% prints("│%s    │", poker[card]);
	prints("\xa9\xa6%s    \xa9\xa6", poker[card]);
	move(x + 2, y);
	//% prints("│%s    │", flower[card % 4]);
	prints("\xa9\xa6%s    \xa9\xa6", flower[card % 4]);
	move(x + 3, y);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(x + 4, y);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(x + 5, y);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(x + 6, y);
	//% prints("╰───╯");
	prints("\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73");
	return 0;
}




int
gagb()
{
	int             money;
	char            genbuf[200], buf[80];
	char            ans[5] = "";
	/* 倍率        0  1   2   3   4   5   6   7   8   9   10 */
	float           bet[11] = {0, 100, 50, 10, 3, 1.5, 1.2, 0.9, 0.8, 0.5, 0.1};
	int             a, b, c, count;

	set_user_status(ST_M_XAXB);
	srandom(time(0));
	money = get_money(0,"game/gagb.welcome");
	if(!money) return 0;
	move(6, 0);
	prints("[36m%s[m", MSG_SEPERATOR);
	move(17, 0);
	prints("[36m%s[m", MSG_SEPERATOR);

	do {
		itoa(random() % 10000, ans);
		for (a = 0; a < 3; a++)
			for (b = a + 1; b < 4; b++)
				if (ans[a] == ans[b])
					ans[0] = 0;
	} while (!ans[0]);

	for (count = 1; count < 11; count++) {
		do {
			//% getdata(5, 0, "请猜[q - 退出] → ", genbuf, 5, DOECHO, YEA);
			getdata(5, 0, "\xc7\xeb\xb2\xc2[q - \xcd\xcb\xb3\xf6] \xa1\xfa ", genbuf, 5, DOECHO, YEA);
			if (!strcmp(genbuf, "Good")) {
				prints("[%s]", ans);
				//% sprintf(genbuf,"猜数字作弊, 下注 %d 元", money);
				sprintf(genbuf,"\xb2\xc2\xca\xfd\xd7\xd6\xd7\xf7\xb1\xd7, \xcf\xc2\xd7\xa2 %d \xd4\xaa", money);
				gamelog(genbuf);
				igetch();
			}
			if ( genbuf[0] == 'q' || genbuf[0] == 'Q' ) {
				//% sprintf(buf,"放弃猜测, 扣除压注金额 %d 元.", money);
				sprintf(buf,"\xb7\xc5\xc6\xfa\xb2\xc2\xb2\xe2, \xbf\xdb\xb3\xfd\xd1\xb9\xd7\xa2\xbd\xf0\xb6\xee %d \xd4\xaa.", money);
				gamelog(buf);
				return;
			}
			c = atoi(genbuf);
			itoa(c, genbuf);
			for (a = 0; a < 3; a++)
				for (b = a + 1; b < 4; b++)
					if (genbuf[a] == genbuf[b])
						genbuf[0] = 0;
			if (!genbuf[0]) {
				move ( 18,3 );
				//% prints("输入数字有问题!!");
				prints("\xca\xe4\xc8\xeb\xca\xfd\xd7\xd6\xd3\xd0\xce\xca\xcc\xe2!!");
				pressanykey();
				move ( 18,3 );
				prints("                ");
			}
		} while (!genbuf[0]);
		move(count + 6, 0);
		//% prints("  [1;31m第 [37m%2d [31m次： [37m%s  ->  [33m%dA [36m%dB [m", count, genbuf, an(genbuf, ans), bn(genbuf, ans));
		prints("  [1;31m\xb5\xda [37m%2d [31m\xb4\xce\xa3\xba [37m%s  ->  [33m%dA [36m%dB [m", count, genbuf, an(genbuf, ans), bn(genbuf, ans));
		if (an(genbuf, ans) == 4)
			break;
	}

	if (count > 10) {
		//% sprintf(buf, "你输了呦！正确答案是 %s，下次再加油吧!!", ans);
		sprintf(buf, "\xc4\xe3\xca\xe4\xc1\xcb\xdf\xcf\xa3\xa1\xd5\xfd\xc8\xb7\xb4\xf0\xb0\xb8\xca\xc7 %s\xa3\xac\xcf\xc2\xb4\xce\xd4\xd9\xbc\xd3\xd3\xcd\xb0\xc9!!", ans);
		//% sprintf(genbuf,"[1;31m可怜没猜到，输了 %d 元！[m", money);
		sprintf(genbuf,"[1;31m\xbf\xc9\xc1\xaf\xc3\xbb\xb2\xc2\xb5\xbd\xa3\xac\xca\xe4\xc1\xcb %d \xd4\xaa\xa3\xa1[m", money);
		gamelog(genbuf);
	} else {
		int             oldmoney = money;
		money *= bet[count];
		inmoney(money);
		if (money - oldmoney > 0)
			//% sprintf(buf, "恭喜！总共猜了 %d 次，净赚奖金 %d 元", count, money - oldmoney);
			sprintf(buf, "\xb9\xa7\xcf\xb2\xa3\xa1\xd7\xdc\xb9\xb2\xb2\xc2\xc1\xcb %d \xb4\xce\xa3\xac\xbe\xbb\xd7\xac\xbd\xb1\xbd\xf0 %d \xd4\xaa", count, money - oldmoney);
		else if (money - oldmoney == 0)
			//% sprintf(buf, "唉～～总共猜了 %d 次，没输没赢！", count);
			sprintf(buf, "\xb0\xa6\xa1\xab\xa1\xab\xd7\xdc\xb9\xb2\xb2\xc2\xc1\xcb %d \xb4\xce\xa3\xac\xc3\xbb\xca\xe4\xc3\xbb\xd3\xae\xa3\xa1", count);
		else
			//% sprintf(buf, "啊～～总共猜了 %d 次，赔钱 %d 元！", count, oldmoney - money);
			sprintf(buf, "\xb0\xa1\xa1\xab\xa1\xab\xd7\xdc\xb9\xb2\xb2\xc2\xc1\xcb %d \xb4\xce\xa3\xac\xc5\xe2\xc7\xae %d \xd4\xaa\xa3\xa1", count, oldmoney - money);
	}
	gamelog(buf);
	move(22, 0);
        clrtobot();
	prints(buf);
	pressanykey();
	return 0;
}

itoa(i, a)
	int             i;
	char           *a;
{
	int             j, k, l = 1000;
	//prints("itoa: i=%d ", i);

	for (j = 3; j > 0; j--) {
		k = i - (i % l);
		i -= k;
		k = k / l + 48;
		a[3 - j] = k;
		l /= 10;
	}
	a[3] = i + 48;
	a[4] = 0;

	//prints(" a=%s\n", a);
	//igetch();
}

int
an(a, b)
	char           *a, *b;
{
	int             i, k = 0;
	for (i = 0; i < 4; i++)
		if (*(a + i) == *(b + i))
			k++;
	return k;
}

int
bn(a, b)
	char           *a, *b;
{
	int             i, j, k = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (*(a + i) == *(b + j))
				k++;
	return (k - an(a, b));
}


/*
 //% * 程式设计：wsyfish 自己评语：写得很烂，乱写一通，没啥深度:)
 * \xb3\xcc\xca\xbd\xc9\xe8\xbc\xc6\xa3\xbawsyfish \xd7\xd4\xbc\xba\xc6\xc0\xd3\xef\xa3\xba\xd0\xb4\xb5\xc3\xba\xdc\xc0\xc3\xa3\xac\xc2\xd2\xd0\xb4\xd2\xbb\xcd\xa8\xa3\xac\xc3\xbb\xc9\xb6\xc9\xee\xb6\xc8:)
 //% * 相容程度：Ptt板本应该都行吧，就用inmoney和demoney，其他如Sob就要改一下罗
 * \xcf\xe0\xc8\xdd\xb3\xcc\xb6\xc8\xa3\xbaPtt\xb0\xe5\xb1\xbe\xd3\xa6\xb8\xc3\xb6\xbc\xd0\xd0\xb0\xc9\xa3\xac\xbe\xcd\xd3\xc3inmoney\xba\xcddemoney\xa3\xac\xc6\xe4\xcb\xfb\xc8\xe7Sob\xbe\xcd\xd2\xaa\xb8\xc4\xd2\xbb\xcf\xc2\xc2\xde
 * */
char           *dice[6][3] = {"        ",
	//% "   ●   ",
	"   \xa1\xf1   ",
	"        ",
	//% "   ●   ",
	"   \xa1\xf1   ",
	"        ",
	//% "   ●   ",
	"   \xa1\xf1   ",
	//% "   ●   ",
	"   \xa1\xf1   ",
	//% "   ●   ",
	"   \xa1\xf1   ",
	//% "   ●   ",
	"   \xa1\xf1   ",
	//% "●    ●",
	"\xa1\xf1    \xa1\xf1",
	"        ",
	//% "●    ●",
	"\xa1\xf1    \xa1\xf1",
	//% "●    ●",
	"\xa1\xf1    \xa1\xf1",
	//% "   ●   ",
	"   \xa1\xf1   ",
	//% "●    ●",
	"\xa1\xf1    \xa1\xf1",
	//% "●    ●",
	"\xa1\xf1    \xa1\xf1",
	//% "●    ●",
	"\xa1\xf1    \xa1\xf1",
	//% "●    ●"
	"\xa1\xf1    \xa1\xf1"
};

int
x_dice()
{
	char            choice[11], buf[60];
	int             i, money;
	char            tmpchar;/* 纪录选项 */
	char            tmpdice[3];	/* 三个骰子的值 */
	char            totaldice;
	time_t          now = time(0);

	srandom(time(0));
	time(&now);

	set_user_status(ST_M_DICE);
	while (1) {
		money = get_money(0,"game/xdice.welcome");
		if(!money) return 0;
		move(2,0);
		//% outs("\n┌────────────────────────────────────┐\n"
		outs("\n\xa9\xb0\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xb4\n"
		     //% "│ ２倍  1. 大      2. 小                                                 │\n"
		     "\xa9\xa6 \xa3\xb2\xb1\xb6  1. \xb4\xf3      2. \xd0\xa1                                                 \xa9\xa6\n"
		     //% "│ ５倍  3. 三点    4. 四点     5. 五点    6. 六点    7. 七点    8. 八点  │\n"
		     "\xa9\xa6 \xa3\xb5\xb1\xb6  3. \xc8\xfd\xb5\xe3    4. \xcb\xc4\xb5\xe3     5. \xce\xe5\xb5\xe3    6. \xc1\xf9\xb5\xe3    7. \xc6\xdf\xb5\xe3    8. \xb0\xcb\xb5\xe3  \xa9\xa6\n"
		     //% "│       9. 九点   10. 十点    11. 十一点 12. 十二点 13. 十三点 14. 十四点│\n"
		     "\xa9\xa6       9. \xbe\xc5\xb5\xe3   10. \xca\xae\xb5\xe3    11. \xca\xae\xd2\xbb\xb5\xe3 12. \xca\xae\xb6\xfe\xb5\xe3 13. \xca\xae\xc8\xfd\xb5\xe3 14. \xca\xae\xcb\xc4\xb5\xe3\xa9\xa6\n"
		     //% "│      15. 十五点 16. 十六点  17. 十七点 18. 十八点                      │\n"
		     "\xa9\xa6      15. \xca\xae\xce\xe5\xb5\xe3 16. \xca\xae\xc1\xf9\xb5\xe3  17. \xca\xae\xc6\xdf\xb5\xe3 18. \xca\xae\xb0\xcb\xb5\xe3                      \xa9\xa6\n"
		     //% "│ ９倍 19. 一一一 20. 二二二  21. 三三三 22. 四四四 23. 五五五 24. 六六六│\n"
		     "\xa9\xa6 \xa3\xb9\xb1\xb6 19. \xd2\xbb\xd2\xbb\xd2\xbb 20. \xb6\xfe\xb6\xfe\xb6\xfe  21. \xc8\xfd\xc8\xfd\xc8\xfd 22. \xcb\xc4\xcb\xc4\xcb\xc4 23. \xce\xe5\xce\xe5\xce\xe5 24. \xc1\xf9\xc1\xf9\xc1\xf9\xa9\xa6\n"
		     //% "└────────────────────────────────────┘\n");
		     "\xa9\xb8\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xbc\n");
		//% getdata(11, 0, "要押哪一项呢？(请输入号码) ", choice, 3, DOECHO, YEA);
		getdata(11, 0, "\xd2\xaa\xd1\xba\xc4\xc4\xd2\xbb\xcf\xee\xc4\xd8\xa3\xbf(\xc7\xeb\xca\xe4\xc8\xeb\xba\xc5\xc2\xeb) ", choice, 3, DOECHO, YEA);
		tmpchar = atoi(choice);
		if (tmpchar <= 0 || tmpchar > 24) {
			//% prints("要押的项目输入有误！离开赌场");
			prints("\xd2\xaa\xd1\xba\xb5\xc4\xcf\xee\xc4\xbf\xca\xe4\xc8\xeb\xd3\xd0\xce\xf3\xa3\xa1\xc0\xeb\xbf\xaa\xb6\xc4\xb3\xa1");
			pressanykey();
			break;
		}
		//% outs("\n按任一键掷出骰子....\n");
		outs("\n\xb0\xb4\xc8\xce\xd2\xbb\xbc\xfc\xd6\xc0\xb3\xf6\xf7\xbb\xd7\xd3....\n");
		egetch();

		do {
			totaldice = 0;
			for (i = 0; i < 3; i++) {
				tmpdice[i] = random() % 6 + 1;
				totaldice += tmpdice[i];
			}

			if (((tmpchar == 1) && totaldice > 10) ||
			    ((tmpchar == 2) && totaldice <= 10)) {
				if ((random() % 10) < 7)	/* 作弊用，中奖率为原来之70% */
					break;
			} else
				break;

		} while (tmpchar <= 2);

		if ((tmpchar <= 18 && totaldice == tmpchar) && (currentuser.money > 10000)) {
			if (tmpdice[0] > 1)
				tmpdice[0]--;
			else if (tmpdice[1] > 1)
				tmpdice[1]--;
			else if (tmpdice[2] < 5)
				tmpdice[2]++;
		}
		//% outs("\n╭────╮╭────╮╭────╮\n");
		outs("\n\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72\n");

		for (i = 0; i < 3; i++)
			//% prints("│%s││%s││%s│\n",
			prints("\xa9\xa6%s\xa9\xa6\xa9\xa6%s\xa9\xa6\xa9\xa6%s\xa9\xa6\n",
			       dice[tmpdice[0] - 1][i],
			       dice[tmpdice[1] - 1][i],
			       dice[tmpdice[2] - 1][i]);

		//% outs("╰────╯╰────╯╰────╯\n\n");
		outs("\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73\n\n");

		if ((tmpchar == 1 && totaldice > 10)
		    || (tmpchar == 2 && totaldice <= 10))	/* 处理大小 */
			//% sprintf(buf, "中了！得到２倍奖金 %d 元，总共有 %d 元",
			sprintf(buf, "\xd6\xd0\xc1\xcb\xa3\xa1\xb5\xc3\xb5\xbd\xa3\xb2\xb1\xb6\xbd\xb1\xbd\xf0 %d \xd4\xaa\xa3\xac\xd7\xdc\xb9\xb2\xd3\xd0 %d \xd4\xaa",
				money * 2, inmoney(money * 2));
		else if (tmpchar <= 18 && totaldice == tmpchar)	/* 处理总和 */
			//% sprintf(buf, "中了！得到５倍奖金 %d 元，总共有 %d 元",
			sprintf(buf, "\xd6\xd0\xc1\xcb\xa3\xa1\xb5\xc3\xb5\xbd\xa3\xb5\xb1\xb6\xbd\xb1\xbd\xf0 %d \xd4\xaa\xa3\xac\xd7\xdc\xb9\xb2\xd3\xd0 %d \xd4\xaa",
				money * 5, inmoney(money * 5));
		else if ((tmpchar - 18) == tmpdice[0] && (tmpdice[0] == tmpdice[1])
			 && (tmpdice[1] == tmpdice[2]))	/* 处理三个一样总和 */
			//% sprintf(buf, "中了！得到９倍奖金 %d 元，总共有 %d 元",
			sprintf(buf, "\xd6\xd0\xc1\xcb\xa3\xa1\xb5\xc3\xb5\xbd\xa3\xb9\xb1\xb6\xbd\xb1\xbd\xf0 %d \xd4\xaa\xa3\xac\xd7\xdc\xb9\xb2\xd3\xd0 %d \xd4\xaa",
				money * 9, inmoney(money * 9));

		else		/* 处理没中 */
			//% sprintf(buf, "很可惜没有押中！输了 %d 元",money);
			sprintf(buf, "\xba\xdc\xbf\xc9\xcf\xa7\xc3\xbb\xd3\xd0\xd1\xba\xd6\xd0\xa3\xa1\xca\xe4\xc1\xcb %d \xd4\xaa",money);
		gamelog(buf);
		prints(buf);
		pressanykey();
	}
	return 0;
}



/* write by dsyan               */
/* 87/10/24                     */
/* 天长地久 Forever.twbbs.org   */

char            card[52], mycard[5], cpucard[5], sty[100], now;

static int
forq(a, b)
	char           *a, *b;
{
	char            c = (*a) % 13;
	char            d = (*b) % 13;
	if (!c)
		c = 13;
	if (!d)
		d = 13;
	if (c == 1)
		c = 14;
	if (d == 1)
		d = 14;
	if (c == d)
		return *a - *b;
	return c - d;
}

int
p_gp()
{
	char            genbuf[200], hold[5];
	char            ans[5];
	int             bet, i, j, k, tmp, x, xx, doub, gw = 0, cont = 0;

	srandom(time(0));
	set_user_status(ST_M_GP);
	bet = 0;
	while (1) {
		screen_clear();
		ans[0] = 0;
		if (cont == 0) 
			if(!(bet = get_money(bet,"game/gp.welcome"))) return 0;
		move(21, 0);
		clrtoeol();
		if (cont > 0)
			//% prints("[33;1m (←)(→)改变选牌  (SPCAE)改变换牌  (Enter)确定[m");
			prints("[33;1m (\xa1\xfb)(\xa1\xfa)\xb8\xc4\xb1\xe4\xd1\xa1\xc5\xc6  (SPCAE)\xb8\xc4\xb1\xe4\xbb\xbb\xc5\xc6  (Enter)\xc8\xb7\xb6\xa8[m");
		else
			//% prints("[33;1m (←)(→)改变选牌  (d)Double  (SPCAE)改变换牌  (Enter)确定[m");
			prints("[33;1m (\xa1\xfb)(\xa1\xfa)\xb8\xc4\xb1\xe4\xd1\xa1\xc5\xc6  (d)Double  (SPCAE)\xb8\xc4\xb1\xe4\xbb\xbb\xc5\xc6  (Enter)\xc8\xb7\xb6\xa8[m");
		show_money(bet,NULL,NA);

		for (i = 0; i < 52; i++)
			card[i] = i;	/* 0~51 ..黑杰克是 1~52 */

		for (i = 0; i < 1000; i++) {
			j = random() % 52;
			k = random() % 52;
			tmp = card[j];
			card[j] = card[k];
			card[k] = tmp;
		}

		now = doub = 0;
		for (i = 0; i < 5; i++) {
			mycard[i] = card[now++];
			hold[i] = 1;
		}
		qsort(mycard, 5, sizeof(char), forq);

		for (i = 0; i < 5; i++)
			show_card(0, mycard[i], i);

		x = xx = tmp = 0;
		while (tmp != 10) {
			for (i = 0; i < 5; i++) {
				move(16, i * 4 + 2);
				//% outs(hold[i] < 0 ? "保" : "  ");
				outs(hold[i] < 0 ? "\xb1\xa3" : "  ");
				move(17, i * 4 + 2);
				//% outs(hold[i] < 0 ? "留" : "  ");
				outs(hold[i] < 0 ? "\xc1\xf4" : "  ");
			}
			move(8, xx * 4 + 2);
			outs("  ");
			move(8, x * 4 + 2);
			//% outs("↓");
			outs("\xa1\xfd");
			move(t_lines - 1, 0);
			refresh();
			xx = x;

			tmp = egetch();
			switch (tmp) {
#ifdef GP_DEBUG
			case KEY_UP:
				//% getdata(21, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
				getdata(21, 0, "\xb0\xd1\xc5\xc6\xbb\xbb\xb3\xc9> ", genbuf, 3, DOECHO, YEA);
				mycard[x] = atoi(genbuf);
				qsort(mycard, 5, sizeof(char), forq);
				for (i = 0; i < 5; i++)
					show_card(0, mycard[i], i);
				break;
#endif
			case KEY_LEFT:
			case 'l':
				x = x ? x - 1 : 4;
				break;
			case KEY_RIGHT:
			case 'r':
				x = (x == 4) ? 0 : x + 1;
				break;
			case ' ':
				hold[x] *= -1;
				break;
			case 'd':
				if (!cont && !doub && currentuser.money >= bet) {
					doub++;
					move(21, 0);
					clrtoeol();
					//% prints("[33;1m (←)(→)改变选牌  (SPCAE)改变换牌  (Enter)确定[m");
					prints("[33;1m (\xa1\xfb)(\xa1\xfa)\xb8\xc4\xb1\xe4\xd1\xa1\xc5\xc6  (SPCAE)\xb8\xc4\xb1\xe4\xbb\xbb\xc5\xc6  (Enter)\xc8\xb7\xb6\xa8[m");
					demoney(bet);
					bet *= 2;
					show_money(bet,NULL,NA);
				}
				break;
			}
		}

		for (i = 0; i < 5; i++)
			if (hold[i] == 1)
				mycard[i] = card[now++];
		qsort(mycard, 5, sizeof(char), forq);
		for (i = 0; i < 5; i++)
			show_card(0, mycard[i], i);
		move(11, x * 4 + 2);
		outs("  ");

		cpu();
#ifdef GP_DEBUG
		for (x = 0; x < 5; x++) {
			//% getdata(21, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
			getdata(21, 0, "\xb0\xd1\xc5\xc6\xbb\xbb\xb3\xc9> ", genbuf, 3, DOECHO, YEA);
			cpucard[x] = atoi(genbuf);
		}
		qsort(cpucard, 5, sizeof(char), forq);
		for (i = 0; i < 5; i++)
			show_card(1, cpucard[i], i);
#endif
		i = gp_win();

		if (i < 0) {
			inmoney(bet * 2);
			//% sprintf(genbuf, "哇!!好棒喔!!净赚 %d 元咧.. :DDD", bet);
			sprintf(genbuf, "\xcd\xdb!!\xba\xc3\xb0\xf4\xe0\xb8!!\xbe\xbb\xd7\xac %d \xd4\xaa\xdf\xd6.. :DDD", bet);
			prints(genbuf);
			if (cont > 0)
				//% sprintf(genbuf, "连胜 %d 次, 赢了 %d 元",
				sprintf(genbuf, "\xc1\xac\xca\xa4 %d \xb4\xce, \xd3\xae\xc1\xcb %d \xd4\xaa",
					cont + 1, bet);
			else
				//% sprintf(genbuf, "赢了 %d 元", bet);
				sprintf(genbuf, "\xd3\xae\xc1\xcb %d \xd4\xaa", bet);
			gamelog(genbuf);
			bet = (bet > 50000 ? 100000 : bet * 2);
			gw = 1;
		} else if (i > 1000) {
			switch (i) {
			case 1001:
				doub = 15;
				break;
			case 1002:
				doub = 10;
				break;
			case 1003:
				doub = 5;
				break;
			}
			inmoney(bet * 2 * doub);
			//% sprintf(genbuf, "哇!!好棒喔!!净赚 %d 元咧.. :DDD", bet * 2 * doub - bet);
			sprintf(genbuf, "\xcd\xdb!!\xba\xc3\xb0\xf4\xe0\xb8!!\xbe\xbb\xd7\xac %d \xd4\xaa\xdf\xd6.. :DDD", bet * 2 * doub - bet);
			prints(genbuf);
			if (cont > 0)
				//% sprintf(genbuf, "连胜 %d 次, 赢了 %d 元",
				sprintf(genbuf, "\xc1\xac\xca\xa4 %d \xb4\xce, \xd3\xae\xc1\xcb %d \xd4\xaa",
				   cont + 1, bet * doub);
			else
				//% sprintf(genbuf, "赢了 %d 元", bet * doub);
				sprintf(genbuf, "\xd3\xae\xc1\xcb %d \xd4\xaa", bet * doub);
			gamelog(genbuf);
			bet = (bet > 50000 ? 100000 : bet * 2 * doub);
			gw = 1;
			bet = (bet >= 100000 ? 100000 : bet);
		} else {
			//% prints("输了..:~~~");
			prints("\xca\xe4\xc1\xcb..:~~~");
			if (cont > 1)
				//% sprintf(genbuf, "中止 %d 连胜, 输了 %d 元", cont, bet);
				sprintf(genbuf, "\xd6\xd0\xd6\xb9 %d \xc1\xac\xca\xa4, \xca\xe4\xc1\xcb %d \xd4\xaa", cont, bet);
			else
				//% sprintf(genbuf, "输了 %d 元", bet);
				sprintf(genbuf, "\xca\xe4\xc1\xcb %d \xd4\xaa", bet);
			gamelog(genbuf);
			cont = 0;
			bet = 0;
			pressanykey();
		}

		if (gw == 1) {
			gw = 0;
			//% getdata(21, 0, "您要把奖金继续压注吗 (y/n)?", ans, 2, DOECHO, YEA);
			getdata(21, 0, "\xc4\xfa\xd2\xaa\xb0\xd1\xbd\xb1\xbd\xf0\xbc\xcc\xd0\xf8\xd1\xb9\xd7\xa2\xc2\xf0 (y/n)?", ans, 2, DOECHO, YEA);
			if (ans[0] == 'y' || ans[0] == 'Y') {
				demoney (bet);/* added by soff */
				cont++;
			} else {
				cont = 0;
				bet = 0;
			}
		}
	}
}

show_card(isDealer, c, x)
	int             isDealer, c, x;
{
	int             beginL;
	//% char           *suit[4] = {"Ｃ", "Ｄ", "Ｈ", "Ｓ"};
	char           *suit[4] = {"\xa3\xc3", "\xa3\xc4", "\xa3\xc8", "\xa3\xd3"};
	//% char           *num[13] = {"Ｋ", "Ａ", "２", "３", "４", "５", "６",
	char           *num[13] = {"\xa3\xcb", "\xa3\xc1", "\xa3\xb2", "\xa3\xb3", "\xa3\xb4", "\xa3\xb5", "\xa3\xb6",
	//% "７", "８", "９", "10", "Ｊ", "Ｑ"};
	"\xa3\xb7", "\xa3\xb8", "\xa3\xb9", "10", "\xa3\xca", "\xa3\xd1"};

	beginL = (isDealer) ? 2 : 12;
	move(beginL, x * 4);
	//% outs("╭───╮");
	outs("\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72");
	move(beginL + 1, x * 4);
	//% prints("│%2s    │", num[c % 13]);
	prints("\xa9\xa6%2s    \xa9\xa6", num[c % 13]);
	move(beginL + 2, x * 4);
	//% prints("│%2s    │", suit[c / 13]);	/* ←这里跟黑杰克 */
	prints("\xa9\xa6%2s    \xa9\xa6", suit[c / 13]);	/* \xa1\xfb\xd5\xe2\xc0\xef\xb8\xfa\xba\xda\xbd\xdc\xbf\xcb */
#ifdef GP_DEBUG
	move(beginL + 3, x * 4);
	//% prints("│%2d    │", c);	/* 有点不同喔!! */
	prints("\xa9\xa6%2d    \xa9\xa6", c);	/* \xd3\xd0\xb5\xe3\xb2\xbb\xcd\xac\xe0\xb8!! */
#else
	move(beginL + 3, x * 4);
	//% outs("│      │");	/* 有点不同喔!! */
	outs("\xa9\xa6      \xa9\xa6");	/* \xd3\xd0\xb5\xe3\xb2\xbb\xcd\xac\xe0\xb8!! */
#endif
	move(beginL + 4, x * 4);
	//% outs("│      │");
	outs("\xa9\xa6      \xa9\xa6");
	move(beginL + 5, x * 4);
	//% outs("│      │");
	outs("\xa9\xa6      \xa9\xa6");
	move(beginL + 6, x * 4);
	//% outs("╰───╯");
	outs("\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73");
}

cpu()
{
	char            i, j, hold[5];
	char            p[13], q[5], r[4];
	char            a[5], b[5];

	for (i = 0; i < 5; i++) {
		cpucard[i] = card[now++];
		hold[i] = 0;
	}
	qsort(cpucard, 5, sizeof(char), forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);

	tran(a, b, cpucard);
	check(p, q, r, cpucard);

	for (i = 0; i < 13; i++)
		if (p[i] > 1)
			for (j = 0; j < 5; j++)
				if (i == cpucard[j] % 13)
					hold[j] = -1;

	for (i = 0; i < 5; i++) {
		if (a[i] == 13 || a[i] == 1)
			hold[i] = -1;
		move(6, i * 4 + 2);
		//% outs(hold[i] < 0 ? "保" : "  ");
		outs(hold[i] < 0 ? "\xb1\xa3" : "  ");
		move(7, i * 4 + 2);
		//% outs(hold[i] < 0 ? "留" : "  ");
		outs(hold[i] < 0 ? "\xc1\xf4" : "  ");
	}
	move(10,25);
	//% prints("[44;37m电脑换牌前...[40m");
	prints("[44;37m\xb5\xe7\xc4\xd4\xbb\xbb\xc5\xc6\xc7\xb0...[40m");
	pressanykey();
	move(10,0); clrtoeol();

	for (i = 0; i < 5; i++)
		if (!hold[i])
			cpucard[i] = card[now++];
	qsort(cpucard, 5, sizeof(char), forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);
}

int
gp_win()
{
	int             my, cpu, ret;
	char            myx, myy, cpux, cpuy;

	my = complex(mycard, &myx, &myy);
	cpu = complex(cpucard, &cpux, &cpuy);
	show_style(my, cpu);

	if (my != cpu)
		ret = my - cpu;
	else if (myx == 1 && cpux != 1)
		ret = -1;
	else if (myx != 1 && cpux == 1)
		ret = 1;
	else if (myx != cpux)
		ret = cpux - myx;
	else if (myy != cpuy)
		ret = cpuy - myy;

	if (ret < 0)
		switch (my) {
		case 1:
			ret = 1001;
			break;
		case 2:
			ret = 1002;
			break;
		case 3:
			ret = 1003;
			break;
		}

	return ret;
}

//同花顺、铁枝、葫、同花、顺、三条、兔胚、胚、一只
int
complex(cc, x, y)
	char           *cc, *x, *y;
{
	char            p[13], q[5], r[4];
	char            a[5], b[5], c[5], d[5];
	int             i, j, k;

	tran(a, b, cc);
	check(p, q, r, cc);

	/* 同花顺 */
	if ((a[0] == a[1] - 1 && a[1] == a[2] - 1 && a[2] == a[3] - 1 && a[3] == a[4] - 1) &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = a[4];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 2 && a[1] == 3 && a[2] == 4 && a[3] == 5 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = a[3];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 10 && a[1] == 11 && a[2] == 12 && a[3] == 13 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = 1;
		*y = b[4];
		return 1;
	}
	/* 四张 */
	if (q[4] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 4)
				*x = i ? i : 13;
		return 2;
	}
	/* 葫芦 */
	if (q[3] == 1 && q[2] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 3)
				*x = i ? i : 13;
		return 3;
	}
	/* 同花 */
	for (i = 0; i < 4; i++)
		if (r[i] == 5) {
			*x = i;
			return 4;
		}
	/* 顺子 */
	memcpy(c, a, 5);
	memcpy(d, b, 5);
	for (i = 0; i < 4; i++)
		for (j = i; j < 5; j++)
			if (c[i] > c[j]) {
				k = c[i];
				c[i] = c[j];
				c[j] = k;
				k = d[i];
				d[i] = d[j];
				d[j] = k;
			}
	if (10 == c[1] && c[1] == c[2] - 1 && c[2] == c[3] - 1 && c[3] == c[4] - 1 && c[0] == 1) {
		*x = 1;
		*y = d[0];
		return 5;
	}
	if (c[0] == c[1] - 1 && c[1] == c[2] - 1 && c[2] == c[3] - 1 && c[3] == c[4] - 1) {
		*x = c[4];
		*y = d[4];
		return 5;
	}
	/* 三条 */
	if (q[3] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 3) {
				*x = i ? i : 13;
				return 6;
			}
	/* 兔胚 */
	if (q[2] == 2) {
		for (*x = 0, i = 0; i < 13; i++)
			if (p[i] == 2) {
				if ((i > 1 ? i : i + 13) > (*x == 1 ? 14 : *x)) {
					*x = i ? i : 13;
					*y = 0;
					for (j = 0; j < 5; j++)
						if (a[j] == i && b[j] > *y)
							*y = b[j];
				}
			}
		return 7;
	}
	/* 胚 */
	if (q[2] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 2) {
				*x = i ? i : 13;
				*y = 0;
				for (j = 0; j < 5; j++)
					if (a[j] == i && b[j] > *y)
						*y = b[j];
				return 8;
			}
	/* 一张 */
	*x = 0;
	*y = 0;
	for (i = 0; i < 5; i++)
		if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1) {
			*x = a[i];
			*y = b[i];
		}
	return 9;
}

/* a 是点数 .. b 是花色 */
tran(a, b, c)
	char           *a, *b, *c;
{
	int             i;
	for (i = 0; i < 5; i++) {
		a[i] = c[i] % 13;
		if (!a[i])
			a[i] = 13;
	}

	for (i = 0; i < 5; i++)
		b[i] = c[i] / 13;
}

check(p, q, r, cc)
	char           *p, *q, *r, *cc;
{
	char            i;

	for (i = 0; i < 13; i++)
		p[i] = 0;
	for (i = 0; i < 5; i++)
		q[i] = 0;
	for (i = 0; i < 4; i++)
		r[i] = 0;

	for (i = 0; i < 5; i++)
		p[cc[i] % 13]++;

	for (i = 0; i < 13; i++)
		q[p[i]]++;

	for (i = 0; i < 5; i++)
		r[cc[i] / 13]++;
}

//同花顺、铁枝、葫、同花、顺、三条、兔胚、胚、一只
show_style(my, cpu)
	char            my, cpu;
{
	//% char           *style[9] = {"同花顺", "四张", "葫芦", "同花", "顺子",
	char           *style[9] = {"\xcd\xac\xbb\xa8\xcb\xb3", "\xcb\xc4\xd5\xc5", "\xba\xf9\xc2\xab", "\xcd\xac\xbb\xa8", "\xcb\xb3\xd7\xd3",
	//% "三条", "兔胚", "单胚", "一张"};
	"\xc8\xfd\xcc\xf5", "\xcd\xc3\xc5\xdf", "\xb5\xa5\xc5\xdf", "\xd2\xbb\xd5\xc5"};
	move(5, 26);
	prints("[41;37;1m%s[m", style[cpu - 1]);
	move(15, 26);
	prints("[41;37;1m%s[m", style[my - 1]);
	//% sprintf(sty, "我的牌[44;1m%s[m..电脑的牌[44;1m%s[m",
	sprintf(sty, "\xce\xd2\xb5\xc4\xc5\xc6[44;1m%s[m..\xb5\xe7\xc4\xd4\xb5\xc4\xc5\xc6[44;1m%s[m",
		style[my - 1], style[cpu - 1]);
}

/********************************/
/* BBS 站内游戏–天地九九       */
/* 11/26/98 */
/* dsyan.bbs@Forever.twbbs.org  */
/********************************/

#undef  NINE_DEBUG

//0 1 2 3 4 5 6 7 8 9 10 11 12	/* 电脑 AI 所在 */
// char         cp[13] = {9, 8, 7, 6, 3, 2, 1, 0, 11, 5, 4, 10, 12};
char            tb[13] = {7, 6, 5, 4, 10, 9, 3, 2, 1, 0, 11, 8, 12};
//% char           *tu[4] = {"↓", "→", "↑", "←"};
char           *tu[4] = {"\xa1\xfd", "\xa1\xfa", "\xa1\xfc", "\xa1\xfb"};
char            card[52], ahand[4][5], now, dir, turn, live;
static char            buf[255];
int             sum;

static int
forqp(a, b)
	char           *a, *b;
{
	return tb[(*a) % 13] - tb[(*b) % 13];
}

p_nine()
{
	int             bet, i, j, k, tmp, x, xx;
	srandom(time(0));
	while (1) {
		set_user_status(ST_M_NINE);
		bet = get_money(bet=0,"game/99.welcome");
		if (!bet)
			return 0;
//		move(1, 0);
                ansimore("game/99", NA);
/*
		fs = fopen("r");
		while (fgets(genbuf, 255, fs))
			prints(genbuf);
*/
		move(21, 0);
		clrtoeol();
		//% prints("[33;1m (←)(→)改变选牌  (↓)查询该牌作用 (SPCAE)(Enter)打牌 (Q)退出 [m");
		prints("[33;1m (\xa1\xfb)(\xa1\xfa)\xb8\xc4\xb1\xe4\xd1\xa1\xc5\xc6  (\xa1\xfd)\xb2\xe9\xd1\xaf\xb8\xc3\xc5\xc6\xd7\xf7\xd3\xc3 (SPCAE)(Enter)\xb4\xf2\xc5\xc6 (Q)\xcd\xcb\xb3\xf6 [m");
		show_money(bet,NULL,NA);

		for (i = 0; i < 52; i++)
			card[i] = 1;

		for (i = 0; i < 4; i++) {
			for (j = 0; j < 5; j++) {
				while (-1) {
					k = random() % 52;
					if (card[k]) {
						ahand[i][j] = k;
						card[k] = 0;
						break;
					}
				}
			}
		}

		qsort(ahand[0], 5, sizeof(char), forqp);
		x = xx = now = turn = sum = tmp = 0;
		dir = 1;
		live = 3;
		show_mycard(100);

		while (1) {
			move(8, 52);
			prints(tu[turn]);
			refresh();
			sum++;
			if (turn)
				//电脑
			{
				qsort(ahand[turn], 5, sizeof(char), forqp);
				for (i = 0; i < 5; i++) {
					tmp = ahand[turn][i] % 13;
					if (tmp == 0 || tmp == 4 || tmp == 5 || tmp > 9)
						break;
					if (now + tmp <= 99 && now + tmp >= 0)
						break;
				}
				if (i < 2)
					if (tmp == 0 || tmp == 4 || tmp == 5 || tmp > 9)
						i += random() % (5 - i);
				if (i == 5)
					cpu_die();
				else
					add(&(ahand[turn][i]));
				if (random() % 5 == 0)
					mmsg();
				continue;
			}
			if (!live) {
				//% //gamelog(NINE, "[32;1m在 %d 张牌赢了.. :)) [m %d", sum, bet);
				//gamelog(NINE, "[32;1m\xd4\xda %d \xd5\xc5\xc5\xc6\xd3\xae\xc1\xcb.. :)) [m %d", sum, bet);
				if (sum < 25)
					live = 20;
				else if (sum < 50)
					live = 15;
				else if (sum < 100)
					live = 10;
				else if (sum < 150)
					live = 7;
				else if (sum < 200)
					live = 5;
				else
					live = 3;
				inmoney(bet * (live + 1));
				//% sprintf(buf, "赢了 %d ... :D", bet * live);
				sprintf(buf, "\xd3\xae\xc1\xcb %d ... :D", bet * live);
				prints(buf);
				break;
			}
			tmp = ahand[0][4] % 13;
			if (tmp != 0 && tmp != 4 && tmp != 5 && tmp < 10 && now + tmp > 99) {
				//% prints("呜呜呜..被电爆了!!.. :~");
				prints("\xce\xd8\xce\xd8\xce\xd8..\xb1\xbb\xb5\xe7\xb1\xac\xc1\xcb!!.. :~");
				//% //game_log(NINE, "[31;1m在 %d 张牌被电脑电爆掉了.. :~ %d[m %d", sum, live, bet);
				//game_log(NINE, "[31;1m\xd4\xda %d \xd5\xc5\xc5\xc6\xb1\xbb\xb5\xe7\xc4\xd4\xb5\xe7\xb1\xac\xb5\xf4\xc1\xcb.. :~ %d[m %d", sum, live, bet);
				break;
			}
			while (tmp != 13 && tmp != 32)
				//% //人类
				//\xc8\xcb\xc0\xe0
			{
				move(18, xx * 4 + 30);
				outs("  ");
				move(18, (xx = x) * 4 + 30);

				if (tb[ahand[0][x] % 13] < 7) {
					if (ahand[0][x] % 13 + now > 99)
						//% outs("！");
						outs("\xa3\xa1");
					else
						//% outs("○");
						outs("\xa1\xf0");
				} else
					//% outs("★");
					outs("\xa1\xef");

				move(18, x * 4 + 31);
				refresh();

				switch (tmp = egetch()) {
#ifdef NINE_DEBUG
				case KEY_UP:
					//% getdata(22, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
					getdata(22, 0, "\xb0\xd1\xc5\xc6\xbb\xbb\xb3\xc9> ", genbuf, 3, DOECHO, YEA);
					card[ahand[0][x]] = 3;
					ahand[0][x] = atoi(genbuf);
					card[ahand[0][x]] = 0;
					qsort(ahand[0], 5, sizeof(char), forqp);
					show_mycard(100);
					break;
#endif
				case KEY_DOWN:
					nhelp(ahand[0][x]);
					break;
				case KEY_LEFT:
				case 'l':
                                move(18, 30);
                                outs("                                            ");
					x = x ? x - 1 : 4;
					break;
				case KEY_RIGHT:
				case 'r':
                                move(18, 30);
                                outs("                                            ");
					x = (x == 4) ? 0 : x + 1;
					break;
				case 'q':
					break;
				}
				if (tmp == 'q')
					return;
			}

			move(18, xx * 4 + 30);
			outs("  ");
			if (add(&(ahand[0][x]))) {
				//% prints("呜呜呜..白烂爆了!!.. :~");
				prints("\xce\xd8\xce\xd8\xce\xd8..\xb0\xd7\xc0\xc3\xb1\xac\xc1\xcb!!.. :~");
				//% //game_log(NINE, "[31;1m在 %d 张牌白烂爆了.. :~ %d[m %d", sum, live, bet);
				//game_log(NINE, "[31;1m\xd4\xda %d \xd5\xc5\xc5\xc6\xb0\xd7\xc0\xc3\xb1\xac\xc1\xcb.. :~ %d[m %d", sum, live, bet);
				break;
			}
			qsort(ahand[0], 5, sizeof(char), forqp);
			show_mycard(100);
		}
	}
}

show_mycard(t)
	char            t;
{
	char            i;
#ifdef NINE_DEBUG
	char            j;
#endif
	//% char           *suit[4] = {"Ｃ", "Ｄ", "Ｈ", "Ｓ"};
	char           *suit[4] = {"\xa3\xc3", "\xa3\xc4", "\xa3\xc8", "\xa3\xd3"};
	//% char           *num[13] = {"Ｋ", "Ａ", "２", "３", "４", "５", "６",
	char           *num[13] = {"\xa3\xcb", "\xa3\xc1", "\xa3\xb2", "\xa3\xb3", "\xa3\xb4", "\xa3\xb5", "\xa3\xb6",
	//% "７", "８", "９", "10", "Ｊ", "Ｑ"};
	"\xa3\xb7", "\xa3\xb8", "\xa3\xb9", "10", "\xa3\xca", "\xa3\xd1"};
	char            coorx[4] = {30, 38, 30, 22};
	char            coory[4] = {8, 6, 4, 6};

#ifdef NINE_DEBUG
	move(22, 0);
	for (i = 3; i > 0; i--) {
		if (ahand[i][0] == -1)
			continue;
		qsort(ahand[i], 5, sizeof(char), forqp);
		for (j = 0; j < 5; j++)
			prints(num[ahand[i][j] % 13]);
		prints("  ");
	}
#endif

	if (t == 100) {
		for (i = 0; i < 5; i++) {
			move(16, 30 + i * 4);
			prints(num[ahand[0][i] % 13]);
			move(17, 30 + i * 4);
			prints(suit[ahand[0][i] / 13]);
		}
		return;
	}
	move(coory[turn], coorx[turn]);
	//% prints("╭───╮");
	prints("\xa8\x71\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x72");
	move(coory[turn] + 1, coorx[turn]);
	//% prints("│%s    │", num[t % 13]);
	prints("\xa9\xa6%s    \xa9\xa6", num[t % 13]);
	move(coory[turn] + 2, coorx[turn]);
	//% prints("│%s    │", suit[t / 13]);
	prints("\xa9\xa6%s    \xa9\xa6", suit[t / 13]);
	move(coory[turn] + 3, coorx[turn]);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	move(coory[turn] + 4, coorx[turn]);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	//% //prints("│    %s│", num[t % 13]);
	//prints("\xa9\xa6    %s\xa9\xa6", num[t % 13]);
	move(coory[turn] + 5, coorx[turn]);
	//% prints("│      │");
	prints("\xa9\xa6      \xa9\xa6");
	//% //prints("│    %s│", suit[t / 13]);
	//prints("\xa9\xa6    %s\xa9\xa6", suit[t / 13]);
	move(coory[turn] + 6, coorx[turn]);
	//% prints("╰───╯");
	prints("\xa8\x74\xa9\xa4\xa9\xa4\xa9\xa4\xa8\x73");

	move(7, 50);
	//% prints("%s  %s", dir == 1 ? "↙" : "↗", dir == 1 ? "↖" : "↘");
	prints("%s  %s", dir == 1 ? "\xa8\x4c" : "\xa8\x4a", dir == 1 ? "\xa8\x49" : "\xa8\x4b");
	move(9, 50);
	//% prints("%s  %s", dir == 1 ? "↘" : "↖", dir == 1 ? "↗" : "↙");
	prints("%s  %s", dir == 1 ? "\xa8\x4b" : "\xa8\x49", dir == 1 ? "\xa8\x4a" : "\xa8\x4c");

	move(19, 52);
	//% prints("点数：%c%c%c%c", (now / 10) ? 162 : 32,
	prints("\xb5\xe3\xca\xfd\xa3\xba%c%c%c%c", (now / 10) ? 162 : 32,
	       (now / 10) ? (now / 10 + 175) : 32, 162, now % 10 + 175);
	move(20, 52);
	//% prints("张数：%d", sum);
	prints("\xd5\xc5\xca\xfd\xa3\xba%d", sum);
	refresh();
	sleep(1);
	move(21, 0);
	clrtoeol();
	refresh();
	//% prints("[33;1m (←)(→)改变选牌  (↓)查询该牌作用 (SPCAE)(Enter)打牌 [m");
	prints("[33;1m (\xa1\xfb)(\xa1\xfa)\xb8\xc4\xb1\xe4\xd1\xa1\xc5\xc6  (\xa1\xfd)\xb2\xe9\xd1\xaf\xb8\xc3\xc5\xc6\xd7\xf7\xd3\xc3 (SPCAE)(Enter)\xb4\xf2\xc5\xc6 [m");
}

int 
add(t)
	char           *t;
{
	int             k = 0, change = 0;

	switch (*t % 13) {
	case 4:
			dir = -dir;
		break;

	case 5:
			move(21, 0);
		clrtoeol();

		//% prints("指定那一家？ ");
		prints("\xd6\xb8\xb6\xa8\xc4\xc7\xd2\xbb\xbc\xd2\xa3\xbf ");
		for (change = 3; change >= 0; change--)
			if (turn != change && ahand[change][0] != -1)
				prints("(%s) ", tu[change]);

		change = 0;

		while (!change) {
			int             tmp;
			if (turn || live == 1)
				tmp = random() % 4 + 3;
			else
				tmp = egetch();

			if ((turn != 3 && ahand[3][0] != -1) && (tmp == KEY_LEFT || tmp == 6))
				change = 6;
			else if ((turn != 2 && ahand[2][0] != -1) && (tmp == KEY_UP || tmp == 5))
				change = 5;
			else if ((turn != 1 && ahand[1][0] != -1) && (tmp == KEY_RIGHT || tmp == 4))
				change = 4;
			else if ((turn != 0 && ahand[0][0] != -1) && tmp == 3)
				change = 3;
		}

		prints("[32;1m(%s)[m", tu[change - 3]);
		break;

	case 10:
		//10 加或减10
			ten_or_twenty(10);
		break;

	case 11:
		//J PASS
			break;

	case 12:
		//Q 加或减20
			ten_or_twenty(20);
		break;

	case 0:
		//K 马上变99
			now = 99;
		break;

	default:
		if ((now + (*t % 13)) > 99)
			return -1;
		else
			now += (*t % 13);
		break;
	}

	refresh();
	show_mycard(*t);
	while (-1) {
		k = random() % 52;
		if (card[k] == 1 && card[k] != 0) {
			card[*t] = 3;
			*t = k;
			card[k] = 0;
			break;
		} else
			card[k]--;
	}

	while (-1) {
		if (change) {
			turn = change - 3;
			break;
		} else {
			turn = (turn + 4 + dir) % 4;
			if (ahand[turn][0] > -1)
				break;
		}
	}
	return 0;
}

ten_or_twenty(t)
	int             t;
{
	if (now < t - 1)
		now += t;
	else if (now > 99 - t)
		now -= t;
	else {
		int             tmp = 0;
		move(21, 0);
		clrtoeol();
		//% prints("(←)(+)加%d  (→)(-)减%d  ", t, t);
		prints("(\xa1\xfb)(+)\xbc\xd3%d  (\xa1\xfa)(-)\xbc\xf5%d  ", t, t);

		while (!tmp) {
			if (turn)
				tmp = random() % 2 + 5;
			else
				tmp = egetch();
			switch (tmp) {
			case KEY_LEFT:
			case '+':
			case 5:
				now += t;
				//% prints("[32;1m加 %d[m", t);
				prints("[32;1m\xbc\xd3 %d[m", t);
				break;
			case KEY_RIGHT:
			case '-':
			case 6:
				now -= t;
				//% prints("[32;1m减 %d[m", t);
				prints("[32;1m\xbc\xf5 %d[m", t);
				break;
			default:
				tmp = 0;
			}
		}
	}
}

cpu_die()
{
	switch (turn) {
	case 1:
		move(9, 55);
		break;
	case 2:
		move(7, 52);
		break;
	case 3:
		move(9, 49);
		break;
	}
	prints("  ");
	live--;
	//% sprintf(buf, "电脑 %d 爆了!!! .. :DD", turn);
	sprintf(buf, "\xb5\xe7\xc4\xd4 %d \xb1\xac\xc1\xcb!!! .. :DD", turn);
	move(20,0);
	prints(buf);
	ahand[turn][0] = -1;
	while (-1) {
		turn = (turn + 4 + dir) % 4;
		if (ahand[turn][0] > -1)
			break;
	}
}

nhelp(t)
	int             t;
{
	t %= 13;
	switch (t) {
	case 0:
		//% prints(" 九九：点数马上变成９９ ");
		prints(" \xbe\xc5\xbe\xc5\xa3\xba\xb5\xe3\xca\xfd\xc2\xed\xc9\xcf\xb1\xe4\xb3\xc9\xa3\xb9\xa3\xb9 ");
		break;
	case 4:
		//% prints(" 回转：游戏进行方向相反 ");
		prints(" \xbb\xd8\xd7\xaa\xa3\xba\xd3\xce\xcf\xb7\xbd\xf8\xd0\xd0\xb7\xbd\xcf\xf2\xcf\xe0\xb7\xb4 ");
		break;
	case 5:
		//% prints(" 指定：自由指定下一个玩家 ");
		prints(" \xd6\xb8\xb6\xa8\xa3\xba\xd7\xd4\xd3\xc9\xd6\xb8\xb6\xa8\xcf\xc2\xd2\xbb\xb8\xf6\xcd\xe6\xbc\xd2 ");
		break;
	case 11:
		//% prints(" PASS：可PASS一次 ");
		prints(" PASS\xa3\xba\xbf\xc9PASS\xd2\xbb\xb4\xce ");
		break;
	case 10:
		//% prints(" 点数加或减 10 ");
		prints(" \xb5\xe3\xca\xfd\xbc\xd3\xbb\xf2\xbc\xf5 10 ");
		break;
	case 12:
		//% prints(" 点数加或减 20 ");
		prints(" \xb5\xe3\xca\xfd\xbc\xd3\xbb\xf2\xbc\xf5 20 ");
		break;
	default:
		//% sprintf(buf, " 点数加 %d ", t);
		sprintf(buf, " \xb5\xe3\xca\xfd\xbc\xd3 %d ", t);
		prints(buf);
		break;
	}
}

mmsg()
{
	char           *msg[12] = {
		//% "喂…打快一点啦！",
		"\xce\xb9\xa1\xad\xb4\xf2\xbf\xec\xd2\xbb\xb5\xe3\xc0\xb2\xa3\xa1",
		//% "打牌专心一点，不要流口水…",
		"\xb4\xf2\xc5\xc6\xd7\xa8\xd0\xc4\xd2\xbb\xb5\xe3\xa3\xac\xb2\xbb\xd2\xaa\xc1\xf7\xbf\xda\xcb\xae\xa1\xad",
		//% "不要看美眉…",
		"\xb2\xbb\xd2\xaa\xbf\xb4\xc3\xc0\xc3\xbc\xa1\xad",
		//% "大爷行行好，放过我吧…",
		"\xb4\xf3\xd2\xaf\xd0\xd0\xd0\xd0\xba\xc3\xa3\xac\xb7\xc5\xb9\xfd\xce\xd2\xb0\xc9\xa1\xad",
		//% "接招受死吧！！！",
		"\xbd\xd3\xd5\xd0\xca\xdc\xcb\xc0\xb0\xc9\xa3\xa1\xa3\xa1\xa3\xa1",
		//% "哇，你好厉害喔！竟然能打出这张牌！",
		"\xcd\xdb\xa3\xac\xc4\xe3\xba\xc3\xc0\xf7\xba\xa6\xe0\xb8\xa3\xa1\xbe\xb9\xc8\xbb\xc4\xdc\xb4\xf2\xb3\xf6\xd5\xe2\xd5\xc5\xc5\xc6\xa3\xa1",
		//% "你一定是赌徒转世的！",
		"\xc4\xe3\xd2\xbb\xb6\xa8\xca\xc7\xb6\xc4\xcd\xbd\xd7\xaa\xca\xc0\xb5\xc4\xa3\xa1",
		//% "last hand…",
		"last hand\xa1\xad",
		//% "再玩电爆你！",
		"\xd4\xd9\xcd\xe6\xb5\xe7\xb1\xac\xc4\xe3\xa3\xa1",
		//% "哇…你有胸肌咧..",
		"\xcd\xdb\xa1\xad\xc4\xe3\xd3\xd0\xd0\xd8\xbc\xa1\xdf\xd6..",
		//% "你的肚子跑出来了咧…",
		"\xc4\xe3\xb5\xc4\xb6\xc7\xd7\xd3\xc5\xdc\xb3\xf6\xc0\xb4\xc1\xcb\xdf\xd6\xa1\xad",
	//% "dsyan是好人…gwen是帅哥…"};
	"dsyan\xca\xc7\xba\xc3\xc8\xcb\xa1\xadgwen\xca\xc7\xcb\xa7\xb8\xe7\xa1\xad"};
	move(21, 0);
	clrtoeol();
	refresh();
	prints("[%d;1m%s[m", random() % 7 + 31, msg[random() % 12]);
}



/* bingo */

int
bingo()
{

	int             place[5][5][3] = {{{3, 2, 0}, {3, 6, 0}, {3, 10, 0}, {3, 14, 0}, {3, 18, 0}},
	{{5, 2, 0}, {5, 6, 0}, {5, 10, 0}, {5, 14, 0}, {5, 18, 0}},
	{{7, 2, 0}, {7, 6, 0}, {7, 10, 0}, {7, 14, 0}, {7, 18, 0}},
	{{9, 2, 0}, {9, 6, 0}, {9, 10, 0}, {9, 14, 0}, {9, 18, 0}},
	{{11, 2, 0}, {11, 6, 0}, {11, 10, 0}, {11, 14, 0}, {11, 18, 0}}};

	int             account, bet, i = 0, j = 0, k = 0, used[25] = {0}, ranrow, rancol, line = 0, pp = 0;
	char            co[2];
	int             may = 20, money;
	char	buf[200];
	set_user_status(ST_M_BINGO);

	while (1) {
		account = 13;
		for (i = 0; i < 25; i++)
			used[i] = 0;
		for (i = 0; i < 5; i++)
			for (j = 0; j < 5; j++)
				place[i][j][2] = 0;
		i = 0;
		j = 0;
		k = 0;
		line = 0;
		pp = 0;

		money = get_money(0,"game/bingo.welcome");
		if (!money)
			return 0;
		while (account >= 0) {
			screen_clear();
			prints("\n\n");
			j = 0;
			for (i = 0; i <= 10; i++) {
				if (i % 2 == 0)
					//% outs("   [1;34;40m■■■■■■■■■■■[0m\n");
					outs("   [1;34;40m\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6\xa1\xf6[0m\n");
				else {
					//% outs("   [1;34;40m■");
					outs("   [1;34;40m\xa1\xf6");
					if (place[j][0][2])
						prints("[1;37;40m%2d", place[j][0][2]);
					else
						//% prints("[1;32m■");
						prints("[1;32m\xa1\xf6");
					//% outs("[1;34;40m■");
					outs("[1;34;40m\xa1\xf6");
					if (place[j][1][2])
						prints("[1;37;40m%2d", place[j][1][2]);
					else
						//% prints("[1;32m■");
						prints("[1;32m\xa1\xf6");
					//% outs("[1;34;40m■");
					outs("[1;34;40m\xa1\xf6");
					if (place[j][2][2])
						prints("[1;37;40m%2d", place[j][2][2]);
					else
						//% prints("[1;32m■");
						prints("[1;32m\xa1\xf6");
					//% outs("[1;34;40m■");
					outs("[1;34;40m\xa1\xf6");
					if (place[j][3][2])
						prints("[1;37;40m%2d", place[j][3][2]);
					else
						//% prints("[1;32m■");
						prints("[1;32m\xa1\xf6");
					//% outs("[1;34;40m■");
					outs("[1;34;40m\xa1\xf6");
					if (place[j][4][2])
						prints("[1;37;40m%2d", place[j][4][2]);
					else
						//% prints("[1;32m■");
						prints("[1;32m\xa1\xf6");
					//% outs("[1;34;40m■[0m\n");
					outs("[1;34;40m\xa1\xf6[0m\n");
					j++;
				}
			}
			prints("\n\n\n\n");
			//% prints("[1;37;44m尚未开出的号码[0m\n");
			prints("[1;37;44m\xc9\xd0\xce\xb4\xbf\xaa\xb3\xf6\xb5\xc4\xba\xc5\xc2\xeb[0m\n");
			for (i = 1; i <= 25; i++) {
				pp += 3;
				if (used[i - 1] == 0)
					prints(" %2d", i);
			}
			if (line == 1 || account == 1)
				break;

			if (account > 9)
				may = 20;
			else if (account == 9)
				may = 10;
			else if (account == 8)
				may = 9;
			else if (account == 7)
				may = 8;
			else if (account == 6)
				may = 7;
			else if (account == 5)
				may = 6;
			else if (account == 4)
				may = 5;
			else if (account == 3)
				may = 3;
			else if (account == 2)
				may = 1;


			//% prints("\n尚有[1;33;41m %2d [0m次机会可猜 下次猜中可得[1;37;44m %d [0m倍 \n", account - 1, may);
			prints("\n\xc9\xd0\xd3\xd0[1;33;41m %2d [0m\xb4\xce\xbb\xfa\xbb\xe1\xbf\xc9\xb2\xc2 \xcf\xc2\xb4\xce\xb2\xc2\xd6\xd0\xbf\xc9\xb5\xc3[1;37;44m %d [0m\xb1\xb6 \n", account - 1, may);
			show_money(money,NULL,NA);
			//% getdata(20, 0, "请输入您的号码(按 Q 表示认输) : ", co, 3, DOECHO, YEA);
			getdata(20, 0, "\xc7\xeb\xca\xe4\xc8\xeb\xc4\xfa\xb5\xc4\xba\xc5\xc2\xeb(\xb0\xb4 Q \xb1\xed\xca\xbe\xc8\xcf\xca\xe4) : ", co, 3, DOECHO, YEA);

                        if ( co[0] == 'q' || co[0] == 'Q' ) {
                                line = 0;
				break;
      			}
			bet = atoi(co);
			if (bet <= 0 || bet > 25 || used[bet - 1] != 0)
				continue;
			used[bet - 1] = 1;
			srandom(time(0));
			ranrow = random();
			ranrow = ranrow % 5;
			rancol = random();
			rancol = rancol % 5;

			while (place[ranrow][rancol][2] != 0) {
				ranrow = random();
				ranrow = ranrow % 5;
				rancol = random();
				rancol = rancol % 5;
			}
			place[ranrow][rancol][2] = bet;
			account--;

			for (i = 0; i < 5; i++) {
				if (place[i][0][2] != 0 && place[i][1][2] != 0 && place[i][2][2] != 0 && place[i][3][2] != 0 && place[i][4][2] != 0) {
					line = 1;
				}
			}
			for (i = 0; i < 5; i++) {
				if (place[0][i][2] != 0 && place[1][i][2] != 0 && place[2][i][2] != 0 && place[3][i][2] != 0 && place[4][i][2] != 0) {
					line = 1;
				}
			}

			if (place[0][0][2] != 0 && place[1][1][2] != 0 && place[3][3][2] != 0 && place[4][4][2] != 0 && place[2][2][2] != 0)
				line = 1;
			if (place[0][4][2] != 0 && place[1][3][2] != 0 && place[2][2][2] != 0 && place[3][2][2] != 0 && place[0][4][2] != 0)
				line = 1;

		}
		move(21,0);
		if (line == 1) {
			//% prints("恭喜你...赢了 %d 元 ", money * may);
			prints("\xb9\xa7\xcf\xb2\xc4\xe3...\xd3\xae\xc1\xcb %d \xd4\xaa ", money * may);
			inmoney((money * may) + money);
			//% sprintf(buf,"赚了 %d 元 ", money * may );
			sprintf(buf,"\xd7\xac\xc1\xcb %d \xd4\xaa ", money * may );
		} else {
			//% prints("运气不佳...再来一盘吧!!");
			prints("\xd4\xcb\xc6\xf8\xb2\xbb\xbc\xd1...\xd4\xd9\xc0\xb4\xd2\xbb\xc5\xcc\xb0\xc9!!");
			//% sprintf(buf,"输掉了 %d 元", money);
			sprintf(buf,"\xca\xe4\xb5\xf4\xc1\xcb %d \xd4\xaa", money);
		}
		gamelog(buf);
		pressanykey();
	}
	return 0;
}
