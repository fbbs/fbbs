#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BBSDISKPATH "/export/disk1/bbs/0Announce/groups/"
#define WWWDISKPATH "/export/disk1/bbswww/htdocs/groups/"
#define TARWORKPATH "/bbsbackup/Anntar/"
#define TRASHPATH "/bbsbackup/anntrash/"
#define DOTNAMES ".Names"
#define DOTINDEX "index.htm"
#define MAXQUEUEDEEP 4096
#define MAXDIRFILES 1024
#define DIRPATHLEN 1024
#define STRLEN 255

char arrDirs[MAXQUEUEDEEP][DIRPATHLEN];
int intArrDirsHead;
int intArrDirsTail;

char arrFiles[MAXDIRFILES][STRLEN];
int intArrFilesCount;

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

//将一个整数longInt转换为一个长度为intLength的字符串strInt，长度为intLength
void longToStr( long longInt, char *strInt, int intLength){

	int intTail;

	strInt[ intLength ]='\0';
	while( intLength -- ){
		intTail = longInt % 10;
		longInt = longInt / 10;
		strInt[ intLength] = '0' + intTail;
	}

}

//将unix文件转换为htm文件
convertFiles(char *strHtmDirName){

	char strBuf[STRLEN], strTemp[STRLEN], ch;
	int intCount;
	FILE *fin, *fout;

	for( intCount = 0; intCount < intArrFilesCount; intCount ++ ){
		//将原来文件中的所有回车替换为<br>，所有空格替换为&nbsp;。暂时不做HTMLEncoding。
		fin = fopen( arrFiles[ intCount ], "rt" );
		longToStr( intCount, strTemp, 6 );
		sprintf( strBuf, "%s/%s.htm", strHtmDirName, strTemp );
		fout = fopen ( strBuf, "wt" );
		if( fin && fout ){
			//这里输入一些必须的头信息
			fprintf( fout, "<html>\n<head>\n" );
			fprintf( fout, "<title>复旦大学日月光华站∶精华区</title>\n" );
			fprintf( fout, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=GB2312\">\n" );
			fprintf( fout, "</head>\n\n" );
			fprintf( fout, "<body>\n" );
			fprintf( fout, "\n<center><h1>复旦大学日月光华站∶精华区</h1></center>\n\n" );
			while( !feof( fin ) ){
				ch = fgetc( fin );
				if( ch == '\n' ){
					fprintf( fout, "<br>\n" );
				}else if( ch == ' ' )
					fprintf( fout, "&nbsp;" );
				else if( ch == '\t' )
					fprintf( fout, "&nbsp;&nbsp;&nbsp;&nbsp;" );
				else
					fputc( ch, fout );
			}
			//这里输入一些必须的尾信息
			fprintf( fout, "\n<center><h1>复旦大学日月光华站∶精华区</h1></center>\n" );
			fprintf( fout, "</body>\n</html>\n" );
		}
		fclose( fout );
		fclose( fin );
	}

	intArrFilesCount = 0;

}

