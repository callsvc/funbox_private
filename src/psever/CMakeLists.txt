add_executable(psever main.c pkg_loader.c pkg_loader.h pfs.c pfs.h)

target_include_directories(psever PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(psever PUBLIC core)