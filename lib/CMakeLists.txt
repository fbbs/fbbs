set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
add_definitions(${FB_XOPEN_SOURCE_DEFINE} ${FB_PLATFORM_DEFINE})

add_library(fbbs_base SHARED cfg.c convert.c fileio.c hash.c mmap.c
		pool.c string.c time.c util.c)

add_library(fbbs SHARED board.c boardrc.c brdcache.c log.c mail.c
		status.c pass.c post.c record.c shm.c helper.c sysconf.c ucache.c
		uinfo.c register.c user.c session.c title.c friend.c mdbi.c)
target_link_libraries(fbbs m crypt fbbs_base fbbs_pg hiredis)

install(TARGETS fbbs_base fbbs LIBRARY DESTINATION lib)

if(ENABLE_PG)
	add_library(fbbs_pg SHARED pg.c)
	target_link_libraries(fbbs_pg fbbs_base pq)
	install(TARGETS fbbs_pg LIBRARY DESTINATION lib)
endif(ENABLE_PG)
