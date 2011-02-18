set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Werror")
add_definitions(-D_XOPEN_SOURCE=600)

add_executable(fbwebd web.c main.c board.c parse.c post.c)

target_link_libraries(fbwebd crypt fcgi fbbs fbbspg)

install(TARGETS fbwebd RUNTIME DESTINATION bin)
