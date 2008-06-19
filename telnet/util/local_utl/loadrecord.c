
#include <stdio.h>
#include <time.h>
#include <rpcsvc/rstat.h>

main()
{
    	double cpu_load[3];
    	char load_buf[200];
    	int load;
	FILE *fp;
	char buf[30];
	int b1, b2, b3, b4;

    	get_load( cpu_load );
    	sprintf( load_buf, 
            "%.2f %.2f %.2f", 
            cpu_load[0], cpu_load[1], cpu_load[2] );
	
	fp = popen( "rfinger |grep \"文章\" | wc -l", "r" );
	fgets( buf, 29, fp );
	b1 = atoi( buf );
	pclose( fp );
	fp = popen( "rfinger |grep \"Talk\" | wc -l", "r" );
	fgets( buf, 29, fp );
	b2 = atoi( buf );
	pclose( fp );
	fp = popen( "rfinger |grep \"公布栏\"| wc -l", "r" );
	fgets( buf, 29, fp );
	b3 = atoi( buf );
	pclose( fp );
	fp = popen( "rfinger | grep -v \"文章\" |grep -v \"Talk\" |grep -v \"公布栏\" | wc -l", "r" );
	fgets( buf, 29, fp );
	b4 = atoi( buf );
	pclose( fp );

	sprintf( load_buf, "%s (R:%d T:%d A:%d M:%d)", load_buf, b1, b2, b3, b4 );
	log_string( load_buf );
}

void log_string( const char *str )
{
    char *strtime;
    FILE *fp;
    int current_time;

    current_time = time(0);
    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fp = fopen( "x", "a" );
    fprintf( fp, "%s : %s\n", strtime, str );
    fclose( fp );
    return;
}

void get_load( double load[] )
{
    struct statstime rs;
    rstat( "localhost", &rs );
    load[ 0 ] = rs.avenrun[ 0 ] / (double) (1 << 8);
    load[ 1 ] = rs.avenrun[ 1 ] / (double) (1 << 8);
    load[ 2 ] = rs.avenrun[ 2 ] / (double) (1 << 8);
    return;
}

