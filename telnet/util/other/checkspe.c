#include "../../include/bbs.h"

struct userec u;

int main(){
	FILE	*fp=fopen("/home/bbs/.PASSWDS","r");
	bool print=false;
	int num=0;
	char genbuf[64];
	int normal_perm=PERM_BASIC
                   +PERM_CHAT
				   +PERM_MAIL
				   +PERM_POST
				   +PERM_LOGINOK
				   +PERM_DENYPOST
				   +PERM_BOARDS
				   +PERM_NOZAP
				   +PERM_MESSAGE
				   +PERM_XEMPT
				   +PERM_SPECIAL8
				   +PERM_SPECIAL4;
  
	fread(&u,sizeof(struct userec),1,fp);
	while(!feof(fp)){
		if ( (u.userlevel & (~normal_perm)) != 0)
			print = true;
			
		if(print==true){
			strcpy(genbuf, "bTCPRD#@XWBA#VS-DOM-F0s2345678\0");
		 	for (num = 0; num < strlen(genbuf) ; num++)
				if (!(u.userlevel & (1 << num)))
					genbuf[num] = '-';
			printf("%-14s: %s\n",u.userid,genbuf);
			print=false;
		}
		
		//printf("%s\n",u.userid);//将所有用户的ID输出
		fread(&u,sizeof(struct userec),1,fp);		
	}
	return 0;
}
