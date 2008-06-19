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
$Id: stuff.c 370 2007-05-14 11:58:08Z danielfree $
*/

#include "bbs.h"
#include <sys/param.h>
#include <sys/sem.h>

extern char *getenv();
extern char fromhost[];

int presskeyfor(char *msg,int x)
{
	extern int showansi;
	showansi = 1;
	//saveline(x,0);
	move(x, 0);
	clrtoeol();
	prints(msg);
	egetch();
	move(x, 0);
	clrtoeol();
	//saveline(x,1);
	return 0;
}
int pressanykey()
{
	presskeyfor("[m                                [5;1;33m°´ÈÎºÎ¼ü¼ÌÐø...[m", t_lines-1);
}

int
pressreturn()
{
	extern int showansi;
	char    buf[3];
	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, "                              [1;33mÇë°´ ¡ô[5;36mEnter[m[1;33m¡ô ¼ÌÐø\033[m", buf, 2, NOECHO, YEA);
	move(t_lines - 1, 0);
	clrtoeol();
	refresh();
	return 0;
}

int
msgmorebar(char *filename)
{
	extern int showansi;
	char    title[256];
	int     ch;
	time_t  now;
	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();

	prints("[0m[1;44;32mÑ¶Ï¢ä¯ÀÀÆ÷   ±£Áô <[1;37mr[32m>    Çå³ý <[1;37mc[1;32m>   ¼Ä»ØÐÅÏä<[1;37mm[1;32m>                                [m");
	move(t_lines - 1, 0);

	ch = morekey();
	if (ch == 'C') {
		if (askyn("È·¶¨ÒªÇå³ýÂð£¿", NA, YEA) == YEA) {
			unlink(filename);
		}
		return ch;
	} else if (ch == 'M') {
		if (askyn("È·¶¨Òª¼Ä»ØÂð£¿", NA, YEA) == YEA) {
			now = time(0);
			getdatestring(now,NA);
			sprintf(title, "[%s] ËùÓÐÑ¶Ï¢±¸·Ý", datestring);
			mail_file(filename, currentuser.userid, title);
			unlink(filename);
		}	
		return ch;
	} else if (ch == 'H') {
		show_help("help/msghelp");
	}
	clrtoeol();
	refresh();
	return ch;
}

int askyn(char str[STRLEN], int defa,int gobottom)
{
	int     x, y;
	char    realstr[100];
	char    ans[3];
	sprintf(realstr, "%s %s", str, 
            (defa) ? "(YES/no)? [Y]" : "(yes/NO)? [N]");
	if (gobottom)
		move(t_lines - 1, 0);
	getyx(&x, &y);
	clrtoeol();
	getdata(x, y, realstr, ans, 2, DOECHO, YEA);
	if (ans[0] != 'Y' && ans[0] != 'y' &&
		ans[0] != 'N' && ans[0] != 'n') {
		return defa;
	} else if (ans[0] == 'Y' || ans[0] == 'y')
		return 1;
	else if (ans[0] == 'N' || ans[0] == 'n')
		return 0;
}

void
printdash(mesg)
char   *mesg;
{
	char    buf[80], *ptr;
	int     len;
	memset(buf, '=', 79);
	buf[79] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > 76)
			len = 76;
		ptr = &buf[40 - len / 2];
		ptr[-1] = ' ';
		ptr[len] = ' ';
		strncpy(ptr, mesg, len);
	}
	prints("%s\n", buf);
}

/* 990807.edwardc fix beep sound in bbsd .. */

void
bell()
{
#ifndef BBSD
	char    sound[3], *ptr;
	ptr = sound;
	memset(ptr, Ctrl('G'), sizeof(sound));
	write(1, ptr, sizeof(sound));
#else
	static char sound[1] = {Ctrl('G')};
  
    send(0, sound, sizeof(sound), 0);
#endif
}

void
touchnew()
{
	sprintf(genbuf, "touch by: %d\n", time(0));
	file_append(FLUSH, genbuf);
}
/* rrr - Snagged from pbbs 1.8 */

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"

//ÓÃÓÚ´æ´¢BBS»·¾³±äÁ¿
char   *bbsenv[MAXENVS];
int     numbbsenvs = 0;

