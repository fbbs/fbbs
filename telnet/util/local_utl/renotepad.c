#include "../../include/bbs.h"
#include "../../include/permissions.h"

int
report()
{
        return;
}

main()
{
        FILE *rec;
        int i=0;
        struct userec user;

        rec=fopen("/home/bbs/.PASSWDS","rb");

        printf("[1;31;5mUserLevel Records Transfering...\n[m");
        while(1)
        {
                if(fread(&user,sizeof(user),1,rec)<=0) break;
                i++;
                printf("%d %s\t\n",i,user.userid);
                if(user.notemode!=3)
                        user.userdefine|=DEF_NOTEPAD;
                else
                        user.userdefine&=~DEF_NOTEPAD;
                substitute_record("/home/bbs/.PASSWDS",&user,
                                         sizeof(user),i);
        }
        printf("\n[1m%d [32mUserLevel Records Tranfered...[m\n",i);
        fclose(rec);
}
