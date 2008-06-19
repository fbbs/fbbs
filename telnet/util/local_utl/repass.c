struct userec2 {                  /* Structure used to hold information in */
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
        time_t          unused_time;
        char            realname[NAMELEN];
        char            address[STRLEN];
        char            email[STRLEN];
};
struct adduserec {                  /* Structure used to add .PASSWDS */
        int             signature;
        int             userdefine;
        time_t          notetime;     
        int             noteline;
        int             notemode;
        int             unuse1;
        int             unuse2;
}

main()
{
        FILE *rec,*rec2;
        int i=0;
        struct userec2 user;
        struct adduserec add;
        rec=fopen("./.PASSWDS","rb");
        rec2=fopen("./.PASSWDS.tmp","wb");
        add.userdefine=-1;
        add.signature=-1;
        add.notemode=-1;
        add.unuse1=-1;
        add.unuse2=-1;

        printf("Records transfer...");
        while(1)
        {
                i++;
                if(fread(&user,sizeof(user),1,rec)<=0) break;
                fwrite(&user,sizeof(user),1,rec2);     
                fwrite(&add,sizeof(add),1,rec2);
        }
        printf("\n%d records changed...\n",i);
        fclose(rec);
        fclose(rec2);
}
