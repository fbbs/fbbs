#include "../telnet/include/bbs.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "stdarg.h"
#include "crypt.inc"

#define FIRST_PAGE	"/bbsmain.html"
#define CSS_FILE 	"/css/bbs%d.css"
#define CHARSET		"gb2312"
#define UCACHE_SHMKEY	get_shmkey("UCACHE_SHMKEY")
#define UTMP_SHMKEY	get_shmkey("UTMP_SHMKEY")
#define BCACHE_SHMKEY	get_shmkey("BCACHE_SHMKEY")
#define ACACHE_SHMKEY get_shmkey("ACACHE_SHMKEY")

#define TLINES 18
#define SQUID
//added by iamfat 2002.08.29 for ¸¡¶¯±íÏÖÖµ
#define MAX_PERF (1000)
#define MAX_LUCKY (20)
void SpecialID(const char* uid, char* host);
int flock(int fd, int op) {
        return lockf(fd, op, 0);
}

/* ADD BY ROLY  FOR mail retriction*/
#ifdef FDQUAN
	#define MAX_LARGEMAIL_UPLIMIT   300
	#define MAX_BOARDSMAIL_UPLIMIT	120
	#define MAX_MAIL_UPLIMIT	60
#else
	#define MAX_LARGEMAIL_UPLIMIT   20000
	#define MAX_BOARDSMAIL_UPLIMIT  20000
	#define MAX_MAIL_UPLIMIT        20000
#endif
/* ADD END */

#ifdef FDQUAN
	#define SECNUM 9  //9 for FDQUAN
#else
	#define SECNUM 12 //11 for fudan
#endif

char seccode[SECNUM][6]={
#ifdef FDQUAN
	"ab","cd","ij","kl","mn","op","qr","st","uv"
#else
//    "0nk","Ccy","IE","Nn","amB","emk","rs","xcd","fyzd","ij","Bbm"
"0oOhH", "1pP", "2qQ", "3rRhH", "4sS", "5tT", "6uU", "7vV", "8wW", "9xX", "ayY", "bzZ"
#endif
	//"a","bcd","ef",/*"ghi","jkl","mn","opq",*/"rst"
	//"a","bcd","ef","rst"
};


char secname[SECNUM][2][20]={
#ifdef FDQUAN
		"BBS ÏµÍ³", "[Õ¾ÄÚ]",
		"¸´µ©´óÑ§", "[±¾Ð£]",
		"¹â»ª¹«Ë¾", "[ÇÚÖú]",	
		"ÉçÍÅ×éÖ¯", "[ÍÅÌå]",
		"¾ºÈü»î¶¯", "[ÁÙÊ±]",
		"Ì¸ÌìÁÄµØ", "[¸ÐÐÔ]",
		"ÓÎÏ·ÐÝÏÐ", "[ÐÝÏÐ]",
		"¸öÈË·ç²É", "[¸öÈË]",
		"»ý·ÖÂÛÌ³", "[ÍÅÌå]"
#else
        "BBS ÏµÍ³", "[Õ¾ÄÚ]",
        "¸´µ©´óÑ§", "[±¾Ð£]",
        "ÔºÏµ·ç²É", "[Ð£Ô°]",
        "µçÄÔ¼¼Êõ", "[µçÄÔ]",
        "ÐÝÏÐÓéÀÖ", "[ÐÝÏÐ]",
        "ÎÄÑ§ÒÕÊõ", "[ÎÄÒÕ]",
        "ÌåÓý½¡Éí", "[ÔË¶¯]",
        "¸ÐÐÔ¿Õ¼ä", "[¸ÐÐÔ]",
        "ÐÂÎÅÐÅÏ¢", "[ÐÂÎÅ]",
        "Ñ§¿ÆÑ§Êõ", "[Ñ§¿Æ]",
	"Ó°ÊÓÒôÀÖ", "[Ó°Òô]",
	"½»Ò××¨Çø", "[½»Ò×]"
#endif
};
/*
char secname[SECNUM][2][20]={
	"BBS ÏµÍ³", "[Õ¾ÄÚ]",
	"¸´µ©´óÑ§", "[±¾Ð£]",
	"ÉçÍÅ×éÖ¯", "[ÍÅÌå]",	
        "¸öÐÔ¿Õ¼ä", "[¸öÈË]"
};
*/
struct boardheader *getbcache();
int junkboard(char *board) {
        // Çë×Ô¶¨Òåjunkboard.
	//if(file_has_word("etc/junkboards", board)) return 1;
		struct boardheader *bp;
		bp = getbcache(board);
		if (bp->flag & BOARD_JUNK_FLAG )
			return 1;
	// added by roly 02.03.23
        return 0;
}

struct post_log {
	char	author[IDLEN+1];
	char	board[18];
	char	title[66];
	time_t	date;
	int	number;
};

int loginok=0;

struct userec currentuser;
struct user_info *u_info;
struct UTMPFILE *shm_utmp;
struct BCACHE *shm_bcache;
struct UCACHE *shm_ucache;
struct boardheader *bcache=NULL;
#ifdef CERTIFYMODE
	struct KEYWORDS_SHM *keywords_shm;
#endif
//ANONCACHE *anonshm;

char fromhost[256];

struct userec *getuser();
char *strcasestr();
char *ModeType();
char *anno_path_of();
void sort_friend(int left, int right);

static int shm_lock(char *lockname)
{
    int lockfd;

    lockfd = open(lockname, O_RDWR | O_CREAT, 0600);
    if (lockfd < 0) {
        //bbslog("3system", "CACHE:lock ucache:%s", strerror(errno));
        return -1;
    }
    flock(lockfd, LOCK_EX);
    return lockfd;
}

static void shm_unlock(int fd)
{
    flock(fd, LOCK_UN);
    close(fd);
}



/* added by roly for fix the bug of solaris "strncasecmp" 
   strncasecmp of solaris has bug  when string include 8-bit character
*/

/* Function: 
 * 	compare two n-string case-sensitively
 * Write: roly			Date:2002.04.13
 */
int
strncasecmp2(char *s1, char *s2,int n)
  {
      register int c1, c2,l=0;
  
      while (*s1 && *s2 && l<n) {
      	c1 = tolower(*s1);
      	c2 = tolower(*s2);
      	if (c1 != c2)
          return (c1 - c2);
      	s1++;
      	s2++;
      	l++;
      }
      if (l==n)
      	return (int) (0);
      else
	return -1;
  }


/* add end */

/* ADDED BY ROLY FOR CHECK MAIL BOX */

/* Function: 
 * 	get the number of records in a file
 * Write: roly			Date:2002.01.10
 */
long
get_num_records(filename,size)
char    *filename ;
int     size;
{
    struct stat st ;

    if(stat(filename,&st) == -1)
        return 0 ;
    return (st.st_size/size) ;
}

//added by iamfat 2002.08.10
/*
void check_anonymous(char* owner)
{
   if(owner[0]&0x80)strcpy(owner,"Anonymous");
}*/
//added end

/* Function:
 * 	set the .DIR file name into buf
 * Write: roly			Date:2002.01.10	
 */
char *
setmdir(buf,userid)
char *buf,*userid;
{
	sprintf(buf,"mail/%c/%s/.DIR",toupper(userid[0]),userid);
	return buf;
}

/* Function:
 *	whether the number of mail is under the limitation
 * Write: roly			Date:2002.01.08
 */
int
mailnum_under_limit(char *userid)
{
    int mail_count;
    char buf[256];
    sprintf(buf,"mail/%c/%s/%s", toupper(userid[0])
        ,userid,DOT_DIR);
    mail_count=get_num_records(buf,sizeof(struct fileheader));
    
    if ((HAS_PERM(PERM_OBOARDS) || HAS_PERM(PERM_LARGEMAIL)) && mail_count<MAX_LARGEMAIL_UPLIMIT*2) return 1;
    if (HAS_PERM(PERM_BOARDS) && mail_count<MAX_BOARDSMAIL_UPLIMIT) return 1;
    if (mail_count<MAX_MAIL_UPLIMIT*2) return 1;
    return 0;
}

/* Function:
 *	whether the size of mail is under the limitation
 * Write: roly			Date:2002.01.08
 */
int mailsize_under_limit(char *userid)
{
    struct userec *userrec1;
    userrec1 = getuser(userid);
    
    if(2*getmailboxsize(userrec1->userlevel)>getmailsize(userid))
		return 1;
     return 0;
}

/* Function:
 *	get the size limitation of one's mailbox
 * Write: roly			Date:2002.01.08
 */
int getmailboxsize(unsigned int userlevel)
{
     if(userlevel&(PERM_SYSOPS)) return MAILBOX_SIZE_SYSOP;
    if(userlevel&(PERM_LARGEMAIL)) return MAILBOX_SIZE_LARGE;
	if(userlevel&(PERM_XEMPT)) return MAILBOX_SIZE_LARGE; 
    if(userlevel&(PERM_BOARDS)) return MAILBOX_SIZE_BM;
    if(userlevel&(PERM_REGISTER)) return MAILBOX_SIZE_NORMAL;
    return 5;
}

/* Function:
 *	get the actual size of one's mail box
 * Write: roly			Date:2002.01.08
 */
int getmailsize(char *userid)
{
   struct fileheader fcache;
   struct stat DIRst, SIZEst, st;
   char sizefile[50],dirfile[256],mailfile[256];
   FILE  *fp;
   int mailsize= -1, fd, ssize = sizeof(struct fileheader);
   
   setmdir(dirfile,userid);
   sprintf(sizefile,"tmp/%s.mailsize",userid);
   if(stat(dirfile, &DIRst)==-1||DIRst.st_size==0) mailsize = 0;
   else if(stat(sizefile, &SIZEst)!=-1 && SIZEst.st_size!=0
	    && SIZEst.st_ctime >= DIRst.st_ctime){
      fp = fopen(sizefile,"r");
      if(fp){
         fscanf(fp,"%d",&mailsize);
	 fclose(fp);
      } 
   }
   if( mailsize != -1 ) return mailsize;
  
   mailsize=0;
   if(stat(dirfile, &st)!=-1)mailsize+=(st.st_size/1024+1); 
   fd = open(dirfile, O_RDONLY);
   if(fd != -1) {
      while(read(fd, &fcache, ssize) == ssize){
         sprintf(mailfile,"mail/%c/%s/%s",
	    toupper(userid[0]),userid,fcache.filename);
         if(stat(mailfile,&st)!=-1) {
	    mailsize += (st.st_size/1024+1);
	 }   
      }
      close(fd);
   }
   fp = fopen(sizefile,"w+");
   if(fp){
      fprintf(fp,"%d",mailsize);
      fclose(fp);
   }
   return mailsize;
}

/* ADDED END */

