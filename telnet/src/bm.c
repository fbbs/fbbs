#include "bbs.h"
//extern  cmpbnames();
//rewritten by iamfat 2002.07.20
//Ô­Ê¼ÎÄ¼þ´ÓÒÔÇ°µÄ±¸·Ý0720ÀïÃæÕÒ:P

//ÐèÒªÓÃµ½µÄÍâ²¿º¯Êý
extern char * strcasestr(char* haystack, char* needle);
extern struct boardheader *getbcache();

//¶¨ÒåÒ³Ãæ´óÐ¡
#define BBS_PAGESIZE (t_lines-4)

//´ÓµÚnoÐÐ¿ªÊ¼,ÔÚÎÄ¼þfnameÖÐ²»·Ö´óÐ¡Ð´²éÕÒstr,
//	ÈôÕÒµ½,·µ»ØÆäËùÔÚµÄÐÐÊý
//		·ñÔò·µ»Ø-1
int text_find(char* fname, int no, char* str, char* line)
{
	int fd;
	int gotit=0;
	if((fd=open(fname,O_RDONLY))==-1)
		return -1;
	seek_nth_line(fd, no);
	while(readln(fd, genbuf)){
		//strtok(genbuf,"\n");
		if(strcasestr(genbuf,str)){
			gotit=1;
			if(line)
				strcpy(line, genbuf);
			break;
		}
		no++;
	}
	close(fd);
	if(gotit)
		return no;
	return -1;
}
//º¯Êý: void list_text(char *fname, void (*title_show)(), int (*key_deal)(), int (*ifcheck)())
//¹¦ÄÜ: È«ÆÁ°´ÐÐÁÐ³öÎÄ±¾ÎÄ¼þµÄÄÚÈÝ, ²¢ÊµÏÖÓÎ±ê²Ù×÷, ÒÔ¼°Ñ¡Ïî²Ù×÷
//(IN)fname: ÎÄ±¾ÎÄ¼þµÄÎÄ¼þÃû
//(IN)title_show(): ÏÔÊ¾±êÌâÐÐ(3ÐÐ)
//(IN)key_deal(fname,ch,line): ·Ç³£¹æ¼üµÄ´¦Àíº¯Êý, fnameÎÄ¼þÃû, ch°´¼ü, lineÓÎ±êÍ£ÁôµÄÐÐ
//(IN)ifcheck(): ÅÐ¶ÏÊÇ·ñÔÚ¸ÃÏîÇ°ÃæÏÔÊ¾¡Ì, Èç¹ûÎªNULL, ÔòºöÂÔÑ¡Ôñ²Ù×÷
void list_text(	char *fname, 
				void (*title_show)(), 
				int (*key_deal)(), 
				int (*ifcheck)()
			  )
{
	int		fd;
	int		no = 0, from = 0;
	int 	to = 0;
	int 	ch;
	int 	empty = 0;
	int 	redrawflag = 1;
	int 	y = 3;
	char 	textfile[STRLEN];
	char 	line[STRLEN];
	strcpy(textfile,fname);
	while(1){
		if(redrawflag){
			clear();
			title_show();
			if((fd=open(textfile,O_RDONLY))==-1){
				empty=1;
			}else	{
				empty=0;
				to=from;
				seek_nth_line(fd, from);
				y=3;
				move(3, 0);
				while(readln(fd, genbuf)){
					strtok(genbuf,"\n");
					if(ifcheck)
						prints(" %-2s%-76s\n",(*ifcheck)(genbuf)?"¡Ì":"  ",genbuf);
					else 
						prints(" %-78s\n",genbuf);
					to++;
					y++;
					if(y>t_lines-2)
						break;
				}
				if(from==to){
					if(from==0)
						empty=1;
					else{
						from-=(BBS_PAGESIZE-1);
						if(from<0)from=0;
						no=from;
						continue;
					}
				}
				if(!empty && no>to-1)
					no=to-1;
				close(fd);
			}
			if(empty)
				prints("(ÎÞÄÚÈÝ)\n");
			update_endline();
		}
		if(!empty){
			move(3+no-from,0);
			prints(">");
		}
		ch = egetch();
		redrawflag=0;
		if(!empty){
			move(3+no-from,0);
			prints(" ");
		}
		switch(ch)	{
		case KEY_UP:
			if(empty)
				break;
			no--;
			if(no<from)	{
				from-=(BBS_PAGESIZE-1);
				if(from<0)from=0;
				if(no<from)no=from;
				redrawflag=1;
			}
			break;
		case KEY_DOWN:
			if(empty)
				break;
			if(no<to-1)	{
				no++;
				if(no==(from+BBS_PAGESIZE-1)){
					from+=(BBS_PAGESIZE-1);
					no=from;
					redrawflag=1;
				}
			}
			break;
		case Ctrl('B'):
		case KEY_PGUP:
			if(empty)
				break;
			from-=(BBS_PAGESIZE-1);
			no-=(BBS_PAGESIZE-1);
			if(from<0)
				from=0;
			if(no<from)
				no=from;
			redrawflag=1;
			break;
		case Ctrl('F'):
		case KEY_PGDN:
			if(empty)
				break;
			if(to-from>=BBS_PAGESIZE){
				from+=(BBS_PAGESIZE-1);
				no+=(BBS_PAGESIZE-1);
				redrawflag=1;
			}
			else 
				no=to-1;
			break;
		case '/':	//²éÕÒ×Ö·û´®
			{
				int oldno;
				if(empty)break;
				getdata(1,0,"²éÕÒ:", line, 50, DOECHO,YEA);
				redrawflag=1;
				oldno=no;
				no++;
				no=text_find(textfile,no,line,line);
				if(no<0)
					no=oldno;
				else 
					from=no-no%(BBS_PAGESIZE-1);
			}
			break;
		case KEY_LEFT:
		case KEY_ESC:
		case '\n':
			return;
		default:
			if(key_deal){
				if(empty||(fd=open(textfile,O_RDONLY))==-1)	{
					redrawflag=(*key_deal)(textfile, ch, NULL);
				}else	{
					seek_nth_line(fd,no);
					readln(fd, genbuf);
					//strtok(genbuf,"\n");
					strcpy(line,genbuf);
					redrawflag=(*key_deal)(textfile, ch, line);
					if(fd!=-1)close(fd);
				}
			}
		}// switch
	}//while (1)
	return ;
}

