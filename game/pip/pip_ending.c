/*---------------------------------------------------------------------------*/
/* 结局函式                                                                  */
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
/*  结局参数设定                                                            */
/*--------------------------------------------------------------------------*/
struct newendingset
{
  char *girl;		/*女生结局的职业*/ 
  char *boy;		/*男生结局的职业*/
  int grade;		/*评分*/
};
typedef struct newendingset newendingset;

/*万能*/
struct newendingset endmodeall_purpose[] = {
//% "女性职业",		"男生职业",		0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",		"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",		0,
//% "成为这个国家新女王",	"成为这个国家新国王",	500,
"\xb3\xc9\xce\xaa\xd5\xe2\xb8\xf6\xb9\xfa\xbc\xd2\xd0\xc2\xc5\xae\xcd\xf5",	"\xb3\xc9\xce\xaa\xd5\xe2\xb8\xf6\xb9\xfa\xbc\xd2\xd0\xc2\xb9\xfa\xcd\xf5",	500,
//% "成为国家的宰相",	"成为国家的宰相",	400,
"\xb3\xc9\xce\xaa\xb9\xfa\xbc\xd2\xb5\xc4\xd4\xd7\xcf\xe0",	"\xb3\xc9\xce\xaa\xb9\xfa\xbc\xd2\xb5\xc4\xd4\xd7\xcf\xe0",	400,
//% "成为教会中的大主教",	"成为教会中的大主教",	350,
"\xb3\xc9\xce\xaa\xbd\xcc\xbb\xe1\xd6\xd0\xb5\xc4\xb4\xf3\xd6\xf7\xbd\xcc",	"\xb3\xc9\xce\xaa\xbd\xcc\xbb\xe1\xd6\xd0\xb5\xc4\xb4\xf3\xd6\xf7\xbd\xcc",	350,
//% "成为国家的大臣",	"成为国家的大臣",	320,
"\xb3\xc9\xce\xaa\xb9\xfa\xbc\xd2\xb5\xc4\xb4\xf3\xb3\xbc",	"\xb3\xc9\xce\xaa\xb9\xfa\xbc\xd2\xb5\xc4\xb4\xf3\xb3\xbc",	320,
//% "成为一位博士",		"成为一位博士",		300,
"\xb3\xc9\xce\xaa\xd2\xbb\xce\xbb\xb2\xa9\xca\xbf",		"\xb3\xc9\xce\xaa\xd2\xbb\xce\xbb\xb2\xa9\xca\xbf",		300,
//% "成为教会中的修女",	"成为教会中的神父",	150,
"\xb3\xc9\xce\xaa\xbd\xcc\xbb\xe1\xd6\xd0\xb5\xc4\xd0\xde\xc5\xae",	"\xb3\xc9\xce\xaa\xbd\xcc\xbb\xe1\xd6\xd0\xb5\xc4\xc9\xf1\xb8\xb8",	150,
//% "成为法庭上的大法官",   "成为法庭上的法官",	200,
"\xb3\xc9\xce\xaa\xb7\xa8\xcd\xa5\xc9\xcf\xb5\xc4\xb4\xf3\xb7\xa8\xb9\xd9",   "\xb3\xc9\xce\xaa\xb7\xa8\xcd\xa5\xc9\xcf\xb5\xc4\xb7\xa8\xb9\xd9",	200,
//% "成为知名的学者",	"成为知名的学者",	120,
"\xb3\xc9\xce\xaa\xd6\xaa\xc3\xfb\xb5\xc4\xd1\xa7\xd5\xdf",	"\xb3\xc9\xce\xaa\xd6\xaa\xc3\xfb\xb5\xc4\xd1\xa7\xd5\xdf",	120,
//% "成为一名女官",		"成为一名男官",		100,
"\xb3\xc9\xce\xaa\xd2\xbb\xc3\xfb\xc5\xae\xb9\xd9",		"\xb3\xc9\xce\xaa\xd2\xbb\xc3\xfb\xc4\xd0\xb9\xd9",		100,
//% "在育幼院工作",		"在育幼院工作",		100,
"\xd4\xda\xd3\xfd\xd3\xd7\xd4\xba\xb9\xa4\xd7\xf7",		"\xd4\xda\xd3\xfd\xd3\xd7\xd4\xba\xb9\xa4\xd7\xf7",		100,
//% "在旅馆工作",		"在旅馆工作",		100,
"\xd4\xda\xc2\xc3\xb9\xdd\xb9\xa4\xd7\xf7",		"\xd4\xda\xc2\xc3\xb9\xdd\xb9\xa4\xd7\xf7",		100,
//% "在农场工作",		"在农场工作",		100,
"\xd4\xda\xc5\xa9\xb3\xa1\xb9\xa4\xd7\xf7",		"\xd4\xda\xc5\xa9\xb3\xa1\xb9\xa4\xd7\xf7",		100,
//% "在餐厅工作",		"在餐厅工作",		100,
"\xd4\xda\xb2\xcd\xcc\xfc\xb9\xa4\xd7\xf7",		"\xd4\xda\xb2\xcd\xcc\xfc\xb9\xa4\xd7\xf7",		100,
//% "在教堂工作",		"在教堂工作",		100,
"\xd4\xda\xbd\xcc\xcc\xc3\xb9\xa4\xd7\xf7",		"\xd4\xda\xbd\xcc\xcc\xc3\xb9\xa4\xd7\xf7",		100,
//% "在地摊工作",		"在地摊工作",		100,
"\xd4\xda\xb5\xd8\xcc\xaf\xb9\xa4\xd7\xf7",		"\xd4\xda\xb5\xd8\xcc\xaf\xb9\xa4\xd7\xf7",		100,
//% "在伐木场工作",		"在伐木场工作",		100,
"\xd4\xda\xb7\xa5\xc4\xbe\xb3\xa1\xb9\xa4\xd7\xf7",		"\xd4\xda\xb7\xa5\xc4\xbe\xb3\xa1\xb9\xa4\xd7\xf7",		100,
//% "在美容院工作",		"在美容院工作",		100,
"\xd4\xda\xc3\xc0\xc8\xdd\xd4\xba\xb9\xa4\xd7\xf7",		"\xd4\xda\xc3\xc0\xc8\xdd\xd4\xba\xb9\xa4\xd7\xf7",		100,
//% "在狩猎区工作",		"在狩猎区工作",		100,
"\xd4\xda\xe1\xf7\xc1\xd4\xc7\xf8\xb9\xa4\xd7\xf7",		"\xd4\xda\xe1\xf7\xc1\xd4\xc7\xf8\xb9\xa4\xd7\xf7",		100,
//% "在工地工作",		"在工地工作",		100,
"\xd4\xda\xb9\xa4\xb5\xd8\xb9\xa4\xd7\xf7",		"\xd4\xda\xb9\xa4\xb5\xd8\xb9\xa4\xd7\xf7",		100,
//% "在墓园工作",		"在墓园工作",		100,
"\xd4\xda\xc4\xb9\xd4\xb0\xb9\xa4\xd7\xf7",		"\xd4\xda\xc4\xb9\xd4\xb0\xb9\xa4\xd7\xf7",		100,
//% "担任家庭教师工作",	"担任家庭教师工作",	100,
"\xb5\xa3\xc8\xce\xbc\xd2\xcd\xa5\xbd\xcc\xca\xa6\xb9\xa4\xd7\xf7",	"\xb5\xa3\xc8\xce\xbc\xd2\xcd\xa5\xbd\xcc\xca\xa6\xb9\xa4\xd7\xf7",	100,
//% "在酒家工作",		"在酒家工作",		100,
"\xd4\xda\xbe\xc6\xbc\xd2\xb9\xa4\xd7\xf7",		"\xd4\xda\xbe\xc6\xbc\xd2\xb9\xa4\xd7\xf7",		100,
//% "在酒店工作",		"在酒店工作",		100,
"\xd4\xda\xbe\xc6\xb5\xea\xb9\xa4\xd7\xf7",		"\xd4\xda\xbe\xc6\xb5\xea\xb9\xa4\xd7\xf7",		100,
//% "在大夜总会工作",	"在大夜总会工作",	100,
"\xd4\xda\xb4\xf3\xd2\xb9\xd7\xdc\xbb\xe1\xb9\xa4\xd7\xf7",	"\xd4\xda\xb4\xf3\xd2\xb9\xd7\xdc\xbb\xe1\xb9\xa4\xd7\xf7",	100,
//% "在家中帮忙",		"在家中帮忙",		50,
"\xd4\xda\xbc\xd2\xd6\xd0\xb0\xef\xc3\xa6",		"\xd4\xda\xbc\xd2\xd6\xd0\xb0\xef\xc3\xa6",		50,
//% "在育幼院兼差",		"在育幼院兼差",		50,
"\xd4\xda\xd3\xfd\xd3\xd7\xd4\xba\xbc\xe6\xb2\xee",		"\xd4\xda\xd3\xfd\xd3\xd7\xd4\xba\xbc\xe6\xb2\xee",		50,
//% "在旅馆兼差",		"在旅馆兼差",		50,
"\xd4\xda\xc2\xc3\xb9\xdd\xbc\xe6\xb2\xee",		"\xd4\xda\xc2\xc3\xb9\xdd\xbc\xe6\xb2\xee",		50,
//% "在农场兼差",		"在农场兼差",		50,
"\xd4\xda\xc5\xa9\xb3\xa1\xbc\xe6\xb2\xee",		"\xd4\xda\xc5\xa9\xb3\xa1\xbc\xe6\xb2\xee",		50,
//% "在餐厅兼差",		"在餐厅兼差",		50,
"\xd4\xda\xb2\xcd\xcc\xfc\xbc\xe6\xb2\xee",		"\xd4\xda\xb2\xcd\xcc\xfc\xbc\xe6\xb2\xee",		50,
//% "在教堂兼差",		"在教堂兼差",		50,
"\xd4\xda\xbd\xcc\xcc\xc3\xbc\xe6\xb2\xee",		"\xd4\xda\xbd\xcc\xcc\xc3\xbc\xe6\xb2\xee",		50,
//% "在地摊兼差",		"在地摊兼差",		50,
"\xd4\xda\xb5\xd8\xcc\xaf\xbc\xe6\xb2\xee",		"\xd4\xda\xb5\xd8\xcc\xaf\xbc\xe6\xb2\xee",		50,
//% "在伐木场兼差",		"在伐木场兼差",		50,
"\xd4\xda\xb7\xa5\xc4\xbe\xb3\xa1\xbc\xe6\xb2\xee",		"\xd4\xda\xb7\xa5\xc4\xbe\xb3\xa1\xbc\xe6\xb2\xee",		50,
//% "在美容院兼差",		"在美容院兼差",		50,
"\xd4\xda\xc3\xc0\xc8\xdd\xd4\xba\xbc\xe6\xb2\xee",		"\xd4\xda\xc3\xc0\xc8\xdd\xd4\xba\xbc\xe6\xb2\xee",		50,
//% "在狩猎区兼差",		"在狩猎区兼差",		50,
"\xd4\xda\xe1\xf7\xc1\xd4\xc7\xf8\xbc\xe6\xb2\xee",		"\xd4\xda\xe1\xf7\xc1\xd4\xc7\xf8\xbc\xe6\xb2\xee",		50,
//% "在工地兼差",		"在工地兼差",		50,
"\xd4\xda\xb9\xa4\xb5\xd8\xbc\xe6\xb2\xee",		"\xd4\xda\xb9\xa4\xb5\xd8\xbc\xe6\xb2\xee",		50,
//% "在墓园兼差",		"在墓园兼差",		50,
"\xd4\xda\xc4\xb9\xd4\xb0\xbc\xe6\xb2\xee",		"\xd4\xda\xc4\xb9\xd4\xb0\xbc\xe6\xb2\xee",		50,
//% "担任家庭教师兼差",	"担任家庭教师兼差",	50,
"\xb5\xa3\xc8\xce\xbc\xd2\xcd\xa5\xbd\xcc\xca\xa6\xbc\xe6\xb2\xee",	"\xb5\xa3\xc8\xce\xbc\xd2\xcd\xa5\xbd\xcc\xca\xa6\xbc\xe6\xb2\xee",	50,
//% "在酒家兼差",		"在酒家兼差",		50,
"\xd4\xda\xbe\xc6\xbc\xd2\xbc\xe6\xb2\xee",		"\xd4\xda\xbe\xc6\xbc\xd2\xbc\xe6\xb2\xee",		50,
//% "在酒店兼差",		"在酒店兼差",		50,
"\xd4\xda\xbe\xc6\xb5\xea\xbc\xe6\xb2\xee",		"\xd4\xda\xbe\xc6\xb5\xea\xbc\xe6\xb2\xee",		50,
//% "在大夜总会兼差",	"在大夜总会兼差",	50,
"\xd4\xda\xb4\xf3\xd2\xb9\xd7\xdc\xbb\xe1\xbc\xe6\xb2\xee",	"\xd4\xda\xb4\xf3\xd2\xb9\xd7\xdc\xbb\xe1\xbc\xe6\xb2\xee",	50,
NULL,		NULL,	0
};

