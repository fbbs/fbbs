#include "BBSLIB.inc"

char origin[512];	//added by iamfat 2002.08.17

int main() {
   	FILE *fp;
   	int type=0, i, num;
	char userid[80], buf[512], path[512], file[512], board[512], title[80]="";
   	struct boardheader *brd;
   	struct fileheader  *x;
	
	//added by iamfat 2002.08.17
    sprintf(origin, ":¡¤%s %s¡¤", BBSNAME, BBSHOST);
	//added end

	init_all();
	if(!loginok) 
	{
		printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("´Ò´Ò¹ı¿Í²»ÄÜĞŞ¸ÄÎÄÕÂ£¬ÇëÏÈµÇÂ¼");
	}
	strsncpy(board, getparm("board"), 20);
	type=atoi(getparm("type"));
	brd=getbcache(board);
	if(brd==0) 
	{
		printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("´íÎóµÄÌÖÂÛÇø");
	}
	strcpy(board, brd->filename);
	strsncpy(file, getparm("file"), 32);
	if(!has_post_perm(&currentuser, board))
	{
		printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("´íÎóµÄÌÖÂÛÇø»òÕßÄúÎŞÈ¨ÔÚ´ËÌÖÂÛÇø·¢±íÎÄÕÂ");
	}
   	x=get_file_ent(board, file);
	if(strstr(file, "..") || strstr(file, "/")) 
	{
		printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("´íÎóµÄ²ÎÊı");
	}
	if(x==0) 
	{
		printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("´íÎóµÄ²ÎÊı");
	}
	if(strcmp(x->owner, currentuser.userid)&& !has_BM_perm(&currentuser, board)) 
	{
		printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s </b><br>\n", BBSNAME);
		printpretable_lite();
		http_fatal("ÄúÎŞÈ¨ĞŞ¸Ä´ËÎÄÕÂ");
	}
	printf("<b>ĞŞ¸ÄÎÄÕÂ ¡¤ %s [Ê¹ÓÃÕß: %s]</b><br>\n", BBSNAME, currentuser.userid);
	printf("<center>\n");
	printpretable_lite();
	if(type!=0)
	{
		//added by iamfat 2002.08.17
		sprintf(buf, "edited post '%s' on %s", x->title, board);
		trace(buf);
		//added end;
		return update_form(board, file);
	}
   	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<tr><td><form method=post action=bbsedit>\n");
   	printf("Ê¹ÓÃ±êÌâ: %s ÌÖÂÛÇø: %s<br>\n", nohtml(x->title), board);
   	printf("±¾ÎÄ×÷Õß£º%s<br>\n", currentuser.userid);
   	printf("<textarea name=text rows=20 cols=80 wrap=physicle>");
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("ÎÄ¼ş¶ªÊ§");
	//added by iamfat 2002.08.17 Ìø¹ıHeader
	while(fgets(buf, 500, fp) && buf[0]!='\n');
	while(fgets(buf, 500, fp))
	{
		if(strstr(buf,origin) && buf[0]!=':')break;
		if(!strcasestr(buf, "</textarea>")) printf("%s", buf);
	}
	fclose(fp);
   	printf("</textarea>\n");
   	printf("<tr><td class=post align=center>\n");
	printf("<input type=hidden name=type value=1>\n");
	printf("<input type=hidden name=board value=%s>\n", board);
	printf("<input type=hidden name=file value=%s>\n", file);
	printf("<input type=submit value=´æÅÌ> \n");
   	printf("<input type=reset value=ÖØÖÃ></form>\n");
	printf("</table>");
	http_quit();
}

/*
int update_form(char *board, char *file) {
	FILE *fp;
	char *buf=getparm("text"), path[80];
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "w");
	if(fp==0) http_fatal("ÎŞ·¨´æÅÌ");
	fprintf(fp, "%s", buf);
//	fprintf(fp, "\n¡ù ĞŞ¸Ä:£®%s ì¶ %s ĞŞ¸Ä±¾ÎÄ£®[FROM: %s] ", currentuser.userid, Ctime(time(0))+4, fromhost);
//modified by iamfat 2002.08.01
	fprintf(fp, "[m[1;36m¡ù ĞŞ¸Ä:¡¤%s ì¶ %16.16s ĞŞ¸Ä±¾ÎÄ¡¤HTTP [FROM: %-.20s][m\n", currentuser.userid, cn_Ctime(time(0))+6, fromhost);
	fclose(fp);
	printf("ĞŞ¸ÄÎÄÕÂ³É¹¦.<br><a href=bbsdoc?board=%s>·µ»Ø±¾ÌÖÂÛÇø</a>", board);
}
*/
//rewrite by iamfat 2002.08.17
int update_form(char *board, char *file) {
	FILE *fpr, *fpw;
	char *buf=getparm("text"), path[80], pathnew[80];
	char buf2[512];
	int flag=0;
	sprintf(path, "boards/%s/%s", board, file);
	sprintf(pathnew, "boards/%s/%s.%d", board, file, u_info->pid);
	fpr=fopen(path, "r");
	fpw=fopen(pathnew, "w");
	if(fpw==0||fpr==0) http_fatal("ÎŞ·¨´æÅÌ");
	while(fgets(buf2, 500, fpr) && buf2[0]!='\n')fprintf(fpw,"%s",buf2);
	fprintf(fpw, "\n%s\n", buf);
	while(fgets(buf2,500,fpr)!=0)
	{
		if(strstr(buf2,origin) && buf2[0]!=':')flag=1;
		if(flag)
		{
			if(strncmp(buf2,"[m[1;36m¡ù ĞŞ¸Ä:¡¤",17))fprintf(fpw,"%s",buf2);
		}
	}
	fprintf(fpw, "[m[1;36m¡ù ĞŞ¸Ä:¡¤%s ì¶ %16.16s ĞŞ¸Ä±¾ÎÄ¡¤HTTP [FROM: %-.20s][m\n", currentuser.userid, cn_Ctime(time(0))+6, fromhost);
	fclose(fpw);
	fclose(fpr);
	rename(pathnew, path);
	printf("ĞŞ¸ÄÎÄÕÂ³É¹¦.<br><a href=bbsdoc?board=%s>·µ»Ø±¾ÌÖÂÛÇø</a>", board);
}
