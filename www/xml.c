#include <inttypes.h>
#include <stdbool.h>
#include "fbbs/convert.h"
#include "fbbs/list.h"
#include "fbbs/pool.h"
#include "fbbs/web.h"
#include "fbbs/xml.h"

struct xml_attr_t {
	int type;
	const char *name;
	union {
		const char *str;
		int integer;
		int64_t bigint;
		bool boolean;
	} value;
	SLIST_FIELD(xml_attr_t) next;
};

SLIST_HEAD(xml_attr_list_t, xml_attr_t);
SLIST_HEAD(xml_node_list_t, xml_node_t);

struct xml_node_t {
	int type;
	const char *name;
	struct xml_attr_list_t attr;
	struct xml_node_list_t child;
	SLIST_FIELD(xml_node_t) next;
};

struct xml_document_t {
	int encoding;
	xml_node_t *root;
};

xml_document_t *xml_new_doc(void)
{
	xml_document_t *doc = palloc(sizeof(*doc));
	doc->root = NULL;
	doc->encoding = XML_ENCODING_UTF8;
	return doc;
}

xml_document_t *xml_set_encoding(xml_document_t *doc, int encoding)
{
	doc->encoding = encoding;
	return doc;
}

void xml_set_doc_root(xml_document_t *doc, xml_node_t *root)
{
	doc->root = root;
}

xml_node_t *xml_new_node(const char *name, int type)
{
	xml_node_t *node = palloc(sizeof(*node));
	node->name = name;
	node->type = type;
	SLIST_INIT_HEAD(&node->attr);
	SLIST_INIT_HEAD(&node->child);
	return node;
}

xml_node_t *xml_add_child(xml_node_t *parent, xml_node_t *child)
{
	SLIST_INSERT_HEAD(&parent->child, child, next);
	return child;
}

xml_node_t *xml_new_child(xml_node_t *parent, const char *name, int type)
{
	return xml_add_child(parent, xml_new_node(name, type));
}

static xml_attr_t *xml_add_attr(xml_node_t *node, xml_attr_t *attr)
{
	SLIST_INSERT_HEAD(&node->attr, attr, next);
	return attr;
}

#define XML_ATTR_HELPER(attr_type, field, attr_value)  \
	do { \
		xml_attr_t *attr = palloc(sizeof(*attr)); \
		attr->name = name; \
		attr->type = attr_type | (as_node ? XML_ATTR_AS_NODE : 0); \
		attr->value.field = attr_value; \
		return xml_add_attr(node, attr); \
	} while (0)

xml_attr_t *xml_attr_string(xml_node_t *node, const char *name,
		const char *value, bool as_node)
{
	XML_ATTR_HELPER(XML_ATTR_TYPE_STRING, str, pstrdup(value));
}

xml_attr_t *xml_attr_integer(xml_node_t *node, const char *name, int value)
{
	bool as_node = false;
	XML_ATTR_HELPER(XML_ATTR_TYPE_INTEGER, integer, value);
}

xml_attr_t *xml_attr_bigint(xml_node_t *node, const char *name, int64_t value)
{
	bool as_node = false;
	XML_ATTR_HELPER(XML_ATTR_TYPE_BIGINT, bigint, value);
}

xml_attr_t *xml_attr_boolean(xml_node_t *node, const char *name, bool value)
{
	bool as_node = false;
	XML_ATTR_HELPER(XML_ATTR_TYPE_BOOLEAN, boolean, value);
}

static inline int real_type(const xml_attr_t *attr)
{
	return attr->type & 0x7f;
}

static int xml_string_helper(const char *s, size_t size, void *arg)
{
	return fwrite((void *)s, 1, size, stdout);
}

static void _xml_attr_print(const xml_attr_t *attr, int encoding)
{
	switch (real_type(attr)) {
		case XML_ATTR_TYPE_INTEGER:
			printf("%d", attr->value.integer);
			break;
		case XML_ATTR_TYPE_BIGINT:
			printf("%"PRId64, attr->value.bigint);
			break;
		case XML_ATTR_TYPE_BOOLEAN:
			if (attr->value.boolean)
				printf("1");
			break;
		default:
			if (encoding == XML_ENCODING_UTF8) {
				printf("%s", attr->value.str);
			} else {
				convert(env_u2g, attr->value.str, CONVERT_ALL, NULL, 0,
						xml_string_helper, NULL);
			}
			break;
	}
}

