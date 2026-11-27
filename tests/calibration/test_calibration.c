/*
 * Unit tests for the calibration module.
 *
 * Synthetic key sampled at 1 kHz:
 *   - Rest reading:        200
 *   - Bottom-out reading:  900
 *   - Noise sigma:         ~3 counts, the 4x oversampling result in
 *                          prototyping/README.md
 *
 * EXAMPLE_CFG is a test fixture, not a tuned default.  Its time constants
 * are 1024 samples inside the rest band and 16 samples below it.
 */

#include "calibration.h"
#include "rapid_trigger.h"
#include "unity.h"

#include <stddef.h>

/* --------------------------------------------------------------------- */
/* Synthetic key and example configuration                               */
/* --------------------------------------------------------------------- */

#define REST          200
#define BOTTOM        900
#define OTHER_REST    1800
#define SAMPLES_PER_S 1000

#define ZERO_SAMPLES 64
#define REST_BAND    16
#define SLOW_SHIFT   10
#define FAST_SHIFT   4

#define NOISE_PEAK    8
#define NOISE_SAMPLES (30 * SAMPLES_PER_S)

#define DRIFT_COUNTS  30
#define DRIFT_SAMPLES (60 * SAMPLES_PER_S)

static const struct cal_config example_cfg = {
        .zero_samples = ZERO_SAMPLES,
        .rest_band = REST_BAND,
        .slow_shift = SLOW_SHIFT,
        .fast_shift = FAST_SHIFT,
};

/* --------------------------------------------------------------------- */
/* Test helpers                                                          */
/* --------------------------------------------------------------------- */

static struct cal_key key;
static uint32_t lcg_state;

/*
 * Feed the same reading count times and return the last depth.
 */
static uint16_t feed(struct cal_key *k, uint16_t raw, int count)
{
    uint16_t depth = 0;

    for (int i = 0; i < count; i++) {
        depth = cal_update(k, raw);
    }
    return depth;
}

/*
 * Initialise a key and run its whole zeroing window at one reading.
 */
static void boot_at(struct cal_key *k, const struct cal_config *cfg, uint16_t raw)
{
    cal_init(k, cfg);
    feed(k, raw, cfg->zero_samples);
}

/*
 * Numerical Recipes LCG.  The low bits have short periods, so only the
 * top 16 are returned.
 */
static uint32_t lcg_next(void)
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state >> 16;
}

/*
 * Sum of four uniform draws from -2..2: zero mean, variance 4 * 2 = 8
 * (sigma 2.83), and never beyond NOISE_PEAK either way.
 */
static int noise(void)
{
    int sum = 0;

    for (int i = 0; i < 4; i++) {
        sum += (int)(lcg_next() % 5) - 2;
    }
    return sum;
}

static uint16_t noisy(uint16_t level)
{
    return (uint16_t)(level + noise());
}

void setUp(void)
{
    lcg_state = 0x12345678u;
}

void tearDown(void)
{
}

/* --------------------------------------------------------------------- */
/* Tests: noise fixture                                                  */
/* --------------------------------------------------------------------- */

void test_noise_source_is_zero_mean_with_sigma_near_three(void)
{
    long sum = 0;
    long sum_sq = 0;

    for (int i = 0; i < NOISE_SAMPLES; i++) {
        int n = noise();

        TEST_ASSERT_INT_WITHIN(NOISE_PEAK, 0, n);
        sum += n;
        sum_sq += n * n;
    }

    TEST_ASSERT_INT_WITHIN(NOISE_SAMPLES / 20, 0, sum);
    TEST_ASSERT_INT_WITHIN(NOISE_SAMPLES, 8 * NOISE_SAMPLES, sum_sq);
}

/* --------------------------------------------------------------------- */
/* Tests: zeroing                                                        */
/* --------------------------------------------------------------------- */

void test_depth_is_zero_while_zeroing(void)
{
    cal_init(&key, &example_cfg);

    for (int i = 0; i < ZERO_SAMPLES; i++) {
        uint16_t raw = i < ZERO_SAMPLES / 2 ? REST : BOTTOM;

        TEST_ASSERT_EQUAL_INT(CAL_ZEROING, cal_get_phase(&key));
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, raw));
    }

    TEST_ASSERT_EQUAL_INT(CAL_TRACKING, cal_get_phase(&key));
}

