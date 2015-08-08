#include "bbs.h"
#include "fbbs/autocomplete.h"
#include "fbbs/terminal.h"

static ac_list *user_build_ac_list(void)
{
	if (!uidshm)
		return NULL;

	ac_list *acl = ac_list_new();
	if (!acl)
		return NULL;

	int size = uidshm->number;
	for (int i = 0; i < size; ++i) {
		const char *user_name = uidshm->userid[i];
		if (*user_name)
			ac_list_add(acl, user_name);
	}
	return acl;
}

void user_complete(int row, const char *prompt, char *name, size_t size)
{
	if (name && size)
		*name = '\0';
	ac_list *acl = user_build_ac_list();
	if (!acl)
		return;

	screen_move(row, 0);
	autocomplete(acl, prompt, name, size);
	ac_list_free(acl);
}
