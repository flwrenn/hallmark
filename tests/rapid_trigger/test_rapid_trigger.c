/*
 * Unit tests for the rapid trigger FSM.
 *
 * Synthetic ADC parameters based on documented hardware expectations:
 *   - Rest ADC:      200   (key fully up)
 *   - Bottom-out ADC: 900  (key fully down)
 *   - Usable range:   700  counts over 3.4mm travel
 *   - counts_per_mm: ~206
 *   - Noise sigma:    ~3-5 counts
 */

#include "rapid_trigger.h"
#include "unity.h"

/* --------------------------------------------------------------------- */
/* Synthetic ADC parameters                                              */
/* --------------------------------------------------------------------- */

#define REST_ADC      200
#define BOTTOM_ADC    900
#define USABLE_RANGE  (BOTTOM_ADC - REST_ADC) /* 700 */
#define TRAVEL_MM     3.4f
#define COUNTS_PER_MM (USABLE_RANGE / TRAVEL_MM) /* ~206 */

/* Actuation at ~0.5mm from rest */
#define ACTUATION_ADC (REST_ADC + 103) /* ~303 */
/* Release point slightly above rest */
#define RELEASE_ADC   (REST_ADC + 20) /* ~220 */

/* Sensitivity thresholds in counts (approximate mm equivalents) */
#define SENS_01MM 21  /* ~0.1mm */
#define SENS_02MM 41  /* ~0.2mm */
#define SENS_05MM 103 /* ~0.5mm */

/* --------------------------------------------------------------------- */
/* Test helpers                                                          */
/* --------------------------------------------------------------------- */

static struct rt_key_state key;

static struct rt_config make_config(uint16_t sensitivity)
{
    struct rt_config cfg = {
            .rest_adc = REST_ADC,
            .actuation_adc = ACTUATION_ADC,
            .release_adc = RELEASE_ADC,
            .rt_sensitivity = sensitivity,
    };
    return cfg;
}

static void init_key(uint16_t sensitivity)
{
    struct rt_config cfg = make_config(sensitivity);
    rt_init(&key, &cfg);
}

/* Feed an array of ADC samples, count press/release events. */
static void feed_samples(const uint16_t *samples, int count, int *presses, int *releases)
{
    *presses = 0;
    *releases = 0;
    for (int i = 0; i < count; i++) {
        enum rt_event ev = rt_update(&key, samples[i]);
        if (ev == RT_EVENT_PRESS) {
            (*presses)++;
        } else if (ev == RT_EVENT_RELEASE) {
            (*releases)++;
        }
    }
}

void setUp(void)
{
}
void tearDown(void)
{
}

/* --------------------------------------------------------------------- */
/* Tests: basic state transitions                                        */
/* --------------------------------------------------------------------- */

void test_starts_idle(void)
{
    init_key(SENS_02MM);
    TEST_ASSERT_EQUAL_INT(RT_IDLE, key.state);
}

void test_no_event_below_actuation(void)
{
    init_key(SENS_02MM);

    /* Stay below actuation point */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, REST_ADC));
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, REST_ADC + 50));
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, ACTUATION_ADC - 1));
    TEST_ASSERT_EQUAL_INT(RT_IDLE, key.state);
}

void test_press_at_actuation(void)
{
    init_key(SENS_02MM);

    TEST_ASSERT_EQUAL_INT(RT_EVENT_PRESS, rt_update(&key, ACTUATION_ADC));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);
}

void test_press_past_actuation(void)
{
    init_key(SENS_02MM);

    TEST_ASSERT_EQUAL_INT(RT_EVENT_PRESS, rt_update(&key, ACTUATION_ADC + 50));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);
}

/* --------------------------------------------------------------------- */
/* Tests: full press and release cycle                                   */
/* --------------------------------------------------------------------- */

void test_full_press_release(void)
{
    init_key(SENS_02MM);

    /* Press down past actuation */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_PRESS, rt_update(&key, ACTUATION_ADC));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);

    /* Continue pressing deeper -- no new event */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, 500));
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, 700));
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, BOTTOM_ADC));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);

    /* Start releasing -- small movement, not enough */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, BOTTOM_ADC - 10));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);

    /* Release past sensitivity threshold from peak */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_RELEASE, rt_update(&key, BOTTOM_ADC - SENS_02MM));
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);

    /* Continue up past release point -> IDLE */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, RELEASE_ADC));
    TEST_ASSERT_EQUAL_INT(RT_IDLE, key.state);
}

void test_release_to_idle_from_active(void)
{
    init_key(SENS_02MM);

    /* Press */
    rt_update(&key, ACTUATION_ADC);

    /* Jump straight back to rest (fast tap) */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_RELEASE, rt_update(&key, RELEASE_ADC));
    TEST_ASSERT_EQUAL_INT(RT_IDLE, key.state);
}