/*
 * 32 readings of 190 and 32 of 214 average 202, which is neither the
 * first reading nor the last.
 */
void test_first_baseline_is_mean_of_zeroing_readings(void)
{
    cal_init(&key, &example_cfg);
    feed(&key, 190, ZERO_SAMPLES / 2);
    feed(&key, 214, ZERO_SAMPLES / 2);

    TEST_ASSERT_EQUAL_UINT16(202, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(100, cal_update(&key, 302));
}

/*
 * Means of 200.75 and 200.25 round to 201 and 200.
 */
void test_first_baseline_rounds_to_nearest_count(void)
{
    static const struct cal_config four_cfg = {
            .zero_samples = 4,
            .rest_band = REST_BAND,
            .slow_shift = SLOW_SHIFT,
            .fast_shift = FAST_SHIFT,
    };

    cal_init(&key, &four_cfg);
    feed(&key, 200, 3);
    cal_update(&key, 203);
    TEST_ASSERT_EQUAL_UINT16(201, cal_get_baseline(&key));

    cal_init(&key, &four_cfg);
    feed(&key, 200, 3);
    cal_update(&key, 201);
    TEST_ASSERT_EQUAL_UINT16(200, cal_get_baseline(&key));
}

void test_init_restarts_zeroing(void)
{
    boot_at(&key, &example_cfg, BOTTOM);
    boot_at(&key, &example_cfg, REST);

    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
}

/* --------------------------------------------------------------------- */
/* Tests: at rest                                                        */
/* --------------------------------------------------------------------- */

void test_steady_rest_reads_zero_and_keeps_baseline(void)
{
    boot_at(&key, &example_cfg, REST);

    for (int i = 0; i < 10 * SAMPLES_PER_S; i++) {
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
        TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
    }
}

/*
 * The mean of 64 noisy readings is within a count of the true rest.  At
 * a shift of 10 the baseline then averages about 2047 readings, which
 * leaves it a sigma of 2.83 / sqrt(2047) = 0.06 counts.  After 5 time
 * constants it must round to the true rest at every sample for 30 s.  A
 * bias of half a count either way fails.
 */
void test_noise_at_rest_does_not_bias_baseline(void)
{
    cal_init(&key, &example_cfg);

    for (int i = 0; i < ZERO_SAMPLES + 5 * SAMPLES_PER_S; i++) {
        cal_update(&key, noisy(REST));
    }
    TEST_ASSERT_EQUAL_INT(CAL_TRACKING, cal_get_phase(&key));

    for (int i = 0; i < NOISE_SAMPLES; i++) {
        uint16_t depth = cal_update(&key, noisy(REST));

        TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(NOISE_PEAK, depth);
    }
}

/*
 * A one-count error decays by 1023/1024 per sample and is under half a
 * count after 710 samples, so the baseline rounds to the reading by 1024.
 */
void test_one_count_error_still_moves_baseline(void)
{
    boot_at(&key, &example_cfg, REST);
    feed(&key, REST + 1, 1024);
    TEST_ASSERT_EQUAL_UINT16(REST + 1, cal_get_baseline(&key));

    boot_at(&key, &example_cfg, REST);
    feed(&key, REST - 1, 1024);
    TEST_ASSERT_EQUAL_UINT16(REST - 1, cal_get_baseline(&key));
}

/*
 * At CAL_SHIFT_MAX a one-count error moves the baseline 1/32768 of a
 * count and then stops, above or below.  A step that rounded one way
 * only would keep moving on that side, 1/32768 of a count per sample,
 * and cross to the next whole count within 40 s.
 */
void test_one_count_errors_up_and_down_both_stall_at_max_shift(void)
{
    static const struct cal_config max_cfg = {
            .zero_samples = ZERO_SAMPLES,
            .rest_band = REST_BAND,
            .slow_shift = CAL_SHIFT_MAX,
            .fast_shift = CAL_SHIFT_MAX,
    };
    struct cal_key below;

    boot_at(&key, &max_cfg, REST);
    boot_at(&below, &max_cfg, REST);
    feed(&key, REST + 1, 40 * SAMPLES_PER_S);
    feed(&below, REST - 1, 40 * SAMPLES_PER_S);

    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&below));
}

