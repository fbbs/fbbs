#include "libweb.h"

int main() {
	struct fileheader *x;
	char board[80], file[80], target[80];
	init_all();
	strlcpy(board, getparm("board"), 30);
	strlcpy(file, getparm("file"), 32);
	strlcpy(target, getparm("target"), 30);
	if(!loginok) 
	{
		printf("<b>×ªÔØÎÄÕÂ ¡¤ %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("´Ò´Ò¹ı¿Í²»ÄÜ½øĞĞ±¾Ïî²Ù×÷");
	}
	if(!has_read_perm(&currentuser, board)) 
	{
		printf("<b>×ªÔØÎÄÕÂ ¡¤ %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("´íÎóµÄÌÖÂÛÇø");
	}
	x=get_file_ent(board, file);
	if(x==0) 
	{
		printf("<b>×ªÔØÎÄÕÂ ¡¤ %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("´íÎóµÄÎÄ¼şÃû");
	}
	printf("<b>×ªÔØÎÄÕÂ ¡¤ %s [Ê¹ÓÃÕß: %s]</b><br>\n", BBSNAME, currentuser.userid);
	printpretable_lite();
	if(target[0]) {
		if(!has_post_perm(&currentuser, target)) http_fatal("´íÎóµÄÌÖÂÛÇøÃû³Æ»òÄúÃ»ÓĞÔÚ¸Ã°æ·¢ÎÄµÄÈ¨ÏŞ");
		return do_ccc(x, board, target);
	}
	printf("<table><tr><td>\n");
	printf("<font color=red>×ªÌù·¢ÎÄ×¢ÒâÊÂÏî:<br>\n");
	printf("±¾Õ¾¹æ¶¨Í¬ÑùÄÚÈİµÄÎÄÕÂÑÏ½ûÔÚ 4 ¸ö»ò 4 ¸öÒÔÉÏÌÖÂÛÇøÄÚÖØ¸´·¢±í¡£");
	printf("Î¥Õß½«±»·â½ûÔÚ±¾Õ¾·¢ÎÄµÄÈ¨Àû<br><br></font>\n");
	printf("ÎÄÕÂ±êÌâ: %s<br>\n", nohtml(x->title));
	printf("ÎÄÕÂ×÷Õß: %s<br>\n", x->owner);
	printf("Ô­ÌÖÂÛÇø: %s<br>\n", board);
	printf("<form action=bbsccc method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf("×ªÔØµ½ <input name=target size=30 maxlength=30> ÌÖÂÛÇø. ");
	printf("<input type=submit value=È·¶¨></form>");
}

int do_ccc(struct fileheader *x, char *board, char *board2) {
	FILE *fp, *fp2;
	struct boardheader *brc = NULL;
	brc = getbcache(board2);
	if (brc -> flag & BOARD_DIR_FLAG) {  //²»¿É×ªÔØµ½Ä¿Â¼ Danielfree 06.3.5
	        http_fatal("ÄãÑ¡ÔñÁËÒ»¸öÄ¿Â¼");
        }
	if ((brc->flag & BOARD_CLUB_FLAG)&& (brc->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, brc->filename)&& !isclubmember(currentuser.userid, brc->filename)) {
			http_fatal ("´íÎóµÄÌÖÂÛÇøÃû³Æ»òÄúÃ»ÓĞÔÚ¸Ã°æ·¢ÎÄµÄÈ¨ÏŞ");
	}
	char title[512], buf[512], path[200], path2[200], i;
	sprintf(path, "boards/%s/%s", board, x->filename);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("ÎÄ¼şÄÚÈİÒÑ¶ªÊ§, ÎŞ·¨×ªÔØ");
	sprintf(path2, "tmp/%d.tmp", getpid());
	fp2=fopen(path2, "w");
	for(i=0; i<3; i++)
		if(fgets(buf, 256, fp)==0) break;
	fprintf(fp2, "[37;1m¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô [32m%s [37mÌÖÂÛÇø ¡¿\n", board);
	fprintf(fp2, "[37;1m¡¾ Ô­ÎÄÓÉ [32m%s [37mËù·¢±í ¡¿[m\n\n", x->owner);
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		fprintf(fp2, "%s", buf);
	}
	fclose(fp);
	fclose(fp2);
	if((!strncmp(x->title, "[×ªÔØ]", 6))||
		(!strncmp(x->title, "Re: [×ªÔØ]", 10))){
		//modified by money 04.01.17 for judge Re & cross
		sprintf(title, x->title);
	} else {
		sprintf(title, "[×ªÔØ]%.55s", x->title);
	}
	post_article(board2, title, path2, currentuser.userid, currentuser.username, fromhost, -1, -1, -1);
	unlink(path2);
	printf("'%s' ÒÑ×ªÌùµ½ %s °æ.<br>\n", nohtml(title), board2);
	printf("[<a href='javascript:history.go(-2)'>·µ»Ø</a>]");
}
