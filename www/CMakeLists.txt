set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")
add_definitions(-D_XOPEN_SOURCE=600)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_executable(bbswebd libweb.c main.c bbssec.c bbsall.c bbslogin.c
	bbslogout.c bbsdoc.c bbscon.c bbspst.c bbssnd.c bbsqry.c bbsclear.c
	bbsupload.c bbsann.c bbsnot.c mail.c bbstcon.c bbsmybrd.c bbserr.c
	bbsfav.c bbsfall.c bbsplan.c bbsdel.c bbsinfo.c bbssel.c bbsrss.c
	bbstop10.c register.c parse.c web.c)
target_link_libraries(bbswebd crypt fcgi fbbs)
install(TARGETS bbswebd
	RUNTIME DESTINATION bin)