//ÈôdstÎªÄ¿Â¼,ÇÒ²¢·Ç.,..,×îºóÒ»¸ö×Ö·û²»Îª/,
//			½«ÆäÉ¾³ý,³É¹¦·µ»Ø	1
//					 ·ñÔò·µ»Ø	0
int	deltree(char   *dst)
{
	if (strstr(dst, "//") || strstr(dst, "..") || strchr(dst, ' '))
		return 0;	/* precaution */
	if (dst[strlen(dst) - 1] == '/')
		return 0;
	if (dashd(dst)) {
		sprintf(genbuf, "/bin/rm -rf %s", dst);
		system(genbuf);
		return 1;
	} else
		return 0;
}

//ÉèÖÃBBS»·¾³ env=val
/*
	Commented by Erebus 2004-11-04
	char * bbsenv[MAXENVS];
	if *env exists ,update its value to *val
	else add "*env=**val" to the end,numberbbsenvs++
*/
int bbssetenv(char *env, char *val)
{
	register int i, len;
	if (numbbsenvs == 0)
		bbsenv[0] = NULL;
	len = strlen(env);
	for (i = 0; bbsenv[i]; i++)
		if (!ci_strncmp(env, bbsenv[i], len))
			break;
	if (i >= MAXENVS)
		return -1;
	if (bbsenv[i])
		free(bbsenv[i]);
	else
		bbsenv[++numbbsenvs] = NULL;
	bbsenv[i] = malloc(strlen(env) + strlen(val) + 2);
	strcpy(bbsenv[i], env);
	strcat(bbsenv[i], "=");
	strcat(bbsenv[i], val);
	return 0;
}

int
do_exec(com, wd)
char   *com, *wd;
{
	char    path[MAXPATHLEN];
	char    pcom[MAXCOMSZ];
	char   *arglist[MAXARGS];
	char   *tz;
	register int i, len;
	register int argptr;
	int     status, pid, w;
	int     pmode;
	void    (*isig) (), (*qsig) ();

	strncpy(path, BINDIR, MAXPATHLEN);
	strncpy(pcom, com, MAXCOMSZ);
	len = Min(strlen(com) + 1, MAXCOMSZ);
	pmode = LOOKFIRST;
	for (i = 0, argptr = 0; i < len; i++) {
		if (pcom[i] == '\0')
			break;
		if (pmode == QUOTEMODE) {
			if (pcom[i] == '\001') {
				pmode = LOOKFIRST;
				pcom[i] = '\0';
				continue;
			}
			continue;
		}
		if (pcom[i] == '\001') {
			pmode = QUOTEMODE;
			arglist[argptr++] = &pcom[i + 1];
			if (argptr + 1 == MAXARGS)
				break;
			continue;
		}
		if (pmode == LOOKFIRST)
			if (pcom[i] != ' ') {
				arglist[argptr++] = &pcom[i];
				if (argptr + 1 == MAXARGS)
					break;
				pmode = LOOKLAST;
			} else
				continue;
		if (pcom[i] == ' ') {
			pmode = LOOKFIRST;
			pcom[i] = '\0';
		}
	}
	arglist[argptr] = NULL;
	if (argptr == 0)
		return -1;
	if (*arglist[0] == '/')
		strncpy(path, arglist[0], MAXPATHLEN);
	else
		strncat(path, arglist[0], MAXPATHLEN);
	reset_tty();
	alarm(0);
#ifdef IRIX
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif

#ifdef BBSD
		waitpid(pid, &status, 0);
#endif

		if (wd)
			if (chdir(wd)) {
				sprintf(genbuf, "Unable to chdir to '%s'\n", wd);
				report(genbuf);
				exit(-1);
			}

		bbssetenv("PATH", "/bin:.");
		bbssetenv("TERM", currentuser.termtype);
		bbssetenv("USER", currentuser.userid);
		bbssetenv("USERNAME", currentuser.username);

		if ((tz = getenv("TZ")) != NULL)
			bbssetenv("TZ", tz);
		if (numbbsenvs == 0)
			bbsenv[0] = NULL;
		//dup2(0,1);
		//dup2(0,2);
		execve(path, arglist, bbsenv);
		sprintf(genbuf, "EXECV FAILED... path = '%s'\n", path);
		report(genbuf);
		exit(-1);
	}
	isig = signal(SIGINT, SIG_IGN);
	qsig = signal(SIGQUIT, SIG_IGN);
