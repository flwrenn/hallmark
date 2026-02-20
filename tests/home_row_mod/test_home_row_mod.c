/*
 * Unit tests for depth-based home row mod logic.
 *
 * Synthetic ADC parameters match the rapid trigger test suite:
 *   - Rest ADC:      200
 *   - Bottom-out ADC: 900
 *   - Usable range:   700 counts over 3.4mm travel
 */

#include "home_row_mod.h"
#include "unity.h"

/* --------------------------------------------------------------------- */
/* Synthetic parameters                                                  */
/* --------------------------------------------------------------------- */

#define REST_ADC 200

/*
 * Depth threshold: ~1.5mm of travel -> ~309 counts above rest.
 * Time threshold: 200ms (standard HRM timeout).
 */
#define DEPTH_THRESHOLD 309
#define TIME_THRESHOLD  200

/* --------------------------------------------------------------------- */
/* Helpers                                                               */
/* --------------------------------------------------------------------- */

static struct hrm_key_state key;

static const struct hrm_config default_cfg = {
        .depth_threshold_adc = DEPTH_THRESHOLD,
        .time_threshold_ms = TIME_THRESHOLD,
        .rest_adc = REST_ADC,
};

/* Pinky config: deeper threshold, longer timeout. */
static const struct hrm_config pinky_cfg = {
        .depth_threshold_adc = 400,
        .time_threshold_ms = 250,
        .rest_adc = REST_ADC,
};

void setUp(void)
{
    hrm_init(&key);
}

void tearDown(void)
{
}

/* --------------------------------------------------------------------- */
/* Tests: initialisation                                                 */
/* --------------------------------------------------------------------- */

void test_init_state(void)
{
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, key.action);
    TEST_ASSERT_FALSE(key.pressed);
}

/* --------------------------------------------------------------------- */
/* Tests: Path 1 -- fast shallow tap resolves as TAP                     */
/* --------------------------------------------------------------------- */

