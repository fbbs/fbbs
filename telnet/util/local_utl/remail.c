/*
*    ÇåÀíÐÅÏäµÄÐÅ¼þ£¬ËÑÑ°Ã¿Ò»¸öµµ°¸£¬È»ááÖØ½¨ .DIR£¬¿³µôÀ¬»øµµ¡£
*
*    $Id: remail.c 2 2005-07-14 15:06:08Z root $
*/

#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <dirent.h>
#include        <limits.h>
#include        "bbs.h"
#define BBSMAIL "/tmp/mail"
#define TRUE  1
#define FALSE 0

report()
{
}

main(argc,argv)
int argc;
char *argv[];
{
   myftw(BBSMAIL) ;
}

int myftw(pathname)
char *pathname ;
{
   DIR *dp ;
   struct dirent *dirp ;
   if(pathname == NULL){
	printf(" ÇëÐÞ¸Ä BBSMAIL ÎªÕýÈ·µÄÄ¿Â¼£¬ ²»ÄÜÎª¿Õ¡£");
	return 0;
   }
   if((dp = opendir(BBSMAIL)) == NULL)
   {
      printf("\nÎÞ·¨¿ªÆôÄ¿Â¼ %s£¡", pathname) ;
      return 0;
   }
   while(dirp = readdir(dp))
   {
      char workpath[256] ;
      char userpath[256] ;
      DIR *workdp, *userdp;
      struct dirent *workdirp, *userdirp;
      char dot_dir[256], dot_dir_bak[256] ;
      
      if(!strcmp(dirp->d_name, ".") ||
         !strcmp(dirp->d_name, "..")
/* || dirp->d_type != DT_DIR */ )
      {
         continue ;
      }
      printf("\n½øÈëÄ¿Â¼ %s/%s ....\n", pathname, dirp->d_name) ;
      sprintf(workpath, "%s/%s", pathname, dirp->d_name) ;
      if( (workdp = opendir(workpath)) == NULL )
      {
         printf("\nÎÞ·¨¿ªÆôÄ¿Â¼ %s£¡", workpath) ;
         return 0 ;
      }
      while(workdirp = readdir(workdp))
      {
         int mailcounts = 0;
         int allactions = 0;
         
         if(!strcmp(workdirp->d_name, ".") ||
            !strcmp(workdirp->d_name, "..") 
/* || dirp->d_type != DT_DIR */  )
         {
            continue ;
         }
         printf("\no¸üÐÂ¡¾%-.12s¡¿µÄÐÅ¼þ", workdirp->d_name) ;
         sprintf(userpath, "%s/%s", workpath, workdirp->d_name) ;
         if( (userdp = opendir(userpath)) == NULL )
         {
            printf("\n      ÎÞ·¨¿ªÆô %s µÄÐÅ¼þÄ¿Â¼£¡", userpath) ;
            continue ;
         }
         sprintf(dot_dir, "%s/.DIR", userpath) ;
         sprintf(dot_dir_bak, "%s.bak", dot_dir) ;
         rename(dot_dir, dot_dir_bak) ;
         printf("\n ==> ÕûÀí %s ÐÅ¼þ£¬ÖØ½¨ .DIR µµ£¡", workdirp->d_name) ;
         while( (userdirp = readdir(userdp)) != NULL)
         {
            if( !strcmp(userdirp->d_name, ".") ||
                !strcmp(userdirp->d_name, "..") ||
                userdirp->d_name[0] != 'M')
            {
               continue ;
            }
            mailcounts += do_remail(userpath, userdirp->d_name, dot_dir,workdirp->d_name) ;
            allactions++ ;
         }
         closedir(userdp) ;
         chown(dot_dir, 9999, 999) ;
         printf("\n ==> ¹²ÖØ½¨ %d ÆªÐÅ¼þ£¬ %dÆªÐÅ¼þÊ§°Ü¡£\n", mailcounts, allactions - mailcounts) ;
         mailcounts = 0 ;
         allactions = 0 ;
      }
      closedir(workdp) ;
   }
   closedir(dp) ;
   printf("\nËùÓÐ×ÊÁÏ×ª»»Íê³É\n") ;
}

int do_remail(path, file, dot_dir, username)
   char *path ;
   char *file ;
   char *dot_dir ;
   char *username;
{
   FILE *fp ;
   char filename[256] ;
   struct fileheader fh ;
   struct stat st;
   int step = 0 , file_size, msgbackup=0;
   char buf[256] ;
   char *ptr, *ptr2 ;
   
   sprintf(filename, "%s/%s", path, file) ;
//   if( (fp = fopen(filename, "r")) == NULL)
   if( stat(filename, &st) == -1 ) {
      printf("\n         ÎÞ·¨¿ªÆôµµ°¸ %s£¡", file) ;
      return 0;
   } else file_size=st.st_blksize*st.st_blocks;
   fp = fopen(filename, "r");
   strncpy(fh.filename, file, sizeof(fh.filename)) ; /* ÌîÈëµµÃû */
   memcpy(fh.filename+STRLEN-5,&file_size,4);

   fh.level = 0 ;
   memset(fh.accessed, 0, sizeof(fh.accessed)) ;
   while ( fgets(buf, 256, fp) != NULL)
   {
      if( strstr(buf,"[0;1;44;36m") ||
          strstr(buf,"[1;32;40mTo") ) msgbackup = 1;
      if(strstr(buf,"·¢ÐÅÈË: ") ||
         strstr(buf,"×÷  Õß: ") ||
         strstr(buf,"¼ÄÐÅÈË: "))
      {
         ptr = &buf[8] ;
         ptr2 = strchr(ptr, ' ') ;
         if(ptr2 != NULL) *ptr2 = '\0' ;
         
         ptr2 = strchr(ptr, '@') ;
         if(ptr2 != NULL) *ptr2 = '\0' ;
         
         ptr2 = strchr(ptr, '.') ;
         if(ptr2 != NULL) *(ptr2+1) = '\0' ;
         
         strncpy(fh.owner, ptr, sizeof(fh.owner)) ;
         printf("#") ;
         step = 1 ;
      }
      if(strstr(buf, "±ê  Ìâ: ")||strstr(buf,"Ìâ  Ä¿: "))
      {
         ptr = &buf[8] ;
         ptr[strlen(ptr) - 1] = 0 ;
         strncpy(fh.title, ptr, sizeof(fh.title)) ;
         step = 2 ;
      }
      if(step == 2)
         break ;
   }
   fclose(fp) ;
   if(step != 2)
   {
      if(msgbackup) {	
         strncpy(fh.title, "ËùÓÐÑ¶Ï¢±¸·Ý",sizeof(fh.title));
         strncpy(fh.owner, username, sizeof(fh.owner)) ;
      } else {
         strncpy(fh.title, "²¹¾ÈÎÄ¼þ£¬Èç²»ÐèÒªÇëÊÖ¹¤É¾³ý£¡",sizeof(fh.title));
         strncpy(fh.owner, "UnkownUser", sizeof(fh.owner)) ;
     }
   }
   append_record(dot_dir, &fh, sizeof(fh)) ;
   return 1;
      
/*
   if(step == 2)
   {
      fh.filename[ STRLEN - 1] = 'S' ;
      fh.filename[ STRLEN - 2] = 'S' ;
      append_record(dot_dir, &fh, sizeof(fh)) ;
      
      return 1 ;
   }
   else
   {
      unlink(filename) ;
      return 0 ;
   }
*/
}
