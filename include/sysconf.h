#ifndef FB_SYSCONF_H
#define FB_SYSCONF_H

typedef struct {
	int line;    ///< Line.
	int col;     ///< Column.
	int level;   ///< Level.
	const char *name;  ///< English name.
	const char *desc;  ///< Description shown on screen.
	const char *arg;   ///< Arguments passed to function.
	const char *func;  ///< Function string.
} menuitem_t;

struct sdefine {
	char *key, *str;
	int val;
};

typedef struct {
	char *buf;           ///< The buffer.
	menuitem_t *item;    ///< Array of menu items.
	struct sdefine *var; ///< 
	int len;             ///< Length of used buffer.
	int items;           ///< Count of menu items.
	int keys;            ///< 
} sysconf_t;

extern sysconf_t sys_conf;

extern const char *sysconf_str(const char *key);
extern int sysconf_eval(const char *key, sysconf_t *conf);
extern int sysconf_load(bool rebuild);

#endif // FB_SYSCONF_H

