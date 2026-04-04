CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

project(libscl C)

#include(CMakeCStdOpt)

set(HEADERS
	include/libscl/SCL.h
	include/libscl/SCL_map.h
	)

set(SOURCES
	src/map.c
	)

add_library(scl ${SOURCES} ${HEADERS})

if (NOT WINDOWS)
target_compile_definitions(scl PUBLIC C99 _unix)
target_compile_options(scl PRIVATE -Wl,-no-undefined)
else()
target_compile_definitions(scl PUBLIC C99)
endif()

target_include_directories(scl
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/libscl
	)

install(TARGETS scl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY include/libscl DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
