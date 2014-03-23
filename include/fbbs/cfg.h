#ifndef FB_CFG_H
#define FB_CFG_H

#define DEFAULT_CFG_FILE "/etc/fbbs/fbbs.conf"

typedef struct config_t config_t;

extern config_t *config_load(const char *file);
extern const char *config_get(const char *key);
extern int config_get_integer(const char *key, int invalid);

#endif // FB_CFG_H