/* added by roly 02.03.28 */
char * entity_char(char *s) {	
	char *buf=calloc(strlen(s)+1024, sizeof(char));
	int i=0, j=0;
	while(s[j])
	{
		if (i>=1024) break;
		if (s[j]==39)  //'
		{j++;buf[i++]='%';buf[i++]='2';buf[i++]='6';
		buf[i++]='%';buf[i++]='2';buf[i++]='3';buf[i++]='3';buf[i++]='9';
		buf[i++]='%';buf[i++]='3';buf[i++]='B';continue;}
		if (s[j]==34)  //"
		{j++;buf[i++]='%';buf[i++]='2';buf[i++]='6';
		buf[i++]='%';buf[i++]='2';buf[i++]='3';buf[i++]='3';buf[i++]='4';
		buf[i++]='%';buf[i++]='3';buf[i++]='B';continue;}
		if (s[j]==38)  //&
		{j++;buf[i++]='%';buf[i++]='2';buf[i++]='6';buf[i++]='a';buf[i++]='m';
		buf[i++]='p';buf[i++]='%';buf[i++]='3';buf[i++]='B';continue;}		
		if (s[j]==43)  //+
		{j++;buf[i++]='%';buf[i++]='2';buf[i++]='B';continue;}
		if (s[j]==35)  //#
		{j++;buf[i++]='%';buf[i++]='2';buf[i++]='3';continue;}
		if (s[j]==37)  //%
		{j++;buf[i++]='%';buf[i++]='2';buf[i++]='5';continue;}

		
		buf[i++]=s[j++];
		
	}
	buf[i]=0;
	return buf;
}
/* add end */

int file_has_word(char *file, char *word) {
	FILE *fp;
	char buf[256], buf2[256];
	fp=fopen(file, "r");
	if(fp==0) return 0;
	while(1) {
		bzero(buf, 256);
		if(fgets(buf, 255, fp)==0) break;
		sscanf(buf, "%s", buf2);
		if(!strcasecmp(buf2, word)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int f_append(const char *file, char *buf) {
	FILE *fp;
	fp=fopen(file, "a");
	if(fp==0) return;
	fprintf(fp, "%s", buf);
	fclose(fp);
}

struct stat *f_stat(char *file) {
	static struct stat buf;
	bzero(&buf, sizeof(buf));
	if(stat(file, &buf)==-1) bzero(&buf, sizeof(buf));
	return &buf;
}

#define file_size(x) f_stat(x)->st_size
#define file_time(x) f_stat(x)->st_mtime
#define file_rtime(x) f_stat(x)->st_atime
#define file_exist(x) (file_time(x)!=0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)

int get_record(void *buf, int size, int num, char *file) {
	FILE *fp;
	if(size<1 || size>4096) return 0;
	if(num<0 || num>1000000) return 0;
	bzero(buf, size);
	fp=fopen(file, "r");
	if(fp==0) return 0;
	fseek(fp, num*size, SEEK_SET);
	if(fread(buf, size, 1, fp)<=0) {
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

int put_record(void *buf, int size, int num, char *file) {
	FILE *fp;
	if(size<1 || size>4096) return 0;
	if(num<0 || num>1000000) return 0;
	fp=fopen(file, "r+");
	if(fp==0) return 0;
	fseek(fp, num*size, SEEK_SET);
	fwrite(buf, size, 1, fp);
	fclose(fp);
	return 1;
}

int append_record(void *buf, int size, char *file)
{
	FILE *fp;
	fp=fopen(file, "ab");
	if(fp==0)return 0;
	fwrite(buf,size,1,fp);
	fclose(fp);
	return 1;
}

int del_record(char *file, int size, int num) {
	FILE *fp;
	int total, i;
	char buf[4096];
	if(size<1 || size>4096) return 0;
	total=file_size(file)/size;
	if(total<1 || total>1000000) return 0;
	fp=fopen(file, "r+");
	if(fp==0) return 0;
	flock(fileno(fp), LOCK_EX);
	for(i=num+1; i<=total-1; i++) {
		fseek(fp, i*size, SEEK_SET);
		if(fread(buf, size, 1, fp)<=0) break;
		fseek(fp, (i-1)*size, SEEK_SET);
		fwrite(buf, size, 1, fp);
	}
	ftruncate(fileno(fp), (total-1)*size);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 1;
}

//added by iamfat 2002.08.01
char *cn_Ctime(time_t t)
{
	static char s[80];
	struct tm *tm;
	char weeknum[7][3]={"Ìì","Ò»","¶þ","Èý","ËÄ","Îå","Áù"};
	tm = localtime(&t);
	sprintf(s,"%4dÄê%02dÔÂ%02dÈÕ%02d:%02d:%02d ÐÇÆÚ%2s",
		tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
		tm->tm_hour,tm->tm_min,tm->tm_sec,
		weeknum[tm->tm_wday]);
	return s;
}

char *Ctime(time_t t)
{
	static char s[80];
	sprintf(s, "%24.24s", ctime(&t));
	return s;
}

char *noansi(char *s) {
	static char buf[1024];
	int i=0, mode=0;
	while(s[0] && i<1023) {
		if(mode==0) { 
			if(s[0]==27) {
				mode=1;
			} else {
				buf[i]=s[0];
				i++;
			}
		} else {
			if(!strchr(";[0123456789", s[0])) mode=0;
		}
		s++;
	}
	buf[i]=0;
	return buf;
}

char *nohtml(char *s) {
/*
	char *buf=calloc(strlen(s)+1, 1);
	int i=0, mode=0;
	while(s[0] && i<1023) {
		if(mode==0) {
			if(s[0]=='<') {
				mode=1;
			} else {
				buf[i]=s[0];
				i++;
			}
		} else {
			if(s[0]=='>') mode=0;
		}
		s++;
	}
	buf[i]=0;
	return buf;
*/

/* Added by Amigo 2002.06.19. For article title display error. */
	int j, k,len;
	char *buf;
	int i=0, mode=0;

	for(j=0, k=0; s[j]; j++) 
	{
		if(s[j]=='>' || s[j]=='<') k+=3;
		else if(s[j]==' ') k+=5;
		else if(s[j]=='&') k+=4;
	}
	len=strlen(s)+k+1;
	buf=calloc(len, 1);
/* Add end. */
/* Following 2 lines commented by Amigo 2002.06.19. For article title display error. */
//	char *buf=calloc(strlen(s)+9, 1);
//	int i=0, mode=0;
	while(s[0] && i<len) 
	{
		if(s[0]=='<') {
            		buf[i]='&';i++;buf[i]='l';i++;buf[i]='t';i++;
            		buf[i]=';';i++; 
         	} 
		else if (s[0]=='>') {
			buf[i]='&';i++;buf[i]='g';i++;buf[i]='t';
                	i++;buf[i]=';';i++;
         	}
		
		//else if (s[0]==' ') {
		//	buf[i]='&';i++;buf[i]='n';i++;buf[i]='b';i++;buf[i]='s';i++;
		//	buf[i]='p';i++;buf[i]=';';i++;
		//}
		else if (s[0]=='&') {
			buf[i]='&';i++;buf[i]='a';i++;buf[i]='m';i++;buf[i]='p';i++;
			buf[i]=';';i++;
		}else{ 
			buf[i]=s[0]; i++;
     		}
    		s++;
         }
	buf[i]=0;
	return buf;      
}



char *strright(char *s, int len) {
	int l=strlen(s);
	if(len<=0) return "";
	if(len>=l) return s;
	return s+(l-len);
}

char *strcasestr(char *s1, char *s2) {
	int l;
	l=strlen(s2);
	while(s1[0]) {
		if(!strncasecmp2(s1, s2, l)) return s1;
		//modified from strncasecmp to strncasecmp2 by roly 02.04.13
		//to fix the bug of solaris when string include 8-bit character
		s1++;
	}
	return 0;
}

int strsncpy(char *s1, char *s2, int n) {
	int l=strlen(s2);
	if(n<0) return;
	if(n>l+1) n=l+1;
	strncpy(s1, s2, n-1);
	s1[n-1]=0;
}

char *const_ltrim(char *s)
{
	char *ptr=s;
	char *s2=s;
	if(s[0]==0) return s;
	while(s2[0] && strchr(" \t\r\n", s2[0])) s2++;
	return s2;
}

char *ltrim(char *s) {
	char *ptr=s;
	char *s2=s;
	if(s[0]==0) return s;
	while(s2[0] && strchr(" \t\r\n", s2[0])) s2++;
	/* add by money 2003.11.05 for trim it but don't change ptr s */
	if (s2 != s)
	while(!strchr(" \t\r\n", s2[0]))
		*ptr++ = *s2++;
	/* add end */
	return s;
}

char *rtrim(char *s) {
	static char t[1024], *t2;
	if(s[0]==0) return s;
	strsncpy(t, s, 1024);
	t2=t+strlen(s)-1;
	while(t2>t && strchr(" \t\r\n", t2[0]) && t2>=t) t2--;
	if(t2==t && strchr(" \t\r\n", t2[0]))t2[0]=0;
	else t2[1]=0;
	return t;
}

#define trim(s) ltrim(rtrim(s))
#define const_trim(s) const_ltrim(rtrim(s))

char *get_new_shm(int key, int size) {
        int id;
        id=shmget(key, size, IPC_CREAT | IPC_EXCL | 0640);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

char *get_old_shm(int key, int size) {
        int id;
        id=shmget(key, size, 0);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

char *get_shm(int key, int size) {
        int id;
        id=shmget(key, size, IPC_CREAT | 0640);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

#define setcookie(a, b)	printf("<script>document.cookie='%s=%s'</script>\n", a, b)

#define setcookie(a, b)	printf("<script>document.cookie='%s=%s'</script>\n", a, b)
#define redirect(x)	printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t)	printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()	printf("Content-type: text/html; charset=%s\n\n", CHARSET)

char *getsenv(char *s) {
        char *t=getenv(s);
        if(t) return t;
        return "";
}

int http_quit() {
	printf("\n</html>\n");
	exit(0);
}

int http_fatal(char *fmt, ...) {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, 1023, fmt, ap);
        va_end(ap);
	buf[1023]=0;
 	printf("´íÎó! %s! <br><br>\n", buf);
	printf("<a href=javascript:history.go(-1)>¿ìËÙ·µ»Ø</a>");
	http_quit();
}

int strnncpy(char *s, int *l, char *s2) {
	strncpy(s+(*l), s2, strlen(s2));
	(*l)+=strlen(s2);
}

int hsprintf(char *s, char *fmt, ...) {
	char buf[1024], ansibuf[80], buf2[80];
	char *tmp;
	int c, bold, m, i, l, len;
	//int flag=0;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	s[0]=0;
	l=strlen(buf);
	len=0;
	bold=0;
	for(i=0; i<l; i++) {
		c=buf[i];
		if(c=='&') {
			strnncpy(s, &len, "&amp;");
		} else if(c=='<') {
			strnncpy(s, &len, "&lt;");
		} else if(c=='>') {
			strnncpy(s, &len, "&gt;");
		} else if(c==27) {
			if(buf[i+1]!='[') continue;
			for(m=i+2; m<l && m<i+24; m++)
				if(strchr("0123456789;", buf[m])==0) break;
			strsncpy(ansibuf, &buf[i+2], m-(i+2)+1);
			i=m;
			if(buf[i]!='m') continue;
			//strnncpy(s, &len, "</font>");
			if(strlen(ansibuf)==0) {
				bold=0;
				strnncpy(s, &len, "</font>");
			}
			tmp=strtok(ansibuf, ";");
			while(tmp) {
				c=atoi(tmp);
				tmp=strtok(0, ";");
				if(c==0) {
					strnncpy(s, &len, "</font>");
					bold=0;
				}
				if((c>=30 && c<=37)/* || (c>=40 && c<=47)*/) {
					if(bold==1) sprintf(buf2, "</font><font class=d%d>", c);
					if(bold==0) sprintf(buf2, "</font><font class=c%d>", c);
					strnncpy(s, &len, buf2);
				}
			}
		} else {
			s[len]=c;
			len++;
		}
	}
	s[len]=0;
}


int hprintf(char *fmt, ...) {
	char buf[8096], buf2[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf2, 1023, fmt, ap);
	va_end(ap);
	hsprintf(buf, "%s", buf2);
	printf("%s", buf);
}

int hhprintf(char *fmt, ...) {
	char buf0[1024], buf[1024], *s, *getparm();
	int len=0;
	int my_link_mode;
	int msg=0;
	int mailto=0;
	int board=0;
	//int upload=0;
	int special=0;
	int tab=0;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023]=0;
	s=buf;
	my_link_mode=atoi(getparm("my_link_mode"));
	if(my_link_mode==1)  return hprintf("%s", buf);
	if(!strcasestr(s, "http://") 
	   && !strcasestr(s, "ftp://") 
	   && !strcasestr(s, "msg://")
	   && !strcasestr(s, "mailto://")
	   && !strcasestr(s, "board://")
	   //&& !strcasestr(s, "upload://")
	   )
		return hprintf("%s", buf);
	while(s[0]) {
		if(!strncasecmp(s, "http://", 7) || !strncasecmp(s, "ftp://", 6)) special=1;
		else if(!strncasecmp(s, "msg://", 6))	{msg=1;special=1;}
		else if(!strncasecmp(s, "mailto://", 9)){mailto=1;special=1;}
		else if(!strncasecmp(s, "board://", 8))	{board=1;special=1;}
	//	else if(!strncasecmp(s, "upload://", 9)){upload=1;special=1;}
		if(special)
		{
			char *tmp,*ltmp;
			if(len>0) {
				buf0[len]=0;
				hprintf("%s", buf0);
				len=0;
			}
			tmp=strtok(s, "\'\" \r\t)(,;\n");
			if(tmp==0) break;
			ltmp = strdup(tmp);
			int i = 0;
			for (; tmp[i]; i++)
				ltmp[i] = tolower(tmp[i]) ;
			
			
			if(1) {
				if(strstr(ltmp, ".gif") || strstr(ltmp, ".jpg") 
				|| strstr(ltmp, ".bmp") || strstr(ltmp, ".png") 
				|| strstr(ltmp, ".jpeg") ) {
					//added for relative path when viewing pictures. 2003.12.25 iamfat
					char host[80];
					int hostlen;
					sprintf(host, "http://%s/" , BBSHOST);
					hostlen = strlen(host);
					if(!strncmp(tmp, host, hostlen)){
						tmp+=hostlen -1;
					}
                    
                    //polygon<<
                    //
                    //if(strstr(ltmp, ".gif"))
                    if(true)//Disable the thumbnail
                    {
                        printf("<IMG border=0 SRC='%s' onload='javascript:if(this.width>(screen.width*0.85))this.width=(screen.width*0.85)'>", nohtml(tmp));
                    }else
                    {
                        printf("<span><IMG SRC='%s' onload=\'javascript:var width0=this.width; var height0= this.height;this.width=(screen.width*0.15); document.getElementById(\"%s\").value=width0; var txt = document.createTextNode(\"&nbsp;[\"+width0+\"x\"+height0+\"]\");if(width0!=0)this.parentNode.appendChild(txt);\' ", nohtml(tmp),nohtml(tmp));
                        printf("onMouseOver=\'javascript:var width1=document.getElementById(\"%s\").value;if(width1!=0)this.width=(width1);if(this.width>(screen.width*0.81))this.width=(screen.width*0.81);\' ",nohtml(tmp));
                        printf("onClick='javascript:this.width=(screen.width*0.15);' ");
                        printf("border=0></span>");
                        printf("<input  id='%s' style='visibility:hidden'>",nohtml(tmp));
                    }
                    
                    //polygon>>
                    tmp=strtok(0, "");
					if(tmp==0) return;
					return hhprintf("%s", tmp);
				}
			}
			if(msg) printf("<a target=_self href='bbssendmsg?destid=%s'>%s</a>",nohtml(tmp+6),nohtml(tmp+6));
			else if(mailto) printf("<a target=_self href='bbspstmail?userid=%s'>%s</a>",nohtml(tmp+9),nohtml(tmp+9));
			else if(board) printf("<a target=_self href='bbsdoc?board=%s'>%s</a>",nohtml(tmp+8),nohtml(tmp+8));
			/*
			else if(upload) 
			{
				char * my_p;
				my_p=nohtml(tmp+9);
				if(strstr(my_p, "%2E%67%69%66") || strstr(my_p, "%2E%6A%70%67") || strstr(my_p, "%2E%6A%70%65%67")
				|| strstr(my_p, "%2C%70%6E%67") || strstr(my_p, "%2E%62%6D%70")	
				|| strstr(my_p, ".jpg") || strstr(my_p, ".gif") || strstr(my_p, ".png")
				|| strstr(my_p, ".bmp") )
				{
					printf("<br><img border=0 src='/upload/%s'><br>",nohtml(tmp+9));
				}else{
					printf("<a target=_self href='/upload/%s'>%s</a>",nohtml(tmp+9),strchr(my_p,'/')+1);
				}
			}*/
			else printf("<a target=_blank href='%s'>%s</a> ", nohtml(tmp), nohtml(tmp));
			tmp=strtok(0, "");
			if(tmp==0) return printf("\n");
			return hhprintf("%s", tmp);
		} 
		else {
			buf0[len]=s[0];
			if(len<1000) len++;
			s++;
		}
	}
}

char parm_name[256][80], *parm_val[256];
int parm_num=0;

int parm_add(char *name, char *val) {
	int len=strlen(val);
	if(parm_num>=255) http_fatal("too many parms.");
	parm_val[parm_num]=calloc(len+1, 1);
	if(parm_val[parm_num]==0) http_fatal("memory overflow2 %d %d", len, parm_num);
	strsncpy(parm_name[parm_num], name, 78);
	strsncpy(parm_val[parm_num], val, len+1);
	parm_num++;
}
char *getparm();

int http_init() {
	char *buf, buf2[1024], *t2, *t3;
	int n, my_style;
#ifndef XMLFILE
	printf("Content-type: text/html; charset=%s\n\n\n", CHARSET);
	printf("<html>\n");
	printf("<meta http-equiv='Content-Type' content='text/html; charset=%s'>\n", CHARSET);
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
#else	
	printf("Connection: close\n");
	printf("Content-type: text/xml; charset=%s\n\n", CHARSET);
#endif
	n=atoi(getsenv("CONTENT_LENGTH"));
	if(n>5000000) n=5000000;
	buf=calloc(n+1, 1);
	if(buf==0) http_fatal("memory overflow");
	fread(buf, 1, n, stdin);
	buf[n]=0;
	t2=strtok(buf, "&");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			__unhcode(t3);
			parm_add(const_trim(t2), t3);
		}
		t2=strtok(0, "&");
	}
	strsncpy(buf2, getsenv("QUERY_STRING"), 1024);
	t2=strtok(buf2, "&");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			__unhcode(t3);
			parm_add(const_trim(t2), t3);
		}
		t2=strtok(0, "&");
	}
	strsncpy(buf2, getsenv("HTTP_COOKIE"), 1024);
	t2=strtok(buf2, ";");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			parm_add(const_trim(t2), t3);
		}
		t2=strtok(0, ";");
	}
#ifdef SQUID
	char *fromtmp;
	fromtmp = strrchr(getsenv("HTTP_X_FORWARDED_FOR"), ',');
	if (fromtmp == NULL)
		strsncpy(fromhost, getsenv("HTTP_X_FORWARDED_FOR"), 32);
	else
	{
		while ((*fromtmp < '0')&&(*fromtmp != '\0'))
			fromtmp++;
		strsncpy(fromhost, fromtmp, 32);
	}
#else
	strsncpy(fromhost, getsenv("REMOTE_ADDR"), 32);
#endif
	my_style=atoi(getparm("my_style"));
#ifndef XMLFILE
#ifndef MY_CSS
        printf("<link rel=stylesheet type=text/css href='/css/bbs%d.css'>\n", my_style);
#else
        printf("<link rel=stylesheet type=text/css href='%s'>\n", MY_CSS);
#endif
#endif
	//printf("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"/stm31.js\"></script>");
	return my_style;
}

