/*

    restruct:  If you want to use the new struct.h instead of struct.h.old,
    follow these instructions...
    
    1. gcc restruct.c ../record.c -o restruct
    2. cp /home/bbs/.PASSWDS PASSWDS
    3. restruct
    4. cp PASSWDS.new /home/bbs/.PASSWDS
    5. recompile and reinstall innbbsd/innd/bbspost
    6. recompile and reinstall bbspop3d, bbstop, bfinger, ...

    If you choose to mv struct.h.old to struct.h, then you don't have to
    worry about this and no need to run this program.

*/    

#include "bbs.h"

report()
{
}

struct olduserec {                  /* Structure used to hold information in */
        char            userid[IDLEN+2];   /* PASSFILE */
        char            fill[30];
        time_t          firstlogin;
        char            lasthost[16];
        unsigned int    numlogins;
        unsigned int    numposts;
        char            flags[2];
        char            passwd[PASSLEN];
        char            username[NAMELEN];
        char            ident[NAMELEN];
        char            termtype[STRLEN];
        unsigned        userlevel;
        time_t          lastlogin;
        time_t          stay;
        char            realname[NAMELEN];
        char            address[STRLEN];
        char            email[STRLEN];
        int             signature;
        unsigned int    userdefine;
        time_t          notedate;
        int             noteline;
        int             notemode;
        int             unuse1;/* no use*/
        int             unuse2;/* no use*/
};

struct newuserec {                  /* Structure used to hold information in */
        char            userid[IDLEN+2];   /* PASSFILE */
        time_t          firstlogin;
        char            lasthost[16];
        unsigned int    numlogins;
        unsigned int    numposts;
        char            flags[2];
        char            passwd[PASSLEN];
        char            username[NAMELEN];
        char            ident[NAMELEN];
        char            termtype[16];
        char            reginfo[64];
        unsigned int    userlevel;
        time_t          lastlogin;
        time_t          stay;
        char            realname[NAMELEN];
        char            address[STRLEN];
        char            email[STRLEN];
        int             signature;
        unsigned int    userdefine;
        time_t          notedate;
        int             noteline;
};

main()
{
    struct olduserec fh;
    struct newuserec nfh;
    char fname[80];
    char dname[80];
    char genbuf[120];
    FILE *fp, *fp2;

    memset(&fh,0,sizeof(struct olduserec));
    memset(&nfh,0,sizeof(struct newuserec));

    printf("old record size: %d\nnew record size: %d\n",sizeof(struct newuserec),sizeof(struct olduserec));
    sprintf( fname, "PASSWDS");
    sprintf( dname, "PASSWDS.new");

    if ((fp = fopen(fname, "rb")) == NULL) {
        printf("Error: Cannot open PASSWDS.\n");
        return 0;
    }
    if ((fp2 = fopen(dname, "wb")) == NULL) {
        printf("Error: Cannot write to PASSWDS.new.\n");
        return 0;
    }
    while(1)
    {
      if(fread(&fh,sizeof(fh),1,fp)<=0) break;    
      strcpy(nfh.userid,fh.userid);
      nfh.firstlogin = fh.firstlogin;
      strcpy(nfh.lasthost,fh.lasthost);
      nfh.numlogins = fh.numlogins;
      nfh.numposts = fh.numposts;
      strcpy(nfh.flags,fh.flags);
      strcpy(nfh.passwd,fh.passwd);
      strcpy(nfh.username,fh.username);
      strcpy(nfh.ident,fh.ident);
      strcpy(nfh.termtype,fh.termtype);
      strcpy(nfh.reginfo,fh.termtype+16);
      nfh.userlevel = fh.userlevel;
      nfh.lastlogin = fh.lastlogin;
      nfh.stay = fh.stay;
      strcpy(nfh.realname,fh.realname);
      strcpy(nfh.address,fh.address);
      strcpy(nfh.email,fh.email);
      nfh.signature = fh.signature;
      nfh.userdefine = fh.userdefine;
      nfh.notedate = fh.notedate;
      nfh.noteline = fh.noteline;
      append_record(dname,&nfh,sizeof(nfh));
    }
    fclose(fp);
    fclose(fp2);
    return 1;
}

