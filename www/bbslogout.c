#include "libweb.h"
#include "fbbs/uinfo.h"
#include "fbbs/web.h"

static void abort_program(void)
{
	struct userec *x = NULL;

	if(!strcmp(u_info->userid, currentuser.userid)) {
		uidshm->status[u_info->uid - 1]--;
		bzero(u_info, sizeof(struct user_info));
	} 

	if (getuser(currentuser.userid)) {
		x = &lookupuser;
		update_user_stay(x, false, false);
		save_user_data(x);
	}
}

int bbslogout_main(web_ctx_t *ctx)
{
	if (!loginok) {
		printf("Location: sec\n\n");
		return 0;
	}
	abort_program();
	printf("Set-cookie: utmpnum=;expires=Fri, 19-Apr-1996 11:11:11 GMT\n"
			"Set-cookie: utmpkey=;expires=Fri, 19-Apr-1996 11:11:11 GMT\n"
			"Set-cookie: utmpuserid=;expires=Fri, 19-Apr-1996 11:11:11 GMT\n"
			"Location: sec\n\n");
	return 0;
}
