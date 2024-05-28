cmake_minimum_required(VERSION 3.20)

project(scratch3)

set(src "src")

set(SOURCES
    ${src}/main.cpp)

add_executable(scratch3 ${SOURCES})

target_link_libraries(scratch3 PUBLIC libscratch3)
target_link_libraries(scratch3 PRIVATE liblysys)
