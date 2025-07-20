cmake_minimum_required(VERSION 3.31)

add_executable(nextps main.c cpu.c cpu.h cop0.c cop0.h bus.c ps.h)
target_include_directories(nextps PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(nextps PUBLIC core mbedtls)