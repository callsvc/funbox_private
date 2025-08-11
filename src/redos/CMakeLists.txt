cmake_minimum_required(VERSION 3.31)
add_executable(redos main.c dospc.c dospc.h disk.c disk.h pc8086.c pc8086.h)
target_include_directories(redos PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:>)

target_link_libraries(redos PUBLIC core)