/*
 * The ramp moves 30 / 60000 = 0.0005 counts per sample, so the baseline
 * trails it by 0.0005 * 1024 = 0.5 counts.  Rounding the reading and the
 * baseline to whole counts adds up to 1.5 more.
 */
static void assert_baseline_follows_drift(int drift_counts)
{
    const uint16_t end = (uint16_t)(REST + drift_counts);

    boot_at(&key, &example_cfg, REST);

    for (int i = 0; i < DRIFT_SAMPLES; i++) {
        uint16_t raw = (uint16_t)(REST + drift_counts * i / DRIFT_SAMPLES);
        uint16_t depth = cal_update(&key, raw);

        TEST_ASSERT_UINT16_WITHIN(2, raw, cal_get_baseline(&key));
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(2, depth);
    }

    feed(&key, end, 5 * SAMPLES_PER_S);
    TEST_ASSERT_EQUAL_UINT16(end, cal_get_baseline(&key));
}

void test_baseline_follows_slow_upward_drift(void)
{
    assert_baseline_follows_drift(DRIFT_COUNTS);
}

void test_baseline_follows_slow_downward_drift(void)
{
    assert_baseline_follows_drift(-DRIFT_COUNTS);
}

/* --------------------------------------------------------------------- */
/* Tests: pressed                                                        */
/* --------------------------------------------------------------------- */

void test_depth_is_reading_minus_rest(void)
{
    boot_at(&key, &example_cfg, REST);

    TEST_ASSERT_EQUAL_UINT16(100, cal_update(&key, REST + 100));
    TEST_ASSERT_EQUAL_UINT16(BOTTOM - REST, cal_update(&key, BOTTOM));
    TEST_ASSERT_EQUAL_UINT16(350, cal_update(&key, REST + 350));
}

void test_long_hold_does_not_move_baseline(void)
{
    boot_at(&key, &example_cfg, REST);

    for (int i = 0; i < 10 * SAMPLES_PER_S; i++) {
        TEST_ASSERT_EQUAL_UINT16(500, cal_update(&key, REST + 500));
    }

    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
}

void test_reading_at_band_edge_is_a_press(void)
{
    boot_at(&key, &example_cfg, REST);

    for (int i = 0; i < 10 * SAMPLES_PER_S; i++) {
        TEST_ASSERT_EQUAL_UINT16(REST_BAND, cal_update(&key, REST + REST_BAND));
    }
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
}

/*
 * One count inside the band is rest, so the baseline absorbs it.  Ten
 * seconds is almost 10 time constants.
 */
void test_reading_inside_band_is_rest(void)
{
    boot_at(&key, &example_cfg, REST);
    feed(&key, REST + REST_BAND - 1, 10 * SAMPLES_PER_S);
    TEST_ASSERT_EQUAL_UINT16(REST + REST_BAND - 1, cal_get_baseline(&key));

    boot_at(&key, &example_cfg, REST);
    feed(&key, REST - (REST_BAND - 1), 10 * SAMPLES_PER_S);
    TEST_ASSERT_EQUAL_UINT16(REST - (REST_BAND - 1), cal_get_baseline(&key));
}

/* --------------------------------------------------------------------- */
/* Tests: baseline too high                                              */
/* --------------------------------------------------------------------- */

/*
 * The baseline starts 700 counts high.  Below the band the error shrinks
 * by 15/16 per sample and is under 16 counts after 59 samples.  Inside
 * the band it shrinks by 1023/1024 and is under half a count 3549 samples
 * later.  The stated limits are 64 samples to get within the band and
 * 4096 to reach the true rest.
 */
