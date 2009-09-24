/*bbsnet.c*/
// NJU bbsnet, preview version, zhch@dii.nju.edu.cn, 2000.3.23 //
// HIT bbsnet, Changed by Sunner, sun@bbs.hit.edu.cn, 2000.6.11

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
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
#include <math.h>

#include "bbs.h"

char host1[100][40], host2[100][40], ip[100][40];
int port[100], counts= 0;

char datafile[80]= BBSHOME"/etc/bbsnet.ini";
char userid[80]= "unknown.";

/**
 * Get appropriate response command.
 * @param command received command
 * @param use use the option or not
 * @return the response command
 */
int telnet_response(int command, bool use)
{
	if (command == WILL || command == WONT) {
		if (use == true)
			return DO;
		else
			return DONT;
	} else if (command == DO || command == DONT){
		if (use == true)
			return WILL;
		else
			return WONT;
	} else {  // command == SB
		return SB;
	}
}

/**
 * Telnet option negotiation.
 * @param fd output file descriptor (to server)
 * @param command received command
 * @param option option to negotiate
 * @note only a small subset of telnet options is implemented
 */
void telnet_opt(int fd, int command, int option)
{
	unsigned char res[16] = {IAC};
	const unsigned char tty[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_IS, 'V', 
			'T', '1', '0', '0', IAC, SE};
	const unsigned char naws[] = { IAC, WILL, TELOPT_NAWS, IAC, SB,
			TELOPT_NAWS, 0, 80, 0, 24, IAC, SE};
	switch (option) {
		case TELOPT_BINARY:
		case TELOPT_SGA:
			res[1] = telnet_response(command, true);
			res[2] = option;
			write(fd, res, 3);
			break;
		case TELOPT_ECHO:
			res[1] = telnet_response(command, false);
			res[2] = option;
			write(fd, res, 3);
			break;
		case TELOPT_TTYPE:
			res[1] = telnet_response(command, true);
			if (res[1] == SB) {
				write(fd, tty, sizeof(tty));
			} else {
				res[2] = option;
				write(fd, res, 3);
			}
			break;
		case TELOPT_NAWS:
			write(fd, naws, sizeof(naws));
			break;
		default:
			break;
	}
}

/**
 * Telnet proxy which automatically responses to server's option negotiation.
 * @param fd output file descriptor (to server)
 * @param buf input buffer
 * @param size bytes in input buffer
 */
void telnet_proxy(int fd, const unsigned char *buf, int size)
{
	static int status, command;
	int option = 0;
	const unsigned char *end = buf + size, *last = buf;
	while (buf != end) {
		switch (status) {
			case TELST_END:
				last = buf;
				status = TELST_NOR;
				// No break here.
			case TELST_NOR:
				if (*buf == IAC) {
					status = TELST_IAC;
					write(STDOUT_FILENO, last, buf - last);
					last = buf;
				}
				break;
			case TELST_IAC:
				command = *buf;
				if (*buf == SB)
					status = TELST_SUB;
				else
					status = TELST_COM;
				break;
			case TELST_COM:
				status = TELST_END;
				telnet_opt(fd, command, *buf);
				break;
			case TELST_SUB:
				option = *buf;
				status = TELST_SBC;
				break;
			case TELST_SBC:
				if (*buf == SE) {
					telnet_opt(fd, command, option);
					status = TELST_END;
				}
				break;
			default:
				break;
		}
		++buf;
	}
	if (status == TELST_NOR)
		write(STDOUT_FILENO, last, buf - last);
}

void init_data(void)
{
	FILE *fp;
	char t[256], *t1, *t2, *t3, *t4;
	fp= fopen(datafile, "r");
	if (fp== NULL)
		return;
	while (fgets(t, 255, fp)&& counts <= 72) {
		t1= strtok(t, " \t");
		t2= strtok(NULL, " \t\n");
		t3= strtok(NULL, " \t\n");
		t4= strtok(NULL, " \t\n");
		if (t1[0]== '#'|| t1== NULL|| t2== NULL|| t3== NULL)
			continue;
		strncpy(host1[counts], t1, 16);
		strncpy(host2[counts], t2, 36);
		strncpy(ip[counts], t3, 36);
		port[counts]= t4 ? atoi(t4) : 23;
		counts++;
	}
	fclose(fp);
}

