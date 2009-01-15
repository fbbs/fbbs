// BBS RSS Service 
// jacobson 2006.4
// This file implements RSS 2.0
#define XMLFILE  
#include "libweb.h"
#include <time.h>
#define BBSURL BBSHOST "/cgi-bin/bbs"
//#define BBSURL "192.168.18.4" "/cgi-bin/bbs"
#define MAXNUM 30 //the largest number of posts RSS fetched
int main()    
{ 
	FILE *fp;
	char board[80], dir[80];
	struct boardheader *x1;
	struct fileheader x;
	int i, start, total, count = 0, cache = 1;
	//char buff[MAXNUM][2048];

	char boardLink[128], boardDes[128], buf[1024], filename[80];
	struct stat latestfile, cachefile;

	//silence = 1;
	init_all();
	count = MAXNUM;

	strlcpy(board, getparm("board"), 32);
	x1 = getbcache(board);
	if (x1 == 0)
		http_fatal("错误的讨论区");
	strcpy(board, x1->filename);
	if (!has_read_perm(&currentuser, board))
		http_fatal("错误的讨论区");
    if (x1 ->flag & BOARD_DIR_FLAG)
		http_fatal("你选择的是一个目录"); //add by Danielfree 06.3.5
	if ((x1->flag & BOARD_CLUB_FLAG)&& (x1->flag & BOARD_READ_FLAG )&& !has_BM_perm(&currentuser, board)&& !isclubmember(currentuser.userid, board))
		http_fatal("您不是俱乐部版 %s 的成员，无权访问该版面", board);
	sprintf(dir, "boards/%s/.DIR", board);
	total = file_size(dir) / sizeof(x);
	if (total == 0)
		http_fatal("讨论区暂无文章");
	start = total - count;
	if (start < 0)
	{
		start = 0;
		count = total;
	}
	sprintf(boardLink, "http://" BBSURL "/bbsdoc?board=%s",
		board);
	strcpy(boardDes, x1->title + 11);
	printf("<?xml version=\"1.0\" encoding=\"GB2312\"?>\n"
		"<?xml-stylesheet href=\"/css/bbsrss.xsl\" type=\"text/xsl\" media=\"screen\"?>\n"
		"<rss version=\"2.0\">\n	<channel>\n"
		"		<title>%s</title>\n		<description> %s </description>\n"
		"		<link>%s</link>\n		<generator>%s</generator>\n",
		board, boardDes, boardLink, BBSURL);
	fp = fopen(dir, "r");
	if (fseek(fp, (start * sizeof(x)), SEEK_SET) == -1)
	{
		fclose(fp);
		http_fatal("seek error");
	}  
	char date[128];
	i=0;
	while (i<count){
			if (fread(&x, sizeof(x), 1, fp) <= 0)
					break;
			if (x.id == x.gid) { //只输出主贴
					i++;
					sprintf(filename, "boards/%s/%s", board, x.filename);
					stat(filename, &cachefile);
					strftime(date, 128, "%a,%d %b %Y %H:%M:%S %z",
									localtime(&cachefile.st_mtime));
					printf("<item>\n<title> %s </title>	\n"
							"<link> <![CDATA[http://" BBSURL "/bbscon?b=%s&f=%s]]> </link>\n"
							"<author> %s </author>	\n"
							"<pubDate> %s </pubDate>	\n"  
							"<source> %s </source>	\n"  
							"<guid> <![CDATA[http://" BBSURL "/bbscon?b=%s&f=%s]]> </guid> \n"
							"<description><![CDATA[\n",
							(nohtml(x.title)),
							board, x.filename, x.owner, date, x.owner, board,x.filename);
					showcontent(filename); //display the post file content
					printf("]]></description>	\n"
					"</item>\n");
			}	
	}
	printf( "%s", "</channel></rss>\n");

}
