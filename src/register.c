//deardrago 2000.09.27  over
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
 $Id: register.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include <gd.h>
#include <gdfontl.h>
#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15
char *sysconf_str();
char *genpasswd();

extern char fromhost[60];
extern time_t login_start_time;
extern char *cexpstr();
time_t system_time;

#ifdef ALLOWSWITCHCODE
extern int convcode;
#endif

/* Add by Amigo. 2001.02.13. Called by ex_strcmp. */
/* Compares at most n characters of s2 to s1 from the tail. */
int ci_strnbcmp(register char *s1, register char *s2, int n) {

	char *s1_tail, *s2_tail;
	char c1, c2;

	s1_tail = s1 + strlen(s1) - 1;
	s2_tail = s2 + strlen(s2) - 1;

	while ( (s1_tail >= s1 ) && (s2_tail >= s2 ) && n --) {
		c1 = *s1_tail --;
		c2 = *s2_tail --;
		if (c1 >= 'a' && c1 <= 'z')
			c1 += 'A' - 'a';
		if (c2 >= 'a' && c2 <= 'z')
			c2 += 'A' - 'a';
		if (c1 != c2)
			return (c1 - c2);
	}
	if ( ++n)
		if ( (s1_tail < s1 ) && (s2_tail < s2 ))
			return 0;
		else if (s1_tail < s1)
			return -1;
		else
			return 1;
	else
		return 0;
}

/* Add by Amigo. 2001.02.13. Called by bad_user_id. */
/* Compares userid to restrictid. */
/* Restrictid support * match in three style: prefix*, *suffix, prefix*suffix. */
/* Prefix and suffix can't contain *. */
/* Modified by Amigo 2001.03.13. Add buffer strUID for userid. Replace all userid with strUID. */
int ex_strcmp(char *restrictid, char *userid) {

	/* Modified by Amigo 2001.03.13. Add definition for strUID. */
	char strBuf[STRLEN ], strUID[STRLEN ], *ptr;
	int intLength;

	/* Added by Amigo 2001.03.13. Add copy lower case userid to strUID. */
	intLength = 0;
	while ( *userid)
		if ( *userid >= 'A' && *userid <= 'Z')
			strUID[ intLength ++ ] = (*userid ++) - 'A' + 'a';
		else
			strUID[ intLength ++ ] = *userid ++;
	strUID[ intLength ] = '\0';

	intLength = 0;
	/* Modified by Amigo 2001.03.13. Copy lower case restrictid to strBuf. */
	while ( *restrictid)
		if ( *restrictid >= 'A' && *restrictid <= 'Z')
			strBuf[ intLength ++ ] = (*restrictid ++) - 'A' + 'a';
		else
			strBuf[ intLength ++ ] = *restrictid ++;
	strBuf[ intLength ] = '\0';

	if (strBuf[ 0 ] == '*' && strBuf[ intLength - 1 ] == '*') {
		strBuf[ intLength - 1 ] = '\0';
		if (strstr(strUID, strBuf + 1) != NULL)
			return 0;
		else
			return 1;
	} else if (strBuf[ intLength - 1 ] == '*') {
		strBuf[ intLength - 1 ] = '\0';
		return strncasecmp(strBuf, strUID, intLength - 1);
	} else if (strBuf[ 0 ] == '*') {
		return ci_strnbcmp(strBuf + 1, strUID, intLength - 1);
	} else if ( (ptr = strstr(strBuf, "*") ) != NULL) {
		return (strncasecmp(strBuf, strUID, ptr - strBuf) || ci_strnbcmp(
				strBuf, strUID, intLength - 1 - (ptr - strBuf )) );
	}
	return 1;
}
/*
 Commented by Erebus 2004-11-08 called by getnewuserid(),new_register()
 configure ".badname" to restrict user id 
 */

