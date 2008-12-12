#include "BBSLIB.inc"
#include "math.h"

time_t get_last_logout(struct userec * );

int count_life_value(struct userec *urec) {
    int value, value2;
    value = (time(0) - urec->lastlogin);
    value2 = (time(0) - urec->firstlogin); //....
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


int main() {
	FILE *fp;
	char userid[14], filename[80], buf[512];
	struct userec *x;
	struct user_info *u;
	int i, tmp1, tmp2, num, clr;
	init_all();
	strsncpy(userid, getparm("userid"), 13);
	//printf("%s",userid);
	printf("<b>≤È—ØÕ¯”— °§ %s </b>\n", BBSNAME);
//	if(userid[0]==0) {
		printf("<br><form action=bbsqry>\n");
		printf("«Î ‰»Î”√ªß√˚: <input name=userid maxlength=12 size=12>\n");
		printf("<input type=submit value=≤È—Ø”√ªß>\n");
		printf("</form>\n");
	if(userid[0]==0){
		http_quit();
	}
/* added by roly */
	if(!strcmp(userid,"≥œ’˜∞Ê÷˜÷–")) {
		printf("faint£¨’‚∏ˆ∂º“™≤È£ø");
		http_quit();
	}
/* added end */
	x=getuser(userid);
	if(!loginok || x==0) {
		printf("”√ªß [%s] ≤ª¥Ê‘⁄.", userid);
		http_quit();
	}
	/*
	{
	char query_string[256];
	sprintf(query_string,"%s from %s", x->userid, fromhost);
	do_report("QUERY_LOG", query_string);
	}*/
printpretable();
printf("<table bgcolor=#ffffff>\n");

	printf("<pre class=ansi>\n");
	sprintf(buf, "%s ([33m%s[37m) π≤…œ’æ [32m%d[m ¥Œ£¨∑¢±ÌŒƒ’¬ [32m%d[m ∆™", 
		x->userid, x->username, x->numlogins, x->numposts);
	hprintf("%s", buf);
	show_special(x->userid);
	printf("\n");
	if(x->userdefine & DEF_COLOREDSEX) {
                clr=(x->gender == 'F') ? 35 : 36;
        } else {
                clr=32;
	}
	if(x->userdefine & DEF_S_HOROSCOPE) hprintf("[[1;%dm%s[m]", clr, horoscope(x->birthmonth, x->birthday));
//	hprintf("…œ¥Œ‘⁄ [[32m%s[37m] ¥” [[32m%s[37m] µΩ±æ’æ“ª”Œ°£\n", Ctime(x->lastlogin), x->lasthost);
//modified by iamfat 2002.08.01
/*
	{
		int i;
		srand(time(0));
		i=(int) (10.0*rand()/(RAND_MAX+1.0));;
		while(i--)
		{
			printf("<!--…œ¥Œ‘⁄ [[32m%s[37m] ¥” [[32m%d.%d.%d.%d[37m] µΩ±æ’æ“ª”Œ°£\n-->", cn_Ctime(time(0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)), (int) (256.0*rand()/(RAND_MAX+1.0)));
		}
	}
	hprintf("…œ¥Œ‘⁄ [[32m%s[37m] ¥” [[32m%s[37m] µΩ±æ’æ“ª”Œ°£\n", cn_Ctime(x->lastlogin), x->lasthost);*/
	count_mails(userid, &tmp1, &tmp2);
	hprintf("–≈œ‰£∫[[32m%s[37m]£¨", tmp2 ? "°—":"  ");
/*
#ifdef SHOWEXP
	hprintf("æ≠—È÷µ£∫[[32m%d[37m]([33m%s[37m) ", countexp(x), cexp(countexp(x)));
#else
	hprintf("æ≠—È÷µ£∫[[33m%s[37m] ", cexp(countexp(x)));
#endif
*/
	printf("æ≠—È÷µ: [");
	iconexp(countexp(x));
	printf("]");
#ifdef SHOWPERF
	hprintf("±Ìœ÷÷µ£∫[[32m%d[37m]([33m%s[37m) ", countperf(x), cperf(countperf(x)));
#else
	hprintf("±Ìœ÷÷µ£∫[[33m%s[37m] ", cperf(countperf(x)));
#endif
	hprintf("…˙√¸¡¶£∫[[32m%d[37m]°£\n", count_life_value(x));
	num=0;
	for(i=0; i<MAXACTIVE; i++) {
		u=&(shm_utmp->uinfo[i]);
		if(!strcmp(u->userid, x->userid)) {
			if(u->active==0 || u->pid==0 || u->invisible && !HAS_PERM(PERM_SEECLOAK)) continue;
			num++;
			if(num==1) hprintf("ƒø«∞‘⁄’æ…œ, ◊¥Ã¨»Áœ¬:\n");
			if(u->invisible) hprintf("[36mC[37m");
			hprintf("[32m%s[m ", ModeType(u->mode));
			if(num%5==0) printf("\n");
		}
	}
	if(num==0) {
		sprintf(filename, "home/%c/%s", toupper(x->userid[0]), x->userid);
//		hprintf("ƒø«∞≤ª‘⁄’æ…œ, …œ¥Œ¿Î’æ ±º‰ [[1;32m%s[m]\n\n", Ctime(file_time(filename)));
//modified by iamfat 2002.08.01
/* 2002.09.22 modified by stephen to fix the web query last login time error and make it fit the telnet query. */
/*		hprintf("ƒø«∞≤ª‘⁄’æ…œ, …œ¥Œ¿Î’æ ±º‰ [[1;32m%s[m]\n\n", cn_Ctime(file_time(filename)));
*/
	hprintf("ƒø«∞≤ª‘⁄’æ…œ, …œ¥Œ¿Î’æ ±º‰ [[1;32m%s[m]\n\n", cn_Ctime( get_last_logout(x) ));
/* 2002.09.22 modified end*/
	}
	printf("\n");
	sprintf(filename, "home/%c/%s/plans", toupper(x->userid[0]), x->userid);
	fp=fopen(filename, "r");
	if(fp) {
		while(1) {
			if(fgets(buf, 256, fp)==0) break;
			hhprintf(buf);
		}
		fclose(fp);
	} else {
		hprintf("[36m√ª”–∏ˆ»ÀÀµ√˜µµ[37m\n");
	}
	printf("</pre>");
//	printf("<br><br>\n");
printf("</table>");

printposttable();

	printf("<p align=center><a href=bbspstmail?userid=%s&title=√ª÷˜Ã‚>[–¥–≈Œ ∫Ú]</a>  ", x->userid);
	printf("<a href=bbssendmsg?destid=%s>[∑¢ÀÕ—∂œ¢]</a>  ", x->userid);
	printf("<a href=bbsfadd?userid=%s>[º”»Î∫√”—]</a>  ", x->userid);
	printf("<a href=bbsfdel?userid=%s>[…æ≥˝∫√”—]</a>  ", x->userid);
	printf("</p>\n");
	http_quit();
}

int show_special(char *id2) {
        FILE *fp;
        char  id1[80], name[80];
        fp=fopen("etc/sysops", "r");
        if(fp==0) return;
        while(1) {
                id1[0]=0;
                name[0]=0;
                if(fscanf(fp, "%s %s", id1, name)<=0) break;
                if(!strcmp(id1, id2))
			hprintf(" [1;31m°Ô[0;36m%s[1;31m°Ô[m", name);
        }
        fclose(fp);
}

/*2002.09.25 add by stephen to keep the web query the same as the telnet query use the same arithmetic,
*/
time_t get_last_logout(struct userec *x)
{
	time_t	now;

	if(x->lastlogout < x->lastlogin){
         now = ((time(0)-x->lastlogin)/120)%47+1+x->lastlogin;
       
      	}
         else if(x->lastlogout > x->lastlogin + 3600*24){
	  now = ((time(0)-x->lastlogin)/120)%47+1+x->lastlogin;         	
        } 
         else  {
         now = x->lastlogout;
		}
	return(now);
}
/*2002.09.25 add end*/
