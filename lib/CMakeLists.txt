set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Werror")
add_definitions(-D_XOPEN_SOURCE=500)

add_library(fbbs STATIC util.c string.c file.c hash.c mmap.c socket.c pass.c cache.c
	cfg.c pool.c)
