cmake_minimum_required(VERSION 3.31)

add_executable(funbox funbox.c applet.c applet.h applet_types.c)
target_include_directories(funbox PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:>)

target_link_libraries(funbox PUBLIC dl core mbedtls)

function(mkdir dirname)
    add_custom_command(TARGET funbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:funbox>/${dirname})
endfunction()

function(add_funbox_applet applet_name)
    add_custom_command(TARGET funbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${applet_name}> $<TARGET_FILE_DIR:funbox>)
    mkdir(${applet_name}_cache)

    if(${applet_name} STREQUAL "nextps")
        mkdir(${applet_name}_cache/bios)
    endif()
endfunction()

add_funbox_applet(nextps)
add_custom_command(TARGET funbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dist $<TARGET_FILE_DIR:funbox>)
