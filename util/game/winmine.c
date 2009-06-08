#include <time.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int a[32][18];  //À×
int m[32][18];  //marked
int o[32][18];  //opened
char topID[20][20],topFROM[20][32];
char userid[20]="unknown.", fromhost[20]="unknown.";
int topT[20], gameover=0;
static char buf[10000]; // output buffer
struct termios oldtty, newtty; 

int main(int n, char* cmd[]) {
    tcgetattr(0, &oldtty);
    //cfmakeraw(&newtty);
    init_tty();
    tcsetattr(0, TCSANOW, &newtty);
    if(n>=2) strcpy(userid, cmd[1]);
    if(n>=3) strcpy(fromhost, cmd[2]);
    syslog("ENTER");
    winmine();
    tcsetattr(0, TCSANOW, &oldtty);
}
// added by soff ÏÔÊ¾ËùÓÐµÄÀ×
int show_mines() {
 int x, y;
 for(x=1; x<=30; x++)
      for(y=1; y<=16; y++)
      {
       if (a[x][y]) {
           move(x*2-2, y-1);
           prints("[1;31mÀ×[m");
          }
         }
        return;
}

init_tty()
{
    long vdisable;

    memcpy( &newtty, &oldtty, sizeof(newtty)) ;
    newtty.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ISIG);
    newtty.c_cflag &= ~CSIZE;
    newtty.c_cflag |= CS8;
    newtty.c_cc[ VMIN ] = 1;
    newtty.c_cc[ VTIME ] = 0;
    if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0)
      {
      newtty.c_cc[VSTART] = vdisable;
      newtty.c_cc[VSTOP]  = vdisable;
      newtty.c_cc[VLNEXT] = vdisable;
      }
    tcsetattr(1, TCSANOW, &newtty);
}

int winmine() {
    int x,y;
    win_showrec();
    clear();
    refresh();
    while(1) {
        clear();
        for (x=0;x<=31;x++)
        for (y=0;y<=17;y++) {
             a[x][y]= 0;
             m[x][y]= 0;
             o[x][y]= 0;
        }
        winrefresh();
        winloop();
        pressanykey();
    }
}

int num_mine_beside(int x1, int y1) {
    int dx, dy, s;
    s= 0;
    for(dx= x1-1; dx<=x1+1; dx++)
    for(dy= y1-1; dy<=y1+1; dy++)
        if(!(dx==x1&&dy==y1)&&a[dx][dy]) s++;
    return s; 
}

int num_mark_beside(int x1, int y1) {
    int dx, dy, s;
    s= 0;
    for(dx= x1-1; dx<=x1+1; dx++)
    for(dy= y1-1; dy<=y1+1; dy++)
        if(!(dx==x1&&dy==y1)&&m[dx][dy]) s++;
    return s;    
}

int wininit(int x1, int y1) {
    int n, x, y;
    randomize();
    for(n=1; n<=99; n++) {
        do {
            x= rand()%30 +1;
            y= rand()%16 +1;
        }
        while(a[x][y]!=0||(abs(x-x1)<2&&abs(y-y1)<2));
        a[x][y]=1;
    }
}

/* Ë«¼ü */
int dblclick(int x, int y) {
    int dx, dy;
    if(x<1|| x>30|| y<1|| y>16) return;
    if(!o[x][y]) return;
    if(num_mine_beside(x, y)!=num_mark_beside(x, y)) return; 
    for(dx=x-1;dx<=x+1;dx++)
    for(dy=y-1;dy<=y+1;dy++)
        windig(dx, dy);
}

/* ×ó¼ü */
int windig(int x, int y) {
    int dx, dy;
    if(x< 1|| x> 30|| y< 1|| y> 16) return;
    if(o[x][y]||m[x][y]) return;
    o[x][y]=1;
    winsh(x, y);
    if(a[x][y]) {
	show_mines();
         gameover=1;
         return;
    }
    if(num_mine_beside(x, y)==0) {
        for(dx=x-1;dx<=x+1;dx++)
        for(dy=y-1;dy<=y+1;dy++)
            windig(dx, dy);
    }
}

/* ÏÔÊ¾[x][y]´¦ */
int winsh(int x, int y) {
    move(x*2-2, y-1);
    winsh0(x, y); 
}

/* Í¬ÉÏ, ¼Ó¿ìËÙ¶È */
int winsh0(int x, int y) {
    int c, d;
    static char word[9][10]= {
        "¡¤", "£±", "£²", "£³", "£´", "£µ", "£¶", "£·", "£¸"
    };
    static int cc[9]= {38, 37, 32, 31, 33, 35, 36, 40, 39};  
    char buf[100];
    if (!o[x][y]&&!m[x][y]) {
        prints("¡ù");
        return;
    }
    if (m[x][y]) {
        prints("¡ñ");
        return;
    }
    if (a[x][y]) {
        prints("[1;31mÀ×[m"); 
        return;  
    } 
    c= num_mine_beside(x, y);
    d= 1;
    if(c==0) d=0; 
    sprintf(buf, "[%d;%dm%s[m", d, cc[c], word[c]); 
    prints(buf);  
}