#ifdef FDQUAN	//¸´µ©ÈªµÄ´©Ëó³ÌÐò
char str[]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
void sh(int n)
{
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

void show_all(void)
{
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

void locate(int n)
{
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

int getch(void)
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

void main_loop(void)
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
			if (p < 3) {
				p += 3 * ( (counts -1)/3 - (p> ((counts -1)%3)));
			} else p -= 3;
		}
		if(c== 258) //ÏòÏÂ
		{
			if (p+3> counts - 1)
			p %= 3;
			else p += 3;
		}
		if(c== 259) //ÏòÓÒ
		{
			p ++;
			if(p> counts - 1) p = 0;
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
	if (n>= counts)
		return -1;
	printf("[H[2J[1;32mo Á¬Íù: %s (%s)\r\n", host2[n], ip[n]);
	printf("%s\r\n\r\n[m", "o Á¬²»ÉÏÊ±ÇëÉÔºò£¬30 Ãëºó½«×Ô¶¯ÍË³ö");
	fflush(stdout);
	proc(host2[n], ip[n], port[n]);
	return 0;
}

void QuitTime(int notused)
{
	reset_tty();
	exit(0);
}

int SetQuitTime(void)
{
	signal(SIGALRM, QuitTime);
	alarm(60);
	return 0;
}

int main(int n, char* cmd[])
{
	SetQuitTime();
	get_tty();
	init_tty();
	if (n>= 2)
		strcpy(datafile, cmd[1]);
	if (n>= 3)
		strcpy(userid, cmd[2]);
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

int bbs_syslog(char* s)
{ //rename by money for conflict with syslog() 2004.01.07
	char buf[512], timestr[16], *thetime;
	time_t dtime;
	FILE *fp;
	fp=fopen("reclog/bbsnet.log", "a");
	time(&dtime);
	thetime = (char*) ctime(&dtime);
	strncpy(timestr, &(thetime[4]), 15);
	timestr[15] = '\0';
	sprintf(buf, "%s %s %s\n", userid, timestr, s) ;
	fprintf(fp, buf);
	fclose(fp);
}

#define stty(fd, data) tcsetattr( fd, TCSANOW, data )
#define gtty(fd, data) tcgetattr( fd, data )
struct termios tty_state, tty_new;

int get_tty(void)
{
	if (gtty(1,&tty_state) < 0)
		return 0;
	return 1;
}

void init_tty(void)
{
	long vdisable;

	memcpy( &tty_new, &tty_state, sizeof(tty_new)) ;
	tty_new.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ISIG);
	tty_new.c_cflag &= ~CSIZE;
	tty_new.c_cflag |= CS8;
	tty_new.c_cc[ VMIN ] = 1;
	tty_new.c_cc[ VTIME ] = 0;
	if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0) {
		tty_new.c_cc[VSTART] = vdisable;
		tty_new.c_cc[VSTOP] = vdisable;
		tty_new.c_cc[VLNEXT] = vdisable;
	}
	tcsetattr(1, TCSANOW, &tty_new);
}

void reset_tty(void)
{
	stty(1,&tty_state);
}

void proc(char *hostname, char *server, int port)
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
	memset((char *)&blah, 0, sizeof(blah));
	blah.sin_family=AF_INET;
	blah.sin_addr.s_addr=inet_addr(server);
	blah.sin_port=htons(port);
	fflush(stdout);
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((he = gethostbyname(server)) != NULL)
		bcopy(he->h_addr, (char *)&blah.sin_addr, he->h_length);
	else if ((blah.sin_addr.s_addr = inet_addr(server)) < 0)
		return;
	if (connect(fd, (struct sockaddr *)&blah, 16)<0)
		return;

	signal(SIGALRM, SIG_IGN);
	printf("ÒÑ¾­Á¬½ÓÉÏÖ÷»ú£¬°´'ctrl+]'¿ìËÙÍË³ö¡£\n");
	sprintf(buf, "%s (%s)", hostname, server);
	bbs_syslog(buf);//rename by money for conflict with syslog() 2004.01.07

	while (1) {
		tv.tv_sec = 2400;
		tv.tv_usec = 0;
		FD_ZERO(&readfds) ;
		FD_SET(fd, &readfds);
		FD_SET(STDIN_FILENO, &readfds);

		result = select(fd + 1, &readfds, NULL, NULL, &tv);
		if (result <= 0)
			break;
		if (FD_ISSET(0, &readfds)) {
			result = read(0, buf, sizeof(buf));
			if (result<=0)
				break;
			if (result==1&&(buf[0]==10||buf[0]==13)) {
				buf[0]=13;
				buf[1]=10;
				result=2;
			}
			if (buf[0]==29) {
				close(fd);
				return;
			}
			write(fd, buf, result);
		} else {
			result=read(fd, buf, sizeof(buf));
			if (result <= 0)
				break;
			telnet_proxy(fd, buf, result);
		}
	}
}

