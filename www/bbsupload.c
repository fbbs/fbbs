#include "libweb.h"

#define UPLOAD_MAX	(1*1024*1024)

int mystrstr(char *haystack, int len, char *needle) 
{
	int i, len2=strlen(needle);
	for(i=0; i<=len-len2; i++)
		if(!strncmp(haystack+i, needle, len2)) 
			return i;
	return -1;
}

void name_check(char * str )
{
	int i;
	for(i=0;str[i];i++)
	{
		if(isdigit(str[i]) || isalpha(str[i]) )
			continue;
		if(str[i]=='_' || str[i]=='~' || str[i]=='.')		//add other legal characters here if needed
			continue;
		str[i]='_';				//turn all the other chars into '_'
	}
}

int BoardQuota(char *board)
{
	int all=0, now=0;
	char path[256], cmd[256];
	sprintf(path, "%s/upload/%s", BBSHOME, board);
	if(dashd(path))
	{
		FILE *fp;
		sprintf(cmd , "du %s|cut -f1>%s/.size", path, path);
		system(cmd);
		sprintf(cmd, "%s/.size", path);
		fp=fopen(cmd, "r");
		if(!fp)return 1;
		fscanf(fp, "%d", &now);
		fclose(fp);
		sprintf(cmd, "%s/.quota", path);
		fp=fopen(cmd, "r");
		if(fp)
		{
			fscanf(fp, "%d", &all);
			if(now>=all)return 1;
			fclose(fp);
		}
		return 0;
	}
	return 1;
}
int maxlen(char *board) //add by Danielfree to check filesize limit 06.11.23
{
	char path[256],cmd[256];
	int	limit=1*1024*1024;
	sprintf(path,"%s/upload/%s/.maxlen",BBSHOME,board);
	if (dashf(path))
	{
		FILE *fp;
		fp=fopen(path,"r");
		if(fp)
			{
				fscanf(fp,"%d",&limit);
				fclose(fp);
			}
	}
	return	limit;
}
void addtodir(char *board, char *tmpfile) 
{
	char file[100],dir[100],url_filename[256];
	static struct dir x;
	FILE * fp;

	init_all();
	printf("<title>上传文件</title>\n");
	if(!loginok) 
		http_fatal("匆匆过客无法执行本操作，请先登录");
	x.reid=1;
	strsncpy(x.owner, currentuser.userid, 13);
	strsncpy(x.filename, tmpfile, 100);
	strtourl(url_filename,x.filename);
	x.timeDeleted=time(0);
	sprintf(file,"%s/upload/%s/%s",BBSHOME,board,x.filename);
	sprintf(dir,"%s/upload/%s/.DIR",BBSHOME,board);
	if(!has_post_perm(&currentuser, board)) 
		http_fatal("错误的讨论区或无权上传文件至本讨论区");
	if(!file_exist(file)) 
		http_fatal("错误的文件名");
	x.id=file_size(file);
	if(x.id>maxlen(board)) 
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
	char buf[256],log[100];
	sprintf(buf, "UP [%s] %s %dB %s %s FILE:%s\n",cn_Ctime(time(0)), currentuser.userid, x.id, fromhost, board, x.filename);
	sprintf(log,"%s/upload.log",BBSHOME);
	f_append(log, buf);

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
int main() 
{
	int m, i=0;
	long content_len, read_len=0, file_len;
	char *buf, *p, *filename, buf_alt[256], http_boundary[280], temp_filename[100];
	char board[80], level[80], live[80], exp[80];
	char postfix[5];
	FILE *fp;
	//printf("Content-type: text/html; charset=gb2312\n\n");
    
	content_len=strtol(getenv("CONTENT_LENGTH"),NULL,10);
	if(content_len<0) 
		content_len=0;
	
	if(content_len>UPLOAD_MAX) 
	{
		if(content_len>UPLOAD_MAX+1024)			//data other than binary file would take about 1K Bytes
			http_fatal("文件大小超过最大限制");
		content_len=UPLOAD_MAX;
	}
	
	buf=malloc(content_len);
	if(buf==0) 
		http_fatal("内部错误：申请内存出错");
	
	fgets(http_boundary, 200, stdin);		
	fgets(buf_alt, 256, stdin);				//line containing 'filename="xxxx"'
	read_len+=strlen(http_boundary)+strlen(buf_alt);
	filename=strstr(buf_alt, "filename=\"");
	if(filename==0) 
	{
		free(buf);
		http_fatal("内部错误：文件名未知");
	}
	
	p=strrchr(filename, '\\');				//windows path symbol
	if(p)
	{
		filename=p+1;		
	}else{	//moz-browsers don't give out full path,just the very filename,so no parse needed
		filename+=strlen("filename=\"");
	}
	p=strchr(filename, '\"');
	if(p) 
		p[0]='\0';							//filename parsing end
	if(strlen(p)>70)
	{
		free(buf);
		http_fatal("内部错误: 文件名过长");
	}
	
	/* 文件扩展名检查，只允许.jpg/.bmp/.gif/.png/.jpeg/.jfif格式 */
	/* added by money 2003.12.22 */
	if (strlen(filename) < 4)
	{
		free(buf);
		http_fatal("内部错误: 文件名过短");
	}
	for (i=1; i<5; i++)
	if (filename[strlen(filename)-i] >= 97)
		filename[strlen(filename)-i] -= 32;
    strncpy(postfix,filename + strlen(filename) - 4,5);
    if(!strncmp(postfix,"JPEG",4))
        strncpy(postfix,filename + strlen(filename) - 5,6);
    else if (strncmp(postfix, ".JPG", 4) &&
		    strncmp(postfix, ".GIF", 4) &&
		    strncmp(postfix, ".PNG", 4) &&
		    strncmp(postfix,".PDF",4) )
	{
		free(buf);
		http_fatal("内部错误: 文件必须是图片文件或PDF 文件");
	}
	/* add end */
	
	fgets(buf_alt, 256, stdin);
	read_len+=strlen(buf_alt);
	fgets(buf_alt, 256, stdin);
	read_len+=strlen(buf_alt);				//supposed to be two blank line
	content_len-=read_len;
	if(content_len<0) 
		content_len=0;
	m=fread(buf, 1, content_len, stdin);
	buf[m]='\0';
	file_len=mystrstr(buf,m,http_boundary);	//find the ending boundary of this file
	if(file_len<0)							//not using strstr() 'cause it won't work properly in binary data mode
	{
		free(buf);
		http_fatal("内部错误：解析http数据格式错误");
	}
	
	if(file_len-2>UPLOAD_MAX)
	{
		free(buf);
		http_fatal("文件大小超过最大文件限制");
	}
	p=buf+file_len;
	sprintf(temp_filename, "%s/tmp/%d.upload", BBSHOME,getpid());
	if((fp=fopen(temp_filename,"w+"))==NULL)
	{
		free(buf);
		http_fatal("内部错误：文件打开错误");
	}
	fwrite(p,1,strlen(p),fp);
	rewind(fp);
	for(m=1;m<=3;m++)
		fgets(buf_alt,256,fp);					//3 lines of rubbish before the 'board' value
	fgets(board,80,fp);
	fclose(fp);
	unlink(temp_filename);
	m=0;										//strip \r\n out of board
	while(board[m]&&board[m]!='\n'&&board[m]!='\r')
		m++;
	if(board[m]=='\n'||board[m]=='\r')
		board[m]='\0';
	if(file_len-2>maxlen(board))
	{
		free(buf);
		http_fatal("文件大小超过最大文件限制");
	}
	if(BoardQuota(board))
	{
		free(buf);
		http_fatal("版面超限 无法上传");
	}
	//name_check(filename);
//	sprintf(temp_filename,"%s/upload/%s/%s",BBSHOME, board, ofname);
//	if(file_exist(temp_filename))
	{   
        srand((int)time(0)+getpid());
		sprintf(buf_alt,"%d-%04d%s",time(0),(int)(10000.0*rand()/RAND_MAX)+(int)filename[0]+(int)filename[1],postfix);
		sprintf(temp_filename,"%s/upload/%s/%s",BBSHOME,board,buf_alt);
	}
	fp=fopen(temp_filename, "w");
	fwrite(buf, 1, file_len-2, fp);
	fclose(fp);
	free(buf);
	addtodir(board, buf_alt);
	http_quit();
}
