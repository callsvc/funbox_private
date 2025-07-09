cmake_minimum_required(VERSION 3.31)

add_executable(smallgba main.c cartridge.c cartridge.h state.c state.h)
target_include_directories(smallgba PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(smallgba PUBLIC core)