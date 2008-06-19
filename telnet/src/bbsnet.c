/*bbsnet.c*/
// NJU bbsnet, preview version, zhch@dii.nju.edu.cn, 2000.3.23 //
// HIT bbsnet, Changed by Sunner, sun@bbs.hit.edu.cn, 2000.6.11

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
//#include <termio.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/telnet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <math.h>

#include "bbs.h"

char host1[100][40], host2[100][40], ip[100][40];
int port[100], counts= 0;

char datafile[80]= BBSHOME"/etc/bbsnet.ini";
char userid[80]=   "unknown.";

init_data() {
    FILE *fp;
    char t[256], *t1, *t2, *t3, *t4;
    fp= fopen(datafile, "r");
    if(fp== NULL) return;
    while(fgets(t, 255, fp)&& counts <= 72) {
        t1= strtok(t, " \t");
        t2= strtok(NULL, " \t\n");
        t3= strtok(NULL, " \t\n");
        t4= strtok(NULL, " \t\n");
        if(t1[0]== '#'|| t1== NULL|| t2== NULL|| t3== NULL) continue;
        strncpy(host1[counts], t1, 16);
        strncpy(host2[counts], t2, 36);
        strncpy(ip[counts], t3, 36);
        port[counts]= t4? atoi(t4): 23;
        counts++;
    }
    fclose(fp);
}

#ifdef FDQUAN	//¸´µ©ÈªµÄ´©Ëó³ÌÐò
char str[]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
sh(int n) {
    static oldn= -1;
    if(n>= counts) return;
    if(oldn >=0) {
        locate(oldn);
        printf("[1;32m %c.[m%s", str[oldn], host2[oldn]);
    }
    oldn= n;
    printf("[22;3H[1;37mµ¥Î»: [1;33m%s                   [22;32H[1;37m Õ¾Ãû: [1;33m%s              \r\n", host1[n], host2[n]);
    printf("[1;37m[23;3HÁ¬Íù: [1;33m%s                   [21;1H", ip[n]);
    locate(n);
    printf("[%c][1;42m%s[m", str[n], host2[n]);
}

show_all() {
    int n;
    printf("[H[2J[m");
    printf("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥[1;35m ´©  Ëó  Òø  ºÓ [m©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·\r\n");
    for(n= 1; n< 22; n++)
        printf("©§                                                                            ©§\r\n");
    printf("©§                                                               [1;36m°´[1;33mCtrl+C[1;36mÍË³ö[m ©§\r\n");
    printf("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿");
    printf("[21;3H©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤");
    for(n= 0; n< counts; n++) {
        locate(n);
        printf("[1;32m %c.[m%s", str[n], host2[n]);
    }
}

locate(int n) {
    int x, y;
    char buf[20];
    if(n>= counts) return;
/*    y= n% 17+ 3;
    x= n/ 17* 25+ 4;*/
    y= n/3 + 3;
    x= n % 3 * 25+ 4;

    sprintf(buf, "[%d;%dH", y, x);
    printf(buf);
}

int getch()
{
    int c, d, e;
    static lastc= 0;
    c= getchar();
    if(c== 10&& lastc== 13) c=getchar();
    lastc= c;
    if(c!= 27) return c;
    d= getchar();
    e= getchar();
    if(d== 27) return 27;
    if(e== 'A') return 257;
    if(e== 'B') return 258;
    if(e== 'C') return 259;
    if(e== 'D') return 260;
    return 0;
}

void main_loop() 
{
    int p= 0;
    int c, n;
L:
    show_all();
    sh(p);
    fflush(stdout);
    while(1) {
        c= getch();
                alarm(60);
        if(c== 3|| c== 4|| c== 27|| c< 0) break;
        if(c== 257) //ÏòÉÏ
        {
            if (p < 3){
		p += 3 * ( (counts -1)/3 - (p > ((counts -1)%3)));
	    } else p -= 3;
        }
        if(c== 258) //ÏòÏÂ
        {
            if (p+3 > counts - 1)
                p %= 3;
	    else p += 3;
        }
        if(c== 259) //ÏòÓÒ
        {
	    p ++;
	    if(p > counts - 1) p = 0;
        }
        if(c== 260) //Ïò×ó
        {
	    p --;
	    if(p<0)p = counts -1;
        }
        if(c== 13|| c== 10) {
			alarm(0);
            bbsnet(p);
            SetQuitTime();
            goto L;
        }
        for(n=0; n< counts; n++) if(str[n]== c) p= n;
        sh(p);
        fflush(stdout);
    }
}
#endif

int bbsnet(int n)
{
    if(n>= counts) return -1;
    printf("[H[2J[1;32mo Á¬Íù: %s (%s)\r\n", host2[n], ip[n]);
    printf("%s\r\n\r\n[m", "o Á¬²»ÉÏÊ±ÇëÉÔºò£¬30 Ãëºó½«×Ô¶¯ÍË³ö");
    fflush(stdout);
    proc(host2[n],ip[n], port[n]);
	return 0;
}

void QuitTime()
{
    reset_tty();
    exit(0);
}

int SetQuitTime()
{
	signal(SIGALRM, QuitTime);
	alarm(60);
	return 0;
}