int winloop()
{
    int x, y, c, marked, t0, inited;
    char buf[100];
    x= 10;
    y= 8;
    inited= 0; 
    marked= 0; 
    clearbuf();
    t0= time(0);
    while(1) {
        c= egetch();
        if((c==257||c=='k')&&y>1) y--;
        if((c==258||c=='j')&&y<16) y++;
        if((c==260||c=='h')&&x>1) x--;
        if((c==259||c=='l')&&x<30) x++;
        move(0, 20);
        sprintf(buf, "Ê±¼ä: %d ", time(0)-t0);
        prints(buf);
        move(40, 20);
        sprintf(buf, "±ê¼Ç: %d ", marked);
        prints(buf);
        move(0, 21);
        sprintf(buf, "×ø±ê: %3d, %3d", x, y);
        prints(buf);
        move(x*2-2, y-1);
        if(c=='H') winhelp();
        if(c=='d'|| c=='D') winrefresh();
        if(c=='f'|| c=='F'){
             if(!inited) {
                  wininit(x, y);
                  inited= 1;
             } 
             dig(x, y);      
        } 
        if((c==83|| c==115)&&!o[x][y]) {
             if(m[x][y]){
                  m[x][y]=0;
                  marked--;
             } else {
                  m[x][y]=1;
                  marked++;
             }
             winsh(x, y);
        }
        if(checkwin()==1) {
            move(0, 22);
            prints("×£ºØÄã£¡Äã³É¹¦ÁË£¡                    ");
            {  char buf[100];
               sprintf(buf, "finished in %d s.", time(0)-t0);
               syslog(buf);
            }
            gameover= 0;
 	    win_checkrec(time(0)-t0);/* added by soff ½øÐÐÅÅÐÐ¼ì²é */
            return;
        }
        if(gameover) {
            move(0, 22);
            prints("ºÜÒÅº¶£¬ÄãÊ§°ÜÁË... ÔÙÀ´Ò»´Î°É£¡                                 ");
            {  char buf[100];
               sprintf(buf, "failed in %d s.", time(0)-t0);
               syslog(buf);
            }
            gameover= 0;
            return;
        }
        move(x*2-2, y-1);
        refresh();
    }
}

int checkwin() {
    int x,y,s;
    s=0;
    for(x=1; x<=30; x++)
    for(y=1; y<=16; y++)
        if(!o[x][y])s++;
    if(s==99) return 1;
    return 0;
}

int dig(int x, int y) {
    if (!o[x][y]) 
           windig(x, y);
    else 
           dblclick(x, y);
}

int winrefresh() {
    int x, y;
    clear();
    move(0, 22);
    prints("[1;32m¡î¼üÅÌÉ¨À×¡î[0;1m [[35m°ïÖú: H[37m] [[36mÍË³ö: Q[37m] [[35m´ò¿ª: F[37m] [[36m±êÀ×: S][m\n\r");
    prints("[v1.00 by zhch.nju 00.3] press \'[1;32mH[m\' to get help, \'[1;32mCtrl+C[m\' to exit.");
    for(y=1; y<=16; y++) { 
        move(0, y-1);
        for(x=1; x<=30; x++)
            winsh0(x, y);
    }
    refresh();  
}

int winhelp() {
    clear();
    prints("==»¶Ó­À´Íæ¼üÅÌÉ¨À×ÓÎÏ·==  (³ÌÐòÓÉ nju BBS Õ¾³¤ zhch Éè¼Æ)\r\n---------------------------------\\r\n\r\n");
    prints("Íæ·¨ºÜ¼òµ¥£¬ºÍ[1;34mwindows[mÏÂµÄÊó±êÉ¨À×²î²»¶à.\r\n");
        prints("  '[1;32mF[m'¼üµÄ×÷ÓÃÏàµ±ÓÚÊó±êµÄ×ó¼ü¼°Ë«»÷µÄ×÷ÓÃ£¬ ³ÌÐò¸ù¾ÝÄãµã»÷µÄÎ»ÖÃ\r\n");
        prints("  ×Ô¶¯ÅÐ¶ÏÒª½øÐÐÄÄÖÖ²Ù×÷¡£\r\n");
        prints("  '[1;32mS[m'¼üÔòÏàµ±ÓÚÊó±êÓÒ¼üµÄ¹¦ÄÜ, ¿ÉÓÃÀ´±êÀ×.\r\n");
        prints("  '[1;32mH[m'¼üÓÃÀ´ÏÔÊ¾±¾°ïÖúÐÅÏ¢.\r\n");
        prints("  '[1;32mQ[m'¼üÍË³öÓÎÏ·.\r\n");
        prints("  µ±ÆÁÄ»ÂÒµôÊ±£¬¿ÉÓÃ'[1;32mD[m'¿ÉÓÃÀ´Ë¢ÐÂÆÁÄ»¡£\r\n");
        prints("½¨ÒéÓÃ[1;32mNetterm[mÀ´Íæ(µ±È»njutermÒ²¿ÉÒÔ,:)),[1;32mtelnet[mÐ§¹û²»ÊÇÌ«ºÃ\r\n");
        prints("µÚÒ»´Îµã»÷Ò»¶¨»á¿ªÒ»Æ¬£¬ºÜÊæ·þ°É¡£\r\n");
        prints("ÊìÁ·ºó£¬ËÙ¶È»¹ÊÇºÜ¿ìµÄ£¬¼¸ºõ¿ÉÒÔ´ïµ½Êó±êÉ¨À×µÄËÙ¶È.\r\n");
        pressanykey();
        winrefresh();
}