/* --------------------------------------------------------------------- */
/* Tests: rapid trigger re-actuation                                     */
/* --------------------------------------------------------------------- */

void test_repress_from_releasing(void)
{
    init_key(SENS_02MM);

    /* Press deep */
    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 700);

    /* Release past sensitivity -> RELEASING */
    rt_update(&key, 700 - SENS_02MM);
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);

    /* Small downward movement -- not enough to re-press */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, 700 - SENS_02MM + 10));
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);

    /* Push back down past sensitivity from trough -> re-ACTIVE */
    uint16_t trough = 700 - SENS_02MM;
    TEST_ASSERT_EQUAL_INT(RT_EVENT_PRESS, rt_update(&key, trough + SENS_02MM));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);
}

void test_rapid_press_release_cycle(void)
{
    init_key(SENS_02MM);

    /* Initial press */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_PRESS, rt_update(&key, ACTUATION_ADC));

    /* Push to 600 */
    rt_update(&key, 500);
    rt_update(&key, 600);

    /* 5 rapid trigger cycles around 600 */
    int presses = 0;
    int releases = 0;
    for (int i = 0; i < 5; i++) {
        /* Release */
        enum rt_event ev = rt_update(&key, 600 - SENS_02MM);
        if (ev == RT_EVENT_RELEASE) {
            releases++;
        }
        /* Re-press */
        ev = rt_update(&key, 600);
        if (ev == RT_EVENT_PRESS) {
            presses++;
        }
    }

    TEST_ASSERT_EQUAL_INT(5, presses);
    TEST_ASSERT_EQUAL_INT(5, releases);
}

/* --------------------------------------------------------------------- */
/* Tests: sensitivity levels                                             */
/* --------------------------------------------------------------------- */

void test_sensitivity_0_1mm(void)
{
    init_key(SENS_01MM);

    /* Press to 700, then tiny reversal of exactly sensitivity */
    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 700);

    /* Just under threshold -- no release */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, 700 - SENS_01MM + 1));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);

    /* At threshold -- release */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_RELEASE, rt_update(&key, 700 - SENS_01MM));
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);
}

void test_sensitivity_0_5mm(void)
{
    init_key(SENS_05MM);

    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 700);

    /* Reversal of 0.2mm -- too small for 0.5mm sensitivity */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, 700 - SENS_02MM));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);

    /* Reversal of 0.5mm -- triggers */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_RELEASE, rt_update(&key, 700 - SENS_05MM));
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);
}

/* --------------------------------------------------------------------- */
/* Tests: noise rejection                                                */
/* --------------------------------------------------------------------- */

void test_no_false_triggers_on_static_input(void)
{
    init_key(SENS_02MM);

    /*
     * 1000 samples at rest with +-4 count noise (realistic sigma ~3).
     * Must produce zero events (S4 pass criterion: 0/1000).
     */
    int presses = 0, releases = 0;
    for (int i = 0; i < 1000; i++) {
        /* Deterministic noise pattern: -4, -2, 0, +2, +4, ... */
        int noise = ((i % 5) - 2) * 2;
        uint16_t sample = (uint16_t)(REST_ADC + noise);
        enum rt_event ev = rt_update(&key, sample);
        if (ev == RT_EVENT_PRESS) {
            presses++;
        }
        if (ev == RT_EVENT_RELEASE) {
            releases++;
        }
    }

    TEST_ASSERT_EQUAL_INT(0, presses);
    TEST_ASSERT_EQUAL_INT(0, releases);
}

void test_no_false_triggers_when_held(void)
{
    init_key(SENS_02MM);

    /* Press to mid-travel */
    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 550);

    /* Hold with noise -- no spurious release */
    int releases = 0;
    for (int i = 0; i < 500; i++) {
        int noise = ((i % 5) - 2) * 2;
        uint16_t sample = (uint16_t)(550 + noise);
        enum rt_event ev = rt_update(&key, sample);
        if (ev == RT_EVENT_RELEASE) {
            releases++;
        }
    }

    TEST_ASSERT_EQUAL_INT(0, releases);
}

/* --------------------------------------------------------------------- */
/* Tests: edge cases                                                     */
/* --------------------------------------------------------------------- */

void test_slow_press_to_bottom(void)
{
    init_key(SENS_02MM);
    int presses = 0, releases = 0;

    /* Ramp from rest to bottom-out in 1-count increments */
    for (uint16_t adc = REST_ADC; adc <= BOTTOM_ADC; adc++) {
        enum rt_event ev = rt_update(&key, adc);
        if (ev == RT_EVENT_PRESS) {
            presses++;
        }
        if (ev == RT_EVENT_RELEASE) {
            releases++;
        }
    }

    /* Exactly one press, no releases */
    TEST_ASSERT_EQUAL_INT(1, presses);
    TEST_ASSERT_EQUAL_INT(0, releases);
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);
}

