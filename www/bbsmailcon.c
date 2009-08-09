#include "libweb.h"

int bbsmailcon_main(void)
{
	if (!loginok)
		return BBS_ELGNREQ;
	char file[40];
	strlcpy(file, getparm("f"), sizeof(file));
	if (!valid_mailname(file))
		return BBS_EINVAL;
	char buf[HOMELEN];
	void *ptr;
	size_t size;
	int fd;
	// deal with index
	setmdir(buf, currentuser.userid);
	if ((fd = mmap_open(buf, MMAP_RDWR, &ptr, &size)) < 0)
		return BBS_ENOFILE;
	struct fileheader *fh = bbsmail_search(ptr, size, file);
	if (fh == NULL) {
		mmap_close(ptr, size, fd);
		return BBS_ENOFILE;
	}
	if (!(fh->accessed[0] & FILE_READ)) {
		fh->accessed[0] |= FILE_READ;
	}
	xml_header("bbsmailcon");
	printf("<bbsmailcon><user>%s</user><title>", currentuser.userid);
	xml_fputs(fh->title, stdout);
	printf("</title>");
	struct fileheader *prev = fh - 1;
	if (prev >= (struct fileheader *)ptr)
		printf("<prev>%s</prev>", prev->filename);
	struct fileheader *next = fh + 1;
	if (next < (struct fileheader *)ptr + size / sizeof(*next))
		printf("<next>%s</next>", next->filename);
	mmap_close(ptr, size, fd);

	// show mail content.
	if (file[0] == 's') // shared mail
		strlcpy(buf, file, sizeof(buf));
	else
		setmfile(buf, currentuser.userid, file);
	if ((fd = mmap_open(buf, MMAP_RDONLY, &ptr, &size)) < 0)
		return BBS_ENOFILE;
	fputs("<mail>", stdout);
	xml_fputs((char *)ptr, stdout);
	fputs("</mail>\n", stdout);
	mmap_close(ptr, size, fd);
	printf("<file>%s</file></bbsmailcon>", file);
	return 0;
}

#if 0
int main() {
	FILE *fp;
	char buf[512], dir[80], file[80], path[80], *ptr, *id;
	struct fileheader x;
	int num, tmp, total;
	init_all();
	//strsncpy(file, getparm("file"), 32);
	strcpy(file, getparm("file"));
    num=atoi(getparm("num"));
	printf("<center>\n");
	id=currentuser.userid;
	printf("阅读信件 · %s [使用者: %s]\n", BBSNAME, id);
    
    if(strncmp(file, "M.", 2) && file[0] != 's') 
	{
		printpretable_lite();
		http_fatal("错误的参数1");
	}
	if((strstr(file, "..") || strstr(file, "/")) && file[0] != 's') 
	{
		printpretable_lite();
		http_fatal("错误的参数2");
	}
	sprintf(dir, "mail/%c/%s/.DIR", toupper(id[0]), id);
	total=file_size(dir)/sizeof(x);
	if(total<=0) 
	{
		printpretable_lite();
		http_fatal("错误的参数3");
	}
	printpretable();
	printf("<table width=100%% border=0>\n");
	printf("<tr><td>\n<pre class=ansi>");
    /****sharedmail****/
    if(file[0] == 's')
        strcpy(path, file);
    else
	    sprintf(path, "mail/%c/%s/%s", toupper(id[0]), id, file);
	fp=fopen(path, "r");
	if(fp==0) 
	{
		printf("本文不存在或者已被删除");
		printf("</pre>\n</table>\n");
		printposttable();
		printf("<br>[<a href=bbsmail>返回信件列表</a>]  ");
		printf("</center>\n");
		http_quit();
	}
	while(1) 
	{
		if(fgets(buf, 512, fp)==0) 
			break;
		hhprintf("%s", buf);
	}
	fclose(fp);
	printf("</pre>\n</table>\n");
	printposttable();
	fp=fopen(dir, "r+");
	if(fp==0) http_fatal("dir error2");
	if(num>0) {
		fseek(fp, sizeof(x)*(num-1), SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		printf("<a href=bbsmailcon?file=%s&num=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一篇</a>  ", x.filename, num-1);
	}
	if(num<total-1) {
		fseek(fp, sizeof(x)*(num+1), SEEK_SET);
      		fread(&x, sizeof(x), 1, fp);
      		printf("<a href=bbsmailcon?file=%s&num=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一篇</a>  ", x.filename, num+1);
	}
	printf("[<a onclick='return confirm(\"您真的要删除这封信吗?\")' href=bbsdelmail?file=%s> 删除</a>]  ", file);
	printf("[<a href=bbsmail>返回信件列表</a>]  ");
	if(num>=0 && num<total) 
	{
		char title2[80];
		fseek(fp, sizeof(x)*num, SEEK_SET);
		if(fread(&x, sizeof(x), 1, fp)>0) 
		{
			//added by iamfat 2002.08.10
			//check_anonymous(x.owner);
			//added end.
			x.accessed[0]|=FILE_READ;
			fseek(fp, sizeof(x)*num, SEEK_SET);
			fwrite(&x, sizeof(x), 1, fp);
			ptr=strtok(x.owner, " (");
			if(ptr==0) 
				ptr=" ";
			strcpy(title2, x.title);
			if(strncmp(x.title, "Re:",3)) 
				sprintf(title2, "Re: %s", x.title);
			title2[60]=0;
			printf("[<a href='bbspstmail?userid=%s&file=%s&title=%s'>回信</a>]",ptr, x.filename, title2);
		}
	}
	fclose(fp);
   	printf("</center>\n"); 
	http_quit();
}

#endif
