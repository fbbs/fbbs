#include <dlfcn.h>
#include "bbs.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15

int domenu(const char *menu_name);
int Announce(), Personal(), Boards(), EGroup(), Info(), Goodbye(),
		BoardGroup();
int New(), GoodBrds(), Read(), Select(), Users(), Welcome();
int setcalltime();
int show_allmsgs(), x_lockscreen(), x_showuser();
int Conditions(), x_cloak(), t_users(), x_info(), x_fillform(), x_vote();
int x_results(), ent_bnet(), a_edits(), x_edits();
int shownotepad(), x_userdefine(), x_csh();
int m_new(), m_read(), m_send(), g_send();
int ov_send(), s_msg(), mailall(), offline();
int r_searchall();
//added by iamfat 2002.09.04
/*2003.04.23 added by stephen*/
int giveUpBBS();
/*stephen add end*/
#ifdef SMS
void sms_menu();
#endif

int ent_bnet2();

#ifdef ALLOWGAME
int ent_winmine();
#endif

#ifdef INTERNET_EMAIL
int m_internet();
#endif

int t_users(), t_friends(), t_list(), t_monitor();
int t_query(), t_talk(), t_pager(), t_friend(), t_reject(), x_cloak();
int ent_chat();
int AddPCorpus(); // deardragon 个人文集 
int sendgoodwish();
//int	show_myfile();

#ifdef DLM 
int exec_mbem();
#endif
//modified by money 2002.11.15

#ifndef WITHOUT_ADMIN_TOOLS
int kick_user(), m_vote();
#ifndef DLM
int x_new_denylevel();
int x_level(), m_info();
int d_user(), m_register();
int d_board(), m_editbrd(), m_newbrd();
int m_ordainBM(), m_retireBM();
int setsystempasswd();
#endif
#endif

int wall();
int friend_wall();
/*Add By Excellent */

typedef struct {
	char *name;
	/*	int     (*fptr) ();*/
	void (*fptr);
	int type;
} MENU;

MENU currcmd;

