#include "bbs.h"
#define MSG_SEPERATOR   "\
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª"

/* ºÚ½Ü¿ËÓÎÏ· */
int 
BlackJack()
{
	int             num[52] = {11, 11, 11, 11, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
		7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10};
	int             cardlist[52] = {0};
	int             i, j, m, tmp = 0, tmp2, ch;
	int             win = 2, win_jack = 5;	/* win ÎªÓ®Ê±µÄ±¶ÂÊ, win_jack
						 * 1 µã±¶ÂÊ */
	int             six = 10, seven = 20, aj = 10, super_jack = 20;	/* 777, A+J, spade A+J µÄ±¶ÂÊ */
	int             host_count = 2, guest_count = 1, card_count = 3,
	                A_count = 0, AA_count = 0;
	int             host_point = 0, guest_point = 0, mov_y = 4;
	int             host_card[12] = {0}, guest_card[12] = {0};
	long int        money;

	int             CHEAT = 0;	/* ×ö±×²ÎÊý, 1 ¾Í×÷±×, 0 ¾Í²»×÷ */

	modify_user_mode(M_BLACKJACK);
	money = get_money(0,"game/blackjack.welcome");
	if(!money) return 0;
	move(1, 0);
	prints("¡¾ºÚ½Ü¿Ë¡¿ÓÎÏ·  [°´ y ÐøÅÆ, n ²»ÐøÅÆ, d double, q ÈÏÊäÍË³ö]");
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
		cardlist[i]--;	/* Ï´ÅÆ */

	if (money >= 20000)
		CHEAT = 1;
	if (CHEAT == 1) {
		if (cardlist[1] <= 3) {
			tmp2 = cardlist[50];
			cardlist[50] = cardlist[1];
			cardlist[1] = tmp2;
		}
	}			/* ×÷±×Âë */
	host_card[0] = cardlist[0];
	if (host_card[0] < 4)
		AA_count++;
	guest_card[0] = cardlist[1];

	if (guest_card[0] < 4)
		A_count++;
	host_card[1] = cardlist[2];
	if (host_card[1] < 4)
		AA_count++;	/* ·¢Ç°ÈýÕÅÅÆ */

	move(5, 0);
	prints("¨q©¤©¤©¤¨r");
	move(6, 0);
	prints("©¦      ©¦");
	move(7, 0);
	prints("©¦      ©¦");
	move(8, 0);
	prints("©¦      ©¦");
	move(9, 0);
	prints("©¦      ©¦");
	move(10, 0);
	prints("©¦      ©¦");
	move(11, 0);
	prints("¨t©¤©¤©¤¨s");
	print_card(host_card[1], 5, 4);
	print_card(guest_card[0], 15, 0);	/* Ó¡³öÇ°ÈýÕÅÅÆ */

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
			prints("[1;41;33m     £·£·£·     [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33m£·£·£· !!! µÃ½±½ð %d ÒøÁ½[m", money * seven);
			prints(genbuf);
			inmoney(money * seven);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}
		if ((guest_card[0] == 40 && guest_card[1] == 0) || (guest_card[0] == 0 && guest_card[1] == 40)) {
			move(18, 3);
			prints("[1;41;33m ³¬¼¶ÕýÍ³ BLACK JACK  [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33m³¬¼¶ÕýÍ³ BLACK JACK !!! µÃ½±½ð %d ÒøÁ½[m", money * super_jack);
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
			sprintf(genbuf,"[1;41;33mSUPER BLACK JACK !!! µÃ½±½ð %d ÒøÁ½[m", money * aj);
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
			sprintf(genbuf,"[1;41;33mBLACK JACK !!![44m µÃ½±½ð %d ÒøÁ½[m", money * win_jack);
			prints(genbuf);
			inmoney(money * win_jack);
			gamelog(genbuf);
			pressanykey();
			return 0;
		}		/* Ç°Á½ÕÅ¾Í 21 µã */
		if (guest_point > 21) {
			if (A_count > 0) {
				guest_point -= 10;
				A_count--;
			};
		}
		move(19, 0);
		//clrtoeol();
		prints("[1;32mµãÊý: [33m%d[m", host_point);
		move(20, 0);
		//clrtoeol();
		prints("[1;32mµãÊý: [33m%d[m", guest_point);
		if (guest_point > 21) {
			move(20, 0);
			//clrtoeol();
			prints("  ±¬µôÀ²~~~  ");
			pressanykey();
			return 0;
		}
		if (guest_count == 5) {
			move(18, 3);
			prints("[1;41;33m            ¹ýÁù¹Ø            [m");
			move(3, 0);
			sprintf(genbuf,"[1;41;33m¹ýÁù¹Ø !!! µÃ½±½ð %d ÒøÁ½[m", money * six);
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
                      && m != 0 );	/* ×¥ key */

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
		prints("[1;32mµãÊý: [33m%d[m", host_point);
		move(20, 0);
		//clrtoeol();
		prints("[1;32mµãÊý: [33m%d[m", guest_point);
		if (host_point > 21) {
			move(20, 0);
			//clrtoeol();
			prints("[1;32mµãÊý: [33m%d [1;41;33m WINNER [m", guest_point);

			move(3, 0);
			sprintf(genbuf,"Ó®ÁË~~~~ µÃ½±½ð %d ÒøÁ½", money * win);
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

	sprintf(genbuf,"ÊäÁË~~~~ Ã»ÊÕ %d ÒøÁ½!", money);
	prints(genbuf);
	gamelog(genbuf);
        pressanykey();
	return 0;
}


