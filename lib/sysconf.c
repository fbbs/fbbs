// For menu config.

#include "bbs.h"

enum {
	SC_BUFSIZE = 20480,
	SC_KEYSIZE = 256,
	SC_CMDSIZE = 256
};

sysconf_t sys_conf;

/**
 * Concatenates string to the sysconf buffer.
 * @param str The string.
 * @param conf The sysconf data structure.
 */
static void *sysconf_addstr(const char *str, sysconf_t *conf)
{
	char *buf = conf->buf + conf->len;
	conf->len += strlcpy(buf, str, SC_BUFSIZE - conf->len) + 1;
	if (conf->len > SC_BUFSIZE)
		conf->len = SC_BUFSIZE;
	return buf;
}

/**
 * Search sysconf for 'key'.
 * @param key The key.
 * @return The correspoding string if found, NULL otherwise.
 */
char *sysconf_str(const char *key)
{
	int n;
	for (n = 0; n < sys_conf.keys; n++)
		if (strcmp(key, sys_conf.var[n].key) == 0)
			return (sys_conf.var[n].str);
	return NULL;
}

/**
 * Searches sysconf for key.
 * @param key The key.
 * @param conf The sysconf data structure.
 * @return The corresponding value if found. Otherwise, key is converted to
 *         integer and returned.
 */
int sysconf_eval(const char *key, sysconf_t *conf)
{
	int n;
	for (n = 0; n < conf->keys; n++)
		if (strcmp(key, conf->var[n].key) == 0)
			return (conf->var[n].val);
	return (strtol(key, NULL, 0));
}

/**
 * Add 'str' 'key' to sysconf.
 * @param key The key.
 * @param str The string.
 * @param val The value.
 * @param conf The sysconf data structure.
 */
static void sysconf_addkey(const char *key, char *str, int val, sysconf_t *conf)
{
	if (conf->keys < SC_KEYSIZE) {
		if (str == NULL)
			str = conf->buf;
		else
			str = sysconf_addstr(str, conf);
		conf->var[conf->keys].key = sysconf_addstr(key, conf);
		conf->var[conf->keys].str = str;
		conf->var[conf->keys].val = val;
		conf->keys++;
	}
}

/**
 * Read file and add menu items. Each line stands for an item.
 * Format: cmd line col level name desc.
 * Line started with '#' are comments.
 * Commands start with '\@' are ordinary cmds.
 * Commands start with '!' represent a link to another cmd group.
 * Otherwise, 'cmd's are properties of the cmd group.
 * 'line', 'col' specifies the location to display the item.
 * Users above 'level' can see the item.
 * The first letter of 'name' is the shortcut to the menu item.
 * 'desc' is the description of item shown on screen.
 * @param fp The file.
 * @param key The key.
 * @param conf The sysconf data structure.
 */
static void sysconf_addmenu(FILE *fp, const char *key, sysconf_t *conf)
{
	struct smenuitem *pm;
	char buf[LINE_BUFSIZE];
	char *cmd, *arg[5], *ptr;
	int n;

	sysconf_addkey(key, "menu", conf->items, conf);
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
		pm = conf->item + (conf->items++);
		pm->line = sysconf_eval(arg[0], conf);
		pm->col = sysconf_eval(arg[1], conf);
		if (*cmd == '@') {
			pm->level = sysconf_eval(arg[2], conf);
			pm->name = sysconf_addstr(arg[3], conf);
			pm->desc = sysconf_addstr(arg[4], conf);
			pm->fptr = sysconf_addstr(cmd + 1, conf);
			pm->arg = pm->name;
		} else if (*cmd == '!') {
			pm->level = sysconf_eval(arg[2], conf);
			pm->name = sysconf_addstr(arg[3], conf);
			pm->desc = sysconf_addstr(arg[4], conf);
			pm->fptr = sysconf_addstr("domenu", conf);
			pm->arg = sysconf_addstr(cmd + 1, conf);
		} else {
			pm->level = -2;
			pm->name = sysconf_addstr(cmd, conf);
			pm->desc = sysconf_addstr(arg[2], conf);
			pm->fptr = (void *) conf->buf;
			pm->arg = conf->buf;
		}
	}
	pm = conf->item + (conf->items++);
	pm->name = pm->desc = pm->arg = conf->buf;
	pm->fptr = (void *) conf->buf;
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

