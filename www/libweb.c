#include "libweb.h"

char seccode[SECNUM][6]={
#ifdef FDQUAN
	"ab","cd","ij","kl","mn","op","qr","st","uv"
#else
		"0oOhH", "1pP", "2qQ", "3rRhH", "4sS", "5tT", "6uU", "7vV", "8wW", "9xX", "ayY", "bzZ"
#endif
};

char secname[SECNUM][2][20] = {
#ifdef FDQUAN
	{"BBS œµÕ≥", "[’æƒ⁄]"},
	{"∏¥µ©¥Û—ß", "[±æ–£]"},
	{"π‚ª™π´Àæ", "[«⁄÷˙]"},	
	{"…ÁÕ≈◊È÷Ø", "[Õ≈ÃÂ]"},
	{"æ∫»¸ªÓ∂Ø", "[¡Ÿ ±]"},
	{"Ã∏ÃÏ¡ƒµÿ", "[∏––‘]"},
	{"”Œœ∑–›œ–", "[–›œ–]"},
	{"∏ˆ»À∑Á≤…", "[∏ˆ»À]"},
	{"ª˝∑÷¬€Ã≥", "[Õ≈ÃÂ]"}
#else
	{"BBS œµÕ≥", "[’æƒ⁄]"},
	{"∏¥µ©¥Û—ß", "[±æ–£]"},
	{"‘∫œµ∑Á≤…", "[–£‘∞]"},
	{"µÁƒ‘ºº ı", "[µÁƒ‘]"},
	{"–›œ–”È¿÷", "[–›œ–]"},
	{"Œƒ—ß“’ ı", "[Œƒ“’]"},
	{"ÃÂ”˝Ω°…Ì", "[‘À∂Ø]"},
	{"∏––‘ø’º‰", "[∏––‘]"},
	{"–¬Œ≈–≈œ¢", "[–¬Œ≈]"},
	{"—ßø∆—ß ı", "[—ßø∆]"},
	{"”∞ ”“Ù¿÷", "[”∞“Ù]"},
	{"Ωª“◊◊®«¯", "[Ωª“◊]"}
#endif
};

int loginok=0;

struct userec currentuser;
struct user_info *u_info;