int 
print_card(int card, int x, int y)
{
	char           *flower[4] = {"£Ó", "£È", "£Ä", "£Ã"};
	char           *poker[52] = {"£Á", "£Á", "£Á", "£Á", "£²", "£²", "£²", "£²", "£³", "£³", "£³", "£³",
		"£´", "£´", "£´", "£´", "£µ", "£µ", "£µ", "£µ", "£¶", "£¶", "£¶", "£¶",
		"£·", "£·", "£·", "£·", "£¸", "£¸", "£¸", "£¸", "£¹", "£¹", "£¹", "£¹",
		"10", "10", "10", "10", "£Ê", "£Ê", "£Ê", "£Ê", "£Ñ", "£Ñ", "£Ñ", "£Ñ",
	"£Ë", "£Ë", "£Ë", "£Ë"};

	move(x, y);
	prints("¨q©¤©¤©¤¨r");
	move(x + 1, y);
	prints("©¦%s    ©¦", poker[card]);
	move(x + 2, y);
	prints("©¦%s    ©¦", flower[card % 4]);
	move(x + 3, y);
	prints("©¦      ©¦");
	move(x + 4, y);
	prints("©¦      ©¦");
	move(x + 5, y);
	prints("©¦      ©¦");
	move(x + 6, y);
	prints("¨t©¤©¤©¤¨s");
	return 0;
}




