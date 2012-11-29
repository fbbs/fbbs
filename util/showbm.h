#ifndef _SHOWBM_H_
#define _SHOWBM_H_

#define NOFILEDAYS      30      /* 本板无新文天数, >NOFILEDAYS以红色显示 */
#define NO0FILEDAYS     30      /* 精华区未整理天数, >NO0FILEDAYS以红色显示*/
#define BMNOLOGIN       30      /* 最近未上站天数, >BMNOLOGIN以红色显示 */
#define BMNOFILEDAYS    30      /* BM本板未发文天数, >BMNOFILEDAYS以红色显示*/
#define DAYSN           7       /* 修改本数字请同步修改下面head描述 */
#define REDMARK		"\x1B[1;31m"	/* 使后面的字体变成红色*/
#define ENDREDMARK	"\x1B[m"	/* 使后面的字变成原来的颜色*/

/* 如果有必要修改以下数据, 请注意空格的数目 */
/*
  const char *head =
"\x1B[1;33;44m版面名称            本周版面文章数      精华区文章总数  未整理精华区天数         版主ID             版主版面文章数      版主未上站天数   版面未发文天数 \x1B[0m\n\n";
*/

const char *head=
"\033[1;33;44m"
 //% "                  本周  精华区 未整理               本周   版面  未上站 未发文\033[m\n"
 "                  \xb1\xbe\xd6\xdc  \xbe\xab\xbb\xaa\xc7\xf8 \xce\xb4\xd5\xfb\xc0\xed               \xb1\xbe\xd6\xdc   \xb0\xe6\xc3\xe6  \xce\xb4\xc9\xcf\xd5\xbe \xce\xb4\xb7\xa2\xce\xc4\033[m\n"
"\033[1;33;44m"
 //% "版面名称         发文数 发文数  天数  版主ID       发文数 发文数  天数   天数 \033[m\n";
 "\xb0\xe6\xc3\xe6\xc3\xfb\xb3\xc6         \xb7\xa2\xce\xc4\xca\xfd \xb7\xa2\xce\xc4\xca\xfd  \xcc\xec\xca\xfd  \xb0\xe6\xd6\xf7ID       \xb7\xa2\xce\xc4\xca\xfd \xb7\xa2\xce\xc4\xca\xfd  \xcc\xec\xca\xfd   \xcc\xec\xca\xfd \033[m\n";
//BBS_Game             65     49      1 lemonyu          17    151      0      0

#endif