/*战斗*/
struct newendingset endmodecombat[] = {
//% "女性职业",		"男生职业",			0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",		"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",			0,
//% "被封为勇者 战士型",  	"被封为勇者 战士型",		420,
"\xb1\xbb\xb7\xe2\xce\xaa\xd3\xc2\xd5\xdf \xd5\xbd\xca\xbf\xd0\xcd",  	"\xb1\xbb\xb7\xe2\xce\xaa\xd3\xc2\xd5\xdf \xd5\xbd\xca\xbf\xd0\xcd",		420,
//% "被拔擢成为一国的将军",	"被拔擢成为一国的将军",		300,
"\xb1\xbb\xb0\xce\xdf\xaa\xb3\xc9\xce\xaa\xd2\xbb\xb9\xfa\xb5\xc4\xbd\xab\xbe\xfc",	"\xb1\xbb\xb0\xce\xdf\xaa\xb3\xc9\xce\xaa\xd2\xbb\xb9\xfa\xb5\xc4\xbd\xab\xbe\xfc",		300,
//% "当上了国家近卫队队长",	"当上了国家近卫队队长",		200,
"\xb5\xb1\xc9\xcf\xc1\xcb\xb9\xfa\xbc\xd2\xbd\xfc\xce\xc0\xb6\xd3\xb6\xd3\xb3\xa4",	"\xb5\xb1\xc9\xcf\xc1\xcb\xb9\xfa\xbc\xd2\xbd\xfc\xce\xc0\xb6\xd3\xb6\xd3\xb3\xa4",		200,
//% "当了武术老师",		"当了武术老师",			150,
"\xb5\xb1\xc1\xcb\xce\xe4\xca\xf5\xc0\xcf\xca\xa6",		"\xb5\xb1\xc1\xcb\xce\xe4\xca\xf5\xc0\xcf\xca\xa6",			150,
//% "变成骑士报效国家",	"变成骑士报效国家",		160,
"\xb1\xe4\xb3\xc9\xc6\xef\xca\xbf\xb1\xa8\xd0\xa7\xb9\xfa\xbc\xd2",	"\xb1\xe4\xb3\xc9\xc6\xef\xca\xbf\xb1\xa8\xd0\xa7\xb9\xfa\xbc\xd2",		160,
//% "投身军旅生活，成为士兵","投身军旅生活，成为士兵",	80,
"\xcd\xb6\xc9\xed\xbe\xfc\xc2\xc3\xc9\xfa\xbb\xee\xa3\xac\xb3\xc9\xce\xaa\xca\xbf\xb1\xf8","\xcd\xb6\xc9\xed\xbe\xfc\xc2\xc3\xc9\xfa\xbb\xee\xa3\xac\xb3\xc9\xce\xaa\xca\xbf\xb1\xf8",	80,
//% "变成奖金猎人",		"变成奖金猎人",			0,
"\xb1\xe4\xb3\xc9\xbd\xb1\xbd\xf0\xc1\xd4\xc8\xcb",		"\xb1\xe4\xb3\xc9\xbd\xb1\xbd\xf0\xc1\xd4\xc8\xcb",			0,
//% "以  兵工作维生",	"以  兵工作维生",		0,
"\xd2\xd4  \xb1\xf8\xb9\xa4\xd7\xf7\xce\xac\xc9\xfa",	"\xd2\xd4  \xb1\xf8\xb9\xa4\xd7\xf7\xce\xac\xc9\xfa",		0,
NULL,           NULL,   0
};

