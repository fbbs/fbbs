#include <stdio.h>
#include <pwd.h>

/*#define BBSUSERONLY 
/**/ 

#ifdef BBSUSERONLY
char *localusers[] = { "root","daemon", NULL};
char *localpasswd = "/etc/localpasswd";
#endif

#define BINMAIL "/bin/mail"
#define BBSMAIL "/usr/local/sendmail/bbs/bbsmail"

extern int errno;
void main(argc,argv)
int argc;
char *argv[];
{
   extern int optind;
   int c;

   while ((c = getopt(argc, argv, "df:r:")) != EOF)
        switch(c) {
	   case 'd':               
	   case 'f':
	   case 'r':                  
	   case '?':
	   default:
	   break;
        }
   if (argv[argc] != NULL) 
       argv[argc] = NULL;
   mailit(optind,argv);
   perror("mailit");
   exit(errno);
}

char *mygetpwnam(name)
char *name;
{
#ifdef BBSUSERONLY
  char **ptr = localusers;
  FILE *fp;
  while (*ptr) {
    if (!strcmp(name,*ptr)) return *ptr;
    ptr++;
  }
  fp = fopen(localpasswd,"r");
  if (fp != NULL) {
      char entry[256];
      char *pentry;
      while (fgets(entry,255,fp) != NULL) {
	if ((pentry= (char*)strchr(entry,':')) != NULL) {
	    *pentry = NULL;
	}
        if (!strcmp(name,entry)) return name;
      }
  }
  return NULL;
#else
  return (char*)getpwnam(name);
#endif
}

mailit(optind,argv)
int optind;
char *argv[];
{
   char *name = argv[optind];
   char *rfc931name = argv[optind+1];
   int dobinmail=0;
    

   if (!name) dobinmail = 1;
   if (name && mygetpwnam(name)) dobinmail = 1;
   if (dobinmail) {
      argv[0] = BINMAIL;
      if (rfc931name != NULL) {
	 argv[optind+1] = NULL;
      }
      execv(BINMAIL,argv);
      perror("execv");
      exit(errno);
   } else {
      int len = strlen(name);
      if ((len>4 && strcmp(&name[len-4],".bbs")) || len <= 4) {
         char *bbsusername;
         bbsusername = (char*)malloc( len + sizeof(".bbs") + 1);
         sprintf(bbsusername,"%s.bbs",name);
         argv[optind] = bbsusername;
      } 
      argv[0] = BBSMAIL;
      execv(BBSMAIL,argv);
      perror("execv");
      exit(errno);
   }
}

