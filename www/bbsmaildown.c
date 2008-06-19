#include "BBSLIB.inc"

int main() {
	struct user_doccount *udcount,*udcounthead;
	
	FILE *fp,*fpfile;
	struct fileheader *data;
	char buf[512];

	int type=0,docstart,docend,total,i,docmin;
	char dir[80],  board[80], userid[80];
	struct boardheader *brd;
	struct fileheader x;
	init_all();

	/* added by roly  2002.05.10 去掉cache */
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	printf("<title>信件下载</title><body>");
	/* add end */

	if(!loginok)
	{
		printf("<b>信件下载 · %s</b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("您尚未登录, 请先登录");
	}
	printf("<b>信件下载 · %s</b><br>\n", BBSNAME);
	type=atoi(getparm("type"));
	
	if(type==0) return show_form();	
	
    docstart=atoi(getparm("start"));
    docend=atoi(getparm("end"));	
	sprintf(dir, "mail/%c/%s/.DIR", toupper(currentuser.userid[0]), currentuser.userid);
   	total=file_size(dir)/sizeof(struct fileheader);
	
        
	if(docstart<=0) docstart=1;
	if(docend<=0 || docend>total) docend=total;
	
        
	//printf("docstart:%d,docend:%d",docstart,docend);

	data=(struct fileheader *)calloc(total, sizeof(struct fileheader));
   	if(data==0) 
		http_fatal("memory overflow");
	fp=fopen(dir, "r");
	if(fp==0) 
		http_fatal("dir error");
	total=fread(data, sizeof(struct fileheader), total, fp);
	printpretable_lite();
	if(fp)
	{
		int i,filetime;
		char path[STRLEN];
		printf("<ol>\n");
		for (i=docstart-1	;i<docend;i++) 
		{
			filetime=atoi(data[i].filename+2);   
			//added by iamfat 2002.08.10
			//check_anonymous(data[i].owner);
			//added end.
			printf("<LI><A href=#%s>%s:(%12.12s) %s</A>\n",data[i].filename,data[i].owner,Ctime(filetime)+4,data[i].title);
		}
		printf("</ol>\n");
		for (i=docstart-1;i<docend;i++) 
		{
			printf("<a name=%s></a>",data[i].filename); //,data[i].title);	
			printpretable();
			printf("<table width=100%% border=0>\n");
			printf("<pre class=ansi>");
			sprintf(path, "mail/%c/%s/%s", toupper(currentuser.userid[0]), currentuser.userid, data[i].filename);
			//printf(path);
			fpfile=fopen(path, "r");
			if(fpfile==0) 
			{
				printf("本文不存在或者已被删除！");
				printf("</pre></table>\n");
				printposttable();
				continue;
			}
			while(1) 
			{
				if(fgets(buf, 512, fpfile)==0) 
					break;
				hhprintf("%s", buf);
			}
			fclose(fpfile);
			printf("</pre></table>\n");
			printposttable();
		}	
      }
	  fclose(fp);
	  printf("</body>\n");
	  http_quit();
}


int show_form() {
	printpretable_lite();
	printf("<table><form action=bbsmaildown?type=1 method=post>\n");
	printf("<tr><td>起始篇数: <input type=text maxlength=8 size=8 name=start><br>\n");
	printf("<tr><td>终止篇数: <input type=text maxlength=8 size=8 name=end>\n");
	printf("<tr><td><input type=submit value=递交查询>\n");
	printf("</form></table>");	
	printposttable_lite();
	http_quit();
}
