#include <dlfcn.h>
#include "bbs.h"
#include "sysconf.h"

#ifndef DLM
#undef  ALLOWGAME
#endif

#ifdef FDQUAN
#define ALLOWGAME
#endif

int domenu(const char *menu_name);
int Announce(), Personal(), board_read_all(), board_read_group(), Info(), Goodbye();
int board_read_new(), goodbrd_show(), board_read(), board_select(), Welcome();
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

typedef int (*telnet_handler_t)();

typedef struct {
	const char *name;
	telnet_handler_t fptr;
} cmd_list_t;

#ifdef DLM
typedef struct {
	const char *name;
	const char *fptr;
} dlm_list_t;
#endif

static telnet_handler_t sysconf_funcptr(const char *name)
{
	static const cmd_list_t cmdlist[] = {
		{ "domenu", domenu },
		{ "EGroups", board_read_group },
		{ "BoardsAll", board_read_all },
		{ "BoardsGood", goodbrd_show },
		{ "BoardsNew", board_read_new },
		{ "LeaveBBS", Goodbye },
		{ "Announce", Announce },
		{ "Personal", Personal },
		{ "SelectBoard", board_select },
		{ "ReadBoard", board_read },
		{ "SetAlarm", setcalltime },
		{ "MailAll", mailall },
		{ "LockScreen", x_lockscreen },
		{ "ShowUser", x_showuser },
		{ "OffLine", offline },
		{ "GiveUpBBS", giveUpBBS },
		{ "ReadNewMail", m_new },
		{ "ReadMail", m_read },
		{ "SendMail", m_send },
		{ "GroupSend", g_send },
		{ "OverrideSend", ov_send },
#ifdef INTERNET_EMAIL
		{ "SendNetMail", m_internet },
#endif
		{ "UserDefine", x_userdefine },
		{ "ShowFriends", online_users_show_override },
		{ "ShowLogins", online_users_show },
		{ "QueryUser", t_query },
		{ "Talk", t_talk },
		{ "SetPager", t_pager },
		{ "SetCloak", x_cloak },
		{ "SendMsg", s_msg },
		{ "ShowMsg", msg_more },
		{ "SetFriends", t_friend },
		{ "SetRejects", t_reject },
		{ "RFriendWall", friend_wall },
		{ "EnterChat", ent_chat },
		{ "ListLogins", t_list },
		{ "Monitor", t_monitor },
		{ "FillForm", x_fillform },
		{ "Information", x_info },
		{ "EditUFiles", x_edits },
		{ "ShowLicense", Conditions },
		{ "ShowVersion", Info },
		{ "Notepad", shownotepad },
		{ "Vote", x_vote },
		{ "VoteResult", x_results },
		{ "ExecBBSNet", ent_bnet },
		{ "ExecBBSNet2", ent_bnet2 },
		{ "ShowWelcome", Welcome },
		{ "AddPCorpus", AddPCorpus },
		{ "GoodWish", sendgoodwish },
#ifdef ALLOWSWITCHCODE
		{ "SwitchCode", switch_code },
#endif
#ifdef ALLOWGAME
		{ "WinMine", ent_winmine },
#endif
		{ "RunMBEM", exec_mbem },
		{ "Kick", kick_user },
		{ "OpenVote", m_vote },
		{ "SearchAll", r_searchall },
#ifndef DLM
		{ "Setsyspass", setsystempasswd },
		{ "Register", m_register },
		{ "ShowRegister", show_register },
		{ "Info", m_info },
		{ "Level", x_level },
		{ "OrdainBM", m_ordainBM },
		{ "RetireBM", m_retireBM },
		{ "NewChangeLevel", x_new_denylevel },
		{ "DelUser", d_user },
		{ "NewBoard", m_newbrd },
		{ "ChangeBrd", m_editbrd },
		{ "BoardDel", d_board },
		{ "SysFiles", a_edits },
		{ "Wall", wall },
#endif
		{ NULL, NULL }
	};

	const cmd_list_t *cmd = cmdlist;
	while (cmd->name != NULL) {
		if (strcmp(name, cmd->name) == 0)
			return cmd->fptr;
		++cmd;
	}
	return NULL;
}

#ifdef DLM
static const char *sysconf_funcstr(const char *name)
{
	static const dlm_list_t dlmlist[] = {
#ifdef ALLOWGAME
		{ "Gagb", "@mod:so/game.so#gagb" },
		{ "BlackJack", "@mod:so/game.so#BlackJack" },
		{ "X_dice", "@mod:so/game.so#x_dice" },
		{ "P_gp", "@mod:so/game.so#p_gp" },
		{ "IP_nine", "@mod:so/game.so#p_nine" },
		{ "OBingo", "@mod:so/game.so#bingo" },
		{ "Chicken", "@mod:so/game.so#chicken_main" },
		{ "Mary", "@mod:so/game.so#mary_m" },
		{ "Borrow", "@mod:so/game.so#borrow" },
		{ "Payoff", "@mod:so/game.so#payoff" },
		{ "Impawn", "@mod:so/game.so#popshop" },
		{ "Doshopping", "@mod:so/game.so#doshopping" },
		{ "Lending", "@mod:so/game.so#lending" },
		{ "StarChicken", "@mod:so/pip.so#mod_default" },
#endif // ALLOWGAME
		{ "Setsyspass", "@mod:so/admintool.so#setsystempasswd" },
		{ "Register", "@mod:so/admintool.so#m_register" },
		{ "ShowRegister", "@mod:so/admintool.so#show_register" },
		{ "Info", "@mod:so/admintool.so#m_info" },
		{ "Level", "@mod:so/admintool.so#x_level" },
		{ "OrdainBM", "@mod:so/admintool.so#m_ordainBM" },
		{ "RetireBM", "@mod:so/admintool.so#m_retireBM" },
		{ "ChangeLevel", "@mod:so/admintool.so#x_denylevel" },
		{ "NewChangeLevel", "@mod:so/admintool.so#x_new_denylevel" },
		{ "DelUser", "@mod:so/admintool.so#d_user" },
		{ "NewBoard", "@mod:so/admintool.so#m_newbrd" },
		{ "ChangeBrd", "@mod:so/admintool.so#m_editbrd" },
		{ "BoardDel", "@mod:so/admintool.so#d_board" },
		{ "SysFiles", "@mod:so/admintool.so#a_edits" },
		{ "Wall", "@mod:so/admintool.so#wall" },
		{ NULL, NULL }
	};

	const dlm_list_t *dlm = dlmlist;
	while (dlm->name != NULL) {
		if (strcmp(name, dlm->name) == 0)
			return dlm->fptr;
		++dlm;
	}
	return NULL;
}
#endif // DLM

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
		ptr = strstr(buf + 5, "#");
		if (ptr) {
			*ptr = '\0';
			++ptr;
		}

		void *hdll = dlopen(buf + 5, RTLD_LAZY);
		if (hdll) {
			int (*func)();
			*(void **)(&func) = dlsym(hdll, ptr ? ptr : "mod_main");
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
					telnet_handler_t func = sysconf_funcptr(pm[now].func);
					if (func) {
						(*func)(pm[now].arg);
						if (func == board_select)
							now++;
					} else {
#ifdef DLM
						const char *ptr = sysconf_funcstr(pm[now].func);
						if (!ptr)
							break;
						else
							exec_mbem(ptr);
#else
						;
#endif // DLM
					}
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
