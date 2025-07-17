cmake_minimum_required(VERSION 3.31)

add_executable(nextps main.c cpu.c bus.c ps.h)
target_include_directories(nextps PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(nextps PUBLIC core mbedtls)