int __to16(char c) {
	if(c>='a'&&c<='f') return c-'a'+10;
	if(c>='A'&&c<='F') return c-'A'+10;
	if(c>='0'&&c<='9') return c-'0';
	return 0;
}

int __unhcode(char *s) {
	int m, n;
	for(m=0, n=0; s[m]!=0; m++, n++) {
		if(s[m]=='+') {
			s[n]=' ';
			continue;
		}
		if(s[m]=='%') {
			s[n]=__to16(s[m+1])*16+__to16(s[m+2]);
			m+=2;
			continue;
		}
		s[n]=s[m];
	}
	s[n]=0;
}

char *getparm(char *var) {
	int n;
	for(n=0; n<parm_num; n++) 
		if(!strcasecmp(parm_name[n], var)) return parm_val[n];
	return "";
}

int get_shmkey(char *s) {
        int n=0;
        while(shmkeys[n].key!=0) {
                if(!strcasecmp(shmkeys[n].key, s)) return shmkeys[n].value;
                n++;
        }
        return 0;
}

int shm_init() {
	int boardfd;
	shm_utmp= (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY, sizeof(struct UTMPFILE));
	shm_bcache= (struct BCACHE *) get_old_shm(BCACHE_SHMKEY, sizeof(struct BCACHE));
	//init mmap bcache
	if (bcache == NULL) {
        		if ((boardfd = open(BOARDS, O_RDWR | O_CREAT, 0644)) == -1) {
            			http_fatal("Can't open " BOARDS "file");
        		}
        		bcache = (struct boardheader *) mmap(NULL, MAXBOARD * sizeof(struct boardheader), PROT_READ, MAP_SHARED, boardfd, 0);
        		if (bcache == (struct boardheader *) -1) {
            			http_fatal("Can't map " BOARDS "file ");
           			 close(boardfd);
        		}	
		close(boardfd);
	}
	shm_ucache= (struct UCACHE *) get_old_shm(UCACHE_SHMKEY, sizeof(struct UCACHE));
#ifdef CERTIFYMODE
	keywords_shm=(struct KEYWORDS_SHM *)get_old_shm(KEYWORDS_SHMKEY,sizeof(struct KEYWORDS_SHM));
#endif
	//anonshm=(ANONCACHE*)get_old_shm(ACACHE_SHMKEY, sizeof(ANONCACHE));
	
	if(shm_utmp==0) http_fatal("shm_utmp error");
	if(shm_bcache==0) http_fatal("shm_bcache error");
	if(shm_ucache==0) http_fatal("shm_ucache error");
	//if(anonshm==0)http_fatal("anonshm error");
}