int
gagb()
{
	int             money;
	char            genbuf[200], buf[80];
	char            ans[5] = "";
	/* ±¶ÂÊ        0  1   2   3   4   5   6   7   8   9   10 */
	float           bet[11] = {0, 100, 50, 10, 3, 1.5, 1.2, 0.9, 0.8, 0.5, 0.1};
	int             a, b, c, count;

	modify_user_mode(M_XAXB);
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
			getdata(5, 0, "Çë²Â[q - ÍË³ö] ¡ú ", genbuf, 5, DOECHO, YEA);
			if (!strcmp(genbuf, "Good")) {
				prints("[%s]", ans);
				sprintf(genbuf,"²ÂÊý×Ö×÷±×, ÏÂ×¢ %d Ôª", money);
				gamelog(genbuf);
				igetch();
			}
			if ( genbuf[0] == 'q' || genbuf[0] == 'Q' ) {
				sprintf(buf,"·ÅÆú²Â²â, ¿Û³ýÑ¹×¢½ð¶î %d Ôª.", money);
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
				prints("ÊäÈëÊý×ÖÓÐÎÊÌâ!!");
				pressanykey();
				move ( 18,3 );
				prints("                ");
			}
		} while (!genbuf[0]);
		move(count + 6, 0);
		prints("  [1;31mµÚ [37m%2d [31m´Î£º [37m%s  ->  [33m%dA [36m%dB [m", count, genbuf, an(genbuf, ans), bn(genbuf, ans));
		if (an(genbuf, ans) == 4)
			break;
	}

	if (count > 10) {
		sprintf(buf, "ÄãÊäÁËßÏ£¡ÕýÈ·´ð°¸ÊÇ %s£¬ÏÂ´ÎÔÙ¼ÓÓÍ°É!!", ans);
		sprintf(genbuf,"[1;31m¿ÉÁ¯Ã»²Âµ½£¬ÊäÁË %d Ôª£¡[m", money);
		gamelog(genbuf);
	} else {
		int             oldmoney = money;
		money *= bet[count];
		inmoney(money);
		if (money - oldmoney > 0)
			sprintf(buf, "¹§Ï²£¡×Ü¹²²ÂÁË %d ´Î£¬¾»×¬½±½ð %d Ôª", count, money - oldmoney);
		else if (money - oldmoney == 0)
			sprintf(buf, "°¦¡«¡«×Ü¹²²ÂÁË %d ´Î£¬Ã»ÊäÃ»Ó®£¡", count);
		else
			sprintf(buf, "°¡¡«¡«×Ü¹²²ÂÁË %d ´Î£¬ÅâÇ® %d Ôª£¡", count, oldmoney - money);
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
 * ³ÌÊ½Éè¼Æ£ºwsyfish ×Ô¼ºÆÀÓï£ºÐ´µÃºÜÀÃ£¬ÂÒÐ´Ò»Í¨£¬Ã»É¶Éî¶È:)
 * ÏàÈÝ³Ì¶È£ºPtt°å±¾Ó¦¸Ã¶¼ÐÐ°É£¬¾ÍÓÃinmoneyºÍdemoney£¬ÆäËûÈçSob¾ÍÒª¸ÄÒ»ÏÂÂÞ
 * */
char           *dice[6][3] = {"        ",
	"   ¡ñ   ",
	"        ",
	"   ¡ñ   ",
	"        ",
	"   ¡ñ   ",
	"   ¡ñ   ",
	"   ¡ñ   ",
	"   ¡ñ   ",
	"¡ñ    ¡ñ",
	"        ",
	"¡ñ    ¡ñ",
	"¡ñ    ¡ñ",
	"   ¡ñ   ",
	"¡ñ    ¡ñ",
	"¡ñ    ¡ñ",
	"¡ñ    ¡ñ",
	"¡ñ    ¡ñ"
};

