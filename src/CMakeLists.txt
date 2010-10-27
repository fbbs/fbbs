set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Werror")
add_definitions(-D_XOPEN_SOURCE=500)

link_directories(../lib)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_executable(bbsd bbsd.c sysconf.c termio.c screen.c main.c stuff.c more.c)
target_link_libraries(bbsd crypt dl fbbs)

install(TARGETS bbsd RUNTIME DESTINATION bin)