int user_init(struct userec *x, struct user_info **y) {
	struct userec *x2;
	char id[20], num[20];
	int i, uid, key;
	strsncpy(id, getparm("utmpuserid"), 13);
	strsncpy(num, getparm("utmpnum"), 12);
	key=atoi(getparm("utmpkey"));
	i=atoi(num);
	if(i<=0 || i>MAXACTIVE) return 0;
	(*y)=&(shm_utmp->uinfo[i-1]);
	if(strncmp((*y)->from, fromhost, 16)) return 0;
	if((*y)->utmpkey != key) return 0;
	if((*y)->active==0) return 0;
	if((*y)->userid[0]==0) return 0;
	if((*y)->mode!=10001) return 0;
	if(!strcasecmp((*y)->userid, "new") || !strcasecmp((*y)->userid, "guest")) return 0;
	x2=getuser((*y)->userid);
	if(x2==0) return 0;
	if(strcmp(x2->userid, id)) return 0;
	memcpy(x, x2, sizeof(*x));
	(*y)->idle_time=time(0);
	return 1;
}

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig) {
	FILE *fp, *fp2;
	char buf3[256], dir[256];
	struct fileheader header;
	int t, i;
	/* added by roly check mail size */
	if (!mailnum_under_limit(currentuser.userid))
			http_fatal("ÄúµÄÐÅ¼þÈÝÁ¿³¬±ê£¬ÎÞ·¨·¢ÐÅ");
	if (!mailsize_under_limit(userid))
			http_fatal("ÊÕ¼þÈËÐÅ¼þÈÝÁ¿³¬±ê£¬ÎÞ·¨ÊÕÐÅ");
	
	/* add end */
	if(strstr(userid, "@")) 
		return post_imail(userid, title, file, id, nickname, ip, sig);
	bzero(&header, sizeof(header));
	strcpy(header.owner, id);
	for(i=0; i<100; i++) {
		t=time(0)+i;
		sprintf(buf3, "mail/%c/%s/M.%d.A", toupper(userid[0]), userid, i+time(0));
		if(!file_exist(buf3)) break;
	}
	if(i>=99) return -1;
	sprintf(header.filename, "M.%d.A", t);
	strsncpy(header.title, title, 60);
	fp=fopen(buf3, "w");
	if(fp==0) return -2;
	fp2=fopen(file, "r");
	fprintf(fp, "¼ÄÐÅÈË: %s (%s)\n", id, nickname);
	fprintf(fp, "±ê  Ìâ: %s\n", title);
//	fprintf(fp, "·¢ÐÅÕ¾: %s (%s)\n", BBSNAME, Ctime(time(0)));
//modified by iamfat 2002.08.01
	fprintf(fp, "·¢ÐÅÕ¾: %s (%s)\n", BBSNAME, cn_Ctime(time(0)));
	fprintf(fp, "À´  Ô´: %s\n\n", ip);
	if(fp2) {
		while(1) {
			if(fgets(buf3, 256, fp2)<=0) break;
			fprintf2(fp, buf3);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "[m[1;%2dm¡ù À´Ô´:¡¤%s %s¡¤HTTP [FROM: %-.20s][m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
	fclose(fp);
        sprintf(dir, "mail/%c/%s/.DIR", toupper(userid[0]), userid);
        fp=fopen(dir, "a");
	if(fp==0) return -1;
        fwrite(&header, sizeof(header), 1, fp);
        fclose(fp);
}

int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig) {
        FILE *fp1, *fp2;
        char buf[256];
	if(strstr(userid, ";") || strstr(userid, "`")) http_fatal("´íÎóµÄÊÕÐÅÈËµØÖ·");
	sprintf(buf, "sendmail -f %s.bbs@%s '%s'", id, BBSHOST, userid);
        fp2=popen(buf, "w");
        fp1=fopen(file, "r");
        if(fp1==0 || fp2==0) return -1;
        fprintf(fp2, "From: %s.bbs@%s\n", id, BBSHOST);
        fprintf(fp2, "To: %s\n", userid);
        fprintf(fp2, "Subject: %s\n\n", title);
        while(1) {
		if(fgets(buf, 255, fp1)==0) break;
		if(buf[0]=='.' && buf[1]=='\n') continue;
                fprintf(fp2, "%s", buf);
        }
	fprintf(fp2, "\n--\n");
 	sig_append(fp2, id, sig);
// 	fprintf(fp2, "\n\n[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s][m\n", 31+rand()%7, BBSNAME, "http://bbs.fudan.edu.cn", ip);
	fprintf(fp2, "[m[1;%2dm¡ù À´Ô´:¡¤%s %s¡¤HTTP [FROM: %-.20s][m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
        fprintf(fp2, ".\n");
        fclose(fp1);
        pclose(fp2);
}

#ifdef CERTIFYMODE 
void Certify(char *board, struct fileheader *fh)
{
	char buf[4096];
	char *p=buf;
	int	 i=4096,j;
	int len;
	FILE *fp;
	sprintf(buf, "boards/%s/%s", board,fh->filename);
	if(fp=fopen(buf, "r"))
	{
		fh->accessed[1]&=~FILE_UNCERTIFIED;
		while(1)
		{
			p=buf;
			while(i>1 && fgets(p,i,fp))
			{
				len=strlen(p);
				while(p[len-1]=='\n'||p[len-1]=='\r')p[--len]='\0';
				i-=len;
				p+=len;
			}
			for(j=keywords_shm->number-1;j>=0;j--)
			{
				if(strcasestr(buf,keywords_shm->word[j]))
				{
					fh->accessed[1]|=FILE_UNCERTIFIED;
					break;
				}
			}
			if(i>1)break;
		}
		fclose(fp);
	}
}
#endif

int getlastpost(char *board, int *lastpost, int *total)
{       
    struct fileheader fh;
    struct stat st;
    char filename[STRLEN * 2];
    int fd, atotal, offset;
    
    sprintf(filename, "boards/%s/" DOT_DIR, board);
    if ((fd = open(filename, O_RDONLY)) < 0)
        return 0;
    fstat(fd, &st);
    atotal = st.st_size / sizeof(fh);
    if (atotal <= 0) {
        *lastpost = 0;
        *total = 0; 
        close(fd);
        return 0;
    }   
    *total = atotal;
    offset = (int) ((char *) &(fh.filename[0]) - (char *) &(fh));
    lseek(fd, (off_t) (offset + (atotal - 1) * sizeof(fh)), SEEK_SET);
    if (read(fd, filename, STRLEN) > 2) { 
        *lastpost = atoi(filename+2);
    }else{
        *lastpost = 0;
    }   
    close(fd);
    return 0;
}   

int updatelastpost(char *board)
{        
    int pos;

    pos = getbnum(board);
    if (pos > 0) {
        getlastpost(board, &(shm_bcache->bstatus[pos - 1].lastpost), &(shm_bcache->bstatus[pos - 1].total));
        return 0;
    } else
        return -1;
}
//from kbs 2.0
static void bcache_setreadonly(int readonly)
{
    int boardfd;
    void *oldptr = bcache;
    munmap((void *)bcache, MAXBOARD * sizeof(struct boardheader));
    if ((boardfd = open(BOARDS, O_RDWR | O_CREAT, 0644)) == -1) {
        http_fatal("Can't open " BOARDS "file %s");
    }
    if (readonly)
        bcache = (struct boardheader *) mmap(oldptr, MAXBOARD * sizeof(struct boardheader), PROT_READ, MAP_SHARED, boardfd, 0);
    else
        bcache = (struct boardheader *) mmap(oldptr, MAXBOARD * sizeof(struct boardheader), PROT_READ | PROT_WRITE, MAP_SHARED, boardfd, 0);
    close(boardfd);
}
static int bcache_lock()
{
    int lockfd;

    lockfd = creat("bcache.lock", 0600);
    if (lockfd < 0) {
        http_fatal("CACHE:lock bcache");
        return -1;
    }
    bcache_setreadonly(0);
    flock(lockfd, LOCK_EX);
    return lockfd;
}
static void bcache_unlock(int fd)
{
    flock(fd, LOCK_UN);
    bcache_setreadonly(1);
    close(fd);
}
//add end
int get_nextid(char* boardname){
  return get_nextid_bid(getbnum(boardname));
}

int get_nextid_bid(int bid){
  int ret;
  ret = bid;
  if (ret > 0){
  	int fd;
	fd = bcache_lock();
    	shm_bcache->bstatus[ret-1].nowid++;
    	ret=shm_bcache->bstatus[ret-1].nowid;
    	bcache_unlock(fd);
  }
  return ret;
}

int post_article(char *board, char *title, char *file, char *id, char *nickname, char *ip, int o_id, int o_gid, int sig) {
	FILE *fp, *fp2;
	char buf3[1024];
	struct fileheader header;
	struct boardheader *brd;
	char board2[30];
	int t, i;
	bzero(&header, sizeof(header));
	strcpy(header.owner, id);

/* add by roly 02.05.29 */
        brd=getbcache(board);
	if(brd==0) http_fatal("´íÎóµÄÌÖÂÛÇø");
	strcpy(board2, brd->filename);	

/* add end */

	for(i=0; i<100; i++) {
		t=time(0)+i;
		sprintf(buf3, "boards/%s/M.%d.A", board2, t);
		if(!file_exist(buf3)) break;
	}
	if(i>=99) return -1;
	sprintf(header.filename, "M.%d.A", t);
	header.id = get_nextid(board2);
	if (strncmp(title, "Re: ", 4) == 0 && !(o_id == -1 && o_gid == -1)){
		strsncpy(header.title, title, 68);
		header.reid = o_id;
		header.gid = o_gid;
	}else{
		strsncpy(header.title, title, 50);
		header.reid = header.id;
		header.gid = header.id;
	}
	fp=fopen(buf3, "w");
	fp2=fopen(file, "r");
	fprintf(fp, "·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n±ê  Ìâ: %s\n·¢ÐÅÕ¾: %s (%s)\n\n",
//		id, nickname, board2, title, BBSNAME, Ctime(time(0)));
//modified by iamfat 2002.08.01
		id, nickname, board2, header.title, BBSNAME, cn_Ctime(time(0)));
	if(fp2!=0) {
		while(1) {
			/* modified by roly */
 			//if(fgets(buf3, 10000, fp2)<=0) break;
			if(fgets(buf3, 1000, fp2)<=0) break;
			/* modified end */
			fprintf2(fp, buf3);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
//	fprintf(fp, "\n[1;%dm¡ù À´Ô´:£®%s %s [FROM: %.20s][m\n", 31+rand()%7, BBSNAME, "http://bbs.fudan.edu.cn", ip);
	fprintf(fp, "[m[1;%2dm¡ù À´Ô´:¡¤%s %s¡¤HTTP [FROM: %-.20s][m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
	fclose(fp);
#ifdef CERTIFYMODE 
    Certify(board2, &header);
#endif
	sprintf(buf3, "boards/%s/.DIR", board2);
	fp=fopen(buf3, "a");
	fwrite(&header, sizeof(header), 1, fp);
	fclose(fp);
    updatelastpost(board);
	return t;
}

/* add by money 2003.11.2 for trim title */
void check_title(char *title)
{
	char *ptr;
	
	ptr = title;
	while(*ptr==' ')
		ptr++;
	if (ptr != title)
		strcpy(title, ptr);
	ptr = &title[strlen(title)-1];
	while(*ptr==' ')
		ptr--;
	ptr[1]='\0';
	return;
}
/* add end */
int sig_append(FILE *fp, char *id, int sig) {
	FILE *fp2;
	char path[256];
	char buf[100][256];
	int i, total;
	struct userec *x;
	if(sig<0 || sig>10) return;
	x=getuser(id);
	if(x==0) return;
	sprintf(path, "home/%c/%s/signatures", toupper(id[0]), id);
	fp2=fopen(path, "r");
	if(fp2==0) return;
	for(total=0; total<100; total++) //total<255 -> 100 money modfied 03.10.21
		if(fgets(buf[total], 255, fp2)==0) break;
	fclose(fp2);
	for(i=sig*6; i<sig*6+6; i++) {
		if(i>=total) break;
		fprintf(fp, "%s", buf[i]);
	}
	fprintf(fp,"\n");
}

char* anno_path_of(char *board) {
	FILE *fp;
	static char buf[256], buf1[80], buf2[80];
	fp=fopen("0Announce/.Search", "r");
	if(fp==0) return "";
	while(1) {
		if(fscanf(fp, "%s %s", buf1, buf2)<=0) break;
		buf1[strlen(buf1)-1]=0;
		if(!strcasecmp(buf1, board)) {
			sprintf(buf, "/%s", buf2);
			fclose(fp);
			return buf;
		}
	}
	fclose(fp);
	return "";
}

int has_BM_perm(struct userec *user, char *board) {
	struct boardheader *x;
	char buf[256], *bm;
	x=getbcache(board);
	if(x==0) return 0;
	if (x->flag & BOARD_CLUB_FLAG){
		if(user_perm(user, PERM_OCLUB)) return 1;
	} else {
		if(user_perm(user, PERM_BLEVELS)) return 1;
	}
	if(!user_perm(user, PERM_BOARDS)) return 0;
	strcpy(buf, x->BM);
	bm=strtok(buf, ",: ;&()\n");
	while(bm) {
		if(!strcasecmp(bm, user->userid)) return 1;
		bm=strtok(0, ",: ;&()\n");
	}
	return 0;
}

int has_read_perm(struct userec *user, char *board) {  
	struct boardheader *x;	/* °æÃæ²»´æÔÚ·µ»Ø0, pºÍz°æÃæ·µ»Ø1, ÓÐÈ¨ÏÞ°æÃæ·µ»Ø1. */
	if(board[0]<=32) return 0;
	x=getbcache(board);
	if(x==0) return 0; 
	if(x->level==0) return 1;
	if(x->flag & (BOARD_POST_FLAG | BOARD_NOZAP_FLAG)) return 1;
	if(!user_perm(user, PERM_LOGIN)) return 0;
	//if(user_perm(user, PERM_SPECIAL8)) return 1;	//commented by stephen 2003.06.02 
	//return user_perm(user, x->level);
    
    if (currentuser.userlevel & x->level) return 1;
    return 0;
}

int has_post_perm(struct userec *user, char *board) {
	char buf3[256];
	struct boardheader *x;
	if(!has_read_perm(user, board)) return 0;
	x=getbcache(board);
	if(x==0) return 0;
	if(!loginok) return 0;
	if(user_perm(user, PERM_SYSOPS)) return 1;
	sprintf(buf3, "boards/%s/deny_users", x->filename);
	if(file_has_word(buf3, user->userid)) return 0;
	//if(!strcasecmp(board, "sysop")) return 1;
        //if(user_perm(user, PERM_SPECIAL8)) return 0; //commented by stephen 2003.05.29
	if(!user_perm(user, PERM_LOGIN)) return 0;
	if(!user_perm(user, PERM_POST)) return 0;
	/*
	if (x->level & PERM_NOZAP ) return 0;
	if (!x->level) return 1;
	if (!(x->level & ~PERM_POSTMASK)) return 1;
	*/
	return user_perm(user, x->level);
}

int getbnum(char *board) {
	int i;
	if(board[0]==0) return 0;
	for(i=0; i<shm_bcache->number; i++)
		if (!strcasecmp(board, bcache[i].filename)) 
			return i+1;
	return 0;
}

struct boardheader *getbcache(char *board) {
	int i;
	if(board[0]==0) return 0;
	for(i=0; i<MAXBOARD; i++)
		if (!strcasecmp(board, bcache[i].filename)) 
			return &bcache[i];
	return 0;
}

int count_mails(char *id, int *total, int *unread) {
        struct fileheader x1;
	char buf[256];
        int n;
        FILE *fp;
	*total=0;
	*unread=0;
        if(getuser(id)==0) return 0;
        sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(id[0]), id);
        fp=fopen(buf, "r");
        if(fp==0) return;
        while(fread(&x1, sizeof(x1), 1, fp)>0) {
                (*total)++;
                if(!(x1.accessed[0] & FILE_READ)) (*unread)++;
        }
        fclose(fp);
}

int findnextutmp(char *id, int from) {
	int i;
	if(from<0) from=0;
	for(i=from; i<MAXACTIVE; i++) 
		if(shm_utmp->uinfo[i].active)
			if(!strcasecmp(shm_utmp->uinfo[i].userid, id)) return i;
	return -1;
}

int sethomefile(char *buf, char *id, char *file) {
	sprintf(buf, "home/%c/%s/%s", toupper(id[0]), id, file);
}

int send_msg(char *myuserid, int mypid, char *touserid, int topid, char msg[256]) {
	char msgbuf[256], buf3[256];
	int i;
	for(i=0; i<strlen(msg); i++)
		if((0<msg[i] && msg[i]<=27 )|| msg[i]==-1) msg[i]=32;
	if(mypid<=0) return -1;
	sprintf(msgbuf, "[0;1;44;36m%-12.12s[33m([36m%-5.5s[33m):[37m%-54.54s[31m(^Z»Ø)[m[%05dm\n",
 		myuserid, Ctime(time(0)) + 11, msg, mypid);
	sethomefile(buf3, touserid, "msgfile");
	f_append(buf3, msgbuf);
	sethomefile(buf3, touserid, "msgfile.me");
	f_append(buf3, msgbuf);


/* added by roly 02.05.10 */
	sethomefile(buf3, myuserid, "msgfile.me"); 
    sprintf(msgbuf, "[1;32;40mTo [1;33;40m%-12.12s[m (%-5.5s):%-55.55s\n", 
                touserid, Ctime(time(0)) + 11, msg); 
    f_append(buf3, msgbuf); 
/* add end */

	if(topid<=0) return -1;
	kill(topid, SIGTTOU);
	kill(topid, SIGUSR2);
	return 0;
}

char *horoscope(int month, int day) {
        int date=month*100+day;
        if(month<1 || month>12 || day<1 || day>31) return "²»Ïê";
        if(date<121 || date>=1222) return "Ä¦ôÉ×ù";
        if(date<219) return "Ë®Æ¿×ù";
        if(date<321) return "Ë«Óã×ù";
        if(date<421) return "ÄµÑò×ù";
        if(date<521) return "½ðÅ£×ù";
        if(date<622) return "Ë«×Ó×ù";
        if(date<723) return "¾ÞÐ·×ù";
        if(date<823) return "Ê¨×Ó×ù";
        if(date<923) return "´¦Å®×ù";
        if(date<1024) return "Ìì³Ó×ù";
        if(date<1123) return "ÌìÐ«×ù";
        if(date<1222) return "ÉäÊÖ×ù";
}

char *ModeType(int mode) {
 	switch (mode) {
 	case IDLE: 	return "";
 	case NEW:	return "ÐÂÕ¾ÓÑ×¢²á";
 	case LOGIN:	return "½øÈë±¾Õ¾";
 	case DIGEST: 	return "ä¯ÀÀ¾«»ªÇø";
 	case MMENU: 	return "Ö÷Ñ¡µ¥";
 	case ADMIN: 	return "¹ÜÀíÕßÑ¡µ¥";
 	case SELECT: 	return "Ñ¡ÔñÌÖÂÛÇø";
 	case READBRD: 	return "ÀÀ±éÌìÏÂ";
 	case READNEW: 	return "ÀÀÐÂÎÄÕÂ";
 	case READING: 	return "Æ·Î¶ÎÄÕÂ";
 	case POSTING: 	return "·¢±íÎÄÕÂ";
 	case MAIL: 	return "´¦ÀíÐÅ¼ã";
 	case SMAIL: 	return "¼ÄÓïÐÅ¸ë";
 	case RMAIL: 	return "ÔÄÀÀÐÅ¼ã";
 	case TMENU: 	return "ÁÄÌìÑ¡µ¥";
 	case LUSERS: 	return "»·¹ËËÄ·½";
 	case FRIEND: 	return "Ñ°ÕÒºÃÓÑ";
        case MONITOR:   return "Ì½ÊÓÃñÇé";
        case QUERY:     return "²éÑ¯ÍøÓÑ";
        case TALK:      return "ÈµÇÅÏ¸Óï";
        case PAGE:      return "°ÍÏ¿Ô³Ìä";
        case CHAT1:     return "ÑàÔ²Ò¹»°";
        case CHAT2:     return "ÑàÔ°Ò¹»°";
        case CHAT3:     return "Chat3";
        case CHAT4:     return "Chat4";
        case LAUSERS:   return "Ì½ÊÓÍøÓÑ";
        case XMENU:     return "ÏµÍ³×ÊÑ¶";
        case VOTING:    return "Í¶Æ±";
        case EDITWELC:  return "±à¼­Welc";
        case EDITUFILE: return "±à¼­¸öÈËµµ";
        case EDITSFILE: return "¶¯ÊÖ¶¯½Å";
        case ZAP:       return "¶©ÔÄÌÖÂÛÇø";
        case LOCKSCREEN:return "ÆÁÄ»Ëø¶¨";
        case NOTEPAD:   return "ÁôÑÔ°å";
        case GMENU:     return "¹¤¾ßÏä";
        case MSG:       return "ËÍÑ¶Ï¢";
        case USERDEF:   return "×Ô¶©²ÎÊý";
        case EDIT:      return "ÐÞ¸ÄÎÄÕÂ";
        case OFFLINE:   return "×ÔÉ±ÖÐ..";
        case EDITANN:   return "±àÐÞ¾«»ª";
        case LOOKMSGS:  return "²ì¿´Ñ¶Ï¢";
        case WFRIEND:   return "Ñ°ÈËÃû²á";
        case WNOTEPAD:  return "Óû×ß»¹Áô";
        case BBSNET:    return "BBSNET..";
        case M_BLACKJACK:return "¡ïºÚ¼×¿Ë¡ï";
	case M_XAXB:return "¡ï²ÂÊý×Ö¡ï";
	case M_DICE:return "¡ïÎ÷°ËÀ­¡ï";
	case M_GP:return "½ðÆË¿ËËó¹þ";
	case M_NINE:return "ÌìµØ¾Å¾Å";
	case WINMINE:return "¼üÅÌÉ¨À×";
	case M_BINGO:return "±ö¹û±ö¹û";
	case FIVE:return "¾öÕ½Îå×ÓÆå";
	case MARKET:	return "½»Ò×ÊÐ³¡";
	case PAGE_FIVE:return "ÑûÇëÏÂÆå";
	case CHICK:return "µç×ÓÐ¡¼¦";
	case MARY:return "³¬¼¶ÂêÀö"; 
	case CHICKEN:return "ÐÇ¿ÕÕ½¶·¼¦";
	case GOODWISH: return "¸øÅóÓÑ×£¸£";
/*2003.04.22 added by stephen*/
	case GIVEUPBBS: return "½äÍøÖÐ";
/*2003.04.22 stephen add end*/
        case 10001:     return "Webä¯ÀÀ";
        default:        return "È¥ÁËÄÄ¶ù??";
        }
}

//modified by iamfat 2002.08.01
//¾­ÑéÖµÀ­¿ª6µµ Ã¿µµ10¼¶
char   *cexpstr(int exp)
{
	static char ce[11];
	char* c=   "-=+*#A";
	int i;
	int j;
	strcpy(ce, "          ");
	if(exp<0)return ce;
	i=exp/2000;
	i=i>5?5:i;
	j=(exp-i*2000)/200;
	j=j>9?9:j;
	memset(ce,c[i],j+1);
	return ce;
}

void iconexp(int exp)
{
        static char ce[11];
        char* c=   "-=+*#A";
        int i;
        int j;
        if(exp<0)j=-1;
	else {
        	i=exp/2000;
        	i=i>5?5:i;
        	j=(exp-i*2000)/200;
        	j=j>9?9:j;
	}
	
	j++;
	while(j--)
	{
		printf("<img src=/images/level/%d.gif border=0 align=absmiddle>",i);
	}

	//printf("<br>");

}
/*
{
	int orgdata;
	long randval;
	orgdata = exp;
	time(&randval);
	if (orgdata<=4) return "    *";
	if (orgdata<=16) 
	if (orgdata>(randval%12+4)) return "   **";
	else return "    *";
	if (orgdata<64)
	if (orgdata>(randval%48+16)) return "  ***";
	else return "   **";
	if (orgdata<256) 
	if (orgdata>(randval%192+64)) return " ****";
	else return "  ***";
	if (orgdata<640) 
	if (orgdata>(randval%384+256)) return "*****";
	else return " ****";
	return "*****";
//modified by roly
        if(exp==-9999) 	return "Ã»µÈ¼¶";
        if(exp<=100)  	return "ÐÂÊÖÉÏÂ·";
        if(exp<=450)	return "Ò»°ãÕ¾ÓÑ";
        if(exp<=850)	return "ÖÐ¼¶Õ¾ÓÑ";
        if(exp<=1500)	return "¸ß¼¶Õ¾ÓÑ";
        if(exp<=2500)	return "ÀÏÕ¾ÓÑ";
        if(exp<=3000)	return "³¤ÀÏ¼¶";
        if(exp<=5000)	return "±¾Õ¾ÔªÀÏ";
        if(exp<=10000)	return "¿ª¹ú´óÀÏ";
        return "³¬¼¶´óÀÏ";
}*/


char   *cperf(int perf) {
/* Commented by Amigo 2002.06.18. Change exp description. */
	/* added by roly */
//	if (perf==100) return "ºÃÑùµÄ";
//    	return "¼ÓÓÍ¼ÓÓÍ";
    /* add end */    

/* Following if criterias modified by Amigo 2002.06.18. */
/* Modified by Amigo 2002.06.23. Change perf description. */
//modified by iamfat 2002.08.01
/*
        if(perf==-9999)	return "ÐÂÊÖÉÏÂ·";
        if(perf<=100)	return "ÐÂÊÖÉÏÂ·";
        if(perf<=200)	return "¸Ï¿ì¼ÓÓÍ";
        if(perf<=400)	return "Å¬Á¦ÖÐ";
        if(perf<=500)	return "»¹²»´í";
        if(perf<=550)	return "ºÜºÃ";
        if(perf<=600)	return "ÓÅÐã";
        if(perf<=650)	return "ºÃ¼«ÁË";
        if(perf<=700)	return "±¾Õ¾³£¿Í";
        if(perf<=800)	return "ÖÐ¼áÁ¦Á¿";
        if(perf<=900)	return "×ÊÉîÈËÊ¿";
        if(perf<1000)	return "½­ºþÔªÀÏ";
        if(perf>=1000)	return "Éñ¡«¡«";
	return "»úÆ÷ÈË£¡";
*/
        if (perf <= 100) return GLY_CPERF0;
        if (perf <= 200) return GLY_CPERF1;
        if (perf <= 400) return GLY_CPERF2;
        if (perf <= 500) return GLY_CPERF3;
        if (perf <= 550) return GLY_CPERF4;
        if (perf <= 600) return GLY_CPERF5;
        if (perf <= 650) return GLY_CPERF6;
        if (perf <= 700) return GLY_CPERF7;
        if (perf <= 800) return GLY_CPERF8;
        if (perf <= 900) return GLY_CPERF9;
        if (perf < 1000) return GLY_CPERFA;
	return GLY_CPERFB;
}

/*2003.06.02 modified by stephen to sync web mode life count*/
int count_life_value(struct userec *urec) {
	int i;
	i=(time(0) - urec->lastlogin)/60;
	if((urec->userlevel & PERM_LONGLIFE) && !(urec->userlevel & PERM_XEMPT))
	        return (366*24*60-i)/1440;
	if((urec->userlevel & PERM_XEMPT) && (urec->userlevel & PERM_LONGLIFE) ||!strcasecmp(urec->userid, "guest"))
        	return 999;
	if((urec->userlevel & PERM_XEMPT) && !(urec->userlevel & PERM_LONGLIFE))
	        return 666;
	if(!(urec->userlevel & PERM_REGISTER))
		return (15*1440-i)/1440;
	return (90*1440-i)/1440;
}
/*2003.06.02 stephen modify end*/

int countexp(struct userec *x) {
        int tmp;
	if(!strcasecmp(x->userid, "guest")) return -9999;
        tmp=x->numposts + x->numlogins/5 + (time(0)-x->firstlogin)/86400 + x->stay/3600;
	//modified by roly form "numlogins/3" to numlogin/5
	if(tmp<0) tmp=0;
	//added by iamfat 2002.08.01
	if(tmp>12000) tmp=12000;
        return tmp;
}

int countperf(struct userec *x) {
	/*
	int day, logins, posts;
        if(!strcasecmp(x->userid, "guest")) return -9999;
	day=(time(0) - x->firstlogin)/86400+1;
	logins=x->numlogins;
	posts=x->numposts;
	if(day<=0 || logins<=0 || posts<0) return 0;
        return (10*posts/logins+10*logins/day);
	*/
	//modified by roly
	int     perf;
	int     reg_days;
	if (!strcmp(x->userid, "guest"))
		return -9999;
	reg_days = (time(0) - x->firstlogin) / 86400 + 1;
	
/*   perf=(reg_days/20>20?20:reg_days/20);
   perf=perf+(x->stay/36000>30?30:(x->stay/36000));
   perf=perf+(x->stay/(36*reg_days)>50?50:(x->stay/(36*reg_days)));*/
/* Modified by Amigo 2002.06.18. Change exp formula. */
   perf=(reg_days/4>250?250:reg_days/4);
   perf=perf+(x->stay/14400>250?250:(x->stay/14400));
   perf=perf+(x->stay/(36*reg_days)>500?500:(x->stay/(36*reg_days)));
   //added by iamfat 2002.08.29 for ¸¡¶¯±íÏÖÖµ
   randomize();
   //perf=perf + rand()%(MAX_LUCKY*2)-MAX_LUCKY;
   perf=(perf>MAX_PERF)?MAX_PERF:perf;
   perf=(perf<0)?0:perf;

   return perf;

}

int modify_mode(struct user_info *x, int newmode) {
	if(x==0) return;
	x->mode=newmode;
}

int save_user_data(struct userec *x) {
	FILE *fp;
	int n;
	n=getusernum(x->userid);
	if(n<0 || n>1000000) return 0;
/*
	fp=fopen(".PASSWDS", "r+");
	if(fp==0) return 0;
	fseek(fp, n*sizeof(struct userec), SEEK_SET);
	fwrite(x, sizeof(struct userec), 1, fp);
	fclose(fp);
*/
memcpy( &(shm_ucache->passwd[n]), x, sizeof(struct userec) );
	return 1;
}

int is_bansite(char *ip) {
	FILE *fp;
	char buf3[256];
	fp=fopen(".bansite", "r");
	if(fp==0) return 0;
	while(fscanf(fp, "%s", buf3)>0)
		if(!strcasecmp(buf3, ip)) return 1;
	fclose(fp);
	return 0;
}

int user_perm(struct userec *x, int level) {
	return (level?x->userlevel & level:1);
}

/* add by stiger, µÃµ½hashkey */
int uhashkey(char *userid, char *a1, char *a2)
{
      char *c=userid;
      int key=0;

      if( *c >= 'a' && *c <= 'z' ){
              *a1 = *c - 'a';
      }else if( *c >= 'A' && *c <= 'Z' ){
              *a1 = *c - 'A';
      }else
              *a1 = 0;

      c++;

      if( *c >= 'a' && *c <= 'z' ){
              *a2 = *c - 'a';
      }else if( *c >= 'A' && *c <= 'Z' ){
              *a2 = *c - 'A';
      }else
              *a2 = 0;

      c++;
      while( *c ){
              key += toupper(*c);
              c++;
      }

      return key%256;
}

int
getusernum(userid)
char   *userid;
{
	int i;
    char a1,a2;
    int key;
      /* use hash */
      key = uhashkey (userid, &a1, &a2);
      i = shm_ucache->hash[a1][a2][key];


      while(i){
              if (!strcasecmp(userid, shm_ucache->userid[i-1] )){
                      return i-1;
              }
              i = shm_ucache->next[i-1];
      }
      return -1;
      /* endof new */
}

struct userec *getuser(char *id) {
	static struct userec userec1;
	int uid;
	FILE *fp;
	uid=getusernum(id);
	if(uid<0) return 0;
/*
	fp=fopen(".PASSWDS", "r");
	fseek(fp, uid*sizeof(userec1), SEEK_SET);
	bzero(&userec1, sizeof(userec1));
	fread(&userec1, sizeof(userec1), 1, fp);
	fclose(fp);	
*/
memcpy(&userec1, &(shm_ucache->passwd[uid]), sizeof(userec1) );
	return &userec1;
}

int checkpasswd(char *pw_crypted, char *pw_try) {
	return !strcmp(crypt1(pw_try, pw_crypted), pw_crypted);
}

int checkuser(char *id, char *pw) {
	int i;
	struct userec *x;
	x=getuser(id);
	if(x==0) return 0;
	return checkpasswd(x->passwd, pw);
}

int count_id_num(char *id) {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active && !strcasecmp(shm_utmp->uinfo[i].userid, id)) total++;
	return total;
}

int count_online() {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++) 
		if(shm_utmp->uinfo[i].active) total++;
	return total;
}

int count_online2() {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active && shm_utmp->uinfo[i].invisible==0) total++;
	return total;
}

struct override fff[MAXFRIENDS];
int friendnum=0;

int loadfriend(char *id) {
        FILE *fp;
        char file[256];
        if(!loginok) return;
        sprintf(file, "home/%c/%s/friends", toupper(id[0]), id);
        fp=fopen(file, "r");
        if(fp) {
                friendnum=fread(fff, sizeof(fff[0]), 200, fp);
                fclose(fp);
        }
}

int isfriend(char *id) {
	static inited=0;
        int n; 
	if(!inited) {
		loadfriend(currentuser.userid);
		sort_friend(0,friendnum-1);
		inited=1;
	}
	return friend_search(id, fff,friendnum);	
/*	for(n=0; n<friendnum; n++)
	{		
		if(!strcasecmp(id, fff[n].id)) return 1;
	}
	return 0;*/

}

// add friend sort and binary-search by jacobson ------------2006.4.18
int friend_search(char *id,struct override *fff,int tblsize)
{
	int     hi, low, mid;
	int     cmp;
	
	if (id == NULL) {
		return 0;
	}
	hi = tblsize - 1;
	low = 0;
	while (low <= hi) {
		mid = (low + hi) / 2;
		cmp = strcasecmp(fff[mid].id, id); 
		if (cmp == 0) {
			return 1;
		}
		if (cmp > 0)
			hi = mid - 1;
		else
			low = mid + 1;
	}
	return 0;
}

void swap_friend(int a,int b)
{
	struct override c;
	c = fff[a];
	fff[a] = fff[b];
	fff[b] = c;
}
int compare_user_record(struct override *left,struct override *right)
{
	int retCode;
	retCode = strcasecmp(left->id, right->id);
	return retCode;
}
//quick sort
void sort_friend(int left, int right)
{
   int     i, last;
   
   if (left >= right) return;
   swap_friend(left, (left + right) / 2);
   last = left;
   for (i = left + 1; i <= right; i++){
      if(compare_user_record(&fff[i], &fff[left])<0){
         swap_friend(++last,i);
      }
   }
   swap_friend(left, last);
   sort_friend(left, last - 1);
   sort_friend(last + 1, right);
}

//-------------------------2006.4.18

struct override bbb[MAXREJECTS];
int badnum=0;

int loadbad(char *id) {
        FILE *fp;
        char file[256];
        if(!loginok) return;
        sprintf(file, "home/%c/%s/rejects", toupper(id[0]), id);
        fp=fopen(file, "r");
        if(fp) {
                badnum=fread(fff, sizeof(fff[0]), MAXREJECTS, fp);
                fclose(fp);
        }
}

int isbad(char *id) {
        static inited=0;
        int n;
        if(!inited) {
                loadbad(currentuser.userid);
                inited=1;
        }
        for(n=0; n<badnum; n++)
                if(!strcasecmp(id, bbb[n].id)) return 1;
        return 0;
}

//int touch_anonshm(int id)
//{
//	int acfd;
//	acfd=shm_lock("tmp/ACACHE.lock");
//	if(id==0){
//		id=anonshm->freenode;
//		if(id){
//			anonshm->freenode=anonshm->next[id-1];
//			anonshm->used++;
//			anonshm->next[id-1]=anonshm->usednode;
//			anonshm->usednode=id;
//			anonshm->item[id-1]=time(0);
//		}
//	}else if(id<=MAX_ANON){
//		anonshm->item[id-1]=time(0);
//	}
//	shm_unlock(acfd);
//	return id;
//}

int init_all() {
	int my_style=0;
	//int anonid;
	srand(time(0)*2+getpid());
	chdir(BBSHOME);
	my_style=http_init();
	seteuid(BBSUID);
	if(geteuid()!=BBSUID) http_fatal("uid error.");
	shm_init();
	loginok=user_init(&currentuser, &u_info);
	/*
     * Add some information to the status bar ... added by polygon 
    */
    
    time_t t = time(NULL);
    struct tm *tp = localtime(&t);
    if(currentuser.birthmonth==((tp->tm_mon)+1)&&currentuser.birthday==(tp->tm_mday))
         printf("<head><script>self.status=\"½ñÌìÊÇÄúµÄÉúÈÕ£¬ÈÕÔÂ¹â»ªBBS×£ÄúÉúÈÕ¿ìÀÖ£¡\"</script></head>");
    /*
    * Add End by polygon
    */
    //count anonymous login
	//#ifdef BBSFOOT
	//anonid=atoi(getparm("anonid"));
	//if(!loginok||!strcmp(currentuser.userid, "guest"))
	//{
		//if(anonid){
		//	touch_anonshm(anonid);
		//}else{
		//	char anonidstr[20];
		//	anonid=touch_anonshm(0);
		//	sprintf(anonidstr,"%d",anonid);
		//	setcookie("anonid", anonidstr);
		//}
	//}
	//#endif
	return my_style;
}

int init_no_http() {
	srand(time(0)+getpid());
	chdir(BBSHOME);
	shm_init();
}

char *sec(char c) {
	int i;
	for(i=0; i<SECNUM; i++) {
		if(strchr(seccode[i], c)) return secname[i][0];
	}
	return "(unknown.)";
}

char *flag_str(int access) {
	static char buf[80];
	char *flag2="";
	strcpy(buf, "  ");
	if(access & FILE_DIGEST) flag2="G";
	if(access & FILE_MARKED) flag2="M";
	if((access & FILE_MARKED) && (access & FILE_DIGEST)) flag2="B";
	sprintf(buf, "%s", flag2);
	return buf;
}

char *flag_str2(int access, int has_read) {
        static char buf[80];
	strcpy(buf, "   ");
        if(loginok) strcpy(buf, "N  ");
        if(access & FILE_DIGEST) buf[0]='G';
        if(access & FILE_MARKED) buf[0]='M';
        if((access & FILE_MARKED) && (access & FILE_DIGEST)) buf[0]='B';
		if(access & FILE_DELETED) buf[0]='W';
	if(has_read) buf[0]=tolower(buf[0]);
	if(buf[0]=='n') buf[0]=' ';
        return buf;
}

char *userid_str(char *s) {
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	strsncpy(tmp, s, 255);
	buf[0]=0;
	ptr=strtok(tmp, " ,();\r\n\t");
	while(ptr && strlen(buf)<400) {
		if(ptr2=strchr(ptr, '.')) {
			ptr2[1]=0;
			strcat(buf, ptr);
			strcat(buf, " ");
		} else {
			ptr=nohtml(ptr);
			sprintf(buf2, "<a href=bbsqry?userid=%s>%s</a> ", ptr, ptr);
			strcat(buf, buf2);
		}
		ptr=strtok(0, " ,();\r\n\t");
	}
	return buf;
}

int fprintf2(FILE *fp, char *s) {
	int i, tail=0, sum=0;
	if(s[0]==':' && s[1]==' ' && strlen(s)>79) {
		sprintf(s+76, "..\n");
		fprintf(fp, "%s", s);
		return;
	}
	for(i=0; s[i]; i++) {
		fprintf(fp, "%c", s[i]);
		sum++;
		if(tail) {
			tail=0;
		} else if(s[i]<0) {
			tail=s[i];
		}
		if(sum>=78 && tail==0) {
			fprintf(fp, "\n");
			sum=0;
		}
	}
}

struct fileheader *get_file_ent(char *board, char *file) {
	FILE *fp;
	char dir[80];
	static struct fileheader x;
	int num=0;
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		if(!strcmp(x.filename, file)) {
			fclose(fp);
			//added by iamfat 2002.08.10
			//check_anonymous(x.owner);
			//added end.
			return &x;
		}
		num++;
	}
	fclose(fp);
	return 0;
}

