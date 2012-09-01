/*---------------------------------------------------------------------------*/
/* ½á¾Öº¯Ê½                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

/*--------------------------------------------------------------------------*/
/*  ½á¾Ö²ÎÊıÉè¶¨                                                            */
/*--------------------------------------------------------------------------*/
struct newendingset
{
  char *girl;		/*Å®Éú½á¾ÖµÄÖ°Òµ*/ 
  char *boy;		/*ÄĞÉú½á¾ÖµÄÖ°Òµ*/
  int grade;		/*ÆÀ·Ö*/
};
typedef struct newendingset newendingset;

/*ÍòÄÜ*/
struct newendingset endmodeall_purpose[] = {
"Å®ĞÔÖ°Òµ",		"ÄĞÉúÖ°Òµ",		0,
"³ÉÎªÕâ¸ö¹ú¼ÒĞÂÅ®Íõ",	"³ÉÎªÕâ¸ö¹ú¼ÒĞÂ¹úÍõ",	500,
"³ÉÎª¹ú¼ÒµÄÔ×Ïà",	"³ÉÎª¹ú¼ÒµÄÔ×Ïà",	400,
"³ÉÎª½Ì»áÖĞµÄ´óÖ÷½Ì",	"³ÉÎª½Ì»áÖĞµÄ´óÖ÷½Ì",	350,
"³ÉÎª¹ú¼ÒµÄ´ó³¼",	"³ÉÎª¹ú¼ÒµÄ´ó³¼",	320,
"³ÉÎªÒ»Î»²©Ê¿",		"³ÉÎªÒ»Î»²©Ê¿",		300,
"³ÉÎª½Ì»áÖĞµÄĞŞÅ®",	"³ÉÎª½Ì»áÖĞµÄÉñ¸¸",	150,
"³ÉÎª·¨Í¥ÉÏµÄ´ó·¨¹Ù",   "³ÉÎª·¨Í¥ÉÏµÄ·¨¹Ù",	200,
"³ÉÎªÖªÃûµÄÑ§Õß",	"³ÉÎªÖªÃûµÄÑ§Õß",	120,
"³ÉÎªÒ»ÃûÅ®¹Ù",		"³ÉÎªÒ»ÃûÄĞ¹Ù",		100,
"ÔÚÓıÓ×Ôº¹¤×÷",		"ÔÚÓıÓ×Ôº¹¤×÷",		100,
"ÔÚÂÃ¹İ¹¤×÷",		"ÔÚÂÃ¹İ¹¤×÷",		100,
"ÔÚÅ©³¡¹¤×÷",		"ÔÚÅ©³¡¹¤×÷",		100,
"ÔÚ²ÍÌü¹¤×÷",		"ÔÚ²ÍÌü¹¤×÷",		100,
"ÔÚ½ÌÌÃ¹¤×÷",		"ÔÚ½ÌÌÃ¹¤×÷",		100,
"ÔÚµØÌ¯¹¤×÷",		"ÔÚµØÌ¯¹¤×÷",		100,
"ÔÚ·¥Ä¾³¡¹¤×÷",		"ÔÚ·¥Ä¾³¡¹¤×÷",		100,
"ÔÚÃÀÈİÔº¹¤×÷",		"ÔÚÃÀÈİÔº¹¤×÷",		100,
"ÔÚá÷ÁÔÇø¹¤×÷",		"ÔÚá÷ÁÔÇø¹¤×÷",		100,
"ÔÚ¹¤µØ¹¤×÷",		"ÔÚ¹¤µØ¹¤×÷",		100,
"ÔÚÄ¹Ô°¹¤×÷",		"ÔÚÄ¹Ô°¹¤×÷",		100,
"µ£ÈÎ¼ÒÍ¥½ÌÊ¦¹¤×÷",	"µ£ÈÎ¼ÒÍ¥½ÌÊ¦¹¤×÷",	100,
"ÔÚ¾Æ¼Ò¹¤×÷",		"ÔÚ¾Æ¼Ò¹¤×÷",		100,
"ÔÚ¾Æµê¹¤×÷",		"ÔÚ¾Æµê¹¤×÷",		100,
"ÔÚ´óÒ¹×Ü»á¹¤×÷",	"ÔÚ´óÒ¹×Ü»á¹¤×÷",	100,
"ÔÚ¼ÒÖĞ°ïÃ¦",		"ÔÚ¼ÒÖĞ°ïÃ¦",		50,
"ÔÚÓıÓ×Ôº¼æ²î",		"ÔÚÓıÓ×Ôº¼æ²î",		50,
"ÔÚÂÃ¹İ¼æ²î",		"ÔÚÂÃ¹İ¼æ²î",		50,
"ÔÚÅ©³¡¼æ²î",		"ÔÚÅ©³¡¼æ²î",		50,
"ÔÚ²ÍÌü¼æ²î",		"ÔÚ²ÍÌü¼æ²î",		50,
"ÔÚ½ÌÌÃ¼æ²î",		"ÔÚ½ÌÌÃ¼æ²î",		50,
"ÔÚµØÌ¯¼æ²î",		"ÔÚµØÌ¯¼æ²î",		50,
"ÔÚ·¥Ä¾³¡¼æ²î",		"ÔÚ·¥Ä¾³¡¼æ²î",		50,
"ÔÚÃÀÈİÔº¼æ²î",		"ÔÚÃÀÈİÔº¼æ²î",		50,
"ÔÚá÷ÁÔÇø¼æ²î",		"ÔÚá÷ÁÔÇø¼æ²î",		50,
"ÔÚ¹¤µØ¼æ²î",		"ÔÚ¹¤µØ¼æ²î",		50,
"ÔÚÄ¹Ô°¼æ²î",		"ÔÚÄ¹Ô°¼æ²î",		50,
"µ£ÈÎ¼ÒÍ¥½ÌÊ¦¼æ²î",	"µ£ÈÎ¼ÒÍ¥½ÌÊ¦¼æ²î",	50,
"ÔÚ¾Æ¼Ò¼æ²î",		"ÔÚ¾Æ¼Ò¼æ²î",		50,
"ÔÚ¾Æµê¼æ²î",		"ÔÚ¾Æµê¼æ²î",		50,
"ÔÚ´óÒ¹×Ü»á¼æ²î",	"ÔÚ´óÒ¹×Ü»á¼æ²î",	50,
NULL,		NULL,	0
};

