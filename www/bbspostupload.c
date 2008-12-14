#include "libweb.h"

#ifndef UPLOAD_MAX
	#define UPLOAD_MAX	(1*1024*1024)
#endif


int main() 
{
	char file[100],dir[100],url_filename[256],board[20];
	int i;
	static struct dir x;
	FILE * fp;

	init_all();
	printf("<title>上传文件</title>\n");
	if(!loginok) 
		http_fatal("匆匆过客无法执行本操作，请先登录");
	x.reid=1;
	strsncpy(board, getparm("board"), 20);
	strsncpy(x.owner, currentuser.userid, 13);
	strsncpy(x.filename, getparm("name"), 100);
	strtourl(url_filename,x.filename);
	x.timeDeleted=time(0);
	sprintf(file,"%s/upload/%s/%s",BBSHOME,board,x.filename);
	sprintf(dir,"%s/upload/%s/.DIR",BBSHOME,board);
	if(!has_post_perm(&currentuser, board)) 
		http_fatal("错误的讨论区或无权上传文件至本讨论区");
	if(!file_exist(file)) 
		http_fatal("错误的文件名");
	x.id=file_size(file);
	if(x.id>UPLOAD_MAX) 
	{
		unlink(file);
		http_fatal("文件大小超过最大文件限制");
	}
	
	fp=fopen(dir, "a");
	if(fp==NULL)
	{
		unlink(file);
		http_fatal("内部错误:写文件错误");
	}
	fwrite(&x, sizeof(struct dir), 1, fp);
	fclose(fp);
	{
		char buf[256],log[100];
		sprintf(buf, "UP [%s] %s %dB %s %s FILE:%s\n",cn_Ctime(time(0)), currentuser.userid, x.id, fromhost, board, x.filename);
		sprintf(log,"%s/upload.log",BBSHOME);
		f_append(log, buf);
	}

	printf("文件上传成功, 详细信息如下:");
	printpretable_lite();
	{
		float my_size=x.id;
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
		printf("文件大小: %s<br>\n", sizestr);
	}
	printf("文件名称: %s<br>\n", x.filename);
	printf("上传人ID: %s<br>\n", x.owner);
	printf("上传时间: %s<br>\n", cn_Ctime(time(0)));
	printf("上传版面: %s<br>\n", board);
	printf("上传文件将自动在文章中添加http://转义,<br>\n");
	printf("请保持自动添加部分原样(虽然看起来像乱码),<br>\n");
	printf("在www界面下转义部分将自动转换为对应的链接/图片.\n");
	printposttable_lite();
	printf("<a href='#' onclick='return closewin()'>返回</a>\n");
	printf("<script language='JavaScript'>\n");
	printf("<!--					\n");
	printf("function closewin()		\n");
	printf("{						\n");
    	printf("    opener.document.forms['postform'].elements['text'].value +='\\nhttp://%s/upload/%s/%s\\n';\n",BBSHOST, board,url_filename);
	printf("	return window.close();		\n");
	printf("}						\n");
	printf("-->						\n");
	printf("</script>\n");
	http_quit();
}

