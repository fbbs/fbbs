#include "bbs.h"

bool gbrd_is_custom_dir(const gbrdh_t *bh)
{
	return (bh->flag & BOARD_CUSTOM_FLAG);
}