/*Õ½¶·*/
struct newendingset endmodecombat[] = {
"Å®ĞÔÖ°Òµ",		"ÄĞÉúÖ°Òµ",			0,
"±»·âÎªÓÂÕß Õ½Ê¿ĞÍ",  	"±»·âÎªÓÂÕß Õ½Ê¿ĞÍ",		420,
"±»°Îßª³ÉÎªÒ»¹úµÄ½«¾ü",	"±»°Îßª³ÉÎªÒ»¹úµÄ½«¾ü",		300,
"µ±ÉÏÁË¹ú¼Ò½üÎÀ¶Ó¶Ó³¤",	"µ±ÉÏÁË¹ú¼Ò½üÎÀ¶Ó¶Ó³¤",		200,
"µ±ÁËÎäÊõÀÏÊ¦",		"µ±ÁËÎäÊõÀÏÊ¦",			150,
"±ä³ÉÆïÊ¿±¨Ğ§¹ú¼Ò",	"±ä³ÉÆïÊ¿±¨Ğ§¹ú¼Ò",		160,
"Í¶Éí¾üÂÃÉú»î£¬³ÉÎªÊ¿±ø","Í¶Éí¾üÂÃÉú»î£¬³ÉÎªÊ¿±ø",	80,
"±ä³É½±½ğÁÔÈË",		"±ä³É½±½ğÁÔÈË",			0,
"ÒÔ  ±ø¹¤×÷Î¬Éú",	"ÒÔ  ±ø¹¤×÷Î¬Éú",		0,
NULL,           NULL,   0
};

/*Ä§·¨*/
struct newendingset endmodemagic[] = {
"Å®ĞÔÖ°Òµ",	     	"ÄĞÉúÖ°Òµ",		0,
"±»·âÎªÓÂÕß Ä§·¨ĞÍ",	"±»·âÎªÓÂÕß Ä§·¨ĞÍ",	420,
"±»Æ¸ÎªÍõ¹¬Ä§·¨Ê¦",	"±»Æ¸ÎªÍõ¹ÙÄ§·¨Ê¦",	280,
"µ±ÁËÄ§·¨ÀÏÊ¦",		"µ±ÁËÄ§·¨ÀÏÊ¦",		160,
"±ä³ÉÒ»Î»Ä§µ¼Ê¿",	"±ä³ÉÒ»Î»Ä§µ¼Ê¿",	180,
"µ±ÁËÄ§·¨Ê¦",		"µ±ÁËÄ§·¨Ê¦",		120,
"ÒÔÕ¼²·Ê¦°ïÈËËãÃüÎªÉú",	"ÒÔÕ¼²·Ê¦°ïÈËËãÃüÎªÉú",	40,
"³ÉÎªÒ»¸öÄ§ÊõÊ¦",	"³ÉÎªÒ»¸öÄ§ÊõÊ¦",	20,
"³ÉÎª½ÖÍ·ÒÕÈË",		"³ÉÎª½ÖÍ·ÒÕÈË",		10,
NULL,           NULL	,0
};

/*Éç½»*/
struct newendingset endmodesocial[] = {
"Å®ĞÔÖ°Òµ",     	"ÄĞÉúÖ°Òµ",		0,
"³ÉÎª¹úÍõµÄ³èåú",	"³ÉÎªÅ®ÍõµÄæâÂíÒ¯",	170,
"±»ÌôÑ¡³ÉÎªÍõåú",	"±»Ñ¡ÖĞµ±Å®ÍõµÄ·òĞö",	260,
"±»²®¾ô¿´ÖĞ£¬³ÉÎª·òÈË",	"³ÉÎªÁËÅ®²®¾ôµÄ·òĞö",	130,
"³ÉÎª¸»ºÀµÄÆŞ×Ó",	"³ÉÎªÅ®¸»ºÀµÄ·òĞö",	100,
"³ÉÎªÉÌÈËµÄÆŞ×Ó",	"³ÉÎªÅ®ÉÌÈËµÄ·òĞö",	80,
"³ÉÎªÅ©ÈËµÄÆŞ×Ó",	"³ÉÎªÅ®Å©ÈËµÄ·òĞö",	80,
"³ÉÎªµØÖ÷µÄÇé¸¾",	"³ÉÎªÅ®µØÖ÷µÄÇé·ò",	-40,
NULL,           NULL,	0
};
/*ÒÕÊõ*/
struct newendingset endmodeart[] = {
"Å®ĞÔÖ°Òµ",		"ÄĞÉúÖ°Òµ",	0,
"³ÉÎªÁËĞ¡³ó",		"³ÉÎªÁËĞ¡³ó",	100,
"³ÉÎªÁË×÷¼Ò",		"³ÉÎªÁË×÷¼Ò",	100,
"³ÉÎªÁË»­¼Ò",		"³ÉÎªÁË»­¼Ò",	100,
"³ÉÎªÁËÎèµ¸¼Ò",		"³ÉÎªÁËÎèµ¸¼Ò",	100,
NULL,           NULL,	0
};

/*°µºÚ*/
struct newendingset endmodeblack[] = {
"Å®ĞÔÖ°Òµ",     	"ÄĞÉúÖ°Òµ",		0,
"±ä³ÉÁËÄ§Íõ",		"±ä³ÉÁËÄ§Íõ",		-1000,
"»ì³ÉÁËÌ«ÃÃ",		"»ì³ÉÁËÁ÷Ã¥",		-350,
"×öÁË[£Ó£ÍÅ®Íõ]µÄ¹¤×÷",	"×öÁË[£Ó£Í¹úÍõ]µÄ¹¤×÷",	-150,
"µ±ÁËºÚ½ÖµÄ´ó½ã",	"µ±ÁËºÚ½ÖµÄÀÏ´ó",	-500,
"±ä³É¸ß¼¶æ½¸¾",		"±ä³É¸ß¼¶Çé·ò",		-350,
"±ä³ÉÕ©ÆÛÊ¦Õ©ÆÛ±ğÈË",	"±ä³É½ğ¹âµ³Æ­±ğÈËÇ®",	-350,
"ÒÔÁ÷İºµÄ¹¤×÷Éú»î",	"ÒÔÅ£ÀÉµÄ¹¤×÷Éú»î",	-350,
NULL,		NULL,	0
};