int
x_dice()
{
	char            choice[11], buf[60];
	int             i, money;
	char            tmpchar;/* ¼ÍÂ¼Ñ¡Ïî */
	char            tmpdice[3];	/* Èý¸ö÷»×ÓµÄÖµ */
	char            totaldice;
	time_t          now = time(0);

	srandom(time(0));
	time(&now);

	modify_user_mode(M_DICE);
	while (1) {
		money = get_money(0,"game/xdice.welcome");
		if(!money) return 0;
		move(2,0);
		outs("\n©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´\n"
		     "©¦ £²±¶  1. ´ó      2. Ð¡                                                 ©¦\n"
		     "©¦ £µ±¶  3. Èýµã    4. ËÄµã     5. Îåµã    6. Áùµã    7. Æßµã    8. °Ëµã  ©¦\n"
		     "©¦       9. ¾Åµã   10. Ê®µã    11. Ê®Ò»µã 12. Ê®¶þµã 13. Ê®Èýµã 14. Ê®ËÄµã©¦\n"
		     "©¦      15. Ê®Îåµã 16. Ê®Áùµã  17. Ê®Æßµã 18. Ê®°Ëµã                      ©¦\n"
		     "©¦ £¹±¶ 19. Ò»Ò»Ò» 20. ¶þ¶þ¶þ  21. ÈýÈýÈý 22. ËÄËÄËÄ 23. ÎåÎåÎå 24. ÁùÁùÁù©¦\n"
		     "©¸©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¼\n");
		getdata(11, 0, "ÒªÑºÄÄÒ»ÏîÄØ£¿(ÇëÊäÈëºÅÂë) ", choice, 3, DOECHO, YEA);
		tmpchar = atoi(choice);
		if (tmpchar <= 0 || tmpchar > 24) {
			prints("ÒªÑºµÄÏîÄ¿ÊäÈëÓÐÎó£¡Àë¿ª¶Ä³¡");
			pressanykey();
			break;
		}
		outs("\n°´ÈÎÒ»¼üÖÀ³ö÷»×Ó....\n");
		egetch();

		do {
			totaldice = 0;
			for (i = 0; i < 3; i++) {
				tmpdice[i] = random() % 6 + 1;
				totaldice += tmpdice[i];
			}

			if (((tmpchar == 1) && totaldice > 10) ||
			    ((tmpchar == 2) && totaldice <= 10)) {
				if ((random() % 10) < 7)	/* ×÷±×ÓÃ£¬ÖÐ½±ÂÊÎªÔ­À´Ö®70% */
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
		outs("\n¨q©¤©¤©¤©¤¨r¨q©¤©¤©¤©¤¨r¨q©¤©¤©¤©¤¨r\n");

		for (i = 0; i < 3; i++)
			prints("©¦%s©¦©¦%s©¦©¦%s©¦\n",
			       dice[tmpdice[0] - 1][i],
			       dice[tmpdice[1] - 1][i],
			       dice[tmpdice[2] - 1][i]);

		outs("¨t©¤©¤©¤©¤¨s¨t©¤©¤©¤©¤¨s¨t©¤©¤©¤©¤¨s\n\n");

		if ((tmpchar == 1 && totaldice > 10)
		    || (tmpchar == 2 && totaldice <= 10))	/* ´¦Àí´óÐ¡ */
			sprintf(buf, "ÖÐÁË£¡µÃµ½£²±¶½±½ð %d Ôª£¬×Ü¹²ÓÐ %d Ôª",
				money * 2, inmoney(money * 2));
		else if (tmpchar <= 18 && totaldice == tmpchar)	/* ´¦Àí×ÜºÍ */
			sprintf(buf, "ÖÐÁË£¡µÃµ½£µ±¶½±½ð %d Ôª£¬×Ü¹²ÓÐ %d Ôª",
				money * 5, inmoney(money * 5));
		else if ((tmpchar - 18) == tmpdice[0] && (tmpdice[0] == tmpdice[1])
			 && (tmpdice[1] == tmpdice[2]))	/* ´¦ÀíÈý¸öÒ»Ñù×ÜºÍ */
			sprintf(buf, "ÖÐÁË£¡µÃµ½£¹±¶½±½ð %d Ôª£¬×Ü¹²ÓÐ %d Ôª",
				money * 9, inmoney(money * 9));

		else		/* ´¦ÀíÃ»ÖÐ */
			sprintf(buf, "ºÜ¿ÉÏ§Ã»ÓÐÑºÖÐ£¡ÊäÁË %d Ôª",money);
		gamelog(buf);
		prints(buf);
		pressanykey();
	}
	return 0;
}



/* write by dsyan               */
/* 87/10/24                     */
/* Ìì³¤µØ¾Ã Forever.twbbs.org   */

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
	modify_user_mode(M_GP);
	bet = 0;
	while (1) {
		clear();
		ans[0] = 0;
		if (cont == 0) 
			if(!(bet = get_money(bet,"game/gp.welcome"))) return 0;
		move(21, 0);
		clrtoeol();
		if (cont > 0)
			prints("[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (SPCAE)¸Ä±ä»»ÅÆ  (Enter)È·¶¨[m");
		else
			prints("[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (d)Double  (SPCAE)¸Ä±ä»»ÅÆ  (Enter)È·¶¨[m");
		show_money(bet,NULL,NA);

		for (i = 0; i < 52; i++)
			card[i] = i;	/* 0~51 ..ºÚ½Ü¿ËÊÇ 1~52 */

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
				outs(hold[i] < 0 ? "±£" : "  ");
				move(17, i * 4 + 2);
				outs(hold[i] < 0 ? "Áô" : "  ");
			}
			move(8, xx * 4 + 2);
			outs("  ");
			move(8, x * 4 + 2);
			outs("¡ý");
			move(t_lines - 1, 0);
			refresh();
			xx = x;

			tmp = egetch();
			switch (tmp) {
#ifdef GP_DEBUG
			case KEY_UP:
				getdata(21, 0, "°ÑÅÆ»»³É> ", genbuf, 3, DOECHO, YEA);
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
					prints("[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (SPCAE)¸Ä±ä»»ÅÆ  (Enter)È·¶¨[m");
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
			getdata(21, 0, "°ÑÅÆ»»³É> ", genbuf, 3, DOECHO, YEA);
			cpucard[x] = atoi(genbuf);
		}
		qsort(cpucard, 5, sizeof(char), forq);
		for (i = 0; i < 5; i++)
			show_card(1, cpucard[i], i);
#endif
		i = gp_win();

		if (i < 0) {
			inmoney(bet * 2);
			sprintf(genbuf, "ÍÛ!!ºÃ°ôà¸!!¾»×¬ %d ÔªßÖ.. :DDD", bet);
			prints(genbuf);
			if (cont > 0)
				sprintf(genbuf, "Á¬Ê¤ %d ´Î, Ó®ÁË %d Ôª",
					cont + 1, bet);
			else
				sprintf(genbuf, "Ó®ÁË %d Ôª", bet);
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
			sprintf(genbuf, "ÍÛ!!ºÃ°ôà¸!!¾»×¬ %d ÔªßÖ.. :DDD", bet * 2 * doub - bet);
			prints(genbuf);
			if (cont > 0)
				sprintf(genbuf, "Á¬Ê¤ %d ´Î, Ó®ÁË %d Ôª",
				   cont + 1, bet * doub);
			else
				sprintf(genbuf, "Ó®ÁË %d Ôª", bet * doub);
			gamelog(genbuf);
			bet = (bet > 50000 ? 100000 : bet * 2 * doub);
			gw = 1;
			bet = (bet >= 100000 ? 100000 : bet);
		} else {
			prints("ÊäÁË..:~~~");
			if (cont > 1)
				sprintf(genbuf, "ÖÐÖ¹ %d Á¬Ê¤, ÊäÁË %d Ôª", cont, bet);
			else
				sprintf(genbuf, "ÊäÁË %d Ôª", bet);
			gamelog(genbuf);
			cont = 0;
			bet = 0;
			pressanykey();
		}

		if (gw == 1) {
			gw = 0;
			getdata(21, 0, "ÄúÒª°Ñ½±½ð¼ÌÐøÑ¹×¢Âð (y/n)?", ans, 2, DOECHO, YEA);
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
	char           *suit[4] = {"£Ã", "£Ä", "£È", "£Ó"};
	char           *num[13] = {"£Ë", "£Á", "£²", "£³", "£´", "£µ", "£¶",
	"£·", "£¸", "£¹", "10", "£Ê", "£Ñ"};

	beginL = (isDealer) ? 2 : 12;
	move(beginL, x * 4);
	outs("¨q©¤©¤©¤¨r");
	move(beginL + 1, x * 4);
	prints("©¦%2s    ©¦", num[c % 13]);
	move(beginL + 2, x * 4);
	prints("©¦%2s    ©¦", suit[c / 13]);	/* ¡ûÕâÀï¸úºÚ½Ü¿Ë */
#ifdef GP_DEBUG
	move(beginL + 3, x * 4);
	prints("©¦%2d    ©¦", c);	/* ÓÐµã²»Í¬à¸!! */
#else
	move(beginL + 3, x * 4);
	outs("©¦      ©¦");	/* ÓÐµã²»Í¬à¸!! */
#endif
	move(beginL + 4, x * 4);
	outs("©¦      ©¦");
	move(beginL + 5, x * 4);
	outs("©¦      ©¦");
	move(beginL + 6, x * 4);
	outs("¨t©¤©¤©¤¨s");
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
		outs(hold[i] < 0 ? "±£" : "  ");
		move(7, i * 4 + 2);
		outs(hold[i] < 0 ? "Áô" : "  ");
	}
	move(10,25);
	prints("[44;37mµçÄÔ»»ÅÆÇ°...[40m");
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

//Í¬»¨Ë³¡¢ÌúÖ¦¡¢ºù¡¢Í¬»¨¡¢Ë³¡¢ÈýÌõ¡¢ÍÃÅß¡¢Åß¡¢Ò»Ö»
int
complex(cc, x, y)
	char           *cc, *x, *y;
{
	char            p[13], q[5], r[4];
	char            a[5], b[5], c[5], d[5];
	int             i, j, k;

	tran(a, b, cc);
	check(p, q, r, cc);

	/* Í¬»¨Ë³ */
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
	/* ËÄÕÅ */
	if (q[4] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 4)
				*x = i ? i : 13;
		return 2;
	}
	/* ºùÂ« */
	if (q[3] == 1 && q[2] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 3)
				*x = i ? i : 13;
		return 3;
	}
	/* Í¬»¨ */
	for (i = 0; i < 4; i++)
		if (r[i] == 5) {
			*x = i;
			return 4;
		}
	/* Ë³×Ó */
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
	/* ÈýÌõ */
	if (q[3] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 3) {
				*x = i ? i : 13;
				return 6;
			}
	/* ÍÃÅß */
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
	/* Åß */
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
	/* Ò»ÕÅ */
	*x = 0;
	*y = 0;
	for (i = 0; i < 5; i++)
		if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1) {
			*x = a[i];
			*y = b[i];
		}
	return 9;
}

