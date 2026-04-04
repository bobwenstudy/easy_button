cmake_minimum_required(VERSION 3.5)

project(EzBtn LANGUAGES C)
add_subdirectory(libscl)
add_executable(EzBtn main.c
                     ebtn/bit_array.h
                     ebtn/ebtn.c
                     ebtn/ebtn.h
                     example_user.c
)

include_directories(EzBtn PUBLIC
                     ebtn
)

target_link_libraries(EzBtn LINK_PRIVATE scl pthread)

target_include_directories(EzBtn PRIVATE 
	$<TARGET_PROPERTY:scl,INTERFACE_INCLUDE_DIRECTORIES>
)

include(GNUInstallDirs)
install(TARGETS EzBtn
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
