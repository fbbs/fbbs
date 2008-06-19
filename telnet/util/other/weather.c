/* 
 * predict.c: auto weather predict 
 * 
 * Author: Chain.bbs@bbs.sjtu.edu.cn 
 * 
 * 
 * Copyright (C) 2000-2001 Sjtu BBS Dev Group. <bbs.sjtu.edu.cn> 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 

 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
  
 * $Id: weather.c 2 2005-07-14 15:06:08Z root $ 
 * 
 *
 * modified by roly 2002/04/27
 * gcc -O2 -L/usr/ucblib -L/usr/lib -lucb -lsocket -lnsl -o weather weather.c 
 */ 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/stat.h> 
#include <sys/un.h> 
#include <netinet/in.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <strings.h> 
#define BBSHOME "./" 
#define LOCALTMPFILE "predict.tmp" 
#define TMPPATH "/home/bbs/tmp/" 
#define LOCALFILE "weather" 
#define DEST_IP "202.106.103.7" 
#define DEST_PORT 80 
#define DEST_PORT 80 
#define LINELEN 84 
int 
read_stream(int fd, char *ptr, int maxbytes) 
{ 
 int nleft, nread; 
 nleft = maxbytes; 
 while (nleft > 0) { 
  if ( (nread = read(fd, ptr, nleft)) < 0) 
   return(nread);  /* error, return < 0 */ 
  else if (nread == 0) 
   break;    /* EOF, return #bytes read */ 
  nleft -= nread; 
  ptr   += nread; 
 } 
 return(maxbytes - nleft);  /* return >= 0 */ 
} 
int get_predict(){ 
 int fd,len,flag;/*flag mean save to the file have begin*/ 
 FILE* lfd; 
 int lastpos; 
 struct sockaddr_in i_a,my_addr; 
 int pos=-1; 
 char tmpfile[40]; 
 char reply[401]; 
 int  havefeedback; 
 int n; 
 static char request[] = "GET /cgi-bin/cgi200010/query?03 \r\n\r\n"; 

 if ((fd=socket(AF_INET,SOCK_STREAM,0))<0){ 
  printf("socket create error"); 
  return -1; 
 } 

 memset(&i_a,0,sizeof(struct sockaddr_in)); 
 i_a.sin_family   = AF_INET; 
 i_a.sin_addr.s_addr = inet_addr(DEST_IP); 
 i_a.sin_port   = htons(DEST_PORT); 

 if ((connect(fd,(struct sockaddr*)&i_a,sizeof(struct sockaddr)))==-1){ 
  printf("connect error\n"); 
  return -2; 
 } 
 if (send(fd, request, strlen(request),0)==-1){ 
  printf("send error"); 
  return -3; 
 } 
 strcpy(tmpfile,TMPPATH); 

 strcat(tmpfile,LOCALTMPFILE); 
 if ((lfd=fopen(tmpfile,"w"))==NULL){ 
  printf("create tmp file error\n"); 
 printf("%s\n",tmpfile); return -4; 
 } 

  while ((n = read_stream(fd, reply, 400)) > 0) { 
   fwrite(reply,n,1,lfd); 
 } 

 close(fd); 
 fclose(lfd); 

 write_valid_weather_file(tmpfile); 
 if (n < 0) 
  printf("read error\n"); 
 unlink(tmpfile);
 exit(0); 
} 
int write_valid_weather_file(char* tmpfile) 
{ 
 FILE *fp; 
 char rdstring[401]; 
 char *rtval; 
 int flag=0; 
 int totalcount=33; 
 int citycount=0; 
 int i,windlen; 
 char genbuf[LINELEN]; 
 int startpoint[6]={12,22,32,37,44,60}; 
 fp=fopen(tmpfile,"r"); 
 if (fp==NULL) 
 { 
  printf("Open temp file failed\n"); 
  return 1; 
 } 
 while (flag!=2) 
 { 
  if (flag==0) 
  { 
   rtval=fgets(rdstring,400,fp); 
   if (rtval==NULL) 
   { 
    return -1; 
   } 
   else 
   { 
    if (strstr(rdstring,"ÔÂ")!=NULL && strstr(rdstring,"ÈÕ")!=NULL && strstr 

(rdstring,"Ê±")!=NULL && strstr(rdstring,"ÖÁ")!=NULL) 
    { 
     printf("\n                         ¹úÄÚ³ÇÊÐ24Ð¡Ê±ÌìÆøÔ¤±¨\n"); 
     printf("                    %s\n",rdstring);/*ÄêÔÂÈÕ*/ 
     printf("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·\n"); 
     printf("©§  ³Ç ÊÐ       ÌìÆøÏÖÏó         ÎÂ¶È£¨C£©            ·çÏò·çÁ¦              ©§\n"); 
     printf("©§           Ò¹¼ä      °×Ìì     ×îµÍ ×î¸ß      Ò¹¼ä            °×Ìì         ©§\n"); 
     printf("©Ä©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ì\n"); 
    } 
    else if ((rtval=(char*)strstr(rdstring,"±±¾©"))!=NULL) 
    { 
     for ( i=0;i<LINELEN;i++)genbuf[i]=' '; 
    strncpy(genbuf,"©§", 2);
     strcpy(genbuf+4,"±±¾©"); 
     genbuf[8]=' '; 
     flag=1; 
     for (i=0;i<6;i++) 
     for (i=0;i<6;i++) 
     { 
      fgets(rdstring,400,fp); 
      rtval=(char*)strstr(rdstring+71,"</"); 
      if (rtval!=NULL) 
      { 
       rdstring[rtval-rdstring]='\0'; 
       strcpy(genbuf+startpoint[i],rdstring+71); 
       genbuf[startpoint[i]+rtval-rdstring-71]=' '; 
      } 
     } 
     genbuf[75]=' ';
     strcpy(genbuf+76,"©§"); 
     printf("%s\n",genbuf); 
     fgets(rdstring,400,fp); 
     fgets(rdstring,400,fp); 
    } 
   } 
  } 
  else if (flag==1) 
  { 
   for (;totalcount>0;totalcount--) 
   { 
    for ( i=0;i<LINELEN;i++)genbuf[i]=' '; 

    strncpy(genbuf,"©§",2);
    fgets(rdstring,400,fp); 
    rtval=(char*)strstr(rdstring+88,"</"); 
    rdstring[rtval-rdstring]='\0'; 
    strcpy(genbuf+3,rdstring+88); 
    genbuf[3+rtval-rdstring-88]=' '; 
    for (i=0;i<6;i++) 
    { 
      fgets(rdstring,400,fp); 
      rtval=(char*)strstr(rdstring+71,"</"); 
      if (rtval!=NULL) 
      { 
       rdstring[rtval-rdstring]='\0'; 
       strcpy(genbuf+startpoint[i],rdstring+71); 
        genbuf[startpoint[i]+rtval-rdstring-71]=' '; 
      } 
    }
    genbuf[75]=' '; 
    strcpy(genbuf+76,"©§"); 
    if(totalcount==33)//ÉÏº££¬Ê¹ÓÃÌØÊâµÄÑÕÉ« 
    { 
     printf("©§");
     printf("[1;34;47m"); 
     genbuf[76]='\0'; 
     printf("%s",genbuf+2); 
     printf("[m©§"); 
     printf("\n"); 
    } 
    else 
    { 
     printf("%s\n",genbuf); 
    } 
    fgets(rdstring,400,fp); 
    fgets(rdstring,400,fp); 
   } 
   flag=2; 
  } 
 } 
 fclose(fp); 
 printf("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿\n"); 
 printf("                 ÆøÏóÐÅÏ¢È¡×Ô ÖÐ¹úÆøÏóÔÚÏß(www.nmc.gov.cn) \n"); 
 return 0; 
} 

int main() 
{ 
 get_predict(); 
 return 0; 
} 