/*魔法*/
struct newendingset endmodemagic[] = {
//% "女性职业",	     	"男生职业",		0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",	     	"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",		0,
//% "被封为勇者 魔法型",	"被封为勇者 魔法型",	420,
"\xb1\xbb\xb7\xe2\xce\xaa\xd3\xc2\xd5\xdf \xc4\xa7\xb7\xa8\xd0\xcd",	"\xb1\xbb\xb7\xe2\xce\xaa\xd3\xc2\xd5\xdf \xc4\xa7\xb7\xa8\xd0\xcd",	420,
//% "被聘为王宫魔法师",	"被聘为王官魔法师",	280,
"\xb1\xbb\xc6\xb8\xce\xaa\xcd\xf5\xb9\xac\xc4\xa7\xb7\xa8\xca\xa6",	"\xb1\xbb\xc6\xb8\xce\xaa\xcd\xf5\xb9\xd9\xc4\xa7\xb7\xa8\xca\xa6",	280,
//% "当了魔法老师",		"当了魔法老师",		160,
"\xb5\xb1\xc1\xcb\xc4\xa7\xb7\xa8\xc0\xcf\xca\xa6",		"\xb5\xb1\xc1\xcb\xc4\xa7\xb7\xa8\xc0\xcf\xca\xa6",		160,
//% "变成一位魔导士",	"变成一位魔导士",	180,
"\xb1\xe4\xb3\xc9\xd2\xbb\xce\xbb\xc4\xa7\xb5\xbc\xca\xbf",	"\xb1\xe4\xb3\xc9\xd2\xbb\xce\xbb\xc4\xa7\xb5\xbc\xca\xbf",	180,
//% "当了魔法师",		"当了魔法师",		120,
"\xb5\xb1\xc1\xcb\xc4\xa7\xb7\xa8\xca\xa6",		"\xb5\xb1\xc1\xcb\xc4\xa7\xb7\xa8\xca\xa6",		120,
//% "以占卜师帮人算命为生",	"以占卜师帮人算命为生",	40,
"\xd2\xd4\xd5\xbc\xb2\xb7\xca\xa6\xb0\xef\xc8\xcb\xcb\xe3\xc3\xfc\xce\xaa\xc9\xfa",	"\xd2\xd4\xd5\xbc\xb2\xb7\xca\xa6\xb0\xef\xc8\xcb\xcb\xe3\xc3\xfc\xce\xaa\xc9\xfa",	40,
//% "成为一个魔术师",	"成为一个魔术师",	20,
"\xb3\xc9\xce\xaa\xd2\xbb\xb8\xf6\xc4\xa7\xca\xf5\xca\xa6",	"\xb3\xc9\xce\xaa\xd2\xbb\xb8\xf6\xc4\xa7\xca\xf5\xca\xa6",	20,
//% "成为街头艺人",		"成为街头艺人",		10,
"\xb3\xc9\xce\xaa\xbd\xd6\xcd\xb7\xd2\xd5\xc8\xcb",		"\xb3\xc9\xce\xaa\xbd\xd6\xcd\xb7\xd2\xd5\xc8\xcb",		10,
NULL,           NULL	,0
};