int bad_user_id(char *userid) {
	/*
	 FILE   *fp;
	 char    buf[STRLEN], ptr2[IDLEN + 2],*ptr, ch;

	 ptr = userid;
	 while ((ch = *ptr++) != '\0') {
	 if (!isalnum(ch) && ch != '_')
	 return 1;
	 }
	 if( !strcasecmp(userid,BBSID) ) return 1;
	 if ((fp = fopen(".badname", "r")) != NULL) {
	 strtolower(ptr2, userid);
	 while (fgets(buf, STRLEN, fp) != NULL) {
	 ptr = strtok(buf, " \n\t\r");
	 if (ptr != NULL && *ptr != '#'){
	 if(  (ptr[0] == '*' && strstr(ptr2, &ptr[1]) != NULL)
	 ||(ptr[0] != '*' && !strcmp(ptr2,ptr)) ) {
	 fclose(fp); 
	 return 1;
	 }
	 }
	 }
	 fclose(fp);
	 }
	 return 0;
	 */

	FILE *fp;
	char buf[STRLEN];
	char *ptr, ch;

	ptr = userid;
	while ( (ch = *ptr++) != '\0') {
		if ( !isalnum(ch) && ch != '_')
			return 1;
	}
	if ( (fp = fopen(".badname", "r")) != NULL) {
		while (fgets(buf, STRLEN, fp) != NULL) {
			ptr = strtok(buf, " \n\t\r");
			/* Modified by Amigo. 2001.02.13.8. * match support added. */
			/* Original: if( ptr != NULL && *ptr != '#' && strcasecmp( ptr, userid ) == 0 ) {*/
			if (ptr != NULL && *ptr != '#' && (strcasecmp(ptr, userid) == 0
					|| ex_strcmp(ptr, userid) == 0 )) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

/*2003.06.02 modified by stephen to enhance the function of users' life      */
/*
 Commented by Erebus 2004-11-08 
 user must register in 30 mins
 !PERM_LOGINOK max_hp=15
 */
int compute_user_value(struct userec *urec) {
	int value, value2;
	value = (time(0) - urec->lastlogin);
	value2 = (time(0) - urec->firstlogin); //×¢²áÊ±¼ä
	// new user should register in 30 mins
	if (strcmp(urec->userid, "new") == 0) {
		return 30 * 60 - value;
	}
#ifdef FDQUAN
	if ((urec->userlevel & PERM_XEMPT)
			|| strcmp(urec->userid, "SYSOP") == 0
			|| strcmp(urec->userid, "guest") == 0)
	return 999;
	if (!(urec->userlevel & PERM_REGISTER))
	return 14 - value / (24 * 60 * 60);
	if (value2 >= 5 * 365 * 24 * 60 * 60)
	return 666 - value / (24 * 60 * 60);
	if (value2 >= 2 * 365 * 24 * 60 * 60)
	return 365 - value / (24 * 60 * 60);
	return 150 - value / (24 * 60 * 60);
#else
	if (((urec->userlevel & PERM_XEMPT) && (urec->userlevel
			& PERM_LONGLIFE)) || strcmp(urec->userid, "SYSOP") == 0
			|| strcmp(urec->userid, "guest") == 0)
		return 999;
	if ((urec->userlevel & PERM_XEMPT) && !(urec->userlevel
			& PERM_LONGLIFE))
		return 666;
	if (!(urec->userlevel & PERM_REGISTER))
		return 14 - value / (24 * 60 * 60);
	if (!(urec->userlevel & PERM_XEMPT) && (urec->userlevel
			& PERM_LONGLIFE))
		return 365 - value / (24 * 60 * 60);
	if (value2 >= 3 * 365 * 24 * 60 * 60)
		return 180 - value / (24 * 60 * 60);
	return 120 - value / (24 * 60 * 60);
#endif
}
/*2003.06.02 stephen modify end*/
/*
 Commented by Erebus 2004-11-08 ,called by new_register()
 if the hp of user is lower than zero ,rm "home/A/abc" and "mail/A/abc"
 use systme("/bin/rm -fr filefolder")
 */

int getnewuserid() {
	struct userec utmp;
	char genbuf_rm[STRLEN]; //added by roly 02.03.22
#ifndef SAVELIVE

	struct userec zerorec;
	int size;
#endif

	struct stat st;
	int fd, val, i;
	/* Following line added by Amigo 2002.04.03. Change clean account time. */
	struct tm *area;
	FILE *fdtmp;
	char nname[50];
	int exp, perf; /* Add by SmallPig */

	system_time = time(0);
	/* Following line added by Amigo 2002.04.03. Change clean account time. */
	area = localtime(&system_time);
#ifndef SAVELIVE
	/* Following line modified by Amigo 2002.04.03. Change clean account time. */
	/*   if (stat("tmp/killuser", &st)==-1 || st.st_mtime+3600 < system_time){ */
	if (stat("tmp/killuser", &st)==-1 || ( (st.st_mtime+72000
			< system_time) && (area->tm_hour < 12) && (area->tm_hour > 5) )) {
		if ((fd = open("tmp/killuser", O_RDWR | O_CREAT, 0600)) == -1)
			return -1;
		getdatestring(system_time, NA);
		write(fd, datestring, 29);
		close(fd);
		strcpy(nname, "tmp/bbs.killid");
		fdtmp = fopen(nname, "w+");
		log_usies("CLEAN", "dated users.");
		prints("Ñ°ÕÒĞÂÕÊºÅÖĞ, ÇëÉÔ´ıÆ¬¿Ì...\n");
		memset(&zerorec, 0, sizeof(zerorec));
		//      if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0600)) == -1)
		//        return -1;
		//      flock(fd, LOCK_EX);
		size = sizeof(utmp);
		for (i = 0; i < MAXUSERS; i++) {
			//         if (read(fd, &utmp, size) != size) break;
			getuserbyuid( &utmp, i+1);
			val = compute_user_value(&utmp);
			if (utmp.userid[0] != '\0' && val < 0) {
				getdatestring(utmp.lastlogin, NA);
				utmp.userid[IDLEN]=0; //added by iamfat 2004.01.05 to avoid overflow
				sprintf(genbuf, "#%d %-12s %14.14s %d %d %d", i + 1,
						utmp.userid, datestring, utmp.numlogins,
						utmp.numposts, val);
				log_usies("KILL ", genbuf);
				//if (!bad_user_id(utmp.userid)) {
				{
					sprintf(genbuf, "mail/%c/%s", toupper(utmp.userid[0]),
							utmp.userid);
					fprintf(
							fdtmp,
							"\033[1;37m%s \033[m(\033[1;33m%s\033[m) ¹²ÉÏÕ¾ \033[1;32m%d\033[m ´Î [\033[1;3%dm%s\033[m]\n",
							utmp.userid, utmp.username, utmp.numlogins,
							(utmp.gender == 'F') ? 5 : 6,
							horoscope(utmp.birthmonth, utmp.birthday));
					getdatestring(utmp.lastlogin, NA);
					fprintf(
							fdtmp,
							"ÉÏ ´Î ÔÚ:[[1;32m%s[m] ´Ó [[1;32m%s[m] µ½±¾Õ¾Ò»ÓÎ¡£\n",
							datestring, (utmp.lasthost[0]=='\0' ? "(²»Ïê)"
									: utmp.lasthost));

					getdatestring(utmp.lastlogout, NA);
					fprintf(fdtmp, "ÀëÕ¾Ê±¼ä:[[1;32m%s[m] ", datestring);

					exp = countexp(&utmp);
					perf = countperf(&utmp);
					fprintf(fdtmp, "±íÏÖÖµ:"
#ifdef SHOW_PERF
							"%d([1;33m%s[m)"
#else
							"[[1;33m%s[m]"
#endif
							" ĞÅÏä:[[5;1;32m%2s[m]\n"
#ifdef SHOW_PERF
							, perf
#endif
							, cperf(perf),
							(check_query_mail(genbuf) == 1) ? "ĞÅ" : "  ");

#ifdef ALLOWGAME
					fprintf(fdtmp, "ÒøĞĞ´æ¿î: [[1;32m%dÔª[m] Ä¿Ç°´û¿î: [[1;32m%dÔª[m]([1;33m%s[m) ¾­ÑéÖµ£º[[1;32m%d[m]([1;33m%s[m)¡£\n",
							utmp.money,utmp.bet,
							cmoney(utmp.money-utmp.bet),exp,cexpstr(exp));

					fprintf(fdtmp, "ÎÄ ÕÂ Êı: [[1;32m%d[m] ½±ÕÂÊı: [[1;32m%d[m]([1;33m%s[m) ÉúÃüÁ¦£º[[1;32m%d[m] ÍøÁä[[1;32m%dÌì[m]\n\n",
							utmp.numposts,
							utmp.nummedals,cnummedals(utmp.nummedals),
							compute_user_value(&utmp),(time (0) - utmp.firstlogin) / 86400);
#else
					fprintf(fdtmp, "ÎÄ ÕÂ Êı:[[1;32m%d[m] ¾­ Ñé Öµ:"
#ifdef SHOWEXP
							"%d([1;33m%-10s[m)"
#else
							"[[1;33m%-10s[m]"
#endif
							" ÉúÃüÁ¦:[[1;32m%d[m] ÍøÁä[[1;32m%dÌì[m]\n\n",
							utmp.numposts,
#ifdef SHOWEXP
							exp,
#endif
							cexpstr(exp), compute_user_value(&utmp),
							(time(0) - utmp.firstlogin) / 86400);

#endif
					sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
					if (!strncmp(genbuf+7, utmp.userid,
							strlen(utmp.userid))) {
						f_rm(genbuf);
						system(genbuf_rm); //added by roly 02.03.24
					}
					sprintf(genbuf, "home/%c/%s", toupper(utmp.userid[0]),
							utmp.userid);
					sprintf(genbuf_rm, "/bin/rm -fr %s", genbuf); //added by roly 02.03.24
					if (!strncmp(genbuf+7, utmp.userid,
							strlen(utmp.userid))) {
						f_rm(genbuf);
						system(genbuf_rm); //added by roly 02.03.24
					}
				}
				/*
				 if(lseek(fd, (off_t)(-size), SEEK_CUR)==-1){
				 flock(fd, LOCK_UN);
				 close(fd);
				 return -1;
				 }
				 write(fd, &zerorec, sizeof(utmp));
				 */
				substitut_record(PASSFILE, &zerorec,
						sizeof(struct userec), i+1);
				del_uidshm(i+1, utmp.userid);
			}
		}
		//      flock(fd, LOCK_UN);
		//      close(fd);
		fclose(fdtmp);
		getdatestring(system_time, NA);
		sprintf(genbuf, "[%8.8s %6.6s] ±¾ÈÕËæ·çÆ®ÊÅµÄID", datestring+6,
				datestring + 23);
		Postfile(nname, "newcomers", genbuf, 1);
		touchnew();
	}
#endif   // of SAVELIVE
	//   if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0600)) == -1) return -1;
	//   flock(fd, LOCK_EX);
	i = searchnewuser();
	fromhost[59]=0; //added by iamfat 2004.01.05 to avoid overflow
	sprintf(genbuf, "uid %d from %s", i, fromhost);
	log_usies("APPLY", genbuf);
	if (i <= 0 || i > MAXUSERS) {
		//      flock(fd, LOCK_UN);
		//      close(fd);
		prints("±§Ç¸, Ê¹ÓÃÕßÕÊºÅÒÑ¾­ÂúÁË, ÎŞ·¨×¢²áĞÂµÄÕÊºÅ.\n\r");
		val = (st.st_mtime - system_time + 3660) / 60;
		prints("ÇëµÈ´ı %d ·ÖÖÓááÔÙÊÔÒ»´Î, ×£ÄúºÃÔË.\n\r", val);
		sleep(2);
		exit(1);
	}
	memset(&utmp, 0, sizeof(utmp));
	strcpy(utmp.userid, "new");
	utmp.lastlogin = time(0);
	/*
	 if (lseek(fd, (off_t)(sizeof(utmp) * (i - 1)), SEEK_SET) == -1) {
	 flock(fd, LOCK_UN);
	 close(fd);
	 return -1;
	 }
	 write(fd, &utmp, sizeof(utmp));
	 //setuserid(i, utmp.userid);
	 flock(fd, LOCK_UN);
	 close(fd);
	 */
	substitut_record(PASSFILE, &utmp, sizeof(struct userec), i);
	return i;
}

//	useridÈ«×ÖÄ¸·µ»Ø0,·ñÔò·µ»Ø1
int id_with_num(char userid[IDLEN + 1]) {
	char *s;
	for (s = userid; *s != '\0'; s++)
		if (*s < 1 || !isalpha(*s))
			return 1;
	return 0;
}

#ifndef FDQUAN
const char *generate_verify_num() {
	/* Declare the image */
	gdImagePtr im;
	/* Declare output files */
	//FILE *gifout;
	/* Declare color indexes */
	int black;
	int white;
	int x, y, z;
	int rd;
	static char s[10];

	/* Allocate the image: 64 pixels across by 64 pixels tall */
	im = gdImageCreate(40, 16);

	/* Allocate the color black (red, green and blue all minimum).
	 Since this is the first color in a new image, it will
	 be the background color. */
	black = gdImageColorAllocate(im, 0, 0, 0);

	white = gdImageColorAllocate(im, 255, 255, 255);

	srandom(time(0)%getpid());

	rd=random()%(100000);
	sprintf(s, "%05d", rd);
	gdImageString(im, gdFontGetLarge(), 0, 0, s, white);
	for (z=0; z<20; z++) {
		x=random()%(im->sx);
		y=random()%(im->sy);
		gdImageSetPixel(im, x, y, white);
	}
	for (y=0; y<im->sy; y++) {
		for (x=0; x<im->sx; x++) {
			if (gdImageGetPixel(im, x, y))
				outc('o');
			else
				outc(' ');
		}
		outc('\n');
	}

	gdImageDestroy(im);

	oflush();

	return s;
}
#endif

void new_register() {
	struct userec newuser;
	char passbuf[STRLEN];
	int allocid, tried;
#ifndef FDQUAN
	char verify_code[IDLEN+1];
	const char *verify_num;
	int sec;
	char log[100];
#endif
	if (dashf("NOREGISTER")) {
		ansimore("NOREGISTER", NA);
		pressreturn();
		exit(1);
	}
	ansimore("etc/register", NA);
	tried = 0;
	prints("\n");
	while (1) {
		if (++tried >= 9) {
			prints("\n°İ°İ£¬°´Ì«¶àÏÂ  <Enter> ÁË...\n");
			refresh();
			longjmp(byebye, -1);
		}
#ifndef FDQUAN
		getdata(22, 0, "ÄúÊÇ·ñ×ĞÏ¸ÔÄ¶Á¹ı±¾Õ¾Announce°æ¾«»ªÇøx-3Ä¿Â¼ËùÁĞÕ¾¹æÖ´ĞĞ°ì·¨²¢±íÊ¾Í¬Òâ[y/N]",
				verify_code, IDLEN + 1, DOECHO, YEA);

		if (verify_code[0] != 'Y' && verify_code[0] != 'y') {
			exit(0);
		}

		verify_num=generate_verify_num();
		getdata(0, 0, "ÇëÊäÈëÉÏÃæÏÔÊ¾µÄÊı×Ö: ", verify_code, IDLEN + 1, DOECHO, YEA);
#endif
		getdata(0, 0, "ÇëÊäÈëÕÊºÅÃû³Æ (Enter User ID, \"0\" to abort): ",
				passbuf, IDLEN + 1, DOECHO, YEA);
		if (passbuf[0] == '0') {
			longjmp(byebye, -1);
		}
#ifndef FDQUAN
		sec=random()%5;
		prints("Îª±ÜÃâÓëÆäËû×¢²áÕß³åÍ»...ÇëÄÍĞÄµÈºò%dÃë...\n", sec);
		oflush();
		sleep(sec);

		if (strcmp(verify_num, verify_code)) {
			sprintf(log, "verify '%s' error with code %s!=%s from %s",
					passbuf, verify_num, verify_code, fromhost);
			report(log);
			prints("±§Ç¸, ÄúÊäÈëµÄÑéÖ¤Âë²»ÕıÈ·.\n");
			continue;
		}

		sprintf(log, "verify '%s' with code %s from %s ", passbuf,
				verify_code, fromhost);
		report(log);

#endif
		if (id_with_num(passbuf)) {
			prints("ÕÊºÅ±ØĞëÈ«ÎªÓ¢ÎÄ×ÖÄ¸!\n");
		} else if (strlen(passbuf) < 2) {
			prints("ÕÊºÅÖÁÉÙĞèÓĞÁ½¸öÓ¢ÎÄ×ÖÄ¸!\n");
		} else if ((*passbuf == '\0') || bad_user_id(passbuf)) {
			prints("±§Ç¸, Äú²»ÄÜÊ¹ÓÃÕâ¸ö×Ö×÷ÎªÕÊºÅ¡£ ÇëÏë¹ıÁíÍâÒ»¸ö¡£\n");
		} else if (dosearchuser(passbuf)) {
			prints("´ËÕÊºÅÒÑ¾­ÓĞÈËÊ¹ÓÃ\n");
		} else
			break;
	}

	memset(&newuser, 0, sizeof(newuser));
	/*
	 allocid = getnewuserid();
	 if (allocid > MAXUSERS || allocid <= 0) {
	 prints("No space for new users on the system!\n\r");
	 exit(1);
	 } 
	 */
	strcpy(newuser.userid, passbuf);
	strcpy(passbuf, "");

	/*2003.04.30 modified by stephen to add new-users' setting passwd limit*/
	for (tried = 0; tried <=7; tried ++) {
		getdata(0, 0, "ÇëÉè¶¨ÄúµÄÃÜÂë (Setup Password): ", passbuf, PASSLEN,
				NOECHO, YEA);
		if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
			prints("ÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÇëÖØĞÂÊäÈë\n");
			continue;
		}
		strncpy(newuser.passwd, passbuf, PASSLEN);
		getdata(0, 0, "ÇëÔÙÊäÈëÒ»´ÎÄúµÄÃÜÂë (Reconfirm Password): ", passbuf,
				PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, newuser.passwd, PASSLEN) != 0) {
			prints("ÃÜÂëÊäÈë´íÎó, ÇëÖØĞÂÊäÈëÃÜÂë.\n");
			continue;
		}
		passbuf[8] = '\0';
#ifdef ENCPASSLEN

		strncpy(newuser.passwd, genpasswd(passbuf), ENCPASSLEN);
#else

		strncpy(newuser.passwd, genpasswd(passbuf), PASSLEN);
#endif

		break;
	}
	if (tried == 8) {
		sleep(1);
		exit(1);
	}
	/*2003.04.30 modify end*/

	strcpy(newuser.termtype, "vt100");
	newuser.gender = 'X';
#ifdef ALLOWGAME

	newuser.money = 1000;
#endif

	newuser.userdefine = -1;
	if (!strcmp(newuser.userid, "guest")) {
		newuser.userlevel = 0;
		newuser.userdefine &= ~(DEF_FRIENDCALL | DEF_ALLMSG
				| DEF_FRIENDMSG);
	} else {
		newuser.userlevel = PERM_LOGIN;
		newuser.flags[0] = PAGER_FLAG;
	}

	newuser.userdefine &= ~(DEF_NOLOGINSEND);
#ifdef ALLOWSWITCHCODE

	if (convcode)
	newuser.userdefine&=~DEF_USEGB;
#endif

	newuser.flags[1] = 0;
	newuser.firstlogin = newuser.lastlogin = time(0);
	/* added by roly */
	sprintf(genbuf, "/bin/rm -fr %s/mail/%c/%s", BBSHOME,
			toupper(newuser.userid[0]), newuser.userid) ;
	system(genbuf);
	sprintf(genbuf, "/bin/rm -fr %s/home/%c/%s", BBSHOME,
			toupper(newuser.userid[0]), newuser.userid) ;
	system(genbuf);
	/* add end */

	allocid = getnewuserid();
	if (allocid > MAXUSERS || allocid <= 0) {
		prints("No space for new users on the system!\n\r");
		exit(1);
	}
	setuserid(allocid, newuser.userid);
	if (substitut_record(PASSFILE, &newuser, sizeof(newuser), allocid)
			== -1) {
		prints("too much, good bye!\n");
		oflush();
		sleep(2);
		exit(1);
	}
	if (!dosearchuser(newuser.userid)) {
		prints("User failed to create\n");
		oflush();
		sleep(2);
		exit(1);
	}
	sprintf(genbuf, "new account from %s", fromhost);
	report(genbuf);
	prints("ÇëÖØĞÂµÇÂ½ %s ²¢ÌîĞ´×¢²áĞÅÏ¢\n", newuser.userid);
	pressanykey();
	exit(0);

}

int invalid_email(char *addr) {
	FILE *fp;
	char temp[STRLEN], tmp2[STRLEN];

	if (strlen(addr)<3)
		return 1;

	strtolower(tmp2, addr);
	if (strstr(tmp2, "bbs") != NULL)
		return 1;

	if ((fp = fopen(".bad_email", "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			strtolower(genbuf, temp);
			if (strstr(tmp2, genbuf)!=NULL||strstr(genbuf, tmp2) != NULL) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

int check_register_ok(void) {
	char fname[STRLEN];

	sethomefile(fname, currentuser.userid, "register");
	if (dashf(fname)) {
		move(21, 0);
		prints("¹§ºØÄú!! ÄúÒÑË³ÀûÍê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø,\n");
		prints("´ÓÏÖÔÚÆğÄú½«ÓµÓĞÒ»°ãÊ¹ÓÃÕßµÄÈ¨ÀûÓëÒåÎñ...\n");
		pressanykey();
		return 1;
	}
	return 0;
}
//#ifdef MAILCHECK
//#ifdef CODE_VALID
char *genrandpwd(int seed) {
	char panel[]=
			"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *result;
	int i, rnd;

	result = (char *) malloc(RNDPASSLEN + 1);
	srand((unsigned) (time(0) * seed));
	memset(result, 0, RNDPASSLEN + 1);
	for (i = 0; i < RNDPASSLEN; i++) {
		rnd = rand() % sizeof(panel);
		if (panel[rnd] == '\0') {
			i--;
			continue;
		}
		result[i] = panel[rnd];
	}
	sethomefile(genbuf, currentuser.userid, ".regpass");
	unlink(genbuf);
	file_append(genbuf, result);
	return ((char *) result);
}
//#endif

void send_regmail(struct userec *trec) {
	time_t code;
	FILE *fin, *fout, *dp;
#ifdef CODE_VALID

	char buf[RNDPASSLEN + 1];
#endif

	sethomefile(genbuf, trec->userid, "mailcheck");
	if ((dp = fopen(genbuf, "w")) == NULL)
		return;
	code = time(0);
	fprintf(dp, "%9.9d:%d\n", code, getpid());
	fclose(dp);

	sprintf(genbuf, "%s -f %s.bbs@%s %s ", SENDMAIL, trec->userid,
			BBSHOST, trec->email);
	fout = popen(genbuf, "w");
	fin = fopen("etc/mailcheck", "r");
	if ((fin != NULL) && (fout != NULL)) {
		fprintf(fout, "Reply-To: SYSOP.bbs@%s\n", BBSHOST);
		fprintf(fout, "From: SYSOP.bbs@%s\n", BBSHOST);
		fprintf(fout, "To: %s\n", trec->email);
		fprintf(fout, "Subject: @%s@[-%9.9d:%d-]%s mail check.\n",
				trec->userid, code, getpid(), BBSID);
		fprintf(fout, "X-Purpose: %s registration mail.\n", BBSNAME);
		fprintf(fout, "\n");
		fprintf(fout, "[ÖĞÎÄ]\n");
		fprintf(fout, "BBS Î»Ö·         : %s (%s)\n", BBSHOST, BBSIP);
		fprintf(fout, "Äú×¢²áµÄ BBS ID  : %s\n", trec->userid);
		fprintf(fout, "ÉêÇëÈÕÆÚ         : %s", ctime(&trec->firstlogin));
		fprintf(fout, "µÇÈëÀ´Ô´         : %s\n", fromhost);
		fprintf(fout, "ÄúµÄÕæÊµĞÕÃû/êÇ³Æ: %s (%s)\n", trec->realname,
				trec->username);
#ifdef CODE_VALID

		sprintf(buf, "%s", (char *) genrandpwd((int) getpid()));
		fprintf(fout, "×¢²áÂë           : %s (Çë×¢Òâ´óĞ¡Ğ´)\n", buf);
#endif

		fprintf(fout, "ÈÏÖ¤ĞÅ·¢³öÈÕÆÚ   : %s\n", ctime(&code));

		fprintf(fout, "[English]\n");
		fprintf(fout, "BBS LOCATION     : %s (%s)\n", BBSHOST, BBSIP);
		fprintf(fout, "YOUR BBS USER ID : %s\n", trec->userid);
		fprintf(fout, "APPLICATION DATE : %s", ctime(&trec->firstlogin));
		fprintf(fout, "LOGIN HOST       : %s\n", fromhost);
		fprintf(fout, "YOUR NICK NAME   : %s\n", trec->username);
		fprintf(fout, "YOUR NAME        : %s\n", trec->realname);
#ifdef CODE_VALID

		fprintf(fout, "VALID CODE       : %s (case sensitive)\n", buf);
#endif

		fprintf(fout, "THIS MAIL SENT ON: %s\n", ctime(&code));

		while (fgets(genbuf, 255, fin) != NULL) {
			if (genbuf[0] == '.' && genbuf[1] == '\n')
				fputs(". \n", fout);
			else
				fputs(genbuf, fout);
		}
		fprintf(fout, ".\n");
		fclose(fin);
		fclose(fout);
	}
}
//#endif

void regmail_send(struct userec *trec, char* mail) {
	time_t code;
	FILE *fout, *dp, *mailp;
	char buf[RNDPASSLEN + 1];
	char mailuser[25], mailpass[25];
	sprintf(buf, "%s", (char *) genrandpwd((int) getpid()));
	sethomefile(genbuf, trec->userid, ".regpass");
	if ((dp = fopen(genbuf, "w")) == NULL)
		return;
	dp = fopen(genbuf, "w+");
	fprintf(dp, "%s\n", buf);
	fprintf(dp, "%s\n", mail);
	fclose(dp);

	code = time(0);
	//	sprintf(genbuf, "%s -f %s.bbs@%s %s ",
	//   SENDMAIL, trec->userid, BBSHOST, mail);
	mailp = fopen("/home/bbs/mailbox", "r");//make sure mailbox exists Danielfree05.12.29
	fscanf(mailp, "%s %s", mailuser, mailpass);
	fclose(mailp);
	sprintf(genbuf, "%s -au %s -ap %s -f %s.bbs@%s %s", SENDMAIL,
			mailuser, mailpass, trec->userid, BBSHOST, mail);
	fout = popen(genbuf, "w");
	if (fout != NULL) {
		fprintf(fout, "Reply-To: SYSOP.bbs@%s\n", BBSHOST);
		fprintf(fout, "From: SYSOP.bbs@%s\n", BBSHOST);
		fprintf(fout, "To: %s\n", mail);
		fprintf(fout, "Subject: %s@%s mail check.\n", trec->userid, BBSID);
		fprintf(fout, "X-Purpose: %s registration mail.\n", BBSNAME);
		fprintf(fout, "\n");
		fprintf(fout, "[ÖĞÎÄ]\n");
		fprintf(fout, "BBS Î»Ö·         : %s (%s)\n", BBSHOST, BBSIP);
		fprintf(fout, "Äú×¢²áµÄ BBS ID  : %s\n", trec->userid);
		fprintf(fout, "ÉêÇëÈÕÆÚ         : %s", ctime(&trec->firstlogin));
		fprintf(fout, "µÇÈëÀ´Ô´         : %s\n", fromhost);
		fprintf(fout, "ÈÏÖ¤Âë           : %s (Çë×¢Òâ´óĞ¡Ğ´)\n", buf);
		fprintf(fout, "ÈÏÖ¤ĞÅ·¢³öÈÕÆÚ   : %s\n", ctime(&code));

		fprintf(fout, "[English]\n");
		fprintf(fout, "BBS LOCATION     : %s (%s)\n", BBSHOST, BBSIP);
		fprintf(fout, "YOUR BBS USER ID : %s\n", trec->userid);
		fprintf(fout, "APPLICATION DATE : %s", ctime(&trec->firstlogin));
		fprintf(fout, "LOGIN HOST       : %s\n", fromhost);
		fprintf(fout, "YOUR NICK NAME   : %s\n", trec->username);
		fprintf(fout, "VALID CODE       : %s (case sensitive)\n", buf);
		fprintf(fout, "THIS MAIL SENT ON: %s\n", ctime(&code));

		fprintf(fout, ".\n");
		fclose(fout);
	}

}
void check_reg_mail() {
	struct userec *urec = &currentuser;
	char buf[192], code[STRLEN], email[STRLEN]="ÄúµÄÓÊÏä";
	FILE *fout;
	int i;
	sethomefile(buf, urec->userid, ".regpass");
	if (!dashf(buf)) {
		move(1, 0);
		prints("    ÇëÊäÈëÄúµÄ¸´µ©ÓÊÏä(username@fudan.edu.cn)\n");
		prints("    [1;32m±¾Õ¾²ÉÓÃ¸´µ©ÓÊÏä°ó¶¨ÈÏÖ¤£¬½«·¢ËÍÈÏÖ¤ÂëÖÁÄúµÄ¸´µ©ÓÊÏä[m");
		do {
			getdata(3, 0, "    E-Mail:> ", email, STRLEN-12, DOECHO, YEA);
			if (invalidaddr(email) ||(strstr(email, "@fudan.edu.cn")
					== NULL) || invalid_email(email) == 1) {
				prints("    ¶Ô²»Æğ, ¸ÃemailµØÖ·ÎŞĞ§, ÇëÖØĞÂÊäÈë \n");
				continue;
			} else
				break;
		} while (1);
		regmail_send(urec, email);
	}
	move(4, 0);
	clrtoeol();
	move(5, 0);
	prints(" [1;33m   ÈÏÖ¤ÂëÒÑ·¢ËÍµ½ %s £¬Çë²éÊÕ[m\n", email);

	getdata(7, 0, "    ÏÖÔÚÊäÈëÈÏÖ¤ÂëÃ´£¿[Y/n] ", buf, 2, DOECHO, YEA);
	if (buf[0] != 'n' && buf[0] != 'N') {
		move(9, 0);
		prints("ÇëÊäÈë×¢²áÈ·ÈÏĞÅÀï, \"ÈÏÖ¤Âë\"À´×öÎªÉí·İÈ·ÈÏ\n");
		prints("Ò»¹²ÊÇ %d ¸ö×Ö·û, ´óĞ¡Ğ´ÊÇÓĞ²î±ğµÄ, Çë×¢Òâ.\n", RNDPASSLEN);
		prints("Çë×¢Òâ, ÇëÊäÈë×îĞÂÒ»·âÈÏÖ¤ĞÅÖĞËù°üº¬µÄÂÒÊıÃÜÂë£¡\n");
		prints("\n[1;31mÌáÊ¾£º×¢²áÂëÊä´í 3´ÎºóÏµÍ³½«ÒªÇóÄúÖØÌîĞè°ó¶¨µÄÓÊÏä¡£[m\n");

		sethomefile(buf, currentuser.userid, ".regpass");
		if ((fout = fopen(buf, "r")) != NULL) {
			//ÊäÈÏÖ¤Âë
			fscanf(fout, "%s", code);
			fscanf(fout, "%s", email);
			fclose(fout);
			//3´Î»ú»á
			for (i = 0; i < 3; i++) {
				move(15, 0);
				prints("Äú»¹ÓĞ %d ´Î»ú»á\n", 3 - i);
				getdata(16, 0, "ÇëÊäÈëÈÏÖ¤Âë: ", genbuf, (RNDPASSLEN+1), DOECHO,
						YEA);

				if (strcmp(genbuf, "") != 0) {
					if (strcmp(genbuf, code) != 0)
						continue;
					else
						break;
				}
			}
		} else
			i = 3;

		unlink(buf);
		if (i == 3) {
			prints("ÈÏÖ¤ÂëÈÏÖ¤Ê§°Ü!ÇëÖØÌîÓÊÏä¡£\n");
			//add by eefree 06.8.16
			sethomefile(buf, currentuser.userid, ".regextra");
			if (dashf(buf))
				unlink(buf);
			//add end
			pressanykey();
		} else {
			set_safe_record();
			urec->userlevel |= PERM_BINDMAIL;
			strcpy(urec->email, email);
			urec->lastjustify = time(0);
			substitut_record(PASSFILE, urec, sizeof(struct userec),
					usernum);
			prints("ÈÏÖ¤ÂëÈÏÖ¤³É¹¦!\n");
			//add by eefree 06.8.10
			sethomefile(buf, currentuser.userid, ".regextra");
			if (dashf(buf)) {
				prints("ÎÒÃÇ½«ÔİÊ±±£ÁôÄúµÄÕı³£Ê¹ÓÃÈ¨ÏŞ,Èç¹ûºË¶ÔÄúÊäÈëµÄ¸öÈËĞÅÏ¢ÓĞÎó½«Í£Ö¹ÄúµÄ·¢ÎÄÈ¨ÏŞ,\n");
				prints("Òò´ËÇëÈ·±£ÄúÊäÈëµÄÊÇ¸öÈËÕæÊµĞÅÏ¢.\n");
			}
			//add end
			if (!HAS_PERM(PERM_REGISTER)) {
				prints("Çë¼ÌĞøÌîĞ´×¢²áµ¥¡£\n");
			}
			pressanykey();
		}
	} else {
	}
}

/*add by Ashinmarch*/
int isNumStr(char *buf) {
	if (*buf =='\0'|| !(*buf))
		return 0;
	int i;
	for (i = 0; i < strlen(buf); i++) {
		if (!(buf[i]>='0' && buf[i]<='9'))
			return 0;
	}
	return 1;
}
int isNumStrPlusX(char *buf) {
	if (*buf =='\0'|| !(*buf))
		return 0;
	int i;
	for (i = 0; i < strlen(buf); i++) {
		if (!(buf[i]>='0' && buf[i]<='9') && !(buf[i] == 'X'))
			return 0;
	}
	return 1;
}
void check_reg_extra() {
	struct schoolmate_info schmate;
	struct userec *urec = &currentuser;
	char buf[192], bufe[192];
	sethomefile(buf, currentuser.userid, ".regextra");

	if (!dashf(buf)) {
		do {
			memset(&schmate, 0, sizeof(schmate));
			strcpy(schmate.userid, currentuser.userid);
			move(1, 0);
			prints("ÇëÊäÈë¸öÈËĞÅÏ¢. Èç¹ûÊäÈë´íÎó,¿ÉÒÔÖØĞÂÊäÈë.\n");
			/*default value is 0*/
			do {
				getdata(2, 0, "ÊäÈëÒÔÇ°µÄÑ§ºÅ: ", schmate.school_num,
						SCHOOLNUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.school_num)); //Èç¹ûÓĞÊäÈë·ÇÊı×Ö,ÖØĞÂÊäÈë!ÏÂÍ¬
			do {
				getdata(4, 0, "ÊäÈëÓÊÏä(Íâ²¿ÓÊÏäÒà¿É): ", schmate.email, STRLEN,
						DOECHO, YEA);
			} while (invalidaddr(schmate.email));
			do {
				getdata(6, 0, "ÊäÈëÉí·İÖ¤ºÅÂë: ", schmate.identity_card_num,
						IDCARDLEN+1, DOECHO, YEA);
			} while (!isNumStrPlusX(schmate.identity_card_num)
					|| strlen(schmate.identity_card_num) !=IDCARDLEN);

			do {
				getdata(8, 0, "ÊäÈë±ÏÒµÖ¤Êé±àºÅ: ", schmate.diploma_num,
						DIPLOMANUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.diploma_num));

			do {
				getdata(10, 0, "ÊäÈëÊÖ»ú»ò¹Ì¶¨µç»°ºÅÂë: ", schmate.mobile_num,
						MOBILENUMLEN+1, DOECHO, YEA);
			} while (!isNumStr(schmate.mobile_num));

			strcpy(buf, "");
			getdata(11, 0, "ÒÔÉÏĞÅÏ¢ÊäÈëÕıÈ·²¢½øĞĞÓÊÏä°ó¶¨ÈÏÖ¤[Y/n]", buf, 2, DOECHO, YEA);
		} while (buf[0] =='n' || buf[0] == 'N');
		sprintf(buf, "%s, %s, %s, %s, %s\n", schmate.school_num,
				schmate.email, schmate.identity_card_num,
				schmate.diploma_num, schmate.mobile_num);
		sethomefile(bufe, currentuser.userid, ".regextra");
		file_append(bufe, buf);
		do_report(".SCHOOLMATE", buf);
		regmail_send(urec, schmate.email);
	}
	clear();
	check_reg_mail();
}

/*add end*/

void check_register_info() {
	struct userec *urec = &currentuser;
	FILE *fout;
	char buf[192], buf2[STRLEN];
#ifdef MAILCHECK

	char ans[4];
#ifdef CODE_VALID

	int i;
#endif
#endif

	if (!(urec->userlevel & PERM_LOGIN)) {
		urec->userlevel = 0;
		return;
	}
#ifdef NEWCOMERREPORT
	if (urec->numlogins == 1) {
		clear();
		sprintf(buf, "tmp/newcomer.%s", currentuser.userid);
		if ((fout = fopen(buf, "w")) != NULL) {
			fprintf(fout, "´ó¼ÒºÃ,\n\n");
			fprintf(fout, "ÎÒÊÇ %s (%s), À´×Ô %s\n",
					currentuser.userid, urec->username, fromhost);
			fprintf(fout, "½ñÌì%s³õÀ´´ËÕ¾±¨µ½, Çë´ó¼Ò¶à¶àÖ¸½Ì¡£\n",
					(urec->gender == 'M') ? "Ğ¡µÜ" : "Ğ¡Å®×Ó");
			move(2, 0);
			prints("·Ç³£»¶Ó­ %s ¹âÁÙ±¾Õ¾£¬Ï£ÍûÄúÄÜÔÚ±¾Õ¾ÕÒµ½ÊôÓÚ×Ô¼ºµÄÒ»Æ¬Ìì¿Õ£¡\n\n", currentuser.userid);
			prints("ÇëÄú×÷¸ö¼ò¶ÌµÄ¸öÈË¼ò½é, Ïò±¾Õ¾ÆäËûÊ¹ÓÃÕß´ò¸öÕĞºô\n");
			prints("(¼ò½é×î¶àÈıĞĞ, Ğ´Íê¿ÉÖ±½Ó°´ <Enter> ÌøÀë)....");
			getdata(6, 0, ":", buf2, 75, DOECHO, YEA);
			if (buf2[0] != '\0') {
				fprintf(fout, "\n\n×ÔÎÒ½éÉÜ:\n\n");
				fprintf(fout, "%s\n", buf2);
				getdata(7, 0, ":", buf2, 75, DOECHO, YEA);
				if (buf2[0] != '\0') {
					fprintf(fout, "%s\n", buf2);
					getdata(8, 0, ":", buf2, 75, DOECHO, YEA);
					if (buf2[0] != '\0') {
						fprintf(fout, "%s\n", buf2);
					}
				}
			}
			fclose(fout);
			sprintf(buf2, "ĞÂÊÖÉÏÂ·: %s", urec->username);
			Postfile(buf, "newcomers", buf2, 2);
			unlink(buf);
		}
		pressanykey();
	}
#endif
#ifdef PASSAFTERTHREEDAYS
	if (urec->lastlogin - urec->firstlogin < 3 * 86400) {
		if (!HAS_PERM(PERM_SYSOP)) {
			set_safe_record();
			urec->userlevel &= ~(PERM_DEFAULT);
			urec->userlevel |= PERM_LOGIN;
			substitut_record(PASSFILE, urec, sizeof(struct userec), usernum);
			ansimore("etc/newregister", YEA);
			return;
		}
	}
#endif
#ifndef FDQUAN
	//¼ì²éÓÊÏä
	while (!HAS_PERM(PERM_BINDMAIL)) {
		clear();
		if (HAS_PERM(PERM_REGISTER)) {
			while (askyn("ÊÇ·ñ°ó¶¨¸´µ©ÓÊÏä", NA, NA)== NA)
			//add  by eefree.06.7.20
			{
				if (askyn("ÊÇ·ñÌîĞ´Ğ£ÓÑĞÅÏ¢", NA, NA) == NA) {
					clear();
					continue;
				}
				check_reg_extra();
				return;
			}
			//add end.
		}
		check_reg_mail();
	}

#endif

	clear();
	if (HAS_PERM(PERM_REGISTER))
		return;
#ifndef AUTOGETPERM

	if (check_register_ok()) {
#endif
		set_safe_record();
		urec->lastjustify = time(0);
		urec->userlevel |= PERM_DEFAULT;
		substitut_record(PASSFILE, urec, sizeof(struct userec), usernum);
		return;
#ifndef AUTOGETPERM

	}
#endif

#ifdef MAILCHECK
#ifdef CODE_VALID
	sethomefile(buf, currentuser.userid, ".regpass");
	if (dashf(buf)) {
		move(13, 0);
		prints("ÄúÉĞÎ´Í¨¹ıÉí·İÈ·ÈÏ... \n");
		prints("ÄúÏÖÔÚ±ØĞëÊäÈë×¢²áÈ·ÈÏĞÅÀï, \"ÈÏÖ¤°µÂë\"À´×öÎªÉí·İÈ·ÈÏ\n");
		prints("Ò»¹²ÊÇ %d ¸ö×Ö·û, ´óĞ¡Ğ´ÊÇÓĞ²î±ğµÄ, Çë×¢Òâ.\n", RNDPASSLEN);
		prints("ÈôÏëÈ¡Ïû¿ÉÒÔÁ¬°´ÈıÏÂ [Enter] ¼ü.\n");
		prints("[1;33mÇë×¢Òâ, ÇëÊäÈë×îĞÂÒ»·âÈÏÖ¤ĞÅÖĞËù°üº¬µÄÂÒÊıÃÜÂë£¡[m\n");
		if ((fout = fopen(buf, "r")) != NULL) {
			fscanf(fout, "%s", buf2);
			fclose(fout);
			for (i = 0; i < 3; i++) {
				move(18, 0);
				prints("Äú»¹ÓĞ %d ´Î»ú»á\n", 3 - i);
				getdata(19,0,"ÇëÊäÈëÈÏÖ¤°µÂë: ",genbuf,(RNDPASSLEN+1),DOECHO,YEA);
				if (strcmp(genbuf, "") != 0) {
					if (strcmp(genbuf, buf2) != 0)
					continue;
					else
					break;
				}
			}
		} else
		i = 3;
		if (i == 3) {
			prints("°µÂëÈÏÖ¤Ê§°Ü! ÄúĞèÒªÌîĞ´×¢²áµ¥»ò½ÓÊÕÈ·ÈÏĞÅÒÔÈ·¶¨ÄúµÄÉí·İ\n");
			getdata(22,0,"ÇëÑ¡Ôñ£º1.Ìî×¢²áµ¥ 2.½ÓÊÕÈ·ÈÏĞÅ [1]:",ans,2,DOECHO,YEA);
			if(ans[0] == '2') {
				send_regmail(&currentuser);
				pressanykey();
			} else
			x_fillform();
		} else {
			set_safe_record();
			urec->userlevel |= PERM_DEFAULT;
			urec->lastjustify = time(0);
			strncpy(urec->reginfo, buf, 62);
			substitut_record(PASSFILE, urec,sizeof(struct userec), usernum);
			prints("¹§ºØÄú!! ÄúÒÑË³ÀûÍê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø,\n");
			prints("´ÓÏÖÔÚÆğÄú½«ÓµÓĞÒ»°ãÊ¹ÓÃÕßµÄÈ¨ÀûÓëÒåÎñ...\n");
			unlink(buf);
			mail_file("etc/smail", "SYSOP", "»¶Ó­¼ÓÈë±¾Õ¾ĞĞÁĞ");
			pressanykey();
		}
		return;
	}
#endif
	if ( (!strstr(urec->email, BBSHOST)) && (!invalidaddr(urec->email)) &&
			(!invalid_email(urec->email))) {
		move(13, 0);
		prints("ÄúµÄµç×ÓĞÅÏä ÉĞĞëÍ¨¹ı»ØĞÅÑéÖ¤...  \n");
		prints("    ±¾Õ¾½«ÂíÉÏ¼ÄÒ»·âÑéÖ¤ĞÅ¸øÄú,\n");
		prints("    ÄúÖ»Òª´Ó %s »ØĞÅ, ¾Í¿ÉÒÔ³ÉÎª±¾Õ¾ºÏ¸ñ¹«Ãñ.\n\n", urec->email);
		prints("    ³ÉÎª±¾Õ¾ºÏ¸ñ¹«Ãñ, ¾ÍÄÜÏíÓĞ¸ü¶àµÄÈ¨Òæà¸!\n");
		prints("    ÄúÒ²¿ÉÒÔÖ±½ÓÌîĞ´×¢²áµ¥£¬È»ºóµÈ´ıÕ¾³¤µÄÊÖ¹¤ÈÏÖ¤¡£\n");
		getdata(21,0,"ÇëÑ¡Ôñ£º1.Ìî×¢²áµ¥ 2.·¢È·ÈÏĞÅ [1]: ",ans,2,DOECHO,YEA);
		if(ans[0] == '2') {
			send_regmail(&currentuser);
			getdata(21,0,"È·ÈÏĞÅÒÑ¼Ä³ö, µÈÄú»ØĞÅÅ¶!! ",ans, 2, DOECHO, YEA);
			return;
		}
	}
#endif
	/* Following line modified by Amigo 2002.04.23. Fill form only when no new letter. */
	/*   x_fillform();*/
	if (!chkmail())
		x_fillform();
}

//deardrago 2000.09.27  over