char fromhost[40]; // IPv6 addresses can be represented in 39 chars.

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
	struct userec user;
	getuserec(userid, &user);
	if(2*getmailboxsize(user.userlevel)>getmailsize(userid))
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
		if (fp != NULL) {
			fscanf(fp, "%d", &mailsize);
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
	if(fp != NULL){
		fprintf(fp, "%d", mailsize);
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
	fp = fopen(file, "r");
	if(fp == NULL)
		return 0;
	while(1) {
		bzero(buf, 256);
		if(fgets(buf, 255, fp)==0)
			break;
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
	if (fp != NULL) {
		fprintf(fp, "%s", buf);
		fclose(fp);
	}
}

struct stat *f_stat(char *file) {
	static struct stat buf;
	bzero(&buf, sizeof(buf));
	if(stat(file, &buf)==-1) bzero(&buf, sizeof(buf));
	return &buf;
}

int del_record(char *file, int size, int num) {
	FILE *fp;
	int total, i;
	char buf[4096];
	if(size<1 || size>4096) return 0;
	total=file_size(file)/size;
	if(total<1 || total>1000000) return 0;
	fp = fopen(file, "r+");
	if (fp == NULL)
		return 0;
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
	char weeknum[7][3]={"ÃÏ","“ª","∂˛","»˝","Àƒ","ŒÂ","¡˘"};
	tm = localtime(&t);
	sprintf(s,"%4dƒÍ%02d‘¬%02d»’%02d:%02d:%02d –«∆⁄%2s",
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
	int i=0;

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

char *get_old_shm(int key, int size) {
	int id;
	id=shmget(key, size, 0);
	if(id<0) return 0;
	return shmat(id, NULL, 0);
}

char *getsenv(char *s) {
	char *t=getenv(s);
	if(t) return t;
	return "";
}

void http_quit(void) {
	printf("\n</html>\n");
	FCGI_Finish();
}

void http_fatal(enum HTTP_STATUS status, const char *prompt)
{
	printf("Content-type: text/html; charset=%s\nStatus: %d\n\n",
			CHARSET, status);
	printf("<html><head><title>∑¢…˙¥ÌŒÛ</title></head><body><div>%s</div>"
			"<a href=javascript:history.go(-1)>øÏÀŸ∑µªÿ</a></body></html>",
			prompt);
	FCGI_Finish();
}

int strnncpy(char *s, int *l, char *s2) {
	strlcpy(s+(*l), s2, strlen(s2));
	(*l)+=strlen(s2);
	return 0;
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
			strlcpy(ansibuf, &buf[i+2], m-(i+2)+1);
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
	return 0;
}


int hprintf(char *fmt, ...) {
	char buf[8096], buf2[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf2, 1023, fmt, ap);
	va_end(ap);
	hsprintf(buf, "%s", buf2);
	printf("%s", buf);
	return 0;
}

int hhprintf(char *fmt, ...) {
	char buf0[1024], buf[1024], *s;
	int len=0;
	int my_link_mode;
	int msg=0;
	int mailto=0;
	int board=0;
	//int upload=0;
	int special=0;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023]=0;
	s=buf;
	my_link_mode=atoi(getparm("my_link_mode"));
	if(my_link_mode==1)  return hprintf("%s", buf);
	if(!strcasestr_gbk(s, "http://") 
			&& !strcasestr_gbk(s, "ftp://") 
			&& !strcasestr_gbk(s, "msg://")
			&& !strcasestr_gbk(s, "mailto://")
			&& !strcasestr_gbk(s, "board://")
	  )
		return hprintf("%s", buf);
	while(s[0]) {
		if(!strncasecmp(s, "http://", 7) || !strncasecmp(s, "ftp://", 6)) special=1;
		else if(!strncasecmp(s, "msg://", 6))	{msg=1;special=1;}
		else if(!strncasecmp(s, "mailto://", 9)){mailto=1;special=1;}
		else if(!strncasecmp(s, "board://", 8))	{board=1;special=1;}
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
					if(1)//Disable the thumbnail
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
					if(tmp==0)
						return 0;
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
	return 0;
}

// Convert a hex char 'c' to a base 10 integer.
static int __to16(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0;
}

static int __unhcode(char *s)
{
	int m, n;
	for(m = 0, n = 0; s[m] != 0; m++, n++) {
		if (s[m] == '+') {
			s[n] = ' ';
			continue;
		}
		if (s[m] == '%') {
			s[n] = __to16(s[m+1]) * 16 +__to16(s[m+2]);
			m += 2;
			continue;
		}
		s[n]=s[m];
	}
	s[n] = 0;
	return 0;
}

char parm_name[256][80], *parm_val[256];
int parm_num = 0;

// Adds 'name' 'val' pairs to global arrays 'parm_name' and 'parm_val'.
// Increases 'parm_num' by 1.
static int parm_add(const char *name, const char *val)
{
	int len = strlen(val);
	if (parm_num >= sizeof(parm_val) - 1)
		http_fatal(HTTP_STATUS_BADREQUEST, "too many parms.");
	parm_val[parm_num] = malloc(len + 1);
	if (parm_val[parm_num] == NULL)
		http_fatal(HTTP_STATUS_SERVICE_UNAVAILABLE, "memory overflow2");
	strlcpy(parm_name[parm_num], name, sizeof(parm_name[0]));
	strlcpy(parm_val[parm_num], val, len + 1);
	return ++parm_num;
}

// Frees 'parm_val'.
static int parm_free(void)
{
	int i;
	for (i = parm_num - 1; i >= 0; --i) {
		free(parm_val[i]);
	}
	return parm_num = 0;
}

// Searches 'parm_name' for 'name'.
// Returns corresponding 'parm_val' if found, otherwise "".
char *getparm(const char *name)
{
	int n;
	for(n = 0; n < parm_num; n++) 
		if(!strcasecmp(parm_name[n], name))
			return parm_val[n];
	return "";
}

// Uses delimeter 'delim' to split 'buf' into "key=value" pairs.
// Adds these pairs into global arrays.
static void parm_parse(char *buf, const char *delim)
{
	char *t2, *t3;
	t2 = strtok(buf, delim);
	while (t2 != NULL) {
		t3 = strchr(t2, '=');
		if (t3 != NULL) {
			*t3++ = '\0';
			__unhcode(t3);
			parm_add(trim(t2), t3);
		}
		t2 = strtok(NULL, delim);
	}
	return;
}

// Read parameters from HTTP header.
void http_parm_init(void)
{
	int n;
	char *buf, buf2[1024];

	parm_free();

	n = atoi(getsenv("CONTENT_LENGTH"));
	if(n > 5000000)
		n = 5000000;
	buf = malloc(n + 1);
	if(buf == NULL)
		http_fatal(HTTP_STATUS_SERVICE_UNAVAILABLE, "memory overflow");
	fread(buf, 1, n, stdin);
	buf[n] = '\0';
	parm_parse(buf, "&");
	free(buf);

	strlcpy(buf2, getsenv("QUERY_STRING"), sizeof(buf2));
	parm_parse(buf2, "&");

	strlcpy(buf2, getsenv("HTTP_COOKIE"), sizeof(buf2));
	parm_parse(buf2, ";");

	return;
}

static int http_init(void)
{
	int my_style;

	http_parm_init();

#ifdef SQUID
	char *fromtmp;
	fromtmp = strrchr(getsenv("HTTP_X_FORWARDED_FOR"), ',');
	if (fromtmp == NULL) {
		strlcpy(fromhost, getsenv("HTTP_X_FORWARDED_FOR"), sizeof(fromhost));
	} else {
		while ((*fromtmp < '0')&&(*fromtmp != '\0'))
			fromtmp++;
		strlcpy(fromhost, fromtmp, sizeof(fromhost));
	}
#else
	strlcpy(fromhost, getsenv("REMOTE_ADDR"), sizeof(fromhost));
#endif

	return my_style;
}

// Gets shared memory. Returns 0 if OK, exits on error.
int shm_init(void)
{
	if (resolve_ucache() == -1)
		exit(1);
	resolve_utmp();
	if (resolve_boards() < 0)
		exit(1);
	if (utmpshm == NULL || brdshm == NULL)
		exit(1);
	return 0;
}

// Load user information from cookie.
// If everything is OK, initializes *'x', 'y' and returns 1,
// on error, set *'x' to 0, 'y' to NULL and returns 0.
int user_init(struct userec *x, struct user_info **y)
{
	char id[IDLEN + 1];
	int i, key;

	// Get information from cookie.
	strlcpy(id, getparm("utmpuserid"), sizeof(id));
	i = strtol(getparm("utmpnum"), NULL, 10);
	key = strtol(getparm("utmpkey"), NULL, 10);

	// Boundary check.
	if (i <= 0 || i > MAXACTIVE)
		return 0;
	// Get user_info from utmp.
	(*y) = &(utmpshm->uinfo[i - 1]);

	// Verify cookie and user status.
	if (strncmp((*y)->from, fromhost, 16)
			|| (*y)->utmpkey != key
			|| (*y)->active == 0
			|| (*y)->userid[0] == '\0'
			|| (*y)->mode != WWW) {
		*y = NULL;
		return 0;
	}

	// If not normal user.
	if (!strcasecmp((*y)->userid, "new")
			|| !strcasecmp((*y)->userid, "guest")) {
		*y = NULL;
		return 0;
	}

	// Refresh idle time.
	(*y)->idle_time = time(NULL);

	// Get userec from ucache.
	getuserbyuid(x, (*y)->uid);
	if (strcmp(x->userid, id)) {
		memset(x, 0, sizeof(*x));
		return 0;
	}

	return 1;
}

void xml_header(const char *xslfile)
{
	printf("Content-type: text/xml; charset=%s\n\n", CHARSET);
	printf("<?xml version=\"1.0\" encoding=\"%s\"?>\n", CHARSET);
	printf("<?xml-stylesheet type=\"text/xsl\" href=\"/xsl/%s.xsl\"?>\n", xslfile);
}

void http_header(void)
{
	printf("Content-type: text/html; charset=%s\n\n", CHARSET);
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
			" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	printf("<html>\n<head>\n");
}

void refreshto(int second, const char *url)
{
	printf("<meta http-equiv='Refresh' content='%d; url=%s' />\n", second, url);
}

void setcookie(const char *a, const char *b)
{
	printf("<script>document.cookie='%s=%s'</script>\n", a, b);
}

static int sig_append(FILE *fp, char *id, int sig) {
	FILE *fp2;
	char path[256];
	char buf[100][256];
	int i, total;
	if (sig < 0 || sig > 10)
		return -1;
	if (getuser(id) == 0)
		return -1;
	sprintf(path, "home/%c/%s/signatures", toupper(id[0]), id);
	fp2 = fopen(path, "r");
	if (fp2 == NULL)
		return -1;
	for(total=0; total<100; total++) //total<255 -> 100 money modfied 03.10.21
		if(fgets(buf[total], 255, fp2)==0) break;
	fclose(fp2);
	for(i=sig*6; i<sig*6+6; i++) {
		if(i>=total) break;
		fprintf(fp, "%s", buf[i]);
	}
	fprintf(fp,"\n");
	return 0;
}

static int fprintf2(FILE *fp, char *s) {
	int i, tail=0, sum=0;
	if(s[0]==':' && s[1]==' ' && strlen(s)>79) {
		sprintf(s+76, "..\n");
		fprintf(fp, "%s", s);
		return -1;
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
	return 0;
}

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig) {
	FILE *fp, *fp2;
	char buf3[256], dir[256];
	struct fileheader header;
	int t, i;
	/* added by roly check mail size */
	if (!mailnum_under_limit(currentuser.userid))
		http_fatal(HTTP_STATUS_OK, "ƒ˙µƒ–≈º˛»›¡ø≥¨±Í£¨Œﬁ∑®∑¢–≈");
	if (!mailsize_under_limit(userid))
		http_fatal(HTTP_STATUS_OK, " ’º˛»À–≈º˛»›¡ø≥¨±Í£¨Œﬁ∑® ’–≈");

	/* add end */
	if(strstr(userid, "@")) 
		return post_imail(userid, title, file, id, nickname, ip, sig);
	bzero(&header, sizeof(header));
	strcpy(header.owner, id);
	for(i=0; i<100; i++) {
		t=time(0)+i;
		sprintf(buf3, "mail/%c/%s/M.%d.A", toupper(userid[0]), userid, i+time(0));
		if(!dashf(buf3))
			break;
	}
	if(i>=99) return -1;
	sprintf(header.filename, "M.%d.A", t);
	strlcpy(header.title, title, 60);
	fp = fopen(buf3, "w");
	if (fp == NULL)
		return -2;
	fp2 = fopen(file, "r");
	fprintf(fp, "ºƒ–≈»À: %s (%s)\n", id, nickname);
	fprintf(fp, "±Í  Ã‚: %s\n", title);
	//	fprintf(fp, "∑¢–≈’æ: %s (%s)\n", BBSNAME, Ctime(time(0)));
	//modified by iamfat 2002.08.01
	fprintf(fp, "∑¢–≈’æ: %s (%s)\n", BBSNAME, cn_Ctime(time(0)));
	fprintf(fp, "¿¥  ‘¥: %s\n\n", ip);
	if (fp2 != NULL) {
		while(1) {
			if(fgets(buf3, 256, fp2)<=0) break;
			fprintf2(fp, buf3);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "[m[1;%2dm°˘ ¿¥‘¥:°§%s %s°§HTTP [FROM: %-.20s][m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
	fclose(fp);
	sprintf(dir, "mail/%c/%s/.DIR", toupper(userid[0]), userid);
	fp = fopen(dir, "a");
	if (fp == NULL)
		return -1;
	fwrite(&header, sizeof(header), 1, fp);
	fclose(fp);
}

int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig) {
	FILE *fp1, *fp2;
	char buf[256];
	if(strstr(userid, ";") || strstr(userid, "`"))
		http_fatal(HTTP_STATUS_BADREQUEST, "¥ÌŒÛµƒ ’–≈»Àµÿ÷∑");
	sprintf(buf, "sendmail -f %s.bbs@%s '%s'", id, BBSHOST, userid);
	fp2 = popen(buf, "w");
	fp1 = fopen(file, "r");
	if(fp1 == NULL || fp2 == NULL)
		return -1;
	fprintf(fp2, "From: %s.bbs@%s\n", id, BBSHOST);
	fprintf(fp2, "To: %s\n", userid);
	fprintf(fp2, "Subject: %s\n\n", title);
	while(1) {
		if(fgets(buf, 255, fp1)==0)
			break;
		if(buf[0]=='.' && buf[1]=='\n')
			continue;
		fprintf(fp2, "%s", buf);
	}
	fprintf(fp2, "\n--\n");
	sig_append(fp2, id, sig);
	fprintf(fp2, "[m[1;%2dm°˘ ¿¥‘¥:°§%s %s°§HTTP [FROM: %-.20s][m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
	fprintf(fp2, ".\n");
	fclose(fp1);
	pclose(fp2);
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
	if(brd==0)
		http_fatal(HTTP_STATUS_NOTFOUND, "¥ÌŒÛµƒÃ÷¬€«¯");
	strcpy(board2, brd->filename);	

	/* add end */

	for(i=0; i<100; i++) {
		t=time(0)+i;
		sprintf(buf3, "boards/%s/M.%d.A", board2, t);
		if(!dashf(buf3))
			break;
	}
	if(i>=99) return -1;
	sprintf(header.filename, "M.%d.A", t);
	header.id = get_nextid(board2);
	if (strncmp(title, "Re: ", 4) == 0 && !(o_id == -1 && o_gid == -1)){
		strlcpy(header.title, title, 68);
		header.reid = o_id;
		header.gid = o_gid;
	}else{
		strlcpy(header.title, title, 50);
		header.reid = header.id;
		header.gid = header.id;
	}
	fp = fopen(buf3, "w");
	fp2 = fopen(file, "r");
	fprintf(fp, "∑¢–≈»À: %s (%s), –≈«¯: %s\n±Í  Ã‚: %s\n∑¢–≈’æ: %s (%s)\n\n",
			//		id, nickname, board2, title, BBSNAME, Ctime(time(0)));
			//modified by iamfat 2002.08.01
		id, nickname, board2, header.title, BBSNAME, cn_Ctime(time(0)));
	if (fp2 != NULL) {
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
	//	fprintf(fp, "\n[1;%dm°˘ ¿¥‘¥:£Æ%s %s [FROM: %.20s][m\n", 31+rand()%7, BBSNAME, "http://bbs.fudan.edu.cn", ip);
	fprintf(fp, "[m[1;%2dm°˘ ¿¥‘¥:°§%s %s°§HTTP [FROM: %-.20s][m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
	fclose(fp);
	sprintf(buf3, "boards/%s/.DIR", board2);
	fp = fopen(buf3, "a");
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

char* anno_path_of(char *board) {
	FILE *fp;
	static char buf[256], buf1[80], buf2[80];
	fp = fopen("0Announce/.Search", "r");
	if (fp == NULL)
		return "";
	while(1) {
		if(fscanf(fp, "%s %s", buf1, buf2)<=0)
			break;
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
	struct boardheader *x;	/* ∞Ê√Ê≤ª¥Ê‘⁄∑µªÿ0, p∫Õz∞Ê√Ê∑µªÿ1, ”–»®œﬁ∞Ê√Ê∑µªÿ1. */
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

int count_mails(char *id, int *total, int *unread) {
	struct fileheader x1;
	char buf[256];
	int n;
	FILE *fp;
	*total=0;
	*unread=0;
	if(getuser(id) == 0)
		return 0;
	sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(id[0]), id);
	fp=fopen(buf, "r");
	if (fp == NULL)
		return;
	while(fread(&x1, sizeof(x1), 1, fp)>0) {
		(*total)++;
		if(!(x1.accessed[0] & FILE_READ)) (*unread)++;
	}
	fclose(fp);
}

int send_msg(char *myuserid, int mypid, char *touserid, int topid, char msg[256]) {
	char msgbuf[256], buf3[256];
	int i;
	for(i=0; i<strlen(msg); i++)
		if((0<msg[i] && msg[i]<=27 )|| msg[i]==-1) msg[i]=32;
	if(mypid<=0) return -1;
	sprintf(msgbuf, "[0;1;44;36m%-12.12s[33m([36m%-5.5s[33m):[37m%-54.54s[31m(^Zªÿ)[m[%05dm\n",
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

// Convert exp to icons.
int iconexp(int exp, int *repeat)
{
	int i, j;

	if (exp < 0)
		j = -1;
	else {
		i = exp / 2000;
		i = i > 5 ? 5 : i;
		j = (exp - i * 2000) / 200;
		j = j > 9 ? 9 : j;
	}
	*repeat = ++j;
	return i;
}

int save_user_data(struct userec *x) {
	FILE *fp;
	int n;
	n = searchuser(x->userid) - 1;
	if(n < 0 || n >= MAXUSERS)
		return 0;
	memcpy( &(uidshm->passwd[n]), x, sizeof(struct userec) );
	return 1;
}

int user_perm(struct userec *x, int level) {
	return (level?x->userlevel & level:1);
}

struct override fff[MAXFRIENDS];
int friendnum=0;

int loadfriend(char *id) {
	FILE *fp;
	char file[256];
	if(!loginok) return;
	sprintf(file, "home/%c/%s/friends", toupper(id[0]), id);
	fp = fopen(file, "r");
	if (fp != 0) {
		friendnum=fread(fff, sizeof(fff[0]), 200, fp);
		fclose(fp);
	}
}

// add friend sort and binary-search by jacobson ------------2006.4.18
static int friend_search(char *id,struct override *fff,int tblsize)
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

int isfriend(char *id) {
	static inited=0;
	int n; 
	if(!inited) {
		loadfriend(currentuser.userid);
		sort_friend(0,friendnum-1);
		inited=1;
	}
	return friend_search(id, fff,friendnum);	
}

static void swap_friend(int a,int b)
{
	struct override c;
	c = fff[a];
	fff[a] = fff[b];
	fff[b] = c;
}
static int compare_user_record(struct override *left,struct override *right)
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

static struct override bbb[MAXREJECTS];
static int badnum=0;

int loadbad(char *id) {
	FILE *fp;
	char file[256];
	if(!loginok) return;
	sprintf(file, "home/%c/%s/rejects", toupper(id[0]), id);
	fp = fopen(file, "r");
	if(fp != NULL) {
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

int fcgi_init_all(void)
{
	srand(time(NULL) * 2 + getpid());
	chdir(BBSHOME);
	seteuid(BBSUID);
	if(geteuid() != BBSUID)
		http_fatal(HTTP_STATUS_INTERNAL_ERROR, "uid error.");
	shm_init();

	return 0;
}

int fcgi_init_loop(void)
{
	int my_style = http_init();
	loginok = user_init(&currentuser, &u_info);

	return my_style;
}

// Will be abolished
int init_all(void)
{
	int my_style = 0;
	srand(time(NULL) * 2 + getpid());
	chdir(BBSHOME);
	my_style = http_init();
	seteuid(BBSUID);
	if(geteuid() != BBSUID)
		http_fatal(HTTP_STATUS_INTERNAL_ERROR, "uid error.");
	shm_init();
	loginok = user_init(&currentuser, &u_info);

	// Happy birthday in status bar.
	time_t t = time(NULL);
	struct tm *tp = localtime(&t);
	if(currentuser.birthmonth == ((tp->tm_mon) + 1)
			&&currentuser.birthday==(tp->tm_mday)) {
		printf("<head><script>self.status=\""
				"ΩÒÃÏ «ƒ˙µƒ…˙»’£¨»’‘¬π‚ª™BBS◊£ƒ˙…˙»’øÏ¿÷£°\"</script></head>");
	}

	return my_style;
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
	strlcpy(tmp, s, 255);
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
	if(fp == NULL)
		return "";
	while(1) {
		bzero(buf1,256);
		bzero(buf2,256);
		if(fscanf(fp, "%s %s", buf1, buf2)<=0)
			break;
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

int set_my_cookie(void)
{
	FILE *fp;
	char path[256], buf[256], buf1[256], buf2[256];
	int my_t_lines=20, my_link_mode=0, my_def_mode=0, my_style=0;
	sprintf(path, "home/%c/%s/.mywww", toupper(currentuser.userid[0]), currentuser.userid);
	fp = fopen(path, "r");
	if(fp != NULL) {
		while(1) {
			if(fgets(buf, 80, fp)==0)
				break;
			if(sscanf(buf, "%80s %80s", buf1, buf2)!=2)
				continue;
			if(!strcmp(buf1, "t_lines"))
				my_t_lines=atoi(buf2);
			if(!strcmp(buf1, "link_mode"))
				my_link_mode=atoi(buf2);
			if(!strcmp(buf1, "def_mode"))
				my_def_mode=atoi(buf2);
			if(!strcmp(buf1, "mystyle"))
				my_style=atoi(buf2);
		}
		fclose(fp);
		printf("<my_t_lines>%d</my_t_lines>\n<my_link_mode>%d</my_link_mode>\n"
				"<my_def_mode>%d</my_def_mode>\n<my_style>%d</my_style>\n",
				my_t_lines, my_link_mode, my_def_mode, my_style);
	}
}

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

void printpretable(void)
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

void printposttable(void)
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

void printpretable_lite(void)
{
	printf("<table border=0 width=100%%>\n");
	printf("	<tr height=6><td background=/images/b.gif width=100%%></td></tr>\n");
	printf("	<tr><td>\n");
}

void printposttable_lite(void)
{	printf("	</td></tr>\n");
	printf("</table>\n");
}

int showcontent(char *filename)
{
	FILE *fp;
	char buf[512];
	char *ptr;
	fp=fopen(filename, "r");
	if(fp == NULL)
		return 0;
	printf("<pre class=ansi>");
	while(1)
	{
		char *id, *s;
		if(fgets(buf, 512, fp)==0)
			break;
		if(!strncmp(buf, "∑¢–≈»À: ", 8))
		{
			ptr=strdup(buf);
			id=strtok(ptr+8, " ");
			s=strtok(0, "");
			if(id==0)
				id=" ";
			if(s==0)
				s="\n";
			if(strlen(id) < 13 && getuser(id))
			{
				printf("∑¢–≈»À: %s%s", userid_str(id), s);
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

void printpostmarquee(void)
{
	printf("</marquee>");
}

void showheadline(char *board)
{
	char path[512], headline[512];
	strlcpy(path,anno_path_of(board),511);
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

	strlcpy(path,anno_path_of(board),511);
	if(strstr(path, "..") || strstr(path, "SYSHome")) return;
	sprintf(names, "0Announce%s/recommend/.Names", path);
	fp=fopen(names, "r");
	board=getbfroma(path);
	if(board[0] && !has_read_perm(&currentuser, board))return;
	if(fp == NULL)
		return;
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
		 *  ∏˘æ›ƒø¬ºµƒ◊˜’ﬂ“‘º∞”√ªß»®œﬁ≈–∂œ «∑Òœ‘ æ 
		 */
		//if (strncmp(id,"Õ∆ºˆ",4)) continue;

		if (!strncmp(id,"SYSOPS",6) && !(currentuser.userlevel & PERM_SYSOPS)) continue;
		//modified by iamfat 2002.10.18 ±£÷§BMSƒø¬º±æ∞Ê∞Ê÷˜ø…º˚
		if (!strncmp(id,"BMS",3) && !has_BM_perm(&currentuser, board))continue;
		//if(!strncmp(id,"BMS",3) && !(currentuser.userlevel & PERM_BOARDS)) continue;
		if (!strncmp(id,"OBOARDS",7) && !(currentuser.userlevel & PERM_OBOARDS)) continue;

		sprintf(buf, "0Announce%s/recommend%s", path, file[i]);
		if(!dashf(buf))  
			continue;
		else if(file_isdir(buf))
		{
			if(showall && showed>=showall)
			{
				printf("<br><a href=bbs0an?path=%s/recommend>∏¸∂‡...</a>",path);
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
				printf("<br><a href=bbs0an?path=%s/recommend>∏¸∂‡...</a>",path);
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
	if(fp == NULL)
		return;
	while(fgets(buf,512,fp))
	{
		printf("%s",buf);
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