void test_boot_with_key_held_recovers_after_release(void)
{
    boot_at(&key, &example_cfg, BOTTOM);
    TEST_ASSERT_EQUAL_UINT16(BOTTOM, cal_get_baseline(&key));

    for (int i = 0; i < 64; i++) {
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
    }
    TEST_ASSERT_UINT16_WITHIN(REST_BAND, REST, cal_get_baseline(&key));

    for (int i = 64; i < 4096; i++) {
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
    }
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));

    TEST_ASSERT_EQUAL_UINT16(300, cal_update(&key, REST + 300));
    TEST_ASSERT_EQUAL_UINT16(BOTTOM - REST, cal_update(&key, BOTTOM));
}

/*
 * A drop of 100 counts is under 16 after 29 samples at 15/16 per sample.
 * The slow rate alone would cover 6 counts in 64 samples.
 */
void test_large_downward_shift_is_followed_fast(void)
{
    boot_at(&key, &example_cfg, REST);
    feed(&key, REST - 100, 64);

    TEST_ASSERT_UINT16_WITHIN(REST_BAND, REST - 100, cal_get_baseline(&key));
}

/*
 * A reading exactly rest_band below the baseline is below rest: the
 * 16-count error moves the baseline by 16 >> 4 = 1 count at once.  One
 * count nearer is rest, where the step is 15/1024 of a count.
 */
void test_reading_at_lower_band_edge_takes_a_fast_step(void)
{
    boot_at(&key, &example_cfg, REST);
    cal_update(&key, REST - REST_BAND);
    TEST_ASSERT_EQUAL_UINT16(REST - 1, cal_get_baseline(&key));

    boot_at(&key, &example_cfg, REST);
    cal_update(&key, REST - (REST_BAND - 1));
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
}

/* --------------------------------------------------------------------- */
/* Tests: extremes                                                       */
/* --------------------------------------------------------------------- */

void test_full_scale_zeroing_window_does_not_wrap(void)
{
    static const struct cal_config full_window_cfg = {
            .zero_samples = UINT16_MAX,
            .rest_band = REST_BAND,
            .slow_shift = SLOW_SHIFT,
            .fast_shift = FAST_SHIFT,
    };

    cal_init(&key, &full_window_cfg);
    TEST_ASSERT_EQUAL_UINT16(0, feed(&key, UINT16_MAX, UINT16_MAX - 1));
    TEST_ASSERT_EQUAL_INT(CAL_ZEROING, cal_get_phase(&key));

    TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, UINT16_MAX));
    TEST_ASSERT_EQUAL_INT(CAL_TRACKING, cal_get_phase(&key));
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, UINT16_MAX));
}

/*
 * From 65535 the error is under 16 counts after 129 fast samples and
 * under half a count 3549 slow samples later, well inside 8192.
 */
void test_reading_can_fall_from_full_scale_to_zero(void)
{
    uint16_t previous = UINT16_MAX;

    boot_at(&key, &example_cfg, UINT16_MAX);

    for (int i = 0; i < 8192; i++) {
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, 0));
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(previous, cal_get_baseline(&key));
        previous = cal_get_baseline(&key);
    }
    TEST_ASSERT_EQUAL_UINT16(0, cal_get_baseline(&key));
}

void test_full_scale_press_from_zero_baseline(void)
{
    boot_at(&key, &example_cfg, 0);

    for (int i = 0; i < SAMPLES_PER_S; i++) {
        TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, cal_update(&key, UINT16_MAX));
    }
    TEST_ASSERT_EQUAL_UINT16(0, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, 0));
}

/*
 * The band reaches past 65535 and past 0.  A reading 5 counts from the
 * baseline is still rest on both sides, and the baseline settles on it
 * without passing it.
 */
void test_rest_band_reaches_past_both_ends_of_the_range(void)
{
    uint16_t previous = UINT16_MAX - 5;

    boot_at(&key, &example_cfg, UINT16_MAX - 5);
    for (int i = 0; i < 10 * SAMPLES_PER_S; i++) {
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(5, cal_update(&key, UINT16_MAX));
        TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, cal_get_baseline(&key));
        previous = cal_get_baseline(&key);
    }
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, cal_get_baseline(&key));

    previous = 5;
    boot_at(&key, &example_cfg, 5);
    for (int i = 0; i < 10 * SAMPLES_PER_S; i++) {
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, 0));
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(previous, cal_get_baseline(&key));
        previous = cal_get_baseline(&key);
    }
    TEST_ASSERT_EQUAL_UINT16(0, cal_get_baseline(&key));
}