//转换一个目录，将目录中的所有文件转为htm，将所有目录压到队列中
convertDir(int intDirIndex){

	char strBuf[STRLEN], strHtmDirName[STRLEN], strName[STRLEN], strPath[STRLEN];
	char *ptr;
	FILE *fl, *indexFile;
	int intFlag;

	//建立htm目录
	longToStr( intDirIndex, strBuf, 6 );
	sprintf( strHtmDirName, "%s%s", TARWORKPATH, strBuf );
	sprintf( strBuf, "mkdir %s", strHtmDirName );
	system( strBuf );

	intFlag = 0;

	//打开index.htm文件
	sprintf( strBuf, "%s/%s", strHtmDirName, DOTINDEX );
	if( indexFile = fopen( strBuf, "wt" ) ){

		//这里输入一些必须的头信息
		fprintf( indexFile, "<html>\n<head>\n" );
		fprintf( indexFile, "<title>复旦大学日月光华站∶精华区</title>\n" );
		fprintf( indexFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=GB2312\">\n" );
		fprintf( indexFile, "</head>\n\n" );
		fprintf( indexFile, "<body>\n" );
		fprintf( indexFile, "\n<center><h1>复旦大学日月光华站∶精华区</h1></center>\n\n" );

		//.Names中读出文件列表
		sprintf( strBuf, "%s/%s", arrDirs[ intDirIndex ], DOTNAMES );
		if( fl = fopen( strBuf, "rt" ) ){

			while( !feof( fl ) ){

				//得到下一行的内容
				fgets( strBuf, STRLEN, fl );
				if( feof( fl ) ) break;

				//取得一个文件显示名，根据是否包含 (BM: ***) 剔除只有斑竹、总管、站长可见的内容
				strBuf[ strlen( strBuf ) - 1 ] = '\0';
				if( ( ptr = strstr( strBuf, "Name=" ) ) && strlen( strBuf ) > 5 
					&& !strstr( strBuf, "(BM: SYSOPS)" ) && !strstr( strBuf, "(BM: OBOARDS)" )
					&& !strstr( strBuf, "(BM: BMS)" ) && !strstr( strBuf, "(BM: SECRET)" ) ){
					strcpy( strName, ptr + 5 );

					//得到紧接着 Name= 的那一行的内容
					fgets( strBuf, STRLEN, fl );
					if( feof( fl ) ) break;
					strBuf[ strlen( strBuf ) - 1 ] = '\0';

					//取得一个文件名，格式应该是 Path=~/ ， 根据是否包含 .. 剔除循环目录
					if( ( ptr = strstr( strBuf, "Path=~/" ) ) && strlen( strBuf ) > 7 ){
						strcpy( strPath, ptr + 7 );
						if( !strstr( strPath, "/" ) && !strstr( strPath, "\\" ) && strPath[0] != '.' ){

							//生成全路径
							sprintf( strBuf , "%s/%s", arrDirs[ intDirIndex ], strPath );

							//判断这个文件是否是目录。如果是目录则放入待处理目录队列中，否则放入待处理文档队列中。
							if( isDir( strBuf ) ){
								//将目录名写入待处理目录队列
								strcpy( arrDirs[ intArrDirsTail ], strBuf );
								//将目录名写入index.htm中
								longToStr( intArrDirsTail, strBuf, 6 );
								fprintf( indexFile, "<a href=""../%s/%s"">%s</a><br>\n", strBuf, DOTINDEX, strName );
								intArrDirsTail ++;
								intFlag = 1;	//该目录不为空
							}else if( isExist( strBuf ) ){
								//将文档名写入待处理文档队列
								strcpy( arrFiles[ intArrFilesCount ], strBuf );
								//将文档名写入index.htm中
								longToStr( intArrFilesCount, strBuf, 6 );
								fprintf( indexFile, "<a href=""%s.htm"">%s</a><br>\n", strBuf, strName );
								intArrFilesCount ++;
								intFlag = 1;	//该目录不为空
							}

						}
					}

				}

			}

			convertFiles( strHtmDirName );

		}

		intArrDirsHead ++;

		//关闭.Names
		fclose( fl );

	}

	//这里输入一些必须的尾信息
	if( intFlag )
		fprintf( indexFile, "\n<center><h1>复旦大学日月光华站∶精华区</h1></center>\n" );
	else
		fprintf( indexFile, "\n<center>目前没有文章</center>\n" );
	fprintf( indexFile, "</body>\n</html>\n" );

	//关闭.index.htm
	fclose( indexFile );

}