static void xml_attr_print(const xml_attr_t *attr, int encoding)
{
	if (real_type(attr) != XML_ATTR_TYPE_BOOLEAN || attr->value.boolean) {
		printf(" %s='", attr->name);
		_xml_attr_print(attr, encoding);
		printf("'");
	}
}

static inline bool anonymous(const xml_node_t *node)
{
	return node->type & XML_NODE_ANONYMOUS;
}

static void _print_as_xml(const xml_node_t *node, int encoding)
{
	if (anonymous(node)) {
		SLIST_FOREACH(xml_node_t, child, &node->child, next) {
			_print_as_xml(child, encoding);
		}
		return;
	}

	printf("<%s", node->name);

	bool attr_as_node = false;
	SLIST_FOREACH(xml_attr_t, attr, &node->attr, next) {
		if (attr->type & XML_ATTR_AS_NODE) {
			attr_as_node = true;
		} else {
			xml_attr_print(attr, encoding);
		}
	}

	if (!attr_as_node && !SLIST_FIRST(&node->child)) {
		printf("/>");
	} else {
		printf(">");

		if (attr_as_node) {
			SLIST_FOREACH(xml_attr_t, attr, &node->attr, next) {
				if (attr->type & XML_ATTR_AS_NODE) {
					printf("<%s>", attr->name);
					_xml_attr_print(attr, encoding);
					printf("</%s>", attr->name);
				}
			}
		}

		SLIST_FOREACH(xml_node_t, child, &node->child, next) {
			_print_as_xml(child, encoding);
		}
		printf("</%s>", node->name);
	}
}

static void print_as_xml(const xml_document_t *doc)
{
	printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	_print_as_xml(doc->root, doc->encoding);
}

static void _json_string(const char *s)
{
	int c;
	while ((c = *s)) {
		if (c == '"') {
			printf("\\\"");
		} else if (c == '\\') {
			printf("\\\\");
		} else if (c >= 0 && c <= 0x1f) {
			printf("\\u00%02x", c);
		} else {
			putchar(c);
		}
		++s;
	}
}

static int json_string_helper(const char *s, size_t len, void *arg)
{
	_json_string(s);
	return 0;
}

static void json_string(const char *s, int encoding)
{
	if (s) {
		putchar('\"');

		if (encoding == XML_ENCODING_UTF8)
			_json_string(s);
		else
			convert(env_u2g, s, CONVERT_ALL, NULL, 0, json_string_helper, NULL);

		putchar('\"');
	} else {
		printf("null");
	}
}

static void json_print_attr(const xml_attr_t *attr, int encoding)
{
	json_string(attr->name, encoding);
	putchar(':');

	switch (real_type(attr)) {
		case XML_ATTR_TYPE_INTEGER:
			printf("%d", attr->value.integer);
			break;
		case XML_ATTR_TYPE_BIGINT:
			printf("%"PRId64, attr->value.bigint);
			break;
		case XML_ATTR_TYPE_BOOLEAN:
			if (attr->value.boolean)
				printf("true");
			else
				printf("false");
			break;
		default:
			json_string(attr->value.str, encoding);
			break;
	}
}

static void _print_as_json(const xml_node_t *node, int encoding)
{
	if (!(node->type & XML_NODE_ANONYMOUS_JSON)) {
		json_string(node->name, encoding);
		putchar(':');
	}

	bool child = SLIST_FIRST(&node->child) || SLIST_FIRST(&node->attr);
	bool array = node->type & XML_NODE_CHILD_ARRAY;

	putchar(array ? '[' : '{');
	if (child) {
		SLIST_FOREACH(xml_attr_t, attr, &node->attr, next) {
			json_print_attr(attr, encoding);
			if (SLIST_NEXT(attr, next) || SLIST_FIRST(&node->child))
				putchar(',');
		}

		SLIST_FOREACH(xml_node_t, child, &node->child, next) {
			_print_as_json(child, encoding);
			if (SLIST_NEXT(child, next))
				putchar(',');
		}
	}
	putchar(array ? ']' : '}');
}

static void print_as_json(const xml_document_t *doc)
{
	_print_as_json(doc->root, doc->encoding);
}

void xml_dump(const xml_document_t *doc, int type)
{
	if (type == XML_AS_XML)
		print_as_xml(doc);
	else
		print_as_json(doc);
}