/* --------------------------------------------------------------------- */
/* Tests: configuration limits                                           */
/* --------------------------------------------------------------------- */

void test_zero_samples_of_zero_zeroes_on_first_reading(void)
{
    static const struct cal_config no_window_cfg = {
            .zero_samples = 0,
            .rest_band = REST_BAND,
            .slow_shift = SLOW_SHIFT,
            .fast_shift = FAST_SHIFT,
    };

    cal_init(&key, &no_window_cfg);
    TEST_ASSERT_EQUAL_INT(CAL_ZEROING, cal_get_phase(&key));

    TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
    TEST_ASSERT_EQUAL_INT(CAL_TRACKING, cal_get_phase(&key));
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(100, cal_update(&key, REST + 100));
}

/*
 * With no band, a reading at or above the baseline is a press and one
 * below it takes the fast step, so the baseline can fall but never rise.
 */
void test_rest_band_of_zero_never_raises_baseline(void)
{
    static const struct cal_config no_band_cfg = {
            .zero_samples = ZERO_SAMPLES,
            .rest_band = 0,
            .slow_shift = SLOW_SHIFT,
            .fast_shift = FAST_SHIFT,
    };

    boot_at(&key, &no_band_cfg, REST);

    TEST_ASSERT_EQUAL_UINT16(1, feed(&key, REST + 1, 10 * SAMPLES_PER_S));
    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));

    TEST_ASSERT_EQUAL_UINT16(0, feed(&key, REST - 5, SAMPLES_PER_S));
    TEST_ASSERT_EQUAL_UINT16(REST - 5, cal_get_baseline(&key));
}

/*
 * At CAL_SHIFT_MAX an 8-count error inside the band decays by
 * 32767/32768 per sample and is under one count after about 68140 samples,
 * where the step rounds to zero.  A wider shift must trace the same
 * path.  The same holds for a 100-count drop, which the fast shift
 * brings inside the band after about 60050 samples.
 */
static void assert_shift_behaves_as_max(uint8_t shift)
{
    static const struct cal_config max_cfg = {
            .zero_samples = ZERO_SAMPLES,
            .rest_band = REST_BAND,
            .slow_shift = CAL_SHIFT_MAX,
            .fast_shift = CAL_SHIFT_MAX,
    };
    const struct cal_config wide_cfg = {
            .zero_samples = ZERO_SAMPLES,
            .rest_band = REST_BAND,
            .slow_shift = shift,
            .fast_shift = shift,
    };
    struct cal_key wide;

    boot_at(&key, &max_cfg, REST);
    boot_at(&wide, &wide_cfg, REST);
    for (int i = 0; i < 70000; i++) {
        cal_update(&key, REST + 8);
        cal_update(&wide, REST + 8);
        TEST_ASSERT_EQUAL_UINT16(cal_get_baseline(&key), cal_get_baseline(&wide));
    }
    TEST_ASSERT_UINT16_WITHIN(1, REST + 8, cal_get_baseline(&wide));

    boot_at(&key, &max_cfg, REST);
    boot_at(&wide, &wide_cfg, REST);
    for (int i = 0; i < 70000; i++) {
        cal_update(&key, REST - 100);
        cal_update(&wide, REST - 100);
        TEST_ASSERT_EQUAL_UINT16(cal_get_baseline(&key), cal_get_baseline(&wide));
    }
    TEST_ASSERT_UINT16_WITHIN(REST_BAND, REST - 100, cal_get_baseline(&wide));
}

void test_shift_wider_than_max_behaves_as_max(void)
{
    assert_shift_behaves_as_max(CAL_SHIFT_MAX + 1);
    assert_shift_behaves_as_max(31);
    assert_shift_behaves_as_max(32);
    assert_shift_behaves_as_max(UINT8_MAX);
}

