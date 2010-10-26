#include <stdio.h>
#include <string.h>
#include "fbbs/cfg.h"
#include "fbbs/string.h"

enum {
	CONFIG_MAX_KEYS = 64,
	CONFIG_MAX_LINE_LEN = 256,
	CONFIG_BUFFER_SIZE = 3584,
};

int config_init(config_t *cfg)
{
	void *ptr = malloc((sizeof(*cfg->keys) + sizeof(*cfg->vals))
			* CONFIG_MAX_KEYS + CONFIG_BUFFER_SIZE);
	if (!ptr)
		return -1;

	cfg->keys = ptr;
	cfg->vals = cfg->keys + CONFIG_MAX_KEYS;
	cfg->buf = (char *)(cfg->vals + CONFIG_MAX_KEYS);
	cfg->pairs = 0;
	cfg->cur = 0;
	return 0;
}

void config_destroy(config_t *cfg)
{
	if (cfg->keys)
		free(cfg->keys);
}

static int _config_add(config_t *cfg, const char *key, const char *val)
{
	if (!key || !val)
		return -1;
	int klen = strlen(key);
	int vlen = strlen(val);
	if (cfg->cur + klen + vlen + 2 > CONFIG_BUFFER_SIZE)
		return -1;

	cfg->keys[cfg->pairs] = cfg->cur;
	strcpy(cfg->buf + cfg->cur, key);
	cfg->cur += klen + 1;

	cfg->vals[cfg->pairs] = cfg->cur;
	strcpy(cfg->buf + cfg->cur, val);
	cfg->cur += vlen + 1;

	++cfg->pairs;
	return 0;
}

int config_load(config_t *cfg, const char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
		return -1;

	char buf[CONFIG_MAX_LINE_LEN];
	while (fgets(buf, sizeof(buf), fp)) {
		if (buf[0] == '#')
			continue;
		const char *key = trim(strtok(buf, "=\n"));
		const char *val = trim(strtok(NULL, "=\n"));
		_config_add(cfg, key, val);
	}

	fclose(fp);
	return 0;
}

const char *config_get(const config_t *cfg, const char *key)
{
	for (int i = 0; i < cfg->pairs; ++i) {
		if (streq(cfg->buf + cfg->keys[i], key))
			return cfg->buf + cfg->vals[i];
	}
	return NULL;
}