/*¼ÒÊÂ*/
struct newendingset endmodefamily[] = {
"Å®ĞÔÖ°Òµ",     	"ÄĞÉúÖ°Òµ",		0,
"ÕıÔÚĞÂÄïĞŞĞĞ",		"ÕıÔÚĞÂÀÉĞŞĞĞ",		50,
NULL,		NULL,	0
};


int /*½á¾Ö»­Ãæ*/
pip_ending_screen()
{
  time_t now;
  char buf[256];
  char endbuf1[50];
  char endbuf2[50];
  char endbuf3[50];
  int endgrade=0;
  int endmode=0;
  clear();
  pip_ending_decide(endbuf1,endbuf2,endbuf3,&endmode,&endgrade);
  move(1,9); 
  prints("[1;33m©³©¥©¥©¥©·©³©¥©¥  ©·©³©¥©¥©¥  ©³©¥©¥©¥©·©³©¥©¥  ©·  ©¥©¥©¥  [0m");
  move(2,9);
  prints("[1;37m©§      ©§©§    ©§©§©§      ©§©§      ©§©§    ©§©§©§      ©§[0m");
  move(3,9);
  prints("[0;37m©§    ©¥  ©§    ©§©§©§      ©§©»©¥©·©³©¿©§    ©§©§©§  ©³©¥©·[0m");
  move(4,9);
  prints("[0;37m©§    ©¥  ©§  ©§  ©§©§      ©§©³©¥©¿©»©·©§  ©§  ©§©§      ©§[0m");
  move(5,9);
  prints("[1;37m©§      ©§©§  ©§  ©§©§      ©§©§      ©§©§  ©§  ©§©§      ©§[0m");
  move(6,9);
  prints("[1;35m©»©¥©¥©¥©¿©»©¥  ©¥©¿©»©¥©¥©¥  ©»©¥©¥©¥©¿©»©¥  ©¥©¿  ©¥©¥©¥  [0m");
  move(7,8);
  prints("[1;31m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[41;37m ĞÇ¿ÕÕ½¶·¼¦½á¾Ö±¨¸æ [0;1;31m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[0m");
  move(9,10);
  prints("[1;36mÕâ¸öÊ±¼ä²»Öª²»¾õµØ»¹ÊÇµ½ÁÙÁË...[0m");
  move(11,10);
  prints("[1;37m[33m%s[37m µÃÀë¿ªÄãµÄÎÂÅ¯»³±§£¬×Ô¼ºÒ»Ö»¼¦ÔÚÍâÃæÇóÉú´æÁË.....[0m",d.name);
  move(13,10);
  prints("[1;36mÔÚÄãÕÕ¹Ë½Ìµ¼ËûµÄÕâ¶ÎÊ±¹â£¬ÈÃËû½Ó´¥ÁËºÜ¶àÁìÓò£¬ÅàÑøÁËºÜ¶àµÄÄÜÁ¦....[0m");
  move(15,10);
  prints("[1;37mÒòÎªÕâĞ©£¬ÈÃĞ¡¼¦ [33m%s[37m Ö®ááµÄÉú»î£¬±äµÃ¸ü¶à²É¶à×ËÁË........[0m",d.name);
  move(17,10);
  prints("[1;36m¶Ôì¶ÄãµÄ¹ØĞÄ£¬ÄãµÄ¸¶³ö£¬ÄãËùÓĞµÄ°®......[0m");
  move(19,10);
  prints("[1;37m[33m%s[37m »áÓÀÔ¶¶¼Ãú¼ÇÔÚĞÄµÄ....[0m",d.name);
  pressanykey("½ÓÏÂÀ´¿´Î´À´·¢Õ¹");
  clrchyiuan(7,19);
  move(7,8);
  prints("[1;34m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[44;37m ĞÇ¿ÕÕ½¶·¼¦Î´À´·¢Õ¹ [0;1;34m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[0m");
  move(9,10);
  prints("[1;36mÍ¸¹ıË®¾§Çò£¬ÈÃÎÒÃÇÒ»ÆğÀ´¿´ [33m%s[36m µÄÎ´À´·¢Õ¹°É.....[0m",d.name);
  move(11,10);
  prints("[1;37mĞ¡¼¦ [33m%s[37m ááÀ´%s....[0m",d.name,endbuf1);
  move(13,10);
  prints("[1;36mÒòÎªËûµÄÖ®Ç°µÄÅ¬Á¦£¬Ê¹µÃËûÔÚÕâÒ»·½Ãæ%s....[0m",endbuf2);
  move(15,10);
  prints("[1;37mÖÁì¶Ğ¡¼¦µÄ»éÒö×´¿ö£¬ËûááÀ´%s£¬»éÒöËãÊÇºÜÃÀÂú.....[0m",endbuf3);
  move(17,10);
  prints("[1;36màÅ..ÕâÊÇÒ»¸ö²»´íµÄ½á¾Öà¡..........[0m");
  pressanykey("ÎÒÏë  ÄãÒ»¶¨ºÜ¸Ğ¶¯°É.....");
  show_ending_pic(0);
  pressanykey("¿´Ò»¿´·ÖÊıÂŞ");
  endgrade=pip_game_over(endgrade);
  pressanykey("ÏÂÒ»Ò³ÊÇĞ¡¼¦×ÊÁÏ  ¸Ï¿ìcopyÏÂÀ´×ö¼ÍÄî");
  pip_data_list();
  pressanykey("»¶Ó­ÔÙÀ´ÌôÕ½....");
  /*¼ÇÂ¼¿ªÊ¼*/
  now=time(0);
  sprintf(buf, "[1;35m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[0m\n");
  pip_log_record(buf);
  sprintf(buf, "[1;37mÔÚ [33m%s [37mµÄÊ±ºò£¬[36m%s [37mµÄĞ¡¼¦ [32m%s[37m ³öÏÖÁË½á¾Ö[0m\n", Cdate(&now), cuser.userid,d.name);
  pip_log_record(buf);
  sprintf(buf, "[1;37mĞ¡¼¦ [32m%s [37mÅ¬Á¦¼ÓÇ¿×Ô¼º£¬ááÀ´%s[0m\n[1;37mÒòÎªÖ®Ç°µÄÅ¬Á¦£¬Ê¹µÃÔÚÕâÒ»·½Ãæ%s[0m\n",d.name,endbuf1,endbuf2);
  pip_log_record(buf);
  sprintf(buf, "[1;37mÖÁì¶»éÒö×´¿ö£¬ËûááÀ´%s£¬»éÒöËãÊÇºÜÃÀÂú.....[0m\n\n[1;37mĞ¡¼¦ [32n%s[37m µÄ×Ü»ı·Ö£½ [33m%d[0m\n",endbuf3,d.name,endgrade);
  pip_log_record(buf);
  sprintf(buf, "[1;35m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[0m\n");
  pip_log_record(buf);  
  /*¼ÇÂ¼ÖÕÖ¹*/
  d.death=3;
  pipdie("[1;31mÓÎÏ·½áÊøÂŞ...[m  ",3);
  return 0;
}