/* a ÊÇµãÊý .. b ÊÇ»¨É« */
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

//Í¬»¨Ë³¡¢ÌúÖ¦¡¢ºù¡¢Í¬»¨¡¢Ë³¡¢ÈýÌõ¡¢ÍÃÅß¡¢Åß¡¢Ò»Ö»
show_style(my, cpu)
	char            my, cpu;
{
	char           *style[9] = {"Í¬»¨Ë³", "ËÄÕÅ", "ºùÂ«", "Í¬»¨", "Ë³×Ó",
	"ÈýÌõ", "ÍÃÅß", "µ¥Åß", "Ò»ÕÅ"};
	move(5, 26);
	prints("[41;37;1m%s[m", style[cpu - 1]);
	move(15, 26);
	prints("[41;37;1m%s[m", style[my - 1]);
	sprintf(sty, "ÎÒµÄÅÆ[44;1m%s[m..µçÄÔµÄÅÆ[44;1m%s[m",
		style[my - 1], style[cpu - 1]);
}

/********************************/
/* BBS Õ¾ÄÚÓÎÏ·¨CÌìµØ¾Å¾Å       */
/* 11/26/98 */
/* dsyan.bbs@Forever.twbbs.org  */
/********************************/

#undef  NINE_DEBUG

//0 1 2 3 4 5 6 7 8 9 10 11 12	/* µçÄÔ AI ËùÔÚ */
// char         cp[13] = {9, 8, 7, 6, 3, 2, 1, 0, 11, 5, 4, 10, 12};
char            tb[13] = {7, 6, 5, 4, 10, 9, 3, 2, 1, 0, 11, 8, 12};
char           *tu[4] = {"¡ý", "¡ú", "¡ü", "¡û"};
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
		modify_user_mode(M_NINE);
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
		prints("[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (¡ý)²éÑ¯¸ÃÅÆ×÷ÓÃ (SPCAE)(Enter)´òÅÆ (Q)ÍË³ö [m");
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
				//µçÄÔ
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
				//gamelog(NINE, "[32;1mÔÚ %d ÕÅÅÆÓ®ÁË.. :)) [m %d", sum, bet);
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
				sprintf(buf, "Ó®ÁË %d ... :D", bet * live);
				prints(buf);
				break;
			}
			tmp = ahand[0][4] % 13;
			if (tmp != 0 && tmp != 4 && tmp != 5 && tmp < 10 && now + tmp > 99) {
				prints("ÎØÎØÎØ..±»µç±¬ÁË!!.. :~");
				//game_log(NINE, "[31;1mÔÚ %d ÕÅÅÆ±»µçÄÔµç±¬µôÁË.. :~ %d[m %d", sum, live, bet);
				break;
			}
			while (tmp != 13 && tmp != 32)
				//ÈËÀà
			{
				move(18, xx * 4 + 30);
				outs("  ");
				move(18, (xx = x) * 4 + 30);

				if (tb[ahand[0][x] % 13] < 7) {
					if (ahand[0][x] % 13 + now > 99)
						outs("£¡");
					else
						outs("¡ð");
				} else
					outs("¡ï");

				move(18, x * 4 + 31);
				refresh();

				switch (tmp = egetch()) {
#ifdef NINE_DEBUG
				case KEY_UP:
					getdata(22, 0, "°ÑÅÆ»»³É> ", genbuf, 3, DOECHO, YEA);
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
				prints("ÎØÎØÎØ..°×ÀÃ±¬ÁË!!.. :~");
				//game_log(NINE, "[31;1mÔÚ %d ÕÅÅÆ°×ÀÃ±¬ÁË.. :~ %d[m %d", sum, live, bet);
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
	char           *suit[4] = {"£Ã", "£Ä", "£È", "£Ó"};
	char           *num[13] = {"£Ë", "£Á", "£²", "£³", "£´", "£µ", "£¶",
	"£·", "£¸", "£¹", "10", "£Ê", "£Ñ"};
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
	prints("¨q©¤©¤©¤¨r");
	move(coory[turn] + 1, coorx[turn]);
	prints("©¦%s    ©¦", num[t % 13]);
	move(coory[turn] + 2, coorx[turn]);
	prints("©¦%s    ©¦", suit[t / 13]);
	move(coory[turn] + 3, coorx[turn]);
	prints("©¦      ©¦");
	move(coory[turn] + 4, coorx[turn]);
	prints("©¦      ©¦");
	//prints("©¦    %s©¦", num[t % 13]);
	move(coory[turn] + 5, coorx[turn]);
	prints("©¦      ©¦");
	//prints("©¦    %s©¦", suit[t / 13]);
	move(coory[turn] + 6, coorx[turn]);
	prints("¨t©¤©¤©¤¨s");

	move(7, 50);
	prints("%s  %s", dir == 1 ? "¨L" : "¨J", dir == 1 ? "¨I" : "¨K");
	move(9, 50);
	prints("%s  %s", dir == 1 ? "¨K" : "¨I", dir == 1 ? "¨J" : "¨L");

	move(19, 52);
	prints("µãÊý£º%c%c%c%c", (now / 10) ? 162 : 32,
	       (now / 10) ? (now / 10 + 175) : 32, 162, now % 10 + 175);
	move(20, 52);
	prints("ÕÅÊý£º%d", sum);
	refresh();
	sleep(1);
	move(21, 0);
	clrtoeol();
	refresh();
	prints("[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (¡ý)²éÑ¯¸ÃÅÆ×÷ÓÃ (SPCAE)(Enter)´òÅÆ [m");
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

		prints("Ö¸¶¨ÄÇÒ»¼Ò£¿ ");
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
		//10 ¼Ó»ò¼õ10
			ten_or_twenty(10);
		break;

	case 11:
		//J PASS
			break;

	case 12:
		//Q ¼Ó»ò¼õ20
			ten_or_twenty(20);
		break;

	case 0:
		//K ÂíÉÏ±ä99
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
		prints("(¡û)(+)¼Ó%d  (¡ú)(-)¼õ%d  ", t, t);

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
				prints("[32;1m¼Ó %d[m", t);
				break;
			case KEY_RIGHT:
			case '-':
			case 6:
				now -= t;
				prints("[32;1m¼õ %d[m", t);
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
	sprintf(buf, "µçÄÔ %d ±¬ÁË!!! .. :DD", turn);
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
		prints(" ¾Å¾Å£ºµãÊýÂíÉÏ±ä³É£¹£¹ ");
		break;
	case 4:
		prints(" »Ø×ª£ºÓÎÏ·½øÐÐ·½ÏòÏà·´ ");
		break;
	case 5:
		prints(" Ö¸¶¨£º×ÔÓÉÖ¸¶¨ÏÂÒ»¸öÍæ¼Ò ");
		break;
	case 11:
		prints(" PASS£º¿ÉPASSÒ»´Î ");
		break;
	case 10:
		prints(" µãÊý¼Ó»ò¼õ 10 ");
		break;
	case 12:
		prints(" µãÊý¼Ó»ò¼õ 20 ");
		break;
	default:
		sprintf(buf, " µãÊý¼Ó %d ", t);
		prints(buf);
		break;
	}
}

