/*
 * Rapid Trigger -- per-key actuation via direction reversal.
 *
 * Pure C module, no RTOS dependencies.  ADC values are expected to
 * increase as the key is pressed deeper (higher value = deeper press).
 *
 * State machine:
 *   IDLE  ──press past actuation──▸  ACTIVE
 *   ACTIVE ──reversal up past sensitivity──▸  RELEASING
 *   RELEASING ──reversal down past sensitivity──▸  ACTIVE
 *   ACTIVE / RELEASING ──returns above release point──▸  IDLE
 */

#ifndef RAPID_TRIGGER_H
#define RAPID_TRIGGER_H

#include <stdbool.h>
#include <stdint.h>

/* ---------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------ */

enum rt_state {
    RT_IDLE,
    RT_ACTIVE,
    RT_RELEASING,
};

enum rt_event {
    RT_EVENT_NONE,
    RT_EVENT_PRESS,
    RT_EVENT_RELEASE,
};

/**
 * Per-key configuration.  All values are in raw ADC counts.
 *
 * @rest_adc        ADC reading when the key is fully released (at rest).
 * @actuation_adc   ADC value that triggers the initial press from IDLE.
 * @release_adc     ADC value below which the key is considered fully
 *                  released, returning the FSM to IDLE.
 * @rt_sensitivity  Minimum change (in counts) from peak/trough required
 *                  to register a direction reversal.
 */
struct rt_config {
    uint16_t rest_adc;
    uint16_t actuation_adc;
    uint16_t release_adc;
    uint16_t rt_sensitivity;
};

/**
 * Per-key runtime state.  Initialise with rt_init() before first use.
 */
struct rt_key_state {
    enum rt_state state;
    uint16_t peak;   /* deepest ADC seen since last upward reversal  */
    uint16_t trough; /* shallowest ADC seen since last downward rev  */
    struct rt_config cfg;
};

/* ---------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * Initialise (or reinitialise) a key's rapid-trigger state.
 */
void rt_init(struct rt_key_state *key, const struct rt_config *cfg);

/**
 * Feed a new ADC sample and return the resulting key event.
 *
 * Call once per scan cycle per key.  The function is pure -- it has no
 * side-effects beyond mutating *key and returns a deterministic result.
 */
enum rt_event rt_update(struct rt_key_state *key, uint16_t adc);

/* ---------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------ */

/**
 * Convert millimetres of travel to ADC counts.
 *
 * @counts_per_mm  Calibrated ratio (e.g. usable_range / travel_mm).
 */
static inline uint16_t rt_mm_to_counts(float mm, float counts_per_mm)
{
    return (uint16_t)(mm * counts_per_mm + 0.5f);
}

/**
 * Convert ADC counts to millimetres of travel.
 */
static inline float rt_counts_to_mm(uint16_t counts, float counts_per_mm)
{
    return (float)counts / counts_per_mm;
}

#endif /* RAPID_TRIGGER_H */
