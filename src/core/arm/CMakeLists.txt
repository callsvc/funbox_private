add_library(arm_dynrec)

target_sources(arm_dynrec PUBLIC dynrec.c dynrec.h frontend/aarch64/arm64_frontend.c frontend/aarch64/arm64_frontend.h jit.c jit.h types.h)

target_link_libraries(arm_dynrec PUBLIC core)

add_subdirectory(unit_tests)