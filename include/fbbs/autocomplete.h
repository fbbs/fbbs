#ifndef FB_AUTOCOMPLETE_H
#define FB_AUTOCOMPLETE_H

#include <stddef.h>

typedef struct ac_list ac_list;

extern ac_list *ac_list_new(void);
extern void ac_list_add(ac_list *acl, const char *name);
extern void ac_list_free(ac_list *acl);
extern void autocomplete(ac_list *acl, const char *prompt, char *buf, size_t size);

#endif // FB_AUTOCOMPLETE_H
