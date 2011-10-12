set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Werror")
add_definitions(-D_XOPEN_SOURCE=500 -D_BSD_SOURCE)

add_library(fbbs_pg SHARED pg.c)
target_link_libraries(fbbs_pg fbbs_base pq)
install(TARGETS fbbs_pg LIBRARY DESTINATION lib)
