// For menu config.

#include "bbs.h"

#define SC_BUFSIZE 20480
#define SC_KEYSIZE 256
#define SC_CMDSIZE 256

static char *sysconf_buf;
int sysconf_menu;
static int sysconf_key, sysconf_len;
struct smenuitem *menuitem;
struct sdefine {
	char *key, *str;
	int val;
};
static struct sdefine *sysvar;

// Concatenates 'str' to the end of 'sysconf_buf'
// and increments 'sysconf_len' accordingly.
static void *sysconf_addstr(const char *str)
{
	int len = sysconf_len;
	char *buf;

	buf = sysconf_buf + len;
	strcpy(buf, str);
	sysconf_len = len + strlen(str) + 1;
	return buf;
}

// Searches structs 'sysvar' for 'key'.
// Returns sysvar.str if found, NULL otherwise.
char *sysconf_str(const char *key)
{
	int n;

	for (n = 0; n < sysconf_key; n++)
		if (strcmp(key, sysvar[n].key) == 0)
			return (sysvar[n].str);
	return NULL;
}

// Searches structs 'sysvar' for 'key'.
// Returns 'sysvar.key' if found,
// the integer 'key' is representing otherwise.
int sysconf_eval(const char *key)
{
	int n;

	for (n = 0; n < sysconf_key; n++)
		if (strcmp(key, sysvar[n].key) == 0)
			return (sysvar[n].val);
	if (*key < '0' || *key> '9') {
		char buf[80];
		sprintf(buf, "sysconf: unknown key: %s.", key);
		report(buf, "");
	}
	return (strtol(key, NULL, 0));
}

// Adds 'str' 'key' to 'sysconf_buf'.
// Stores them (together with 'val') in sysvar.
static void sysconf_addkey(const char *key, char *str, int val)
{
	int num;

	if (sysconf_key < SC_KEYSIZE) {
		if (str == NULL)
			str = sysconf_buf;
		else
			str = sysconf_addstr(str);
		num = sysconf_key++;
		sysvar[num].key = sysconf_addstr(key);
		sysvar[num].str = str;
		sysvar[num].val = val;
	}
}

// Reads 'fp' and add menu items.(Each line stands for an item.)
// Format: cmd line col level name desc
// Line started with '#' are comments.
// 'cmd's start with '@' are ordinary cmds.
// 'cmd's start with '!' represent a link to another cmd group.
// Otherwise, 'cmd's are properties of the cmd group.
// 'line', 'col' specifies the location to display the item.
// Users above 'level' can see the item.
// The first letter of 'name' is the shortcut to the menu item.
// 'desc' is the description of item shown on screen.
static void sysconf_addmenu(FILE *fp, const char *key)
{
	struct smenuitem *pm;
	char buf[256];
	char *cmd, *arg[5], *ptr;
	int n;

	sysconf_addkey(key, "menu", sysconf_menu);
	while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
		cmd = strtok(buf, " \t\n");
		if (cmd == NULL || *cmd == '#')
			continue;
		arg[0] = arg[1] = arg[2] = arg[3] = arg[4] = "";
		n = 0;
		for (n = 0; n < 5; n++) {
			if ((ptr = strtok(NULL, ",\n")) == NULL)
				break;
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;
			if (*ptr == '"') {
				arg[n] = ++ptr;
				while (*ptr != '"' && *ptr != '\0')
					ptr++;
				*ptr = '\0';
			} else {
				arg[n] = ptr;
				while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
					ptr++;
				*ptr = '\0';
			}
		}
		pm = &menuitem[sysconf_menu++];
		pm->line = sysconf_eval(arg[0]);
		pm->col = sysconf_eval(arg[1]);
		if (*cmd == '@') {
			pm->level = sysconf_eval(arg[2]);
			pm->name = sysconf_addstr(arg[3]);
			pm->desc = sysconf_addstr(arg[4]);
			pm->fptr = sysconf_addstr(cmd + 1);
			pm->arg = pm->name;
		} else if (*cmd == '!') {
			pm->level = sysconf_eval(arg[2]);
			pm->name = sysconf_addstr(arg[3]);
			pm->desc = sysconf_addstr(arg[4]);
			pm->fptr = sysconf_addstr("domenu");
			pm->arg = sysconf_addstr(cmd + 1);
		} else {
			pm->level = -2;
			pm->name = sysconf_addstr(cmd);
			pm->desc = sysconf_addstr(arg[2]);
			pm->fptr = (void *) sysconf_buf;
			pm->arg = sysconf_buf;
		}
	}
	pm = &menuitem[sysconf_menu++];
	pm->name = pm->desc = pm->arg = sysconf_buf;
	pm->fptr = (void *) sysconf_buf;
	pm->level = -1;
}

