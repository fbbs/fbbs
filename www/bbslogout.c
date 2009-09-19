#include "libweb.h"

static void abort_program(void)
{
	int stay = 0;
	struct userec *x = NULL;
	int loginstart = 0;

	if(!strcmp(u_info->userid, currentuser.userid)) {
#ifdef SPARC
		loginstart = *(int*)(u_info->from + 30);
#else
		loginstart = *(int*)(u_info->from + 32);
#endif
		uidshm->status[u_info->uid - 1]--;
		bzero(u_info, sizeof(struct user_info));
	} 

	if (getuser(currentuser.userid)) {
		x = &lookupuser;
		time_t now = time(NULL);
		time_t recent;
		recent = loginstart;
		if (x->lastlogout > recent)
			recent = x->lastlogout;
		if (x->lastlogin > recent)
			recent = x->lastlogin;
		stay = now - recent;
		if (stay < 0)
			stay = 0;
		x->stay += stay;
		x->lastlogout = now;
		save_user_data(x);
	}
}

int bbslogout_main(void)
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