//保存字符串所对应的函数
static MENU sysconf_cmdlist[] = {
	{ "domenu", domenu, 0 },
	{ "EGroups", EGroup, 0 },
	{ "BGroups", BoardGroup, 0 },
	{ "BoardsAll", Boards, 0 },
	{ "BoardsGood", GoodBrds, 0 },
	{ "BoardsNew", New, 0 },
	{ "LeaveBBS", Goodbye, 0 },
	{ "Announce", Announce, 0 },
	{ "Personal", Personal, 0 },
	{ "SelectBoard", Select, 0 },
	{ "ReadBoard", Read, 0 },
	{ "SetAlarm", setcalltime, 0 },
	{ "MailAll", mailall, 0 },
	{ "LockScreen", x_lockscreen, 0 },
	{ "ShowUser", x_showuser, 0 },
	{ "OffLine", offline, 0 },
	{ "GiveUpBBS", giveUpBBS, 0 },
#ifdef SMS
	{ "Sms_Menu", sms_menu, 0},
#endif
	{ "ReadNewMail", m_new, 0 },
	{ "ReadMail", m_read, 0 },
	{ "SendMail", m_send, 0 },
	{ "GroupSend", g_send, 0 },
	{ "OverrideSend", ov_send, 0 },
#ifdef INTERNET_EMAIL
	{ "SendNetMail", m_internet, 0},
#endif
	{ "UserDefine", x_userdefine, 0 },
	{ "ShowFriends", t_friends, 0 },
	{ "ShowLogins", t_users, 0 },
	{ "QueryUser", t_query, 0 },
	{ "Talk", t_talk, 0 },
	{ "SetPager", t_pager, 0 },
	{ "SetCloak", x_cloak, 0 },
	{ "SendMsg", s_msg, 0 },
	{ "ShowMsg", show_allmsgs, 0 },
	{ "SetFriends", t_friend, 0 },
	{ "SetRejects", t_reject, 0 },
	{ "RFriendWall", friend_wall, 	0 },
	{ "EnterChat", ent_chat, 0 },
	{ "ListLogins", t_list, 0 },
	{ "Monitor", t_monitor, 0 },
	{ "FillForm", x_fillform, 0 },
	{ "Information", x_info, 0 },
	{ "EditUFiles", x_edits, 0 },
	{ "ShowLicense", Conditions, 0 },
	{ "ShowVersion", Info, 0 },
	{ "Notepad", shownotepad, 0 },
	{ "Vote", x_vote, 0 },
	{ "VoteResult", x_results, 0 },
	{ "ExecBBSNet", ent_bnet, 0 },
	{ "ExecBBSNet2", ent_bnet2, 0 },
	{ "ShowWelcome", Welcome, 0 },
	{ "AllUsers", Users, 0 },
	{ "AddPCorpus", AddPCorpus, 0 },
	{ "GoodWish", sendgoodwish, 0 },
#ifdef ALLOWSWITCHCODE
	{ "SwitchCode", switch_code, 0 },
#endif
#ifdef ALLOWGAME
	{ "WinMine", ent_winmine,0},
	{ "Gagb", "@mod:so/game.so#gagb", 1},
	{ "BlackJack", "@mod:so/game.so#BlackJack", 1},
	{ "X_dice", "@mod:so/game.so#x_dice", 1},
	{ "P_gp", "@mod:so/game.so#p_gp", 1},
	{ "IP_nine", "@mod:so/game.so#p_nine", 1},
	{ "OBingo", "@mod:so/game.so#bingo", 1},
	{ "Chicken", "@mod:so/game.so#chicken_main", 1},
	{ "Mary", "@mod:so/game.so#mary_m", 1},
	{ "Borrow", "@mod:so/game.so#borrow", 1},
	{ "Payoff", "@mod:so/game.so#payoff", 1},
	{ "Impawn", "@mod:so/game.so#popshop", 1},
	{ "Doshopping", "@mod:so/game.so#doshopping", 1},
	{ "Lending", "@mod:so/game.so#lending", 1},
	{ "StarChicken", "@mod:so/pip.so#mod_default", 1},
#endif 
#ifdef DLM 
	{ "RunMBEM",exec_mbem,0},
#endif
	{ "Kick", kick_user, 0 },
	{ "OpenVote", m_vote, 0 },
	{ "SearchAll", r_searchall, 0 },
#ifndef DLM
	{ "Setsyspass", setsystempasswd, 0 },
	{ "Register", m_register, 0 },
	{ "ShowRegister", show_register, 0 },
	{ "Info", m_info, 0 },
	{ "Level", x_level, 0 },
	{ "OrdainBM", m_ordainBM, 0 },
	{ "RetireBM", m_retireBM, 0 },
	{ "NewChangeLevel", x_new_denylevel, 0 },
	{ "DelUser", d_user, 0 },
	{ "NewBoard", m_newbrd, 0 },
	{ "ChangeBrd", m_editbrd, 0 },
	{ "BoardDel", d_board, 0 },
	{ "SysFiles", a_edits, 0 },
	{ "Wall", wall, 0 },
#else
	{ "Setsyspass", "@mod:so/admintool.so#setsystempasswd",1},
	{ "Register", "@mod:so/admintool.so#m_register", 1},
	{ "ShowRegister", "@mod:so/admintool.so#show_register",1},
	{ "Info", "@mod:so/admintool.so#m_info", 1},
	{ "Level", "@mod:so/admintool.so#x_level", 1},
	{ "OrdainBM", "@mod:so/admintool.so#m_ordainBM", 1},
	{ "RetireBM", "@mod:so/admintool.so#m_retireBM", 1},
	{ "ChangeLevel", "@mod:so/admintool.so#x_denylevel", 1},
	{ "NewChangeLevel", "@mod:so/admintool.so#x_new_denylevel", 1},
	{ "DelUser", "@mod:so/admintool.so#d_user", 1},
	{ "NewBoard", "@mod:so/admintool.so#m_newbrd", 1},
	{ "ChangeBrd", "@mod:so/admintool.so#m_editbrd", 1},
	{ "BoardDel", "@mod:so/admintool.so#d_board", 1},
	{ "SysFiles", "@mod:so/admintool.so#a_edits", 1},
	{ "Wall", "@mod:so/admintool.so#wall", 1},
#endif
	{ 0, 0, 0 }
};

