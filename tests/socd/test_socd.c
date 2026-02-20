/*
 * Unit tests for SOCD cleaning (last-input-wins).
 */

#include "socd.h"
#include "unity.h"

/* --------------------------------------------------------------------- */

static struct socd_pair pair;

void setUp(void)
{
    socd_init(&pair);
}

void tearDown(void)
{
}

/* --------------------------------------------------------------------- */
/* Tests: initial state                                                  */
/* --------------------------------------------------------------------- */

void test_init_neither_active(void)
{
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

/* --------------------------------------------------------------------- */
/* Tests: single key                                                     */
/* --------------------------------------------------------------------- */

void test_only_a_pressed(void)
{
    socd_press_a(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_TRUE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

void test_only_b_pressed(void)
{
    socd_press_b(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_TRUE(r.b_output);
}

void test_a_press_release(void)
{
    socd_press_a(&pair);
    socd_release_a(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

/* --------------------------------------------------------------------- */
/* Tests: simultaneous press -- last input wins                          */
/* --------------------------------------------------------------------- */

void test_a_then_b_outputs_b(void)
{
    socd_press_a(&pair);
    socd_press_b(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_TRUE(r.b_output);
}

void test_b_then_a_outputs_a(void)
{
    socd_press_b(&pair);
    socd_press_a(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_TRUE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

/* --------------------------------------------------------------------- */
/* Tests: release ordering                                               */
/* --------------------------------------------------------------------- */

void test_release_winner_reverts_to_loser(void)
{
    /* A then B -> B wins */
    socd_press_a(&pair);
    socd_press_b(&pair);

    /* Release B (the winner) -> A should now output */
    socd_release_b(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_TRUE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

void test_release_loser_keeps_winner(void)
{
    /* A then B -> B wins */
    socd_press_a(&pair);
    socd_press_b(&pair);

    /* Release A (the loser) -> B still outputs */
    socd_release_a(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_TRUE(r.b_output);
}

void test_release_both(void)
{
    socd_press_a(&pair);
    socd_press_b(&pair);
    socd_release_a(&pair);
    socd_release_b(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

/* --------------------------------------------------------------------- */
/* Tests: staggered sequences                                            */
/* --------------------------------------------------------------------- */

void test_a_b_release_b_b_again(void)
{
    /* A, then B (B wins) */
    socd_press_a(&pair);
    socd_press_b(&pair);

    /* Release B, re-press B -> B wins again */
    socd_release_b(&pair);
    socd_press_b(&pair);
    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_FALSE(r.a_output);
    TEST_ASSERT_TRUE(r.b_output);
}

void test_alternating_presses(void)
{
    /* A -> B -> release A -> press A -> A wins */
    socd_press_a(&pair);
    socd_press_b(&pair);
    socd_release_a(&pair);
    socd_press_a(&pair);

    struct socd_result r = socd_resolve(&pair);
    TEST_ASSERT_TRUE(r.a_output);
    TEST_ASSERT_FALSE(r.b_output);
}

/* --------------------------------------------------------------------- */
/* Tests: multiple pairs are independent                                 */
/* --------------------------------------------------------------------- */

void test_two_independent_pairs(void)
{
    struct socd_pair lr, ud;
    socd_init(&lr);
    socd_init(&ud);

    /* Left+Right: right wins */
    socd_press_a(&lr);
    socd_press_b(&lr);

    /* Up only */
    socd_press_a(&ud);

    struct socd_result r_lr = socd_resolve(&lr);
    struct socd_result r_ud = socd_resolve(&ud);

    TEST_ASSERT_FALSE(r_lr.a_output); /* left suppressed */
    TEST_ASSERT_TRUE(r_lr.b_output);  /* right wins */
    TEST_ASSERT_TRUE(r_ud.a_output);  /* up active */
    TEST_ASSERT_FALSE(r_ud.b_output); /* down inactive */
}

/* --------------------------------------------------------------------- */
/* Runner                                                                */
/* --------------------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();

    /* Initial state */
    RUN_TEST(test_init_neither_active);

    /* Single key */
    RUN_TEST(test_only_a_pressed);
    RUN_TEST(test_only_b_pressed);
    RUN_TEST(test_a_press_release);

    /* Simultaneous -- last input wins */
    RUN_TEST(test_a_then_b_outputs_b);
    RUN_TEST(test_b_then_a_outputs_a);

    /* Release ordering */
    RUN_TEST(test_release_winner_reverts_to_loser);
    RUN_TEST(test_release_loser_keeps_winner);
    RUN_TEST(test_release_both);

    /* Staggered sequences */
    RUN_TEST(test_a_b_release_b_b_again);
    RUN_TEST(test_alternating_presses);

    /* Multiple pairs */
    RUN_TEST(test_two_independent_pairs);

    return UNITY_END();
}
