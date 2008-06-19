#include "BBSLIB.inc"

static char *MENUPARENT="<a href=\"#\" style=\"text-decoration: none;\" onclick=\"return SwitchPanel('%s')\">\n<img src=\"%s\">%s\n</a>\n";
static char *MENUDROP_BEGIN="<div id=\"%s\">\n";
static char *MENUITEM="<a href=\"%s\" style=\"text-decoration: none;\" target=\"%s\">\n<img src=\"%s\">%s\n</a>\n";
static char *MENUSUBITEM="<a href=\"%s\" style=\"text-decoration: none;\" target=\"%s\">\n<img src=\"%s\">%s\n</a>\n";
static char *MENUSUBITEM_NOIMAGE="<a href=\"%s\" target=\"%s\">\n%s\n</a>\n";
static char *FRAME_VIEW="view";

void BeginMenuDrop(char *mid, char *img, char *title)
{
        printf(MENUPARENT, mid, img, title);
        printf(MENUDROP_BEGIN,mid);
}

void EndMenuDrop()
{
        printf("</div>");
}

int main() {
        char buf[1024], *ptr;
        init_all();
        printf("<title>欢迎光临日月光华BBS</title>");
        printf("<style type=text/css>\n"
"               body {\n"
"                 margin:0; padding:0; font-family: tahoma, verdana, arial; font-size: 9pt; color: #ffffff; background: #336699; overflow:hidden;\n"
"               }\n"
"               img {\n"
"                 border:0;\n"
"               }\n"
"               a {\n"
"                 color: #ffffff; text-decoration: none; display:block; padding:0 0 4px 2px;word-break: break-all;\n"
"               }\n"
"               #mainbar {\n"
"                 height:100%%; background-color: #336699; overflow: auto; \n"
"               }\n"
"               #mainbar a {\n"
"                 border: #336699 1px solid;  background-color: #336699; font-weight:bold; \n"
"               }\n"
"               #mainbar a:hover {\n"
"                 border: #99ccff 1px solid;  background-color: #6699cc; border-color: #99ccff #003366 #003366 #99ccff;\n"
"               }\n"
"               #mainbar a img {\n"
"                 margin: 2px 2px -6px 0;\n"
"               }\n"
"               #mainbar div {\n"
"                 display:none; border: #6699cc 1px solid; padding:0; \n"
"               }\n"
"               #mainbar div a {\n"
"                 font-weight: normal;\n"
"               }\n"
"               #switchbar {\n"
"                 background:#dddddd url(/images/collapse.gif) no-repeat right 50%;\n"
"                 float:right;\n"
"                 height:100%%;\n"
"                 width:4px;\n"
"               }\n"
"               </style>\n"
"               <script language=\"javascript\" type=\"text/javascript\">\n"
"                 var imgCollapse=new Image();\n"
"                 imgCollapse.src='/images/collapse.gif';\n"
"                 var imgExpand=new Image();\n"
"                 imgExpand.src='/images/expand.gif';\n"
"                 var imgNow=imgCollapse;\n"
"                 function switch_bar()\n"
"                 {\n"
"                   var mybar=document.getElementById('mainbar');\n"
"                   if(mybar.style.display!='none')\n"
"                   {\n"
"                       mybar.style.display='none';\n"
"                       imgNow=imgExpand;\n"
"                       parent.document.getElementById('bar').cols='6, *';\n"
"                   }\n"
"                   else\n"
"                   {\n"
"                       mybar.style.display='block';\n"
"                       imgNow=imgCollapse;\n"
"                       parent.document.getElementById('bar').cols='130, *';\n"
"                   }\n"
"                   document.getElementById('switchbar').style.backgroundImage='url('+imgNow.src+')';\n"
"                 }\n"
"                 var lastitem='undefined';\n"
"                 function SwitchPanel(itemid)\n"
"                 {\n"
"                   if(lastitem!='undefined' && lastitem!=itemid) document.getElementById(lastitem).style.display='none'; \n"
"                   var item=document.getElementById(itemid).style;\n"
"                   if(item.display==\"block\"){\n"
"                     item.display=\"none\";\n"
"                     lastitem=\"undefined\";\n"
"                   } \n"
"                   else{\n"
"                     item.display=\"block\";\n"
"                     lastitem=itemid;\n"
"                   }\n"
"                   return false;\n"
"                 }\n"
"               </script>\n"
"             </HEAD>\n");

printf("<body>\n");
printf("<div style=\"height:100%%\">");
printf("<a href=\"#\" onclick=\"switch_bar()\" id=\"switchbar\">&nbsp;</a>");
printf("<div id=\"mainbar\">");

        printf(MENUITEM ,"/cgi-bin/bbs/bbs0an",FRAME_VIEW,"/images/announce.gif","本站精华");
        printf(MENUITEM ,"/cgi-bin/bbs/bbsall",FRAME_VIEW,"/images/penguin.gif","全部讨论");

        BeginMenuDrop("Stat","/images/top10.gif","统计数据");
        printf(MENUSUBITEM,"/cgi-bin/bbs/bbstop10",FRAME_VIEW,"/images/blankblock.gif","本日十大");
        printf(MENUSUBITEM,"/cgi-bin/bbs/bbstopb10",FRAME_VIEW,"/images/blankblock.gif","热门讨论");
        printf(MENUSUBITEM,"/cgi-bin/bbs/bbsuserinfo",FRAME_VIEW,"/images/blankblock.gif","在线统计");
        EndMenuDrop();

        if(loginok) {
                FILE *fp;
                int i;
                char *cgi="bbsdoc";
                if(atoi(getparm("my_def_mode"))!=0) cgi="bbstdoc";
                BeginMenuDrop("Favorite", "/images/favorite.gif","我的收藏");
                printf(MENUSUBITEM, "/cgi-bin/bbs/bbsmybrd",FRAME_VIEW,"/images/blankblock.gif","预定管理");
                sprintf(buf, "home/%c/%s/.goodbrd", toupper(currentuser.userid[0]), currentuser.userid);
                fp=fopen(buf, "rb");
                if(fp!=NULL)
                {
                    char path[80];
                    struct goodbrdheader gbhd;
                    int brdcount=0;
                    struct boardheader *brc = NULL;
                    while (fread(&gbhd, sizeof(struct goodbrdheader),1,fp))
                    {
                        if (gbhd.flag & BOARD_CUSTOM_FLAG)
                            continue;
                        if (bcache[gbhd.pos].flag & BOARD_DIR_FLAG)
                            sprintf(path, "/cgi-bin/bbs/%s?board=%s", "bbsboa", bcache[gbhd.pos].filename);
                        else
                            sprintf(path, "/cgi-bin/bbs/%s?board=%s", cgi, bcache[gbhd.pos].filename);
                        printf(MENUSUBITEM_NOIMAGE, path,FRAME_VIEW,bcache[gbhd.pos].filename); 
                        brdcount++;
                        if(brdcount>=GOOD_BRC_NUM)
                            break;
                    }

                    fclose(fp);
                }
                EndMenuDrop();
        }

        BeginMenuDrop("EGroup","/images/egroup.gif","分类讨论");
        {
                int i, j;
                struct boardheader *x;
                char path[80];
                char name[80];
                for(i=0; i<SECNUM; i++)
                {
                        sprintf(path, "/cgi-bin/bbs/bbsboa?%d", i);
                        sprintf(name, "%X %s",i,secname[i][0]);
                        printf(MENUSUBITEM,path,FRAME_VIEW,"/images/types/folder1.gif",name);
                }
        }
        EndMenuDrop();

        BeginMenuDrop("QueQiao","/images/chat.gif","鹊桥相会");
        if(loginok)
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsfriend",FRAME_VIEW,"/images/blankblock.gif","在线好友");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsusr",FRAME_VIEW,"/images/blankblock.gif","环顾四方");
        if(currentuser.userlevel & PERM_TALK) {
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbssendmsg",FRAME_VIEW,"/images/blankblock.gif","发送讯息");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsmsg",FRAME_VIEW,"/images/blankblock.gif","查看所有讯息");
        }
        EndMenuDrop();