/*社交*/
struct newendingset endmodesocial[] = {
//% "女性职业",     	"男生职业",		0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",     	"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",		0,
//% "成为国王的宠妃",	"成为女王的驸马爷",	170,
"\xb3\xc9\xce\xaa\xb9\xfa\xcd\xf5\xb5\xc4\xb3\xe8\xe5\xfa",	"\xb3\xc9\xce\xaa\xc5\xae\xcd\xf5\xb5\xc4\xe6\xe2\xc2\xed\xd2\xaf",	170,
//% "被挑选成为王妃",	"被选中当女王的夫婿",	260,
"\xb1\xbb\xcc\xf4\xd1\xa1\xb3\xc9\xce\xaa\xcd\xf5\xe5\xfa",	"\xb1\xbb\xd1\xa1\xd6\xd0\xb5\xb1\xc5\xae\xcd\xf5\xb5\xc4\xb7\xf2\xd0\xf6",	260,
//% "被伯爵看中，成为夫人",	"成为了女伯爵的夫婿",	130,
"\xb1\xbb\xb2\xae\xbe\xf4\xbf\xb4\xd6\xd0\xa3\xac\xb3\xc9\xce\xaa\xb7\xf2\xc8\xcb",	"\xb3\xc9\xce\xaa\xc1\xcb\xc5\xae\xb2\xae\xbe\xf4\xb5\xc4\xb7\xf2\xd0\xf6",	130,
//% "成为富豪的妻子",	"成为女富豪的夫婿",	100,
"\xb3\xc9\xce\xaa\xb8\xbb\xba\xc0\xb5\xc4\xc6\xde\xd7\xd3",	"\xb3\xc9\xce\xaa\xc5\xae\xb8\xbb\xba\xc0\xb5\xc4\xb7\xf2\xd0\xf6",	100,
//% "成为商人的妻子",	"成为女商人的夫婿",	80,
"\xb3\xc9\xce\xaa\xc9\xcc\xc8\xcb\xb5\xc4\xc6\xde\xd7\xd3",	"\xb3\xc9\xce\xaa\xc5\xae\xc9\xcc\xc8\xcb\xb5\xc4\xb7\xf2\xd0\xf6",	80,
//% "成为农人的妻子",	"成为女农人的夫婿",	80,
"\xb3\xc9\xce\xaa\xc5\xa9\xc8\xcb\xb5\xc4\xc6\xde\xd7\xd3",	"\xb3\xc9\xce\xaa\xc5\xae\xc5\xa9\xc8\xcb\xb5\xc4\xb7\xf2\xd0\xf6",	80,
//% "成为地主的情妇",	"成为女地主的情夫",	-40,
"\xb3\xc9\xce\xaa\xb5\xd8\xd6\xf7\xb5\xc4\xc7\xe9\xb8\xbe",	"\xb3\xc9\xce\xaa\xc5\xae\xb5\xd8\xd6\xf7\xb5\xc4\xc7\xe9\xb7\xf2",	-40,
NULL,           NULL,	0
};
/*艺术*/
struct newendingset endmodeart[] = {
//% "女性职业",		"男生职业",	0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",		"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",	0,
//% "成为了小丑",		"成为了小丑",	100,
"\xb3\xc9\xce\xaa\xc1\xcb\xd0\xa1\xb3\xf3",		"\xb3\xc9\xce\xaa\xc1\xcb\xd0\xa1\xb3\xf3",	100,
//% "成为了作家",		"成为了作家",	100,
"\xb3\xc9\xce\xaa\xc1\xcb\xd7\xf7\xbc\xd2",		"\xb3\xc9\xce\xaa\xc1\xcb\xd7\xf7\xbc\xd2",	100,
//% "成为了画家",		"成为了画家",	100,
"\xb3\xc9\xce\xaa\xc1\xcb\xbb\xad\xbc\xd2",		"\xb3\xc9\xce\xaa\xc1\xcb\xbb\xad\xbc\xd2",	100,
//% "成为了舞蹈家",		"成为了舞蹈家",	100,
"\xb3\xc9\xce\xaa\xc1\xcb\xce\xe8\xb5\xb8\xbc\xd2",		"\xb3\xc9\xce\xaa\xc1\xcb\xce\xe8\xb5\xb8\xbc\xd2",	100,
NULL,           NULL,	0
};

/*暗黑*/
struct newendingset endmodeblack[] = {
//% "女性职业",     	"男生职业",		0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",     	"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",		0,
//% "变成了魔王",		"变成了魔王",		-1000,
"\xb1\xe4\xb3\xc9\xc1\xcb\xc4\xa7\xcd\xf5",		"\xb1\xe4\xb3\xc9\xc1\xcb\xc4\xa7\xcd\xf5",		-1000,
//% "混成了太妹",		"混成了流氓",		-350,
"\xbb\xec\xb3\xc9\xc1\xcb\xcc\xab\xc3\xc3",		"\xbb\xec\xb3\xc9\xc1\xcb\xc1\xf7\xc3\xa5",		-350,
//% "做了[ＳＭ女王]的工作",	"做了[ＳＭ国王]的工作",	-150,
"\xd7\xf6\xc1\xcb[\xa3\xd3\xa3\xcd\xc5\xae\xcd\xf5]\xb5\xc4\xb9\xa4\xd7\xf7",	"\xd7\xf6\xc1\xcb[\xa3\xd3\xa3\xcd\xb9\xfa\xcd\xf5]\xb5\xc4\xb9\xa4\xd7\xf7",	-150,
//% "当了黑街的大姐",	"当了黑街的老大",	-500,
"\xb5\xb1\xc1\xcb\xba\xda\xbd\xd6\xb5\xc4\xb4\xf3\xbd\xe3",	"\xb5\xb1\xc1\xcb\xba\xda\xbd\xd6\xb5\xc4\xc0\xcf\xb4\xf3",	-500,
//% "变成高级娼妇",		"变成高级情夫",		-350,
"\xb1\xe4\xb3\xc9\xb8\xdf\xbc\xb6\xe6\xbd\xb8\xbe",		"\xb1\xe4\xb3\xc9\xb8\xdf\xbc\xb6\xc7\xe9\xb7\xf2",		-350,
//% "变成诈欺师诈欺别人",	"变成金光党骗别人钱",	-350,
"\xb1\xe4\xb3\xc9\xd5\xa9\xc6\xdb\xca\xa6\xd5\xa9\xc6\xdb\xb1\xf0\xc8\xcb",	"\xb1\xe4\xb3\xc9\xbd\xf0\xb9\xe2\xb5\xb3\xc6\xad\xb1\xf0\xc8\xcb\xc7\xae",	-350,
//% "以流莺的工作生活",	"以牛郎的工作生活",	-350,
"\xd2\xd4\xc1\xf7\xdd\xba\xb5\xc4\xb9\xa4\xd7\xf7\xc9\xfa\xbb\xee",	"\xd2\xd4\xc5\xa3\xc0\xc9\xb5\xc4\xb9\xa4\xd7\xf7\xc9\xfa\xbb\xee",	-350,
NULL,		NULL,	0
};

/*家事*/
struct newendingset endmodefamily[] = {
//% "女性职业",     	"男生职业",		0,
"\xc5\xae\xd0\xd4\xd6\xb0\xd2\xb5",     	"\xc4\xd0\xc9\xfa\xd6\xb0\xd2\xb5",		0,
//% "正在新娘修行",		"正在新郎修行",		50,
"\xd5\xfd\xd4\xda\xd0\xc2\xc4\xef\xd0\xde\xd0\xd0",		"\xd5\xfd\xd4\xda\xd0\xc2\xc0\xc9\xd0\xde\xd0\xd0",		50,
NULL,		NULL,	0
};


