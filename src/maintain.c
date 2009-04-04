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
		if (toboard) {
    		if (mode == 1) {
				Postfile(fname, currboard, title, 3);
			} else if (mode == 2) {
				Postfile(fname, currboard, title, 2);
			} else {
				Postfile(fname, currboard, title, 1);
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

#ifndef WITHOUT_ADMIN_TOOLS
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
   if (!ind||!uin.active||(uin.pid && bbskill(&uin, 0)==-1)) {
      if(uinfo.mode!=LUSERS&&uinfo.mode!=OFFLINE&&uinfo.mode!=FRIEND) {
         move(3, 0);
	 prints("”√ªß [%s] ≤ª‘⁄œﬂ…œ",kickuser);
	 clrtoeol();
	 pressreturn();
	 clear();
      }
      return 0;
   }
     if (uin.mode == WWW ) {
	 	bbskill(&uin, SIGABRT);
     }
     else {
	 	bbskill(&uin, SIGHUP);
     }
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
