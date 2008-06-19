#include "BBSLIB.inc"
struct user_info user[MAXACTIVE];

int cmpuser(a, b)
struct user_info *a, *b;
{
	char id1[80], id2[80];
	sprintf(id1, "%d%s", !isfriend(a->userid), a->userid);
	sprintf(id2, "%d%s", !isfriend(b->userid), b->userid);
	return strcasecmp(id1, id2);
}

int main() {
	int i, start, total=0, fh, shmkey, shmid; 
	struct user_info *x;
	int cOutFudan=0,cInFudan=0,cSouth=0,cNorth=0,cEast=0,cMedic=0,cNotknow=0,cZhangJ=0;
	int mWeb=0,mTelnet=0;
	int mcInWeb=0,mcOutWeb=0;
	int mcInTelnet=0,mcOutTelnet=0;
	char search;
	init_all();
	printf("<center>\n");
	for(i=0; i<MAXACTIVE; i++) {
		x=&(shm_utmp->uinfo[i]);
		if(x->active==0) continue;
		if(x->invisible && !HAS_PERM(PERM_SEECLOAK)) continue;
		memcpy(&user[total], x, sizeof(struct user_info));
		total++;
	}

	printf("<b>%s · 在线用户统计 [在线总人数: %d人]</b>\n", BBSNAME, total);

	printf("<table align=center border=0 cellpadding=0 cellspacing=0 width=350>\n");
	printf("	<tr height=6>\n");
	printf("		<td width=6><img border=0 src='/images/lt.gif'></td>\n");
	printf("		<td background='/images/t.gif' width=100%%></td>\n");
	printf("		<td width=6><img border=0 src='/images/rt.gif'></td>\n");
	printf("	</tr>\n");
	printf("	<tr  height=100%%>\n");
	printf("		<td width=6 background='/images/l.gif'>\n");
	printf("		<td width=100%%>\n");
		

	for(i=0; i<total; i++) {
		if (!strncmp(user[i].from,"10.",3) || !strncmp(user[i].from,"192.168",7)) { 
			cInFudan++;
			if (!strncmp(user[i].from,"10.100",6)) cNorth++;
			if (!strncmp(user[i].from,"10.85",5)) cSouth++;
			if (!strncmp(user[i].from,"10.102",6)) cEast++;
			if (!strncmp(user[i].from,"192.168",7)) cMedic++;
			if (!strncmp(user[i].from,"10.13",5) || !strncmp(user[i].from,"10.14",5)) cZhangJ++; //add by Danielfree 06.1.16
			if (user[i].mode==10001) mcInWeb++;
			else mcInTelnet ++;
		} else {
			cOutFudan++;
			if (user[i].mode==10001) mcOutWeb++;
			else 	mcOutTelnet++;
		}
		if (user[i].mode==10001) mWeb++;
		else mTelnet ++;
	}
	printf("<table border=0 width=100%%>\n");
	printf("<tr class=pt9h><td><b>复旦校内用户</font><td align=right><b>%4d", cInFudan);
	
	printf("<tr class=pt9lc><td>&nbsp;&nbsp;&nbsp;南区用户<td align=right>%4d", cSouth);
	printf("<tr class=pt9dc><td>&nbsp;&nbsp;&nbsp;北区用户<td align=right>%4d", cNorth);
    printf("<tr class=pt9lc><td>&nbsp;&nbsp;&nbsp;东区用户<td align=right>%4d", cEast);
	printf("<tr class=pt9dc><td>&nbsp;&nbsp;&nbsp;枫林校区用户<td align=right>%4d",cMedic);
	printf("<tr class=pt9lc><td>&nbsp;&nbsp;&nbsp;张江校区用户<td align=right>%4d", cZhangJ);
	printf("<tr class=pt9lc><td>&nbsp;&nbsp;&nbsp;Telnet用户<td align=right>%4d", mcInTelnet);
	printf("<tr class=pt9dc><td>&nbsp;&nbsp;&nbsp;WWW用户<td align=right>%4d", mcInWeb);

//	printf("<tr class=pt9lc><td>&nbsp;&nbsp;<td>&nbsp;&nbsp;&nbsp");
	printf("<tr class=pt9h ><td><b>复旦校外用户</font><td align=right><b>%4d", cOutFudan);
	printf("<tr class=pt9lc><td>&nbsp;&nbsp;&nbsp;Telnet用户<td align=right>%4d", mcOutTelnet);
	printf("<tr class=pt9dc><td>&nbsp;&nbsp;&nbsp;WWW用户<td align=right>%4d", mcOutWeb);

//	printf("<tr class=pt9lc><td>&nbsp;&nbsp;<td>&nbsp;&nbsp;&nbsp");
	printf("<tr class=pt9h><td><b>合计:</font><td align=right><b>");
	printf("<tr class=pt9lc><td>&nbsp;&nbsp;&nbsp;Telnet用户合计<td align=right>%4d", mTelnet);
	printf("<tr class=pt9dc><td>&nbsp;&nbsp;&nbsp;WWW用户合计<td align=right>%4d", mWeb);
	printf("</tr>\n");
	printf("</table>");
	
	printposttable();			
	printf("<br>");
	printf("[<a href='javascript:history.go(-1)'>返回</a>]  ");
	printf("</center>\n");
	http_quit();
}
