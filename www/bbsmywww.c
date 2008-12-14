#include "libweb.h"

int main() {
	FILE *fp;
	char *ptr, path[256], buf[256], buf1[256], buf2[256];
	int t_lines=20, link_mode=0, def_mode=0, type, my_style=0;
	my_style=init_all();
	if(!loginok) 
	{
		printf("<b>WWW个人定制 · %s </b><br>\n",BBSNAME);
		printpretable_lite();
		http_fatal("匆匆过客不能定制界面");
	}
	sprintf(path, "home/%c/%s/.mywww", toupper(currentuser.userid[0]), currentuser.userid);
        type=atoi(getparm("type"));
	if(type==0)
	{
		fp=fopen(path, "r");
		if(fp) {
			while(1) {
				if(fgets(buf, 80, fp)==0) break;
				if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
				if(!strcmp(buf1, "t_lines")) t_lines=atoi(buf2);
				if(!strcmp(buf1, "link_mode")) link_mode=atoi(buf2);
				if(!strcmp(buf1, "def_mode")) def_mode=atoi(buf2);
				if(!strcmp(buf1, "my_style")) my_style=atoi(buf2);
			}
			fclose(fp);
		}
	}
	else
	{
		ptr=getparm("t_lines");
		if(ptr[0]) t_lines=atoi(ptr);
		ptr=getparm("link_mode");
		if(ptr[0]) link_mode=atoi(ptr);
       	 	ptr=getparm("def_mode");
        	if(ptr[0]) def_mode=atoi(ptr);
	}
	printf("<b>WWW个人定制 · %s [使用者: %s]</b>", BBSNAME, currentuser.userid);
	printpretable_lite();
	if(type>0) return save_set(path, t_lines, link_mode, def_mode, my_style);
	printf("<table>\n");
	if(t_lines<10 || t_lines>40) t_lines=TLINES;
	if(link_mode<0 || link_mode>1) link_mode=0;
	printf("<tr><td><form action=bbsmywww>\n");
	printf("<input type=hidden name=type value=1>");
	printf("一屏显示的文章行数(10-40): <input name=t_lines size=8 value=%d><br>\n", t_lines);
	printf("链接识别 (0识别, 1不识别): <input name=link_mode size=8 value=%d><br>\n", link_mode);
	printf("缺省模式 (0一般, 1主题)  : <input name=def_mode size=8 value=%d><br><hr>\n", def_mode);
	printf("界面风格 (0仿TERM, 1传统, 2仿黑底TERM): <input name=my_style size=8 value=%d><br><br>\n", my_style);
	printf("<tr><td align=center><input type=submit value=确定> <input type=reset value=复原>\n");
	printf("</form>\n");
}

int save_set(char *path, int t_lines, int link_mode, int def_mode, int my_style)
{
	FILE *fp;
	char buf[80];
	if(t_lines<10 || t_lines>40) http_fatal("错误的行数");
	if(link_mode<0 || link_mode>1) http_fatal("错误的链接识别参数");
	if(def_mode<0 || def_mode>1) http_fatal("错误的缺省模式");
	if(my_style<0||my_style>2) http_fatal("错误的界面风格");	
	fp=fopen(path, "w");
	fprintf(fp, "t_lines %d\n", t_lines);
	fprintf(fp, "link_mode %d\n", link_mode);
	fprintf(fp, "def_mode %d\n", def_mode);
	fprintf(fp, "my_style %d\n", my_style);
	fclose(fp);
	sprintf(buf, "%d", t_lines);
	setcookie("my_t_lines", buf);
	sprintf(buf, "%d", link_mode);
	setcookie("my_link_mode", buf);
        sprintf(buf, "%d", def_mode);
        setcookie("my_def_mode", buf);
	sprintf(buf, "%d", my_style);
	setcookie("my_style", buf);
	printf("WWW定制参数设定成功.<br>\n");
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
}
