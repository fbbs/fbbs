#include <stdio.h>
#include <string.h>
#include <time.h>

main()
{
  FILE *fp1, *fp2;
  char temp[400];
  int telnet,auth,chat,finger,ftp,gopher,pop3,smtp,www;
  time_t now;

  if ((fp1 = fopen("/var/adm/syslogs/daemon.log", "r")) == NULL) {
    printf("Error: Cannot open input file.\n");
  }
  else {
    telnet=0;
    while (fgets(temp,398,fp1) != NULL)
    {
      if(strstr(temp," telnetd["))
        telnet++;
    }  
    fclose(fp1);
  }
  if ((fp1 = fopen("/var/adm/syslogs/local.log", "r")) == NULL) {
    printf("Error: Cannot open input file.\n");
    exit(1);
  }
  if ((fp2 = fopen("/root/report.tmp", "w")) == NULL) {
    printf("Error: Cannot write to target file.\n");
    exit(1);
  }
  auth=0;
  chat=0;
  finger=0;
  ftp=0;
  gopher=0;
  pop3=0;
  smtp=0;
  www=0;
  time(&now);
  fprintf(fp2, "Reply-To: SYSOP.bbs@MSIA.pine.ncu.edu.tw\n");
  fprintf(fp2, "From: SYSOP.bbs@MSIA.pine.ncu.edu.tw\n");
  fprintf(fp2, "Subject: MSIA system security report.\n\n");
  fprintf(fp2, "System security report produced at %s\n",ctime(&now));
  while (fgets(temp,398,fp1) != NULL)
  {
    if(strstr(temp,"auth from"))
      auth++;
    else if(strstr(temp,"bbs-chatd from"))
      chat++;
    else if(strstr(temp,"finger from"))
      finger++;
    else if(strstr(temp,"ftp from"))
      ftp++;
    else if(strstr(temp,"gopher from"))
      gopher++;
    else if(strstr(temp,"pop3 from"))
      pop3++;
    else if(strstr(temp,"smtp from"))
      smtp++;
    else if(strstr(temp,"www from"))
      www++;
    else if(!strstr(temp,"last message repeated")
            && !strstr(temp,"MSIA.pine.ncu.edu.tw")    
            && !strstr(temp,"innbbsd from"))
      fputs(temp,fp2);
  }
  fprintf(fp2,"\n\n     bbs connections: %d\n",telnet);
  fprintf(fp2,"chatroom connections: %d\n",chat);
  fprintf(fp2,"  finger connections: %d\n",finger);
  fprintf(fp2,"     ftp connections: %d\n",ftp);
  fprintf(fp2,"  gopher connections: %d\n",gopher);
  fprintf(fp2,"   ident connections: %d\n",auth);
  fprintf(fp2,"    pop3 connections: %d\n",pop3);
  fprintf(fp2,"    smtp connections: %d\n",smtp);
  fprintf(fp2,"     www connections: %d\n",www);  
  fclose(fp1);
  fclose(fp2);
}