void *sysconf_funcptr(const char *func_name, int *type)
{
	int n = 0;
	char *str;

	while ((str = sysconf_cmdlist[n].name) != NULL) {
		if (strcmp(func_name, str) == 0) {
			*type = sysconf_cmdlist[n].type;
			return (sysconf_cmdlist[n].fptr);
		}
		n++;
	}
	*type = -1;
	return NULL;
}

#ifdef DLM 
//modified by money 2002.11.15
int exec_mbem(char *s)
{
	void *hdll;
	int (*func)();
	char *c;
	char buf[80];

	strcpy(buf,s);
	s = strstr(buf,"@mod:");
	if (s)	{
		c = strstr(s + 5,"#");
		if (c) {
			*c = 0;
			c++;
		}
		hdll = dlopen(s + 5, RTLD_LAZY);
		if (hdll) {
			if (func = dlsym(hdll, c ? c : "mod_main"))
				func();
			else
				report(dlerror(), currentuser.userid);
			dlclose(hdll);
		}
		else {
			report(dlerror(), currentuser.userid);
		}
	}
}
#endif

void decodestr(char *str)
{
	register char ch;
	int n;

	while ((ch = *str++) != '\0')
		if (ch != '\01')
			outc(ch);
		else
			if (*str != '\0' && str[1] != '\0') {
				ch = *str++;
				n = *str++;
				while (--n >= 0)
					outc(ch);
			}
}

void load_sysconf_image(const char *imgfile)
{
	struct sysheader shead;
	struct stat st;
	char *ptr, *func;
	int fh, n, diff, x;

	if ((fh = open(imgfile, O_RDONLY))> 0) {
		fstat(fh, &st);
		ptr = malloc(st.st_size);
		if (ptr == NULL)
			report( "Insufficient memory available", "");
		read(fh, &shead, sizeof(shead));
		read(fh, ptr, st.st_size);
		close(fh);

		menuitem = (void *) ptr;
		ptr += shead.menu * sizeof(struct smenuitem);
		sysvar = (void *) ptr;
		ptr += shead.key * sizeof(struct sdefine);
		sysconf_buf = (void *) ptr;
		ptr += shead.len;
		sysconf_menu = shead.menu;
		sysconf_key = shead.key;
		sysconf_len = shead.len;
		diff = sysconf_buf - shead.buf;
		for (n = 0; n < sysconf_menu; n++) {
			menuitem[n].name += diff;
			menuitem[n].desc += diff;
			menuitem[n].arg += diff;
			func = (char *) menuitem[n].fptr;
			menuitem[n].fptr = sysconf_funcptr(func + diff, &x);
		}
		for (n = 0; n < sysconf_key; n++) {
			sysvar[n].key += diff;
			sysvar[n].str += diff;
		}
	}
}

void load_sysconf(void)
{
	if (!dashf("sysconf.img")) {
		report("build sysconf.img", "");
		build_sysconf("etc/sysconf.ini", "sysconf.img");
	}
	load_sysconf_image("sysconf.img");
}

int domenu_screen(struct smenuitem *pm)
{
	char *str;
	int line, col, num;

	clear();
	line = 3;
	col = 0;
	num = 0;
	while (1) {
		switch (pm->level) {
			case -1:
				return (num);
			case -2:
				if (strcmp(pm->name, "title") == 0) {
					firsttitle(pm->desc);
				} else if (strcmp(pm->name, "screen") == 0) {
					if ((str = sysconf_str(pm->desc)) != NULL) {
						move(pm->line, pm->col);
						decodestr(str);
					}
				}
				break;
			default:
				if (pm->line >= 0 && HAS_PERM(pm->level)) {
					if (pm->line == 0) {
						pm->line = line;
						pm->col = col;
					} else {
						line = pm->line;
						col = pm->col;
					}
					move(line, col);
					prints("  %s", pm->desc);
					line++;
				} else {
					if (pm->line> 0) {
						line = pm->line;
						col = pm->col;
					}
					pm->line = -1;
				}
		}
		num++;
		pm++;
	}
}