main(int n, char* cmd[]) {
        SetQuitTime();
    get_tty();
    init_tty();
    if(n>= 2) strcpy(datafile, cmd[1]);
    if(n>= 3) strcpy(userid, cmd[2]);
//    if(n>= 4) strcpy(userid, cmd[3]);
    init_data();

#ifdef FDQUAN
    main_loop();
#else
	bbsnet(0);
#endif

    printf("[m");
    reset_tty();
}

bbs_syslog(char* s) { //rename by money for conflict with syslog() 2004.01.07
        char buf[512], timestr[16], *thetime;
        time_t dtime;
        FILE *fp;
        fp=fopen("reclog/bbsnet.log", "a");
        time(&dtime);
        thetime = (char*) ctime(&dtime);
        strncpy(timestr, &(thetime[4]), 15);
        timestr[15] = '\0';
        sprintf(buf,"%s %s %s\n", userid, timestr, s) ;
        fprintf(fp, buf);
        fclose(fp);
}

#define stty(fd, data) tcsetattr( fd, TCSANOW, data )
#define gtty(fd, data) tcgetattr( fd, data )
struct termios tty_state, tty_new ;

get_tty()
{
    if(gtty(1,&tty_state) < 0)
        return 0;
    return 1 ;
}

init_tty()
{
    long vdisable;

    memcpy( &tty_new, &tty_state, sizeof(tty_new)) ;
    tty_new.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ISIG);
    tty_new.c_cflag &= ~CSIZE;
    tty_new.c_cflag |= CS8;
    tty_new.c_cc[ VMIN ] = 1;
    tty_new.c_cc[ VTIME ] = 0;
    if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0)
      {
      tty_new.c_cc[VSTART] = vdisable;
      tty_new.c_cc[VSTOP]  = vdisable;
      tty_new.c_cc[VLNEXT] = vdisable;
      }
    tcsetattr(1, TCSANOW, &tty_new);
}

reset_tty()
{
    stty(1,&tty_state) ;
}

proc(char *hostname, char *server,int port)
{
        int fd;
        struct sockaddr_in blah;
        struct hostent *he;
        int result;
        unsigned char buf[2048];
        fd_set readfds;
        struct timeval tv;
        signal(SIGALRM, QuitTime);
        alarm(30);
        memset((char *)&blah,0,sizeof(blah));
        blah.sin_family=AF_INET;
        blah.sin_addr.s_addr=inet_addr(server);
        blah.sin_port=htons(port);
        fflush(stdout);
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if ((he = gethostbyname(server)) != NULL)
                bcopy(he->h_addr, (char *)&blah.sin_addr, he->h_length);
            else
                if ((blah.sin_addr.s_addr = inet_addr(server)) < 0) return;
            if(connect(fd,(struct sockaddr *)&blah,16)<0) return;

        signal(SIGALRM, SIG_IGN);
        printf("ÒÑ¾­Á¬½ÓÉÏÖ÷»ú£¬°´'ctrl+]'¿ìËÙÍË³ö¡£\n");
	sprintf(buf, "%s (%s)", hostname, server);
	bbs_syslog(buf);//rename by money for conflict with syslog() 2004.01.07

        while(1)
        {
                tv.tv_sec = 2400;
                tv.tv_usec = 0;
                FD_ZERO(&readfds) ;
                FD_SET(fd,&readfds);
                FD_SET(0,&readfds);

                result = select(fd + 1,&readfds,NULL,NULL,&tv);
                if (result <= 0) break;
                if (FD_ISSET(0,&readfds))
                {
                        result = read(0,buf,2048);
                        if (result<=0) break;
                        if(result==1&&(buf[0]==10||buf[0]==13)){
				buf[0]=13;
				buf[1]=10;
				result=2;
			}
                        if(buf[0]==29){close(fd);return;}
                        write(fd,buf,result);
                }
                else
                {
                        result=read(fd,buf,2048);
                        if (result<=0) break;
                        if(strchr(buf, 255))
                          telnetopt(fd, buf,result);
                        else
                          write(0,buf,result);
                }
        }
}

int telnetopt(int fd, char* buf, int max)
{
                unsigned char c,d,e;
                int pp=0;
                unsigned char tmp[30];
                while(pp<max)
                {
                   c=buf[pp++];
                   if(c==255)
                   {
                      d=buf[pp++];
                      e=buf[pp++];
                      fflush(stdout);
                      if((d==253)&&(e==3||e==24)) {
                       tmp[0]=255;tmp[1]=251;tmp[2]=e;write(fd,tmp,3);continue;}
                      if((d==251||d==252)&&(e==1||e==3||e==24)) {
                       tmp[0]=255;tmp[1]=253;tmp[2]=e;write(fd,tmp,3);continue;}
                      if(d==251||d==252) {
                       tmp[0]=255;tmp[1]=254;tmp[2]=e;write(fd,tmp,3);continue;}
                      if(d==253||d==254) {
                       tmp[0]=255;tmp[1]=252;tmp[2]=e;write(fd,tmp,3);continue;}
                      if(d==250) {
                         while(e!=240&&pp<max)e=buf[pp++];
                         tmp[0]=255;tmp[1]=250;tmp[2]=24;tmp[3]=0;tmp[4]=65;
                         tmp[5]=78;tmp[6]=83;tmp[7]=73;tmp[8]=255;tmp[9]=240;
                         write(fd,tmp,10);
                      }
                   }
                   else
                      write(0,&c,1);
                }
}

