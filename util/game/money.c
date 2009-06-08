/* ¿ìÒâ¹àË®Õ¾ ½»Ò×ÊĞ³¡´úÂë 1999.12.19 */
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
        modify_user_mode(MARKET);
        clear();
        set_safe_record();
        move(2,0);
        prints("»¶Ó­½øÈë [[32m%s[37m]....\n\n",title);
	return 0;
}

int lending()
{
        int     id, canlending=0,maxnum = 0, num = 0;
	char	ans[STRLEN];
	time_t 	now=time(0);
	extern int gettheuserid();
	if(gotomarket("½»Ò×ÊĞ³¡")) return 0;
	maxnum = currentuser.money - currentuser.bet - 1000;
	prints("»¶Ó­Ê¹ÓÃ[0;1;33m"BBSNAME"[37m½»Ò×ÊĞ³¡[32mÓÑÇé×ªÕÊ[37m¹¦ÄÜ[m");
	prints("\n\nÄúÄ¿Ç°µÄÇé¿öÊÇ£º\n×¢²áÌìÊı([32m%d[37m Ìì) ÉÏÕ¾×ÜÊ±Êı([32m%d[37m Ğ¡Ê±) [44;37m¿É×ªÕÊ×Ê½ğ([32m%d[37m Ôª)[m[37m",
		(now - currentuser.firstlogin)/86400,currentuser.stay/3600,currentuser.money-currentuser.bet-1000);
	if ( currentuser.stay <= 36000 || now - currentuser.firstlogin  <= 2592000 || maxnum <= 0 ) {
		 prints("\n\nÄ¿Ç°ÊĞ³¡¹æ¶¨£º ²ÎÓë[32mÓÑÇé×ªÕÊ[mµÄ ID ±ØĞë¾ß±¸ÒÔÏÂÈ«²¿Ìõ¼ş£º\n    1. ±¾ÕÊºÅ×¢²áÌìÊı³¬¹ı 30 Ìì;\n    2. ×ÜÉÏÕ¾Ê±Êı³¬¹ı 10 Ğ¡Ê±;\n    3. ×îÖÕÓµÓĞ´æ¿î³¬¹ı 1000 Ôª.\n      (×¢£ºÖ¸´æ¿î¼õÈ¥´û¿îºóµÄ²îÖµ.)");
		prints("\n\n¸ù¾İÊĞ³¡¹æ¶¨£¬ÄúÄ¿Ç°ÉĞÃ»ÓĞ[32mÓÑÇé×ªÕÊ[mµÄ×Ê¸ñ¡£ :P \n");
		pressanykey();
		return 0;
	}
        if (!gettheuserid(9,"ÄúÏë×ªÕÊµ½Ë­µÄÕÊ»§ÉÏ£¿ÇëÊäÈëËûµÄÕÊºÅ: ",&id))
                return 0;
	if(!strcmp(currentuser.userid,lookupuser.userid)) {
		prints("\nºÇºÇ£¬×ªÕÊ¸ø×Ô¼º°¡£¿ àÅ£¬Ò²ĞĞ¡£²»¹ı±¾Õ¾²»Ìá¹©Õâ¸ö·şÎñ¡£");
		pressanykey();
		return 0;
	}
        if( lookupuser.money+lookupuser.nummedals*1000 > 90000 ) {
                prints("\n¶Ô²»Æğ£¬¶Ô·½Ä¿Ç°¾­¼ÃÄÜÁ¦ÉĞ²»ĞèÒªÄúµÄ×ªÕÊ£¡");
                pressanykey();
                return 0;
        }
	move(10,0);
	canlending = maxnum > 90000 ? 90000 : maxnum;
	prints("Äú½«×ªÕÊµ½ [1;32m%s[m µÄÕÊºÅ£¬Äú¿ÉÒÔ×î¶à¿ÉÒÔ×ªÕÊ [1;33m%d[m Ôª¡£",lookupuser.userid, canlending);
        getdata(12, 0, "È·ÈÏÒª×ªÕÊ£¬ÇëÊäÈë×ªÕÊÊıÄ¿£¬·ñÔò£¬ÇëÖ±½Ó»Ø³µÈ¡Ïû×ªÕÊ: ",ans, 6, DOECHO, YEA);
        num = atoi(ans);
        if ( num <= 0 || num > canlending ) {
                prints("\nÊäÈëÓĞ´íÎóÅ¶¡£ »¹ÊÇËãÁË°É¡£¡£¡£");
                pressanykey();
                return 0;
        }
	set_safe_record();
	if(currentuser.money - currentuser.bet - 1000 != maxnum) {
		prints("\n¶Ô²»Æğ£¬ÄúµÄ¿É×ªÕÊ×Ê½ğÓĞËù±ä»¯£¬È¡Ïû´Ë´Î½»Ò×£¡");
		prints("\nÇëÖØĞÂÖ´ĞĞ±¾½»Ò×¡£");
		pressanykey();
		return 0;
	}
 	currentuser.money -= num;
	substitut_record(PASSFILE, &currentuser, sizeof(currentuser),usernum);
	lookupuser.money += num;
	substitut_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	prints("\nĞ»Ğ»Äú£¬ÄúÒÑ¾­¸ø [1;32m%s[m ×ªÕÊ [1;33m%d[m Ôª¡£",lookupuser.userid,num);
	prints("\nÎª±íÊ¾¶ÔÄãµÄ×ªÕÊĞĞÎªµÄ¸ĞĞ»£¬±¾Õ¾ÒÑ¾­ÓÃĞÅ¼şÍ¨ÖªÁËËû¡£");
	sprintf(genbuf,"¸ø %s ×ªÕÊ %d Ôª",lookupuser.userid,num);
	gamelog(genbuf);
	sprintf(ans,"%s ¸øÄú¼ÄÀ´ÁË %d ÔªÓÑÇé×ªÕÊ",currentuser.userid,num);
	sprintf(genbuf,"\n %s :\n\tÄúºÃ£¡\n\tÄúµÄÅóÓÑ %s ¸øÄú¼ÄÀ´ÁË %d ÔªÓÑÇé×ªÕÊ×Ê½ğ¡£\n\tÇëÄúÔÚ Market °å¶ÔËûµÄÎŞË½ĞĞÎª·¢ÎÄ±íÊ¾¸ĞĞ»£¬\n\tÕâÑù£¬Äú¾Í¿ÉÒÔ»ñµÃÕâ±ÊÓÑÇé×ªÕÊ×Ê½ğ¡£\n\n\tµ±È»£¬ÄúÒ²¿ÉÒÔÍË³öÒ»´Îºó£¬ÔÙ½øÈë±¾Õ¾£¬\n\tÒ»Ñù¿ÉÒÔ»ñµÃÕâ±ÊÓÑÇé×ªÕÊ×Ê½ğ¡£\n  ",lookupuser.userid,currentuser.userid,num);
	autoreport(ans,genbuf,NA,lookupuser.userid);
	pressanykey();
	return 1;
}

