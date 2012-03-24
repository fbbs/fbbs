#include "bbs.h"
#include "fbbs/fbbs.h"
#include "fbbs/session.h"
#include "fbbs/terminal.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

char    cexplain[STRLEN];
char    lookgrp[30];
FILE   *cleanlog;

//	ÏµÍ³°²È«¼ÇÂ¼,×Ô¶¯·¢ËÍµ½syssecurity°æ
//  mode == 0		syssecurity
//	mode == 1		boardsecurity
//  mode == 2		bmsecurity
//  mode == 3		usersecurity
void securityreport(char *str, int save, int mode)
{
	FILE*	se;
	char    fname[STRLEN];
	int     savemode;
	savemode = session.status;
	report(str, currentuser.userid);
	sprintf(fname, "tmp/security.%s.%05d", currentuser.userid, session.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼\n[1mÔ­Òò£º%s[m\n", str);
		if (save){
			fprintf(se, "ÒÔÏÂÊÇ¸öÈË×ÊÁÏ:");
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
		set_user_status(savemode);
	}
}

//	ºË¶ÔÏµÍ³ÃÜÂë
int	check_systempasswd()
{
	FILE*	pass;
	char    passbuf[20], prepass[STRLEN];
	clear();
	if ((pass = fopen("etc/.syspasswd", "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen(prepass) - 1] = '\0';
		getdata(1, 0, "ÇëÊäÈëÏµÍ³ÃÜÂë: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!passwd_match(prepass, passbuf)) {
			move(2, 0);
			prints("´íÎóµÄÏµÍ³ÃÜÂë...");
			securityreport("ÏµÍ³ÃÜÂëÊäÈë´íÎó...", 0, 0);
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

//	×Ô¶¯·¢ËÍµ½°æÃæ
//			title		±êÌâ
//			str			ÄÚÈÝ
//			toboard		¾ö¶¨ÊÇ·ñ·¢ËÍµ½°æÃæ	
//			userid		·¢ËÍµ½µÄÓÃ»§Ãû,ÎªnullÔò²»·¢ËÍ.
//			mode		·Ö±ð½±³Í,1±íÊ¾BMSÈÎÃü,0±íÊ¾deliver´¦·£
//					2±íÊ¾µ±Ç°ÓÃ»§
int autoreport(char *title,char *str,int toboard,char *userid,int mode)
{
	FILE	*se;
    char	fname[STRLEN];
    int 	savemode;
	
    savemode = session.status;
    report(title, currentuser.userid);
    sprintf(fname,"tmp/AutoPoster.%s.%05d",currentuser.userid, session.pid);
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
        set_user_status(savemode);
    }
	return 0;	//·µ»ØÖµÏÖÎÞÒâÒå
}

// ÇåÆÁ,²¢ÔÚµÚÒ»ÐÐÏÔÊ¾title
void	stand_title(char   *title)
{
	clear();
	standout();
	prints("%s",title);
	standend();
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
