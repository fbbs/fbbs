#include "../../include/bbs.h"
#include "../../include/permissions.h"
#include "UPDATE.h"

int
report()
{
        return;
}

main()
{
        FILE *rec;
        int i;
        char buf[256];
        char buf2[256];
        struct userec user;

        rec=fopen(PASS,"rb");
        printf("1. Create Index directory\n");
        for(i='A';i<='Z';i++)
        {
                sprintf(buf,"%s/home/%c",BBSHOME,i);
                mkdir(buf,0760);
                chown(buf,9999,99);
                sprintf(buf,"%s/mail/%c",BBSHOME,i);
                mkdir(buf,0760);
                chown(buf,9999,99);
        }
        i=0;        
        printf("2. Moving User Directory\n");
        while(1)
        {
/*                break;  */
                if(fread(&user,sizeof(user),1,rec)<=0) break;
                i++;
                if(user.numlogins<=0)
                        continue;
                sprintf(buf,"%s/home/%s",BBSHOME,user.userid);
                sprintf(buf2,"%s/home/%c/%s",BBSHOME,toupper(user.userid[0]),user.userid);
                rename(buf,buf2);
                sprintf(buf,"%s/mail/%s",BBSHOME,user.userid);
                sprintf(buf2,"%s/mail/%c/%s",BBSHOME,toupper(user.userid[0]),user.userid);
                rename(buf,buf2);
        }
        printf("3. Done.\n");
        fclose(rec);
}