int /*结局画面*/
pip_ending_screen()
{
  time_t now;
  char buf[256];
  char endbuf1[50];
  char endbuf2[50];
  char endbuf3[50];
  int endgrade=0;
  int endmode=0;
  screen_clear();
  pip_ending_decide(endbuf1,endbuf2,endbuf3,&endmode,&endgrade);
  move(1,9); 
  //% prints("[1;33m┏━━━┓┏━━  ┓┏━━━  ┏━━━┓┏━━  ┓  ━━━  [0m");
  prints("[1;33m\xa9\xb3\xa9\xa5\xa9\xa5\xa9\xa5\xa9\xb7\xa9\xb3\xa9\xa5\xa9\xa5  \xa9\xb7\xa9\xb3\xa9\xa5\xa9\xa5\xa9\xa5  \xa9\xb3\xa9\xa5\xa9\xa5\xa9\xa5\xa9\xb7\xa9\xb3\xa9\xa5\xa9\xa5  \xa9\xb7  \xa9\xa5\xa9\xa5\xa9\xa5  [0m");
  move(2,9);
  //% prints("[1;37m┃      ┃┃    ┃┃┃      ┃┃      ┃┃    ┃┃┃      ┃[0m");
  prints("[1;37m\xa9\xa7      \xa9\xa7\xa9\xa7    \xa9\xa7\xa9\xa7\xa9\xa7      \xa9\xa7\xa9\xa7      \xa9\xa7\xa9\xa7    \xa9\xa7\xa9\xa7\xa9\xa7      \xa9\xa7[0m");
  move(3,9);
  //% prints("[0;37m┃    ━  ┃    ┃┃┃      ┃┗━┓┏┛┃    ┃┃┃  ┏━┓[0m");
  prints("[0;37m\xa9\xa7    \xa9\xa5  \xa9\xa7    \xa9\xa7\xa9\xa7\xa9\xa7      \xa9\xa7\xa9\xbb\xa9\xa5\xa9\xb7\xa9\xb3\xa9\xbf\xa9\xa7    \xa9\xa7\xa9\xa7\xa9\xa7  \xa9\xb3\xa9\xa5\xa9\xb7[0m");
  move(4,9);
  //% prints("[0;37m┃    ━  ┃  ┃  ┃┃      ┃┏━┛┗┓┃  ┃  ┃┃      ┃[0m");
  prints("[0;37m\xa9\xa7    \xa9\xa5  \xa9\xa7  \xa9\xa7  \xa9\xa7\xa9\xa7      \xa9\xa7\xa9\xb3\xa9\xa5\xa9\xbf\xa9\xbb\xa9\xb7\xa9\xa7  \xa9\xa7  \xa9\xa7\xa9\xa7      \xa9\xa7[0m");
  move(5,9);
  //% prints("[1;37m┃      ┃┃  ┃  ┃┃      ┃┃      ┃┃  ┃  ┃┃      ┃[0m");
  prints("[1;37m\xa9\xa7      \xa9\xa7\xa9\xa7  \xa9\xa7  \xa9\xa7\xa9\xa7      \xa9\xa7\xa9\xa7      \xa9\xa7\xa9\xa7  \xa9\xa7  \xa9\xa7\xa9\xa7      \xa9\xa7[0m");
  move(6,9);
  //% prints("[1;35m┗━━━┛┗━  ━┛┗━━━  ┗━━━┛┗━  ━┛  ━━━  [0m");
  prints("[1;35m\xa9\xbb\xa9\xa5\xa9\xa5\xa9\xa5\xa9\xbf\xa9\xbb\xa9\xa5  \xa9\xa5\xa9\xbf\xa9\xbb\xa9\xa5\xa9\xa5\xa9\xa5  \xa9\xbb\xa9\xa5\xa9\xa5\xa9\xa5\xa9\xbf\xa9\xbb\xa9\xa5  \xa9\xa5\xa9\xbf  \xa9\xa5\xa9\xa5\xa9\xa5  [0m");
  move(7,8);
  //% prints("[1;31m——————————[41;37m 星空战斗鸡结局报告 [0;1;31m———————————[0m");
  prints("[1;31m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[41;37m \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6\xbd\xe1\xbe\xd6\xb1\xa8\xb8\xe6 [0;1;31m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[0m");
  move(9,10);
  //% prints("[1;36m这个时间不知不觉地还是到临了...[0m");
  prints("[1;36m\xd5\xe2\xb8\xf6\xca\xb1\xbc\xe4\xb2\xbb\xd6\xaa\xb2\xbb\xbe\xf5\xb5\xd8\xbb\xb9\xca\xc7\xb5\xbd\xc1\xd9\xc1\xcb...[0m");
  move(11,10);
  //% prints("[1;37m[33m%s[37m 得离开你的温暖怀抱，自己一只鸡在外面求生存了.....[0m",d.name);
  prints("[1;37m[33m%s[37m \xb5\xc3\xc0\xeb\xbf\xaa\xc4\xe3\xb5\xc4\xce\xc2\xc5\xaf\xbb\xb3\xb1\xa7\xa3\xac\xd7\xd4\xbc\xba\xd2\xbb\xd6\xbb\xbc\xa6\xd4\xda\xcd\xe2\xc3\xe6\xc7\xf3\xc9\xfa\xb4\xe6\xc1\xcb.....[0m",d.name);
  move(13,10);
  //% prints("[1;36m在你照顾教导他的这段时光，让他接触了很多领域，培养了很多的能力....[0m");
  prints("[1;36m\xd4\xda\xc4\xe3\xd5\xd5\xb9\xcb\xbd\xcc\xb5\xbc\xcb\xfb\xb5\xc4\xd5\xe2\xb6\xce\xca\xb1\xb9\xe2\xa3\xac\xc8\xc3\xcb\xfb\xbd\xd3\xb4\xa5\xc1\xcb\xba\xdc\xb6\xe0\xc1\xec\xd3\xf2\xa3\xac\xc5\xe0\xd1\xf8\xc1\xcb\xba\xdc\xb6\xe0\xb5\xc4\xc4\xdc\xc1\xa6....[0m");
  move(15,10);
  //% prints("[1;37m因为这些，让小鸡 [33m%s[37m 之後的生活，变得更多采多姿了........[0m",d.name);
  prints("[1;37m\xd2\xf2\xce\xaa\xd5\xe2\xd0\xa9\xa3\xac\xc8\xc3\xd0\xa1\xbc\xa6 [33m%s[37m \xd6\xae\xe1\xe1\xb5\xc4\xc9\xfa\xbb\xee\xa3\xac\xb1\xe4\xb5\xc3\xb8\xfc\xb6\xe0\xb2\xc9\xb6\xe0\xd7\xcb\xc1\xcb........[0m",d.name);
  move(17,10);
  //% prints("[1;36m对於你的关心，你的付出，你所有的爱......[0m");
  prints("[1;36m\xb6\xd4\xec\xb6\xc4\xe3\xb5\xc4\xb9\xd8\xd0\xc4\xa3\xac\xc4\xe3\xb5\xc4\xb8\xb6\xb3\xf6\xa3\xac\xc4\xe3\xcb\xf9\xd3\xd0\xb5\xc4\xb0\xae......[0m");
  move(19,10);
  //% prints("[1;37m[33m%s[37m 会永远都铭记在心的....[0m",d.name);
  prints("[1;37m[33m%s[37m \xbb\xe1\xd3\xc0\xd4\xb6\xb6\xbc\xc3\xfa\xbc\xc7\xd4\xda\xd0\xc4\xb5\xc4....[0m",d.name);
  //% pressanykey("接下来看未来发展");
  pressanykey("\xbd\xd3\xcf\xc2\xc0\xb4\xbf\xb4\xce\xb4\xc0\xb4\xb7\xa2\xd5\xb9");
  clrchyiuan(7,19);
  move(7,8);
  //% prints("[1;34m——————————[44;37m 星空战斗鸡未来发展 [0;1;34m———————————[0m");
  prints("[1;34m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[44;37m \xd0\xc7\xbf\xd5\xd5\xbd\xb6\xb7\xbc\xa6\xce\xb4\xc0\xb4\xb7\xa2\xd5\xb9 [0;1;34m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[0m");
  move(9,10);
  //% prints("[1;36m透过水晶球，让我们一起来看 [33m%s[36m 的未来发展吧.....[0m",d.name);
  prints("[1;36m\xcd\xb8\xb9\xfd\xcb\xae\xbe\xa7\xc7\xf2\xa3\xac\xc8\xc3\xce\xd2\xc3\xc7\xd2\xbb\xc6\xf0\xc0\xb4\xbf\xb4 [33m%s[36m \xb5\xc4\xce\xb4\xc0\xb4\xb7\xa2\xd5\xb9\xb0\xc9.....[0m",d.name);
  move(11,10);
  //% prints("[1;37m小鸡 [33m%s[37m 後来%s....[0m",d.name,endbuf1);
  prints("[1;37m\xd0\xa1\xbc\xa6 [33m%s[37m \xe1\xe1\xc0\xb4%s....[0m",d.name,endbuf1);
  move(13,10);
  //% prints("[1;36m因为他的之前的努力，使得他在这一方面%s....[0m",endbuf2);
  prints("[1;36m\xd2\xf2\xce\xaa\xcb\xfb\xb5\xc4\xd6\xae\xc7\xb0\xb5\xc4\xc5\xac\xc1\xa6\xa3\xac\xca\xb9\xb5\xc3\xcb\xfb\xd4\xda\xd5\xe2\xd2\xbb\xb7\xbd\xc3\xe6%s....[0m",endbuf2);
  move(15,10);
  //% prints("[1;37m至於小鸡的婚姻状况，他後来%s，婚姻算是很美满.....[0m",endbuf3);
  prints("[1;37m\xd6\xc1\xec\xb6\xd0\xa1\xbc\xa6\xb5\xc4\xbb\xe9\xd2\xf6\xd7\xb4\xbf\xf6\xa3\xac\xcb\xfb\xe1\xe1\xc0\xb4%s\xa3\xac\xbb\xe9\xd2\xf6\xcb\xe3\xca\xc7\xba\xdc\xc3\xc0\xc2\xfa.....[0m",endbuf3);
  move(17,10);
  //% prints("[1;36m嗯..这是一个不错的结局唷..........[0m");
  prints("[1;36m\xe0\xc5..\xd5\xe2\xca\xc7\xd2\xbb\xb8\xf6\xb2\xbb\xb4\xed\xb5\xc4\xbd\xe1\xbe\xd6\xe0\xa1..........[0m");
  //% pressanykey("我想  你一定很感动吧.....");
  pressanykey("\xce\xd2\xcf\xeb  \xc4\xe3\xd2\xbb\xb6\xa8\xba\xdc\xb8\xd0\xb6\xaf\xb0\xc9.....");
  show_ending_pic(0);
  //% pressanykey("看一看分数罗");
  pressanykey("\xbf\xb4\xd2\xbb\xbf\xb4\xb7\xd6\xca\xfd\xc2\xde");
  endgrade=pip_game_over(endgrade);
  //% pressanykey("下一页是小鸡资料  赶快copy下来做纪念");
  pressanykey("\xcf\xc2\xd2\xbb\xd2\xb3\xca\xc7\xd0\xa1\xbc\xa6\xd7\xca\xc1\xcf  \xb8\xcf\xbf\xeccopy\xcf\xc2\xc0\xb4\xd7\xf6\xbc\xcd\xc4\xee");
  pip_data_list();
  //% pressanykey("欢迎再来挑战....");
  pressanykey("\xbb\xb6\xd3\xad\xd4\xd9\xc0\xb4\xcc\xf4\xd5\xbd....");
  /*记录开始*/
  now=time(0);
  //% sprintf(buf, "[1;35m———————————————————————————————————————[0m\n");
  sprintf(buf, "[1;35m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[0m\n");
  pip_log_record(buf);
  //% sprintf(buf, "[1;37m在 [33m%s [37m的时候，[36m%s [37m的小鸡 [32m%s[37m 出现了结局[0m\n", Cdate(&now), cuser.userid,d.name);
  sprintf(buf, "[1;37m\xd4\xda [33m%s [37m\xb5\xc4\xca\xb1\xba\xf2\xa3\xac[36m%s [37m\xb5\xc4\xd0\xa1\xbc\xa6 [32m%s[37m \xb3\xf6\xcf\xd6\xc1\xcb\xbd\xe1\xbe\xd6[0m\n", Cdate(&now), cuser.userid,d.name);
  pip_log_record(buf);
  //% sprintf(buf, "[1;37m小鸡 [32m%s [37m努力加强自己，後来%s[0m\n[1;37m因为之前的努力，使得在这一方面%s[0m\n",d.name,endbuf1,endbuf2);
  sprintf(buf, "[1;37m\xd0\xa1\xbc\xa6 [32m%s [37m\xc5\xac\xc1\xa6\xbc\xd3\xc7\xbf\xd7\xd4\xbc\xba\xa3\xac\xe1\xe1\xc0\xb4%s[0m\n[1;37m\xd2\xf2\xce\xaa\xd6\xae\xc7\xb0\xb5\xc4\xc5\xac\xc1\xa6\xa3\xac\xca\xb9\xb5\xc3\xd4\xda\xd5\xe2\xd2\xbb\xb7\xbd\xc3\xe6%s[0m\n",d.name,endbuf1,endbuf2);
  pip_log_record(buf);
  //% sprintf(buf, "[1;37m至於婚姻状况，他後来%s，婚姻算是很美满.....[0m\n\n[1;37m小鸡 [32n%s[37m 的总积分＝ [33m%d[0m\n",endbuf3,d.name,endgrade);
  sprintf(buf, "[1;37m\xd6\xc1\xec\xb6\xbb\xe9\xd2\xf6\xd7\xb4\xbf\xf6\xa3\xac\xcb\xfb\xe1\xe1\xc0\xb4%s\xa3\xac\xbb\xe9\xd2\xf6\xcb\xe3\xca\xc7\xba\xdc\xc3\xc0\xc2\xfa.....[0m\n\n[1;37m\xd0\xa1\xbc\xa6 [32n%s[37m \xb5\xc4\xd7\xdc\xbb\xfd\xb7\xd6\xa3\xbd [33m%d[0m\n",endbuf3,d.name,endgrade);
  pip_log_record(buf);
  //% sprintf(buf, "[1;35m———————————————————————————————————————[0m\n");
  sprintf(buf, "[1;35m\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa\xa1\xaa[0m\n");
  pip_log_record(buf);  
  /*记录终止*/
  d.death=3;
  //% pipdie("[1;31m游戏结束罗...[m  ",3);
  pipdie("[1;31m\xd3\xce\xcf\xb7\xbd\xe1\xca\xf8\xc2\xde...[m  ",3);
  return 0;
}

