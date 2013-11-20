#include "bbs.h"
#include "fbbs/board.h"
#include "fbbs/helper.h"
#include "fbbs/string.h"
#include "fbbs/terminal.h"

extern int numofsig;
extern int noreply;
#ifdef ENABLE_PREFIX
char prefixbuf[MAX_PREFIX][6];
int numofprefix;
#endif
void check_title(char *title, size_t size)
{
	char temp[50];
	trim(title);
	if (!strncasecmp(title, "Re:", 3) && !HAS_PERM(PERM_SYSOPS)) {
		//% snprintf(temp, sizeof(temp), "Re：%s", &title[3]);
		snprintf(temp, sizeof(temp), "Re\xa3\xba%s", &title[3]);
		strlcpy(title, temp, sizeof(title));
	}
	valid_title_gbk(title);
}
#ifdef ENABLE_PREFIX
void set_prefix() {
	char filename[STRLEN], buf[128];
	int i;
	FILE *fp;

	setvfile(filename, currboard, "prefix");
	if ((fp = fopen(filename, "r"))) {
		for (i = 0; i < MAX_PREFIX; i++) {
			if (!fgets(buf, STRLEN, fp)) {
				break;
			}
			if (i == 0 && (buf[0] == '\n' || buf[0] == ' ') )
			buf[0] = '\0';
			strtok(buf, " \r\n\t");
			string_remove_ansi_control_code(prefixbuf[i], buf);
			prefixbuf[i][4] = '\0';
		}
		numofprefix = i;
		fclose(fp);
	} else numofprefix = 0;
}

