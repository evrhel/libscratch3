cmake_minimum_required(VERSION 3.15)

project(sdisas3)

find_package(SDL2 CONFIG REQUIRED)

set(src "src")

set(SOURCES
    ${src}/main.cpp)

add_executable(sdisas3 ${SOURCES})

target_link_libraries(sdisas3 PRIVATE liblysys)
target_link_libraries(sdisas3 PUBLIC libscratch3)
target_link_libraries(sdisas3 PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_include_directories(sdisas3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libscratch3/src)