int 
pip_ending_decide(endbuf1,endbuf2,endbuf3,endmode,endgrade)
char *endbuf1,*endbuf2,*endbuf3;
int *endmode,*endgrade;
{
  char *name[8][2]={{"ÄĞµÄ","Å®µÄ"},
  	           {"¼Ş¸øÍõ×Ó","È¢ÁË¹«Ö÷"},
  	           {"¼Ş¸øÄã","È¢ÁËÄã"},
                   {"¼Ş¸øÉÌÈË£Á","È¢ÁËÅ®ÉÌÈË£Á"},
                   {"¼Ş¸øÉÌÈË£Â","È¢ÁËÅ®ÉÌÈË£Â"},
                   {"¼Ş¸øÉÌÈË£Ã","È¢ÁËÅ®ÉÌÈË£Ã"},
                   {"¼Ş¸øÉÌÈË£Ä","È¢ÁËÅ®ÉÌÈË£Ä"},
                   {"¼Ş¸øÉÌÈË£Å","È¢ÁËÅ®ÉÌÈË£Å"}}; 
  int m=0,n=0,grade=0;
  int modeall_purpose=0;
  char buf1[256];
  char buf2[256];
  
  *endmode=pip_future_decide(&modeall_purpose);
  switch(*endmode)
  {
  /*1:°µºÚ 2:ÒÕÊõ 3:ÍòÄÜ 4:Õ½Ê¿ 5:Ä§·¨ 6:Éç½» 7:¼ÒÊÂ*/
    case 1:
      pip_endingblack(buf1,&m,&n,&grade);    
      break;
    case 2:
      pip_endingart(buf1,&m,&n,&grade);
      break;
    case 3:
      pip_endingall_purpose(buf1,&m,&n,&grade,modeall_purpose);
      break;
    case 4:
      pip_endingcombat(buf1,&m,&n,&grade);
      break;
    case 5:
      pip_endingmagic(buf1,&m,&n,&grade);
      break;
    case 6:
      pip_endingsocial(buf1,&m,&n,&grade);
      break;
    case 7:
      pip_endingfamily(buf1,&m,&n,&grade);
      break;
  }
  
  grade+=pip_marry_decide();
  strcpy(endbuf1, buf1);  
  if(n==1)
  {
    *endgrade=grade+300;
    sprintf(buf2,"·Ç³£µÄË³Àû..");
  }
  else if(n==2)
  {
    *endgrade=grade+100;
    sprintf(buf2,"±íÏÖ»¹²»´í..");
  }
  else if(n==3)
  {
    *endgrade=grade-10;
    sprintf(buf2,"³£Óöµ½ºÜ¶àÎÊÌâ....");
  }  
  strcpy(endbuf2, buf2);  
  if(d.lover>=1 && d.lover <=7)
  {
    if(d.sex==1)
      sprintf(buf2,"%s",name[d.lover][1]);
    else
      sprintf(buf2,"%s",name[d.lover][0]);
  }
  else if(d.lover==10)
    sprintf(buf2,"%s",buf1);
  else if(d.lover==0)
  {
    if(d.sex==1)
      sprintf(buf2,"È¢ÁËÍ¬ĞĞµÄÅ®º¢");
    else
      sprintf(buf2,"¼Ş¸øÁËÍ¬ĞĞµÄÄĞÉú");  
  } 
  strcpy(endbuf3, buf2);  
  return 0;
}
/*½á¾ÖÅĞ¶Ï*/
/*1:°µºÚ 2:ÒÕÊõ 3:ÍòÄÜ 4:Õ½Ê¿ 5:Ä§·¨ 6:Éç½» 7:¼ÒÊÂ*/
int
pip_future_decide(modeall_purpose)
int *modeall_purpose;
{
  int endmode;
  /*°µºÚ*/
  if((d.etchics==0 && d.offense >=100) || (d.etchics>0 && d.etchics<50 && d.offense >=250))
     endmode=1;
  /*ÒÕÊõ*/
  else if(d.art>d.hexp && d.art>d.mexp && d.art>d.hskill && d.art>d.mskill &&
          d.art>d.social && d.art>d.family && d.art>d.homework && d.art>d.wisdom &&
          d.art>d.charm && d.art>d.belief && d.art>d.manners && d.art>d.speech &&
          d.art>d.cookskill && d.art>d.love)
     endmode=2;
  /*Õ½¶·*/
  else if(d.hexp>=d.social && d.hexp>=d.mexp && d.hexp>=d.family)
  {
     *modeall_purpose=1;
     if(d.hexp>d.social+50 || d.hexp>d.mexp+50 || d.hexp>d.family+50)
        endmode=4;
     else
        endmode=3;     
  }
  /*Ä§·¨*/
  else if(d.mexp>=d.hexp && d.mexp>=d.social && d.mexp>=d.family)
  {  
     *modeall_purpose=2;
     if(d.mexp>d.hexp || d.mexp>d.social || d.mexp>d.family)
        endmode=5;
     else
        endmode=3;
  }   
  else if(d.social>=d.hexp && d.social>=d.mexp && d.social>=d.family)
  {
     *modeall_purpose=3;
     if(d.social>d.hexp+50 || d.social>d.mexp+50 || d.social>d.family+50)
        endmode=6;
     else
        endmode=3;
  }

  else 
  {  
     *modeall_purpose=4;
     if(d.family>d.hexp+50 || d.family>d.mexp+50 || d.family>d.social+50)
        endmode=7;
     else
        endmode=3;     
  }     
  return endmode;
}
/*½á»éµÄÅĞ¶Ï*/
int
pip_marry_decide()
{
  int grade;
  if(d.lover!=0)
  {  
     /* 3 4 5 6 7:ÉÌÈË */
     d.lover=d.lover;
     grade=80;
  }
  else
  {
     if(d.royalJ>=d.relation && d.royalJ>=100)
     {
        d.lover=1;  /*Íõ×Ó*/
        grade=200;
     }
     else if(d.relation>d.royalJ && d.relation>=100)
     {
        d.lover=2;  /*¸¸Ç×»òÄ¸Ç×*/
        grade=0;
     }
     else
     {
        d.lover=0;
        grade=40;
     }
  }
  return grade;
}


