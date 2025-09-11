cmake_minimum_required(VERSION 3.31)

set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)
add_subdirectory(libzip)

set(ENABLE_PROGRAMS OFF CACHE BOOL "Build Mbed TLS programs." FORCE)
add_subdirectory(mbedtls)
add_subdirectory(libxml2)


if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(MI_TRACK_VALGRIND ON CACHE BOOL "Compile with Valgrind support (adds a small overhead)" FORCE)
endif()
set(MI_BUILD_TESTS OFF CACHE BOOL "Build test executables" FORCE)

add_subdirectory(mimalloc)
add_subdirectory(SDL_ttf)
