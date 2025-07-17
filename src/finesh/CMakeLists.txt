cmake_minimum_required(VERSION 3.31)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
add_executable(finesh main.c config.c config.h bundle/pkglist.c bundle/pkglist.h bundle/loader.c bundle/loader.h sh_fs/plist.c sh_fs/plist.h)

target_include_directories(finesh PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(finesh PRIVATE core LibXml2::LibXml2)