if(loginok) {
        BeginMenuDrop("Config","/images/config.gif","个人设置");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsinfo",FRAME_VIEW,"/images/blankblock.gif","个人资料");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsplan",FRAME_VIEW,"/images/blankblock.gif","改说明档");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbssig",FRAME_VIEW,"/images/blankblock.gif","改签名档");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsmywww",FRAME_VIEW,"/images/blankblock.gif","WWW定制");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbspwd",FRAME_VIEW,"/images/blankblock.gif","修改密码");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsnick",FRAME_VIEW,"/images/blankblock.gif","临时改昵称");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsfall",FRAME_VIEW,"/images/blankblock.gif","设定好友");
                if(currentuser.userlevel & PERM_CLOAK)
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbscloak",FRAME_VIEW,"/images/blankblock.gif","切换隐身");
        EndMenuDrop();

        BeginMenuDrop("Mail","/images/mail.gif","处理信件");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsnewmail",FRAME_VIEW,"/images/mail_new.gif","阅览新信件");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsmail",FRAME_VIEW,"/images/mail.gif","所有信件");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbsmaildown",FRAME_VIEW,"/images/mail_get.gif","下载信件");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbspstmail",FRAME_VIEW,"/images/mail_write.gif","发送信件");
        EndMenuDrop();
}

        BeginMenuDrop("Search","/images/search.gif","查找选项");
                if(HAS_PERM(PERM_OBOARDS)&&HAS_PERM(PERM_SPECIAL0)) //修改权限限定+0权限控制06.1.2
                        printf(MENUSUBITEM,"/cgi-bin/bbs/bbsfind",FRAME_VIEW,"/images/blankblock.gif","查找文章");
                if(loginok)printf(MENUSUBITEM,"/cgi-bin/bbs/bbsqry",FRAME_VIEW,"/images/blankblock.gif","查询网友");
                printf(MENUSUBITEM,"/cgi-bin/bbs/bbssel",FRAME_VIEW,"/images/blankblock.gif","查找讨论区");
                printf(MENUSUBITEM,"/cgi-bin/rsearch",FRAME_VIEW,"/images/search.gif","日月光华搜索");
                printf(MENUSUBITEM,"http://ycul.com?ref=bbs.fudan.edu.cn",FRAME_VIEW,"/images/service/ycul.gif","Ycul搜索");
                printf(MENUSUBITEM,"http://10.13.200.200:8080/",FRAME_VIEW,"/images/blankblock.gif","富库搜索");
                printf(MENUSUBITEM,"http://www.cit.fudan.edu.cn/opensource/",FRAME_VIEW,"/images/blankblock.gif","信息产业主体数据库");      
        EndMenuDrop();


        BeginMenuDrop("Service","/images/service.gif","公共服务");
                printf(MENUSUBITEM,"#","_self","/images/blankblock.gif","开发中...");
        EndMenuDrop();

        printf(MENUITEM,"telnet://bbs.fudan.sh.cn:2323","_top","/images/telnet.gif","终端登录");
        if(loginok)
                printf(MENUITEM,"/cgi-bin/bbs/bbslogout","_top","/images/exit.gif","注销登录");
        
        printf("</body>\n</html>");
}

int count_new_mails() {
        struct fileheader x1;
        int n, unread=0;
        char buf[1024];
        FILE *fp;
        if(currentuser.userid[0]==0) return 0;
        sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(currentuser.userid[0]), currentuser.userid);
        fp=fopen(buf, "r");
        if(fp==0) return;
        while(fread(&x1, sizeof(x1), 1, fp)>0)
                if(!(x1.accessed[0] & FILE_READ)) unread++;
        fclose(fp);
        return unread;
}

