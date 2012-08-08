#ifndef FB_XML_H
#define FB_XML_H

enum {
	XML_ATTR_TYPE_VALUE = 0,
	XML_ATTR_TYPE_TEXT,
	XML_ATTR_TYPE_INTEGER,
	XML_ATTR_TYPE_BIGINT,
	XML_ATTR_TYPE_BOOLEAN,
};

typedef struct xml_document_t xml_document_t;
typedef struct xml_node_t xml_node_t;
typedef struct xml_attr_t xml_attr_t;

extern xml_document_t *xml_new_doc(void);
extern void xml_set_doc_root(xml_document_t *doc, xml_node_t *root);

extern xml_node_t *xml_new_node(const char *name);
extern xml_node_t *xml_add_child(xml_node_t *parent, xml_node_t *child);

extern xml_attr_t *xml_attr_value(xml_node_t *node, const char *name, const char *value);
extern xml_attr_t *xml_attr_text(xml_node_t *node, const char *name, const char *value);
extern xml_attr_t *xml_attr_integer(xml_node_t *node, const char *name, int value);
extern xml_attr_t *xml_attr_bigint(xml_node_t *node, const char *name, int64_t value);
extern xml_attr_t *xml_attr_boolean(xml_node_t *node, const char *name, bool value);

#endif // FB_XML_H
