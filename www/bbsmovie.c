#include "libweb.h"

int main() {
        FILE *fp;
        char buf[120][256], buf2[80];
        int total=0, num, i;
        init_all();
        num=atoi(getparm("num"));
        fp=fopen("etc/movie", "r");
        if(fp==0)
                http_fatal("本站活动看版尚未启动");
        while(total<120) {
                if(fgets(buf[total], 255, fp)==0) break;
                total++;
        }
        fclose(fp);
        if(num>total/5-1) num=total/5-1;
        if(num<0) num=0;
        if(total>=5) {
                sprintf(buf2, "bbsmovie?num=%d", (num+1)%(total/5));
                refreshto(buf2, 10);
        }
        printf("<center>%s -- 活动看版 [第%d/%d页]<hr color=green>\n", BBSNAME, num+1, total/5);
        printf("<table><tr><td><pre>");
        printf("<font color=red>□————————————┤日月光华活动看板 ├————————————□</font>\n");
       
 		for(i=num*5; i<=num*5+4; i++)
                hhprintf("%s", buf[i]);
        printf("<font color=red>□—————————————————————————————————————□</font>\n");
        printf("</pre></table><hr color=green>\n");
        printf("<div align=right>");
        printf("(10秒自动刷新) ");
        for(i=0; i<total/5; i++) {
                if(i!=num) {
                        printf("<a href=bbsmovie?num=%d>[%d]</a>", i, i+1);
                } else {
                        printf("[%d]", i+1);
                }
        }
        printf("</div>");
}