#ifndef BBSD
	while ((w = wait(&status)) != pid && w != 1)
		 /* NULL STATEMENT */ ;
#endif		 
	signal(SIGINT, isig);
	signal(SIGQUIT, qsig);
	restore_tty();
#ifdef DOTIMEOUT
	alarm(IDLE_TIMEOUT);
#endif
	return ((w == -1) ? w : status);
}

char*	horoscope(char month,char day)
{
	static char   *name[12] = {
		"Ä¦ôÉ", "Ë®Æ¿", "Ë«Óã", "ÄµÑò", "½ðÅ£", "Ë«×Ó",
		"¾ÞÐ·", "Ê¨×Ó", "´¦Å®", "Ìì³Ó", "ÌìÐ«", "ÉäÊÖ"
	};
	switch (month) {
		case 1:
			if (day < 21)
				return (name[0]);
			else
				return (name[1]);
		case 2:
			if (day < 19)
				return (name[1]);
			else
				return (name[2]);
		case 3:
			if (day < 21)
				return (name[2]);
			else
				return (name[3]);
		case 4:
			if (day < 21)
				return (name[3]);
			else
				return (name[4]);
		case 5:
			if (day < 21)
				return (name[4]);
			else
				return (name[5]);
		case 6:
			if (day < 22)
				return (name[5]);
			else
				return (name[6]);
		case 7:
			if (day < 23)
				return (name[6]);
			else
				return (name[7]);
		case 8:
			if (day < 23)
				return (name[7]);
			else
				return (name[8]);
		case 9:
			if (day < 23)
				return (name[8]);
			else
				return (name[9]);
		case 10:
			if (day < 24)
				return (name[9]);
			else
				return (name[10]);
		case 11:
			if (day < 23)
				return (name[10]);
			else
				return (name[11]);
		case 12:
			if (day < 22)
				return (name[11]);
			else
				return (name[0]);
	}
	return ("²»Ïê");
}

sigjmp_buf bus_jump;
void sigbus(int signo)
{
    siglongjmp(bus_jump, 1);
};
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag, void **ret_ptr, size_t * size)
{
    struct stat st;

    if (fd < 0)
        return 0;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return 0;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return 0;
    }
    if (st.st_size <= 0) {
        close(fd);
        return 0;
    }
    *ret_ptr = mmap(NULL, st.st_size, prot, flag, fd, 0);
    if (*ret_ptr == NULL)
        return 0;
    /*
     * signal(SIGSEGV,sigbus);
     */
    *size = st.st_size;
    return 1;
}
//	½«ÎÄ¼þfilenameÓ³Éäµ½ÄÚ´æÖÐ,
//	Èôret_fdÎª¿Õ,½«ÆäÖ¸Ïò´ò¿ªfilenameµÄÃèÊö·û,²¢Ëø×¡ÎÄ¼þ,½«*sizeÖÃÎªÎÄ¼þ´óÐ¡
//	³É¹¦Ê±·µ»Ø1,·ñÔò0
int safe_mmapfile(	char *filename, 
					int openflag, 
					int prot, 
					int flag, 
					void **ret_ptr, 
					size_t * size, 
					int *ret_fd
				 )
{
    int fd;
    struct stat st;

    fd = open(filename, openflag, 0600);
    if (fd < 0)//Î´³É¹¦´ò¿ª 
        return 0;
    if (fstat(fd, &st) < 0) {	//Î´³É¹¦¼ì²âÎÄ¼þ×´Ì¬
        close(fd);
        return 0;
    }
    if (!S_ISREG(st.st_mode)) {	//·Ç³£¹æÎÄ¼þ,·ûºÅÎÄ¼þ¿´ÆäËùÖ¸ÏòµÄÎÄ¼þÊôÐÔ
        close(fd);
        return 0;
    }
    if (st.st_size <= 0) {	//ÎÄ¼þ´óÐ¡Îª0
        close(fd);
        return 0;
    }
    *ret_ptr = mmap(NULL, st.st_size, prot, flag, fd, 0);//Ó³ÉäÕû¸öÎÄ¼þµ½ÄÚ´æÖÐ
    if (!ret_fd){
        close(fd);
    }	else	{
        *ret_fd = fd;
        flock(fd, LOCK_EX);
    }
    if (*ret_ptr == NULL)
        return 0;
    *size = st.st_size;
    return 1;
}
//ÖÐÖ¹ÄÚ´æÓ³Éä,ÈôfdÓÐÐ§,½«Æä½âËø
void end_mmapfile(void *ptr, int size, int fd)
{
    munmap(ptr, size);
    /*
     * signal(SIGSEGV,SIG_IGN);
     */
    if (fd != -1){
    	flock(fd, LOCK_UN);
        close(fd);
    }
}