/**
 * Read file and add menu blocks (background).
 * @param fp The file.
 * @param key The key.
 * @param conf The sysconf data structure.
 */
static void sysconf_addblock(FILE *fp, const char *key, sysconf_t *conf)
{
	char buf[LINE_BUFSIZE];
	if (conf->keys < SC_KEYSIZE) {
		conf->var[conf->keys].key = sysconf_addstr(key, conf);
		conf->var[conf->keys].str = conf->buf + conf->len;
		conf->var[conf->keys].val = -1;
		conf->keys++;
		while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
			encodestr(buf);
			conf->len += strlcpy(conf->buf + conf->len, buf,
					SC_BUFSIZE - conf->len);
		}
		conf->len++;
	} else {
		while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
		}
	}
}

/**
 * Parse sysconfig file 'fname'.
 * '\#include' has the same effect of that in C file.
 * Lines starting with '\%' splits the file into groups.
 * '\%menu' indicates a group of menu items, otherwise menu background.
 */
static void sysconf_parse(const char *fname, sysconf_t *conf)
{
	char buf[LINE_BUFSIZE], tmp[LINE_BUFSIZE], *ptr, *key, *str;
	int val;

	FILE *fp = fopen(fname, "r");
	if (fp == NULL)
		return;

	sysconf_addstr("(null ptr)", conf);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (*ptr == '%') {
			strtok(ptr, " \t\n");
			if (strcmp(ptr, "%menu") == 0) {
				str = strtok(NULL, " \t\n");
				if (str != NULL)
					sysconf_addmenu(fp, str, conf);
			} else {
				sysconf_addblock(fp, ptr + 1, conf);
			}
		} else if (*ptr == '#') {
			key = strtok(ptr, " \t\"\n");
			str = strtok(NULL, " \t\"\n");
			if (key != NULL && str != NULL &&
					strcmp(key, "#include") == 0) {
				sysconf_parse(str, conf);
			}
		} else if (*ptr != '\n') {
			key = strtok(ptr, "=#\n");
			str = strtok(NULL, "#\n");
			if (key != NULL && str != NULL) {
				strtok(key, " \t");
				while (*str == ' ' || *str == '\t')
					str++;
				if (*str == '"') {
					str++;
					strtok(str, "\"");
					val = atoi(str);
					sysconf_addkey(key, str, val, conf);
				} else {
					val = 0;
					strlcpy(tmp, str, sizeof(tmp));
					ptr = strtok(tmp, ", \t");
					while (ptr != NULL) {
						val |= sysconf_eval(ptr, conf);
						ptr = strtok(NULL, ", \t");
					}
					sysconf_addkey(key, NULL, val, conf);
				}
			} else {
				report(ptr, "");
			}
		}
	}
	fclose(fp);
}

/**
 * Parse sysconfig file.
 * @param configfile The configuration file.
 * @param imgfile The file to hold the result.
 */
void sysconf_build(const char *configfile, const char *imgfile)
{
	sysconf_t conf;
	conf.item = malloc(SC_CMDSIZE * sizeof(*conf.item));
	conf.var = malloc(SC_KEYSIZE * sizeof(*conf.var));
	conf.buf = malloc(SC_BUFSIZE);
	if (!conf.buf || !conf.var || !conf.item) {
		if (conf.item)
			free(conf.item);
		if (conf.var)
			free(conf.var);
		if (conf.buf)
			free(conf.buf);
		return;
	}
	conf.len = conf.items = conf.keys = 0;

	sysconf_parse(configfile, &conf);

	struct sysheader shead;
	FILE *fp = fopen(imgfile, "w");
	if (fp) {
		shead.buf = conf.buf;
		shead.menu = conf.items;
		shead.key = conf.keys;
		shead.len = conf.len;
		fwrite(&shead, sizeof(shead), 1, fp);
		fwrite(conf.item, sizeof(*conf.item), conf.items, fp);
		fwrite(conf.var, sizeof(*conf.var), conf.keys, fp);
		fwrite(conf.buf, conf.len, 1, fp);
		fclose(fp);
	}

	free(conf.item);
	free(conf.var);
	free(conf.buf);
}
