#include "bbs.h"
#include <sys/param.h>
#include <sys/sem.h>

extern char fromhost[];

int presskeyfor(char *msg, int x) {
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
int pressanykey() {
	presskeyfor(
			"[m                                [5;1;33m°´ÈÎºÎ¼ü¼ÌÐø...[m",
			t_lines-1);
}

int pressreturn() {
	extern int showansi;
	char buf[3];
	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(
			t_lines - 1,
			0,
			"                              [1;33mÇë°´ ¡ô[5;36mEnter[m[1;33m¡ô ¼ÌÐø\033[m",
			buf, 2, NOECHO, YEA);
	move(t_lines - 1, 0);
	clrtoeol();
	refresh();
	return 0;
}

int msgmorebar(char *filename) {
	extern int showansi;
	char title[256];
	int ch;
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
			sprintf(title, "[%s] ËùÓÐÑ¶Ï¢±¸·Ý", getdatestring(time(NULL), DATE_ZH));
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

int askyn(char str[STRLEN], int defa, int gobottom) {
	int x, y;
	char realstr[100];
	char ans[3];
	sprintf(realstr, "%s %s", str, (defa) ? "(YES/no)? [Y]"
			: "(yes/NO)? [N]");
	if (gobottom)
		move(t_lines - 1, 0);
	getyx(&x, &y);
	clrtoeol();
	getdata(x, y, realstr, ans, 2, DOECHO, YEA);
	if (ans[0] != 'Y' && ans[0] != 'y' && ans[0] != 'N' && ans[0] != 'n') {
		return defa;
	} else if (ans[0] == 'Y' || ans[0] == 'y')
		return 1;
	else if (ans[0] == 'N' || ans[0] == 'n')
		return 0;
}

void printdash(const char *mesg)
{
	char buf[80], *ptr;
	int len;
	memset(buf, '=', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > sizeof(buf) - 4)
		len = sizeof(buf) - 4;
		ptr = buf + (sizeof(buf) - 1 - len) / 2 - 1;
		*ptr++ = ' ';
		memcpy(ptr, mesg, len);
		ptr[len] = ' ';
	}
	prints("%s\n", buf);
}

/* 990807.edwardc fix beep sound in bbsd .. */

void bell(void)
{
	static char sound[1] = {Ctrl('G')};

	send(0, sound, sizeof(sound), 0);
}

void touchnew() {
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
char *bbsenv[MAXENVS];
int numbbsenvs = 0;

//ÈôdstÎªÄ¿Â¼,ÇÒ²¢·Ç.,..,×îºóÒ»¸ö×Ö·û²»Îª/,
//			½«ÆäÉ¾³ý,³É¹¦·µ»Ø	1
//					 ·ñÔò·µ»Ø	0
int deltree(char *dst) {
	if (strstr(dst, "//") || strstr(dst, "..") || strchr(dst, ' '))
		return 0; /* precaution */
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
int bbssetenv(char *env, char *val) {
	register int i, len;
	if (numbbsenvs == 0)
		bbsenv[0] = NULL;
	len = strlen(env);
	for (i = 0; bbsenv[i]; i++)
		if (!strncasecmp(env, bbsenv[i], len))
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

int do_exec(char *com, char *wd)
{
	char path[MAXPATHLEN];
	char pcom[MAXCOMSZ];
	char *arglist[MAXARGS];
	char *tz;
	register int i, len;
	register int argptr;
	int status, pid, w;
	int pmode;
	void (*isig) (), (*qsig) ();

	strlcpy(path, BINDIR, MAXPATHLEN);
	strlcpy(pcom, com, MAXCOMSZ);
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
	strlcpy(path, arglist[0], MAXPATHLEN);
	else
	strncat(path, arglist[0], MAXPATHLEN);
	reset_tty();
	alarm(0);
#ifdef IRIX
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif
			waitpid(pid, &status, 0);

			if (wd)
			if (chdir(wd)) {
				sprintf(genbuf, "Unable to chdir to '%s'\n", wd);
				report(genbuf, currentuser.userid);
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
			report(genbuf, currentuser.userid);
			exit(-1);
		}
		isig = signal(SIGINT, SIG_IGN);
		qsig = signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, isig);
		signal(SIGQUIT, qsig);
		restore_tty();
#ifdef DOTIMEOUT
		alarm(IDLE_TIMEOUT);
#endif
		return ((w == -1) ? w : status);
	}

int sem(int key) {
	int val=1;
	int semid;
	semid=semget(key, 1, 0);
	if (semid<0) {
		semid=semget(key, 1, IPC_CREAT|0660);
		if (semid<0) {
			//errlog("[0x%x] semget\n",key);
			return -1;
		}
		semctl(semid, 0, SETVAL, val);
	}
	return semid;
}

void d_sem(int semid) {
	int val=0;
	semctl(semid, 0, IPC_RMID, val);
}

int p_nowait(int semid) {
	struct sembuf sb= { 0, -1, IPC_NOWAIT };
	return semop(semid, &sb, 1);
}

void p(int semid) {
	struct sembuf sb= { 0, -1, SEM_UNDO };
	semop(semid, &sb, 1);
}

void v(int semid) {
	struct sembuf sb= { 0, 1, 0 };
	semop(semid, &sb, 1);
}
