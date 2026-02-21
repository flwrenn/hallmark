add_executable(test_smoke test_smoke.c)
target_link_libraries(test_smoke PRIVATE unity hallmark_algorithms)

add_test(NAME smoke COMMAND $<TARGET_FILE:test_smoke>)
