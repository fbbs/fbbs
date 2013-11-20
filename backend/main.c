#include "fbbs/backend.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/parcel.h"

extern void post_new(parcel_t *);

#define ENTRY(handler)  [BACKEND_REQUEST_##handler] = handler

typedef void (*handler_t)(parcel_t *parcel);

const static handler_t handlers[] = {
	ENTRY(post_new),
};

int main(int argc, char **argv)
{
	initialize_environment(INIT_MDB | INIT_DB | INIT_CONV);

	while (1) {
		mdb_res_t *res = mdb_res("BLPOP", BACKEND_REQUEST_KEY);
		if (!res)
			return 0;

		size_t size;
		const char *ptr = mdb_string_and_size(res, &size);
		if (ptr) {
			parcel_t parcel;
			parcel_read_new(ptr, size, &parcel);

			int idx = parcel_read_varint(&parcel);
			if (parcel_ok(&parcel) && idx >= 0 && idx < ARRAY_SIZE(handlers)) {
				handler_t handler = handlers[idx];
				if (handler)
					handler(&parcel);
			}
		}

		mdb_clear(res);
	}
	return 0;
}
