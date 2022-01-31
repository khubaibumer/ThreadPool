cmake_minimum_required(VERSION 3.18)
project(ThreadPool C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

add_library(ThreadPool SHARED ThreadPool.c)
add_definitions(${GLIB_CFLAGS_OTHER})
target_link_libraries(ThreadPool ${GLIB_LIBRARIES})
