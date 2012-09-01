#ifndef FB_LIST_H
#define FB_LIST_H

#define SLIST_HEAD(name, type)  struct name { struct type *first; }

#define SLIST_HEAD_INITIALIZER  { .first = NULL }

#define SLIST_FIRST(head)  ((head)->first)

#define SLIST_INIT_HEAD(head)  do { (head)->first = NULL; } while (0)

#define SLIST_FIELD(type)  struct { struct type *next; }

#define SLIST_NEXT(entry, field)  ((entry)->field.next)

#define SLIST_INSERT_HEAD(head, entry, field)  \
	do { \
		SLIST_NEXT(entry, field) = SLIST_FIRST(head); \
		SLIST_FIRST(head) = entry; \
	} while (0)

#define SLIST_INSERT_AFTER(base, entry, field) \
	do { \
		SLIST_NEXT(entry, field) = SLIST_NEXT(base, field); \
		SLIST_NEXT(base, field) = entry; \
	} while (0)

#define SLIST_FOREACH(type, var, head, field)  \
	for (type *var = SLIST_FIRST(head); var; var = SLIST_NEXT(var, field))

#endif // FB_LIST_H
