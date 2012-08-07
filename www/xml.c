#include "fbbs/list.h"
#include "fbbs/pool.h"
#include "fbbs/web.h"
#include "fbbs/xml.h"

struct xml_attr_t {
	const char *name;
	const char *value;
	SLIST_FIELD(xml_attr_t) next;
};

SLIST_HEAD(xml_attr_list_t, xml_attr_t);
SLIST_HEAD(xml_node_list_t, xml_node_t);

struct xml_node_t {
	const char *name;
	struct xml_attr_list_t attr;
	struct xml_node_list_t child;
	SLIST_FIELD(xml_node_t) next;
};

struct xml_document_t {
	xml_node_t *root;
};

static inline void *palloc(size_t size)
{
	return pool_alloc(ctx.p, size);
}

static inline char *pstrdup(const char *s)
{
	return pool_strdup(ctx.p, s, 0);
}

xml_document_t *xml_new_doc(void)
{
	xml_document_t *doc = palloc(sizeof(*doc));
	doc->root = NULL;
	return doc;
}

void xml_set_doc_root(xml_document_t *doc, xml_node_t *root)
{
	doc->root = root;
}

xml_node_t *xml_new_node(const char *name)
{
	xml_node_t *node = palloc(sizeof(*node));
	node->name = pstrdup(name);
	SLIST_INIT_HEAD(&node->attr);
	SLIST_INIT_HEAD(&node->child);
	return node;
}

xml_node_t *xml_attach_child(xml_node_t *parent, xml_node_t *child)
{
	SLIST_INSERT_HEAD(&parent->child, child, next);
	return child;
}

static xml_attr_t *xml_attach_attr(xml_node_t *node, xml_attr_t *attr)
{
	SLIST_INSERT_HEAD(&node->attr, attr, next);
	return attr;
}

static xml_attr_t *_xml_attr_str(const char *name, const char *value,
		bool copy)
{
	xml_attr_t *attr = palloc(sizeof(*attr));
	attr->name = name;
	attr->value = copy ? pstrdup(value) : value;
	return attr;
}

xml_attr_t *xml_attr_str(xml_node_t *node, const char *name, const char *value)
{
	return xml_attach_attr(node, _xml_attr_str(name, value, true));
}

static xml_attr_t *xml_attr_vprintf(const char *name, const char *fmt,
		va_list ap)
{
	char buf[16];
	va_list aq;
	va_copy(aq, ap);
	size_t size = vsnprintf(buf, sizeof(buf), fmt, aq);
	va_end(aq);

	if (size >= sizeof(buf)) {
		char *str = palloc(size + 1);
		vsnprintf(str, size, fmt, ap);
		return _xml_attr_str(name, str, false);
	}

	return _xml_attr_str(name, buf, true);
}

xml_attr_t *xml_attr(xml_node_t *node, const char *name, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	xml_attr_t *attr = xml_attr_vprintf(name, fmt, ap);
	va_end(ap);
	return xml_attach_attr(node, attr);
}

static xml_attr_t *xml_attr_int(xml_node_t *node, const char *name, int value)
{
	return xml_attr(node, name, "%d", value);
}

static xml_attr_t *xml_attr_bigint(xml_node_t *node, const char *name, int64_t value)
{
	return xml_attr(node, name, "%"PRId64, value);
}

static xml_attr_t *xml_attr_bool_default(xml_node_t *node, const char *name,
		bool value)
{
	if (value)
		return xml_attach_attr(node, _xml_attr_str(name, "1", false));
	return NULL;
}

xml_attr_t *xml_attrs(xml_node_t *node, xml_attr_pair_t *attrs, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		switch (attrs->type) {
			case XML_ATTR_ENUM_STRING:
				xml_attr_str(node, attrs->name, attrs->value.str);
				break;
			case XML_ATTR_ENUM_INTEGER:
				xml_attr_int(node, attrs->name, attrs->value.integer);
				break;
			case XML_ATTR_ENUM_BIGINT:
				xml_attr_bigint(node, attrs->name, attrs->value.bigint);
				break;
			case XML_ATTR_ENUM_BOOLEAN:
				xml_attr_bool_default(node, attrs->name, attrs->value.boolean);
			default:
				break;
		}
		++attrs;
	}
	return SLIST_FIRST(&node->attr);
}
