/*
 $Id: postheader.c 366 2007-05-12 16:35:51Z danielfree $
 ±¾Ò³ÃæÖ÷Òª´¦Àí·¢±íÎÄÕÂ£¬»Ø¸´ÎÄÕÂºÍ·¢ÐÅ¼þÊ±µÄÍ·´¦Àí
 */

#include "bbs.h"

extern int numofsig;
extern int noreply;
extern int mailtoauther;
extern struct boardheader *getbcache();
#ifdef ENABLE_PREFIX
char prefixbuf[MAX_PREFIX][6];
int numofprefix;
#endif
void check_title(char *title) {
	char tempfilename[50];
	trim(title);
	if (!strncasecmp(title, "Re:", 3) && !HAS_PERM(PERM_SYSOPS)) {
		sprintf(tempfilename, "Re£º%s", &title[3]);
		strcpy(title, tempfilename);
	}
}
#ifdef ENABLE_PREFIX
void set_prefix() {
	char filename[STRLEN], buf[128];
	int i;
	FILE *fp;

	setvfile(filename, currboard, "prefix");
	if ((fp = fopen(filename, "r"))> 0) {
		for (i = 0; i < MAX_PREFIX; i++) {
			if (!fgets(buf, STRLEN, fp)) {
				break;
			}
			if (i == 0 && (buf[0] == '\n' || buf[0] == ' ') )
			buf[0] = '\0';
			strtok(buf, " \r\n\t");
			ansi_filter(prefixbuf[i], buf);
			prefixbuf[i][4] = '\0';
		}
		numofprefix = i;
		fclose(fp);
	} else numofprefix = 0;
}