char *getbfroma(char *path) {
    FILE *fp;
	static char buf1[256], buf2[256];
	int len;
	if(path[0]==0) return "";
	path++;
	fp=fopen("0Announce/.Search", "r");
	if(fp==0) return "";
	while(1) {
		bzero(buf1,256);
		bzero(buf2,256);
		if(fscanf(fp, "%s %s", buf1, buf2)<=0) break;
		if(buf1[0]) buf1[strlen(buf1)-1]=0;
		if(buf1[0]=='*') continue;
		len = strlen(buf2);
		buf2[len]= '/';
		len++;
		buf2[len]= '\0';
		if(!strncmp(buf2, path,(len>strlen(path))? strlen(path):len))
			return buf1;
    }
	fclose(fp);
	return "";
}

int set_my_cookie() {
	FILE *fp;
	char path[256], buf[256], buf1[256], buf2[256];
	int my_t_lines=20, my_link_mode=0, my_def_mode=0, my_style=0;
	sprintf(path, "home/%c/%s/.mywww", toupper(currentuser.userid[0]), currentuser.userid);
 	fp=fopen(path, "r");
 	if(fp) {
 		while(1) {
 			if(fgets(buf, 80, fp)==0) break;
 			if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
 			if(!strcmp(buf1, "t_lines")) my_t_lines=atoi(buf2);
 			if(!strcmp(buf1, "link_mode")) my_link_mode=atoi(buf2);
			if(!strcmp(buf1, "def_mode")) my_def_mode=atoi(buf2);
			if(!strcmp(buf1, "mystyle")) my_style=atoi(buf2);
 		}
 		fclose(fp);
		sprintf(buf, "%d", my_t_lines);
		setcookie("my_t_lines", buf);
		sprintf(buf, "%d", my_link_mode);
		setcookie("my_link_mode", buf);
	        sprintf(buf, "%d", my_def_mode);
            	setcookie("my_def_mode", buf);
		sprintf(buf, "%d", my_style);
		setcookie("my_style", buf);
 	}
}

