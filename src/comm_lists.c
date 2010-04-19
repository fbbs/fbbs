#include <dlfcn.h>
#include "bbs.h"
#include "sysconf.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif
//modified by money 2002.11.15

int domenu(const char *menu_name);
int Announce(), Personal(), board_read_all(), board_read_group(), Info(), Goodbye();
int board_read_new(), goodbrd_show(), Read(), Select(), Users(), Welcome();
int setcalltime();
int msg_more(), x_lockscreen(), x_showuser();
int Conditions(), x_cloak(), online_users_show(), x_info(), x_fillform(), x_vote();
int x_results(), ent_bnet(), a_edits(), x_edits();
int shownotepad(), x_userdefine();
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

int online_users_show_override(), t_list(), t_monitor();
int t_query(), t_talk(), t_pager(), t_friend(), t_reject(), x_cloak();
int ent_chat();
int AddPCorpus(); // deardragon 个人文集 
int sendgoodwish();

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
static int exec_mbem(const char *s);

typedef struct {
	char *name;
	void *fptr;
	int type;
} smenu_t;

smenu_t currcmd;

static void *sysconf_funcptr(const char *func_name, int *type)
{
	static const smenu_t cmdlist[] = {
		{ "domenu", domenu, 0 },
		{ "EGroups", board_read_group, 0 },
		{ "BoardsAll", board_read_all, 0 },
		{ "BoardsGood", goodbrd_show, 0 },
		{ "BoardsNew", board_read_new, 0 },
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
		{ "ShowFriends", online_users_show_override, 0 },
		{ "ShowLogins", online_users_show, 0 },
		{ "QueryUser", t_query, 0 },
		{ "Talk", t_talk, 0 },
		{ "SetPager", t_pager, 0 },
		{ "SetCloak", x_cloak, 0 },
		{ "SendMsg", s_msg, 0 },
		{ "ShowMsg", msg_more, 0 },
		{ "SetFriends", t_friend, 0 },
		{ "SetRejects", t_reject, 0 },
		{ "RFriendWall", friend_wall,	0 },
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
		{ "RunMBEM", exec_mbem, 0 },
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

	int n = 0;
	char *str;

	while ((str = cmdlist[n].name) != NULL) {
		if (strcmp(func_name, str) == 0) {
			*type = cmdlist[n].type;
			return (cmdlist[n].fptr);
		}
		n++;
	}
	*type = -1;
	return NULL;
}

/**
 * Execute function in dynamic loaded modules.
 * @param str function location, format: \@mod:[file]\#[function].
 * @return 0.
 */
static int exec_mbem(const char *str)
{
	char buf[128];
	strlcpy(buf, str, sizeof(buf));

	char *ptr = strstr(buf, "@mod:");
	if (ptr) {
		ptr = strstr(str + 5, "#");
		if (ptr) {
			*ptr = '\0';
			++ptr;
		}

		void *hdll = dlopen(str + 5, RTLD_LAZY);
		if (hdll) {
			int (*func)() = dlsym(hdll, ptr ? ptr : "mod_main");
			if (func)
				func();
			dlclose(hdll);
		}
	}
	return 0;
}

static void decodestr(const char *str)
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

static int draw_menu(menuitem_t *pm)
{
	const char *str;

	clear();
	int line = 3;
	int col = 0;
	int num = 0;

	while (1) {
		switch (pm->level) {
			case -1:
				return num;
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
					if (pm->line > 0) {
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
	int ch, i;

	if (sys_conf.items <= 0)
		return -1;

	menuitem_t *pm = sys_conf.item + sysconf_eval(menu_name, &sys_conf);

	int size = draw_menu(pm);

	int now = 0;

	// Jump to mail menu if user have unread mail.
	if (strcmp(menu_name, "TOPMENU") == 0 && chkmail()) {
		for (i = 0; i < size; i++)
		if (pm[i].line> 0 && pm[i].name[0] == 'M')
		now = i;
	}

	modify_user_mode(MMENU);

	// TODO: deprecate
	R_monitor();

	while (1) {
		printacbar();

		while (pm[now].level < 0 || !HAS_PERM(pm[now].level)) {
			now++;
			if (now >= size)
				now = 0;
		}

		move(pm[now].line, pm[now].col);
		prints(">");

		ch = egetch();

		move(pm[now].line, pm[now].col);
		prints(" ");

		switch (ch) {
			case EOF:
				// TODO: deprecate
				if (!refscreen) {
					abort_bbs(0);
				}
				draw_menu(pm);
				modify_user_mode(MMENU);
				R_monitor();
				break;
			case KEY_RIGHT:
				for (i = 0; i < size; i++) {
					if (pm[i].line == pm[now].line && pm[i].level >= 0 &&
							pm[i].col > pm[now].col && HAS_PERM(pm[i].level))
						break;
				}
				// If there are items on the right to current item.
				if (i < size) {
					now = i;
					break;
				}
				// fall through.
			case '\n':
			case '\r':
				if (strcmp(pm[now].arg, "..") == 0)
					return 0;
				if (pm[now].func) {
					int type;
					int (*func)() = sysconf_funcptr(pm[now].name, &type);

					if (type == 1)
						exec_mbem(pm[now].func);
					else
						(*func)(pm[now].arg);

					if (func == Select)
						now++;

					draw_menu(pm);
					modify_user_mode(MMENU);
					R_monitor();
				}
				break;
			case KEY_LEFT:
				for (i = 0; i < size; i++) {
					if (pm[i].line == pm[now].line && pm[i].level >= 0 &&
							pm[i].col < pm[now].col && HAS_PERM(pm[i].level))
						break;
					if (strcmp(pm[i].func, "LeaveBBS") == 0)
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
					if (now > 0)
						now--;
					else
						now = size - 1;
				}
				break;
			case KEY_PGUP:
				now = 0;
				break;
			case KEY_PGDN:
				now = size - 1;
				while (pm[now].level < 0 || !HAS_PERM(pm[now].level))
					now--;
				break;
			case '~':
				if (!(HAS_PERM(PERM_ESYSFILE)))
					break;
				sysconf_load(true);
				report("reload sysconf.img", currentuser.userid);
				pm = sys_conf.item + sysconf_eval(menu_name, &sys_conf);
				ActiveBoard_Init();
				size = draw_menu(pm);
				now = 0;
				break;
			case '!':
				if (strcmp("TOPMENU", menu_name) == 0)
					break;
				else
					return 0;
			default:
				if (ch >= 'a' && ch <= 'z')
					ch = ch - 'a' + 'A';
				for (i = 0; i < size; i++) {
					if (pm[i].line> 0 && ch == pm[i].name[0]
							&& HAS_PERM(pm[i].level)) {
						now = i;
						break;
				}
			}
		}
	}
}
