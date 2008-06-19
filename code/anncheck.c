#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BBSDISKPATH "/home/bbs/0Announce/groups/"
#define LOGPATH "/home/bbs/0Announce/groups/system.faq/Yyzabdc/auto/"
#define DOTNAMES ".Names"
#define MAXQUEUEDEEP 8192
#define MAXDIRFILES 1024
#define DIRPATHLEN 1024
#define STRLEN 255

char arrDirs[MAXQUEUEDEEP][DIRPATHLEN];
int dirLayers[MAXQUEUEDEEP];
int intArrDirsHead;
int intArrDirsTail;
FILE *noexist, *toolong, *toodeep, *loop;

//判断一个路径是否存在且是目录，如果是存在的目录则返回1，否则返回0。
int isDir(char *strFileName){
	struct stat st;
	return ( stat( strFileName, &st ) == 0 && S_ISDIR( st.st_mode ) );
}

//判断一个路径是否存在，如果是存在的则返回1，否则返回0。
int isExist(char *strPath){
	struct stat st;
	return ( stat( strPath, &st ) == 0 );
}

//检查一个目录
convertDir(int intDirIndex){

	char strBuf[STRLEN], strHtmDirName[STRLEN], strPath[STRLEN], *ptr;
	FILE *fl;

	//.Names中读出文件列表
	sprintf( strBuf, "%s/%s", arrDirs[ intDirIndex ], DOTNAMES );
	if( fl = fopen( strBuf, "rt" ) ){

		while( !feof( fl ) ){

			//得到下一行的内容
			fgets( strBuf, STRLEN, fl );
			if( feof( fl ) ) break;

			//取得一个文件显示名，确保格式的正确性
			strBuf[ strlen( strBuf ) - 1 ] = '\0';
			if( ( ptr = strstr( strBuf, "Name=" ) ) && strlen( strBuf ) > 5 ){

				//得到紧接着 Name= 的那一行的内容
				fgets( strBuf, STRLEN, fl );
				if( feof( fl ) ) break;
				strBuf[ strlen( strBuf ) - 1 ] = '\0';

				//取得一个文件名，格式应该是 Path=~/ ， 根据是否包含 .. 剔除循环目录
				if( ( ptr = strstr( strBuf, "Path=~/" ) ) && strlen( strBuf ) > 7 ){
					strcpy( strPath, ptr + 7 );
					if( strstr( strPath, "/" ) || strstr( strPath, "\\" ) ){
						//给出精华区目录出错报告
						fprintf( loop, "索引文件有循环目录：%s\n\n", arrDirs[ intDirIndex ] );
					}else{

						//生成全路径
						sprintf( strBuf , "%s/%s", arrDirs[ intDirIndex ], strPath );

						//判断这个文件是否是目录。如果是目录则放入待处理目录队列中，否则放入待处理文档队列中。
						if( isDir( strBuf ) ){
							if( strlen( strBuf ) > 130 )
								fprintf( toolong, "路径名过长：%s\n\n", strBuf );
							//将目录名写入待处理目录队列
							strcpy( arrDirs[ intArrDirsTail ], strBuf );
							dirLayers[ intArrDirsTail ] = dirLayers[ intDirIndex ] + 1;
							if( dirLayers[ intArrDirsTail ] > 9 )
								fprintf( toodeep, "目录层次太深：%s\n\n", strBuf );
							intArrDirsTail ++;
						}else if( isExist( strBuf ) ){
							if( strlen( strBuf ) > 130 )
								fprintf( toolong, "路径名过长：%s\n\n", strBuf );
						}else{
							//错误目录项报告
							fprintf( noexist, "不存在的路径：%s\n\n", strBuf );
						}

					}
				}else if( strlen( strBuf ) == 7 ){
					//给出精华区空目录报告
					fprintf( noexist, "索引文件有空目录：%s\n\n", arrDirs[ intDirIndex ] );
				}
			}

		}

	}

	intArrDirsHead ++;

	//关闭.Names
	fclose( fl );

}

//遍历一个版的精华区
void convertBoard(char *strBoardName){

	char strBuf[STRLEN];

	intArrDirsHead = 0;
	intArrDirsTail = 0;
	sprintf( strBuf, "%s%s", BBSDISKPATH, strBoardName );
	strcpy( arrDirs[0], strBuf );
	dirLayers[0] = 0;
	intArrDirsTail ++;

	while( intArrDirsHead < intArrDirsTail )
		convertDir(intArrDirsHead);

}

//遍历一个区的所有版面
void convertPart(char *strPartName){

	char strBuf[STRLEN], strBoardName[STRLEN];
	FILE *fl;

	//用ls将目录中的文件名列表存入BBSDISKPATH中的list_txt文件中
	sprintf( strBuf, "ls %s%s > %sboard_txt", BBSDISKPATH, strPartName, BBSDISKPATH );
	system( strBuf );

	//从list_txt中读出文件列表
	sprintf( strBuf, "%sboard_txt", BBSDISKPATH );
	if( fl = fopen( strBuf, "rt" ) )
		while( !feof( fl ) ){

			//取得一个字符串
			fgets( strBuf, STRLEN, fl );
			if( feof( fl ) ) break;

			//得到包含区名的版名，及版的路径名
			strBuf[ strlen(strBuf) - 1 ] = '\0';
			sprintf( strBoardName , "%s/%s", strPartName, strBuf );
			sprintf( strBuf , "%s%s", BBSDISKPATH, strBoardName );

			//判断这个文件是否是目录。如果是目录则调用convertBoard，检查这个版的精华区。
			if( isDir( strBuf ) ) convertBoard( strBoardName );

		}

	//关闭list_txt
	fclose(fl);

}

main(){

	char strBuf[STRLEN];

	//打开存放查询结果的各个文件
	sprintf( strBuf, "%snoexist", LOGPATH );
	if( ( noexist = fopen(strBuf, "w") ) == NULL ) exit(0);
	fprintf( noexist, "目录列表始：\n\n" );

	sprintf( strBuf, "%stoolong", LOGPATH );
	if( ( toolong = fopen(strBuf, "w") ) == NULL ) exit(0);
	fprintf( toolong, "目录列表始：\n\n" );

	sprintf( strBuf, "%stoodeep", LOGPATH );
	if( ( toodeep = fopen(strBuf, "w") ) == NULL ) exit(0);
	fprintf( toodeep, "目录列表始：\n\n" );

	sprintf( strBuf, "%sloop", LOGPATH );
	if( ( loop = fopen(strBuf, "w") ) == NULL ) exit(0);
	fprintf( loop, "目录列表始：\n\n" );

	//检查所有精华区的目录
	convertPart( "system.faq" );
	convertPart( "ccu.faq" );
	convertPart( "comp.faq" );
	convertPart( "rec.faq" );
	convertPart( "music.faq" );
	convertPart( "literal.faq" );
	convertPart( "sport.faq" );
	convertPart( "talk.faq" );
	convertPart( "news.faq" );
	convertPart( "sci.faq" );
	convertPart( "other.faq" );
	convertPart( "soc.faq" );

	//关闭文件
	fprintf( noexist, "目录列表终。\n\n" );
	fclose(noexist);
	fprintf( toolong, "目录列表终。\n\n" );
	fclose(toolong);
	fprintf( toodeep, "目录列表终。\n\n" );
	fclose(toodeep);
	fprintf( loop, "目录列表终。\n\n" );
	fclose(loop);
}
