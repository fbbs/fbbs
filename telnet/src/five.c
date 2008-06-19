/*  Îå×ÓÆå³ÌÊ½   Programmed by Birdman     */
/*  140.116.102.125 Á¬ÖéÍÛ¹þ¹þÐ¡Õ¾         */
/*  ³É´óµç»ú88¼¶                           */
#ifdef FIVEGAME

#include "bbs.h"
#include "screen.h"
#include <sys/socket.h>
#define black 1
#define white 2
#define FDATA "five"
#define b_lines 24
#define LCECHO (2)
#define cuser currentuser
#define setutmpmode(a) modify_user_mode( a )

int player,winner=0,quitf;
int px,py,hand,tdeadf,tlivef,livethree,threefour;
int chess[250][2]={0,0};
int playboard[15][15]={0,0};
char abcd[15]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O'};
extern int RMSG;

int five_pk(int, int);
void five_chat (char *, int);
void press (void);

void Box( int x, int y, int x1, int y1 )
{
    char *lt="©°", *rt="©´", *hor="©¤", *ver="©¦", *lb="©¸", *rb="©¼";
    int i;

    move( x, y );
    outs( lt );
    for( i = y+2; i <= y1-2; i+=2 )
        outs( hor );
    outs( rt );
    for( i = x+1; i <= x1-1; i++ )
    {
        move( i, y );
        outs( ver );
        move( i, y1 );
        outs( ver );
    }
    move( x1, y );
    outs( lb );
    for( i = y+2; i <= y1-2; i+=2 )
        outs( hor );
    outs( rb );
}


void InitScreen()
{
    int i;

    for (i = 0; i < 16; i ++) {
        move (i, 0);
        clrtoeol ();
    }
    move(0,0);
    outs(
"©°©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©Ð©´15\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È14\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È13\n"
"©À©à©à£«©à©à©à©à©à©à©à£«©à©à©È12\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È11\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È10\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È9\n"
"©À©à©à©à©à©à©à£«©à©à©à©à©à©à©È8\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È7\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È6\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È5\n"
"©À©à©à£«©à©à©à©à©à©à©à£«©à©à©È4\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È3\n"
"©À©à©à©à©à©à©à©à©à©à©à©à©à©à©È2\n"
"©¸©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©Ø©¼1\n"
"A B C D E F G H I J K L M N O");

    user_guide();
    move(0,33);
    outs("[35;43m¡ôÎå×ÓÆå¶ÔÕ½¡ô[30;42m  ³ÌÊ½:³É´óµç»ú88¼¶ Birdman  [m");
}

int user_guide()
{
    move(4,64);
    outs("ÇÐ»»:   Tab¼ü");
    move(5,64);
    outs("ÒÆ¶¯:  ·½Ïò¼ü");
    move(6,64);
    outs("      H,J,K,L");
    move(7,64);
    outs("ÏÂ×Ó:  ¿Õ¸ñ¼ü");
    move(8,64);
    outs("ÖØ¿ª:  N »òÕß");
    move(9,64);
    outs("       Ctrl+N");
    move(10,64);
    outs("ÍË³ö:  Q »òÕß");
    move(11,64);
    outs("       Ctrl+C");
    move(12,64);
    outs("  ºÚÏÈÓÐ½ûÊÖ");
    Box( 3,62,13,78 );
    move( 3,64 );
    outs( "ÓÃ·¨" );
}

void haha( int what )
{
    char *logo[3] = { " »îÈýà¶! ", "¹þ¹þ»îËÄ!", " Ð¡ÐÄ³åËÄ! " };

    move( 15, 64 );
    if (what >= 3)
        outs ("            ");
    else
        outs( logo[what] );
}