mmsg()
{
	char           *msg[12] = {
		"Î¹¡­´ò¿ìÒ»µãÀ²£¡",
		"´òÅÆ×¨ÐÄÒ»µã£¬²»ÒªÁ÷¿ÚË®¡­",
		"²»Òª¿´ÃÀÃ¼¡­",
		"´óÒ¯ÐÐÐÐºÃ£¬·Å¹ýÎÒ°É¡­",
		"½ÓÕÐÊÜËÀ°É£¡£¡£¡",
		"ÍÛ£¬ÄãºÃÀ÷º¦à¸£¡¾¹È»ÄÜ´ò³öÕâÕÅÅÆ£¡",
		"ÄãÒ»¶¨ÊÇ¶ÄÍ½×ªÊÀµÄ£¡",
		"last hand¡­",
		"ÔÙÍæµç±¬Äã£¡",
		"ÍÛ¡­ÄãÓÐÐØ¼¡ßÖ..",
		"ÄãµÄ¶Ç×ÓÅÜ³öÀ´ÁËßÖ¡­",
	"dsyanÊÇºÃÈË¡­gwenÊÇË§¸ç¡­"};
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
	modify_user_mode(M_BINGO);

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
			clear();
			prints("\n\n");
			j = 0;
			for (i = 0; i <= 10; i++) {
				if (i % 2 == 0)
					outs("   [1;34;40m¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö¡ö[0m\n");
				else {
					outs("   [1;34;40m¡ö");
					if (place[j][0][2])
						prints("[1;37;40m%2d", place[j][0][2]);
					else
						prints("[1;32m¡ö");
					outs("[1;34;40m¡ö");
					if (place[j][1][2])
						prints("[1;37;40m%2d", place[j][1][2]);
					else
						prints("[1;32m¡ö");
					outs("[1;34;40m¡ö");
					if (place[j][2][2])
						prints("[1;37;40m%2d", place[j][2][2]);
					else
						prints("[1;32m¡ö");
					outs("[1;34;40m¡ö");
					if (place[j][3][2])
						prints("[1;37;40m%2d", place[j][3][2]);
					else
						prints("[1;32m¡ö");
					outs("[1;34;40m¡ö");
					if (place[j][4][2])
						prints("[1;37;40m%2d", place[j][4][2]);
					else
						prints("[1;32m¡ö");
					outs("[1;34;40m¡ö[0m\n");
					j++;
				}
			}
			prints("\n\n\n\n");
			prints("[1;37;44mÉÐÎ´¿ª³öµÄºÅÂë[0m\n");
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


			prints("\nÉÐÓÐ[1;33;41m %2d [0m´Î»ú»á¿É²Â ÏÂ´Î²ÂÖÐ¿ÉµÃ[1;37;44m %d [0m±¶ \n", account - 1, may);
			show_money(money,NULL,NA);
			getdata(20, 0, "ÇëÊäÈëÄúµÄºÅÂë(°´ Q ±íÊ¾ÈÏÊä) : ", co, 3, DOECHO, YEA);

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
			prints("¹§Ï²Äã...Ó®ÁË %d Ôª ", money * may);
			inmoney((money * may) + money);
			sprintf(buf,"×¬ÁË %d Ôª ", money * may );
		} else {
			prints("ÔËÆø²»¼Ñ...ÔÙÀ´Ò»ÅÌ°É!!");
			sprintf(buf,"ÊäµôÁË %d Ôª", money);
		}
		gamelog(buf);
		pressanykey();
	}
	return 0;
}