/* --------------------------------------------------------------------- */
/* Tests: independent keys                                               */
/* --------------------------------------------------------------------- */

void test_keys_sharing_a_config_are_independent(void)
{
    struct cal_key other;

    boot_at(&key, &example_cfg, REST);
    cal_init(&other, &example_cfg);

    for (int i = 0; i < ZERO_SAMPLES; i++) {
        TEST_ASSERT_EQUAL_INT(CAL_ZEROING, cal_get_phase(&other));
        TEST_ASSERT_EQUAL_UINT16(500, cal_update(&key, REST + 500));
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&other, OTHER_REST));
    }
    TEST_ASSERT_EQUAL_INT(CAL_TRACKING, cal_get_phase(&other));

    for (int i = 0; i < SAMPLES_PER_S; i++) {
        TEST_ASSERT_EQUAL_UINT16(0, cal_update(&key, REST));
        TEST_ASSERT_EQUAL_UINT16(300, cal_update(&other, OTHER_REST + 300));
    }

    TEST_ASSERT_EQUAL_UINT16(REST, cal_get_baseline(&key));
    TEST_ASSERT_EQUAL_UINT16(OTHER_REST, cal_get_baseline(&other));
}

/* --------------------------------------------------------------------- */
/* Tests: calibration feeding rapid trigger                              */
/* --------------------------------------------------------------------- */

#define RAMP_STEP         30
#define CYCLES            9
#define PRESSES_PER_CYCLE 3

struct segment {
    uint16_t depth; /* true key depth to move to, in counts */
    uint16_t hold;  /* samples to stay there                */
};

/*
 * One cycle is a full press, a light touch that stays under actuation,
 * and a press with a partial release and re-press: 3 presses and 3
 * releases in 6106 samples, starting and ending at rest.
 *
 * Depths move in steps of RAMP_STEP, so every sample is a multiple of 30.
 * That keeps each one at least 10 counts from a rapid trigger threshold
 * (actuation 103, release 20, sensitivity 41), more than the 2 counts
 * the baseline trails the drift by.
 */
static const struct segment press_cycle[] = {
        {600, 500}, {0, 1000}, {90, 1000}, {0, 1000}, {600, 200}, {300, 100}, {600, 200}, {0, 2000},
};

static const struct rt_config depth_rt_cfg = {
        .rest_adc = 0,
        .actuation_adc = 103,
        .release_adc = 20,
        .rt_sensitivity = 41,
};

static uint16_t trace[DRIFT_SAMPLES];

static size_t append_sample(size_t len, uint16_t depth)
{
    TEST_ASSERT_LESS_THAN_size_t(DRIFT_SAMPLES, len);
    trace[len] = depth;
    return len + 1;
}

/*
 * Ramp from the last depth in the trace to the segment's depth, then
 * hold there.
 */
static size_t append_segment(size_t len, const struct segment *seg)
{
    int depth = len > 0 ? trace[len - 1] : 0;

    while (depth != seg->depth) {
        depth += depth < seg->depth ? RAMP_STEP : -RAMP_STEP;
        len = append_sample(len, (uint16_t)depth);
    }
    for (int i = 0; i < seg->hold; i++) {
        len = append_sample(len, seg->depth);
    }
    return len;
}

/*
 * Fill the trace with CYCLES press cycles, then rest to the end.
 */
static void build_trace(void)
{
    size_t len = 0;

    for (int cycle = 0; cycle < CYCLES; cycle++) {
        for (size_t i = 0; i < sizeof(press_cycle) / sizeof(press_cycle[0]); i++) {
            len = append_segment(len, &press_cycle[i]);
        }
    }
    while (len < DRIFT_SAMPLES) {
        len = append_sample(len, 0);
    }
}

/*
 * The expected events come from rapid trigger run on the true depth.
 * The calibrated path sees the same presses on top of a rest level that
 * drifts across the trace, and must give the same event on every sample.
 *
 * The control subtracts the boot-time rest and nothing else.  It must
 * disagree somewhere, or the drift is too small to prove anything.
 */