int has_fill_form() {
        FILE *fp;
        int r;
        char userid[256], tmp[256], buf[256], *ptr;
        fp=fopen("new_register", "r");
        if(fp==0) return 0;
        while(1) {
                if(fgets(buf, 100, fp)==0) break;
                r=sscanf(buf, "%s %s", tmp, userid);
                if(r==2) {
                        if(!strcasecmp(tmp, "userid:") && !strcasecmp(userid, currentuser.userid)) {
                                fclose(fp);
                                return 1;
                        }
                }
        }
        fclose(fp);
        return 0;
}

struct dir {
	char	filename[STRLEN-8];     /* the DIR files */
	unsigned int	id;
	unsigned int  	gid;
	char 	owner[STRLEN];
	char 	title[STRLEN-IDLEN-1];
	char 	szEraser[IDLEN+1];
	unsigned 	level;
	unsigned char 	accessed[4];   /* struct size = 256 bytes */
	unsigned int reid;
	time_t 	timeDeleted;
										
};

#ifdef USE_METALOG
void do_report(const char* fname, const char* content)
{
	char buf[256];
	
	if (strncmp(fname, "usies", 5)==0) {
		syslog(LOG_LOCAL4|LOG_INFO, "%-12.12s HTTP %s", 
			currentuser.userid, content);
	} else if (strncmp(fname, "trace", 5)==0) {
//		syslog(LOG_LOCAL6, "%-12.12s %8.8s HTTP %s",
//			currentuser.userid, cn_Ctime(time(0))+14, content);
		syslog(LOG_LOCAL6|LOG_INFO, "%-12.12s HTTP %s",
			currentuser.userid, content);
	} else {
		sprintf(buf, "%-12.12s %16.16s HTTP %s\n",
		                currentuser.userid, cn_Ctime(time(0))+6, content);
		f_append(fname, buf);
	}
	return;
}
#else
//added by iamfat 2002.08.17
void do_report(const char* fname, const char* content)
{
	char buf[256];
	sprintf(buf, "%-12.12s %16.16s HTTP %s\n", 
			currentuser.userid, cn_Ctime(time(0))+6, content);
	f_append(fname, buf);
}
#endif

