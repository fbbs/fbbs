/*
    功能     :  列出站内使用者资料
    注意事项 :  须要目前的 bbs.h
    Compile  :  gcc -o showuser showuser.c
    使用方式 :  showuser i10 l5 e25
                (列出使用者的 ID, login 次数, email 地址)
    查看结果 ： 在 ~bbs/tmp/showuser.result 中		
 */

/* Modify by deardraogn 2000.10.28 */

#define  FIRSTLOGIN

#include <stdio.h>
#include "../../include/bbs.h"

FILE *outf;
struct userec aman;
char field_str[ 20 ][ 128 ];
char field_idx[] = "dihlpnvVraetufFmI" ;
int  field_count = 0;
int  field_lst_no [ 20 ];
int  field_lst_size [ 20 ];
int  field_default_size [ 20 ] = {
    4, 12, 16,  5,  5,  
   18, 24, 14, 25, 30, 
   30,  8, 32, 24, 14, 
   45, 20,  0,  0,  0
};

char *field_name[] = {
    "Num", 
    "ID ", 
    "LastHost",
    "Visit",
    "Post",
    "Nick",
    "LastVisit_text",
    "LastVisit_num",
    "Real",
    "Addr",
    "Email",
    "Term",
    "Userlevel",
    "FirstVisit_text",
    "FirstVisit_num",
    "RealMail",
    "IdentInfo",
    NULL
};

char MYPASSFILE[160];

set_opt(argc, argv)
int argc;
char *argv[];
{
    int i, field, size;
    char *field_ptr;

    field_count = 0;

    for (i = 1; i < argc; i++) {
        field_ptr = (char *)strchr(field_idx, argv[ i ][ 0 ] );
        if (field_ptr == NULL) continue;
        else field = field_ptr - field_idx ;

        size  = atoi( argv[ i ] + 1 );

        field_lst_no[ field_count ] = field;
        field_lst_size[ field_count ] = (size == 0) ? 
            field_default_size[ field ] : size;
        field_count++;
    }

}

char *repeat(ch, n)
int ch, n;
{
    char *p;
    int   i;
    static char buf[ 256 ];

    p = buf;
    for (i = 0 ; i < n ; i++) *(p++) = ch ;
    *p = '\0';
    return buf;
}

print_head()
{
    int i, field, size;

    for (i = 0; i < field_count; i++) {
        field = field_lst_no[ i ];
        size  = field_lst_size[ i ];
        fprintf(outf,"%-*.*s ", size, size, field_name[ field ] );
    }
    fprintf(outf,"\n");
    for (i = 0; i < field_count; i++) {
        field = field_lst_no[ i ];
        size  = field_lst_size[ i ];
        fprintf(outf,"%-*.*s ", size, size, repeat('=', size ));
    }
    fprintf(outf,"\n");
}

print_record()
{
    int i, field, size;

    for (i = 0 ; i < field_count; i++) {
        field = field_lst_no[ i ];
        size  = field_lst_size[ i ];
        fprintf(outf,"%-*.*s ", size, size, field_str[ field ] );
    }
    fprintf(outf,"\n");
}

char *my_ctime(t)
time_t *t;
{
    static char time_str[ 80 ];
    strcpy( time_str, (char *)ctime( t ) );
    time_str[ strlen( time_str ) - 1 ] = '\0';
    return time_str ;
}

dump_record(serial_no, p)
int serial_no;
struct userec *p;
{
    int i = 0, j ;
    int pat;

    /* the order of sprint should follow the order of list_idx[] */
  
    sprintf( field_str[ i++ ], "%d", serial_no );
    sprintf( field_str[ i++ ], "%s", p->userid );
    sprintf( field_str[ i++ ], "%s", p->lasthost );
    sprintf( field_str[ i++ ], "%d", p->numlogins );
    sprintf( field_str[ i++ ], "%d", p->numposts );
    sprintf( field_str[ i++ ], "%s", p->username );
    sprintf( field_str[ i++ ], "%s", my_ctime(&p->lastlogin) );
    sprintf( field_str[ i++ ], "%d", p->lastlogin );
    sprintf( field_str[ i++ ], "%s", p->realname );
    sprintf( field_str[ i++ ], "%s", p->address );
    sprintf( field_str[ i++ ], "%s", p->email);
    sprintf( field_str[ i++ ], "%s", p->termtype );

    pat = p->userlevel;
    for ( j=0; j<31; j++, pat >>= 1) {
        field_str[ i ][ j ] = (pat &  1) ? '1' : '0' ;
    } 
    field_str[ i++ ][ j ] = '\0'; 
    
    sprintf( field_str[ i++ ], "%s", my_ctime(&p->firstlogin) );
    sprintf( field_str[ i++ ], "%d", p->firstlogin );
    sprintf( field_str[ i++ ], "%s", p->termtype + 16 );
    sprintf( field_str[ i++ ], "%s", p->ident );
}

main(argc, argv)
int  argc;
char *argv[];
{
    FILE *inf;
    int  i;

    if (argc < 2) {
        printf("Usage: %s %s\n", argv[ 0 ], "[XN] ....");
        printf("Example: %s %s\n", argv[ 0 ], "d3 i12 e30");
        printf("N is field width, X is one of the following char :\n");
    
        for (i=0; field_name[ i ]; i++) {
            printf("\t%c -> %20.20s (default size = %2d)\n", 
                field_idx[ i ], field_name[ i ], field_default_size[ i ] );
        }

        exit( 0 );
    } else {
        set_opt( argc, argv );
        sprintf(MYPASSFILE,"/home/bbs/.PASSWDS");
    }

    
    inf = fopen( MYPASSFILE, "rb" );
    if (inf == NULL) {
        printf("Error open %s\n", MYPASSFILE); 
        exit( 0 );
    }
    outf = fopen("./showuser.result","w+");
    if(outf == NULL){
       printf("Error open ./showuser.result");
       exit(0);
    }   
    print_head(); 

    for (i=0; ; i++) {
        if (fread(&aman, sizeof( aman ), 1, inf ) <= 0) break;
        dump_record(i,  &aman);    
        print_record();
    }

    fclose( inf );
    fclose( outf);
}
