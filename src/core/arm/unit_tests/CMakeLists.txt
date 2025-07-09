add_executable(fibo_arm64 fibo_arm64.c)

target_link_libraries(fibo_arm64 PRIVATE arm_dynrec)