void trace(const char* content)
{
	do_report("trace", content);
}
//added end

//added by iamfat 2002.08.27
int specialboard(char* board)
{
	if(file_has_word("etc/specialboard", board))return 1;
	return 0;
}
//added end

//added by iamfat 2002.09.03
//º¯ÊýÔ­ÐÍ: void SpecialID(const char* uid, char* host)
//º¯Êý¹¦ÄÜ: ¸ù¾ÝidÍ¨¹ý¶Ôetc/special.iniÀïÃæµÄÆ¥Åä ¸ø³ö¸ÃidÏàÓ¦µÄÌØÊâµÇÂ½host
//added by iamfat 2002.07.30
//²ÎÊý½éÉÜ:
//(IN)uid:ÓÃ»§µÄid
//(IN/OUT)host:´«µÝ¹ýÀ´ÓÃÓÚÐÞ¸ÄµÄÓÃ»§µÄhostÖ¸Õë
void SpecialID(const char* uid, char* host)
{
	FILE *fp;
	char line[STRLEN];
	char *special;
	//char *lasts;
	if(fp=fopen("etc/special.ini","r"))
	{
		while(fgets(line, STRLEN, fp))
		{
			//special=strtok_r(line," \r\n\t", &lasts);
			special=strtok(line," \r\n\t");
			if(special && !strcasecmp(uid,special))
			{
				//special=strtok_r(lasts," \r\n\t", &lasts);
				special=strtok(NULL," \r\n\t");
				if(special)strcpy(host,special);
				break;
			}
		}
		fclose(fp);
	}
}
//added end

