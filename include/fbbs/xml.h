#ifndef FB_XML_H
#define FB_XML_H

#define XML_ATTR_STRING(k, v)  { k, XML_ATTR_ENUM_STRING, { .str = v } }
#define XML_ATTR_INTEGER(k, v)  { k, XML_ATTR_ENUM_INTEGER, { .integer = v } }
#define XML_ATTR_BIGINT(k, v)  { k, XML_ATTR_ENUM_BIGINT, { .bigint = v } }
#define XML_ATTR_BOOLEAN(k, v)  { k, XML_ATTR_ENUM_BOOLEAN, { .boolean = v } }


enum {
	XML_ATTR_ENUM_STRING = 0,
	XML_ATTR_ENUM_INTEGER,
	XML_ATTR_ENUM_BIGINT,
	XML_ATTR_ENUM_BOOLEAN,
};

typedef struct xml_attr_pair_t {
	const char *name;
	int type;
	union {
		const char *str;
		int integer;
		int64_t bigint;
		bool boolean;
	} value;
} xml_attr_pair_t;

typedef struct xml_document_t xml_document_t;
typedef struct xml_node_t xml_node_t;
typedef struct xml_attr_t xml_attr_t;

extern xml_document_t *xml_new_doc(void);
extern void xml_set_doc_root(xml_document_t *doc, xml_node_t *root);
extern xml_node_t *xml_new_node(const char *name);
extern xml_node_t *xml_attach_child(xml_node_t *parent, xml_node_t *child);
extern xml_attr_t *xml_attr_str(xml_node_t *node, const char *name, const char *value);
extern xml_attr_t *xml_attr(xml_node_t *node, const char *name, const char *fmt, ...);
extern xml_attr_t *xml_attrs(xml_node_t *node, xml_attr_pair_t *attrs, size_t size);

#endif // FB_XML_H