void print_prefixbuf(char *buf, int index) {
	int i;
	buf += sprintf(buf, "Ç°×º:");
	for (i = 0; i < numofprefix; i++ ) {
		if (i == 0 && prefixbuf[i][0] == '\0' )
		buf += sprintf(buf, "1.%sÎÞ%s",
				(index == i + 1)?"\033[1m":"",
				(index == i + 1)?"\033[m":"");
		else
		buf += sprintf(buf, " %d.%s%s%s", i + 1,
				(index == i + 1)?"\033[1;33m":"",
				prefixbuf[i],
				(index == i + 1)?"\033[m":"");
	}
	sprintf(buf, "[%d]:", index);
}
#endif
int post_header(struct postheader *header) {
	int anonyboard = 0;
#ifdef ENABLE_PREFIX
	int index = 0;
	char pbuf[128];
#endif
	char r_prompt[20], mybuf[256], ans[5];
	char titlebuf[STRLEN];
	//ÔÚ»Ø¸´Ä£Ê½ºÍÍ¶ÌõÊ±×÷Îª±êÌâµÄbuffer
	struct boardheader *bp;
#ifdef IP_2_NAME
	extern char fromhost[];
#endif
#ifdef RNDSIGN  //Ëæ»úÇ©Ãûµµ
	/*rnd_sign=0±íÃ÷·ÇËæ»ú,=1±íÃ÷Ëæ»ú*/
	int oldset, rnd_sign = 0;
#endif

	//¶Ôµ±Ç°Ç©ÃûµµµÄÔ½½ç´¦Àí
	if (currentuser.signature > numofsig || currentuser.signature < 0) {
		currentuser.signature = 1;
	}
#ifdef RNDSIGN
	if (numofsig> 0) {
		if (DEFINE(DEF_RANDSIGN)) {
			oldset = currentuser.signature;
			srand((unsigned) time(0));
			currentuser.signature = (rand() % numofsig) + 1;//²úÉúËæ»úÇ©Ãûµµ
			rnd_sign = 1; //±ê¼ÇËæ»úÇ©Ãûµµ
		} else {
			rnd_sign = 0;
		}
	}
#endif

	/*Èç¹ûµ±Ç°ÊÇ»Ø¸´Ä£Ê½£¬Ôò°ÑÔ­±êÌâcopyµ½µ±Ç°±êÌâÖÐ£¬²¢±ê¼Çµ±Ç°Îª»Ø¸´Ä£Ê½
	 ·ñÔòµ±Ç°±êÌâÎª¿Õ*/
	if (header->reply_mode) {
		strcpy(titlebuf, header->title);
		header->include_mode = 'R'; //exchange 'R' and 'S' by Danielfree 07.04.05
	} else {
		titlebuf[0] = '\0';
#ifdef ENABLE_PREFIX
		set_prefix();
#endif
	}

	//bp¼ÇÂ¼µ±Ç°ËùÔÚ°æÃæµÄÐÅÏ¢
	bp = getbcache(currboard);

	//Èç¹ûÊÇ·¢±íÎÄÕÂ£¬ÔòÊ×ÏÈ¼ì²éËùÔÚ°æÃæÊÇ·ñÄäÃû£¨·¢ÐÅÊ±²»´æÔÚÕâ¸öÎÊÌâ£©
	if (header->postboard) {
		anonyboard = bp->flag & BOARD_ANONY_FLAG;
	}

#ifdef IP_2_NAME
	if (anonyboard && (fromhost[0] < '1'||fromhost[0]> '9'))
	anonyboard = 0;
#endif

	// modified by roly 02.03.07
	// modified by iamfat 02.10.30
	header->chk_anony = (anonyboard) ? 1 : 0;
	//header->chk_anony = 0;
	//modifiy end
#ifdef ENABLE_PREFIX
	if (numofprefix> 0)
	index = 1;
#endif	
	while (1) {
#ifdef ENABLE_PREFIX
		if (header->reply_mode) {
			sprintf(r_prompt, "ÒýÑÔÄ£Ê½ [[1m%c[m]", header->include_mode);
			move(t_lines - 4, 0);
		} else if (numofprefix == 0)
		move(t_lines - 4, 0);
		else
		move(t_lines - 5, 0);
#else
		if (header->reply_mode)
			sprintf(r_prompt, "ÒýÑÔÄ£Ê½ [[1m%c[m]", header->include_mode);
		move(t_lines - 4, 0);
#endif
		//Çå³ý¸ÃÐÐÄÚÈÝ
		clrtobot();

		//¸ù¾ÝÏàÓ¦²Ù×÷´òÓ¡³öµ±Ç°ÐÅÏ¢
		prints(
				"[m%s [1m%s[m      %s    %s%s\n",
				(header->postboard) ? "·¢±íÎÄÕÂì¶" : "ÊÕÐÅÈË£º",
				header->ds,
				(anonyboard) ? (header->chk_anony == 1 ? "[1mÒª[mÊ¹ÓÃÄäÃû"
						: "[1m²»[mÊ¹ÓÃÄäÃû") : "",
				(header->postboard) ? ((noreply) ? "[±¾ÎÄ[1;33m²»¿ÉÒÔ[m»Ø¸´"
						: "[±¾ÎÄ[1;33m¿ÉÒÔ[m»Ø¸´") : "",
				(header->postboard&&header->reply_mode) ? ((mailtoauther) ? ",ÇÒ[1;33m·¢ËÍ[m±¾ÎÄÖÁ×÷ÕßÐÅÏä]"
						: ",ÇÒ[1;33m²»·¢ËÍ[m±¾ÎÄÖÁ×÷ÕßÐÅÏä]")
						: (header->postboard) ? "]" : "");
#ifdef ENABLE_PREFIX
		if (!header->reply_mode && numofprefix) {
			if (bp->flag & BOARD_PREFIX_FLAG && !header->title[0]) {
				index = 0;
				print_prefixbuf(pbuf, index);
				while (1) {
					getdata(t_lines - 4, 0, pbuf, ans, 2, DOECHO, YEA);
					if (!ans[0])
					return NA;
					index = ans[0] - '0';
					if (index> 0 && index<= numofprefix) {
						print_prefixbuf(pbuf, index);
						break;
					}
				}
			} else {
				print_prefixbuf(pbuf, index);
			}
			move(t_lines - 4, 0);
			prints(pbuf);
		}

		//¶ÔÓÚ»Ø¸´ºÍ·¢ÐÅ£¬title³õÊ¼²»Îª¿Õ.ËùÒÔÖ»ÓÐÔÚ·¢±íÎÄÕÂÊ±£¬²Å»á³öÏÖ"[ÕýÔÚÉè¶¨Ö÷Ìâ]"
		move(t_lines-3, 0 );
#endif
		prints("Ê¹ÓÃ±êÌâ: [1m%-50s[m\n",
				(header->title[0] == '\0') ? "[ÕýÔÚÉè¶¨Ö÷Ìâ]" : header->title);
#ifdef RNDSIGN
		//ÔÚ»Ø¸´Ä£Ê½ÏÂ»á³öÏÖÏàÓ¦µÄÒýÑÔÄ£Ê½ÐÅÏ¢
		prints("Ê¹ÓÃµÚ [1m%d[m ¸öÇ©Ãûµµ     %s %s", currentuser.signature
				,(header->reply_mode) ? r_prompt : "", (rnd_sign == 1) ? "[Ëæ»úÇ©Ãûµµ]" : "");
#else
		prints("Ê¹ÓÃµÚ [1m%d[m ¸öÇ©Ãûµµ     %s", currentuser.signature,
				(header->reply_mode) ? r_prompt : "");
#endif
		//¶ÔÓÚ·¢±íÎÄÕÂ»òÕßÍ¶ÌõÇé¿ö
		if (titlebuf[0] == '\0') {
			//moveµ½ÏàÓ¦µÄÐÐ£¬ÎªÊäÈë×ö×¼±¸
			move(t_lines - 1, 0);
			if (header->postboard == YEA || strcmp(header->title, "Ã»Ö÷Ìâ"))
				ansi_filter(titlebuf, header->title);

			//´Óµ±Ç°ÐÐ»ñµÃÓÃ»§ÊäÈë·Åµ½titlebufÖÐ£¬×î¶à´æÈë50-1¸ö×Ö½Ú(´Ë´¦»á×èÈûÔÚÓÃ»§ÊäÈëÉÏ£¬Ö»µ½ÏìÓ¦enter)
			getdata(t_lines - 1, 0, "±êÌâ: ", titlebuf, 50, DOECHO, NA);
			check_title(titlebuf);

			//ÔÚÓÃ»§ÊäÈëÎª¿ÕµÄÇé¿öÏÂ£¬Èç¹ûÊÇ·¢±íÎÄÕÂÔòÖ±½ÓÈ¡Ïû£¬Èç¹ûÊÇÍ¶ÌõÓÃ»§»¹¿ÉÒÔ¼ÌÐø£¬ÐÅÍ·ÎªÃ»Ö÷Ìâ
			if (titlebuf[0] == '\0') {
				if (header->title[0] != '\0') {
					titlebuf[0] = ' ';
					continue;
				} else
					return NA;
			}

			//½«ÓÃ»§Éè¶¨title¸´ÖÆµ½ÏàÓ¦½á¹¹ÖÐ
			strcpy(header->title, titlebuf);
			continue;
		}

		trim(header->title); //add by money 2003.10.29.
		move(t_lines - 1, 0);

#ifdef ENABLE_PREFIX	
		sprintf(mybuf,
				"[1;32m0[m~[1;32m%d V[m¿´Ç©Ãûµµ%s [1;32mX[mËæ»úÇ©Ãûµµ,[1;32mT[m±êÌâ%s%s%s,[1;32mQ[m·ÅÆú:",
				numofsig, (header->reply_mode) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m ÒýÑÔÄ£Ê½" : " \033[1;32mF\033[mÇ°×º",
				(anonyboard) ? "£¬[1;32mL[mÄäÃû" : "",(header->postboard)?",[1;32mU[mÊôÐÔ":"",(header->postboard&&header->reply_mode)?",[1;32mM[m¼ÄÐÅ":"");
#else
		sprintf(
				mybuf,
				"[1;32m0[m~[1;32m%d V[m¿´Ç©Ãûµµ%s [1;32mX[mËæ»úÇ©Ãûµµ,[1;32mT[m±êÌâ%s%s%s,[1;32mQ[m·ÅÆú:",
				numofsig,
				(header->reply_mode) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m ÒýÑÔÄ£Ê½"
						: "", (anonyboard) ? "£¬[1;32mL[mÄäÃû" : "",
				(header->postboard) ? ",[1;32mU[mÊôÐÔ" : "",
				(header->postboard&&header->reply_mode) ? ",[1;32mM[m¼ÄÐÅ"
						: "");
#endif
		//´òÓ¡³öÌáÊ¾ÐÅÏ¢£¬²¢×èÈûÔÚÓÃ»§ÊäÈë¶¯×÷ÉÏ,ÓÃ»§×î¶àÊäÈë2¸ö×Ö½Ú
		getdata(t_lines - 1, 0, mybuf, ans, 3, DOECHO, YEA);
		ans[0] = toupper(ans[0]);

		//ÓÃ»§¶ÔÇ©ÃûµµÉèÖÃ£¬°üÀ¨È¡Ïûµ±Ç°²Ù×÷
		if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
			//ÓÐÐ§µÄÇ©ÃûµµÑ¡Ôñ
			if (atoi(ans) <= numofsig)
				currentuser.signature = atoi(ans);
		} else if (ans[0] == 'Q' || ans[0] == 'q') { //È¡Ïûµ±Ç°¶¯×÷
			return -1;
		}
		//¶ÔÓÚ»Ø¸´Ä£Ê½
		else if (header->reply_mode && (ans[0] == 'Y' || ans[0] == 'N'
				|| ans[0] == 'A' || ans[0] == 'R'||ans[0]=='S')) {
			header->include_mode = ans[0];
		} //ÖØÐÂÉèÖÃ±êÌâ
		else if (ans[0] == 'T') {
			titlebuf[0] = '\0';
		}//¶ÔÓÚÄäÃû°æµÄÌØ±ð´¦Àí 
		else if (ans[0] == 'L' && anonyboard) {
			header->chk_anony = (header->chk_anony == 1) ? 0 : 1;
		}//¶ÔÓÚÎÄÕÂÖÐ£¬¿É·ñ»Ø¸´µÄ¸ü¸Ä 
		else if (ans[0] == 'U' && header->postboard) {
			noreply = ~noreply;
		}//¶ÔÓÚ»Ø¸´ÎÄÕÂÊ±µÄÌØÊâÊôÐÔÉèÖÃ 
		else if (ans[0] == 'M' && header->postboard && header->reply_mode) {
			mailtoauther = ~mailtoauther;
		}//¶ÔÇ©ÃûµµµÄ´¦Àí 
		else if (ans[0] == 'V') {
			setuserfile(mybuf, "signatures");
			if (askyn("Ô¤ÉèÏÔÊ¾Ç°Èý¸öÇ©Ãûµµ, ÒªÏÔÊ¾È«²¿Âð", NA, YEA) == YEA)
				ansimore(mybuf);
			else {
				clear();
				ansimore2(mybuf, NA, 0, 18);
			}
#ifdef RNDSIGN
		}//Ëæ»úÇ©Ãûµµ 
		else if (ans[0] == 'X') {
			if (rnd_sign == 0 && numofsig != 0) {
				oldset = currentuser.signature;
				srand((unsigned) time(0));
				currentuser.signature = (rand() % numofsig) + 1;
				rnd_sign = 1;
			} else if (rnd_sign == 1 && numofsig != 0) {
				rnd_sign = 0;
				currentuser.signature = oldset;
			}
			ans[0] = ' ';
#endif
		}
#ifdef ENABLE_PREFIX
		//ÐÞ¸ÄÇ°×º
		else if (!header->reply_mode && numofprefix && ans[0] == 'F') {
			int i;
			getdata(t_lines - 1, 0, pbuf, ans, 3, DOECHO, YEA);
			i = ans[0] - '0';
			if (i >= 0 && i <= numofprefix &&
					!(i == 0 && (bp->flag & BOARD_PREFIX_FLAG)))
			index = i;
		}
#endif
		else {
			if (header->title[0] == '\0')
				return NA;
			else {
#ifdef ENABLE_PREFIX
				strcpy(header->prefix, index?prefixbuf[index - 1]:"");
#endif
				return YEA;
			}
		}
	}
}

