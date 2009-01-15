#include "libweb.h"

int main() 
{
	FILE *fp;
	char buf[512], board[80], dir[80], file[80], filename[80], *ptr;
	struct fileheader x;
	int num, tmp, total;
	init_all();
	strlcpy(board, getparm("board"), 32);
	strlcpy(file, getparm("file"), 32);
	num=atoi(getparm("num"));
	printf("<center>\n");
	if(!has_read_perm(&currentuser, board)) 
	{
		printf("<b>文章阅读 · %s </b></center><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("错误的讨论区");
	}
	strcpy(board, getbcache(board)->filename);
	printf("<b>文章阅读 · %s [讨论区: %s]</b></center><br>\n", BBSNAME, board);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2)) 
	{
		printpretable_lite();
		http_fatal("错误的参数1");
	}
	if(strstr(file, "..") || strstr(file, "/")) 
	{
		printpretable_lite();
		http_fatal("错误的参数2");
	}
	sprintf(dir, "boards/%s/.DIGEST", board);
	total=file_size(dir)/sizeof(x);
	if(total<=0) 
	{
		printpretable_lite();
		http_fatal("此讨论区不存在或者为空");
	}
	printpretable();
	printf("<table width=100%% border=0>\n");
	printf("<tr><td>\n<pre>");
	sprintf(filename, "boards/%s/%s", board, file);
	fp=fopen(filename, "r");
	if(fp==0) 
	{
		printf("本文不存在或者已被删除");
		printf("</pre>\n</table>\n");
		printposttable();
		printf("[<a href=bbsdoc?board=%s>本讨论区</a>]", board);
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
	printf("<center>\n");
	printf("[<a href=bbssec>分类讨论区</a>]  ");
//	printf("[<a href=bbsall>全部讨论区</a>]  ");
	fp=fopen(dir, "r+");
	if(fp==0) 
		http_fatal("dir error2");
	if(num>0) 
	{
		fseek(fp, sizeof(x)*(num-2), SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		printf("<a href=bbsgcon?board=%s&file=%s&num=%d><img border=0 src=/images/button/up.gif align=absmiddle>上一篇</a>  ", board, x.filename, num-1);
	}
	printf("[<a href=bbsdoc?board=%s>本讨论区</a>]  ", board);
	if(num<total-1) 
	{
		fseek(fp, sizeof(x)*(num), SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		printf("<a href=bbsgcon?board=%s&file=%s&num=%d><img border=0 src=/images/button/down.gif align=absmiddle>下一篇</a>  ", board, x.filename, num+1);
	}
	if(num>0 && num<=total) 
	{
		fseek(fp, sizeof(x)*(num-1), SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		#ifdef SPARC
			(*(int*)(x.title+72))++;//modified by roly from 73 to 72 for sparc solaris
		#else
			(*(int*)(x.title+73))++;//modified by roly from 73 to 72 for sparc solaris
		#endif
		//deleted by cometcaptor 2006-10-29 以下代码造成文摘区目录损坏
		//fseek(fp, sizeof(x)*num, SEEK_SET);
		//fwrite(&x, sizeof(x), 1, fp);
	}
	fclose(fp);
	ptr=x.title;
	if(!strncmp(ptr, "Re: ", 4)) 
		ptr+=4;
	printf("[<a href='bbstfind?board=%s&gid=%d'>同主题阅读</a>]\n", board, x.gid);
   	printf("</center>\n"); 
	http_quit();
}