void test_half_press_and_hold(void)
{
    init_key(SENS_02MM);

    /* Press halfway */
    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 550);

    /* Hold for many cycles */
    for (int i = 0; i < 200; i++) {
        TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, 550));
    }

    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);
}

void test_rapid_double_tap(void)
{
    init_key(SENS_02MM);

    /*
     * 50 full tap cycles: press to 500, release to rest.
     * Each should produce exactly 1 press + 1 release.
     */
    int total_presses = 0, total_releases = 0;

    for (int tap = 0; tap < 50; tap++) {
        /* Down stroke */
        for (uint16_t adc = REST_ADC; adc <= 500; adc += 20) {
            enum rt_event ev = rt_update(&key, adc);
            if (ev == RT_EVENT_PRESS) {
                total_presses++;
            }
        }
        /* Up stroke back to rest */
        for (int adc = 500; adc >= REST_ADC; adc -= 20) {
            enum rt_event ev = rt_update(&key, (uint16_t)adc);
            if (ev == RT_EVENT_RELEASE) {
                total_releases++;
            }
        }
    }

    TEST_ASSERT_EQUAL_INT(50, total_presses);
    TEST_ASSERT_EQUAL_INT(50, total_releases);
    TEST_ASSERT_EQUAL_INT(RT_IDLE, key.state);
}

void test_partial_release_repress(void)
{
    init_key(SENS_02MM);

    /* Press deep */
    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 800);

    /* Partial release -- triggers RT release */
    rt_update(&key, 800 - SENS_02MM);
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);

    /* Re-press without returning to rest -- triggers RT press */
    uint16_t trough = 800 - SENS_02MM;
    TEST_ASSERT_EQUAL_INT(RT_EVENT_PRESS, rt_update(&key, trough + SENS_02MM));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);

    /* Continue pressing deeper */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, BOTTOM_ADC));
    TEST_ASSERT_EQUAL_INT(RT_ACTIVE, key.state);
}

void test_releasing_to_idle_on_rest(void)
{
    init_key(SENS_02MM);

    /* Press, then release past sensitivity to enter RELEASING */
    rt_update(&key, ACTUATION_ADC);
    rt_update(&key, 600);
    rt_update(&key, 600 - SENS_02MM);
    TEST_ASSERT_EQUAL_INT(RT_RELEASING, key.state);

    /* Return to rest -> should go IDLE, no extra events */
    TEST_ASSERT_EQUAL_INT(RT_EVENT_NONE, rt_update(&key, RELEASE_ADC));
    TEST_ASSERT_EQUAL_INT(RT_IDLE, key.state);
}

/* --------------------------------------------------------------------- */
/* Tests: conversion helpers                                             */
/* --------------------------------------------------------------------- */

void test_mm_to_counts(void)
{
    uint16_t c = rt_mm_to_counts(0.2f, COUNTS_PER_MM);
    /* 0.2 * ~206 = ~41 */
    TEST_ASSERT_INT_WITHIN(2, 41, c);
}

void test_counts_to_mm(void)
{
    float mm = rt_counts_to_mm(206, COUNTS_PER_MM);
    /* 206 / ~206 = ~1.0mm */
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, mm);
}

/* --------------------------------------------------------------------- */
/* Runner                                                                */
/* --------------------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();

    /* Basic transitions */
    RUN_TEST(test_starts_idle);
    RUN_TEST(test_no_event_below_actuation);
    RUN_TEST(test_press_at_actuation);
    RUN_TEST(test_press_past_actuation);

    /* Full cycles */
    RUN_TEST(test_full_press_release);
    RUN_TEST(test_release_to_idle_from_active);

    /* Rapid trigger re-actuation */
    RUN_TEST(test_repress_from_releasing);
    RUN_TEST(test_rapid_press_release_cycle);

    /* Sensitivity levels */
    RUN_TEST(test_sensitivity_0_1mm);
    RUN_TEST(test_sensitivity_0_5mm);

    /* Noise rejection */
    RUN_TEST(test_no_false_triggers_on_static_input);
    RUN_TEST(test_no_false_triggers_when_held);

    /* Edge cases */
    RUN_TEST(test_slow_press_to_bottom);
    RUN_TEST(test_half_press_and_hold);
    RUN_TEST(test_rapid_double_tap);
    RUN_TEST(test_partial_release_repress);
    RUN_TEST(test_releasing_to_idle_on_rest);

    /* Helpers */
    RUN_TEST(test_mm_to_counts);
    RUN_TEST(test_counts_to_mm);

    return UNITY_END();
}
