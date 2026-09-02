add_executable(test_keymap_config test_keymap_config.c)
target_link_libraries(test_keymap_config PRIVATE unity hallmark_algorithms)

add_test(NAME keymap_config COMMAND $<TARGET_FILE:test_keymap_config>)