int
pip_endingblack(buf,m,n,grade) /*°µºÚ*/
char *buf;
int *m,*n,*grade;
{
 if(d.offense>=500 && d.mexp>=500) /*Ä§Íõ*/
 {
   *m=1;
   if(d.mexp>=1000)
     *n=1;
   else if(d.mexp<1000 && d.mexp >=800)
     *n=2;
   else
     *n=3;
 }

else if(d.hexp>=600)  /*Á÷Ã¥*/
 {
   *m=2;
   if(d.wisdom>=350)
     *n=1;
   else if(d.wisdom<350 && d.wisdom>=300)
     *n=2;
   else 
     *n=3;  
 } 
 else if(d.speech>=100 && d.art>=80) /*SM*/
 {
   *m=3;
   if(d.speech>150 && d.art>=120)
     *n=1;
   else if(d.speech>120 && d.art>=100)
     *n=2;
   else   
     *n=3;
 }
 else if(d.hexp>=320 && d.character>200 && d.charm< 200)	/*ºÚ½ÖÀÏ´ó*/
 {
   *m=4;
   if(d.hexp>=400)
     *n=1;
   else if(d.hexp<400 && d.hexp>=360)
     *n=2;
   else 
     *n=3;  
 }
 else if(d.character>=200 && d.charm >=200 && d.speech>70 && d.toman >70)  /*¸ß¼¶æ½¸¾*/
 {
   *m=5;
   if(d.charm>=300)
     *n=1;
   else if(d.charm<300 && d.charm>=250)
     *n=2;
   else 
     *n=3;  
 }
 
 else if(d.wisdom>=450)  /*Õ©Æ­Ê¦*/
 {
   *m=6;
   if(d.wisdom>=550)
     *n=1;
   else if(d.wisdom<550 && d.wisdom>=500)
     *n=2;
   else 
     *n=3;  
 }
 
 else /*Á÷İº*/
 {
   *m=7;
   if(d.charm>=350)
     *n=1;
   else if(d.charm<350 && d.charm>=300)
     *n=2;
   else 
     *n=3;  
 }
 if(d.sex==1)
   strcpy(buf, endmodeblack[*m].boy);
 else
   strcpy(buf, endmodeblack[*m].girl);
 *grade=endmodeblack[*m].grade;
 return 0; 
}


int
pip_endingsocial(buf,m,n,grade) /*Éç½»*/
char *buf;
int *m,*n,*grade;
{
 int class;
 if(d.social>600) class=1;
 else if(d.social>450) class=2;
 else if(d.social>380) class=3;
 else if(d.social>250) class=4;
 else class=5;

 switch(class)
 {
   case 1:
     if(d.charm>500)
     {
       *m=1;
       d.lover=10;
       if(d.character>=700)
        *n=1;
       else if(d.character<700 && d.character>=500)
        *n=2;
       else   
        *n=3;
     }
     else
     {
       *m=2;
       d.lover=10;
       if(d.character>=700)
        *n=1;
       else if(d.character<700 && d.character>=500)
        *n=2;
       else   
        *n=3;
     }
     break;
     
   case 2:
     *m=1;
     d.lover=10;
     if(d.character>=700)
        *n=1;
     else if(d.character<700 && d.character>=500)
        *n=2;
     else   
        *n=3;   
     break;
     
   case 3:
     if(d.character>=d.charm)
     {
       *m=3;
       d.lover=10;
       if(d.toman>=250)
         *n=1;
       else if(d.toman<250 && d.toman>=200)
         *n=2;
       else   
         *n=3;     
     }
     else
     {
       *m=4;
       d.lover=10;
       if(d.character>=400)
         *n=1;
       else if(d.character<400 && d.character>=300)
         *n=2;
       else   
         *n=3;     
     }
     break;
     
   case 4:
     if(d.wisdom>=d.affect)	
     {
	   *m=5;
	   d.lover=10;
	   if(d.toman>120 && d.cookskill>300 && d.homework>300)
	     *n=1;
	   else if(d.toman<120 && d.cookskill<300 && d.homework<300 &&d.toman>100 && d.cookskill>250 && d.homework>250)
	     *n=2;
	   else   
	     *n=3;     	
     }
     else
     {
	   *m=6;
	   d.lover=10;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=300)
	     *n=2;
	   else   
	     *n=3;     
     }
     break;
   
   case 5:
     *m=7;
     d.lover=10;
     if(d.charm>=200)
       *n=1;
     else if(d.charm<200 && d.charm>=100)
       *n=2;
     else   
       *n=3;
     break;
 }
 if(d.sex==1)
   strcpy(buf, endmodesocial[*m].boy);
 else
   strcpy(buf, endmodesocial[*m].girl);
 *grade=endmodesocial[*m].grade;
 return 0; 
}

