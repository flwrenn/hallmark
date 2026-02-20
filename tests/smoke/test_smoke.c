/*
 * Smoke test -- validates that the host-native test pipeline
 * (CMake + Unity + CTest) is working correctly.
 */

#include "unity.h"

void setUp(void)
{
}
void tearDown(void)
{
}

void test_sanity(void)
{
    TEST_ASSERT_TRUE(1);
}

void test_arithmetic(void)
{
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sanity);
    RUN_TEST(test_arithmetic);
    return UNITY_END();
}
