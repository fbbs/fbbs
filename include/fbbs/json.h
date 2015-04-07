#ifndef FB_JSON_H
#define FB_JSON_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	JSON_STRING,
	JSON_INTEGER,
	JSON_BIGINT,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BOOL,
	JSON_NULL,
} json_value_e;

typedef struct json_value_t json_value_t;
typedef struct json_object_t json_object_t;
typedef json_object_t json_array_t;
typedef json_object_t json_document_t;

extern json_object_t *json_object_new(void);
extern json_array_t *json_array_new(void);

extern json_value_t *json_object_null(json_object_t *object, const char *key);
extern json_value_t *json_object_string(json_object_t *object, const char *key, const char *value);
extern json_value_t *json_object_integer(json_object_t *object, const char *key, int value);
extern json_value_t *json_object_bigint(json_object_t *object, const char *key, int64_t value);
extern json_value_t *json_object_bool(json_object_t *object, const char *key, bool value);
extern json_value_t *json_object_append(json_object_t *object, const char *key, json_object_t *value);

extern json_value_t *json_array_null(json_array_t *array);
extern json_value_t *json_array_string(json_array_t *array, const char *value);
extern json_value_t *json_array_integer(json_array_t *array, int value);
extern json_value_t *json_array_bigint(json_array_t *array, int64_t value);
extern json_value_t *json_array_bool(json_array_t *array, bool value);
extern json_value_t *json_array_append(json_array_t *array, json_object_t *value);

extern void json_dump(const json_object_t *object, json_value_e type);
#endif // FB_JSON_H
