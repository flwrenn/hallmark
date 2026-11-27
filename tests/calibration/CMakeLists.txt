add_executable(test_calibration test_calibration.c)
target_link_libraries(test_calibration PRIVATE unity hallmark_algorithms)

add_test(NAME calibration COMMAND $<TARGET_FILE:test_calibration>)
