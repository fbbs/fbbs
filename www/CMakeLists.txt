set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
add_definitions(${FB_XOPEN_SOURCE_DEFINE} ${FB_PLATFORM_DEFINE})

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_directories(/usr/local/lib)

add_executable(bbswebd libweb.c main.c bbssec.c bbsall.c login.c
	bbslogout.c bbsdoc.c bbscon.c bbspst.c bbssnd.c bbsqry.c bbsclear.c
	bbsupload.c bbsann.c bbsnot.c mail.c bbstcon.c bbserr.c
	bbsfall.c bbsplan.c bbsdel.c bbsinfo.c bbsrss.c
	bbstop10.c register.c parse.c web.c board.c prop.c)
target_link_libraries(bbswebd crypt gcrypt fcgi fbbs)
install(TARGETS bbswebd RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID SETGID)
