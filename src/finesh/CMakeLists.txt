cmake_minimum_required(VERSION 3.31)
add_executable(finesh main.c config.c config.h bundle/pkglist.c bundle/pkglist.h)

target_include_directories(finesh PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(finesh PRIVATE core)