void print_prefixbuf(char *buf, int index) {
	int i;
	//% buf += sprintf(buf, "前缀:");
	buf += sprintf(buf, "\xc7\xb0\xd7\xba:");
	for (i = 0; i < numofprefix; i++ ) {
		if (i == 0 && prefixbuf[i][0] == '\0' )
		//% buf += sprintf(buf, "1.%s无%s",
		buf += sprintf(buf, "1.%s\xce\xde%s",
				(index == i + 1)?"\033[1m":"",
				(index == i + 1)?"\033[m":"");
		else
		buf += sprintf(buf, " %d.%s%s%s", i + 1,
				(index == i + 1)?"\033[1;33m":"",
				prefixbuf[i],
				(index == i + 1)?"\033[m":"");
	}
	sprintf(buf, "[%d]:", index);
}
#endif
int post_header(struct postheader *header)
{
	int anonyboard = 0;
#ifdef ENABLE_PREFIX
	int index = 0;
	char pbuf[128];
#endif
	char r_prompt[20], mybuf[256], ans[5];
	char titlebuf[STRLEN];
#ifdef IP_2_NAME
	extern char fromhost[];
#endif
#ifdef RNDSIGN  //随机签名档
	/*rnd_sign=0表明非随机,=1表明随机*/
	int oldset, rnd_sign = 0;
#endif

	//对当前签名档的越界处理
	if (currentuser.signature > numofsig || currentuser.signature < 0) {
		currentuser.signature = 1;
	}
#ifdef RNDSIGN
	if (numofsig> 0) {
		if (DEFINE(DEF_RANDSIGN)) {
			oldset = currentuser.signature;
			srand((unsigned) time(0));
			currentuser.signature = (rand() % numofsig) + 1;//产生随机签名档
			rnd_sign = 1; //标记随机签名档
		} else {
			rnd_sign = 0;
		}
	}
#endif

	/*如果当前是回复模式，则把原标题copy到当前标题中，并标记当前为回复模式
	 否则当前标题为空*/
	if (header->reply) {
		strcpy(titlebuf, header->title);
		header->include_mode = 'R'; //exchange 'R' and 'S' by Danielfree 07.04.05
	} else {
		titlebuf[0] = '\0';
#ifdef ENABLE_PREFIX
		set_prefix();
#endif
	}

	board_t board;
	get_board(currboard, &board);

	//如果是发表文章，则首先检查所在版面是否匿名（发信时不存在这个问题）
	if (header->postboard) {
		anonyboard = board.flag & BOARD_ANONY_FLAG;
	}

#ifdef IP_2_NAME
	if (anonyboard && (fromhost[0] < '1'||fromhost[0]> '9'))
	anonyboard = 0;
#endif

	// modified by roly 02.03.07
	// modified by iamfat 02.10.30
	header->anonymous = (anonyboard) ? 1 : 0;
	//header->chk_anony = 0;
	//modifiy end
#ifdef ENABLE_PREFIX
	if (numofprefix> 0)
	index = 1;
#endif	
	while (1) {
#ifdef ENABLE_PREFIX
		if (header->reply) {
			//% sprintf(r_prompt, "引言模式 [[1m%c[m]", header->include_mode);
			sprintf(r_prompt, "\xd2\xfd\xd1\xd4\xc4\xa3\xca\xbd [[1m%c[m]", header->include_mode);
			move(-4, 0);
		} else if (numofprefix == 0)
		move(-4, 0);
		else
		move(-5, 0);
#else
		if (header->reply)
			//% sprintf(r_prompt, "引言模式 [[1m%c[m]", header->include_mode);
			sprintf(r_prompt, "\xd2\xfd\xd1\xd4\xc4\xa3\xca\xbd [[1m%c[m]", header->include_mode);
		move(-4, 0);
#endif
		//清除该行内容
		clrtobot();

		//根据相应操作打印出当前信息
		prints(
				"[m%s [1m%s[m      %s    %s%s\n",
				//% (header->postboard) ? "发表文章於" : "收信人：",
				(header->postboard) ? "\xb7\xa2\xb1\xed\xce\xc4\xd5\xc2\xec\xb6" : "\xca\xd5\xd0\xc5\xc8\xcb\xa3\xba",
				header->ds,
				//% (anonyboard) ? (header->anonymous ? "[1m要[m使用匿名"
				(anonyboard) ? (header->anonymous ? "[1m\xd2\xaa[m\xca\xb9\xd3\xc3\xc4\xe4\xc3\xfb"
						//% : "[1m不[m使用匿名") : "",
						: "[1m\xb2\xbb[m\xca\xb9\xd3\xc3\xc4\xe4\xc3\xfb") : "",
				//% (header->postboard) ? ((header->locked) ? "[本文[1;33m不可以[m回复"
				(header->postboard) ? ((header->locked) ? "[\xb1\xbe\xce\xc4[1;33m\xb2\xbb\xbf\xc9\xd2\xd4[m\xbb\xd8\xb8\xb4"
						//% : "[本文[1;33m可以[m回复") : "",
						: "[\xb1\xbe\xce\xc4[1;33m\xbf\xc9\xd2\xd4[m\xbb\xd8\xb8\xb4") : "",
				(header->postboard && header->reply) ? ((header->mail_owner)
					//% ? ",且[1;33m发送[m本文至作者信箱]"
					? ",\xc7\xd2[1;33m\xb7\xa2\xcb\xcd[m\xb1\xbe\xce\xc4\xd6\xc1\xd7\xf7\xd5\xdf\xd0\xc5\xcf\xe4]"
					//% : ",且[1;33m不发送[m本文至作者信箱]")
					: ",\xc7\xd2[1;33m\xb2\xbb\xb7\xa2\xcb\xcd[m\xb1\xbe\xce\xc4\xd6\xc1\xd7\xf7\xd5\xdf\xd0\xc5\xcf\xe4]")
						: (header->postboard) ? "]" : "");
#ifdef ENABLE_PREFIX
		if (!header->reply && numofprefix) {
			if ((board.flag & BOARD_PREFIX_FLAG) && !header->title[0]) {
				index = 0;
				print_prefixbuf(pbuf, index);
				while (1) {
					getdata(-4, 0, pbuf, ans, 2, DOECHO, YEA);
					if (!ans[0])
					return NA;
					index = ans[0] - '0';
					if (index> 0 && index<= numofprefix) {
						print_prefixbuf(pbuf, index);
						break;
					}
				}
			} else {
				print_prefixbuf(pbuf, index);
			}
			move(-4, 0);
			outs(pbuf);
		}

		//对于回复和发信，title初始不为空.所以只有在发表文章时，才会出现"[正在设定主题]"
		move(-3, 0);
#endif
		//% prints("使用标题: [1m%-50s[m\n",
		prints("\xca\xb9\xd3\xc3\xb1\xea\xcc\xe2: [1m%-50s[m\n",
				//% (header->title[0] == '\0') ? "[正在设定主题]" : header->title);
				(header->title[0] == '\0') ? "[\xd5\xfd\xd4\xda\xc9\xe8\xb6\xa8\xd6\xf7\xcc\xe2]" : header->title);
#ifdef RNDSIGN
		//在回复模式下会出现相应的引言模式信息
		//% prints("使用第 [1m%d[m 个签名档     %s %s", currentuser.signature
		prints("\xca\xb9\xd3\xc3\xb5\xda [1m%d[m \xb8\xf6\xc7\xa9\xc3\xfb\xb5\xb5     %s %s", currentuser.signature
				//% ,(header->reply) ? r_prompt : "", (rnd_sign == 1) ? "[随机签名档]" : "");
				,(header->reply) ? r_prompt : "", (rnd_sign == 1) ? "[\xcb\xe6\xbb\xfa\xc7\xa9\xc3\xfb\xb5\xb5]" : "");
#else
		//% prints("使用第 [1m%d[m 个签名档     %s", currentuser.signature,
		prints("\xca\xb9\xd3\xc3\xb5\xda [1m%d[m \xb8\xf6\xc7\xa9\xc3\xfb\xb5\xb5     %s", currentuser.signature,
				(header->reply) ? r_prompt : "");
#endif
		//对于发表文章或者投条情况
		if (titlebuf[0] == '\0') {
			//move到相应的行，为输入做准备
			move(-1, 0);
			//% if (header->postboard == YEA || strcmp(header->title, "没主题"))
			if (header->postboard == YEA || strcmp(header->title, "\xc3\xbb\xd6\xf7\xcc\xe2"))
				string_remove_ansi_control_code(titlebuf, header->title);

			//从当前行获得用户输入放到titlebuf中，最多存入50-1个字节(此处会阻塞在用户输入上，只到响应enter)
			//% getdata(-1, 0, "标题: ", titlebuf, 50, DOECHO, NA);
			getdata(-1, 0, "\xb1\xea\xcc\xe2: ", titlebuf, 50, DOECHO, NA);
			check_title(titlebuf, sizeof(titlebuf));

			//在用户输入为空的情况下，如果是发表文章则直接取消，如果是投条用户还可以继续，信头为没主题
			if (titlebuf[0] == '\0') {
				if (header->title[0] != '\0') {
					titlebuf[0] = ' ';
					continue;
				} else
					return NA;
			}

			//将用户设定title复制到相应结构中
			strcpy(header->title, titlebuf);
			continue;
		}

		trim(header->title); //add by money 2003.10.29.
		move(-1, 0);

#ifdef ENABLE_PREFIX	
		sprintf(mybuf,
				//% "[1;32m0[m~[1;32m%d V[m看签名档%s [1;32mX[m随机签名档,[1;32mT[m标题%s%s%s,[1;32mQ[m放弃:",
				"[1;32m0[m~[1;32m%d V[m\xbf\xb4\xc7\xa9\xc3\xfb\xb5\xb5%s [1;32mX[m\xcb\xe6\xbb\xfa\xc7\xa9\xc3\xfb\xb5\xb5,[1;32mT[m\xb1\xea\xcc\xe2%s%s%s,[1;32mQ[m\xb7\xc5\xc6\xfa:",
				//% numofsig, (header->reply) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m 引言模式" : " \033[1;32mF\033[m前缀",
				numofsig, (header->reply) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m \xd2\xfd\xd1\xd4\xc4\xa3\xca\xbd" : " \033[1;32mF\033[m\xc7\xb0\xd7\xba",
				//% (anonyboard) ? "，[1;32mL[m匿名" : "",(header->postboard)?",[1;32mU[m属性":"",(header->postboard&&header->reply)?",[1;32mM[m寄信":"");
				(anonyboard) ? "\xa3\xac[1;32mL[m\xc4\xe4\xc3\xfb" : "",(header->postboard)?",[1;32mU[m\xca\xf4\xd0\xd4":"",(header->postboard&&header->reply)?",[1;32mM[m\xbc\xc4\xd0\xc5":"");
#else
		sprintf(
				mybuf,
				//% "[1;32m0[m~[1;32m%d V[m看签名档%s [1;32mX[m随机签名档,[1;32mT[m标题%s%s%s,[1;32mQ[m放弃:",
				"[1;32m0[m~[1;32m%d V[m\xbf\xb4\xc7\xa9\xc3\xfb\xb5\xb5%s [1;32mX[m\xcb\xe6\xbb\xfa\xc7\xa9\xc3\xfb\xb5\xb5,[1;32mT[m\xb1\xea\xcc\xe2%s%s%s,[1;32mQ[m\xb7\xc5\xc6\xfa:",
				numofsig,
				//% (header->reply) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m 引言模式"
				(header->reply) ? ",[1;32mS[m/[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m \xd2\xfd\xd1\xd4\xc4\xa3\xca\xbd"
						//% : "", (anonyboard) ? "，[1;32mL[m匿名" : "",
						: "", (anonyboard) ? "\xa3\xac[1;32mL[m\xc4\xe4\xc3\xfb" : "",
				//% (header->postboard) ? ",[1;32mU[m属性" : "",
				(header->postboard) ? ",[1;32mU[m\xca\xf4\xd0\xd4" : "",
				//% (header->postboard&&header->reply) ? ",[1;32mM[m寄信"
				(header->postboard&&header->reply) ? ",[1;32mM[m\xbc\xc4\xd0\xc5"
						: "");
#endif
		//打印出提示信息，并阻塞在用户输入动作上,用户最多输入2个字节
		getdata(-1, 0, mybuf, ans, 3, DOECHO, YEA);
		ans[0] = toupper(ans[0]);

		//用户对签名档设置，包括取消当前操作
		if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
			//有效的签名档选择
			if (atoi(ans) <= numofsig)
				currentuser.signature = atoi(ans);
		} else if (ans[0] == 'Q' || ans[0] == 'q') { //取消当前动作
			return -1;
		}
		//对于回复模式
		else if (header->reply && (ans[0] == 'Y' || ans[0] == 'N'
				|| ans[0] == 'A' || ans[0] == 'R'||ans[0]=='S')) {
			header->include_mode = ans[0];
		} //重新设置标题
		else if (ans[0] == 'T') {
			titlebuf[0] = '\0';
		}//对于匿名版的特别处理 
		else if (ans[0] == 'L' && anonyboard) {
			header->anonymous = !header->anonymous;
		}//对于文章中，可否回复的更改 
		else if (ans[0] == 'U' && header->postboard) {
			header->locked = !header->locked;
		}//对于回复文章时的特殊属性设置 
		else if (ans[0] == 'M' && header->postboard && header->reply) {
			header->mail_owner = !header->mail_owner;
		}//对签名档的处理 
		else if (ans[0] == 'V') {
			setuserfile(mybuf, "signatures");
			//% if (askyn("预设显示前三个签名档, 要显示全部吗", NA, YEA) == YEA)
			if (askyn("\xd4\xa4\xc9\xe8\xcf\xd4\xca\xbe\xc7\xb0\xc8\xfd\xb8\xf6\xc7\xa9\xc3\xfb\xb5\xb5, \xd2\xaa\xcf\xd4\xca\xbe\xc8\xab\xb2\xbf\xc2\xf0", NA, YEA) == YEA)
				ansimore(mybuf, YEA);
			else {
				screen_clear();
				ansimore2(mybuf, NA, 0, 18);
			}
#ifdef RNDSIGN
		}//随机签名档 
		else if (ans[0] == 'X') {
			if (rnd_sign == 0 && numofsig != 0) {
				oldset = currentuser.signature;
				srand((unsigned) time(0));
				currentuser.signature = (rand() % numofsig) + 1;
				rnd_sign = 1;
			} else if (rnd_sign == 1 && numofsig != 0) {
				rnd_sign = 0;
				currentuser.signature = oldset;
			}
			ans[0] = ' ';
#endif
		}
#ifdef ENABLE_PREFIX
		//修改前缀
		else if (!header->reply && numofprefix && ans[0] == 'F') {
			int i;
			getdata(-1, 0, pbuf, ans, 3, DOECHO, YEA);
			i = ans[0] - '0';
			if (i >= 0 && i <= numofprefix &&
					!(i == 0 && (board.flag & BOARD_PREFIX_FLAG)))
			index = i;
		}
#endif
		else {
			if (header->title[0] == '\0')
				return NA;
			else {
#ifdef ENABLE_PREFIX
				strcpy(header->prefix, index?prefixbuf[index - 1]:"");
#endif
				return YEA;
			}
		}
	}
}

