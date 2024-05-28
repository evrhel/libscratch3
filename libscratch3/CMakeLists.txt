cmake_minimum_required(VERSION 3.20)

project(libscratch3)

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
target_link_libraries(libscratch3 cairo::cairo)
