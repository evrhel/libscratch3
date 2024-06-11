cmake_minimum_required(VERSION 3.15)

message(STATUS "Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

project("libscratch3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libzip)
add_subdirectory(lysys)
add_subdirectory(libscratch3)
add_subdirectory(scratch3)
add_subdirectory(sdisas3)
