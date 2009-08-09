#include "libweb.h"

int bbsmail_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;

	int start = strtol(getparm("start"), NULL, 10);
	char buf[HOMELEN];
	setmdir(buf, currentuser.userid);
	void *ptr;
	size_t size;
	int fd = mmap_open(buf, MMAP_RDONLY, &ptr, &size);
	if (fd < 0)
		return BBS_ENOFILE; // TODO: empty?

	int total = size / sizeof(struct fileheader);
	if (start <= 0)
		start = total - TLINES + 1;
	if (start < 1)
		start = 1;
	struct fileheader *fh = (struct fileheader *)ptr + start - 1;
	struct fileheader *end = (struct fileheader *)ptr + total;
	xml_header("bbsmail");
	printf("<bbsmail>\n");
	for (int i = 0; i < TLINES && fh != end; ++i) {
		int mark = ' ';
		if (fh->accessed[0] & MAIL_REPLY)
			mark = 'r';
		if (fh->accessed[0] & FILE_MARKED) {
			if (mark == 'r')
				mark = 'b';
			else
				mark = 'm';
		}
		if (!(fh->accessed[0] & FILE_READ)) {
			if (mark == ' ')
				mark = '+';
			else
				mark = toupper(mark);
		}
		printf("<mail><mark>%c</mark><sender>%s</sender><date>%s</date><title>",
				mark, fh->owner, getdatestring(getfiletime(fh), DATE_XML));
		xml_fputs(fh->title, stdout);
		printf("</title><name>%s</name></mail>\n", fh->filename);
		fh++;
	}
	mmap_close(ptr, size, fd);
	printf("<user>%s</user><total>%d</total><start>%d</start><page>%d</page>",
			currentuser.userid, total, start, TLINES);
	printf("</bbsmail>");	
	return 0;
}
#if 0
int main() {
	FILE *fp;
	int filetime, i, start, total, type;
	char *ptr, buf[512], path[80], dir[80];
	struct fileheader *data;

	strlcpy(buf, getparm("start"), 10);
	start=atoi(buf);
        /* 02.11.17 added by stephen to fix the jump to mail num trouble */
	start--;
	/* 02.11.17 add end */
	if(buf[0]==0) start=999999;
   	printf("<b>信件列表 · %s [使用者: %s]</b><br>\n", BBSNAME, currentuser.userid);
   	sprintf(dir, "mail/%c/%s/.DIR", toupper(currentuser.userid[0]), currentuser.userid);
   	total=file_size(dir)/sizeof(struct fileheader);
	if(total<0 || total>30000) 
	{
		printpretable_lite();
		http_fatal("too many mails");
	}
   	data=(struct fileheader *)calloc(total, sizeof(struct fileheader));
   	if(data==0) 
	{
		printpretable_lite();
		http_fatal("memory overflow");
	}
	fp=fopen(dir, "r");
	if(fp==0) 
	{
		printpretable_lite();
		http_fatal("dir error");
	}
	total=fread(data, sizeof(struct fileheader), total, fp);
	fclose(fp);
	if(start>total-19) start=total-19;
	if(start<0) start=0;
   	printf("<center>\n");
	printpretable();
	printf("<form name=form1 method=post action=bbsmailman>");
	printf("<table width=100%%>\n");
	printf("<tr class=pt9h ><td><font color=white>序号<td><font color=white>管理<td><font color=white>状态<td><font color=white>发信人<td><font color=white>日期<td><font color=white>信件标题\n");
	int cc=0;
	for(i=start; i<start+19 && i<total; i++) 
	{
	 	int type='N';
	 	printf("<tr class=%s><td>%d",((cc++)%2)?"pt9dc":"pt9lc" , i+1);
		/* added by roly 05.11 */
		printf("<td><input style='height:18px' name=box%s type=checkbox>",data[i].filename);
		/* add end */
        if(data[i].accessed[0] & FILE_READ) 
			type=' ';
        if(data[i].accessed[0] & FILE_MARKED) 
			type= (type=='N') ? 'M' : 'm';
		printf("<td>%c", type);
			//added by iamfat 2002.08.10
			//check_anonymous(data[i].owner);
			//added end.
 		ptr=strtok(data[i].owner, " (");
		if(ptr==0) ptr=" ";
		ptr=nohtml(ptr);
		printf("<td><a href=bbsqry?userid=%s>%13.13s</a>", ptr, ptr);
	 	filetime=atoi(data[i].filename+2);
		printf("<td>%12.12s", Ctime(filetime)+4);
		printf("<td><a href=bbsmailcon?file=%s&num=%d>", data[i].filename, i);
	 	if(strncmp("Re: ", data[i].title, 4)) 
			printf("★ ");
		hprintf("%42.42s", data[i].title);
	 	printf("</a>\n");
	}
    free(data);
    printf("</table>\n");
	printposttable();
	printf("</center>");
		/* added by roly 05.11 */
	printf("<input type=hidden name=mode value=''>");
	printf("</form>");
	printf("<b>信件总数: %d</b>   ", total);
	printf("\n[<a href='#' onclick=\"var all_inputs=document.getElementsByName('form1')[0].getElementsByTagName('input'); for(var i=0;i<all_inputs.length;i++){if(all_inputs[i].type=='checkbox')all_inputs[i].checked=true;} return false;\">本页全选</a>] \n");

    printf("[<a href='#' onclick=\"var all_inputs=document.getElementsByName('form1')[0].getElementsByTagName('input'); for(var i=0;i<all_inputs.length;i++){if(all_inputs[i].type=='checkbox')all_inputs[i].checked=(all_inputs[i].checked)?false:true;} return false;\">本页反选</a>]\n");

	
		
	if (!mailnum_under_limit(currentuser.userid) || !mailsize_under_limit(currentuser.userid)) 
		printf("<font color=#ff0000>你的信件超标！</font><br>");
	else
		printf("<br>[<a href=bbspstmail>发送信件</a>]  ");
	printf("[<a href='#' onclick='document.form1.mode.value=1; document.form1.submit();'>删除选择信件</a>]  ");
		/* add end */
		/* modified by roly for mail check */
	/* modify end */
	if(start>0) 
	{
		i=start-19;
		if(i<0)
			i=0;
		printf("[<a href=bbsmail?start=0>第一页</a>]  ");
		printf("[<a href=bbsmail?start=%d>上一页</a>]  ", i);
	}
	if(start<total-19) 
	{
		i=start+19;
		if(i>total-1) 
			i=total-1;
		printf("[<a href=bbsmail?start=%d>下一页</a>]  ", i);
		printf("[<a href=bbsmail>最后一页</a>]");
	}
	/* 02.11.17 added by stephen to fix the jump to mail trouble */
	start ++;
	/* 02.11.17 add end */
	printf("<form><input type=submit value=跳转到> 第 <input style='height:20px' type=text name=start size=3> 封</form>");
	http_quit();
}
#endif
