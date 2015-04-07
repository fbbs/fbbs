set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
add_definitions(${FB_XOPEN_SOURCE_DEFINE} ${FB_PLATFORM_DEFINE})

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_directories(/usr/local/lib)

add_executable(bbswebd libweb.c main.c login.c toc.c post.c bbsupload.c
	bbsann.c mail.c bbserr.c friend.c session.c register.c parse.c web.c
	board.c prop.c xml.c user.c json.c)
target_link_libraries(bbswebd crypt gcrypt fcgi fbbs)
install(TARGETS bbswebd RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID SETGID)