int sem(int key)
{
        int val=1;
        int semid;
        semid=semget(key,1,0);
        if(semid<0) {
                semid=semget(key,1,IPC_CREAT|0660);
                if (semid<0) {
                        //errlog("[0x%x] semget\n",key);
			return -1;
                }
                semctl(semid,0,SETVAL,val);
        }
        return semid;
}

void d_sem(int semid)
{
        int val=0;
        semctl(semid,0,IPC_RMID, val);
}

int p_nowait(int semid)
{
	struct sembuf sb={0,-1,IPC_NOWAIT};
	return semop(semid, &sb, 1);
}

void p(int semid)
{
        struct sembuf sb={0,-1,SEM_UNDO};
        semop(semid, &sb, 1);
}

void v(int semid)
{
        struct sembuf sb={0,1,0};
        semop(semid, &sb, 1);
}

/* Added by IAMFAT 2002-05-25 */
//	Ïû³ýÓÒ±ßµÄÐ¡ÓÚ0x20(¿Õ¸ñ)µÄ×Ö·û
void trimright(char *str)
{
	unsigned char *ustr=(unsigned char *)str;
	int i=strlen(str)-1;
	if(i<0)
		return;
	while(i>=0 && ustr[i]<=32)
		i--;
	str[i+1]='\0';
}
/* End */
//Ïû³ýtitleËùÖ¸ÏòµÄ×Ö·û´®Á½±ßµÄ¿Õ¸ñ×Ö·û
void trimboth(char *title)
{
	char *begin=title;
	int len=strlen(title);
	int end=len-1;
	
	while(*begin==' '){
		begin++;
		len--;
	}
	while(title[end]==' '){
		end--;
		len--;
	}
	if(end>=0 && len>0){
		if(begin!=title)
			memmove(title, begin, len);
		title[len]='\0';
	}else{
		title[0]='\0';
	}
}

//	Ïû³ýtitleËùÖ¸ÏòµÄ×Ö·û´®×ó±ßµÄ¿Õ¸ñ
void trimleft(char *title)
{
	char *begin=title;
	int len=strlen(title);
	
	while(*begin==' '){
		begin++;
		len--;
	}
	if(len>0){
		if(begin!=title){
			memmove(title, begin, len);
			title[len]='\0';
		}
	}else{
		title[0]='\0';
	}
}

//Added by IAMFAT 2002-05-27
// ³¬¹ýÒ»¶¨³¤¶ÈµÄ×Ö·û´®ÓàÏÂ²¿·ÖÒÔ...ÏÔÊ¾
void ellipsis(char *str,int len)
{
	register int l = 0;
	register int hz=0, ohz=0;
	unsigned char *ptr = (unsigned char *) str;
	if(len<3){
  		str[len]=0;
		return;
	}
	while(*ptr && l<len){
  		if(!hz && *ptr>=0xa1 && *ptr<=0xfe){
			hz=1;
		}else{
			if(hz && l==len-3)
				ohz=1;
			hz=0;
		}
		l++;
	  	ptr++;
	}
	if(l<len){
  		memset(ptr, ' ', len-l);
		str[len]=0;
	}else if(*ptr){
  		ptr-=2;
	  	strcpy(ptr, "..");
		if(!ohz){
			*(ptr-1)='.';
		}
	}
}
//End IAMFAT

int safe_strcpy(char *dst, const char *src)
{
	register int mode=0, i=0;
    while(*src) {
    	if(mode==0) {
        	if(*src==27) {
            	mode=1;
            } else {
	        	*(dst++)=*(src++);
				i++;
				continue;
            }
        } else {
        	if(!strchr(";[0123456789", *src))
				mode=0;
        }
        src++;
	}
    *dst=0;
	return i;
}
