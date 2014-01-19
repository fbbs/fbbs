#include <signal.h>
#include "bbs.h"
#include "fbbs/backend.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/parcel.h"

BACKEND_DECLARE(post_new);
BACKEND_DECLARE(post_delete);
BACKEND_DECLARE(post_undelete);
BACKEND_DECLARE(post_set_flag);

#define ENTRY(function)  [BACKEND_REQUEST_##function] = backend_##function

typedef bool (*handler_t)(parcel_t *parcel_in, parcel_t *parcel_out,
		int channel);

static const handler_t handlers[] = {
	ENTRY(post_new),
	ENTRY(post_delete),
	ENTRY(post_undelete),
	ENTRY(post_set_flag),
};

static sig_atomic_t backend_shutdown = false;
static sig_atomic_t backend_accepting = false;

void backend_respond(parcel_t *parcel, int channel)
{
	if (channel <= 0)
		return;
	mdb_cmd_safe("LPUSH", "%s_%d %b", BACKEND_RESPONSE_KEY, channel,
			parcel->ptr, parcel_size(parcel));
}

static void backend_respond_error(parcel_t *parcel, int channel)
{
	parcel_clear(parcel);
	parcel_put(bool, false);
	backend_respond(parcel, channel);
}

static void shutdown_handler(int sig)
{
	if (backend_accepting)
		exit(EXIT_SUCCESS);
	backend_shutdown = true;
}

extern int resolve_ucache(void);

int main(int argc, char **argv)
{
	start_daemon();

	if (setgid(BBSGID) != 0)
		return EXIT_FAILURE;
	if (setuid(BBSUID) != 0)
		return EXIT_FAILURE;
	chdir(BBSHOME);
	umask(S_IWGRP | S_IWOTH);

	initialize_environment(INIT_MDB | INIT_DB | INIT_CONV);
	if (resolve_ucache() < 0)
		return EXIT_FAILURE;

	fb_signal(SIGTERM, shutdown_handler);

	while (!backend_shutdown) {
		backend_accepting = true;
		mdb_res_t *res = mdb_res("BLPOP", "%s %d", BACKEND_REQUEST_KEY, 0);
		backend_accepting = false;
		if (!res)
			return 0;

		bool ok = false;
		int type = 0, channel = 0;
		parcel_t parcel_out;
		parcel_new(&parcel_out);

		mdb_res_t *real_res = mdb_res_at(res, 1);
		size_t size;
		const char *ptr = mdb_string_and_size(real_res, &size);
		if (ptr) {
			parcel_t parcel_in;
			parcel_read_new(ptr, size, &parcel_in);
			type = parcel_read_varint(&parcel_in);
			channel = parcel_read_varint(&parcel_in);

			if (parcel_ok(&parcel_in) && type > 0
					&& type < ARRAY_SIZE(handlers)) {
				handler_t handler = handlers[type];
				if (handler) {
					parcel_write_bool(&parcel_out, true);
					parcel_write_varint(&parcel_out, type);

					ok = handler(&parcel_in, &parcel_out, channel);
				}
			}

		}
		mdb_clear(res);

		if (!ok)
			backend_respond_error(&parcel_out, channel);
		parcel_free(&parcel_out);
	}
	return EXIT_SUCCESS;
}
