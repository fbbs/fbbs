#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "fbbs/backend.h"
#include "fbbs/fileio.h"

static int backend_proxy_fd = -1;

int backend_proxy_connect(const char *socket_path, bool client)
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return -1;

	struct sockaddr_un local, proxy;
	memset(&local, 0, sizeof(local));
	memset(&proxy, 0, sizeof(proxy));
	local.sun_family = AF_UNIX;
	proxy.sun_family = AF_UNIX;
	snprintf(local.sun_path, sizeof(local.sun_path),
			"%s/%s.%d", client ? "client" : "server", socket_path, getpid());
	snprintf(proxy.sun_path, sizeof(proxy.sun_path), "%s/proxy", socket_path);

	if (bind(fd, (struct sockaddr *) &local, sizeof(local)) != 0
			|| connect(fd, (struct sockaddr *) &proxy, sizeof(proxy)) != 0) {
		close(fd);
		return -1;
	}
	backend_proxy_fd = fd;
	return fd;
}

char *backend_proxy_read(int fd, char *buf, size_t *size)
{
	if (fd < 0)
		return NULL;
	size_t received = 0, length = 0;
	do {
		int rc = file_read(fd, buf + received, *size - received);
		if (rc <= 0)
			return NULL;
		received += rc;

		if (received >= 4) {
			for (int i = 0; i < 4; ++i) {
				length |= (buf[i] << (i * 8));
			}
			if (length > *size) {
				char *old_buf = buf;
				buf = malloc(length);
				if (!buf)
					return NULL;
				memcpy(buf, old_buf, received);
			}
			*size = length;
		}
	} while (received < *size);
	return buf;
}

enum {
	BACKEND_OK,
	BACKEND_ERROR,
	BACKEND_SERVER_BUSY,
};

static int _backend_request(const void *req, void *resp,
		backend_serializer_t serializer, backend_deserializer_t deserializer,
		backend_request_e type)
{
	int pid = getpid();

	parcel_t parcel_out;
	parcel_new(&parcel_out);
	parcel_write_varint(&parcel_out, type);
	parcel_write_varint(&parcel_out, pid);
	serializer(req, &parcel_out);

	if (!parcel_ok(&parcel_out)) {
		parcel_free(&parcel_out);
		return BACKEND_ERROR;
	}

	bool send_success = parcel_flush(&parcel_out, backend_proxy_fd);
	parcel_free(&parcel_out);
	if (!send_success)
		return BACKEND_ERROR;

	char buf[4096];
	size_t size = sizeof(buf);
	char *res = backend_proxy_read(backend_proxy_fd, buf, &size);

	int rc = BACKEND_ERROR;
	if (buf) {
		parcel_t parcel_in;
		parcel_read_new(res, size, &parcel_in);
		bool busy = parcel_read_varint(&parcel_in);
		if (busy) {
			rc = BACKEND_SERVER_BUSY;
			goto r;
		}

		bool ok = parcel_read_varint(&parcel_in);
		backend_request_e response_type = parcel_read_varint(&parcel_in);
		if (parcel_ok(&parcel_in) && ok && response_type == type) {
			deserializer(&parcel_in, resp);
			if (parcel_ok(&parcel_in)) {
				rc = BACKEND_OK;
				goto r;
			}
		}

	}
r:	if (res != buf)
		free(res);
	return rc;
}

bool backend_request(const void *req, void *resp,
		backend_serializer_t serializer, backend_deserializer_t deserializer,
		backend_request_e type)
{
	for (int i = 0; i < 5; ++i) {
		int rc = _backend_request(req, resp, serializer, deserializer, type);
		switch (rc) {
			case BACKEND_OK:
				return true;
			case BACKEND_SERVER_BUSY:
				sleep(1);
				break;
			default:
				return false;
		}
	}
	return false;
}
