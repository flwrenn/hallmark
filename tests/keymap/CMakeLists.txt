add_executable(test_keymap test_keymap.c)
target_link_libraries(test_keymap PRIVATE unity hallmark_algorithms)

add_test(NAME keymap COMMAND $<TARGET_FILE:test_keymap>)
