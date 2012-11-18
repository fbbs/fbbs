find_path(HIREDIS_INCLUDE_DIR
	NAMES
		hiredis/hiredis.h
    PATHS
		/usr/include
		/usr/local/include
)

find_library(HIREDIS_LIBRARY
	NAMES
		hiredis
    PATHS
		/usr/lib
		/usr/local/lib
)
