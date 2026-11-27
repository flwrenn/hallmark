/*
 * Calibration implementation.
 *
 * The baseline and every error against it are Q16.15 values in an
 * int32_t.  The largest magnitude is 65535 << 15, which is below 2^31, so
 * no sum, difference, or negation here can overflow.
 */

#include "calibration.h"

#define ROUND_HALF_Q15 (1 << (CAL_FRAC_BITS - 1))

enum regime {
    REGIME_PRESSED,
    REGIME_BELOW_REST,
    REGIME_AT_REST,
};

/* --------------------------------------------------------------------- */

static int32_t to_q15(uint16_t counts)
{
    return (int32_t)((uint32_t)counts << CAL_FRAC_BITS);
}

/*
 * Mean of count readings.  The sum of 65535 full-scale readings fits a
 * uint32_t, and the 64-bit widening covers the shift into Q16.15.
 */
static int32_t mean_q15(uint32_t sum, uint16_t count)
{
    return (int32_t)(((uint64_t)sum << CAL_FRAC_BITS) / count);
}

static enum regime classify(int32_t error_q15, int32_t band_q15)
{
    if (error_q15 >= band_q15) {
        return REGIME_PRESSED;
    }
    if (error_q15 <= -band_q15) {
        return REGIME_BELOW_REST;
    }
    return REGIME_AT_REST;
}

/*
 * error >> shift, rounded toward zero so that equal errors above and
 * below the baseline give equal steps.
 */
static int32_t scale(int32_t error_q15, uint8_t shift)
{
    unsigned int bits = shift < CAL_SHIFT_MAX ? shift : CAL_SHIFT_MAX;

    return error_q15 < 0 ? -(-error_q15 >> bits) : error_q15 >> bits;
}

static int32_t baseline_step(const struct cal_config *cfg, int32_t error_q15)
{
    switch (classify(error_q15, to_q15(cfg->rest_band))) {
    case REGIME_PRESSED:
        return 0;
    case REGIME_BELOW_REST:
        return scale(error_q15, cfg->fast_shift);
    case REGIME_AT_REST:
        return scale(error_q15, cfg->slow_shift);
    }

    return 0;
}

/* --------------------------------------------------------------------- */

/*
 * zero_count stops at zero_samples, so it cannot wrap.  Comparing after
 * the reading is counted makes zero_samples == 0 finish on the first one.
 */
static uint16_t handle_zeroing(struct cal_key *key, uint16_t raw)
{
    key->zero_sum += raw;
    key->zero_count++;

    if (key->zero_count >= key->cfg->zero_samples) {
        key->baseline_q15 = mean_q15(key->zero_sum, key->zero_count);
        key->phase = CAL_TRACKING;
    }

    return 0;
}

static uint16_t handle_tracking(struct cal_key *key, uint16_t raw)
{
    uint16_t rest;

    key->baseline_q15 += baseline_step(key->cfg, to_q15(raw) - key->baseline_q15);
    rest = cal_get_baseline(key);

    return raw > rest ? (uint16_t)(raw - rest) : 0;
}

/* --------------------------------------------------------------------- */

void cal_init(struct cal_key *key, const struct cal_config *cfg)
{
    key->cfg = cfg;
    key->phase = CAL_ZEROING;
    key->zero_sum = 0;
    key->zero_count = 0;
    key->baseline_q15 = 0;
}

uint16_t cal_update(struct cal_key *key, uint16_t raw)
{
    switch (key->phase) {
    case CAL_ZEROING:
        return handle_zeroing(key, raw);
    case CAL_TRACKING:
        return handle_tracking(key, raw);
    }

    return 0;
}

enum cal_phase cal_get_phase(const struct cal_key *key)
{
    return key->phase;
}

uint16_t cal_get_baseline(const struct cal_key *key)
{
    return (uint16_t)((key->baseline_q15 + ROUND_HALF_Q15) >> CAL_FRAC_BITS);
}
