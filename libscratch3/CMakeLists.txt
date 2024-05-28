cmake_minimum_required(VERSION 3.15)

project(libscratch3)

find_package(PkgConfig REQUIRED)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)

find_package(SDL2 CONFIG REQUIRED)

set(src "src")

set(SOURCES
	${src}/ast/ast.cpp
	${src}/ast/astdef.cpp
	${src}/ast/visitor.cpp
	${src}/semantics/syminfo.cpp
	${src}/vm/vm.cpp
	${src}/ref.cpp
	${src}/core.cpp
	${src}/resource.cpp)

add_library(libscratch3 SHARED ${SOURCES})

target_include_directories(libscratch3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(libscratch3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson/include)

target_include_directories(libscratch3 PRIVATE ${src})

target_link_libraries(libscratch3 PRIVATE liblysys)
target_link_libraries(libscratch3 PRIVATE zip)
target_link_libraries(libscratch3 PRIVATE PkgConfig::cairo)
target_link_libraries(libscratch3 PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