//modified by iamfat 2002.07.20
//ÒÔÏÂÊÇreasonº¯Êý
char* def_reasonlist="-------------------------------";
char* reasonlist    ="ABCDEFGHIJKLMNOPQRSTUVWXYZ;'.[]";
int reasoncount=30;
char reason[50];	//·â½ûÔ­Òò È«¾Ö±äÁ¿
char detailreason[4096];
char reason_detail[STRLEN];	//Ô­ÒòÎÄ¼þ
char reason_suggestion[STRLEN];	//Ô­ÒòÎÄ¼þ
char deny_uid[IDLEN+1];
char club_uid[IDLEN+1];

//	ÉèÖÃÄ¬ÈÏ·â½ûÔ­Òò
void setreasondefault()
{
	strcpy(reason,def_reasonlist);
}
//ÉèÖÃÔ­Òò,½«²ÎÊý´«¹ýÀ´µÄ×Ö·û´®¿½±´µ½reason×Ö·û´®ÖÐÈ¥
void setreason(char* rsn, int i)
{
	strncpy(reason,rsn,i);
	reason[i]='\0';
}
//	ÉèÖÃ·â½ûÊ±µÄ±êÌâ
void reason_title_show()
{
	move(0,0);
	prints("[1;44;36m                                    ·â½ûÔ­ÒòÁÐ±í                               [m\n");
	prints(" ·â½û%sµÄÔ­Òò:%s\n", deny_uid, reason);
	prints("[1;44m ·â½ûµÄÔ­Òò¼°´¦·£½¨Òé                                                          [m\n");
}
//
int reason_key_deal(char* fname, int ch, char* line)
{
	char flag;
	int no=0;
	if(!line)return 0;
	ch=toupper(ch);
	if(strchr(reasonlist,ch)){
		while(no<reasoncount && reasonlist[no])	{
			if(reasonlist[no]==ch)	{
				reason[no]=(reason[no]=='-')?reasonlist[no]:'-';
				break;
			}
			no++;
		}
	}else if(ch==' '){
		flag=line[0];
		while(reasonlist[no]!=flag)
			no++;
		reason[no]=(reason[no]=='-')?reasonlist[no]:'-';
	}
	return 1;
}
//¼ì²éline×Ö·û´®Ê××ÖÄ¸ÊÇ·ñÔÚreasonÖÐ,ÊÇ·µ»Ø1,·ñÔò0
int reason_check(char* line)
{
	if(strchr(reason,line[0]))
		return 1;
	return 0;
}
int	getreasoncount()
{
	FILE *fp;
	char line[256];
	if(fp=fopen(reason_suggestion,"r"))	{
		reasoncount=0;
		while(fgets(line,256,fp) && strchr(reasonlist,line[0]))
			reasoncount++;
		fclose(fp);
	}
	return reasoncount;
}
void changereason(char* fname)
{
	sprintf(reason_suggestion,"%s.suggestion",fname);
	sprintf(reason_detail,"%s.detail",fname);
	reason[getreasoncount()]='\0';
	while(1){
		list_text(	reason_suggestion,
					reason_title_show,
					reason_key_deal, 
					reason_check
				 );
		if(strncmp(reason,def_reasonlist,reasoncount))
			break;
	}
}
char* getreason(){return reason;}
char* getdetailreason()
{
	FILE* fp;
	char line[256];
	char* dtlrsn=detailreason;
	if(fp=fopen(reason_detail,"r"))
	{
		while(fgets(line,256,fp))
		{
			if(strchr(reason,line[0]))
			{
				strcpy(dtlrsn, line+1);
				dtlrsn+=strlen(line+1);
				//*(dtlrsn++)='\n';
			}
		}
		fclose(fp);
	}
	*dtlrsn='\0';
	return detailreason;
}