int 
pip_ending_decide(endbuf1,endbuf2,endbuf3,endmode,endgrade)
char *endbuf1,*endbuf2,*endbuf3;
int *endmode,*endgrade;
{
  //% char *name[8][2]={{"男的","女的"},
  char *name[8][2]={{"\xc4\xd0\xb5\xc4","\xc5\xae\xb5\xc4"},
  	           //% {"嫁给王子","娶了公主"},
  	           {"\xbc\xde\xb8\xf8\xcd\xf5\xd7\xd3","\xc8\xa2\xc1\xcb\xb9\xab\xd6\xf7"},
  	           //% {"嫁给你","娶了你"},
  	           {"\xbc\xde\xb8\xf8\xc4\xe3","\xc8\xa2\xc1\xcb\xc4\xe3"},
                   //% {"嫁给商人Ａ","娶了女商人Ａ"},
                   {"\xbc\xde\xb8\xf8\xc9\xcc\xc8\xcb\xa3\xc1","\xc8\xa2\xc1\xcb\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc1"},
                   //% {"嫁给商人Ｂ","娶了女商人Ｂ"},
                   {"\xbc\xde\xb8\xf8\xc9\xcc\xc8\xcb\xa3\xc2","\xc8\xa2\xc1\xcb\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc2"},
                   //% {"嫁给商人Ｃ","娶了女商人Ｃ"},
                   {"\xbc\xde\xb8\xf8\xc9\xcc\xc8\xcb\xa3\xc3","\xc8\xa2\xc1\xcb\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc3"},
                   //% {"嫁给商人Ｄ","娶了女商人Ｄ"},
                   {"\xbc\xde\xb8\xf8\xc9\xcc\xc8\xcb\xa3\xc4","\xc8\xa2\xc1\xcb\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc4"},
                   //% {"嫁给商人Ｅ","娶了女商人Ｅ"}}; 
                   {"\xbc\xde\xb8\xf8\xc9\xcc\xc8\xcb\xa3\xc5","\xc8\xa2\xc1\xcb\xc5\xae\xc9\xcc\xc8\xcb\xa3\xc5"}}; 
  int m=0,n=0,grade=0;
  int modeall_purpose=0;
  char buf1[256];
  char buf2[256];
  
  *endmode=pip_future_decide(&modeall_purpose);
  switch(*endmode)
  {
  /*1:暗黑 2:艺术 3:万能 4:战士 5:魔法 6:社交 7:家事*/
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
    //% sprintf(buf2,"非常的顺利..");
    sprintf(buf2,"\xb7\xc7\xb3\xa3\xb5\xc4\xcb\xb3\xc0\xfb..");
  }
  else if(n==2)
  {
    *endgrade=grade+100;
    //% sprintf(buf2,"表现还不错..");
    sprintf(buf2,"\xb1\xed\xcf\xd6\xbb\xb9\xb2\xbb\xb4\xed..");
  }
  else if(n==3)
  {
    *endgrade=grade-10;
    //% sprintf(buf2,"常遇到很多问题....");
    sprintf(buf2,"\xb3\xa3\xd3\xf6\xb5\xbd\xba\xdc\xb6\xe0\xce\xca\xcc\xe2....");
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
      //% sprintf(buf2,"娶了同行的女孩");
      sprintf(buf2,"\xc8\xa2\xc1\xcb\xcd\xac\xd0\xd0\xb5\xc4\xc5\xae\xba\xa2");
    else
      //% sprintf(buf2,"嫁给了同行的男生");  
      sprintf(buf2,"\xbc\xde\xb8\xf8\xc1\xcb\xcd\xac\xd0\xd0\xb5\xc4\xc4\xd0\xc9\xfa");  
  } 
  strcpy(endbuf3, buf2);  
  return 0;
}
/*结局判断*/
/*1:暗黑 2:艺术 3:万能 4:战士 5:魔法 6:社交 7:家事*/
int
pip_future_decide(modeall_purpose)
int *modeall_purpose;
{
  int endmode;
  /*暗黑*/
  if((d.etchics==0 && d.offense >=100) || (d.etchics>0 && d.etchics<50 && d.offense >=250))
     endmode=1;
  /*艺术*/
  else if(d.art>d.hexp && d.art>d.mexp && d.art>d.hskill && d.art>d.mskill &&
          d.art>d.social && d.art>d.family && d.art>d.homework && d.art>d.wisdom &&
          d.art>d.charm && d.art>d.belief && d.art>d.manners && d.art>d.speech &&
          d.art>d.cookskill && d.art>d.love)
     endmode=2;
  /*战斗*/
  else if(d.hexp>=d.social && d.hexp>=d.mexp && d.hexp>=d.family)
  {
     *modeall_purpose=1;
     if(d.hexp>d.social+50 || d.hexp>d.mexp+50 || d.hexp>d.family+50)
        endmode=4;
     else
        endmode=3;     
  }
  /*魔法*/
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
/*结婚的判断*/
int
pip_marry_decide()
{
  int grade;
  if(d.lover!=0)
  {  
     /* 3 4 5 6 7:商人 */
     d.lover=d.lover;
     grade=80;
  }
  else
  {
     if(d.royalJ>=d.relation && d.royalJ>=100)
     {
        d.lover=1;  /*王子*/
        grade=200;
     }
     else if(d.relation>d.royalJ && d.relation>=100)
     {
        d.lover=2;  /*父亲或母亲*/
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
pip_endingblack(buf,m,n,grade) /*暗黑*/
char *buf;
int *m,*n,*grade;
{
 if(d.offense>=500 && d.mexp>=500) /*魔王*/
 {
   *m=1;
   if(d.mexp>=1000)
     *n=1;
   else if(d.mexp<1000 && d.mexp >=800)
     *n=2;
   else
     *n=3;
 }

else if(d.hexp>=600)  /*流氓*/
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
 else if(d.hexp>=320 && d.character>200 && d.charm< 200)	/*黑街老大*/
 {
   *m=4;
   if(d.hexp>=400)
     *n=1;
   else if(d.hexp<400 && d.hexp>=360)
     *n=2;
   else 
     *n=3;  
 }
 else if(d.character>=200 && d.charm >=200 && d.speech>70 && d.toman >70)  /*高级娼妇*/
 {
   *m=5;
   if(d.charm>=300)
     *n=1;
   else if(d.charm<300 && d.charm>=250)
     *n=2;
   else 
     *n=3;  
 }
 
 else if(d.wisdom>=450)  /*诈骗师*/
 {
   *m=6;
   if(d.wisdom>=550)
     *n=1;
   else if(d.wisdom<550 && d.wisdom>=500)
     *n=2;
   else 
     *n=3;  
 }
 
 else /*流莺*/
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
pip_endingsocial(buf,m,n,grade) /*社交*/
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
pip_endingmagic(buf,m,n,grade) /*魔法*/
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
pip_endingcombat(buf,m,n,grade) /*战斗*/
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
pip_endingfamily(buf,m,n,grade) /*家事*/
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
pip_endingall_purpose(buf,m,n,grade,mode) /*万能*/
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
pip_endingart(buf,m,n,grade) /*艺术*/
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
	//% prints("[1;36m感谢您玩完整个星空小鸡的游戏.....[0m");
	prints("[1;36m\xb8\xd0\xd0\xbb\xc4\xfa\xcd\xe6\xcd\xea\xd5\xfb\xb8\xf6\xd0\xc7\xbf\xd5\xd0\xa1\xbc\xa6\xb5\xc4\xd3\xce\xcf\xb7.....[0m");
	move(10,17);
	//% prints("[1;37m经过系统计算的结果：[0m");
	prints("[1;37m\xbe\xad\xb9\xfd\xcf\xb5\xcd\xb3\xbc\xc6\xcb\xe3\xb5\xc4\xbd\xe1\xb9\xfb\xa3\xba[0m");
	move(12,17);
	//% prints("[1;36m您的小鸡 [37m%s [36m总得分＝ [1;5;33m%d [0m",d.name,gradeall);
	prints("[1;36m\xc4\xfa\xb5\xc4\xd0\xa1\xbc\xa6 [37m%s [36m\xd7\xdc\xb5\xc3\xb7\xd6\xa3\xbd [1;5;33m%d [0m",d.name,gradeall);
	return gradeall;
}

int pip_divine() /*占卜师来访*/
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
  //% prints("[1;33;5m叩叩叩...[0;1;37m突然传来阵阵的敲门声.........[0m");
  prints("[1;33;5m\xdf\xb5\xdf\xb5\xdf\xb5...[0;1;37m\xcd\xbb\xc8\xbb\xb4\xab\xc0\xb4\xd5\xf3\xd5\xf3\xb5\xc4\xc7\xc3\xc3\xc5\xc9\xf9.........[0m");
  //% pressanykey("去瞧瞧是谁吧......");
  pressanykey("\xc8\xa5\xc7\xc6\xc7\xc6\xca\xc7\xcb\xad\xb0\xc9......");
  clrchyiuan(0,24);
  move(10,14);
  //% prints("[1;37;46m    原来是云游四海的占卜师来访了.......    [0m");
  prints("[1;37;46m    \xd4\xad\xc0\xb4\xca\xc7\xd4\xc6\xd3\xce\xcb\xc4\xba\xa3\xb5\xc4\xd5\xbc\xb2\xb7\xca\xa6\xc0\xb4\xb7\xc3\xc1\xcb.......    [0m");
  //% pressanykey("开门让他进来吧....");
  pressanykey("\xbf\xaa\xc3\xc5\xc8\xc3\xcb\xfb\xbd\xf8\xc0\xb4\xb0\xc9....");
  if(d.money>=money)
  {
    randvalue=rand()%5;
    //% sprintf(buf,"你要占卜吗? 要花%d元喔...[Y/n]",money);
    sprintf(buf,"\xc4\xe3\xd2\xaa\xd5\xbc\xb2\xb7\xc2\xf0? \xd2\xaa\xbb\xa8%d\xd4\xaa\xe0\xb8...[Y/n]",money);
#ifdef MAPLE
    getdata(12,14,buf, ans, 2, 1, 0);
#else
    getdata(12,14,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(ans[0]!='N' && ans[0]!='n')
    {
      pip_ending_decide(endbuf1,endbuf2,endbuf3,&endmode,&endgrade);
      if(randvalue==0)
      		//% sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodemagic[2+rand()%5].girl);
      		sprintf(buf,"[1;37m  \xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6%s\xd2\xd4\xe1\xe1\xbf\xc9\xc4\xdc\xb5\xc4\xc9\xed\xb7\xdd\xca\xc7%s  [0m",d.name,endmodemagic[2+rand()%5].girl);
      else if(randvalue==1)
      		//% sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodecombat[2+rand()%6].girl);
      		sprintf(buf,"[1;37m  \xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6%s\xd2\xd4\xe1\xe1\xbf\xc9\xc4\xdc\xb5\xc4\xc9\xed\xb7\xdd\xca\xc7%s  [0m",d.name,endmodecombat[2+rand()%6].girl);
      else if(randvalue==2)
      		//% sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodeall_purpose[6+rand()%15].girl);
      		sprintf(buf,"[1;37m  \xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6%s\xd2\xd4\xe1\xe1\xbf\xc9\xc4\xdc\xb5\xc4\xc9\xed\xb7\xdd\xca\xc7%s  [0m",d.name,endmodeall_purpose[6+rand()%15].girl);
      else if(randvalue==3)
      		//% sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodeart[2+rand()%6].girl);
      		sprintf(buf,"[1;37m  \xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6%s\xd2\xd4\xe1\xe1\xbf\xc9\xc4\xdc\xb5\xc4\xc9\xed\xb7\xdd\xca\xc7%s  [0m",d.name,endmodeart[2+rand()%6].girl);
      else if(randvalue==4)
      		//% sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endbuf1);
      		sprintf(buf,"[1;37m  \xc4\xe3\xb5\xc4\xd0\xa1\xbc\xa6%s\xd2\xd4\xe1\xe1\xbf\xc9\xc4\xdc\xb5\xc4\xc9\xed\xb7\xdd\xca\xc7%s  [0m",d.name,endbuf1);
      d.money-=money;
      clrchyiuan(0,24);
      move(10,14);
      //% prints("[1;33m在我占卜结果看来....[0m");
      prints("[1;33m\xd4\xda\xce\xd2\xd5\xbc\xb2\xb7\xbd\xe1\xb9\xfb\xbf\xb4\xc0\xb4....[0m");
      move(12,14);
      prints(buf);
      //% pressanykey("谢谢惠顾，有缘再见面了.(不准不能怪我喔)");
      pressanykey("\xd0\xbb\xd0\xbb\xbb\xdd\xb9\xcb\xa3\xac\xd3\xd0\xd4\xb5\xd4\xd9\xbc\xfb\xc3\xe6\xc1\xcb.(\xb2\xbb\xd7\xbc\xb2\xbb\xc4\xdc\xb9\xd6\xce\xd2\xe0\xb8)");
    }
    else
    {
      //% pressanykey("你不想占卜啊?..真可惜..那只有等下次吧...");
      pressanykey("\xc4\xe3\xb2\xbb\xcf\xeb\xd5\xbc\xb2\xb7\xb0\xa1?..\xd5\xe6\xbf\xc9\xcf\xa7..\xc4\xc7\xd6\xbb\xd3\xd0\xb5\xc8\xcf\xc2\xb4\xce\xb0\xc9...");
    }
  }
  else
  {
    //% pressanykey("你的钱不够喔..真是可惜..等下次吧...");
    pressanykey("\xc4\xe3\xb5\xc4\xc7\xae\xb2\xbb\xb9\xbb\xe0\xb8..\xd5\xe6\xca\xc7\xbf\xc9\xcf\xa7..\xb5\xc8\xcf\xc2\xb4\xce\xb0\xc9...");
  }
  return 0;
}
