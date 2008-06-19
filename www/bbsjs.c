#include "BBSLIB.inc"

int main() {
  	FILE *fp;
    	static char s[300];
	init_all();
	printf("<font style='font-size:12px'>\n");
  	printf("<center>欢迎访问[%s], 目前在线人数(www/all) [<font color=green>%d/%d</font>]", count_www(), count_online());
	printf("</font>");
}

int count_www() {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].mode==10001) total++;
	return total;
}
