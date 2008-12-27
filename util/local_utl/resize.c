#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

FILE	*inf, *outf;
char	*inf_name, *outf_name;
int	inf_size, outf_size, t;
char	buf[ 0x10000 ];

int fsize( char *path )
{
     struct stat buf;

     if (stat( path, &buf )) {
         return 0;
     } else { 
         return buf.st_size;
     }
}

main(int argc, char *argv[])
{
    int len, record_num = 0;

    if (argc != 5) {
        printf("%s old_file old_size new_file new_size\n", argv[ 0 ] );
        exit( 0 );
    }

    inf_name = argv[ 1 ];
    inf_size = atoi( argv[ 2 ] );
    outf_name = argv[ 3 ];
    outf_size = atoi( argv[ 4 ] );

    inf = fopen( inf_name, "r" );
    outf = fopen( outf_name, "w" );

    if ( inf == NULL || outf == NULL ) {
        printf("Error open file \n");
        exit( 0 );
    }

    t = fsize( inf_name );
    printf("Old file: %s  %d = %d x %d + %d \n", 
        inf_name, t, inf_size, t / inf_size, t % inf_size);
    printf("New file: %s  %d = %d x %d + %d \n", 
        outf_name, 0, outf_size, 0, 0);

    if ( t % inf_size ) {
        char ans[ 80 ];
        printf("Cannot match alignment, continue? ");
        gets( ans );
        if (!( ans[ 0 ] == 'Y' || ans[ 0 ] == 'y' )) exit( 0 ); 
    }

    memset( buf, 0, outf_size );

    while ( (len = fread( buf, 1, inf_size, inf )) == inf_size ) {
        record_num++;
        fwrite( buf, 1, outf_size, outf );
    }

    fclose( inf );
    fclose( outf );

    printf("After resize:\n");
    printf("len = %d, record_num = %d\n", len, record_num );

    t = fsize( inf_name );
    printf("Old: %s, %d x %d = %d (%d) %s\n", 
        inf_name, record_num, inf_size, inf_size * record_num,
        t, t == inf_size * record_num ? "OK" : "Error");

    t = fsize( outf_name );
    printf("New: %s, %d x %d = %d (%d) %s\n", 
        outf_name, record_num, outf_size, outf_size * record_num,
        t, t == outf_size * record_num ? "OK" : "Error");
}