/*
int
FLOCK(fd, op)
int     fd, op;
{
	switch (op) {
	case LOCK_EX:
		return lockf(fd, F_LOCK, 0);
	case LOCK_UN:
		return lockf(fd, F_ULOCK, 0);
	default:
		return -1;
	}
}
*/

#define FLOCK(x,y) flock(x,y)
int
dashf(char *fname)
{
struct stat st;
return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

int
dashd(char *fname)
{
   struct stat st;
   return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}


void printpretable()
{
	printf("<table align=center border=0 cellpadding=0 cellspacing=0>\n");
	printf("	<tr height=6>\n");
	printf("		<td width=6><img border=0 src='/images/lt.gif'></td>\n");
	printf("		<td background='/images/t.gif' width=100%%></td>\n");
	printf("		<td width=6><img border=0 src='/images/rt.gif'></td>\n");
	printf("	</tr>\n");
	printf("	<tr  height=100%%>\n");
	printf("		<td width=6 background='/images/l.gif'>\n");
	printf("		<td width=100%% nowrap bgcolor=#ffffff>\n");
}

void printposttable()
{
	printf("		</td>\n");
	printf("		<td width=6 background='/images/r.gif'></td>\n");
	printf("	</tr>\n");
	printf("	<tr height=6>\n");
	printf("		<td width=6><img border=0 src='/images/lb.gif'></td>\n");
	printf("		<td background='/images/b.gif' width=100%%></td>\n");
	printf("		<td width=6><img border=0 src='/images/rb.gif'></td>\n");
	printf("	</tr>\n");	
   	printf("</table>\n");
}

void printpretable_lite()
{
	printf("<table border=0 width=100%%>\n");
	printf("	<tr height=6><td background=/images/b.gif width=100%%></td></tr>\n");
	printf("	<tr><td>\n");
}

void printposttable_lite()
{	printf("	</td></tr>\n");
	printf("</table>\n");
}

int showcontent(char *filename)
{
	FILE *fp;
	char buf[512];
	char *ptr;
        fp=fopen(filename, "r");
        if(fp==0)return 0;
	printf("<pre class=ansi>");
        while(1)
        {
                char *id, *s;
                if(fgets(buf, 512, fp)==0)
                        break;
                if(!strncmp(buf, "·¢ÐÅÈË: ", 8))
                {
                        ptr=strdup(buf);
                        id=strtok(ptr+8, " ");
                        s=strtok(0, "");
                        if(id==0)
                                id=" ";
                        if(s==0)
                                s="\n";
                        if(strlen(id)<13 && getuser(id))
                        {
                                printf("·¢ÐÅÈË: %s%s", userid_str(id), s);
                                free(ptr);
                                continue;
                        }
                        free(ptr);
                }
                if(!strncmp(buf, ": ", 2))
                        printf("<font color=808080>");
                hhprintf("%s", buf);
                if(!strncmp(buf, ": ", 2))
                        printf("</font>");
        }
	printf("</pre>");
        fclose(fp);
	return 1;
}

void printpremarquee(char *width, char *height)
{
	printf("<marquee behavior=scroll direction=up width=%s height=%s scrollamount=1 scrolldelay=60 onmouseover='this.stop()' onmouseout='this.start()'>", width, height);
}

void printpostmarquee()
{
	printf("</marquee>");
}
		
void showheadline(char *board)
{
	char path[512], headline[512];
	strsncpy(path,anno_path_of(board),511);
	if(strstr(path, "..") || strstr(path, "SYSHome")) return;
	sprintf(headline, "0Announce%s/headline", path); 
        if(dashf(headline))
        {
                printpretable();
                printf("<b>HEADLINE</b><br>");
                printpremarquee("100%%", "48");
                showcontent(headline);
                printpostmarquee();
                printposttable();
	}
}

void showrecommend(char *board, int showall, int showborder) 
{
	FILE *fp;
	int i, index=0, total=0;
	int showed=0;
	char *ptr, path[512], names[512], name[1024][80], file[1024][80], buf[512], title[256]=" ";
	
	strsncpy(path,anno_path_of(board),511);
	if(strstr(path, "..") || strstr(path, "SYSHome")) return;
	sprintf(names, "0Announce%s/recommend/.Names", path);
	fp=fopen(names, "r");
	board=getbfroma(path);
	if(board[0] && !has_read_perm(&currentuser, board))return;
	if(fp==0) return;
	while(1) {
		if(fgets(buf, 511, fp)==0) break;
		if(!strncmp(buf, "# Title=", 8)) strcpy(title, buf+8);
		if(!strncmp(buf, "Name=", 5) && total<1023) {
			strcpy(name[total], trim(buf+5));
			strcpy(file[total], "");
			total++;
		}
		if(!strncmp(buf, "Path=~", 6) && total>0) {
			sprintf(file[total-1], "%s", trim(buf+6));
		}
	}
	fclose(fp);
	//if(strstr(title, "BM: SYSOPS") && !(currentuser.userlevel & PERM_SYSOP)) return;
	//if(strstr(title, "BM: OBOARDS") && !(currentuser.userlevel & PERM_OBOARDS)) return;
	//if(strstr(title, "BM: BMS") && !has_BM_perm(&currentuser, board)) return;
	buf[0]=0;
	if(total<=0)return;
	if(total>showall) 
		showall--;
	else
		showall=0;
	
	if(showborder==1)printpretable();
	else if(showborder==2)printpretable_lite();
	for(i=0; i<total; i++) {
		char *id;
		
		index++;
		if(strlen(name[i])<=39) {
			id="";
		} else {
			name[i][38]=0;
			id=name[i]+39;
			if(!strncmp(id, "BM: ", 4)) id+=4;
			ptr=strchr(id, ')');
			if(ptr) ptr[0]=0;
		}
		
		/*  add by roly 2002.01.03
		 *  ¸ù¾ÝÄ¿Â¼µÄ×÷ÕßÒÔ¼°ÓÃ»§È¨ÏÞÅÐ¶ÏÊÇ·ñÏÔÊ¾ 
		 */
		//if (strncmp(id,"ÍÆ¼ö",4)) continue;
		
		if (!strncmp(id,"SYSOPS",6) && !(currentuser.userlevel & PERM_SYSOPS)) continue;
		//modified by iamfat 2002.10.18 ±£Ö¤BMSÄ¿Â¼±¾°æ°æÖ÷¿É¼û
		if (!strncmp(id,"BMS",3) && !has_BM_perm(&currentuser, board))continue;
		//if(!strncmp(id,"BMS",3) && !(currentuser.userlevel & PERM_BOARDS)) continue;
		if (!strncmp(id,"OBOARDS",7) && !(currentuser.userlevel & PERM_OBOARDS)) continue;
		
		sprintf(buf, "0Announce%s/recommend%s", path, file[i]);
		if(!file_exist(buf))  
			continue;
		else if(file_isdir(buf))
		{
			if(showall && showed>=showall)
			{
				printf("<br><a href=bbs0an?path=%s/recommend>¸ü¶à...</a>",path);
				break;
			}
			if(showed>0)printf("<br>");
			printf("<img src=/images/types/folder.gif align=absmiddle border=0> <a href=bbs0an?path=%s/recommend%s>%s</a>", path, file[i], nohtml(name[i]));
			showed++;
		}
		else 
		{
			if(showall && showed>=showall)
			{
				printf("<br><a href=bbs0an?path=%s/recommend>¸ü¶à...</a>",path);
				break;
			}
			if(showed>0)printf("<br>");
			printf("<img src=/images/types/text.gif align=absmiddle border=0> <a href=bbsanc?path=%s/recommend%s>%s</a>", path, file[i], nohtml(name[i]));
			showed++;
		}
	}
	if(showborder==1)printposttable();
	else if(showborder==2)printposttable_lite();
}

void showrawcontent(char *filename)
{
        FILE *fp;
	char buf[512];
        fp=fopen(filename, "r");
        if(fp==0)return;
        while(fgets(buf,512,fp))
	{
		printf("%s",buf);
	}
	fclose(fp);
}


void showbrdlist(char *path,int withbr)
{
	struct boardheader *x;
	char board[256];
	FILE *fp;
	int showed=0;
	fp=fopen(path,"r");
	if(fp==0)return;
	while(fgets(board,255,fp))
	{
		strtok(board,"\t\r\n ");
		if(!board[0])continue;
		x=getbcache(board);
		if(!x)return;
		if(showed>0)
		{
			if(withbr)printf("<br>");
			else printf("&nbsp;");
		}
		if ( x->flag & BOARD_DIR_FLAG ){
		 printf("<img src=/images/types/folder.gif align=absmiddle border=0> <a href=bbsboa?board=%s>%s</a>"
		                         , board, x->title+10);}
		else {
		printf("<img src=/images/types/folder.gif align=absmiddle border=0> <a href=bbsdoc?board=%s>%s</a>"
			, board, x->title+10);
		}
		showed++;
	}
	fclose(fp);
	
}

int strtourl(char * url, char * str)
{
        int i,c;
	char mybuf[4];
	url[0]='\0';
	for(i=0;str[i];i++)
	{
		c=str[i];
		if(c=='\r'||c=='\n')
			return 0;
		if(c<-1)
			c+=256;
		if(isprint(c)&&(c!=' ')&&(c!='%'))
			sprintf(mybuf, "%c", c);
		else
			sprintf(mybuf,"%%%X",c);
		strcat(url,mybuf);
	}
	return 0;
}

int urltostr(char * str, char * url)
{
        int i,c,j,flag;
	char mybuf[4];
	/*      for(i=0;url[i];i++)
			url[i]=toupper(url[i]);
	*/
	for(i=0;url[i];i+=3)
	{
		if(url[i]!='%')
			return -1;
		if(isxdigit(url[i+1])==0 || isxdigit(url[i+2])==0)
			return -1;
	}
	flag=0;
	for(i=0,j=0;url[i];i+=3,j++)
	{
		mybuf[0]=url[i+1];
		mybuf[1]=url[i+2];
		mybuf[2]='\0';
		c=strtol(mybuf,NULL,16);
		if(flag==1)
		{
			c-=256;
			flag=0;
		}else
			if(c>128)
			{
				c-=256;
				flag=1;
			}
		str[j]=c;
	}
	str[j]='\0';
	return 0;
}

int isclubmember(char *member, char *board)
{
	FILE* fp;
	char uident[IDLEN+1];
	char line[256];
	char buf[256];
	
	
	struct boardheader *x;
	x=getbcache(board);
	sprintf(buf, "boards/%s/club_users", x->filename);
				
	if(!(fp=fopen(buf,"r")))
		return 0;
	while(fgets(line,256,fp))
	{
		strncpy(uident, line, IDLEN);
		uident[IDLEN] = '\0';
		strtok(uident," \r\n\t");
		if(strcasecmp(member,uident)==0)
		return 1;
	}
	fclose(fp);
	return 0;
}