void test_fast_shallow_tap(void)
{
    hrm_press(&key, 0);

    /* Shallow press -- stays below depth threshold */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + 100, 10, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    a = hrm_update(&key, REST_ADC + 50, 30, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    /* Release before time threshold -> TAP */
    a = hrm_release(&key);
    TEST_ASSERT_EQUAL_INT(HRM_TAP, a);
}

void test_immediate_release_is_tap(void)
{
    hrm_press(&key, 0);

    /* Release immediately without any update -> TAP */
    enum hrm_action a = hrm_release(&key);
    TEST_ASSERT_EQUAL_INT(HRM_TAP, a);
}

/* --------------------------------------------------------------------- */
/* Tests: Path 2 -- deep press resolves as immediate HOLD                */
/* --------------------------------------------------------------------- */

void test_deep_press_immediate_hold(void)
{
    hrm_press(&key, 0);

    /* Press past depth threshold */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + DEPTH_THRESHOLD, 5, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

void test_deep_press_well_past_threshold(void)
{
    hrm_press(&key, 0);

    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + DEPTH_THRESHOLD + 200, 5, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

void test_depth_hold_speed_under_50ms(void)
{
    /*
     * S4 pass criterion: HOLD activation < 50ms for depth-triggered path.
     * The depth path resolves on the very first sample that crosses
     * the threshold, so as long as scan rate is >=1kHz (1ms per sample),
     * this is guaranteed.  Test that it resolves at timestamp 1ms.
     */
    hrm_press(&key, 0);

    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + DEPTH_THRESHOLD, 1, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

/* --------------------------------------------------------------------- */
/* Tests: Path 3 -- time threshold fallback HOLD                         */
/* --------------------------------------------------------------------- */

void test_time_threshold_hold(void)
{
    hrm_press(&key, 100);

    /* Hold shallow for a long time */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + 50, 150, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    a = hrm_update(&key, REST_ADC + 50, 299, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    /* At exactly time threshold -> HOLD */
    a = hrm_update(&key, REST_ADC + 50, 300, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

void test_time_threshold_just_under(void)
{
    hrm_press(&key, 0);

    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + 50, TIME_THRESHOLD - 1, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    /* Release just before timeout -> TAP */
    a = hrm_release(&key);
    TEST_ASSERT_EQUAL_INT(HRM_TAP, a);
}

/* --------------------------------------------------------------------- */
/* Tests: latched result                                                 */
/* --------------------------------------------------------------------- */

void test_hold_latches(void)
{
    hrm_press(&key, 0);

    /* Trigger depth HOLD */
    hrm_update(&key, REST_ADC + DEPTH_THRESHOLD, 5, &default_cfg);

    /* Subsequent updates still return HOLD */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + 50, 10, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);

    /* Release also returns HOLD (not TAP) */
    a = hrm_release(&key);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

void test_reset_on_new_press(void)
{
    hrm_press(&key, 0);
    hrm_update(&key, REST_ADC + DEPTH_THRESHOLD, 5, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, key.action);

    /* New press resets to PENDING */
    hrm_press(&key, 100);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, key.action);
}

/* --------------------------------------------------------------------- */
/* Tests: per-finger configuration                                       */
/* --------------------------------------------------------------------- */

void test_pinky_deeper_threshold(void)
{
    hrm_press(&key, 0);

    /* At default depth threshold -- not enough for pinky */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + DEPTH_THRESHOLD, 5, &pinky_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    /* At pinky threshold -> HOLD */
    a = hrm_update(&key, REST_ADC + 400, 10, &pinky_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

void test_pinky_longer_timeout(void)
{
    hrm_press(&key, 0);

    /* At default time threshold -- not enough for pinky */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + 50, TIME_THRESHOLD, &pinky_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);

    /* At pinky time threshold -> HOLD */
    a = hrm_update(&key, REST_ADC + 50, 250, &pinky_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, a);
}

/* --------------------------------------------------------------------- */
/* Tests: edge cases                                                     */
/* --------------------------------------------------------------------- */

void test_update_when_not_pressed(void)
{
    /* Not pressed -- update should be no-op */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC + 500, 100, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);
}

void test_adc_below_rest(void)
{
    hrm_press(&key, 0);

    /* ADC below rest (noise) -- depth should be 0, no HOLD */
    enum hrm_action a;
    a = hrm_update(&key, REST_ADC - 10, 5, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_PENDING, a);
}

void test_rapid_tap_hold_tap_alternation(void)
{
    /* Tap */
    hrm_press(&key, 0);
    hrm_update(&key, REST_ADC + 50, 10, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_TAP, hrm_release(&key));

    /* Hold */
    hrm_press(&key, 100);
    hrm_update(&key, REST_ADC + DEPTH_THRESHOLD, 105, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_HOLD, hrm_release(&key));

    /* Tap again */
    hrm_press(&key, 200);
    hrm_update(&key, REST_ADC + 50, 210, &default_cfg);
    TEST_ASSERT_EQUAL_INT(HRM_TAP, hrm_release(&key));
}

/* --------------------------------------------------------------------- */
/* Tests: false activation rate                                          */
/* --------------------------------------------------------------------- */

void test_no_false_holds_on_fast_typing(void)
{
    /*
     * Simulate 500 fast taps (shallow + quick release).
     * S4 pass criterion: < 1/500 false activations.
     */
    int false_holds = 0;

    for (int i = 0; i < 500; i++) {
        uint32_t t = (uint32_t)(i * 80); /* ~75 WPM keystroke interval */
        hrm_press(&key, t);

        /* Shallow press at t+10, release at t+40 */
        hrm_update(&key, REST_ADC + 100, t + 10, &default_cfg);
        hrm_update(&key, REST_ADC + 80, t + 25, &default_cfg);
        enum hrm_action a = hrm_release(&key);

        if (a == HRM_HOLD) {
            false_holds++;
        }
    }

    TEST_ASSERT_EQUAL_INT(0, false_holds);
}

/* --------------------------------------------------------------------- */
/* Runner                                                                */
/* --------------------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();

    /* Initialisation */
    RUN_TEST(test_init_state);

    /* Path 1: TAP */
    RUN_TEST(test_fast_shallow_tap);
    RUN_TEST(test_immediate_release_is_tap);

    /* Path 2: depth HOLD */
    RUN_TEST(test_deep_press_immediate_hold);
    RUN_TEST(test_deep_press_well_past_threshold);
    RUN_TEST(test_depth_hold_speed_under_50ms);

    /* Path 3: time HOLD */
    RUN_TEST(test_time_threshold_hold);
    RUN_TEST(test_time_threshold_just_under);

    /* Latching */
    RUN_TEST(test_hold_latches);
    RUN_TEST(test_reset_on_new_press);

    /* Per-finger config */
    RUN_TEST(test_pinky_deeper_threshold);
    RUN_TEST(test_pinky_longer_timeout);

    /* Edge cases */
    RUN_TEST(test_update_when_not_pressed);
    RUN_TEST(test_adc_below_rest);
    RUN_TEST(test_rapid_tap_hold_tap_alternation);

    /* False activation rate */
    RUN_TEST(test_no_false_holds_on_fast_typing);

    return UNITY_END();
}
