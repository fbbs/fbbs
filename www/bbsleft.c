#include "libweb.h"

static char *MENUPARENT="<a href=\"#\" style=\"text-decoration: none;\" onclick=\"return SwitchPanel('%s')\">\n<img src=\"%s\">%s\n</a>\n";
static char *MENUDROP_BEGIN="<div id=\"%s\">\n";
static char *MENUITEM="<a href=\"%s\" style=\"text-decoration: none;\" target=\"%s\">\n<img src=\"%s\">%s\n</a>\n";
static char *MENUSUBITEM="<a href=\"%s\" style=\"text-decoration: none;\" target=\"%s\">\n<img src=\"%s\">%s\n</a>\n";
static char *MENUSUBITEM_NOIMAGE="<a href=\"%s\" target=\"%s\">\n%s\n</a>\n";
static char *FRAME_VIEW="view";

static void BeginMenuDrop(char *mid, char *img, char *title)
{
	printf(MENUPARENT, mid, img, title);
	printf(MENUDROP_BEGIN,mid);
}

static void EndMenuDrop(void)
{
	printf("</div>");
}

int bbsleft_main(void)
{
	printf("<title>欢迎光临日月光华BBS</title>\n");
	printf("<link type=\"text/css\" rel=\"stylesheet\" href=\"/css/bbsleft.css\">\n");
	printf("<script type=\"text/javascript\" src=\"/js/bbsleft.js\"></script>\n");

	printf("<body>\n");
	printf("<div style=\"height:100%%\">");
	printf("<a href=\"#\" onclick=\"switch_bar()\" id=\"switchbar\"></a>");
	printf("<div id=\"mainbar\">");

	printf(MENUITEM, CGIPATH"bbs0an", FRAME_VIEW, "/images/announce.gif", "本站精华");
	printf(MENUITEM, CGIPATH"bbsall", FRAME_VIEW, "/images/penguin.gif", "全部讨论");

	BeginMenuDrop("Stat", "/images/top10.gif", "统计数据");
	printf(MENUSUBITEM, CGIPATH"bbstop10", FRAME_VIEW, "/images/blankblock.gif","本日十大");
	printf(MENUSUBITEM, CGIPATH"bbstopb10", FRAME_VIEW, "/images/blankblock.gif","热门讨论");
	printf(MENUSUBITEM, CGIPATH"bbsuserinfo", FRAME_VIEW, "/images/blankblock.gif","在线统计");
	EndMenuDrop();

	if(loginok) {
		char buf[HOMELEN];
		FILE *fp;
		char *cgi = "bbsdoc";
		if (atoi(getparm("my_def_mode")) != 0)
			cgi = "bbstdoc";
		BeginMenuDrop("Favorite", "/images/favorite.gif", "我的收藏");
		printf(MENUSUBITEM, CGIPATH"bbsmybrd", FRAME_VIEW, "/images/blankblock.gif", "预定管理");
		sethomefile(buf, currentuser.userid, ".goodbrd");
		fp = fopen(buf, "rb");
		if (fp != NULL) {
			char path[LINKLEN];
			struct goodbrdheader gbhd;
			int brdcount = 0;
			while (fread(&gbhd, sizeof(struct goodbrdheader), 1, fp)) {
				if (gbhd.flag & BOARD_CUSTOM_FLAG)
					continue;
				if (bcache[gbhd.pos].flag & BOARD_DIR_FLAG)
					sprintf(path, CGIPATH"%s?board=%s", "bbsboa", bcache[gbhd.pos].filename);
				else
					sprintf(path, CGIPATH"%s?board=%s", cgi, bcache[gbhd.pos].filename);
				printf(MENUSUBITEM_NOIMAGE, path,FRAME_VIEW,bcache[gbhd.pos].filename); 
				brdcount++;
				if(brdcount >= GOOD_BRC_NUM)
					break;
			}
			fclose(fp);
		}
		EndMenuDrop();
	}

	BeginMenuDrop("EGroup", "/images/egroup.gif", "分类讨论");
	{
		int i;
		char path[LINKLEN];
		char name[STRLEN];
		for(i = 0; i < SECNUM; i++)
		{
			sprintf(path, CGIPATH"bbsboa?s=%d", i);
			sprintf(name, "%X %s", i, secname[i][0]);
			printf(MENUSUBITEM, path, FRAME_VIEW, "/images/types/folder1.gif", name);
		}
	}
	EndMenuDrop();

	BeginMenuDrop("QueQiao", "/images/chat.gif", "鹊桥相会");
	if (loginok)
		printf(MENUSUBITEM,CGIPATH"bbsfriend", FRAME_VIEW, "/images/blankblock.gif", "在线好友");
	printf(MENUSUBITEM, CGIPATH"bbsusr", FRAME_VIEW, "/images/blankblock.gif", "环顾四方");
	if(currentuser.userlevel & PERM_TALK) {
		printf(MENUSUBITEM, CGIPATH"bbssendmsg",FRAME_VIEW, "/images/blankblock.gif", "发送讯息");
		printf(MENUSUBITEM, CGIPATH"bbsmsg",FRAME_VIEW, "/images/blankblock.gif", "查看所有讯息");
	}
	EndMenuDrop();

	if(loginok) {
		BeginMenuDrop("Config", "/images/config.gif", "个人设置");
		printf(MENUSUBITEM, CGIPATH"bbsinfo", FRAME_VIEW, "/images/blankblock.gif", "个人资料");
		printf(MENUSUBITEM, CGIPATH"bbsplan", FRAME_VIEW, "/images/blankblock.gif", "改说明档");
		printf(MENUSUBITEM, CGIPATH"bbssig", FRAME_VIEW, "/images/blankblock.gif", "改签名档");
		printf(MENUSUBITEM, CGIPATH"bbsmywww", FRAME_VIEW, "/images/blankblock.gif", "WWW定制");
		printf(MENUSUBITEM, CGIPATH"bbspwd", FRAME_VIEW, "/images/blankblock.gif", "修改密码");
		printf(MENUSUBITEM, CGIPATH"bbsnick", FRAME_VIEW, "/images/blankblock.gif", "临时改昵称");
		printf(MENUSUBITEM, CGIPATH"bbsfall", FRAME_VIEW, "/images/blankblock.gif", "设定好友");
		if (currentuser.userlevel & PERM_CLOAK)
			printf(MENUSUBITEM, CGIPATH"bbscloak", FRAME_VIEW, "/images/blankblock.gif","切换隐身");
		EndMenuDrop();

		BeginMenuDrop("Mail", "/images/mail.gif", "处理信件");
		printf(MENUSUBITEM, CGIPATH"bbsnewmail", FRAME_VIEW, "/images/mail_new.gif", "阅览新信件");
		printf(MENUSUBITEM, CGIPATH"bbsmail", FRAME_VIEW, "/images/mail.gif", "所有信件");
		printf(MENUSUBITEM, CGIPATH"bbsmaildown", FRAME_VIEW, "/images/mail_get.gif", "下载信件");
		printf(MENUSUBITEM, CGIPATH"bbspstmail", FRAME_VIEW, "/images/mail_write.gif", "发送信件");
		EndMenuDrop();
	}

	BeginMenuDrop("Search", "/images/search.gif", "查找选项");
	if (HAS_PERM(PERM_OBOARDS) && HAS_PERM(PERM_SPECIAL0))
		printf(MENUSUBITEM, CGIPATH"bbsfind", FRAME_VIEW, "/images/blankblock.gif", "查找文章");
	if (loginok)
		printf(MENUSUBITEM, CGIPATH"bbsqry", FRAME_VIEW, "/images/blankblock.gif", "查询网友");
	printf(MENUSUBITEM, CGIPATH"bbssel", FRAME_VIEW, "/images/blankblock.gif", "查找讨论区");
	EndMenuDrop();

	printf(MENUITEM, "telnet://"BBSHOST":2323", "_top", "/images/telnet.gif", "终端登录");
	if(loginok)
		printf(MENUITEM, CGIPATH"bbslogout", "_top", "/images/exit.gif", "注销登录");

	printf("</body>\n</html>");

	return 0;
}