int
pip_endingmagic(buf,m,n,grade) /*Ä§·¨*/
char *buf;
int *m,*n,*grade;
{
 int class;
 if(d.mexp>800) class=1;
 else if(d.mexp>600) class=2;
 else if(d.mexp>500) class=3;
 else if(d.mexp>300) class=4;
 else class=5;

 switch(class)
 {
    case 1:
      if(d.affect>d.wisdom && d.affect>d.belief && d.etchics>100)
      {
	   *m=1;
	   if(d.etchics>=800)
	     *n=1;
	   else if(d.etchics<800 && d.etchics>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.etchics<50)
      {
	   *m=4;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;    
      }
      else
      {
	   *m=2;
	   if(d.wisdom>=800)
	     *n=1;
	   else if(d.wisdom<800 && d.wisdom>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      break;
      
    case 2:
      if(d.etchics>=50)
      {
	   *m=3;
	   if(d.wisdom>=500)
	     *n=1;
	   else if(d.wisdom<500 && d.wisdom>=200)
	     *n=2;
	   else   
	     *n=3;     
      }
      else
      {
	   *m=4;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;          
      }
      break;
    
    case 3:
      *m=5;
      if(d.mskill>=300)
	*n=1;
      else if(d.mskill<300 && d.mskill>=150)
	*n=2;
      else   
	*n=3;
      break;

    case 4:
      *m=6;
      if(d.speech>=150)
	*n=1;
      else if(d.speech<150 && d.speech>=60)
	*n=2;
      else   
	*n=3;
      break; 
     
    case 5:
      if(d.character>=200)
      {
	*m=7;
        if(d.speech>=150)
 	  *n=1;
        else if(d.speech<150 && d.speech>=60)
      	  *n=2;
        else   
	  *n=3;        
      }
      else
      {
        *m=8;
        if(d.speech>=150)
 	  *n=1;
        else if(d.speech<150 && d.speech>=60)
      	  *n=2;
        else   
	  *n=3;      
      }
      break;
    
 }

 if(d.sex==1)
   strcpy(buf, endmodemagic[*m].boy);
 else
   strcpy(buf, endmodemagic[*m].girl); 
 *grade=endmodemagic[*m].grade;
 return 0; 
}

int
pip_endingcombat(buf,m,n,grade) /*Õ½¶·*/
char *buf;
int *m,*n,*grade;
{
 int class;
 if(d.hexp>1500) class=1;
 else if(d.hexp>1000) class=2;
 else if(d.hexp>800) class=3;
 else class=4;

 switch(class)
 {
    case 1:
      if(d.affect>d.wisdom && d.affect>d.belief && d.etchics>100)
      {
	   *m=1;
	   if(d.etchics>=800)
	     *n=1;
	   else if(d.etchics<800 && d.etchics>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.etchics<50)
      {

      }
      else
      {
	   *m=2;
	   if(d.wisdom>=800)
	     *n=1;
	   else if(d.wisdom<800 && d.wisdom>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      break;	
    
    case 2:
      if(d.character>=300 && d.etchics>50)
      {
	   *m=3;
	   if(d.etchics>=300 && d.charm >=300)
	     *n=1;
	   else if(d.etchics<300 && d.charm<300 && d.etchics>=250 && d.charm >=250)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.character<300 && d.etchics>50)
      {
	   *m=4;
	   if(d.speech>=200)
	     *n=1;
	   else if(d.speech<150 && d.speech>=80)
	     *n=2;
	   else   
	     *n=3;      
      }
      else
      {
	   *m=7;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;          
      }
      break;
    
    case 3:
      if(d.character>=400 && d.etchics>50)
      {
	   *m=5;
	   if(d.etchics>=300)
	     *n=1;
	   else if(d.etchics<300 && d.etchics>=150)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.character<400 && d.etchics>50)
      {
	   *m=4;
	   if(d.speech>=200)
	     *n=1;
	   else if(d.speech<150 && d.speech>=80)
	     *n=2;
	   else   
	     *n=3;      
      }
      else
      {
	   *m=7;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;          
      }
      break;    
    
    case 4:
      if(d.etchics>=50)
      {
	   *m=6;
      }
      else
      {
	   *m=8;
      }
      if(d.hskill>=100)
        *n=1;
      else if(d.hskill<100 && d.hskill>=80)
        *n=2;
      else   
        *n=3;       
      break;
 }

 if(d.sex==1)
   strcpy(buf, endmodecombat[*m].boy);
 else
   strcpy(buf, endmodecombat[*m].girl);
 *grade=endmodecombat[*m].grade;
 return 0;
}


int
pip_endingfamily(buf,m,n,grade) /*¼ÒÊÂ*/
char *buf;
int *m,*n,*grade;
{
 *m=1;
 if(d.charm>=200)
   *n=1;
 else if(d.charm<200 && d.charm>100)
   *n=2;
 else 
   *n=3;
   
 if(d.sex==1)
   strcpy(buf, endmodefamily[*m].boy);
 else
   strcpy(buf, endmodefamily[*m].girl);
 *grade=endmodefamily[*m].grade;
 return 0;
}


int
pip_endingall_purpose(buf,m,n,grade,mode) /*ÍòÄÜ*/
char *buf;
int *m,*n,*grade;
int mode;
{
 int data;
 int class;
 int num=0;
 
 if(mode==1)
    data=d.hexp;
 else if(mode==2)
    data=d.mexp;
 else if(mode==3)
    data=d.social;
 else if(mode==4)
    data=d.family;
 if(class>1000) class=1;
 else if(class>800) class=2;
 else if(class>500) class=3;
 else if(class>300) class=4;
 else class=5;

 data=pip_max_worktime(&num);
 switch(class)
 {
   case 1:
	if(d.character>=1000)
	{
	   *m=1;
	   if(d.etchics>=900)
	     *n=1;
	   else if(d.etchics<900 && d.etchics>=600)
	     *n=2;
	   else   
	     *n=3;	
	}
	else
	{
	   *m=2;
	   if(d.etchics>=650)
	     *n=1;
	   else if(d.etchics<650 && d.etchics>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	break;
   
   case 2:
	if(d.belief > d.etchics && d.belief > d.wisdom)
	{
	   *m=3;
	   if(d.etchics>=500)
	     *n=1;
	   else if(d.etchics<500 && d.etchics>=250)
	     *n=2;
	   else   
	     *n=3;	
	}
	else if(d.etchics > d.belief && d.etchics > d.wisdom)
	{
	   *m=4;
	   if(d.wisdom>=800)
	     *n=1;
	   else if(d.wisdom<800 && d.wisdom>=600)
	     *n=2;
	   else   
	     *n=3;	
	}
	else
	{
	   *m=5;
	   if(d.affect>=800)
	     *n=1;
	   else if(d.affect<800 && d.affect>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	break;

   case 3:
	if(d.belief > d.etchics && d.belief > d.wisdom)
	{
	   *m=6;
	   if(d.belief>=400)
	     *n=1;
	   else if(d.belief<400 && d.belief>=150)
	     *n=2;
	   else   
	     *n=3;	
	}
	else if(d.etchics > d.belief && d.etchics > d.wisdom)
	{
	   *m=7;
	   if(d.wisdom>=700)
	     *n=1;
	   else if(d.wisdom<700 && d.wisdom>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	else
	{
	   *m=8;
	   if(d.affect>=800)
	     *n=1;
	   else if(d.affect<800 && d.affect>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	break;   

   case 4:
	if(num>=2)
	{
	   *m=8+num;
	   switch(num)
	   {
	   	case 2:
	   		if(d.love>100)	*n=1;
	   		else if(d.love>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 3:
	   		if(d.homework>100) *n=1;
	   		else if(d.homework>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 4:
	   		if(d.hp>600) *n=1;
	   		else if(d.hp>300) *n=2;
	   		else *n=3;
	   		break;
	   	case 5:
	   		if(d.cookskill>200) *n=1;
	   		else if(d.cookskill>100) *n=2;
	   		else *n=3;
	   		break;
	   	case 6:
	   		if((d.belief+d.etchics)>600) *n=1;
	   		else if((d.belief+d.etchics)>200) *n=2;
	   		else *n=3;
	   		break;
	   	case 7:
	   		if(d.speech>150) *n=1;
	   		else if(d.speech>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 8:
	   		if((d.hp+d.wrist)>900) *n=1;
	   		else if((d.hp+d.wrist)>600) *n=2;
	   		else *n=3;
	   		break;
	   	case 9:
	   	case 11:
	   		if(d.art>250) *n=1;
	   		else if(d.art>100) *n=2;
	   		else *n=3;
	   		break;
	   	case 10:
	   		if(d.hskill>250) *n=1;
	   		else if(d.hskill>100) *n=2;
	   		else *n=3;
	   		break;
	   	case 12:
	   		if(d.belief>500) *n=1;
	   		else if(d.belief>200) *n=2;
	   		else *n=3;
	   		break;
	   	case 13:
	   		if(d.wisdom>500) *n=1;
	   		else if(d.wisdom>200) *n=2;
	   		else *n=3;
	   		break;	
	   	case 14:
	   	case 16:
	   		if(d.charm>1000) *n=1;
	   		else if(d.charm>500) *n=2;
	   		else *n=3;
	   		break;
	   	case 15:
	   		if(d.charm>700) *n=1;
	   		else if(d.charm>300) *n=2;
	   		else *n=3;
	   		break;	   	
	   }
	}
	else
	{
	   *m=9;
	   if(d.etchics > 400)
	     *n=1;
	   else if(d.etchics >200)
	     *n=2;
	   else
	     *n=3;
	}
	break;
   case 5:
	if(num>=2)
	{
	   *m=24+num;
	   switch(num)
	   {
	   	case 2:
	   	case 3:
	   		if(d.hp>400) *n=1;
	   		else if(d.hp>150) *n=2;
	   		else *n=3;
	   		break;
	   	case 4:
	   	case 10:
	   	case 11:
	   		if(d.hp>600) *n=1;
	   		else if(d.hp>300) *n=2;
	   		else *n=3;
	   		break;
	   	case 5:
	   		if(d.cookskill>150) *n=1;
	   		else if(d.cookskill>80) *n=2;
	   		else *n=3;
	   		break;
	   	case 6:
	   		if((d.belief+d.etchics)>600) *n=1;
	   		else if((d.belief+d.etchics)>200) *n=2;
	   		else *n=3;
	   		break;
	   	case 7:
	   		if(d.speech>150) *n=1;
	   		else if(d.speech>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 8:
	   		if((d.hp+d.wrist)>700) *n=1;
	   		else if((d.hp+d.wrist)>300) *n=2;
	   		else *n=3;
	   		break;
	   	case 9:
	   		if(d.art>100) *n=1;
	   		else if(d.art>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 12:
	   		if(d.hp>300) *n=1;
	   		else if(d.hp>150) *n=2;
	   		else *n=3;
	   		break;
	   	case 13:
	   		if(d.speech>100) *n=1;
	   		else if(d.speech>40) *n=2;
	   		else *n=3;
	   		break;	
	   	case 14:
	   	case 16:
	   		if(d.charm>1000) *n=1;
	   		else if(d.charm>500) *n=2;
	   		else *n=3;
	   		break;
	   	case 15:
	   		if(d.charm>700) *n=1;
	   		else if(d.charm>300) *n=2;
	   		else *n=3;
	   		break;	   	
	   }
	}
	else
	{
	   *m=25;
	   if(d.relation > 100)
	     *n=1;
	   else if(d.relation >50)
	     *n=2;
	   else
	     *n=3;
	}
	break; 
 } 

 if(d.sex==1)
   strcpy(buf, endmodeall_purpose[*m].boy);
 else
   strcpy(buf, endmodeall_purpose[*m].girl);
 *grade=endmodeall_purpose[*m].grade;
 return 0;
}

int
pip_endingart(buf,m,n,grade) /*ÒÕÊõ*/
char *buf;
int *m,*n,*grade;
{
 if(d.speech>=100)
 {
   *m=1;
   if(d.hp>=300 && d.affect>=350)
     *n=1;
   else if(d.hp<300 && d.affect<350 && d.hp>=250 && d.affect>=300)
     *n=2;
   else   
     *n=3;
 } 
 else if(d.wisdom>=400)
 {
   *m=2;
   if(d.affect>=500)
     *n=1;
   else if(d.affect<500 && d.affect>=450)
     *n=2;
   else   
     *n=3;
 }
 else if(d.classI>=d.classJ)
 {
   *m=3;
   if(d.affect>=350)
     *n=1;
   else if(d.affect<350 && d.affect>=300)
     *n=2;
   else   
     *n=3;
 }
 else 
 {
   *m=4;
   if(d.affect>=200 && d.hp>150)
     *n=1;
   else if(d.affect<200 && d.affect>=180 && d.hp>150)
     *n=2;
   else   
     *n=3;
 }
 if(d.sex==1)
   strcpy(buf, endmodeart[*m].boy);
 else
   strcpy(buf, endmodeart[*m].girl);
 *grade=endmodeart[*m].grade;  
 return 0;
}

int
pip_max_worktime(num)
int *num;
{
  int data=20;
  if(d.workA>data)
  {
     data=d.workA;
     *num=1;
  }
  if(d.workB>data)
  {
     data=d.workB;
     *num=2;
  }
  if(d.workC>data)
  {
     data=d.workC;
     *num=3;
  }
  if(d.workD>data)
  {
     data=d.workD;
     *num=4;
  }
  if(d.workE>data)
  {
     data=d.workE;
     *num=5;
  }

  if(d.workF>data)
  {
     data=d.workF;
     *num=6;
  }
  if(d.workG>data)
  {
     data=d.workG;
     *num=7;
  }
  if(d.workH>data)
  {
     data=d.workH;
     *num=8;
  }
  if(d.workI>data)
  {
     data=d.workI;
     *num=9;
  }
  if(d.workJ>data)
  {
     data=d.workJ;
     *num=10;
  }
  if(d.workK>data)
  {
     data=d.workK;
     *num=11;
  }
  if(d.workL>data)
  {
     data=d.workL;
     *num=12;
  }
  if(d.workM>data)
  {
     data=d.workM;
     *num=13;
  }
  if(d.workN>data)
  {
     data=d.workN;
     *num=14;
  }
  if(d.workO>data)
  {
     data=d.workO;
     *num=16;
  }
  if(d.workP>data)
  {
     data=d.workP;
     *num=16;
  }  

  return data;
}

int pip_game_over(endgrade)
int endgrade;
{
	long gradebasic;
	long gradeall;
	
	gradebasic=(d.maxhp+d.wrist+d.wisdom+d.character+d.charm+d.etchics+d.belief+d.affect)/10-d.offense;
	clrchyiuan(1,23);
	gradeall=gradebasic+endgrade;
	move(8,17);
	prints("[1;36m¸ĞĞ»ÄúÍæÍêÕû¸öĞÇ¿ÕĞ¡¼¦µÄÓÎÏ·.....[0m");
	move(10,17);
	prints("[1;37m¾­¹ıÏµÍ³¼ÆËãµÄ½á¹û£º[0m");
	move(12,17);
	prints("[1;36mÄúµÄĞ¡¼¦ [37m%s [36m×ÜµÃ·Ö£½ [1;5;33m%d [0m",d.name,gradeall);
	return gradeall;
}

int pip_divine() /*Õ¼²·Ê¦À´·Ã*/
{
  char buf[256];
  char ans[4];
  char endbuf1[50];
  char endbuf2[50];
  char endbuf3[50];
  int endgrade=0;
  int endmode=0;
  long money;
  int tm;
  int randvalue;
  
  tm=d.bbtime/60/30;
  move(b_lines-2,0);
  money=300*(tm+1);
  clrchyiuan(0,24);
  move(10,14);
  prints("[1;33;5mßµßµßµ...[0;1;37mÍ»È»´«À´ÕóÕóµÄÇÃÃÅÉù.........[0m");
  pressanykey("È¥ÇÆÇÆÊÇË­°É......");
  clrchyiuan(0,24);
  move(10,14);
  prints("[1;37;46m    Ô­À´ÊÇÔÆÓÎËÄº£µÄÕ¼²·Ê¦À´·ÃÁË.......    [0m");
  pressanykey("¿ªÃÅÈÃËû½øÀ´°É....");
  if(d.money>=money)
  {
    randvalue=rand()%5;
    sprintf(buf,"ÄãÒªÕ¼²·Âğ? Òª»¨%dÔªà¸...[Y/n]",money);
#ifdef MAPLE
    getdata(12,14,buf, ans, 2, 1, 0);
#else
    getdata(12,14,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(ans[0]!='N' && ans[0]!='n')
    {
      pip_ending_decide(endbuf1,endbuf2,endbuf3,&endmode,&endgrade);
      if(randvalue==0)
      		sprintf(buf,"[1;37m  ÄãµÄĞ¡¼¦%sÒÔáá¿ÉÄÜµÄÉí·İÊÇ%s  [0m",d.name,endmodemagic[2+rand()%5].girl);
      else if(randvalue==1)
      		sprintf(buf,"[1;37m  ÄãµÄĞ¡¼¦%sÒÔáá¿ÉÄÜµÄÉí·İÊÇ%s  [0m",d.name,endmodecombat[2+rand()%6].girl);
      else if(randvalue==2)
      		sprintf(buf,"[1;37m  ÄãµÄĞ¡¼¦%sÒÔáá¿ÉÄÜµÄÉí·İÊÇ%s  [0m",d.name,endmodeall_purpose[6+rand()%15].girl);
      else if(randvalue==3)
      		sprintf(buf,"[1;37m  ÄãµÄĞ¡¼¦%sÒÔáá¿ÉÄÜµÄÉí·İÊÇ%s  [0m",d.name,endmodeart[2+rand()%6].girl);
      else if(randvalue==4)
      		sprintf(buf,"[1;37m  ÄãµÄĞ¡¼¦%sÒÔáá¿ÉÄÜµÄÉí·İÊÇ%s  [0m",d.name,endbuf1);
      d.money-=money;
      clrchyiuan(0,24);
      move(10,14);
      prints("[1;33mÔÚÎÒÕ¼²·½á¹û¿´À´....[0m");
      move(12,14);
      prints(buf);
      pressanykey("Ğ»Ğ»»İ¹Ë£¬ÓĞÔµÔÙ¼ûÃæÁË.(²»×¼²»ÄÜ¹ÖÎÒà¸)");
    }
    else
    {
      pressanykey("Äã²»ÏëÕ¼²·°¡?..Õæ¿ÉÏ§..ÄÇÖ»ÓĞµÈÏÂ´Î°É...");
    }
  }
  else
  {
    pressanykey("ÄãµÄÇ®²»¹»à¸..ÕæÊÇ¿ÉÏ§..µÈÏÂ´Î°É...");
  }
  return 0;
}
