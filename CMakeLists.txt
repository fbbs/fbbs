cmake_minimum_required(VERSION 2.8)

project(fbbs)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
set(CMAKE_COLOR_MAKEFILE off)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(ENABLE_SSH "compile ssh module" off)
option(ENABLE_DEBUG "compile debug version" on)
option(ENABLE_WWW "compile www module" on)
option(ENABLE_PG "compile postgresql module" on)
option(ENABLE_BANK "compile bbs bank module" on)
option(ENABLE_FDQUAN "compile for Fudan Quan BBS" on)

include(CheckIncludeFiles)
check_include_files(crypt.h HAVE_CRYPT_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(features.h HAVE_FEATURES_H)
check_include_files(endian.h HAVE_ENDIAN_H)

include(CheckFunctionExists)
check_function_exists(select HAVE_SELECT)
check_function_exists(getaddrinfo HAVE_GETADDRINFO)
check_function_exists(poll HAVE_POLL)
check_function_exists(gethostbyname HAVE_GETHOSTBYNAME)
check_function_exists(regcomp HAVE_REGCOMP)
check_function_exists(strlcpy HAVE_STRLCPY)

set(CMAKE_C_FLAGS "-O2 -pipe -Wall -std=c99 -fPIC -fstack-protector-all")
if(ENABLE_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
endif(ENABLE_DEBUG)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -fPIE")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_REQUIRED_INCLUDES "/usr/local/include/;${CMAKE_REQUIRED_INCLUDES}")
include_directories("${CMAKE_BINARY_DIR}/include" "include" "/usr/local/include")
if(ENABLE_PG)
	set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};/usr/include/postgresql")
	include_directories("${CMAKE_BINARY_DIR}/include" "include" "/usr/include/postgresql")
endif(ENABLE_PG)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(FB_XOPEN_SOURCE_DEFINE -D_XOPEN_SOURCE=600)
	set(FB_PLATFORM_DEFINE -DLINUX)
	set(FB_LIBDL_LINK dl)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_package(HiRedis REQUIRED)
find_package(GCrypt REQUIRED)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(backend)
add_subdirectory(src)
if(ENABLE_WWW)
	find_program(UGLIFYJS uglifyjs)
	find_program(LESSC lessc)
	set(DOCUMENT_ROOT "/var/www/localhost/htdocs/fbbs")
	add_subdirectory(html)
	add_subdirectory(fcgi)
	add_subdirectory(less)
	add_subdirectory(js)
endif(ENABLE_WWW)
add_subdirectory(util)

configure_file(include/config.h.cmake include/config.h)