int win_loadrec() {
    FILE *fp;
    int n;
    for(n=0; n<=19; n++) {
        strcpy(topID[n], "null.");
        topT[n]=999;
        strcpy(topFROM[n], "unknown.");
    }
    fp=fopen("mine2.rec", "r");
    if(fp==NULL) {
        win_saverec();
        return;
    }
    for(n=0; n<=19; n++)
        fscanf(fp, "%s %d %s\n", topID[n], &topT[n], &topFROM[n]);
    fclose(fp);
}

int win_saverec() {
    FILE *fp;
    int n;
    fp=fopen("mine2.rec", "w");
    for(n=0; n<=19; n++) {
        fprintf(fp, "%s %d %s\n", topID[n], topT[n], topFROM[n]);
    }
    fclose(fp);
}

int win_showrec() {
    char buf[200];
    int n;
    win_loadrec();
    clear();
    prints("[44;37m                         --== É¨À×ÅÅÐÐ°ñ ==--                             \r\n[m");
    prints("[41m No.          ID        TIME                         FROM                      [m\r\n");
    for(n=0; n<=19; n++) {
        sprintf(buf, "[1;37m%3d[32m%13s[0;37m%12d[m%29s\r\n", n+1, topID[n], topT[n], topFROM[n]);
        prints(buf);
    }
    sprintf(buf, "[41m                                                                               [m\r\n");
    prints(buf);
    pressanykey();
}

int win_checkrec(int dt) {
    char id[20];
    int n;
    win_loadrec();
    strcpy(id, userid);
    for(n=0; n<=19; n++)
    if(!strcmp(topID[n], id)) {
        if(dt< topT[n]) {
            topT[n]= dt;
            strcpy(topFROM[n], fromhost);
            win_sort();
            win_saverec();
        }
        return;
    }
    if(dt<topT[19]) {
        strcpy(topID[19], id);
        topT[19]= dt;
        strcpy(topFROM[19], fromhost);
        win_sort();
        win_saverec();
        return;
    }
}

int win_sort() {
    int n, n2, tmp;
    char tmpID[20];
    clear();
    prints("×£ºØ! ÄúË¢ÐÂÁË×Ô¼ºµÄ¼ÍÂ¼!\r\n");
    pressanykey();
    for(n=0; n<=18; n++)
    for(n2=n+1; n2<=19; n2++)
    if(topT[n]> topT[n2]) {
        tmp= topT[n];
        topT[n]= topT[n2];
        topT[n2]= tmp;
        strcpy(tmpID, topID[n]);
        strcpy(topID[n], topID[n2]);
        strcpy(topID[n2], tmpID);
        strcpy(tmpID, topFROM[n]);
        strcpy(topFROM[n], topFROM[n2]);
        strcpy(topFROM[n2], tmpID);
    }
}

int clear() {
    prints("[H[J");
}

int refresh() {
    write(0, buf, strlen(buf));
    buf[0]= 0;
}

int prints(char* b) {
    strcat(buf, b);
}

int randomize() {
    srandom(time(0));
}

int move(int x, int y) {
    char c[100];
    sprintf(c, "[%d;%dH", y+1, x+1);
    prints(c); 
}

int egetch() {
    int c,d,e;
    c=getch0();
    if(c==3||c==4||c==-1||c=='q'||c=='Q') quit();
    if(c!=27) return c;
    d=getch0();
    e=getch0();
    if(e=='A') return 257;
    if(e=='B') return 258;
    if(e=='C') return 259;
    if(e=='D') return 260;
    return 0;
}

int getch0() {
    char c;
    if(read(0, &c, 1)<=0) quit();
    return c; 
}

int quit() {
    tcsetattr(0, TCSANOW, &oldtty);
    clear();
    refresh();
    syslog("QUIT");
    exit(0); 
}

int pressanykey() {
    refresh();
    clearbuf(); 
}

int clearbuf() {
    char buf[128];
    refresh();
    read(0, buf, 100); 
}

syslog(char* cc )
{
   FILE *fp;
   time_t t;
   t=time(0);
   fp=fopen("winmine.log","a");
   fprintf(fp,"%s did %s on %s", userid, cc, ctime(&t));
   fclose(fp);
}
