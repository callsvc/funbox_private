add_executable(starsys2 main.c ee/ee.c ee/ee.h ee/cop0.c ee/cop0.h ee/dmac.c ee/dmac.h iop/mips.c iop/mips.h sony.c sony.h bridge.c bridge.h types.h)

target_include_directories(starsys2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)
target_link_libraries(starsys2 PRIVATE core)