//ÒÔÏÂÊÇ°æÃæ·â½ûÏà¹Øº¯Êý
int seekname(char *deny_uid)
{
	FILE* fp;
	char uident[IDLEN+1];
	char fname[STRLEN];
	char line[256];
    setbfile(fname, currboard, "deny_users");
	if(!(fp=fopen(fname,"r")))return 0;
	while(fgets(line,256,fp))
	{
		strncpy(uident, line, IDLEN);
		uident[IDLEN] = '\0';
		strtok(uident," \r\n\t");
		if(strcasecmp(deny_uid,uident)==0){
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int addtodeny(char *uident,char *msg,int ischange)
{
	char strtosave[512], buf[50], buf2[50];
	int day, seek;
	time_t nowtime;
	if(!strcmp(uident,"guest"))
	{
	  move(t_lines-1,0);
	  prints("ÄãÔÚ¸ãÐ¦Âð?·âguest?");
	  egetch();
	  return -1;
	} 
	seek = seekname(uident);
	if(seek && !ischange){
	  move(1, 0);
	  prints(" %s ÒÑ¾­ÔÚ·â½ûÃûµ¥ÖÐ¡£", uident);
	  egetch();
	  return -1;
	}   
	if (ischange && !seek) {
	  move(1,0);
	  prints(" %s ²»ÔÚ·â½ûÃûµ¥ÖÐ¡£", uident);
	  egetch();
	  return -1;
	}
	if(!ischange)setreasondefault();
	changereason("etc/denyboard");
	getdata(1,0,"ÊäÈë²¹³äËµÃ÷:", buf, 50, DOECHO,YEA);
	getdata(1,0,"ÊäÈëÌìÊý(Ä¬ÈÏ1Ìì):", buf2, 4, DOECHO,YEA);
	day=atoi(buf2);
	day=(day>90)?90:day;
	day=(day<1)?1:day;
    move(1, 0);
    if (ischange){
		sprintf(strtosave,"¸Ä±ä¶Ô%sµÄ·â½ûÂð?",uident);
    } else{
		sprintf(strtosave,"ÕæµÄ·â½û%sÂð?",uident);
    }
    if(askyn(strtosave,NA,NA)==NA)return -1;
    nowtime=time(0);
    struct tm* tmtime;
    time_t daytime=nowtime+(day)*24*60*60;
    tmtime=localtime(&daytime);
    sprintf(strtosave, "%-12s %-31s %2dÌì %02dÄê%02dÔÂ%02dÈÕ½â %-12s",
	    uident, reason , day, tmtime->tm_year%100, tmtime->tm_mon+1,tmtime->tm_mday,currentuser.userid);
    sprintf(msg, "%sÒò:\n%s\nÓ¦±»·â½û%s°æ·¢ÎÄÈ¨ÏÞ%dÌì\nÇëÔÚ´¦·£ÆÚÂúºó(%04d.%02d.%02d), Ïò%sÐ´ÐÅÒªÇó½â³ý´¦·£.\nÈç²»·þ±¾¾ö¶¨, ¿ÉÒÔÁªÏµ´¦·£¾ö¶¨ÈË»òÔÚ7ÈÕÄÚµ½AppealÉêÇë¸´Òé¡£\nP.S.: %s\nÖ´ÐÐÈË: %s\n", 
	    uident, getdetailreason(),currboard,day,1900+tmtime->tm_year, tmtime->tm_mon+1,tmtime->tm_mday,currentuser.userid,buf,currentuser.userid);
    if (ischange) deldeny(uident);
    setbfile( genbuf, currboard,"deny_users" );
	bm_log(currentuser.userid, currboard, BMLOG_DENYPOST, 1);
    return add_to_file(genbuf,strtosave);
}

int deldeny(uident)
char   *uident;
{
	char    fn[STRLEN];
	setbfile(fn, currboard, "deny_users");
	bm_log(currentuser.userid, currboard, BMLOG_UNDENY, 1);
	return del_from_file(fn, uident);
}

void deny_title_show()
{
	move(0,0);
	prints("[1;44;36m Éè¶¨ÎÞ·¨·¢ÎÄµÄÃûµ¥                                                            [m\n");
	prints(" Àë¿ª[[1;32m¡û[m] Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ý[m] Ìí¼Ó[[1;32ma[m] ÐÞ¸Ä[[1;32mc[m] ½â·â[[1;32md[m] ²éÕÒ[[1;32m/[m]\n");
	prints("[1;44m ÓÃ»§´úºÅ     ·â½ûÔ­Òò(A-Z,;'[])              ÌìÊý    ½â·âÈÕÆÚ       °æÖ÷      [m\n");
}

int deny_key_deal(char* fname, int ch, char* line)
{
    char msgbuf[4096];
    char repbuf[500];
	int tmp;
	if(line)
	{
		strncpy(deny_uid, line, IDLEN);
		deny_uid[IDLEN] = '\0';
		strtok(deny_uid, " \n\r\t");
		tmp=strlen(reasonlist);
		strncpy(reason,line+IDLEN+1,tmp);
		reason[tmp]='\0';
		while(tmp--)
		{
			if(reason[tmp]!=reasonlist[tmp])reason[tmp]='-';
		}
	}
	switch(ch)
	{
	case 'a':	//·âÈË
		move(1,0);
		usercomplete("·â½ûÊ¹ÓÃÕß: ", deny_uid);
		if(*deny_uid!='\0' && getuser(deny_uid))
		{
			if(!strcmp(deny_uid,currentuser.userid))
			{
				move(1,0);
				prints("ft! ·â×Ô¼ºÍæ!!!??? NO WAY! :P");
				egetch();
				break;
			}
			if(addtodeny(deny_uid,msgbuf,0)==1)
			{
				sprintf(repbuf, "·â½û%sÔÚ%s°æµÄ·¢ÎÄÈ¨ÏÞ", deny_uid, currboard);
				//autoreport(repbuf,msgbuf,YEA,deny_uid); //infotech
				autoreport(repbuf,msgbuf,YEA,deny_uid,0);
				Poststring(msgbuf,"Notice",repbuf,1);
				sprintf(repbuf, "±»%s·â½ûÔÚ%s°æµÄ·¢ÎÄÈ¨ÏÞ", currentuser.userid, currboard);
				log_DOTFILE(deny_uid, repbuf);
			}
		}
		break;
	case 'd':	//½â·â
		if(!line)return 0;
		move(1, 0);
		sprintf(msgbuf,"½â³ý%sµÄ·â½ûÂð?",deny_uid);
	    if(askyn(msgbuf,NA,NA)==NA)return 1;
	    if (deldeny(deny_uid)) {
	       sprintf(repbuf, "»Ö¸´%sÔÚ%s°æµÄ·¢ÎÄÈ¨ÏÞ",deny_uid, currboard);
	       sprintf(msgbuf, "%s»Ö¸´%sÔÚ%s°æ·¢ÎÄÈ¨ÏÞ.\n",  currentuser.userid, deny_uid, currboard);
	       autoreport(repbuf,msgbuf,YEA,deny_uid,0);
	       //autoreport(repbuf,msgbuf,YEA,deny_uid);
	       Poststring(msgbuf,"Notice",repbuf,1);
               sprintf(repbuf, "±»%s»Ö¸´ÔÚ%s°æµÄ·¢ÎÄÈ¨ÏÞ", currentuser.userid, currboard);
               log_DOTFILE(deny_uid, repbuf);
	    }
		break;
	case 'c':	//ÐÞ¸ÄÈÕÆÚ
		if(!line)return 0;
		if(addtodeny(deny_uid,msgbuf,1)==1)
		{
			sprintf(repbuf,"ÐÞ¸Ä%sÔÚ%s°æµÄ·â½ûÊ±¼ä»òËµÃ÷",deny_uid,currboard);
			autoreport(repbuf,msgbuf,YEA,deny_uid,0);
			//autoreport(repbuf,msgbuf,YEA,deny_uid);
			Poststring(msgbuf,"Notice",repbuf,1);
		}
		break;
	case Ctrl('A'):
	case KEY_RIGHT:	//ÓÃ»§ÐÅÏ¢
		if(!line)return 0;
		t_query(deny_uid);
		break;
	}
	return 1;
}

int deny_user()
{  
	if (!chk_currBM(currBM, 0)) 
   		return DONOTHING;
	setbfile(genbuf, currboard, "deny_users");
	list_text(genbuf,deny_title_show,deny_key_deal, NULL);
	return FULLUPDATE;
}

int isclubmember(char *member, char *board)
{
	FILE* fp;
	char uident[IDLEN+1];
	char fname[STRLEN];
	char line[256];
	setbfile(fname, board, "club_users");
	if(!(fp=fopen(fname,"r")))
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


int addtoclub(char *uident,char *msg)
{
	char strtosave[512], buf[50];
	int seek;
	if(!strcmp(uident,"guest"))
	{
		move(t_lines-1,0);
		prints("²»ÄÜÑûÇëguest¼ÓÈë¾ãÀÖ²¿");
		egetch();
		return -1;
	} 
	seek = isclubmember(uident, currboard);
	if(seek){
		move(1, 0);
		prints(" %s ÒÑ¾­ÔÚ¾ãÀÖ²¿Ãûµ¥ÖÐ¡£", uident);
		egetch();
		return -1;
	}   
	getdata(1,0,"ÊäÈë²¹³äËµÃ÷:", buf, 50, DOECHO,YEA);
	move(1, 0);
	sprintf(strtosave,"ÑûÇë%s¼ÓÈë¾ãÀÖ²¿Âð?",uident);
	if(askyn(strtosave,YEA,NA)==NA)return -1;
	time_t daytime= time(0);
	struct tm* tmtime=localtime(&daytime);
	sprintf(strtosave, "%-12s %-40s %04d.%02d.%02d %-12s",
		uident,buf, 1900+tmtime->tm_year, tmtime->tm_mon+1,tmtime->tm_mday, currentuser.userid);
	
	sprintf(msg, "%s:\n\n    Äú±»ÑûÇë¼ÓÈë¾ãÀÖ²¿°æ %s\n\n²¹³äËµÃ÷£º%s\n\nÑûÇëÈË: %s\n",
		uident, currboard, buf, currentuser.userid);
	setbfile( genbuf, currboard,"club_users" );
	bm_log(currentuser.userid, currboard, BMLOG_ADDCLUB, 1);
	return add_to_file(genbuf,strtosave);
}

int delclub(char   *uident)
{
	char    fn[STRLEN];
	setbfile(fn, currboard, "club_users");
	bm_log(currentuser.userid, currboard, BMLOG_DELCLUB, 1);
	return del_from_file(fn, uident);
}

void club_title_show()
{
	move(0,0);
	prints("[1;44;36m Éè¶¨¾ãÀÖ²¿µÄÃûµ¥                                                               [m\n");
	prints("Àë¿ª[[1;32m¡û[m] Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ý[m] Ìí¼Ó[[1;32ma[m] É¾³ý[[1;32md[m] ²éÕÒ[[1;32m/[m]\n");
	prints("[1;44mÓÃ»§´úºÅ               ¸½¼ÓËµÃ÷                         ÑûÇëÈÕÆÚ     ÑûÇëÈË     [m\n");
}

int club_key_deal(char* fname, int ch, char* line)
{
	char msgbuf[4096];
	char repbuf[500];
	if(line)
	{
		strncpy(club_uid, line, IDLEN);
		club_uid[IDLEN] = '\0';
		strtok(club_uid, " \n\r\t");
	}
	
	switch(ch)
	{
		case 'a':	//Ôö¼Ó
			move(1,0);
			usercomplete("Ôö¼Ó¾ãÀÖ²¿³ÉÔ±: ", club_uid);
			if(*club_uid!='\0' && getuser(club_uid))
			{
				if(addtoclub(club_uid,msgbuf)==1)
				{
					sprintf(repbuf, "%sÑûÇë%s¼ÓÈë¾ãÀÖ²¿°æ%s", currentuser.userid, club_uid, currboard);
					autoreport(repbuf,msgbuf,YEA,club_uid, 2);
					Poststring(msgbuf,"club",repbuf,2);
					log_DOTFILE(club_uid, repbuf);
				}
			}
			break;
		case 'd':	//É¾³ý³ÉÔ±
			if(!line)return 0;
			move(1, 0);
			sprintf(msgbuf,"É¾³ý¾ãÀÖ²¿³ÉÔ±%sÂð?",club_uid);
			if(askyn(msgbuf,NA,NA)==NA)return 1;
			if (delclub(club_uid)) {
				sprintf(repbuf, "%sÈ¡Ïû%sÔÚ¾ãÀÖ²¿°æ%sµÄÈ¨Àû",currentuser.userid, club_uid, currboard);
				sprintf(msgbuf,"");
				autoreport(repbuf,msgbuf,YEA,club_uid, 2);
				Poststring(msgbuf,"club",repbuf,2);
				log_DOTFILE(club_uid, repbuf);
			}
			break;
		
		case Ctrl('A'):
		case KEY_RIGHT: //ÓÃ»§ÐÅÏ¢
			if(!line)return 0;
			t_query(club_uid);
			break;											     
/*		case 'c':    //Çå¿Õ³ÉÔ±£¬Ã»·¨¸øËùÓÐ³ÉÔ±·¢ÐÅ
			move(1,0);
			sprintf(msgbuf,"[1;31mÇå¿Õ[m¾ãÀÖ²¿³ÉÔ±Âð?");
			if(askyn(msgbuf,NA,NA)==NA)return 1;
			setbfile( genbuf, currboard,"club_users" );
			sprintf(msgbuf,"%s Çå¿Õ¾ãÀÖ²¿ %s µÄ³ÉÔ±",  currentuser.userid, currboard);
			Poststring(msgbuf,club",repbuf,2);
			unlink(genbuf);
			break;
			*/
			
	}
	return 1;
}

int club_user()
{
	struct boardheader *bp;
	extern struct boardheader *getbcache();
	bp = getbcache(currboard);
			    
	if ((bp->flag & BOARD_CLUB_FLAG) && chk_currBM(currBM, 1)){
		setbfile(genbuf, currboard, "club_users");
		list_text(genbuf,club_title_show, club_key_deal, NULL);
		return FULLUPDATE;
	}
	else
		return DONOTHING;
}



int bm_log(char *id, char *boardname, int type, int value)
{
	int fd, data[BMLOGLEN];
	struct flock ldata;
	struct stat buf;
	struct boardheader *btemp;
	char direct[STRLEN], BM[BM_LEN];
	char *ptr;

	btemp = getbcache(boardname);
	if (btemp == NULL)
		return 0;
	strncpy(BM, btemp->BM, sizeof(BM) - 1);
	BM[sizeof(BM) - 1] = '\0';
	ptr = strtok (BM, ",: ;|&()\0\n");
	while(ptr)
	{
		if (!strcmp(ptr, currentuser.userid)) break;
		ptr = strtok (NULL, ",: ;|&()\0\n");
	}
	if (!ptr)
		return 0;
	sprintf(direct, "boards/%s/.bm.%s", boardname, id);
	if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1)
		return 0;
	ldata.l_type = F_RDLCK;
	ldata.l_whence = 0;
	ldata.l_len = 0;
	ldata.l_start = 0;
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		close(fd);
		return 0;
	}
	fstat(fd, &buf);
	if (buf.st_size < BMLOGLEN * sizeof(int)) {
		memset(data, 0, sizeof(int) * BMLOGLEN);
	} else{
		read(fd, data, sizeof(int) * BMLOGLEN);
	}
	if (type >= 0 && type < BMLOGLEN)
		data[type] += value;
	lseek(fd, 0, SEEK_SET);
	write(fd, data, sizeof(int) * BMLOGLEN);
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &ldata);
	close(fd);
	return 0;
}

