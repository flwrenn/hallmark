/*
 * Calibration -- per-key rest baseline: boot auto-zero and drift tracking.
 *
 * Pure C module, no RTOS dependencies.  Readings are expected to increase
 * as the key is pressed deeper, the same convention as rapid_trigger.h.
 * Sensor polarity is the scan module's job: it flips the reading before
 * this module sees it.
 *
 * cal_update() turns a raw reading into depth above the key's rest level,
 * in ADC counts, clamped at 0.  Every key then shares one set of rapid
 * trigger thresholds, with rt_config.rest_adc = 0.
 *
 * Phases:
 *   ZEROING  ──zero_samples readings averaged──▸  TRACKING
 *
 * While TRACKING, where the reading sits against the baseline decides how
 * the baseline moves:
 *
 *   reading >= baseline + rest_band   pressed      baseline held
 *   reading <= baseline - rest_band   below rest   follows by error >> fast_shift
 *   otherwise                         at rest      follows by error >> slow_shift
 *
 * "Below rest" means the baseline is too high: the key was held during
 * zeroing, or the rest level dropped.  The at-rest step has the same size
 * for an error above or below the baseline, so noise does not bias it.
 *
 * The opposite case has no fast path.  A rest level that jumps up by
 * rest_band or more looks like a press, so the baseline holds until the
 * reading comes back inside the band.  A short downward glitch ends the
 * same way: once the fast path has pulled the baseline down by rest_band
 * or more, the true rest level reads as a press.  At a fast_shift of 4,
 * a dip of 100 counts does that in 3 samples.  cal_init() re-zeroes the
 * key.
 *
 * Time constants are in samples: a shift of n follows a step with a time
 * constant of 2^n calls to cal_update().
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>

/* ---------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------ */

/*
 * The baseline is held in Q16.15 fixed point.  Fifteen fractional bits
 * are the most that keep the baseline, and the signed error against any
 * uint16_t reading, inside an int32_t.
 */
#define CAL_FRAC_BITS 15

/*
 * Widest shift the tracker applies.  A wider slow_shift or fast_shift
 * behaves as CAL_SHIFT_MAX.
 */
#define CAL_SHIFT_MAX CAL_FRAC_BITS

/* ---------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------ */

enum cal_phase {
    CAL_ZEROING,
    CAL_TRACKING,
};

/**
 * Configuration shared by every key.
 *
 * @zero_samples  Readings averaged into the first baseline.  0 behaves
 *                as 1.
 * @rest_band     Counts either side of the baseline treated as rest.
 *                Keep it above the noise peak: a reading that noise
 *                pushes rest_band below the baseline takes a fast step.
 *                Keep it at or below rt_config.release_adc: the baseline
 *                absorbs a press held inside the band, so its depth
 *                decays to 0, or to the stall limit below.
 *                0 leaves no rest regime, so the baseline never rises.
 * @slow_shift    Baseline step inside the band: error >> slow_shift.
 * @fast_shift    Baseline step below the band: error >> fast_shift.
 *
 * The step rounds toward zero, so the baseline stops within
 * 2^(shift - CAL_FRAC_BITS) counts of a steady reading: 1/32 count at a
 * shift of 10, one count at CAL_SHIFT_MAX.
 */
struct cal_config {
    uint16_t zero_samples;
    uint16_t rest_band;
    uint8_t slow_shift;
    uint8_t fast_shift;
};

/**
 * Per-key runtime state.  Initialise with cal_init() before first use.
 */
struct cal_key {
    const struct cal_config *cfg;
    enum cal_phase phase;
    uint32_t zero_sum;    /* sum of the readings taken while zeroing */
    uint16_t zero_count;  /* number of readings in zero_sum          */
    int32_t baseline_q15; /* rest level in Q16.15, valid in TRACKING */
};

/* ---------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * Start (or restart) a key in CAL_ZEROING.
 *
 * *cfg is not copied.  It must outlive the key.
 */
void cal_init(struct cal_key *key, const struct cal_config *cfg);

/**
 * Feed a new reading and return the key's depth above rest in counts.
 *
 * Call once per scan cycle per key.  Returns 0 for every reading taken
 * while zeroing, including the one that completes it.
 */
uint16_t cal_update(struct cal_key *key, uint16_t raw);

/**
 * Current phase of the key.
 */
enum cal_phase cal_get_phase(const struct cal_key *key);

/**
 * Rest baseline rounded to the nearest count.  0 while zeroing.
 */
uint16_t cal_get_baseline(const struct cal_key *key);

#endif /* CALIBRATION_H */
