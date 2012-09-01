#include <fcntl.h>
 
#define STRLEN          256
#define BRC_MAXSIZE     50000
#define BRC_MAXNUM      60
#define BRC_STRLEN      15

char    brc_name[ BRC_STRLEN ];
int     brc_num;
int     brc_list[ BRC_MAXNUM ];
int     brc_changed = 0;

char *
brc_getrecord( ptr, name, pnum, list )
char    *ptr, *name;
int     *pnum, *list;
{
    int         num;
    char        *tmp;

    strncpy( name, ptr, BRC_STRLEN );
    ptr += BRC_STRLEN;
    num = (*ptr++) & 0xff;
    tmp = ptr + num * sizeof( int );
    if( num > BRC_MAXNUM ) {
        num = BRC_MAXNUM;
    }
    *pnum = num;
    memcpy( list, ptr, num * sizeof( int ) );
    return tmp;
}

brc_initial( filename )
char    *filename;
{
    char        brc_buf[ BRC_MAXSIZE ], *ptr;
    int         fd, len, n;

    if( (fd = open( filename, O_RDONLY )) != -1 ) {
        len = read( fd, brc_buf, sizeof( brc_buf ) );
        close( fd );
        ptr = brc_buf;
        while( ptr < &brc_buf[ len ] ) {
            if( *ptr < 'A' || *ptr > 'z' )
                break;
            ptr = brc_getrecord( ptr, brc_name, &brc_num, brc_list );
            printf( "board: %s\t(%d)\n", brc_name, brc_num );
            for( n = 0; n < brc_num; n++ ) {
                printf( " %d", brc_list[n] );
            }
            printf( "\n" );
        }
    }
}

main( argc, argv )
char    *argv[];
{
    if( argc <= 1 ) {
        printf( "Usage: %s .boardrc\n", argv[0] );
    } else {
        brc_initial( argv[1] );
    }
}