void win( int who )
{
    move(12,35);
    outs("[47m[31m©°¡ª¡ª¡ª¡ª©´[m");
    move(13,35);
    if( who == black )
        outs("[47m[31m©¦  [30;42mºÚÊ¤[m[47m [31m ©¦[m");
    else
        outs("[47m[31m©¦  [30;42m°×Ê¤[m[47m [31m ©¦[m");
    move(14,35);
    outs("[47m[31m©¸¡ª¡ª¡ª¡ª©¼[m");
    refresh ();
    winner=who;
    press ();
}

void quit(void)
{
    move(12,35);
    outs("[47m[31m©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
    move(13,35);
    outs("[47m[31m©¦  [30;42m¶Ô·½ÍË³öÁË[m[47m [31m ©¦[m");
    move(14,35);
    outs("[47m[31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
    refresh ();
    bell();
    press ();
}

int calvalue(int x1,int y1,
             int x2,int y2,
             int x3,int y3,
             int x4,int y4,
             int x5,int y5)
{
    int n_black,n_white,empty,i,j /* ,k */;

    n_black=n_white=empty=0;

    if(x1<0||x2<0||x3<0||x4<0||x5<0||
                x1>14||x2>14||x3>14||x4>14||x5>14)    return;
    if(winner!=0) return;
    if(playboard[x2][y2]==0 || playboard[x3][y3]==0
                || playboard[x4][y4]==0)
        empty=1; /*check 10111ÐÍËÀËÄ*/

    if(playboard[x1][y1]==black) n_black+=1;
    if(playboard[x1][y1]==white) n_white+=1;
    if(playboard[x2][y2]==black) n_black+=1;
    if(playboard[x2][y2]==white) n_white+=1;
    if(playboard[x3][y3]==black) n_black+=1;
    if(playboard[x3][y3]==white) n_white+=1;
    if(playboard[x4][y4]==black) n_black+=1;
    if(playboard[x4][y4]==white) n_white+=1;
    if(playboard[x5][y5]==black) n_black+=1;
    if(playboard[x5][y5]==white) n_white+=1;

    if(playboard[x1][y1]==0 && playboard[x5][y5]==0)
    {
        if(n_white==3 || n_black==3)
            haha(0);

        if(n_black==3)
            livethree+=1;
    }

    if((n_white==4 || n_black==4) && (empty ==1))
    {
        tdeadf+=1;
        tlivef+=1;
        haha(2);
        return;
    }

    if(n_black==5)      
    {                   /*ÔÙÉ¨Á¬Áù*/
        tlivef=-1;
        tdeadf=0;
        livethree=0;
        for(i=0;i<=14;i++ )    /*ËÄ×ÝÏò*/
            for(j=0;j<=9;j++)
                callfour(i,j,i,j+1,i,j+2,i,j+3,i,j+4,i,j+5);
        for(i=0;i<=9;i++)      /*ËÄºáÏò*/
            for(j=0;j<=14;j++)
                callfour(i,j,i+1,j,i+2,j,i+3,j,i+4,j,i+5,j);
        for(i=0;i<=9;i++)      /*ËÄÐ±ÓÒÏÂ*/
            for(j=0;j<=9;j++)
            {
                callfour(i,j,i+1,j+1,i+2,j+2,i+3,j+3,i+4,j+4,i+5,j+5);
                              /*ËÄÐ±×óÏÂ*/
                callfour(i,j+5,i+1,j+4,i+2,j+3,i+3,j+2,i+4,j+1,i+5,j);
            }
        if(winner==0)
            win( black );
    }
    if(n_white==5)
        win( white );
    return;
}

int callfour(int x1,int y1,int x2,int y2,int x3,int y3,
             int x4,int y4,int x5,int y5,int x6,int y6)
{
    int n_black,n_white,dead /* ,i,j,k */;

    n_black=n_white=dead=0;

    if(x1<0||x2<0||x3<0||x4<0||x5<0||x6<0||
                x1>14||x2>14||x3>14||x4>14||x5>14||x6>14)    return;

    if(winner!=0) return;

    if((playboard[x1][y1]!=0 && playboard[x6][y6]==0)||
                (playboard[x1][y1]==0 && playboard[x6][y6]!=0))
        dead=1;      /* for checking  ³åËÄ */

    if(playboard[x2][y2]==black) n_black+=1;
    if(playboard[x2][y2]==white) n_white+=1;
    if(playboard[x3][y3]==black) n_black+=1;
    if(playboard[x3][y3]==white) n_white+=1;
    if(playboard[x4][y4]==black) n_black+=1;
    if(playboard[x4][y4]==white) n_white+=1;
    if(playboard[x5][y5]==black) n_black+=1;
    if(playboard[x5][y5]==white) n_white+=1;

    if(playboard[x1][y1]==0 && playboard[x6][y6]==0 &&
                (playboard[x3][y3]==0 || playboard[x4][y4]==0))
    {
        if(n_black==3 || n_white==3)
            haha(0);
        if(n_black==3)
            livethree+=1;
    }


    if(n_black==4)
    {
        if(playboard[x1][y1]== black && playboard[x6][y6]== black)
             bandhand(6);
        if(playboard[x1][y1]!=0 && playboard[x6][y6]!=0) return;

        if(dead)
        {
/* add by satan Mar 19, 1999 start*/
            if (playboard[x1][y1]==0 && playboard[x5][y5]==0 ||
                    playboard[x2][y2]==0 && playboard[x6][y6]==0)
                livethree -= 1;
/* add by satan Mar 19, 1999 end*/

            haha(2);
            tdeadf+=1;
            tlivef+=1;      /*ºÚËÀËÄÀ²*/
            threefour=0;
            return;
        }

        threefour=black; 
        tlivef+=1;         /*»îËÄÒ²ËãË«ËÄ*/
    }
    if(n_white==4)
    {
        if(playboard[x1][y1]!=0 && playboard[x6][y6]!=0) return;
        if(dead)
        {
            haha(2);
            tdeadf+=1;
            threefour=0;
            return;
        }

        threefour=white;
        tlivef+=1;

    }
    if(playboard[x1][y1]==black) n_black+=1;     /*check Á¬×Ó*/
    if(playboard[x6][y6]==black) n_black+=1;

    if(n_black==5 && (playboard[x3][y3]==0 || playboard[x4][y4]==0 ||
                          playboard[x5][y5]==0 || playboard[x2][y2]==0))
        tlivef-=1;     /* ÁùÈ±Ò»ÐÍ, ²»Ëã³åËÄ */

    if( n_black >= 6 )
        bandhand(6);
    return;
}

int bandhand(int style)
{
    if(style==3)
    {
        move(12,35);
        outs("[47m[31m©°ºÚ°Ü¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
        move(13,35);
        outs("[47m[31m©¦  [37;41mºÚ½ûÊÖË«»îÈý[m[47m  [31m©¦[m");
        move(14,35);
        outs("[47m[31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
    }
        else if(style==4)
    {
        move(12,35);
        outs("[47m[31m©°ºÚ°Ü¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
        move(13,35);
        outs("[47m[31m©¦  [37;41mºÚ½ûÊÖË«  ËÄ[m[47m  [31m©¦[m");
        move(14,35);
        outs("[47m[31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
    }
    else
    {
        move(12,35);
        outs("[47m[31m©°ºÚ°Ü¡ª¡ª¡ª¡ª¡ª¡ª©´[m");
        move(13,35);
        outs("[47m[31m©¦  [37;41mºÚ½ûÊÖÁ¬Áù×Ó[m[47m  [31m©¦[m");
        move(14,35);
        outs("[47m[31m©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©¼[m");
    }

    winner=white;
    press ();
    return;
}

char save_page_requestor[40];

int five_pk(fd,first)
int fd;
int first;
{
    int cx, ch, cy,datac,fdone,x /* ,y */;
    char genbuf[100],data[90],xy_po[5],genbuf1[20] /* ,x1[1],y1[1],done[1] */;
/*    struct user_info *opponent; */
/*     char fname[50]; */
    int i,j /* ,k */,fway,banf,idone;


/*
 *      Ôö¼ÓÁÄÌì¹¦ÄÜ. Added by satan. 99.04.02
 */

#define START    17
#define END      21
#define PROMPT   23
#undef MAX
#define MAX      (END - START)
#define BSIZE    60


    char chatbuf[80], *cbuf;
    int ptr = 0, chating = 0 /*, over = 0 */;

    setutmpmode(FIVE);       /*ÓÃ»§×´Ì¬ÉèÖÃ*/
    clear ();
    InitScreen();
    five_chat (NULL, 1);

    cbuf = chatbuf + 19;
    chatbuf[0] = '\0';
    chatbuf[79] = '\0';
    cbuf[0] = '\0';
    sprintf (chatbuf + 1, "%-16s: ", cuser.username);

    add_io(fd, 0);

begin:
    for(i=0;i<=14;i++)
        for(j=0;j<=14;j++)
            playboard[i][j]=0;


    hand=1;
    winner=0;
    quitf=0;
    px=14;
    py=7;
    fway = 1;
    banf = 1;
    idone = 0;


    sprintf(genbuf, "%s (%s)", cuser.userid, cuser.username);

    if(first)
    {
        move(1,33);
        prints("ºÚ¡ñÏÈÊÖ %s  ",genbuf);
        move(2,33);
        prints("°×¡ðºóÊÖ %s  ",save_page_requestor);
    }
    else
    {
        move(1,33);
        prints("°×¡ðºóÊÖ %s  ",genbuf);
        move(2,33);
        prints("ºÚ¡ñÏÈÊÖ %s  ",save_page_requestor);
    }


    move(15,35);
    if(first)
        outs("¡ïµÈ´ý¶Ô·½ÏÂ×Ó¡ï");
    else
        outs("¡ôÏÖÔÚ¸Ã×Ô¼ºÏÂ¡ô");
    move(7,14);
    outs("¡ñ");
    player=white;
    playboard[7][7]=black;
    chess[1][0]=14; /*¼ÍÂ¼ËùÏÂÎ»Ö·*/
    chess[1][1]=7;
    move(4,35);
    outs("µÚ 1ÊÖ ¡ñH 8");

    if(!first) {      /*³¬¹Ö!*/
        move (7, 14);
        fdone=1;
    }
    else fdone=0;    /*¶ÔÊÖÍê³É*/

    while (1)
    {
        ch=igetkey();

        if (ch == I_OTHERDATA)
        {
            datac = recv(fd, data, sizeof(data), 0);
            if (datac <= 0)
            {
                move(17,30);
                outs("[47m[31;47m ¶Ô·½Í¶½µÁË...@_@ [m");
                break;
            }
            if (data[0] == '\0') {
                five_chat (data + 1, 0);
                if (chating)
                    move (PROMPT, ptr + 6);
                else
                    move (py, px);
                continue;
            } else if (data[0] == '\1') {
                bell ();
                RMSG = YEA;
                saveline (PROMPT, 0);
                sprintf (genbuf, "%s Ëµ: ÖØÀ´Ò»ÅÌºÃÂð? (Y/N)[Y]:", save_page_requestor);
                getdata (PROMPT, 0, genbuf, genbuf1, 2, LCECHO, YEA);
                RMSG = NA;
                if (genbuf1[0] == 'n' || genbuf1[0] == 'N') {
                    saveline (PROMPT, 1);
                    send (fd, "\3", 1, 0);
                    continue;
                } else {
                    saveline (PROMPT, 1);
                    InitScreen ();
                    first = 0;
                    send (fd, "\2", 1, 0);
                    goto begin;
                }
            } else if (data[0] == '\2') {
                bell ();
                saveline (PROMPT, 0);
                move (PROMPT, 0);
                clrtoeol ();
                prints ("%s ½ÓÊÜÁËÄúµÄÇëÇó :-)", save_page_requestor);
                refresh ();
                sleep (1);
                saveline (PROMPT, 1);
                InitScreen ();
                first = 1;
                goto begin;
            } else if (data[0] == '\3') {
                bell ();
                saveline (PROMPT, 0);
                move (PROMPT, 0);
                clrtoeol ();
                prints ("%s ¾Ü¾øÁËÄúµÄÇëÇó :-(", save_page_requestor);
                refresh ();
                sleep (1);
                saveline (PROMPT, 1);
                if (chating)
                    move (PROMPT, ptr + 6);
                else
                    move (py, px);
                continue;
            } else if (data[0] == '\xff') {
                move (PROMPT, 0);
                quit ();
                break;
            }
            i=atoi(data);
            cx=i/1000;   /*½âÒëdata³ÉÆåÅÌ×ÊÁÏ*/
            cy=(i%1000)/10;
            fdone=i%10;
            hand+=1;

            if(hand%2==0)
                move(((hand-1)%20)/2+4,48);
            else
                move(((hand-1)%19)/2+4,35);

            prints("µÚ%2dÊÖ %s%c%2d",hand,
                   (player==black)?"¡ñ":"¡ð",abcd[cx/2],15-cy);


            move(cy,cx);
            x=cx/2;
            playboard[x][cy]=player;
            if(player==black)
            {
                outs("¡ñ");
                player=white;
            }
            else
            {
                outs("¡ð");
                player=black;
            }
            move (cy, cx);
            refresh ();
            bell ();
            move(15,35);
            outs("¡ôÏÖÔÚ¸Ã×Ô¼ºÏÂ¡ô");
            haha (5);

            tdeadf=tlivef=livethree=threefour=0;
            for(j=0;j<=10;j++)
                calvalue(cx/2,j,cx/2,j+1,cx/2,j+2,cx/2,j+3,cx/2,j+4);
            for(i=0;i<=10;i++)/*ºáÏò*/
                calvalue(i,cy,i+1,cy,i+2,cy,i+3,cy,i+4,cy);
            for(i=-4;i<=0;i++)/*Ð±ÓÒÏÂ*/
                calvalue(cx/2+i,cy+i,cx/2+i+1,cy+i+1,cx/2+i+2,cy+i+2,
                         cx/2+i+3,cy+i+3,cx/2+i+4,cy+i+4);
            for(i=-4;i<=0;i++)/*Ð±×óÏÂ*/
                calvalue(cx/2-i,cy+i,cx/2-i-1,cy+i+1,cx/2-i-2,cy+i+2,cx/2-i-3,
                         cy+i+3,cx/2-i-4,cy+i+4);

            for(j=0;j<=9;j++)
                callfour(cx/2,j,cx/2,j+1,cx/2,j+2,cx/2,j+3,cx/2,j+4,cx/2,j+5);
            for(i=0;i<=9;i++)/*ËÄºáÏò*/
                callfour(i,cy,i+1,cy,i+2,cy,i+3,cy,i+4,cy,i+5,cy);
            for(i=-5;i<=0;i++)
            {/*ËÄÐ±ÓÒÏÂ*/
                callfour(cx/2+i,cy+i,cx/2+i+1,cy+i+1,cx/2+i+2,cy+i+2,
                         cx/2+i+3,cy+i+3,cx/2+i+4,cy+i+4,cx/2+i+5,cy+i+5);
                /*ËÄÐ±×óÏÂ*/
                callfour(cx/2-i,cy+i,cx/2-i-1,cy+i+1,cx/2-i-2,cy+i+2,cx/2-i-3,
                         cy+i+3,cx/2-i-4,cy+i+4,cx/2-i-5,cy+i+5);
            }

            py = cy;
            px = cx;
            if(tlivef>=2 && winner==0)
                bandhand(4);
            if(livethree>=2 && tlivef ==0) 
                bandhand(3);
            if(threefour==black)  
                haha(1);
            else if(threefour==white)
                haha(1);       
            if (chating) {
                sleep (1);
                move (PROMPT, ptr + 6);
            } else
                move (py, px);
            if (winner) {
                InitScreen ();
                goto begin;
            }
        } else {
            if (ch == Ctrl('X')) {
                quitf = 1;
            } else if (ch==Ctrl('C') || ((ch=='Q' || ch=='q') && !chating)) {
                RMSG = YEA;
                saveline (PROMPT, 0);
                getdata(PROMPT, 0, "ÄúÈ·¶¨ÒªÀë¿ªÂð? (Y/N)?[N] ", genbuf1, 2, LCECHO, YEA);
                if (genbuf1[0] == 'Y' || genbuf1[0] == 'y')
                    quitf = 1;
                else 
                    quitf = 0;
                saveline (PROMPT, 1);
                RMSG = NA;
            } else if (ch==Ctrl('N') || ((ch=='N' || ch=='n') && !chating)) {
                saveline (PROMPT, 0);
                RMSG = YEA;
                getdata(PROMPT,0,"ÄúÈ·¶¨ÒªÖØÐÂ¿ªÊ¼Âð? (Y/N)?[N] ",genbuf1,2,LCECHO, YEA);
                if (genbuf1[0] == 'Y' || genbuf1[0] == 'y') {
                    send (fd, "\1", 1, 0);
                    move (PROMPT, 0);
                    bell ();
                    clrtoeol ();
                    move (PROMPT, 0);
                    outs ("ÒÑ¾­ÒÑ¾­ÌæÄú·¢³öÇëÇóÁË");
                    refresh ();
                    sleep (1);
                }
                RMSG = NA;
                saveline (PROMPT, 1);
                if (chating)
                    move (PROMPT, ptr + 6);
                else
                    move (py, px);
                continue;
            } else if (ch == '\t') {
                if (chating) {
                    chating = 0;
                    move (py, px);
                } else {
                    chating = 1;
                    move (PROMPT, 6 + ptr);
                }
                continue;
            } else if (ch == '\0')
                continue;
            else if (chating) {
                if (ch == '\n' || ch == '\r') { 
                    if (!cbuf[0])
                        continue;
                    ptr = 0;
                    five_chat (chatbuf + 1, 0);
                    send (fd, chatbuf, strlen (chatbuf + 1) + 2, 0);
                    cbuf[0] = '\0';
                    move (PROMPT, 6);
                    clrtoeol ();
                } else if (ch == KEY_LEFT) {
                    if (ptr)
                        ptr --;
                } else if (ch == KEY_RIGHT) {
                    if (cbuf[ptr])
                        ptr ++;
                } else if (ch == Ctrl ('H') || ch == '\177') {
                    if (ptr) {
                        ptr --;
                        memcpy (&cbuf[ptr], &cbuf[ptr+1], BSIZE-ptr);
                        move (PROMPT, ptr+6);
                        clrtoeol ();
                        prints ("%s",&cbuf[ptr]);
                    }
                } else if (ch == KEY_DEL) {
                    if (cbuf[ptr]) {
                        memcpy (&cbuf[ptr], &cbuf[ptr+1], BSIZE-ptr);
                        clrtoeol ();
                        prints ("%s",&cbuf[ptr]);
                    }
                } else if (ch == Ctrl ('A')) {
                    ptr = 0;
                } else if (ch == Ctrl ('E')) {
                    while (cbuf[++ptr]);
                } else if (ch == Ctrl ('K')) {
                    ptr = 0;
                    cbuf[ptr] = '\0';
                    move (PROMPT, ptr+6);
                    clrtoeol ();
                } else if (ch == Ctrl ('U')) {
                    memmove (cbuf, &cbuf[ptr], BSIZE - ptr + 1);
                    ptr = 0;
                    move (PROMPT, ptr+6);
                    clrtoeol ();
                    prints ("%s",cbuf);
                } else if (ch == Ctrl ('W')) {
                    if (ptr) {
                        int optr;

                        optr = ptr;
                        ptr --;
                        do {
                            if (cbuf[ptr] != ' ')
                                break;
                        } while (-- ptr);
                        do {
                            if (cbuf[ptr] == ' ') {
                                if (cbuf[ptr+1] != ' ')
                                    ptr ++;
                                break;
                            }
                        } while (-- ptr);
                        memcpy (&cbuf[ptr], &cbuf[optr], BSIZE-optr+1);
                        move (PROMPT, ptr+6);
                        clrtoeol ();
                        prints ("%s",&cbuf[ptr]);
                    }
                } else if (isprint2 (ch)) {
                    if (ptr == BSIZE)
                        continue;
                    if (!cbuf[ptr]) {
                        cbuf[ptr] = ch;
                        move (PROMPT, 6 + ptr);
                        outc (ch);
                        cbuf[++ptr] = 0;
                    } else {
                        memmove (&cbuf[ptr+1], &cbuf[ptr], BSIZE-ptr+1);
                        cbuf[ptr] = ch;
                        move (PROMPT, 6 + ptr);
                        prints ("%s",&cbuf[ptr]);
                        ptr ++;
                    }
                }
                move (PROMPT, 6 + ptr);
                continue;
            }
        }
            
        if(fdone==1 && !chating && ch != I_OTHERDATA)/*»»ÎÒ*/
        {

            move(py,px);
            switch (ch)
            {

                case KEY_DOWN:
                case 'j':
                case 'J':
                    py=py+1;
                    if(py>14) py=0;
                    break;

                case KEY_UP:
                case 'k':
                case 'K':
                    py=py-1;
                    if(py<0) py=14;
                    break;

                case KEY_LEFT:
                case 'h':
                case 'H':
                    px=px-1;
                    if(px<0) px=28;
                    break;

                case KEY_RIGHT:
                case 'l':
                case 'L':
                    px=px+1;
                    if(px>28)
                    {
                        px=0;px=px-1;
                    }                      /*»áÌø¸ñßÖ*/
                    break;
                case ' ':
                    if(banf==1) break;

                    if((px%2)==1) px=px-1; /*½â¾önetterm²»ºÏÎÊÌâ*/
                    move(py,px);
                    hand+=1;
                    playboard[x][py]=player;
                    if(player==black)
                    {
                        outs("¡ñ");
                        player=white;
                    }
                    else
                    {
                        outs("¡ð");
                        player=black;
                    }
                    chess[hand][0]=px;
                    chess[hand][1]=py;
                    if(hand%2==0)
                        move(((hand-1)%20)/2+4,48);
                    else
                        move(((hand-1)%19)/2+4,35);

                    prints("µÚ%2dÊÖ %s%c%2d",hand,
                           (hand%2==1)?"¡ñ":"¡ð",abcd[px/2],15-py);
                    idone=1;
                    move (py, px);
                    refresh ();
                    break;
                default:
                    break;
            }
            move(py,px);
            x=px/2;
            if(playboard[x][py]!=0)
                banf=1;
            else
                banf=0;

            if(idone==1)
            {
                xy_po[0] = px/10 + '0';
                xy_po[1] = px%10 + '0';
                xy_po[2] = py/10 + '0';
                xy_po[3] = py%10 + '0';
                fdone=0;
                xy_po[4]='1';
                if(send(fd,xy_po,sizeof(xy_po),0)==-1)
                    break;

                move(15,35);
                outs("¡ïµÈ´ý¶Ô·½ÏÂ×Ó¡ï");
                haha (5);

                tdeadf=tlivef=livethree=threefour=0;
                for(j=0;j<=10;j++)
                    calvalue(px/2,j,px/2,j+1,px/2,j+2,px/2,j+3,px/2,j+4);
                for(i=0;i<=10;i++)/*ºáÏò*/
                    calvalue(i,py,i+1,py,i+2,py,i+3,py,i+4,py);
                for(i=-4;i<=0;i++)/*Ð±ÓÒÏÂ*/
                    calvalue(px/2+i,py+i,px/2+i+1,py+i+1,px/2+i+2,py+i+2,
                             px/2+i+3,py+i+3,px/2+i+4,py+i+4);
                for(i=-4;i<=0;i++)/*Ð±×óÏÂ*/
                    calvalue(px/2-i,py+i,px/2-i-1,py+i+1,px/2-i-2,py+i+2,px/2-i-3,
                             py+i+3,px/2-i-4,py+i+4);

                for(j=0;j<=9;j++)
                    callfour(px/2,j,px/2,j+1,px/2,j+2,px/2,j+3,px/2,j+4,px/2,j+5);
                for(i=0;i<=9;i++)/*ËÄºáÏò*/
                    callfour(i,py,i+1,py,i+2,py,i+3,py,i+4,py,i+5,py);
                for(i=-5;i<=0;i++)
                {    /*ËÄÐ±ÓÒÏÂ*/
                    callfour(px/2+i,py+i,px/2+i+1,py+i+1,px/2+i+2,py+i+2,
                             px/2+i+3,py+i+3,px/2+i+4,py+i+4,px/2+i+5,py+i+5);
                    /*ËÄÐ±×óÏÂ*/
                    callfour(px/2-i,py+i,px/2-i-1,py+i+1,px/2-i-2,py+i+2,px/2-i-3,
                             py+i+3,px/2-i-4,py+i+4,px/2-i-5,py+i+5);
                }

                if(tlivef>=2 && winner==0)
                    bandhand(4);
                if(livethree>=2 && tlivef ==0)
                    bandhand(3);
                if(threefour==black)          
                    haha(1);  
                else if(threefour==white)
                    haha(1);

            }
            idone=0;
        }
        if (quitf) {
            genbuf1[0] = '\xff';
            send (fd, genbuf1, 1, 0);
            press ();
            break;
        }
        if (winner) {
            InitScreen ();
            goto begin;
        }
    }

    add_io(0, 0);
    close(fd);
    return;
}

void
five_chat (char *msg, int init)
{
    char prompt[] = "===>";
    char chat[] = "ÁÄÌì: ";
    static char win[MAX][80];
    static int curr, p, i;

    if (init) {
        for (i = 0; i < MAX; i ++)
            win[i][0] = '\0';
        curr = START;
        p = 0;
        move (START - 1, 0);
        for (i = 0; i < 80; i ++)
            outc ('-');
        move (END + 1, 0);
        for (i = 0; i < 80; i ++)
            outc ('-');
        move (curr, 0);
        clrtoeol ();
        prints ("%s",prompt);
        move (PROMPT, 0);
        prints ("%s",chat);
        return;
    }

    if (msg) {
        strncpy (win[p], msg, 80);
        move (curr, 0);
        clrtoeol ();
        prints ("%s",win[p]);
        p ++;
        if (p == MAX)
            p = 0;
        curr ++;
        if (curr > END) {
            for (i = START; i < END; i ++) {
                move (i, 0);
                clrtoeol ();
                prints ("%s",win[(p+MAX+(i-START))%MAX]);
            }
            curr = END;
        }
        move (curr, 0);
        clrtoeol ();
        prints ("%s",prompt);
        refresh ();
    }
}

void
press (void)
{
    int c;
    extern int showansi;
    int tmpansi;

    tmpansi = showansi;
    showansi = 1;
    saveline (t_lines-1, 0);
    move (t_lines-1, 0);
    clrtoeol ();
    prints ("[37;40m[0m                               [33m°´ÈÎÒâ¼ü¼ÌÐø ...[37;40m[0m");
    refresh ();
    read (0, &c, sizeof (int));
    move (t_lines-1, 0);
    saveline (t_lines-1, 1);
    showansi = tmpansi;
}

#endif
