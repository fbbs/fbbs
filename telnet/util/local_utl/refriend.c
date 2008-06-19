#include "bbs.h"

int
report()
{
        return;
}

int
tranfer(uid)
char *uid;
{
    struct friend fh;
    char genbuf[80],*str;
    char fname[80];
    char dname[80];
    FILE *fp;

    memset(&fh,0,sizeof(struct friend));

    sprintf( fname, "/home/bbs/home/%s/overrides",uid);
    sprintf( dname, "/home/bbs/home/%s/friends",uid);

    if ((fp = fopen(fname, "r")) == NULL) {
        return 0;
    }
    while (fgets(genbuf, 80, fp) != NULL) 
    {
        if ( (str=strtok( genbuf, " \n\r\t") )!= NULL) 
        {
            sprintf( fh.id,"%.12s", str );
            str= strtok( NULL, " \n\r\t");
            sprintf( fh.exp,"%.14s", (str==NULL)?"\0":str );
            append_record(dname,&fh,sizeof(fh));
        }
    }
    fclose(fp);
    sprintf(genbuf,"chown bbs.bbs %s",dname);
    system(genbuf);
    unlink(fname);
    return 1;
}

main()
{
        FILE *rec;
        int i=0;
        struct userec user;

        rec=fopen("/home/bbs/.PASSWDS","rb");

        printf("[1;31;5mFriends Records Transfering...[m");
        while(1)
        {
                if(fread(&user,sizeof(user),1,rec)<=0) break;
                if(user.numlogins<=0)
                        continue;
                if(tranfer(user.userid)==1)
                {
                        printf("[1m%.12s[36m Tranfered[m\n",user.userid);
                }else
                        printf("[1m%.12s[34m No overrides File...[m\n",user.userid);

                i++;
        }
        printf("\n[1m%d [32mFriends Records Tranfered...[m\n",i);
        fclose(rec);
}
