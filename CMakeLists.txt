cmake_minimum_required(VERSION 3.23)
project(experiment C)

set(CMAKE_C_STANDARD 17)

add_executable(experiment P2P.c Filer.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

target_link_libraries(experiment ${GTK3_LIBRARIES})