static void encodestr(char *str)
{
	register char ch, *buf;
	int n;

	buf = str;
	while ((ch = *str++) != '\0') {
		if (*str == ch && str[1] == ch && str[2] == ch) {
			n = 4;
			str += 3;
			while (*str == ch && n < 100) {
				str++;
				n++;
			}
			*buf++ = '\01';
			*buf++ = ch;
			*buf++ = n;
		} else
		*buf++ = ch;
	}
	*buf = '\0';
}

// Read 'fp' and add menu blocks(background).
static void sysconf_addblock(FILE *fp, const char *key)
{
	char buf[256];
	int num;

	if (sysconf_key < SC_KEYSIZE) {
		num = sysconf_key++;
		sysvar[num].key = sysconf_addstr(key);
		sysvar[num].str = sysconf_buf + sysconf_len;
		sysvar[num].val = -1;
		while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
			encodestr(buf);
			strcpy(sysconf_buf + sysconf_len, buf);
			sysconf_len += strlen(buf);
		}
		sysconf_len++;
	} else {
		while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
		}
	}
}

// Parse sysconfig file 'fname'.
// '#include' has the same effect of that in C file.
// Lines starting with '%' splits the file into groups.
// '%menu' indicates a group of menu items, otherwise menu background.
static void parse_sysconf(const char *fname)
{
	FILE *fp;
	char buf[256];
	char tmp[256], *ptr;
	char *key, *str;
	int val;

	if ((fp = fopen(fname, "r")) == NULL) {
		return;
	}
	sysconf_addstr("(null ptr)");
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (*ptr == '%') {
			strtok(ptr, " \t\n");
			if (strcmp(ptr, "%menu") == 0) {
				str = strtok(NULL, " \t\n");
				if (str != NULL)
					sysconf_addmenu(fp, str);
			} else {
				sysconf_addblock(fp, ptr + 1);
			}
		} else if (*ptr == '#') {
			key = strtok(ptr, " \t\"\n");
			str = strtok(NULL, " \t\"\n");
			if (key != NULL && str != NULL &&
					strcmp(key, "#include") == 0) {
				parse_sysconf(str);
			}
		} else if (*ptr != '\n') {
			key = strtok(ptr, "=#\n");
			str = strtok(NULL, "#\n");
			if (key != NULL & str != NULL) {
				strtok(key, " \t");
				while (*str == ' ' || *str == '\t')
					str++;
				if (*str == '"') {
					str++;
					strtok(str, "\"");
					val = atoi(str);
					sysconf_addkey(key, str, val);
				} else {
					val = 0;
					strcpy(tmp, str);
					ptr = strtok(tmp, ", \t");
					while (ptr != NULL) {
						val |= sysconf_eval(ptr);
						ptr = strtok(NULL, ", \t");
					}
					sysconf_addkey(key, NULL, val);
				}
			} else {
				report(ptr, "");
			}
		}
	}
	fclose(fp);
}

// Parse sysconfig file 'configfile' and store the result in 'imgfile'.
void build_sysconf(const char *configfile, const char *imgfile)
{
	struct smenuitem *old_menuitem;
	struct sdefine *old_sysvar;
	char *old_buf;
	int old_menu, old_key, old_len;
	struct sysheader shead;
	int fh;

	old_menuitem = menuitem;
	old_menu = sysconf_menu;
	old_sysvar = sysvar;
	old_key = sysconf_key;
	old_buf = sysconf_buf;
	old_len = sysconf_len;
	menuitem = (void *) malloc(SC_CMDSIZE * sizeof(struct smenuitem));
	sysvar = (void *) malloc(SC_KEYSIZE * sizeof(struct sdefine));
	sysconf_buf = (void *) malloc(SC_BUFSIZE);
	sysconf_menu = 0;
	sysconf_key = 0;
	sysconf_len = 0;
	parse_sysconf(configfile);
	if ((fh = open(imgfile, O_WRONLY | O_CREAT, 0644))> 0) {
		ftruncate(fh, 0);
		shead.buf = sysconf_buf;
		shead.menu = sysconf_menu;
		shead.key = sysconf_key;
		shead.len = sysconf_len;
		write(fh, &shead, sizeof(shead));
		write(fh, menuitem, sysconf_menu * sizeof(struct smenuitem));
		write(fh, sysvar, sysconf_key * sizeof(struct sdefine));
		write(fh, sysconf_buf, sysconf_len);
		close(fh);
	}
	free(menuitem);
	free(sysvar);
	free(sysconf_buf);
	menuitem = old_menuitem;
	sysconf_menu = old_menu;
	sysvar = old_sysvar;
	sysconf_key = old_key;
	sysconf_buf = old_buf;
	sysconf_len = old_len;
}

// Loads sysconf image from 'imgfile'.
void load_sysconf_image(const char *imgfile)
{
	extern void *sysconf_funcptr(const char *func_name, int *type);
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