int popshop(void)
{
	int no,num,maxnum,templog;
	char ans[10];
	EXCHANGES exchanges[10] = {
		{"ÉÏÕ¾´ÎÊı",2},
		{"ÎÄÕÂÊı",5},
		{"½±ÕÂÊı",10000},
		{"ÒşÉíÊõ",16000},
		{"¿´´©ÒşÉíÊõ",4500},
		{"ÕÊºÅÓÀ¾Ã±£Áô",45000},
		{"Ç¿ÖÆºô½Ğ",54000}, //expired function 06.1.5
		{"ÑÓ³¤·¢´ôÊ±¼ä",9000},//expired function 06.1.5
		{"´óĞÅÏä",45000}
		};
	if(gotomarket("ÊĞ³¡µäµ±ĞĞ")) return 1;
	prints("½ñÈÕµäµ±±¨¼Û:");
	for(no = 0; no < 9; no ++) {
		move(5+no, 2);
		prints("(%2d) %s",no+1,exchanges[no].title);
		move(5+no, 20);
		prints("==> %6d Ôª", exchanges[no].value);
	}
	move(16,0);
	prints("ÄúÄ¿Ç°µÄÇé¿öÊÇ: (1)ÉÏÕ¾´ÎÊı([32m%d[37m)  (2)ÎÄÕÂÊı([32m%d[37m)  (3)½±ÕÂÊı.([32m%d[37m)\n",
	    currentuser.numlogins,currentuser.numposts,currentuser.nummedals);
	getdata(18, 0, "ÄúÏëµäµ±ÄÄÒ»Ïî£¿", ans, 10, DOECHO, YEA);
	no = atoi(ans);
	if ( no < 1 || no > 9 ) {
		prints("\nºÇºÇ£¬²»µäµ±ÁË£¿ ÄÇ£¬ºÃ×ß¡£¡£»¶Ó­ÔÙÀ´ ;)");
		pressanykey();
		return 0;
	}
	move(18, 30);
	prints("ÄãÑ¡Ôñµäµ±¡°[32m%s[m¡±¡£",exchanges[no-1].title);
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
		prints("¶Ô²»Æğ, Äã»¹Ã»ÓĞÕâÖÖÈ¨ÏŞ. ");
		pressanykey();
		return 0;
	}
        if(askyn("ÄúÈ·¶¨Òªµäµ±Âğ£¿",NA,NA) == NA ) {
                move(23,0);clrtoeol();
                prints("ÏÖÔÚ²»µäµ±ÁË£¿ ÄÇÄãÏÂ´ÎÔÙÀ´¡£ Òª¼ÇµÃÅ¶¡£");
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
                prints("¶Ô²»Æğ, ÄãµÄÊı¾İ·¢ÉúÁË±ä»¯, ÄãÄ¿Ç°Ã»ÓĞÕâÖÖÈ¨ÏŞ. ");
                pressanykey();
                return 0;
        }
} else {
	if( no == 1 )maxnum = currentuser.numlogins;
	else if ( no == 2) maxnum = currentuser.numposts;  
	else	maxnum = currentuser.nummedals;
	templog = maxnum;
	sprintf(genbuf,"ÄúÏëµäµ±¶àÉÙÄØ(×î¶à%d)£¿",maxnum);
	getdata(19, 0, genbuf,ans, 10, DOECHO, YEA);
	num = atoi(ans);
	if ( num <= 0 || num > maxnum ) {
		prints("ÊäÈëÓĞ´íÎóÅ¶¡£ »¹ÊÇËãÁË°É¡£¡£¡£");
		pressanykey();
		return 0;
	}
        maxnum = num*exchanges[no-1].value;
	move(19,0);
	prints("Äú¹²¼Æµäµ±%s[32m%d[m ·İ£¬ %s [33m%d[m Ôª¡£\n",exchanges[no-1].title,num,"¿ÉÒÔ»ñµÃ",maxnum);
        if(askyn("ÄúÈ·¶¨Òªµäµ±Âğ£¿",NA,NA) == NA ) {
                move(21,0);clrtoeol();
		prints("ÏÖÔÚ²»µäµ±ÁË£¿ ÄÇÄãÏÂ´ÎÔÙÀ´¡£ Òª¼ÇµÃÅ¶¡£");
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
		prints("¶Ô²»Æğ, ÔÚ½»Ò×¹ı³ÌÖĞÄúµÄÊı¾İ·¢ÉúÁË±ä»¯.\nÎª±£Ö¤½»Ò×µÄÕı³£½øĞĞ, ´Ë´Î½»Ò×È¡Ïû.\nÄú¿ÉÒÔÖØĞÂ½øĞĞ±¾½»Ò×.");
		pressanykey();
		return 0;
	}
}
	currentuser.money += maxnum;
	if( currentuser.money > 400000000 ){
		move(21,0); clrtoeol();
		prints("¶Ô²»Æğ£¬½»Ò×Êı¾İ¹ı´ó£¬²úÉúÒç³ö£¬ÇëÖØĞÂ½»Ò×£¡");
		pressanykey();
		return 0;
	}
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
	move(21,0); clrtoeol();
        prints("Ë³ÀûÍê³É½»Ò×£¬»¶Ó­ÄúµÄÔÙ´Î»İ¹Ë¡£;P");
	sprintf(genbuf,"µäµ±%s %d ·İ£¬»¨·Ñ %d Ôª.",exchanges[no-1].title,num,maxnum);
	gamelog(genbuf);
        pressanykey();
        return 1;
}
int doshopping()
{
        int no,hasperm=1,maxnum;
        char ans[10];
        EXCHANGES exchanges[10] = {
                {"ÒşÉíÊõ",40000},
                {"¿´´©ÒşÉíÊõ",10000},
                {"ÕÊºÅÓÀ¾Ã±£Áô",100000},
                {"Ç¿ÖÆºô½Ğ",120000},//expired 06.1.5
                {"ÑÓ³¤·¢´ôÊ±¼ä",20000},//expired 06.1.5
                {"´óĞÅÏä",100000}
                };
        if(gotomarket("ÊĞ³¡¹ºÎïÖĞĞÄ")) return 1;
        prints("½ñÈÕÉÌÆ·±¨¼Û:");
        for(no = 0; no < 6; no ++) {
                move(5+no, 2);
                prints("(%2d) %s",no+1,exchanges[no].title);
                move(5+no, 20);
                prints("==> %6d Ôª", exchanges[no].value);
        }
        move(16,0);
        prints("ÄúÄ¿Ç°ÉĞÓĞ %d Ôª (½±ÕÂ %d ¸ö)\n",
            currentuser.money,currentuser.nummedals);
        getdata(18, 0, "ÄúÏë¹ºÂòÄÄÒ»¼şÉÌÆ·£¿", ans, 10, DOECHO, YEA);
        no = atoi(ans);
        if ( no < 1 || no > 6 ) {
                prints("\nºÇºÇ£¬²»ÏëÂòÁË£¿ ÄÇ£¬ºÃ×ß¡£¡£»¶Ó­ÔÙÀ´ ;)");
                pressanykey();
                return 0;
        }
        if ( no == 4 || no == 5 ) {
                prints("\nĞ¡µê²»Ìá¹©¸ÃÉÌÆ·ÁËÅ¶ :)");
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
                prints("ÄúÒÑ¾­ÓĞÕâÖÖÈ¨ÏŞ, ²»ĞèÒªÔÙ¹ºÂò. ");
                pressanykey();
                return 0;
        }
        if(currentuser.money < maxnum) {
                prints("\n¶Ô²»Æğ, ÄãÃ»ÓĞ×ã¹»µÄÇ®¹ºÂòÕâÖÖÈ¨ÏŞ.");
                pressanykey();
                return 0;
        }
        if(askyn("ÄúÈ·¶¨Òª¹ºÂòÂğ£¿",NA,NA) == NA ) {
                move(23,0);clrtoeol();
                prints("ÏÖÔÚ²»ÂòÁË£¿ ÄÇÄãÏÂ´ÎÔÙÀ´¡£ Òª¼ÇµÃÅ¶¡£");
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
                prints("ÔÚ½»Ò×½øĞĞÇ°ÄúÒÑ¾­ÓĞÁËÕâÖÖÈ¨ÏŞ, ËùÒÔÈ¡Ïû´Ë´Î½»Ò×. ");
                pressanykey();
                return 0;
        }
        if(currentuser.money < maxnum) {
                prints("\n¶Ô²»Æğ, ÄãÃ»ÓĞ×ã¹»µÄÇ®¹ºÂòÕâÖÖÈ¨ÏŞ.");
                pressanykey();
                return 0;
        }
	currentuser.money -= maxnum;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);        move(23,0); clrtoeol();
        prints("Ë³ÀûÍê³É½»Ò×£¬»¶Ó­ÄúµÄÔÙ´Î»İ¹Ë¡£;P");
        sprintf(genbuf,"¹ºÂò[%s]£¬»¨·Ñ %d Ôª.",exchanges[no-1].title,maxnum);
        gamelog(genbuf);
        pressanykey();
        return 1;
}	

