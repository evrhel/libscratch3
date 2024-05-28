cmake_minimum_required(VERSION 3.20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)

project("libscratch3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libscratch3)
add_subdirectory(scratch3)
add_subdirectory(libzip)
add_subdirectory(lysys)
add_subdirectory(SDL)
