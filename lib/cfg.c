#include <stdio.h>
#include <string.h>
#include "fbbs/cfg.h"
#include "fbbs/string.h"
#include "fbbs/pool.h"

enum {
	CONFIG_MAX_KEYS = 64,
	CONFIG_MAX_LINE_LEN = 256,
};

struct config_t {
	pool_t *pool;
	const char **keys;
	const char **vals;
	int pairs;
};

static config_t *_config_init(pool_t *p)
{
	config_t *c = pool_alloc(p, sizeof(*c));
	if (!c)
		return NULL;

	c->keys = pool_alloc(p, sizeof(*c->keys) * CONFIG_MAX_KEYS);
	c->vals = pool_alloc(p, sizeof(*c->vals) * CONFIG_MAX_KEYS);
	if (!c->keys || !c->vals)
		return NULL;

	c->pool = p;
	c->pairs = 0;

	return c;
}

static const char *pool_strdup(pool_t *p, const char *str)
{
	if (!str)
		return NULL;

	size_t len = strlen(str);
	char *s = pool_alloc(p, len + 1);
	if (!s)
		return NULL;

	memcpy(s, str, len);
	s[len] = '\0';
	return s;
}

static int _config_add(config_t *c, const char *key, const char *val)
{
	if (!key || !val || !c || c->pairs >= CONFIG_MAX_KEYS)
		return -1;

	c->keys[c->pairs] = pool_strdup(c->pool, key);
	c->vals[c->pairs] = pool_strdup(c->pool, val);
	if (!c->keys[c->pairs] || !c->vals[c->pairs])
		return -1;

	++c->pairs;
	return 0;
}

config_t *config_load(pool_t *p, const char *file)
{
	config_t *c = _config_init(p);
	if (!c)
		return NULL;

	FILE *fp = fopen(file, "r");
	if (!fp)
		return NULL;

	char buf[CONFIG_MAX_LINE_LEN];
	while (fgets(buf, sizeof(buf), fp)) {
		if (buf[0] == '#')
			continue;
		const char *key = trim(strtok(buf, "=\n"));
		const char *val = trim(strtok(NULL, "=\n"));
		_config_add(c, key, val);
	}

	fclose(fp);
	return c;
}

const char *config_get(const config_t *cfg, const char *key)
{
	for (int i = 0; i < cfg->pairs; ++i) {
		if (streq(cfg->keys[i], key))
			return cfg->vals[i];
	}
	return NULL;
}
