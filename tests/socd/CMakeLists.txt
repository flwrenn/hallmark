add_executable(test_socd test_socd.c)
target_link_libraries(test_socd PRIVATE unity hallmark_algorithms)

add_test(NAME socd COMMAND test_socd)
