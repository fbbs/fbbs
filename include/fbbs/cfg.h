#ifndef FB_CFG_H
#define FB_CFG_H

typedef struct config_t {
	int pairs;
	int cur;
	int *keys;
	int *vals;
	char *buf;
} config_t;

extern int config_init(config_t *cfg);
extern void config_destroy(config_t *cfg);
extern int config_load(config_t *cfg, const char *file);
extern const char *config_get(const config_t *cfg, const char *key);

#endif // FB_CFG_H
