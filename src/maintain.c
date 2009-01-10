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
/*
$Id: maintain.c 248 2006-05-29 05:43:57Z SpiritRain $
*/

#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

char    cexplain[STRLEN];
char    lookgrp[30];
FILE   *cleanlog;

//	∫À∂‘œµÕ≥√‹¬Î
int	check_systempasswd()
{
	FILE*	pass;
	char    passbuf[20], prepass[STRLEN];
	clear();
	if ((pass = fopen("etc/.syspasswd", "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen(prepass) - 1] = '\0';
		getdata(1, 0, "«Î ‰»ÎœµÕ≥√‹¬Î: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!checkpasswd(prepass, passbuf)) {
			move(2, 0);
			prints("¥ÌŒÛµƒœµÕ≥√‹¬Î...");
			securityreport("œµÕ≥√‹¬Î ‰»Î¥ÌŒÛ...", 0, 0);
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

//	◊‘∂Ø∑¢ÀÕµΩ∞Ê√Ê
//			title		±ÍÃ‚
//			str			ƒ⁄»›
//			toboard		æˆ∂® «∑Ò∑¢ÀÕµΩ∞Ê√Ê	
//			userid		∑¢ÀÕµΩµƒ”√ªß√˚,Œ™null‘Ú≤ª∑¢ÀÕ.
//			mode		∑÷±Ω±≥Õ,1±Ì æBMS»Œ√¸,0±Ì ædeliver¥¶∑£
//					2±Ì æµ±«∞”√ªß
int autoreport(char *title,char *str,int toboard,char *userid,int mode)
{
	FILE	*se;
    char	fname[STRLEN];
    int 	savemode;
	
    savemode = uinfo.mode;
    report(title, currentuser.userid);
    sprintf(fname,"tmp/AutoPoster.%s.%05d",currentuser.userid,uinfo.pid);
    if((se=fopen(fname,"w"))!=NULL) {
	    fprintf(se,"%s",str);
        fclose(se);
        if(userid != NULL) {
			mail_file(fname,userid,title);
		}
		/* Modified by Amigo 2002.04.17. Set BMS announce poster as 'BMS'. */
//		if(toboard) Postfile( fname,currboard,title,1);
		if(toboard) {
    		if(mode == 1){
			Postfile( fname,currboard,title,3);    	//”…BMS∑¢±Ìµƒ»Œ√¸π´?		} else if (mode == 2){
			Postfile( fname,currboard,title,2);
		} else{
			//mode ==
			Postfile( fname,currboard,title,1);		//”…deliver∑¢±Ìµƒ¥¶∑£π´∏Ê
		}
		}
		/* Modify end. */
        unlink(fname);
        modify_user_mode( savemode );
    }
	return 0;	//∑µªÿ÷µœ÷Œﬁ“‚“Â
}

//	œµÕ≥∞≤»´º«¬º,◊‘∂Ø∑¢ÀÕµΩsyssecurity∞Ê
//  mode == 0		syssecurity
//	mode == 1		boardsecurity
//  mode == 2		bmsecurity
//  mode == 3		usersecurity
int	securityreport(char *str, int save, int mode)
{
	FILE*	se;
	char    fname[STRLEN];
	int     savemode;
	savemode = uinfo.mode;
	report(str, currentuser.userid);
	sprintf(fname, "tmp/security.%s.%05d", currentuser.userid, uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "œµÕ≥∞≤»´º«¬º\n[1m‘≠“Ú£∫%s[m\n", str);
		if (save){
			fprintf(se, "“‘œ¬ «∏ˆ»À◊ ¡œ:");
			getuinfo(se);
		}
		fclose(se);
		if (mode == 0){
			Postfile(fname, "syssecurity", str, 2);
		} else if (mode == 1){
			Postfile(fname, "boardsecurity", str, 2);
		} else if (mode == 2){
		    Postfile(fname, "bmsecurity", str, 2);
		} else if (mode == 3){
		    Postfile(fname, "usersecurity", str, 2);
		} else if (mode == 4){
		    Postfile(fname, "vote", str, 2);
		}
		unlink(fname);
		modify_user_mode(savemode);
	}
}

int
get_grp(seekstr)
char    seekstr[STRLEN];
{
	FILE   *fp;
	char    buf[STRLEN];
	char   *namep;
	if ((fp = fopen("0Announce/.Search", "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			strtok(NULL, "/");
			namep = strtok(NULL, "/");
			if (strlen(namep) < 30) {
				strcpy(lookgrp, namep);
				return 1;
			} else
				return 0;
		}
	}
	fclose(fp);
	return 0;
}

// «Â∆¡,≤¢‘⁄µ⁄“ª––œ‘ ætitle
void	stand_title(char   *title)
{
	clear();
	standout();
	prints("%s",title);
	standend();
}

int
valid_brdname(brd)
char   *brd;
{
	char    ch;
	ch = *brd++;
	if (!isalnum(ch) && ch != '_' && ch != '.' )
		return 0;
	while ((ch = *brd++) != '\0') {
		if (!isalnum(ch) && ch != '_' && ch != '.')
			return 0;
	}
	return 1;
}

char   *
chgrp()
{
	int     i, ch;
	char    buf[STRLEN], ans[6];
	static char *explain[] = {
		"BBS œµÕ≥",
		"∏¥µ©¥Û—ß",
 		"‘∫œµ∑Á≤…",
 		"µÁƒ‘ºº ı",
 		"–›œ–”È¿÷",
 		"Œƒ—ß“’ ı",
 		"ÃÂ”˝Ω°…Ì",
		"∏––‘ø’º‰",
		"–¬Œ≈–≈œ¢",
 		"—ßø∆—ß ı",
 		"“Ù¿÷”∞ ”",
		"Ωª“◊◊®«¯",
		"“˛≤ÿ∑÷«¯",
		NULL
	};

	static char *groups[] = {
        "system.faq",
 		"campus.faq",
 		"ccu.faq",
 		"comp.faq",
 		"rec.faq",
 		"literal.faq",
 		"sport.faq",
		"talk.faq",
		"news.faq",
 		"sci.faq",
		"other.faq",
		"business.faq",
		"hide.faq",
		NULL
	};
//modified by roly 02.03.08
	clear();
	move(2, 0);
	prints("—°‘Òæ´ª™«¯µƒƒø¬º\n\n");
	for (i = 0;; i++) {
		if (explain[i] == NULL || groups[i] == NULL)
			break;
		prints("[1;32m%2d[m. %-20s%-20s\n", i, explain[i], groups[i]);
	}
	sprintf(buf, "«Î ‰»Îƒ˙µƒ—°‘Ò(0~%d): ", --i);
	while (1) {
		getdata(i + 6, 0, buf, ans, 4, DOECHO, YEA);
		if (!isdigit(ans[0]))
			continue;
		ch = atoi(ans);
		if (ch < 0 || ch > i || ans[0] == '\r' || ans[0] == '\0')
			continue;
		else
			break;
	}
	sprintf(cexplain, "%s", explain[ch]);

	return groups[ch];
}

char    curruser[IDLEN + 2];
extern int delmsgs[];
extern int delcnt;

void
domailclean(fhdrp)
struct fileheader *fhdrp;
{
	static int newcnt, savecnt, deleted, idc;
	char    buf[STRLEN];
	if (fhdrp == NULL) {
		fprintf(cleanlog, "new = %d, saved = %d, deleted = %d\n",
			newcnt, savecnt, deleted);
		newcnt = savecnt = deleted = idc = 0;
		if (delcnt) {
			sprintf(buf, "mail/%c/%s/%s", toupper(curruser[0]), curruser, DOT_DIR);
			while (delcnt--)
				delete_record(buf, sizeof(struct fileheader), delmsgs[delcnt],NULL,NULL);
		}
		delcnt = 0;
		return;
	}
	idc++;
	if (!(fhdrp->accessed[0] & FILE_READ))
		newcnt++;
	else if (fhdrp->accessed[0] & FILE_MARKED)
		savecnt++;
	else {
		deleted++;
		sprintf(buf, "mail/%c/%s/%s", toupper(curruser[0]), curruser, fhdrp->filename);
		unlink(buf);
		delmsgs[delcnt++] = idc;
	}
}

int
cleanmail(urec)
struct userec *urec;
{
	struct stat statb;
	if (urec->userid[0] == '\0' || !strcmp(urec->userid, "new"))
		return 0;
	sprintf(genbuf, "mail/%c/%s/%s", toupper(urec->userid[0]), urec->userid, DOT_DIR);
	fprintf(cleanlog, "%s: ", urec->userid);
	if (stat(genbuf, &statb) == -1)
		fprintf(cleanlog, "no mail\n");
	else if (statb.st_size == 0)
		fprintf(cleanlog, "no mail\n");
	else {
		strcpy(curruser, urec->userid);
		delcnt = 0;
		apply_record(genbuf, domailclean, sizeof(struct fileheader),0,0,0);
		domailclean(NULL);
	}
	return 0;
}


void
trace_state(flag, name, size)
int     flag, size;
char   *name;
{
	char    buf[STRLEN];
	if (flag != -1) {
		sprintf(buf, "ON (size = %d)", size);
	} else {
		strcpy(buf, "OFF");
	}
	prints("%sº«¬º %s\n", name, buf);
}

/*
int
scan_register_form(regfile)
char   *regfile;
{
	static char *field[] = {"usernum", "userid", "realname", "dept",
	"addr", "phone", "assoc", NULL};
	static char *finfo[] = {"’ ∫≈Œª÷√", "…Í«Î’ ∫≈", "’Ê µ–’√˚", "—ß–£œµº∂",
	"ƒø«∞◊°÷∑", "¡™¬ÁµÁª∞", "–£ ”— ª·", NULL};
	static char *reason[] = {"«Î»∑ µÃÓ–¥’Ê µ–’√˚.", "«ÎœÍÃÓ—ß–£ø∆œµ”ÎƒÍº∂.",
		"«ÎÃÓ–¥ÕÍ’˚µƒ◊°÷∑◊ ¡œ.", "«ÎœÍÃÓ¡™¬ÁµÁª∞.",
		"«Î»∑ µÃÓ–¥◊¢≤·…Í«Î±Ì.", "«Î”√÷–ŒƒÃÓ–¥…Í«Îµ•.","NULL","∆‰À˚",
		NULL};
	struct userec uinfo;
	FILE   *fn, *fout, *freg;
	char    fdata[7][STRLEN];
	char    fname[STRLEN], buf[STRLEN];
	char    ans[5], *ptr, *uid;
	int     n, unum;
	uid = currentuser.userid;
	stand_title("“¿–Ú…Ë∂®À˘”––¬◊¢≤·◊ ¡œ");
	sprintf(fname, "%s.tmp", regfile);
	move(2, 0);
	if (dashf(fname)) {
		move(1, 0);
		prints("∆‰À˚ SYSOP ’˝‘⁄≤Èø¥◊¢≤·…Í«Îµ•, «ÎºÏ≤È π”√’ﬂ◊¥Ã¨.\n");
		getdata(2, 0, "ƒ„»∑∂®√ª”–∆‰À˚ SYSOP ‘⁄…Û∫À◊¢≤·µ•¬ £ø [y/N]: ", ans, 2, DOECHO, YEA);
		if (ans[0] == 'Y' || ans[0] == 'y')
			f_cp(fname, regfile, O_APPEND);
		else {
			pressreturn();
			return -1;
		}
	}
	rename(regfile, fname);
	if ((fn = fopen(fname, "r")) == NULL) {
		move(2, 0);
		prints("œµÕ≥¥ÌŒÛ, Œﬁ∑®∂¡»°◊¢≤·◊ ¡œµµ: %s\n", fname);
		pressreturn();
		return -1;
	}
	memset(fdata, 0, sizeof(fdata));
	while (fgets(genbuf, STRLEN, fn) != NULL) {
		if ((ptr = (char *) strstr(genbuf, ": ")) != NULL) {
			*ptr = '\0';
			for (n = 0; field[n] != NULL; n++) {
				if (strcmp(genbuf, field[n]) == 0) {
					strcpy(fdata[n], ptr + 2);
					if ((ptr = (char *) strchr(fdata[n], '\n')) != NULL)
						*ptr = '\0';
				}
			}
		} else if ((unum = getuser(fdata[1])) == 0) {
			move(2, 0);
			clrtobot();
			prints("œµÕ≥¥ÌŒÛ, ≤ÈŒﬁ¥À’ ∫≈.\n\n");
			for (n = 0; field[n] != NULL; n++)
				prints("%s     : %s\n", finfo[n], fdata[n]);
			pressreturn();
			memset(fdata, 0, sizeof(fdata));
		} else {
			memcpy(&uinfo, &lookupuser, sizeof(uinfo));
			move(1, 0);
			prints("’ ∫≈Œª÷√     : %d\n", unum);
			disply_userinfo(&uinfo);
			move(15, 0);
			printdash(NULL);
			for (n = 0; field[n] != NULL; n++)
				prints("%s     : %s\n", finfo[n], fdata[n]);
			if (uinfo.userlevel & PERM_LOGINOK) {
				move(t_lines - 1, 0);
				prints("¥À’ ∫≈≤ª–Ë‘ŸÃÓ–¥◊¢≤·µ•.\n");
				igetkey();
				ans[0] = 'D';
			} else {
				getdata(t_lines - 1, 0, " «∑ÒΩ” ‹¥À◊ ¡œ (Y./N/Q/Del/Skip/0/1/2/3/4/5/6/7)? [S]: ",
					ans, 3, DOECHO, YEA);
			}
			move(1, 0);
			clrtobot();
			switch (ans[0]) {
			case 'D':
			case 'd':
				break;
			case '.':
			case 'Y':
			case 'y':
				prints("“‘œ¬ π”√’ﬂ◊ ¡œ“—æ≠∏¸–¬:\n");
				n = strlen(fdata[5]);
				if (n + strlen(fdata[3]) > 60) {
					if (n > 40)
						fdata[5][n = 40] = '\0';
					fdata[3][60 - n] = '\0';
				}
				strncpy(uinfo.realname, fdata[2], NAMELEN);
				strncpy(uinfo.address, fdata[4], NAMELEN);
				sprintf(genbuf, "%s$%s@%s", fdata[3], fdata[5], uid);
				genbuf[STRLEN - 16] = '\0';
				strncpy(uinfo.reginfo, genbuf, STRLEN - 17);
#ifdef ALLOWGAME
				uinfo.money = 1000;
#endif
				uinfo.lastjustify = time(0);
				substitute_record(PASSFILE, &uinfo, sizeof(uinfo), unum);
				sethomefile(buf, uinfo.userid, "register");
				if (dashf(buf)) {
					sethomefile(genbuf, uinfo.userid, "register.old");
					rename(buf, genbuf);
				}
				if ((fout = fopen(buf, "w")) != NULL) {
					for (n = 0; field[n] != NULL; n++)
						fprintf(fout, "%s: %s\n", field[n], fdata[n]);
					n = time(NULL);
					getdatestring(n,NA);
					fprintf(fout, "Date: %s\n", datestring);
					fprintf(fout, "Approved: %s\n", uid);
					fclose(fout);
				}
				mail_file("etc/s_fill", uinfo.userid, "πßÏ˚ƒ˙£¨ƒ˙“—æ≠ÕÍ≥…◊¢≤·°£");
				sethomefile(buf, uinfo.userid, "mailcheck");
				unlink(buf);
				sprintf(genbuf, "»√ %s Õ®π˝…Ì∑÷»∑»œ.", uinfo.userid);
				securityreport(genbuf);
				break;
			case 'Q':
			case 'q':
				if ((freg = fopen(regfile, "a")) != NULL) {
					for (n = 0; field[n] != NULL; n++)
						fprintf(freg, "%s: %s\n", field[n], fdata[n]);
					fprintf(freg, "----\n");
					while (fgets(genbuf, STRLEN, fn) != NULL)
						fputs(genbuf, freg);
					fclose(freg);
				}
				break;
			case 'N':
			case 'n':
				for (n = 0; field[n] != NULL; n++)
					prints("%s: %s\n", finfo[n], fdata[n]);
				printdash(NULL);
				move(9, 0);
				prints("«Î—°‘Ò/ ‰»ÎÕÀªÿ…Í«Î±Ì‘≠“Ú, ∞¥ <enter> »°œ˚.\n\n");
				for (n = 0; reason[n] != NULL; n++)
					prints("%d) %s\n", n, reason[n]);
				getdata(12 + n, 0, "ÕÀªÿ‘≠“Ú: ", buf, 60, DOECHO, YEA);
				if (buf[0] != '\0') {
					if (buf[0] >= '0' && buf[0] < '0' + n -2) {
						strcpy(buf, reason[buf[0] - '0']);
					}
				// added by roly 02.05.17 
 					else if (buf[0]=='0'+n-1) {
							getdata(13+n,0," ‰»ÎÀµ√˜: ", buf,40,DOECHO,YEA);
					}
 					else if (buf[0]=='0'+n-2) {
							strcpy(buf,"ƒ˙µƒ◊¢≤·µ•≤ªƒ‹±ªΩ” ‹");
					}
				// add end 
					sprintf(genbuf, "<◊¢≤· ß∞‹> - %s", buf);
					strncpy(uinfo.address, genbuf, NAMELEN);
					substitute_record(PASSFILE, &uinfo, sizeof(uinfo), unum);
					mail_file("etc/f_fill", uinfo.userid, uinfo.address);
					// user_display( &uinfo, 1 );
					// pressreturn();
					break;
				}
				move(10, 0);
				clrtobot();
				prints("»°œ˚ÕÀªÿ¥À◊¢≤·…Í«Î±Ì.\n");
				// run default -- put back to regfile
			//added by money 2003.09.17, for don't need enter N menu
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				if (ans[0] < '6')
					strcpy(buf, reason[ans[0] - '0']);
				else if (ans[0] == '6')
					getdata(13+n,0," ‰»ÎÀµ√˜: ", buf,40,DOECHO,YEA);
				else
					strcpy(buf,"ƒ˙µƒ◊¢≤·µ•≤ªƒ‹±ªΩ” ‹");
				sprintf(genbuf, "<◊¢≤· ß∞‹> - %s", buf);
				strncpy(uinfo.address, genbuf, NAMELEN);
				substitute_record(PASSFILE, &uinfo, sizeof(uinfo), unum);
				mail_file("etc/f_fill", uinfo.userid, uinfo.address);
				break;
			// copyed from N menu
			// add end 
			default:
				if ((freg = fopen(regfile, "a")) != NULL) {
					for (n = 0; field[n] != NULL; n++)
						fprintf(freg, "%s: %s\n", field[n], fdata[n]);
					fprintf(freg, "----\n");
					fclose(freg);
				}
			}
			memset(fdata, 0, sizeof(fdata));
		}
	}
	fclose(fn);
	unlink(fname);
	return (0);
}
*/

#ifndef WITHOUT_ADMIN_TOOLS
extern int cmpuids();
extern int t_cmpuids();
int kick_user( struct user_info *userinfo )
{
   int     id, ind;
   struct user_info uin;
   struct userec kuinfo;
   char    kickuser[40], buffer[40];
   
   if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
   if (!(HAS_PERM(PERM_OBOARDS))) return 0;
      modify_user_mode(ADMIN);
      stand_title("Ãﬂ π”√’ﬂœ¬’æ");
      move(1, 0);
      usercomplete(" ‰»Î π”√’ﬂ’ ∫≈: ", kickuser);
      if (*kickuser == '\0') {
         clear();
	 return 0;
      }
      if (!(id = getuser(kickuser))) {
         move(3, 0);
	 prints("Œﬁ–ßµƒ”√ªß ID£°");
	 clrtoeol();
	 pressreturn();
	 clear();
	 return 0;
      }
      move(1, 0);
      clrtoeol();
      sprintf(genbuf,"ÃﬂµÙ π”√’ﬂ : [%s].", kickuser);
      move(2, 0);
      if (askyn(genbuf, NA, NA) == NA) {
         move(2, 0);
	 prints("»°œ˚Ãﬂ π”√’ﬂ..\n");
	 pressreturn();
	 clear();
	 return 0;
      }
      search_record(PASSFILE, &kuinfo, sizeof(kuinfo), cmpuids, kickuser);
      ind = search_ulist(&uin, t_cmpuids, id);
   } else {
      uin = *userinfo;
      strcpy(kickuser, uin.userid);
      ind = YEA;
   }
   if (!ind||!uin.active||(uin.pid && kill(uin.pid, 0)==-1)) {
      if(uinfo.mode!=LUSERS&&uinfo.mode!=OFFLINE&&uinfo.mode!=FRIEND) {
         move(3, 0);
	 prints("”√ªß [%s] ≤ª‘⁄œﬂ…œ",kickuser);
	 clrtoeol();
	 pressreturn();
	 clear();
      }
      return 0;
   }
     if (uin.mode == 10001 ) {
	 kill (uin.pid,SIGABRT);
     }
     else {
	 kill(uin.pid, SIGHUP);
     }
     //kill (uin.pid, SIGHUP);
   sprintf(buffer, "kick out %s", kickuser);
   report(buffer, currentuser.userid);
   kuinfo.userid[IDLEN]=0;        //added by iamfat 2004.01.05 to avoid overflow
   kuinfo.username[NAMELEN-1]=0;        //added by iamfat 2004.01.05 to avoid overflow
   sprintf(genbuf, "%s (%s)", kuinfo.userid, kuinfo.username);
   log_usies("KICK ", genbuf, &currentuser);
   move(2, 0);
   if(uinfo.mode!=LUSERS&&uinfo.mode!=OFFLINE&&uinfo.mode!=FRIEND) {
      prints("”√ªß [%s] “—æ≠±ªÃﬂœ¬’æ.\n",kickuser);
      pressreturn();
      clear();
   }
   return 1;
}
#endif
