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
BACKEND_DECLARE(post_alter_title);

#define ENTRY(function)  [BACKEND_REQUEST_##function] = backend_##function

typedef bool (*handler_t)(parcel_t *parcel_in, parcel_t *parcel_out,
		int channel);

static const handler_t handlers[] = {
	ENTRY(post_new),
	ENTRY(post_delete),
	ENTRY(post_undelete),
	ENTRY(post_set_flag),
	ENTRY(post_alter_title),
};

static sig_atomic_t backend_shutdown = false;
static sig_atomic_t backend_accepting = false;

void backend_respond(parcel_t *parcel, int fd)
{
	parcel_flush(parcel, fd);
}

static void backend_respond_error(parcel_t *parcel, int fd)
{
	parcel_clear(parcel);
	parcel_put(bool, false);
	backend_respond(parcel, fd);
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
	const char *socket_path = getenv("FBBS_SOCKET_PATH");
	if (!socket_path)
		return EXIT_FAILURE;

	if (setgid(BBSGID) != 0)
		return EXIT_FAILURE;
	if (setuid(BBSUID) != 0)
		return EXIT_FAILURE;
	chdir(BBSHOME);
	umask(S_IWGRP | S_IWOTH);

	initialize_environment(INIT_MDB | INIT_DB | INIT_CONV);
	if (resolve_ucache() < 0)
		return EXIT_FAILURE;

	int fd = backend_proxy_connect(socket_path, false);
	if (fd < 0)
		return EXIT_FAILURE;

	backend_proxy_error_on_sighup();
	fb_signal(SIGTERM, shutdown_handler);

	while (!backend_shutdown) {
		backend_accepting = true;

		char buf[4096];
		size_t size = sizeof(buf);
		char *ptr = backend_proxy_read(fd, buf, &size);

		backend_accepting = false;
		if (!ptr)
			return EXIT_FAILURE;

		bool ok = false;
		int type = 0;
		parcel_t parcel_out;
		parcel_new(&parcel_out);
		parcel_write_bool(&parcel_out, false);

		if (ptr) {
			parcel_t parcel_in;
			parcel_read_new(ptr, size, &parcel_in);
			type = parcel_read_varint(&parcel_in);

			if (parcel_ok(&parcel_in) && type > 0
					&& type < ARRAY_SIZE(handlers)) {
				handler_t handler = handlers[type];
				if (handler) {
					parcel_write_bool(&parcel_out, true);
					parcel_write_varint(&parcel_out, type);

					ok = handler(&parcel_in, &parcel_out, fd);
				}
			}
		}
		if (ptr != buf)
			free(ptr);

		if (!ok)
			backend_respond_error(&parcel_out, fd);
		parcel_free(&parcel_out);
	}
	return EXIT_SUCCESS;
}
