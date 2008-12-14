#include "libweb.h"

//add by Danielfree to check filesize limit 06.11.23
int maxlen(char *board)
{
	char path[256];
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

int main() 
{
	char board[80];
	init_all();
	strsncpy(board, getparm("board"), 30);
	printf("<title>上传文件</title>\n");
	if(!loginok) 
		http_fatal("匆匆过客无权限上传文件");
	if(!has_read_perm(&currentuser, board)) 
		http_fatal("错误的讨论区");
	if(!has_post_perm(&currentuser, board)) 
		http_fatal("该用户在这个版面无权限上传文件");
	strsncpy(board, getparm("board"), 80);
	printf("<script lang='Javascript'>	\n");
	printf("function clickup()			\n");
	printf("{	if(document.forms['upform'].elements['up'].value)	\n");
	printf("		document.forms['upform'].submit();	\n");
	printf("	else	\n");
	printf("		alert('你还没有选中上传文件吧,:)')\n");
	printf("}	\n");
	printf("</script>	\n");
	printf("<b>上传文件至%s讨论区 · %s [使用者: %s] <b><br>\n", board, BBSNAME, currentuser.userid);
	printpretable_lite();
	printf("服务器资源有限，为节省空间，请勿上传过大的文件。请勿上传与版面无关的文件。<br>\n");
	printf("目前本版单个上传文件大小限制为%-2.1fK字节. <br>\n",(float)(maxlen(board)/1024));
	printf("请遵守国家法律，<font color=red>严禁上传非法资料和可能导致纠纷的资料</font>。<br>\n");
	printf("<form method=post name=upform action=bbsupload enctype='multipart/form-data'>\n");
	printf("<table>\n");
	printf("<tr><td>上传文件: <td><input type=file name=up>");
	printf("<input type=hidden name=board value='%s'>", board);
	printf("</table>\n");
	printposttable_lite();
	printf("<input type=button value='上传' onclick='clickup()'> \n");
	printf("</form>");
	http_quit();
}
