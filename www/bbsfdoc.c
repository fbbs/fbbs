#include "libweb.h"

char *getsize(char *board)
{
        int now=0;
	static char str[20];
        char path[256], cmd[256];
        sprintf(path, "%s/upload/%s", BBSHOME, board);
        if(dashd(path))
        {
                FILE *fp;
                sprintf(cmd, "%s/.size", path);
                fp=fopen(cmd, "r");
                if(fp)
		{
			fscanf(fp, "%d", &now);
                	fclose(fp);
			sprintf(str,"%dK", now);
			return str;
		}
	}
	strcpy(str, "未知");
	return str;
}

char* getall(char *board)
{
	int all=-1;
	static char str[20];
	char path[256], cmd[256];
	sprintf(path, "%s/upload/%s", BBSHOME, board);
	if(dashd(path))
        {
		FILE *fp;
		
                sprintf(cmd, "%s/.quota", path);
                fp=fopen(cmd, "r");
                if(fp)
		{
                	fscanf(fp, "%d", &all);
	                fclose(fp);
			sprintf(str,"%dK", all);
			return str;
		}	
        }
	strcpy(str, "未知");
        return str;
}


int main() 
{
	FILE *fp;
	char *ptr, board[256],dir[100];
	struct dir x[30000], xx;
	int i, start, total=0;
	init_all();
	strlcpy(board, getparm("board"), 30);
	if(!has_read_perm(&currentuser, board)) 
		http_fatal("错误的讨论区");
	sprintf(dir,"%s/upload/%s/.DIR",BBSHOME,board);
	fp=fopen(dir, "rb");
	if(fp==0) 
		http_fatal("没有找到文件");
	while(total<30000) 
	{
		if(fread(&xx, sizeof(xx), 1, fp)<=0) 
			break;
		if(!xx.reid)continue;
//		if(strcasecmp(xx.board, board))continue;
		x[total]=xx;
		total++;
	}
	fclose(fp);
	ptr=getparm("start");
	if(ptr[0]==0) 
	{
		start=total-19;
	} else {
		start=strtol(ptr,NULL,10);
	}
	if(start>total-19) 
		start=total-19;
	start = start - 1;
	if(start<0) 
		start=0;
	printf("<b><font style='font-size: 18pt'>%s</font> · %s 上传文件区</b> 目前容量[%s] 已用[%s]<br>\n", board, BBSNAME, getall(board), getsize(board));
	printf("<td width=15%% align=right>\n");
	printf("<form name=form1 action=bbsfdoc?board=%s method=post>", board);
	printf("<input border=0 src=/images/button/forward.gif type=image align=absmiddle> 第 <input class=thinborder type=text name=start size=4> 篇");
	 printf("</form></td>\n");
	printf("<style>                                        \n");
	printf("div.frame {height: 450px;}                      \n");
	printf("img.frame { height:auto; width:250px; border:0px;}          \n");
	printf("</style>                                       \n");
	printf("<script lang='Javascript'>			\n");
	printf("function preview(mysrc)				\n");
	printf("{						\n");
	printf("	document.myimg.src=mysrc;	return;	\n");
	printf("}						\n");
	printf("</script>					\n");
	printpretable();
	printf("<table width=100%% border=0>");
	printf("<tr valign=top><td><table bgcolor=#ffffff width=100%%>");
	printf("<tr class=pt9h><td>序号</td><td>文件名</td><td>上传者</td><td>时间</td><td>大小</td><td>管理</td><td>预览</td></tr>\n");
	for(i=start; i<start+20 && i<total; i++) 
	{
		char url_filename[256];
		strtourl(url_filename,x[i].filename);
		printf("<tr class=%s>\n",((i-start)%2)?"pt9dc":"pt9lc");
		printf("<td>%d</td>\n", i+1);
		printf("<td><a href='/upload/%s/%s' target=_blank >%s</a></td>\n",nohtml(board),url_filename,nohtml(x[i].filename));
		printf("<td>%s</td>\n", userid_str(x[i].owner));
		printf("<td>%6.6s</td>\n", Ctime(x[i].timeDeleted)+4);
		{
			float my_size=x[i].id;
			char sizestr[10];
			if(my_size>1024)
			{
				my_size=my_size/1024;
				if(my_size>1024)
				{
					my_size=my_size/1024;
					sprintf(sizestr,"%-6.2fMB",my_size);
				}else{
					sprintf(sizestr,"%-4.2fKB",my_size);
				}
				
			}else{
				sprintf(sizestr,"%dB",(int)(my_size));
			}
			printf("<td>%s</td>\n", sizestr);
		}
		printf("<td><a onclick='return confirm(\"确定删除吗?\")' href=bbsdelf?board=%s&file=%s&start=%d>删除</a></td>\n", board, url_filename, start);
		printf("<td>");
		{
			if(strstr(x[i].filename, ".gif") || strstr(x[i].filename, ".jpg") || strstr(x[i].filename, ".jpeg") || strstr(x[i].filename, ".png") || strstr(x[i].filename, ".bmp")
			|| strstr(x[i].filename, ".GIF") || strstr(x[i].filename, ".JPG") || strstr(x[i].filename, ".JPEG") || strstr(x[i].filename, ".PNG") || strstr(x[i].filename, ".BMP")
			)
			{
				printf("<a href='#' onclick='preview(\"/upload/%s/%s\")'><img border=0 src='/upload/preview.gif'></a>",board,url_filename);	
			}else{
				printf(" ");
			}	
		}
		printf("</td>\n");
		printf("</tr>\n");
	}
	printf("</table>\n");
	printf("</td><td width=250><center>");
	printf("<div class=frame><img name=myimg class=frame src='/upload/blank.gif'>");
	printf("</center></td></tr></table>");
	printposttable();
	printf("<a href='javascript:location=location'><img border=0 src=/images/button/reload.gif align=absmiddle>刷新</a> ");
	printf("<a href=bbsdoc?board=%s>本讨论区</a> ",board);
//	printf("<a href=bbspreupload?board=%s>上传文件</a> ", board);
	if(start>0) 
		printf("<a href=bbsfdoc?board=%s&start=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一页</a> ", board, start-19);
	if(start<total-20) 
		printf("<a href=bbsfdoc?board=%s&start=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一页</a> ", board, start+19);
	
	http_quit();
}
