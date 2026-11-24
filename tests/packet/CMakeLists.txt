add_executable(test_packet test_packet.c)
target_link_libraries(test_packet PRIVATE unity hallmark_algorithms)

add_test(NAME packet COMMAND $<TARGET_FILE:test_packet>)
