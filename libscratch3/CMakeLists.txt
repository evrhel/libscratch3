cmake_minimum_required(VERSION 3.15)

project(libscratch3)

find_package(PkgConfig REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)

pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(LIBRSVG librsvg-2.0 IMPORTED_TARGET REQUIRED)

set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glad)

set(src "src")

set(SOURCES
	${src}/ast/ast.cpp
	${src}/ast/astdef.cpp
	${src}/ast/visitor.cpp
	${src}/codegen/compiler.cpp
	${src}/render/renderer.cpp
	${src}/render/shader.cpp
	${src}/render/stb.cpp
	${src}/vm/costume.cpp
	${src}/vm/debug.cpp
	${src}/vm/io.cpp
	${src}/vm/memory.cpp
	${src}/vm/script.cpp
	${src}/vm/sprite.cpp
	${src}/vm/vm.cpp
	${src}/ref.cpp
	${src}/core.cpp
	${src}/resource.cpp
	${GLAD_DIR}/src/glad.c)

add_library(libscratch3 SHARED ${SOURCES})

target_include_directories(libscratch3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(libscratch3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson/include)

target_include_directories(libscratch3 PRIVATE ${src})
target_include_directories(libscratch3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MatrixUtil/MatrixUtil/include)
target_include_directories(libscratch3 PRIVATE ${GLAD_DIR}/include)
target_include_directories(libscratch3 PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(libscratch3 PRIVATE liblysys)
target_link_libraries(libscratch3 PRIVATE zip)
target_link_libraries(libscratch3 PRIVATE PkgConfig::cairo)
target_link_libraries(libscratch3 PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(libscratch3 PRIVATE imgui::imgui)
target_link_libraries(libscratch3 PRIVATE PkgConfig::LIBRSVG)