static void assert_drift_does_not_change_events(int drift_counts)
{
    struct rt_key_state ideal;
    struct rt_key_state calibrated;
    struct rt_key_state uncompensated;
    int presses = 0;
    int releases = 0;
    int uncompensated_mismatches = 0;

    build_trace();
    rt_init(&ideal, &depth_rt_cfg);
    rt_init(&calibrated, &depth_rt_cfg);
    rt_init(&uncompensated, &depth_rt_cfg);
    boot_at(&key, &example_cfg, REST);

    for (int i = 0; i < DRIFT_SAMPLES; i++) {
        uint16_t raw = (uint16_t)(REST + trace[i] + drift_counts * i / DRIFT_SAMPLES);
        uint16_t fixed_depth = raw > REST ? (uint16_t)(raw - REST) : 0;
        enum rt_event expected = rt_update(&ideal, trace[i]);

        TEST_ASSERT_EQUAL_INT(expected, rt_update(&calibrated, cal_update(&key, raw)));
        uncompensated_mismatches += rt_update(&uncompensated, fixed_depth) != expected;
        presses += expected == RT_EVENT_PRESS;
        releases += expected == RT_EVENT_RELEASE;
    }

    TEST_ASSERT_EQUAL_INT(CYCLES * PRESSES_PER_CYCLE, presses);
    TEST_ASSERT_EQUAL_INT(CYCLES * PRESSES_PER_CYCLE, releases);
    TEST_ASSERT_GREATER_THAN_INT(0, uncompensated_mismatches);
}

void test_upward_drift_does_not_change_rapid_trigger_events(void)
{
    assert_drift_does_not_change_events(DRIFT_COUNTS);
}

void test_downward_drift_does_not_change_rapid_trigger_events(void)
{
    assert_drift_does_not_change_events(-DRIFT_COUNTS);
}

/* --------------------------------------------------------------------- */
/* Runner                                                                */
/* --------------------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_noise_source_is_zero_mean_with_sigma_near_three);

    RUN_TEST(test_depth_is_zero_while_zeroing);
    RUN_TEST(test_first_baseline_is_mean_of_zeroing_readings);
    RUN_TEST(test_first_baseline_rounds_to_nearest_count);
    RUN_TEST(test_init_restarts_zeroing);

    RUN_TEST(test_steady_rest_reads_zero_and_keeps_baseline);
    RUN_TEST(test_noise_at_rest_does_not_bias_baseline);
    RUN_TEST(test_one_count_error_still_moves_baseline);
    RUN_TEST(test_one_count_errors_up_and_down_both_stall_at_max_shift);
    RUN_TEST(test_baseline_follows_slow_upward_drift);
    RUN_TEST(test_baseline_follows_slow_downward_drift);

    RUN_TEST(test_depth_is_reading_minus_rest);
    RUN_TEST(test_long_hold_does_not_move_baseline);
    RUN_TEST(test_reading_at_band_edge_is_a_press);
    RUN_TEST(test_reading_inside_band_is_rest);

    RUN_TEST(test_boot_with_key_held_recovers_after_release);
    RUN_TEST(test_large_downward_shift_is_followed_fast);
    RUN_TEST(test_reading_at_lower_band_edge_takes_a_fast_step);

    RUN_TEST(test_full_scale_zeroing_window_does_not_wrap);
    RUN_TEST(test_reading_can_fall_from_full_scale_to_zero);
    RUN_TEST(test_full_scale_press_from_zero_baseline);
    RUN_TEST(test_rest_band_reaches_past_both_ends_of_the_range);

    RUN_TEST(test_zero_samples_of_zero_zeroes_on_first_reading);
    RUN_TEST(test_rest_band_of_zero_never_raises_baseline);
    RUN_TEST(test_shift_wider_than_max_behaves_as_max);

    RUN_TEST(test_keys_sharing_a_config_are_independent);

    RUN_TEST(test_upward_drift_does_not_change_rapid_trigger_events);
    RUN_TEST(test_downward_drift_does_not_change_rapid_trigger_events);

    return UNITY_END();
}