int
payoff()
{
	if(gotomarket("Ë®Õ¾»¹´û´¦")) return 0;
        prints("±¾´¦¹æ¶¨: ³¥»¹´û¿î±ØĞëÒ»´Î»¹Çå. \n\n");
	if(currentuser.bet == 0 ) {
		prints("Äã²¢Ã»ÓĞÔÚ±¾ÊĞ³¡½èÇ®£¬ËùÒÔÎŞĞè»¹Ç®£¬ºÇºÇ");
		pressanykey();
		return 0;
	}
	if(currentuser.money < currentuser.bet) {
		prints("ÄãµÄÇ®²»¹»»¹´û¿î£¬ÇëÏÂ´ÎÔÙÀ´»¹ÂŞ¡£");
		pressanykey();
		return 0;
	}
	prints("ÄúÔÚ±¾´¦¹²´û¿î %d Ôª.\n\n", currentuser.bet);
	 if(askyn("ÄúÏÖÔÚ¾ÍÏë»¹Çå´û¿îÂğ£¿",NA,NA) == NA ) {
		prints("ÏÖÔÚ²»»¹ÁË£¿ ÄÇÄãÏÂ´ÎÔÙÀ´¡£ Òª¼ÇµÃÅ¶¡£");
		pressanykey();
		return 0;
	}
        currentuser.money -= currentuser.bet;
        sprintf(genbuf,"»¹Çå´û¿î %d Ôª.",currentuser.bet);
        gamelog(genbuf);
        currentuser.bet = 0;
        currentuser.dateforbet = 0;
        substitut_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
        prints("ÄúÒÑ¾­»¹ÇåÁËÔÚ±¾ÊĞ³¡Ëù½èµÄÇ®¡£»¶Ó­ÄúµÄÔÙ´Î»İ¹Ë¡£;P");
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
	if(gotomarket("Ë®Õ¾½è´û´¦"))return 0;
	prints("±¾´¦¹æ¶¨: Ä¿Ç°Ã¿ÈË×î¶à¿ÉÒÔ´û¿î %d Ôª.\n\n", MAXBET);
	if(!currentuser.bet)
		prints("ÄúÄ¿Ç°»¹Ã»ÓĞÔÚ±¾´¦´û¿î.\n\n");
	else {
		prints("ÄúÒÑ¾­ÔÚ±¾´¦´û¿î %d Ôª.\n",currentuser.bet);
		dstr = getdatestring(currentuser.dateforbet, NA);
		sprintf(genbuf, "Äú³¥»¹´û¿îµÄ×îºóÆÚÏŞÊÇ:  %14.14s(%s) \n\n", dstr, dstr + 23);
		prints(genbuf);
		if( currentuser.bet>=MAXBET) {
               		prints("¶Ô²»Æğ, ÄúµÄ´û¿îÒÑ¾­´ïµ½¹æ¶¨ÊıÄ¿, ²»ÄÜÔÙÏíÊÜ´û¿î·şÎñ.");
                        pressanykey();
                        return 0;
                }

	}
	if(askyn("ÄúÏÖÔÚÏëÏò±¾Õ¾´û¿îÂğ£¿",NA,NA) == NA ) return 0;
	maxbet = MAXBET-currentuser.bet;
	if( maxbet > 1000 ) {
		sprintf(genbuf,  "Äú¿ÉÒÔ´û¿î: ÖÁÉÙ 1000 Ôª, ×î¶à %d Ôª¡£ÄúÏë½è¶àÉÙÄØ£¿",maxbet);
		getdata(10, 0, genbuf, buf, 10, DOECHO, YEA);
		tempbet = atoi(buf);
	} else {
		sprintf(genbuf,"Äú¿ÉÒÔ´û¿î %d Ôª£¬ÄúÈ·¶¨Òª´û¿îÂğ£¿",maxbet);
		if( askyn(genbuf,YEA,NA) == NA) {
			prints("\nàÅ£¬²»½èÁË£¿ ÄÇºÃ£¬ÏÂ´ÎÔÙÀ´¡£ ;p");
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
		sprintf(genbuf, "\nÄãÏò±¾Õ¾×Ü¹²½è¿î %d Ôª£¬ÄúĞèÒªÔÚ %14.14s(%s) »¹Çå´û¿î¡£",currentuser.bet, dstr, dstr + 23);
		prints(genbuf);
		sprintf(genbuf,"%s ½è¿î %d Ôª.",currentuser.userid,tempbet);
		gamelog(genbuf);
		pressanykey();
		return 1;
        }
	prints("\nÄúÊäÈëµÄÊıÄ¿²»ÕıÈ·£¬È¡Ïû´Ë´Î½»Ò×¡£");
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
        prints("±§Ç¸£¡Äú²»¿ÉÒÔÏÂ×¢ %d Ôª, ÒòÎªÄúÏÖÔÚÉíÉÏÖ»ÓĞ %d Ôª£¡",
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
        prints("[0;1;37;44m                  ÄãÏÖÓĞÏÖ½ğ: [36m%-18d[37mÑº×¢½ğ¶î: [36m%-20d[m  ", currentuser.money, m);
}

int get_money(int m, char *welcome)
{
   unsigned int	money;
   char	buf[5];
   do {
      show_money(m,welcome,YEA);
      getdata(23,16,"¡îÒªÑº×¢¶àÉÙÇ®(1 - 2000)? ", buf, 5, DOECHO, YEA);
      if(buf[0] == '\0') return 0;
      money = abs(atoi(buf));
      if ( money <= 0) return 0;
      if(check_money(money))return 0;
   } while ((money < 1) || (money > 2000));
   demoney(money);
   show_money(money,NULL,YEA);
   return money;
}
