cmake_minimum_required(VERSION 3.31)

set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)
add_subdirectory(libzip)

set(ENABLE_PROGRAMS OFF CACHE BOOL "Build Mbed TLS programs." FORCE)
add_subdirectory(mbedtls)
add_subdirectory(libxml2)