int domenu(const char *menu_name)
{
	extern int refscreen;
	struct smenuitem *pm;
	int size, now;
	int cmd, i;
	if (sysconf_menu <= 0) {
		return -1;
	}
	pm = &menuitem[sysconf_eval(menu_name)];
	size = domenu_screen(pm);
	now = 0;
	if (strcmp(menu_name, "TOPMENU") == 0 && chkmail()) {
		for (i = 0; i < size; i++)
		if (pm[i].line> 0 && pm[i].name[0] == 'M')
		now = i;

	}
	modify_user_mode(MMENU);
	R_monitor();
	while (1) {
		printacbar();
		while (pm[now].level < 0 || !HAS_PERM(pm[now].level)) {
			now++;
			if (now >= size)
			now = 0;
		}
		move(pm[now].line, pm[now].col);
		prints("> ");
		move(pm[now].line, pm[now].col+1);
		cmd = egetch();
		move(pm[now].line, pm[now].col);
		prints("  ");
		switch (cmd) {
			case EOF:
			if (!refscreen) {
				abort_bbs(0);
			}
			domenu_screen(pm);
			modify_user_mode(MMENU);
			R_monitor();
			break;
			case KEY_RIGHT:
			for (i = 0; i < size; i++) {
				if (pm[i].line == pm[now].line && pm[i].level >= 0 &&
						pm[i].col> pm[now].col && HAS_PERM(pm[i].level))
				break;
			}
			if (i < size) {
				now = i;
				break;
			}
			case '\n':
			case '\r':
			if (strcmp(pm[now].arg, "..") == 0) {
				return 0;
			}
			if (pm[now].fptr != NULL) {
				int type;

				(void *) sysconf_funcptr(pm[now].name, &type);
#ifdef DLM 
				if (type == 1) {
					exec_mbem((char *)pm[now].fptr);
				} else
#endif
				//modified by money 2002.11.15
				(*pm[now].fptr) (pm[now].arg);
				if (pm[now].fptr == Select) {
					now++;
				}
				domenu_screen(pm);
				modify_user_mode(MMENU);
				R_monitor();
			}
			break;
			case KEY_LEFT:
			for (i = 0; i < size; i++) {
				if (pm[i].line == pm[now].line && pm[i].level >= 0 &&
						pm[i].col < pm[now].col && HAS_PERM(pm[i].level))
				break;
				if (pm[i].fptr == Goodbye)
				break;
			}
			if (i < size) {
				now = i;
				break;
			}
			return 0;
			case KEY_DOWN:
			now++;
			break;
			case KEY_UP:
			now--;
			while (pm[now].level < 0 || !HAS_PERM(pm[now].level)) {
				if (now> 0)
				now--;
				else
				now = size - 1;
			}
			break;
			// Modified by Flier - 2000.5.12 - Begin
			case KEY_PGUP:
			now = 0;
			break;
			case KEY_PGDN:
			now = size - 1;
			while (pm[now].level < 0 || !HAS_PERM(pm[now].level)) now--;
			break;
			// Modified by Flier - 2000.5.12 - End
			case '~':
			if (!(HAS_PERM(PERM_ESYSFILE))) {
				//modified by roly 02.01.24 add PERM_WELCOME
				break;
			}
			free(menuitem);
			report("rebuild sysconf.img", currentuser.userid);
			build_sysconf("etc/sysconf.ini", "sysconf.img");
			report("reload sysconf.img", currentuser.userid);
			load_sysconf_image("sysconf.img");
			pm = &menuitem[sysconf_eval(menu_name)];
			ActiveBoard_Init();
			size = domenu_screen(pm);
			now = 0;
			break;
			case '!': /* youzi leave */
			if (strcmp("TOPMENU", menu_name) == 0)
			break;
			else
			return Goodbye();
			default:
			if (cmd >= 'a' && cmd <= 'z')
			cmd = cmd - 'a' + 'A';
			for (i = 0; i < size; i++) {
				if (pm[i].line> 0 && cmd == pm[i].name[0] &&
						HAS_PERM(pm[i].level)) {
					now = i;
					break;
				}
			}
		}
	}
}
