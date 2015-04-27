#include <inttypes.h>
#include "fbbs/list.h"
#include "fbbs/json.h"
#include "fbbs/web.h"

struct json_value_t {
	json_value_e type;
	const char *key;
	union {
		const char *str;
		int integer;
		int64_t bigint;
		bool boolean;
		json_object_t *object;
	} value;
	STAILQ_FIELD(json_value_t) next;
};

STAILQ_HEAD(json_value_list_t, json_value_t);

struct json_object_t {
	struct json_value_list_t values;
};

json_object_t *json_object_new(void)
{
	json_object_t *object = web_palloc(sizeof(*object));
	if (object) {
		STAILQ_INIT_HEAD(&object->values);
	}
	return object;
}

json_array_t *json_array_new(void)
{
	return json_object_new();
}

static json_value_t *json_object_add(json_object_t *object,
		json_value_t *value)
{
	STAILQ_INSERT_TAIL(&object->values, value, next);
	return value;
}

#define JSON_OBJECT_HELPER(_type, _field, _key, _value)  \
	do { \
		json_value_t *val = web_palloc(sizeof(*val)); \
		if (val) { \
			val->key = _key; \
			val->type = _type; \
			val->value._field = _value; \
		} \
		return json_object_add(object, val); \
	} while (0)

json_value_t *json_object_null(json_object_t *object, const char *key)
{
	json_value_t *value = web_palloc(sizeof(*value));
	if (value)
		value->type = JSON_NULL;
	return json_object_add(object, value);
}

json_value_t *json_object_string(json_object_t *object, const char *key,
		const char *value)
{
	if (value)
		JSON_OBJECT_HELPER(JSON_STRING, str, key, web_pstrdup(value));
	else
		return json_object_null(object, key);
}

json_value_t *json_object_integer(json_object_t *object, const char *key,
		int value)
{
	JSON_OBJECT_HELPER(JSON_INTEGER, integer, key, value);
}

json_value_t *json_object_bigint(json_object_t *object, const char *key,
		int64_t value)
{
	JSON_OBJECT_HELPER(JSON_BIGINT, bigint, key, value);
}

json_value_t *json_object_bool(json_object_t *object, const char *key,
		bool value)
{
	JSON_OBJECT_HELPER(JSON_BOOL, boolean, key, value);
}

json_value_t *json_object_append(json_object_t *object, const char *key,
		json_object_t *value, json_value_e type)
{
	JSON_OBJECT_HELPER(type, object, key, value);
}

json_value_t *json_array_null(json_array_t *array)
{
	return json_object_null(array, NULL);
}

json_value_t *json_array_string(json_array_t *array, const char *value)
{
	return json_object_string(array, NULL, value);
}

json_value_t *json_array_integer(json_array_t *array, int value)
{
	return json_object_integer(array, NULL, value);
}

json_value_t *json_array_bigint(json_array_t *array, int64_t value)
{
	return json_object_bigint(array, NULL, value);
}

json_value_t *json_array_bool(json_array_t *array, bool value)
{
	return json_object_bool(array, NULL, value);
}

json_value_t *json_array_append(json_array_t *array, json_object_t *value,
		json_value_e type)
{
	return json_object_append(array, NULL, value, type);
}

static void json_print_string(const char *s)
{
	unsigned char *p = (unsigned char *) s;
	if (p) {
		putchar('\"');
		unsigned int c;
		while ((c = *p)) {
			if (c == '"') {
				printf("\\\"");
			} else if (c == '\\') {
				printf("\\\\");
			} else if (c == '\n') {
				printf("\\n");
			} else if (c <= 0x1f) {
				printf("\\u00%02x", c);
			} else {
				putchar(c);
			}
			++p;
		}
		putchar('\"');
	} else {
		printf("null");
	}
}

static void json_print_value(const json_value_t *value)
{
	if (value->key) {
		json_print_string(value->key);
		putchar(':');
	}

	switch (value->type) {
		case JSON_INTEGER:
			printf("%d", value->value.integer);
			break;
		case JSON_BIGINT:
			printf("\"%"PRId64"\"", value->value.bigint);
			break;
		case JSON_BOOL:
			if (value->value.boolean)
				printf("true");
			else
				printf("false");
			break;
		case JSON_NULL:
			printf("null");
			break;
		case JSON_STRING:
			json_print_string(value->value.str);
			break;
		case JSON_OBJECT: case JSON_ARRAY:
			json_dump(value->value.object, value->type);
			break;
		default:
			break;
	}
}

void json_dump(const json_object_t *object, json_value_e type)
{
	putchar(type == JSON_OBJECT ? '{' : '[');
	STAILQ_FOREACH(json_value_t, value, &object->values, next) {
		json_print_value(value);
		if (STAILQ_NEXT(value, next))
			putchar(',');
	}
	putchar(type == JSON_OBJECT ? '}' : ']');
}
