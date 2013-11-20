#include <time.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int a[32][18];  //雷
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
// added by soff 显示所有的雷
int show_mines() {
 int x, y;
 for(x=1; x<=30; x++)
      for(y=1; y<=16; y++)
      {
       if (a[x][y]) {
           move(x*2-2, y-1);
           //% prints("[1;31m雷[m");
           prints("[1;31m\xc0\xd7[m");
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
    screen_clear();
    refresh();
    while(1) {
        screen_clear();
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

/* 双键 */
int dblclick(int x, int y) {
    int dx, dy;
    if(x<1|| x>30|| y<1|| y>16) return;
    if(!o[x][y]) return;
    if(num_mine_beside(x, y)!=num_mark_beside(x, y)) return; 
    for(dx=x-1;dx<=x+1;dx++)
    for(dy=y-1;dy<=y+1;dy++)
        windig(dx, dy);
}

/* 左键 */
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

/* 显示[x][y]处 */
int winsh(int x, int y) {
    move(x*2-2, y-1);
    winsh0(x, y); 
}

/* 同上, 加快速度 */
int winsh0(int x, int y) {
    int c, d;
    static char word[9][10]= {
        //% "·", "１", "２", "３", "４", "５", "６", "７", "８"
        "\xa1\xa4", "\xa3\xb1", "\xa3\xb2", "\xa3\xb3", "\xa3\xb4", "\xa3\xb5", "\xa3\xb6", "\xa3\xb7", "\xa3\xb8"
    };
    static int cc[9]= {38, 37, 32, 31, 33, 35, 36, 40, 39};  
    char buf[100];
    if (!o[x][y]&&!m[x][y]) {
        //% prints("※");
        prints("\xa1\xf9");
        return;
    }
    if (m[x][y]) {
        //% prints("●");
        prints("\xa1\xf1");
        return;
    }
    if (a[x][y]) {
        //% prints("[1;31m雷[m"); 
        prints("[1;31m\xc0\xd7[m"); 
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
        //% sprintf(buf, "时间: %ld ", time(0)-t0);
        sprintf(buf, "\xca\xb1\xbc\xe4: %ld ", time(0)-t0);
        prints(buf);
        move(40, 20);
        //% sprintf(buf, "标记: %d ", marked);
        sprintf(buf, "\xb1\xea\xbc\xc7: %d ", marked);
        prints(buf);
        move(0, 21);
        //% sprintf(buf, "坐标: %3d, %3d", x, y);
        sprintf(buf, "\xd7\xf8\xb1\xea: %3d, %3d", x, y);
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
            //% prints("祝贺你！你成功了！                    ");
            prints("\xd7\xa3\xba\xd8\xc4\xe3\xa3\xa1\xc4\xe3\xb3\xc9\xb9\xa6\xc1\xcb\xa3\xa1                    ");
            {  char buf[100];
               sprintf(buf, "finished in %ld s.", time(0)-t0);
               syslog(buf);
            }
            gameover= 0;
 	    win_checkrec(time(0)-t0);/* added by soff 进行排行检查 */
            return;
        }
        if(gameover) {
            move(0, 22);
            //% prints("很遗憾，你失败了... 再来一次吧！                                 ");
            prints("\xba\xdc\xd2\xc5\xba\xb6\xa3\xac\xc4\xe3\xca\xa7\xb0\xdc\xc1\xcb... \xd4\xd9\xc0\xb4\xd2\xbb\xb4\xce\xb0\xc9\xa3\xa1                                 ");
            {  char buf[100];
               sprintf(buf, "failed in %ld s.", time(0)-t0);
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
    screen_clear();
    move(0, 22);
    //% prints("[1;32m☆键盘扫雷☆[0;1m [[35m帮助: H[37m] [[36m退出: Q[37m] [[35m打开: F[37m] [[36m标雷: S][m\n\r");
    prints("[1;32m\xa1\xee\xbc\xfc\xc5\xcc\xc9\xa8\xc0\xd7\xa1\xee[0;1m [[35m\xb0\xef\xd6\xfa: H[37m] [[36m\xcd\xcb\xb3\xf6: Q[37m] [[35m\xb4\xf2\xbf\xaa: F[37m] [[36m\xb1\xea\xc0\xd7: S][m\n\r");
    prints("[v1.00 by zhch.nju 00.3] press \'[1;32mH[m\' to get help, \'[1;32mCtrl+C[m\' to exit.");
    for(y=1; y<=16; y++) { 
        move(0, y-1);
        for(x=1; x<=30; x++)
            winsh0(x, y);
    }
    refresh();  
}

int winhelp() {
    screen_clear();
    //% prints("==欢迎来玩键盘扫雷游戏==  (程序由 nju BBS 站长 zhch 设计)\r\n---------------------------------\\r\n\r\n");
    prints("==\xbb\xb6\xd3\xad\xc0\xb4\xcd\xe6\xbc\xfc\xc5\xcc\xc9\xa8\xc0\xd7\xd3\xce\xcf\xb7==  (\xb3\xcc\xd0\xf2\xd3\xc9 nju BBS \xd5\xbe\xb3\xa4 zhch \xc9\xe8\xbc\xc6)\r\n---------------------------------\\r\n\r\n");
    //% prints("玩法很简单，和[1;34mwindows[m下的鼠标扫雷差不多.\r\n");
    prints("\xcd\xe6\xb7\xa8\xba\xdc\xbc\xf2\xb5\xa5\xa3\xac\xba\xcd[1;34mwindows[m\xcf\xc2\xb5\xc4\xca\xf3\xb1\xea\xc9\xa8\xc0\xd7\xb2\xee\xb2\xbb\xb6\xe0.\r\n");
        //% prints("  '[1;32mF[m'键的作用相当于鼠标的左键及双击的作用， 程序根据你点击的位置\r\n");
        prints("  '[1;32mF[m'\xbc\xfc\xb5\xc4\xd7\xf7\xd3\xc3\xcf\xe0\xb5\xb1\xd3\xda\xca\xf3\xb1\xea\xb5\xc4\xd7\xf3\xbc\xfc\xbc\xb0\xcb\xab\xbb\xf7\xb5\xc4\xd7\xf7\xd3\xc3\xa3\xac \xb3\xcc\xd0\xf2\xb8\xf9\xbe\xdd\xc4\xe3\xb5\xe3\xbb\xf7\xb5\xc4\xce\xbb\xd6\xc3\r\n");
        //% prints("  自动判断要进行哪种操作。\r\n");
        prints("  \xd7\xd4\xb6\xaf\xc5\xd0\xb6\xcf\xd2\xaa\xbd\xf8\xd0\xd0\xc4\xc4\xd6\xd6\xb2\xd9\xd7\xf7\xa1\xa3\r\n");
        //% prints("  '[1;32mS[m'键则相当于鼠标右键的功能, 可用来标雷.\r\n");
        prints("  '[1;32mS[m'\xbc\xfc\xd4\xf2\xcf\xe0\xb5\xb1\xd3\xda\xca\xf3\xb1\xea\xd3\xd2\xbc\xfc\xb5\xc4\xb9\xa6\xc4\xdc, \xbf\xc9\xd3\xc3\xc0\xb4\xb1\xea\xc0\xd7.\r\n");
        //% prints("  '[1;32mH[m'键用来显示本帮助信息.\r\n");
        prints("  '[1;32mH[m'\xbc\xfc\xd3\xc3\xc0\xb4\xcf\xd4\xca\xbe\xb1\xbe\xb0\xef\xd6\xfa\xd0\xc5\xcf\xa2.\r\n");
        //% prints("  '[1;32mQ[m'键退出游戏.\r\n");
        prints("  '[1;32mQ[m'\xbc\xfc\xcd\xcb\xb3\xf6\xd3\xce\xcf\xb7.\r\n");
        //% prints("  当屏幕乱掉时，可用'[1;32mD[m'可用来刷新屏幕。\r\n");
        prints("  \xb5\xb1\xc6\xc1\xc4\xbb\xc2\xd2\xb5\xf4\xca\xb1\xa3\xac\xbf\xc9\xd3\xc3'[1;32mD[m'\xbf\xc9\xd3\xc3\xc0\xb4\xcb\xa2\xd0\xc2\xc6\xc1\xc4\xbb\xa1\xa3\r\n");
        //% prints("建议用[1;32mNetterm[m来玩(当然njuterm也可以,:)),[1;32mtelnet[m效果不是太好\r\n");
        prints("\xbd\xa8\xd2\xe9\xd3\xc3[1;32mNetterm[m\xc0\xb4\xcd\xe6(\xb5\xb1\xc8\xbbnjuterm\xd2\xb2\xbf\xc9\xd2\xd4,:)),[1;32mtelnet[m\xd0\xa7\xb9\xfb\xb2\xbb\xca\xc7\xcc\xab\xba\xc3\r\n");
        //% prints("第一次点击一定会开一片，很舒服吧。\r\n");
        prints("\xb5\xda\xd2\xbb\xb4\xce\xb5\xe3\xbb\xf7\xd2\xbb\xb6\xa8\xbb\xe1\xbf\xaa\xd2\xbb\xc6\xac\xa3\xac\xba\xdc\xca\xe6\xb7\xfe\xb0\xc9\xa1\xa3\r\n");
        //% prints("熟练后，速度还是很快的，几乎可以达到鼠标扫雷的速度.\r\n");
        prints("\xca\xec\xc1\xb7\xba\xf3\xa3\xac\xcb\xd9\xb6\xc8\xbb\xb9\xca\xc7\xba\xdc\xbf\xec\xb5\xc4\xa3\xac\xbc\xb8\xba\xf5\xbf\xc9\xd2\xd4\xb4\xef\xb5\xbd\xca\xf3\xb1\xea\xc9\xa8\xc0\xd7\xb5\xc4\xcb\xd9\xb6\xc8.\r\n");
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
        fscanf(fp, "%s %d %s\n", topID[n], &topT[n], topFROM[n]);
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
    screen_clear();
    //% prints("[44;37m                         --== 扫雷排行榜 ==--                             \r\n[m");
    prints("[44;37m                         --== \xc9\xa8\xc0\xd7\xc5\xc5\xd0\xd0\xb0\xf1 ==--                             \r\n[m");
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
    screen_clear();
    //% prints("祝贺! 您刷新了自己的纪录!\r\n");
    prints("\xd7\xa3\xba\xd8! \xc4\xfa\xcb\xa2\xd0\xc2\xc1\xcb\xd7\xd4\xbc\xba\xb5\xc4\xbc\xcd\xc2\xbc!\r\n");
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

int screen_clear() {
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
    screen_clear();
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
