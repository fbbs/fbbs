#include "libweb.h"

int main() {
	FILE *fp;
	char buf[256];
	init_all();
	printf("<b><font style='font-size: 18pt'>%s</font> · 留言版 </b>[日期: %6.6s]<pre>\n", BBSNAME, Ctime(time(0))+4);
	printpretable();
	fp=fopen("etc/notepad", "r");
	if(fp==0) {
		printf("今天的留言版为空");
		printposttable();
		http_quit();
	}
	while(1) {
		if(fgets(buf, 255, fp)==0) break;
		hprintf("%s", buf);
	}
	fclose(fp);
	printposttable();
	http_quit();
}