//遍历一个版的精华区
void convertBoard(char *strBoardName){

	char strBuf[STRLEN];
	FILE *indexFile;

	intArrFilesCount = 0;
	intArrDirsHead = 0;
	intArrDirsTail = 0;
	sprintf( strBuf, "%s%s", BBSDISKPATH, strBoardName );
	strcpy( arrDirs[0], strBuf );
	intArrDirsTail ++;

	while( intArrDirsHead < intArrDirsTail )
		convertDir(intArrDirsHead);

	sprintf( strBuf, "%s%s", TARWORKPATH, DOTINDEX );
	if( indexFile = fopen( strBuf, "wt" ) ){

		//输入根目录的index.htm
		fprintf( indexFile, "<html><head>\n" );
		fprintf( indexFile, "<title>复旦大学日月光华站∶精华区</title>\n" );
		fprintf( indexFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=GB2312\">\n" );
		fprintf( indexFile, "<script>\n" );
		fprintf( indexFile, "function redirect(){window.location = \"000000/index.htm\"}\n" );
		fprintf( indexFile, "</script></head>\n" );
		fprintf( indexFile, "<body onload=\"redirect()\"></body></html>\n" );

	}
	fclose( indexFile );

	sprintf( strBuf, "%sgzipfile.txt", TRASHPATH );
	indexFile = fopen( strBuf, "wt" );
	fclose( indexFile );

	//将生成的精华区内容打包到anntrash
	sprintf( strBuf, "tar cfX - %sgzipfile.txt * |gzip >%s%s.html.tgz", TRASHPATH, WWWDISKPATH, strBoardName );
	system( strBuf );

	//清理Anntar目录
	sprintf( strBuf, "rm -r %s/*", TARWORKPATH );
	system( strBuf );

}

//遍历一个区的所有版面
void convertPart(char *strPartName){

	char strBuf[STRLEN], strBoardName[STRLEN];
	FILE *fl;

	//用ls将目录中的文件名列表存入TRASHPATH中的list_txt文件中
	sprintf( strBuf, "ls %s%s > %sboard_txt", BBSDISKPATH, strPartName, TRASHPATH );
	system( strBuf );

	//从list_txt中读出文件列表
	sprintf( strBuf, "%sboard_txt", TRASHPATH );
	if( fl = fopen( strBuf, "rt" ) )
		while( !feof( fl ) ){

			//取得一个字符串
			fgets( strBuf, STRLEN, fl );
			if( feof( fl ) ) break;

			//得到包含区名的版名，及版的路径名
			strBuf[ strlen(strBuf) - 1 ] = '\0';
			sprintf( strBoardName , "%s/%s", strPartName, strBuf );
			sprintf( strBuf , "%s%s", BBSDISKPATH, strBoardName );

			//判断这个文件是否是目录。如果是目录则调用convertBoard，对这个版打包。
			if( isDir( strBuf ) ) convertBoard( strBoardName );

		}

	//关闭list_txt
	fclose(fl);

}

main(){

	char strBuf[STRLEN];

	//0区的精华只对指定的一些版打包
	convertBoard( "system.faq/Announce" );
	convertBoard( "system.faq/BBS_Dev" );
	convertBoard( "system.faq/BBS_Help" );
	convertBoard( "system.faq/BM_Home" );
	convertBoard( "system.faq/Dispute" );
	convertBoard( "system.faq/New_Board" );
	convertBoard( "system.faq/Notice" );
	convertBoard( "system.faq/SysOp" );
	convertBoard( "system.faq/Test" );

//	convertBoard( "system.faq/Zzzzz" );

	//其他区的精华对整个区打包
//	convertPart( "system.faq" );
	convertPart( "ccu.faq" );
	convertPart( "campus.faq" );
	convertPart( "comp.faq" );
	convertPart( "rec.faq" );
	convertPart( "music.faq" );
	convertPart( "literal.faq" );
	convertPart( "sport.faq" );
	convertPart( "talk.faq" );
	convertPart( "news.faq" );
	convertPart( "sci.faq" );
//	convertPart( "other.faq" );
//	convertPart( "soc.faq" );

	//清理anntrash
	sprintf( strBuf, "rm -r %s/*", TRASHPATH );
	system( strBuf );

}
