cmake_minimum_required(VERSION 3.15)

project(scratch3)

set(CMAKE_CXX_STANDARD 14)

set(src "src")

set(SOURCES
    ${src}/main.cpp)

add_executable(scratch3 ${SOURCES})

target_link_libraries(scratch3 PUBLIC libscratch3)
target_link_libraries(scratch3 PRIVATE liblysys)
