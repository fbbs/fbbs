#ifndef FB_CFG_H
#define FB_CFG_H

#define DEFAULT_CFG_FILE "/etc/fbbs/fbbs.conf"

typedef struct config_t config_t;

extern config_t *config_load(const char *file);
extern const char *config_get(const char *key);

#endif // FB_CFG_H
