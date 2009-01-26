#include "libweb.h"

int main() {
	int i, total=0, mode;
	char board[80], *ptr;
	struct boardheader *brd;
	init_all();
	printf("<b>管理模式 · %s </b><br>\n",BBSNAME);
	printpretable_lite();

	if(!loginok) http_fatal("请先登录");
	strlcpy(board, getparm("board"), 60);
	mode=atoi(getparm("mode"));
	brd=getbcache(board);
	if(brd==0) http_fatal("错误的讨论区");
	strcpy(board, brd->filename);
	if(!has_BM_perm(&currentuser, board)) http_fatal("您无权访问本页");
	if(mode<=0 || mode>5) http_fatal("错误的参数");
	printf("<table>");
	for(i=0; i<parm_num && i<40; i++) {
		if(!strncmp(parm_name[i], "box", 3)) {
			total++;
			if(mode==1) do_del(board, parm_name[i]+3);
			if(mode==2) do_set(board, parm_name[i]+3, FILE_MARKED);
			if(mode==3) do_set(board, parm_name[i]+3, FILE_DIGEST);
            /* added by roly */
            if(mode==4) do_set(board, parm_name[i]+3, FILE_NOREPLY);
            /* add end */
			if(mode==5) do_set(board, parm_name[i]+3, 0);
		}
	}
	printf("</table>");
	if(total<=0) printf("请先选定文章<br>\n");
	else if(mode==1) updatelastpost(board);
	printf("<br><a href=bbsmdoc?board=%s>返回管理模式</a>", board);
	http_quit();
}

int do_del(char *board, char *file) {
	FILE *fp;
	int num=0;
	char path[256], buf[256], dir[256], *id=currentuser.userid;
	struct fileheader f;
	struct userec *u;
	struct boardheader *brd=getbcache(board);
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("错误的参数");
	while(1) {
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0) break;
		//added by iamfat 2002.08.10
		//check_anonymous(f.owner);
		//added end.
		if(!strcmp(f.filename, file)) {
                        del_record(dir, sizeof(struct fileheader), num);
//                      sprintf(buf, "\n※ %s 于 %s 删除[FROM: %s]", currentuser.userid, Ctime(time(0))+4, fromhost);
//modified by iamfat 2002.08.01
//                      sprintf(buf, "\n※ %s 于 %16.16s 删除[FROM: %s]", currentuser.userid, cn_Ctime(time(0))+6, fromhost);
//                      f_append(path, buf);
                        sprintf(dir, strcmp(id, f.owner)?"boards/%s/.TRASH":"boards/%s/.JUNK", board);
                        strcpy(f.szEraser, id);
                        f.timeDeleted=time(0);
						append_record(dir, &f, sizeof(f));
 			printf("<tr><td>%s  <td>标题:%s <td>删除成功.\n", f.owner, nohtml(f.title));
			u=getuser(f.owner);
			if(!junkboard(getbcache(board)) && u) {
				if(u->numposts>0) u->numposts--;
				save_user_data(u);
			}
			sprintf(buf, "Del %s\n", board);
			trace(buf);
			fclose(fp);
			return;
		}
		num++;
	}
	fclose(fp);
	printf("<tr><td><td>%s<td>文件不存在.\n", file);
}

int do_set(char *board, char *file, int flag) {
	FILE *fp;
	char path[256], dir[256];
	struct fileheader f;
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(dir, "r+");
	if(fp==0) http_fatal("错误的参数");
	while(1) {
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0) break;
		if(!strcmp(f.filename, file)) {
			f.accessed[0]|=flag;
			if(flag==0) f.accessed[0]=0;
			fseek(fp, -1*sizeof(struct fileheader), SEEK_CUR);
			fwrite(&f, sizeof(struct fileheader), 1, fp);
			fclose(fp);
			printf("<tr><td>%s<td>标题:%s<td>标记成功.\n", f.owner, nohtml(f.title));
			return;
		}
	}
	fclose(fp);
	printf("<td><td><td>%s<td>文件不存在.\n", file);
}
